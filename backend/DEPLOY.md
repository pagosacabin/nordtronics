# Deploying backend v1 to the VPS

Written for a second person: every command is copy-pasteable, in order, as the
`deploy` admin user (passwordless sudo). VPS is the Contabo host at
`89.117.21.105`, Ubuntu 24.04, already hardened by
[`docs/vps-hardening-runbook.md`](../docs/vps-hardening-runbook.md) — UFW
allowing only 22/80/443/8883, fail2ban, unattended-upgrades, DNS for
`nordtronics.io` / `mqtt.nordtronics.io` / `api.nordtronics.io`, and a Let's
Encrypt certificate covering all three names.

Where everything lands:

| Path | Owner | Purpose |
|---|---|---|
| `/opt/nordtronics/backend` | root | the tree from this repo |
| `/opt/nordtronics/venv` | root | one venv both services share |
| `/etc/mosquitto/nordtronics.conf`, `/etc/mosquitto/acl` | root:mosquitto 0640 | broker config |
| `/etc/mosquitto/passwd` | root:mosquitto 0640 | MQTT users (created here, never in git) |
| `/etc/nordtronics/ingest.env` | root 0600 | the ingest worker's MQTT password |
| `/etc/nordtronics/mqtt-ca.pem` | root:wildfire-data 0640 | the ingest worker's pinned MQTT CA bundle (step 7) |
| `/var/lib/nordtronics/` | wildfire-ingest:wildfire-data 2770, database files 0660 | the SQLite database and its WAL sidecars |
| `/var/lib/mosquitto/` | mosquitto:mosquitto | broker persistence |

Three processes: `mosquitto` (packaged service user), `wildfire-ingest`,
`wildfire-api`. **Nothing runs as root or as `deploy`.**

---

## 1. Packages

```bash
sudo apt-get update
sudo apt-get install -y mosquitto mosquitto-clients acl python3-venv ca-certificates
```

`acl` is needed for step 4. `mosquitto-clients` gives you `mosquitto_pub` /
`mosquitto_sub` for the verification in step 10.

The packaged broker is **not** used: our unit runs `mosquitto -c
/etc/mosquitto/nordtronics.conf`, so the packaged `/etc/mosquitto/mosquitto.conf`
and `/etc/mosquitto/conf.d/` are ignored. Leave them alone; nothing reads them.

## 2. Service users and the shared data group

```bash
sudo addgroup --system wildfire-data
sudo adduser --system --no-create-home --disabled-login --group wildfire-ingest
sudo adduser --system --no-create-home --disabled-login --group wildfire-api
sudo adduser wildfire-ingest wildfire-data
sudo adduser wildfire-api wildfire-data
```

`wildfire-ingest` owns the database; `wildfire-api` shares the group so it can
open the WAL sidecars. Verify:

```bash
getent group wildfire-data
```

## 3. The tree and the Python environment

```bash
cd ~
git clone --branch main https://github.com/pagosacabin/nordtronics.git nordtronics-src
sudo install -d -o root -g root -m 0755 /opt/nordtronics
sudo rsync -a --delete ~/nordtronics-src/backend/ /opt/nordtronics/backend/
sudo python3 -m venv /opt/nordtronics/venv
sudo /opt/nordtronics/venv/bin/pip install --upgrade pip
sudo /opt/nordtronics/venv/bin/pip install \
  -r /opt/nordtronics/backend/ingest/requirements.txt \
  -r /opt/nordtronics/backend/api/requirements.txt
```

`rsync --delete` makes the VPS tree match the checkout exactly, so deleting a
file in the repo deletes it here too. The venv is root-owned and world-readable;
both service users only read and execute from it.

## 4. Broker config, ACL and certificate access

