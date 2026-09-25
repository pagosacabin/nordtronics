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
| `/var/lib/nordtronics/` | wildfire-ingest:wildfire-data 2770 | the SQLite database |
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

# reload the broker whenever certbot renews
sudo install -o root -g root -m 0755 \
  /opt/nordtronics/backend/mosquitto/renew-hook.sh \
  /etc/letsencrypt/renewal-hooks/deploy/00-reload-mosquitto.sh
```

Confirm the broker will be able to read the key — this must print the key:

```bash
sudo -u mosquitto cat /etc/letsencrypt/live/nordtronics.io/privkey.pem | head -1
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
```

The setgid bit is deliberate: every file the ingest worker creates — including
the `-wal` and `-shm` sidecars — stays in the `wildfire-data` group, which is
how the API reads them.

## 7. The ingest worker's secret

No secret ever enters the repository. systemd reads this file as root before
dropping privileges, so root-only permissions are correct here.

```bash
sudo install -d -o root -g root -m 0755 /etc/nordtronics
sudo tee /etc/nordtronics/ingest.env >/dev/null <<EOF
WILDFIRE_MQTT_HOST=127.0.0.1
WILDFIRE_MQTT_PORT=8883
WILDFIRE_MQTT_CA=/etc/letsencrypt/live/nordtronics.io/chain.pem
WILDFIRE_MQTT_USERNAME=wildfire-ingest
WILDFIRE_MQTT_PASSWORD=$INGEST_PW
WILDFIRE_DB_PATH=/var/lib/nordtronics/wildfire.db
WILDFIRE_LOG_LEVEL=INFO
EOF
sudo chmod 0600 /etc/nordtronics/ingest.env
```

The worker connects to `127.0.0.1:8883` — the broker's own listener — so the
certificate name does not have to resolve here.

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
mosquitto_pub -h mqtt.nordtronics.io -p 8883 \
  --cafile /etc/letsencrypt/live/nordtronics.io/chain.pem \
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
  --cafile /etc/letsencrypt/live/nordtronics.io/chain.pem \
  -t nordtronics/wildfire/node-01/telemetry -m x -q 1
# expect: Connection error / not authorised

# plaintext 1883: must time out — UFW never opened it
timeout 5 mosquitto_pub -h mqtt.nordtronics.io -p 1883 -t x -m y
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
| ingest worker: `not authorised` / connection refused | wrong password in `/etc/nordtronics/ingest.env`, or the broker is not running |
| API returns 503 `telemetry database is not available` | the database file does not exist yet: the ingest worker creates it, so start that first |
| API returns 200 locally but nginx gives 502 | `proxy_pass` port differs from `WILDFIRE_API_PORT`, or `wildfire-api` is down |
| readings rejected as `invalid` in the journal | the base station is sending out-of-range or non-numeric fields; the log line names the field |
| readings rejected as `node_mismatch` | the payload's `node_id` differs from the topic's `<node-id>` — fix the firmware, not the broker |
| certificate renewed but the broker still serves the old one | the deploy hook is missing or not executable in `/etc/letsencrypt/renewal-hooks/deploy/` |