```bash
sudo install -o root -g mosquitto -m 0640 \
  /opt/nordtronics/backend/mosquitto/mosquitto.conf /etc/mosquitto/nordtronics.conf
sudo install -o root -g mosquitto -m 0640 \
  /opt/nordtronics/backend/mosquitto/acl /etc/mosquitto/acl

# certbot's private key is root-only, so the unprivileged `mosquitto` user
# cannot read it. rX = traverse directories, read files; the -d entries make
# renewed key files inherit the same access, so renewals need no manual step.
sudo setfacl -R  -m u:mosquitto:rX /etc/letsencrypt/live /etc/letsencrypt/archive
sudo setfacl -R -d -m u:mosquitto:rX /etc/letsencrypt/live /etc/letsencrypt/archive

# Retained for the ingest worker, which used to verify the broker against this
# same chain.pem. Step 7 now gives it a pinned CA bundle of its own under
# /etc/nordtronics/ (chain.pem alone cannot anchor the chain), so the worker no
# longer reads this directory and the grant is no longer load-bearing for its
# TLS — it is kept because it is harmless and still lets this user read the
# live certificate directory.
sudo setfacl -R  -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive
sudo setfacl -R -d -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive

# On renewal the hook does two jobs: rebuild the worker's pinned CA bundle
# (step 7 — a renewed chain must not leave that bundle stale) and reload the
# broker so it serves the new certificate.
sudo install -o root -g root -m 0755 \
  /opt/nordtronics/backend/mosquitto/renew-hook.sh \
  /etc/letsencrypt/renewal-hooks/deploy/00-reload-mosquitto.sh
```

Confirm the broker will be able to read the key — this must print the key:

```bash
sudo -u mosquitto cat /etc/letsencrypt/live/nordtronics.io/privkey.pem | head -1
```

Confirm the ingest grant above is in place — it is no longer needed for the
worker's own TLS (step 7 builds it a bundle), so this is a check of the grant
itself; it must print `-----BEGIN CERTIFICATE-----`:

```bash
sudo -u wildfire-ingest cat /etc/letsencrypt/live/nordtronics.io/chain.pem | head -1
```

## 5. MQTT users

Node usernames are node ids: the ACL gives each authenticated user write access
to `nordtronics/wildfire/<username>/telemetry` and nothing else.

```bash
INGEST_PW="$(openssl rand -base64 24)"
NODE01_PW="$(openssl rand -base64 24)"
sudo mosquitto_passwd -c -b /etc/mosquitto/passwd wildfire-ingest "$INGEST_PW"
sudo mosquitto_passwd -b /etc/mosquitto/passwd node-01 "$NODE01_PW"
sudo chown root:mosquitto /etc/mosquitto/passwd && sudo chmod 0640 /etc/mosquitto/passwd
echo "node-01 password (record it for the base station): $NODE01_PW"
```

`$INGEST_PW` is used in step 7. Keep that shell open, or re-run this step.

## 6. Database directory

```bash
sudo install -d -o wildfire-ingest -g wildfire-data -m 2770 /var/lib/nordtronics

# Create the database file now, at mode 0660. The group-write bit is not
# cosmetic: SQLite creates a new `-shm`/`-wal` sidecar with the mode of the
# database file it belongs to, and WAL needs the *reader* to write `-shm`
# (backend/common/db.py says so; the API opens the database with mode=ro and
# still needs this). For a file it has to create itself SQLite uses base mode
# 0644, and the units' UMask=0007 can only clear bits — 0644 & ~0007 = 0640 —
# so if the database is created by the service the sidecars come out 0640 and
# whichever service touches the database first locks the other one out with
# `attempt to write a readonly database`. `touch` never truncates, so this is
# safe to re-run against a database that already has data in it.
sudo -u wildfire-ingest touch /var/lib/nordtronics/wildfire.db
sudo find /var/lib/nordtronics -maxdepth 1 -name 'wildfire.db*' \
  -exec chown wildfire-ingest:wildfire-data {} + -exec chmod 0660 {} +
```

The setgid bit is deliberate: every file the ingest worker creates — including
the `-wal` and `-shm` sidecars — stays in the `wildfire-data` group, which is
how the API reads them.

The file is empty until step 8's worker applies the schema, so for those few
seconds `/healthz` answers `500 internal error` (the tables do not exist yet)
rather than the `503 telemetry database is not available` it returns for a
missing file. Both clear as soon as the worker has started once; step 10 checks
the endpoint after that.

## 7. The ingest worker's secret and its pinned CA bundle

No secret ever enters the repository. systemd reads this file as root before
dropping privileges, so root-only permissions are correct here.

The worker verifies the broker's certificate against its own pinned bundle
rather than the live `chain.pem`, because `chain.pem` **cannot anchor the
chain**: it ends at ISRG Root X2 cross-signed by ISRG Root X1, and that
self-signed X1 is not part of the file, so a client that trusts `chain.pem`
fails the handshake with `Verify return code: 2 (unable to get issuer
certificate)`. Appending the self-signed X1 from the system CA store gives the
bundle the anchor the chain needs:

```bash
sudo install -d -o root -g root -m 0755 /etc/nordtronics

# 4 certificates: the served chain (leaf, Root YE, cross-signed ISRG Root X2)
# plus the SELF-SIGNED ISRG Root X1 that terminates it. Root-owned, group
# wildfire-data: the worker can read it, nothing else can write it.
sudo sh -c 'cat /etc/letsencrypt/live/nordtronics.io/chain.pem \
             /usr/share/ca-certificates/mozilla/ISRG_Root_X1.crt \
             > /etc/nordtronics/mqtt-ca.pem'
sudo chown root:wildfire-data /etc/nordtronics/mqtt-ca.pem
sudo chmod 0640 /etc/nordtronics/mqtt-ca.pem

# The certificate's SANs are DNS names only (nordtronics.io, www., api.,
# mqtt.) — there is no IP SAN — and Python/paho verify the host they are
# given, so WILDFIRE_MQTT_HOST=127.0.0.1 can never pass hostname
# verification (`IP address mismatch, certificate is not valid for
# '127.0.0.1'`). Point the name at the broker's own loopback listener
# instead: the worker then verifies `mqtt.nordtronics.io`, which is in the
# certificate, while the traffic still stays on 127.0.0.1.
grep -q '^127\.0\.0\.1 mqtt\.nordtronics\.io$' /etc/hosts || \
  echo '127.0.0.1 mqtt.nordtronics.io' | sudo tee -a /etc/hosts

sudo tee /etc/nordtronics/ingest.env >/dev/null <<EOF
WILDFIRE_MQTT_HOST=mqtt.nordtronics.io
WILDFIRE_MQTT_PORT=8883
WILDFIRE_MQTT_CA=/etc/nordtronics/mqtt-ca.pem
WILDFIRE_MQTT_USERNAME=wildfire-ingest
WILDFIRE_MQTT_PASSWORD=$INGEST_PW
WILDFIRE_DB_PATH=/var/lib/nordtronics/wildfire.db
WILDFIRE_LOG_LEVEL=INFO
EOF
sudo chmod 0600 /etc/nordtronics/ingest.env
```

Both halves are needed: a bundle that anchors the chain does not help while the
host is an IP, and the right hostname does not help while the CA cannot build a
path to a trust anchor.

Step 4's renewal hook rebuilds this bundle from the renewed `chain.pem`, so a
renewal cannot leave the worker trusting a stale chain. Confirm the worker can
read its CA before starting the service:

```bash
sudo -u wildfire-ingest head -1 /etc/nordtronics/mqtt-ca.pem   # -----BEGIN CERTIFICATE-----
```

## 8. systemd units

```bash
sudo install -o root -g root -m 0644 \
  /opt/nordtronics/backend/mosquitto/mosquitto.service /etc/systemd/system/mosquitto.service
sudo install -o root -g root -m 0644 \
  /opt/nordtronics/backend/ingest/wildfire-ingest.service /etc/systemd/system/
sudo install -o root -g root -m 0644 \
  /opt/nordtronics/backend/api/wildfire-api.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now mosquitto wildfire-ingest wildfire-api
```

`/etc/systemd/system/mosquitto.service` deliberately keeps the packaged unit's
name so it overrides `/lib/systemd/system/mosquitto.service`; the packaged
service user and data directory are reused.

## 9. nginx for api.nordtronics.io

Add to the existing nginx config (e.g. `/etc/nginx/sites-available/nordtronics`):

```nginx
server {
    listen 443 ssl;
    listen [::]:443 ssl;
    server_name api.nordtronics.io;

    ssl_certificate     /etc/letsencrypt/live/nordtronics.io/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/nordtronics.io/privkey.pem;

    location / {
        proxy_pass         http://127.0.0.1:8000;
        proxy_set_header   Host              $host;
        proxy_set_header   X-Real-IP         $remote_addr;
        proxy_set_header   X-Forwarded-For   $proxy_add_x_forwarded_for;
        proxy_set_header   X-Forwarded-Proto $scheme;
    }
}
```

The API binds `127.0.0.1:8000` only; nginx terminates TLS. The existing
port-80 site keeps serving the ACME challenge. Then:

```bash
sudo nginx -t && sudo systemctl reload nginx
```

## 10. Verify end to end

```bash
# all three services up
systemctl is-active mosquitto wildfire-ingest wildfire-api

# broker read the certificate and the password file
sudo journalctl -u mosquitto -n 20 --no-pager
# expect: "Opening ipv4 listen socket on port 8883" and no cert/key error

# API is alive on loopback and through nginx
curl -sS http://127.0.0.1:8000/healthz
curl -sS https://api.nordtronics.io/healthz
```

Then push a reading through the real path and watch it come back:

```bash
# `deploy` is not in the `wildfire-data` group, so it cannot read the worker's
# pinned bundle (step 7, 0640 root:wildfire-data). The operator-side client
# below therefore verifies against the system CA store, which contains the same
# self-signed ISRG Root X1 and anchors the identical chain — the worker's own
# bundle is proven by `wildfire-ingest` starting and staying up in check 1.
mosquitto_pub -h mqtt.nordtronics.io -p 8883 \
  --cafile /etc/ssl/certs/ca-certificates.crt \
  -u node-01 -P "$NODE01_PW" -q 1 \
  -t nordtronics/wildfire/node-01/telemetry \
  -m '{"node_id":"node-01","pm25":5.0,"temperature_c":20.0,"humidity_pct":40.0,"battery_v":3.9,"observed_utc":"'"$(date -u +%Y-%m-%dT%H:%M:%SZ)"'"}'

sleep 2
curl -sS "https://api.nordtronics.io/v1/nodes" | python3 -m json.tool
# expect node-01 with reading_count >= 1 and status "ok"
curl -sS "https://api.nordtronics.io/v1/nodes/node-01/readings?limit=5" | python3 -m json.tool
```

Finally, confirm the broker refuses what it should:

```bash
# anonymous: must fail
mosquitto_pub -h mqtt.nordtronics.io -p 8883 \
  --cafile /etc/ssl/certs/ca-certificates.crt \
  -t nordtronics/wildfire/node-01/telemetry -m x -q 1
# expect: Connection error / not authorised

# plaintext 1883: must time out — UFW never opened it, and the broker has no
# 1883 listener. Address the host by its PUBLIC IP here: step 7 pinned
# mqtt.nordtronics.io to 127.0.0.1 in /etc/hosts, and a loopback connection
# would be refused by the missing listener instead of dropped by the firewall.
timeout 5 mosquitto_pub -h 89.117.21.105 -p 1883 -t x -m y
```

## 11. Adding a node

```bash
NODE_PW="$(openssl rand -base64 24)"
sudo mosquitto_passwd -b /etc/mosquitto/passwd node-02 "$NODE_PW"
sudo systemctl reload mosquitto      # password_file is read on startup/reload
echo "node-02 password: $NODE_PW"
```

No config change is needed: the ACL pattern `nordtronics/wildfire/%u/telemetry`
already grants every username its own subtree, and the API picks new nodes up
from the database as readings arrive.

## 12. Updating to a newer version

```bash
cd ~/nordtronics-src && git fetch origin && git checkout main && git pull --ff-only
sudo rsync -a --delete ~/nordtronics-src/backend/ /opt/nordtronics/backend/
sudo /opt/nordtronics/venv/bin/pip install \
  -r /opt/nordtronics/backend/ingest/requirements.txt \
  -r /opt/nordtronics/backend/api/requirements.txt
sudo install -o root -g mosquitto -m 0640 \
  /opt/nordtronics/backend/mosquitto/mosquitto.conf /etc/mosquitto/nordtronics.conf
sudo install -o root -g mosquitto -m 0640 \
  /opt/nordtronics/backend/mosquitto/acl /etc/mosquitto/acl
sudo install -o root -g root -m 0644 \
  /opt/nordtronics/backend/mosquitto/mosquitto.service /etc/systemd/system/mosquitto.service
sudo install -o root -g root -m 0644 \
  /opt/nordtronics/backend/ingest/wildfire-ingest.service /etc/systemd/system/
sudo install -o root -g root -m 0644 \
  /opt/nordtronics/backend/api/wildfire-api.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl restart mosquitto wildfire-ingest wildfire-api
```

Adding a schema column is safe (`common/schema.sql` is applied with
`CREATE TABLE IF NOT EXISTS` at every worker start), but a column *change* needs
a migration — there is none yet, so check `schema_version` before rolling one
out.

## Rollback

```bash
sudo systemctl disable --now wildfire-api wildfire-ingest mosquitto
sudo rm /etc/systemd/system/wildfire-api.service \
        /etc/systemd/system/wildfire-ingest.service \
        /etc/systemd/system/mosquitto.service
sudo systemctl daemon-reload
sudo systemctl enable --now mosquitto   # back to the packaged default broker
```

The database under `/var/lib/nordtronics` is never touched by a rollback.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| `mosquitto` exits: `Unable to open /etc/letsencrypt/.../privkey.pem` | step 4's `setfacl` did not run, or was run before the cert existed — re-run both `setfacl` lines |
| `mosquitto` exits: `Unable to open acl_file` / `password_file` | file missing or not readable by the `mosquitto` group (`0640 root:mosquitto`) |
| `mosquitto` will not start at all after upgrade, port in use | the packaged broker is still running an old config: `sudo systemctl restart mosquitto` |
| ingest worker: `unable to open database file` | `/var/lib/nordtronics` ownership — `sudo chown -R wildfire-ingest:wildfire-data /var/lib/nordtronics` |
| ingest worker: `attempt to write a readonly database` while the API is up | the database or its `-wal`/`-shm` sidecars have no group-write bit: re-run step 6's `touch` + `find … -chmod 0660` pair, then `sudo systemctl restart wildfire-ingest wildfire-api` (a sidecar the other service created is what locks this one out) |
| ingest worker: `unable to get issuer certificate` | the CA is the live `chain.pem` instead of the step-7 pinned bundle, or the bundle is stale after a renewal — check `WILDFIRE_MQTT_CA` and that step 4's deploy hook is installed |
| ingest worker: `IP address mismatch, certificate is not valid for '127.0.0.1'` | `WILDFIRE_MQTT_HOST` is a literal IP; use `mqtt.nordtronics.io` with step 7's `/etc/hosts` pin (the certificate has no IP SAN) |
| ingest worker: `not authorised` / connection refused | wrong password in `/etc/nordtronics/ingest.env`, or the broker is not running |
| API returns 503 `telemetry database is not available` | the database file does not exist — step 6 creates it |
| API returns 500 `internal error` on `/healthz` | the file exists but has no schema yet: the ingest worker applies it at startup, so start that service (step 8) |
| API returns 200 locally but nginx gives 502 | `proxy_pass` port differs from `WILDFIRE_API_PORT`, or `wildfire-api` is down |
| readings rejected as `invalid` in the journal | the base station is sending out-of-range or non-numeric fields; the log line names the field |
| readings rejected as `node_mismatch` | the payload's `node_id` differs from the topic's `<node-id>` — fix the firmware, not the broker |
| certificate renewed but the broker still serves the old one | the deploy hook is missing or not executable in `/etc/letsencrypt/renewal-hooks/deploy/` |
