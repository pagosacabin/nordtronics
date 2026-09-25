---
task_id: "0064"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: "hermes/0063-deploy-abc-fix"
  sha: "95de2ae6bdd7b60906a6583b941175b603694715"
  run: "https://github.com/pagosacabin/nordtronics/actions/runs/36170346279"
  files: []
  applied_to_vps:
    - "/opt/nordtronics/backend/** rsynced to revision 95de2ae6bdd7"
    - "/etc/nordtronics/mqtt-ca.pem (new; 0640 root:wildfire-data; sha256 6645eb20a82a34ef4421ae543b9020ecbe377ba4bafa3b2305e1dd12ddc71928)"
    - "/etc/nordtronics/ingest.env (WILDFIRE_MQTT_HOST, WILDFIRE_MQTT_CA)"
    - "/etc/hosts (+127.0.0.1 mqtt.nordtronics.io)"
    - "/etc/letsencrypt/renewal-hooks/deploy/00-reload-mosquitto.sh (0063 revision; executed, exit 0)"
    - "/var/lib/nordtronics/wildfire.db, -shm, -wal -> 0660 wildfire-ingest:wildfire-data"
    - "/etc/systemd/system/wildfire-ingest.service, /etc/systemd/system/wildfire-api.service"
    - "/etc/nginx/sites-available/nordtronics.io (api vhost appended; backup nordtronics.io.bak-0064)"
  services:
    mosquitto: active
    wildfire-api: active
    wildfire-ingest: active
  end_to_end: "one MQTT message published over TLS -> stored in SQLite (reading id 1) -> served by https://api.nordtronics.io/v1/nodes (checked from off-host, 200)"
notes: |
  DONE on the VPS, end to end. Applied the 0063 runbook changes exactly:
  pinned CA bundle + renew hook, WILDFIRE_MQTT_HOST=mqtt.nordtronics.io with the
  /etc/hosts pin, database and sidecars at 0660, units reinstalled (UMask=0007
  confirmed in both), worker restarted. Crash loop is over: NRestarts 1410 -> 0
  with zero scheduled restarts in the journal since the fix, and the worker
  connects to mqtt.nordtronics.io:8883 against its own bundle.

  No repository file changed (this is the apply task): proof.files is empty on
  purpose and proof.branch/sha/run name the revision applied to the VPS and the
  CI run that green-verified that revision, re-checked this run.

  Declared deviations / corrections:
  1. VPS tree moved to the 0063 tip and rsynced to /opt (step 12 path), because
     step 4 installs the changed renew-hook.sh. No source edited on the VPS. The
     tree was left deploy-owned by `rsync -a`; restored to root:root per the
     DEPLOY.md ownership table.
  2. The step-10 plaintext-1883 check does NOT test UFW when run on the VPS: it
     returns "Connection refused" (UFW accepts lo, and a host reaching its own
     public IP goes via lo). From an off-host machine 1883 times out (6.01s)
     while 8883/443/80 connect, so UFW is correct and the runbook's expected
     output is only reachable from outside. Doc fix is a follow-up; no source
     edit made.
  3. deploy cannot read the pinned bundle (0640 root:wildfire-data), so the
     bundle probes ran as wildfire-ingest; 0063's open decision (read ACL or
     0644 for deploy) is NOT taken.
  4. Cross-node ACL denial is invisible to the publisher under MQTT 3.1.1
     (PUBACK RC:0 either way), so it was proven with an independent subscriber
     on the ingest credentials: only the control message arrived, and no node-02
     row exists.
  5. Pre-existing (not fixed, out of scope): mosquitto warns that
     /etc/mosquitto/passwd and /etc/mosquitto/acl are not owned by `mosquitto`;
     future versions will refuse to load them.
  6. Secrets: none in the reply. node-01 credentials read from
     /etc/nordtronics/mqtt-credentials.env into shell variables, never printed.

  No secrets, nothing written to handoff/outbox/, 0059/0062 files untouched.
---

# 0064 — Apply the 0063 A/B/C fixes on the VPS and finish the deploy

## Context

0063 is verified green (`hermes/0063-deploy-abc-fix @
95de2ae6bdd7b60906a6583b941175b603694715`, CI run 36170346279 success).
Its runbook changes fix the three defects blocking 0059/0062. This task
applies them on the VPS and completes the deploy. This supersedes the
blocked 0059 and 0062 — when this is done, both are done.

Fold in the two corrections from 0063's notes:

1. Fix C is narrower than 0062 concluded: SQLite gives new -shm/-wal the
   mode of the db FILE, so `chmod 0660` on `wildfire.db` makes the
   sidecars shareable. `UMask=0007` was already present at the base
   commit — verify it is there, do not treat it as the fix.
2. The pinned bundle is 0640 root:wildfire-data and `deploy` is not in
   wildfire-data: for the operator-side end-to-end test, run
   mosquitto_pub against the system CA store
   (`--cafile /etc/ssl/certs/ca-certificates.crt`), not the bundle. No
   permission change needed for the test.

## Task

SSH to the VPS as `deploy` and apply the 0063 runbook changes exactly:

1. Build `/etc/nordtronics/mqtt-ca.pem` = `chain.pem` + the self-signed
   ISRG Root X1, owned `root:wildfire-data`, mode 0640. Point
   `WILDFIRE_MQTT_CA` at it in the ingest environment file. Install the
   renew-hook step that rebuilds the bundle on renewal (the hook also
   restarts wildfire-ingest — keep that).
2. Set `WILDFIRE_MQTT_HOST=mqtt.nordtronics.io` in the ingest
   environment file and add `127.0.0.1 mqtt.nordtronics.io` to
   `/etc/hosts`.
3. `chmod 0660 /var/lib/nordtronics/wildfire.db` (and the -shm/-wal),
   group `wildfire-data`; confirm `UMask=0007` is present in both
   service units.
4. Restart `wildfire-ingest`. Confirm the crash loop is over: the
   service stays active and `NRestarts` stops climbing. Note: the
   preseeded DB makes `/healthz` return 500 until the worker writes its
   schema — that transient is expected; verify `/healthz` goes 200 after.
5. Resume the runbook from step 8: finish nginx/API proxying and run the
   full end-to-end test — `mosquitto_pub` (TLS, system CA store, as
   `deploy`) publishes a test reading, then show it landed in SQLite and
   is served by the HTTPS API.

## Success criteria

1. `mosquitto`, `wildfire-api`, `wildfire-ingest` all active, ingest no
   longer restart-looping.
2. End-to-end proven: one published MQTT message is visible in SQLite
   and retrievable through the public HTTPS API.
3. 0059/0062's original goal (backend deployed, API proxied, pipeline
   tested) is met.

## Constraints

- VPS changes only as the 0063 runbook directs. No source edits on the
  VPS — laptop builds, VPS serves.
- No secrets in the reply (redact passwords/tokens).
- Cost bound: deepseek-flash only, off-peak hours.

## Proof

Active states for all three services plus the exact `mosquitto_pub`,
SQLite, and HTTPS API test output, quoted in the reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then
the reply body: per-step results, the service states, and the verbatim
end-to-end test output.

---

## Reply — 0063's A/B/C fixes applied on the VPS; deploy complete (2026-09-25 19:17-19:21 UTC)

Applied revision `hermes/0063-deploy-abc-fix @ 95de2ae6bdd7b60906a6583b941175b603694715`.
Re-verified this run: `git ls-remote --heads origin hermes/0063-deploy-abc-fix` =
`95de2ae6bdd7b60906a6583b941175b603694715`, and `gh run view 36170346279 --json
headSha,conclusion` = `{"conclusion":"success","headSha":"95de2ae6bdd7…","status":"completed"}`
— the run's `headSha` equals the branch tip, so it is not a stale pointer. The VPS checkout
`~/nordtronics-src` was moved to that same revision (`git rev-parse HEAD` =
`95de2ae6bdd7b60906a6583b941175b603694715`) and rsynced to `/opt/nordtronics/backend/`, which is
what step 4's `install` of the new `renew-hook.sh` and step 8's unit install read from.

This task changes no file in the repository — it is the VPS apply — so `proof.files` is empty by
design and `proof.applied_to_vps` lists what actually changed on 89.117.21.105. The
`branch`/`sha`/`run` entries name the revision that was applied and the CI run that green-verified
that exact revision.

### Step 1 — pinned CA bundle (fix A)

```console
$ sudo sh -c 'cat /etc/letsencrypt/live/nordtronics.io/chain.pem \
               /usr/share/ca-certificates/mozilla/ISRG_Root_X1.crt > /etc/nordtronics/mqtt-ca.pem'
$ sudo chown root:wildfire-data /etc/nordtronics/mqtt-ca.pem && sudo chmod 0640 /etc/nordtronics/mqtt-ca.pem
$ sudo ls -l /etc/nordtronics/mqtt-ca.pem
-rw-r----- 1 root wildfire-data 5462 Sep 25 21:17 /etc/nordtronics/mqtt-ca.pem
$ sudo grep -c "BEGIN CERTIFICATE" /etc/nordtronics/mqtt-ca.pem
4
$ sudo -u wildfire-ingest head -1 /etc/nordtronics/mqtt-ca.pem
-----BEGIN CERTIFICATE-----
```

`ingest.env` now carries the new path (password line elided; the file is still 0600 root):

```console
$ sudo sed -E 's/(PASSWORD=).*/\1<redacted>/' /etc/nordtronics/ingest.env
WILDFIRE_MQTT_HOST=mqtt.nordtronics.io
WILDFIRE_MQTT_PORT=8883
WILDFIRE_MQTT_CA=/etc/nordtronics/mqtt-ca.pem
WILDFIRE_MQTT_USERNAME=wildfire-ingest
WILDFIRE_MQTT_PASSWORD=<redacted>
WILDFIRE_DB_PATH=/var/lib/nordtronics/wildfire.db
WILDFIRE_LOG_LEVEL=INFO
```

Before restarting the worker I proved the bundle anchors the served chain, and that the old value
still fails — `deploy` cannot read the 0640 bundle, so both probes run as `wildfire-ingest`:

```console
$ sudo -u wildfire-ingest openssl s_client -connect 127.0.0.1:8883 -servername mqtt.nordtronics.io \
    -CAfile /etc/nordtronics/mqtt-ca.pem -verify_hostname mqtt.nordtronics.io </dev/null
subject=CN = nordtronics.io
issuer=C = US, O = Let's Encrypt, CN = YE1
Verification: OK
Verify return code: 0 (ok)

$ sudo -u wildfire-ingest openssl s_client -connect 127.0.0.1:8883 -servername mqtt.nordtronics.io \
    -CAfile /etc/letsencrypt/live/nordtronics.io/chain.pem </dev/null
verify error:num=2:unable to get issuer certificate
Verify return code: 2 (unable to get issuer certificate)
```

Renew hook installed from the applied revision, then **executed** (installed-but-untested is not a
verification): exit 0, bundle rebuilt to a byte-identical hash, mosquitto reloaded, worker
restarted by the hook itself.

```console
$ sudo install -o root -g root -m 0755 \
    /opt/nordtronics/backend/mosquitto/renew-hook.sh \
    /etc/letsencrypt/renewal-hooks/deploy/00-reload-mosquitto.sh
$ sudo ls -l /etc/letsencrypt/renewal-hooks/deploy/
-rwxr-xr-x 1 root root 2026 Sep 25 21:17 00-reload-mosquitto.sh
-rwxr-xr-x 1 root root   33 Sep 24 14:30 reload-nginx.sh
$ sudo /etc/letsencrypt/renewal-hooks/deploy/00-reload-mosquitto.sh; echo "hook exit: $?"
hook exit: 0
bundle sha256 before=6645eb20a82a34ef4421ae543b9020ecbe377ba4bafa3b2305e1dd12ddc71928 after=6645eb20… same=yes
```

### Step 2 — hostname (fix B)

```console
$ sudo sed -i -e 's|^WILDFIRE_MQTT_HOST=.*|WILDFIRE_MQTT_HOST=mqtt.nordtronics.io|' \
              -e 's|^WILDFIRE_MQTT_CA=.*|WILDFIRE_MQTT_CA=/etc/nordtronics/mqtt-ca.pem|' \
              /etc/nordtronics/ingest.env
$ grep -q "^127\.0\.0\.1 mqtt\.nordtronics\.io$" /etc/hosts || \
    echo "127.0.0.1 mqtt.nordtronics.io" | sudo tee -a /etc/hosts
$ grep -n "mqtt.nordtronics.io" /etc/hosts
16:127.0.0.1 mqtt.nordtronics.io
```

The worker's own log is the proof both halves took effect — it dials the DNS name and completes
the handshake against the pinned bundle:

```console
2026-09-25 21:17:55,586 INFO wildfire.ingest database ready at /var/lib/nordtronics/wildfire.db (schema v1)
2026-09-25 21:17:55,588 INFO wildfire.ingest connecting to mqtt.nordtronics.io:8883
2026-09-25 21:17:55,634 INFO wildfire.ingest connected to mqtt.nordtronics.io:8883
2026-09-25 21:17:55,634 INFO wildfire.ingest subscribed to nordtronics/wildfire/+/telemetry (qos 1)
```

### Step 3 — database modes (fix C)

```console
$ sudo -u wildfire-ingest touch /var/lib/nordtronics/wildfire.db
$ sudo find /var/lib/nordtronics -maxdepth 1 -name "wildfire.db*" \
    -exec chown wildfire-ingest:wildfire-data {} + -exec chmod 0660 {} +
$ sudo ls -la /var/lib/nordtronics/
drwxrws---  2 wildfire-ingest wildfire-data  4096 Sep 25 21:18 .
-rw-rw----  1 wildfire-ingest wildfire-data 40960 Sep 25 21:18 wildfire.db
-rw-rw----  1 wildfire-ingest wildfire-data 32768 Sep 25 21:20 wildfire.db-shm
-rw-rw----  1 wildfire-ingest wildfire-data 24752 Sep 25 21:20 wildfire.db-wal
$ grep -n "UMask" /etc/systemd/system/wildfire-ingest.service /etc/systemd/system/wildfire-api.service
/etc/systemd/system/wildfire-ingest.service:32:UMask=0007
/etc/systemd/system/wildfire-api.service:29:UMask=0007
```

`UMask=0007` was confirmed present in both units (it was already there at the base commit, exactly
as 0063's note 1 says). The `-shm`/`-wal` sidecars are now the database's own 0660 mode and
`wildfire-data` group, whoever touches them; the `-wal` file grew to 24 kB of real WAL traffic
while the worker was up, so the mode is not a one-off snapshot.

### Step 4 — restart: the crash loop is over

```console
$ systemctl show -p NRestarts wildfire-ingest        # before
NRestarts=1410
$ sudo systemctl restart wildfire-ingest
$ systemctl is-active mosquitto wildfire-api wildfire-ingest
active
active
active
$ systemctl show -p NRestarts -p ActiveEnterTimestamp -p SubState wildfire-ingest
NRestarts=0
SubState=running
ActiveEnterTimestamp=Fri 2026-09-25 21:18:55 CEST
$ curl -sS http://127.0.0.1:8000/healthz
{"status":"ok","database":"ok","schema_version":1,"generated_utc":"2026-09-25T19:18:07Z"} [http 200]
```

`NRestarts=0` on its own only means the counter reset on an explicit restart, so here is the
stronger check — no restart has been *scheduled* at all since the fix, and no restart failure is
in the journal:

```console
$ sudo journalctl -u wildfire-ingest --since "21:18:55" --no-pager | grep -cE "Scheduled restart job|Failed with result"
0
$ systemctl status wildfire-ingest --no-pager | head -5
     Active: active (running) since Fri 2026-09-25 21:18:55 CEST; 1min 52s ago
```

The `attempt to write a readonly database` line in the journal at 21:17:34 is the last one, from
before the fix. `/healthz` answered `500` for the few seconds between the first worker start and
its schema write (the preseeded database) and `200` after, as the runbook predicts.

### Step 5 — nginx API proxy and the end-to-end test

The runbook's step-9 server block was appended verbatim to the existing nginx config
(`/etc/nginx/sites-available/nordtronics.io`, backup `nordtronics.io.bak-0064`):

```console
$ sudo nginx -t
nginx: configuration file /etc/nginx/nginx.conf test is successful
$ sudo systemctl reload nginx
$ curl -sS --resolve api.nordtronics.io:443:127.0.0.1 https://api.nordtronics.io/healthz
{"status":"ok","database":"ok","schema_version":1,"generated_utc":"2026-09-25T19:18:23Z"} [http 200]
```

Then one published message through the real path, exactly as step 10 prescribes (TLS, system CA
store, as `deploy`, node-01 credentials read from `/etc/nordtronics/mqtt-credentials.env` and not
printed):

```console
$ PAYLOAD='{"node_id":"node-01","pm25":5.0,"temperature_c":20.0,"humidity_pct":40.0,"battery_v":3.9,"observed_utc":"2026-09-25T19:18:44Z"}'
$ mosquitto_pub -h mqtt.nordtronics.io -p 8883 --cafile /etc/ssl/certs/ca-certificates.crt \
    -u node-01 -P "$NODE01_PW" -q 1 -t nordtronics/wildfire/node-01/telemetry -m "$PAYLOAD" -d
Client null sending CONNECT
Client null received CONNACK (0)
Client null sending PUBLISH (d0, q1, r0, m1, 'nordtronics/wildfire/node-01/telemetry', ... (127 bytes))
Client null received PUBACK (Mid: 1, RC:0)
Client null sending DISCONNECT
```

Broker side and worker side of the same message:

```console
Sep 25 21:18:44 mosquitto: New client connected from 127.0.0.1:49552 as auto-200666A2-… (p2, c1, k60, u'node-01').
Sep 25 21:18:44 python[31195]: 2026-09-25 21:18:44,390 INFO wildfire.ingest stored node-01 pm25=5.0 temp=20.0 rh=40.0 batt=3.9
```

It landed in SQLite (read-only URI, so this probe cannot leave root-owned sidecars behind):

```console
$ sudo -u wildfire-ingest /opt/nordtronics/venv/bin/python -c "…mode=ro…"
nodes: [('node-01', '2026-09-25T19:18:44Z', '2026-09-25T19:18:44Z', 1, 5.0, 20.0, 40.0, 3.9)]
readings: [(1, 'node-01', '2026-09-25T19:18:44Z', '2026-09-25T19:18:44Z', 5.0, 20.0, 40.0, 3.9, 'nordtronics/wildfire/node-01/telemetry')]
```

and it is served by the public HTTPS API — read from this laptop, i.e. off the VPS, through
Cloudflare:

```console
$ curl -sS https://api.nordtronics.io/healthz
{"status":"ok","database":"ok","schema_version":1,"generated_utc":"2026-09-25T19:20:59Z"}   [http 200]

$ curl -sS https://api.nordtronics.io/v1/nodes
{"nodes": [{"node_id": "node-01", "first_seen_utc": "2026-09-25T19:18:44Z",
 "last_seen_utc": "2026-09-25T19:20:01Z", "reading_count": 2, "age_seconds": 58,
 "status": "ok", "latest": {"pm25": 7.5, "temperature_c": 21.0, "humidity_pct": 41.0,
 "battery_v": 3.8}}], "count": 1, "stale_after_seconds": 900,
 "generated_utc": "2026-09-25T19:20:59Z"}

$ curl -sS "https://api.nordtronics.io/v1/nodes/node-01/readings?limit=5"
{"node_id": "node-01", "count": 2, "limit": 5, "since": null, "readings": [
 {"recorded_utc": "2026-09-25T19:20:01Z", "observed_utc": null, "pm25": 7.5,
  "temperature_c": 21.0, "humidity_pct": 41.0, "battery_v": 3.8},
 {"recorded_utc": "2026-09-25T19:18:44Z", "observed_utc": "2026-09-25T19:18:44Z",
  "pm25": 5.0, "temperature_c": 20.0, "humidity_pct": 40.0, "battery_v": 3.9}]}
```

(`reading_count: 2` because I published a second, control message during the ACL check below; its
payload omitted `observed_utc`, hence the `null`. Nothing was deleted.)

### Negative checks from step 10

```console
$ mosquitto_pub -h mqtt.nordtronics.io -p 8883 --cafile /etc/ssl/certs/ca-certificates.crt \
    -t nordtronics/wildfire/node-01/telemetry -m x -q 1
Connection error: Connection Refused: not authorised.
$ timeout 5 mosquitto_pub -h 89.117.21.105 -p 1883 -t x -m y
Error: Connection refused
```

The cross-node ACL was checked with an independent subscriber holding the ingest credentials,
because a denial is invisible to the publisher under MQTT 3.1.1 (a denied PUBLISH still gets
`PUBACK RC:0` — the client is told nothing):

```console
$ mosquitto_sub -h mqtt.nordtronics.io -p 8883 --cafile $CA -u wildfire-ingest -P "$INGEST_PW" \
    -q 1 -t 'nordtronics/wildfire/+/telemetry' -v      # 12s window
nordtronics/wildfire/node-01/telemetry {"node_id":"node-01","pm25":7.5,…}   # control, as node-01
                                                        # node-01 → node-02 topic: nothing arrived
$ sudo -u wildfire-ingest … "select node_id, reading_count from nodes"
nodes: [('node-01', 2)]                                 # no node-02 row was ever created
```

### Declarations (deviations, corrections, open decisions)

1. **The VPS source tree was moved to the 0063 tip** (`git fetch`/`git checkout`/`rsync -a --delete`
   into `/opt/nordtronics/backend/`, the step-12 update path). It is required by this task's own
   steps: step 4 installs `renew-hook.sh`, which step 1/4 changed, and step 8 installs the two
   revised units. No source was *edited* on the VPS. The plain `sudo rsync -a` left the tree
   `deploy`-owned; I restored `root:root` to match the ownership table in `DEPLOY.md`.
2. **0062's WAL mechanism correction is honoured**: both units already carried `UMask=0007`, and
   the 0063 unit diff is comment-only. The operative change is 0660 on `wildfire.db`. Nothing in
   the reply attributes the fix to a umask.
3. **The runbook's plaintext-1883 check does not test UFW when run on the VPS itself.** It returns
   `Connection refused` (an RST), not the timeout the runbook expects, because UFW accepts
   loopback-interface traffic and a host connecting to its own public IP routes via `lo`. Run
   off-host it behaves as documented. From this laptop:

   ```console
   port 1883: TimeoutError: timed out (6.01s)      # dropped
   port 8883: CONNECTED (0.18s)  port 443: CONNECTED  port 80: CONNECTED
   ```

   So UFW is correct; the runbook's expected output is only reachable from another host. A
   documentation fix (address the host from outside, or expect "refused" on-host) is a follow-up —
   I made no source edit.
4. **`deploy` cannot read the pinned bundle** (0640 `root:wildfire-data`, and `deploy` is not in
   `wildfire-data`), so the bundle-anchoring probe ran as `wildfire-ingest`; the step-10
   operator-side publish uses the system CA store exactly as 0063's note 2 directs. The open
   decision from 0063 (a read ACL or 0644 for `deploy`) is **not taken** — Juno's call.
5. **The api vhost is the runbook's block verbatim**, appended to the existing
   `/etc/nginx/sites-available/nordtronics.io`; it does not include that site's
   `nordtronics-tls.conf` / `nordtronics-headers.conf` snippets, because the runbook's block does
   not. Adding them is a one-line change if you want the API to carry the same HSTS/CSP headers as
   the apex.
6. **Pre-existing, not fixed:** the broker's reload log shows
   `Warning: File /etc/mosquitto/passwd owner is not mosquitto … Future versions will refuse to
   load this file` (same for `/etc/mosquitto/acl`). The runbook installs both `root:mosquitto`
   0640, so this is a forward-compatibility risk on a mosquitto upgrade, not a defect introduced
   here. Out of scope for this task; flagging it rather than changing ownership on my own
   authority.
7. **Secrets**: no password appears in this reply. The publisher credentials were read from
   `/etc/nordtronics/mqtt-credentials.env` into shell variables and used with `-P "$PW"`. The
   `deploy` password is untouched. Nothing was written to `handoff/outbox/`.
8. `/etc/nordtronics/ingest.env` was edited with `sed -i`, which preserves its 0600 root ownership
   and its existing password line.

### Scope

VPS changes are exactly those listed in `proof.applied_to_vps`, i.e. the ones the 0063 runbook
directs, plus the nginx vhost of step 9 with a backup copy taken first. Nothing was written to
`/opt/nordtronics/venv` (requirements unchanged in 0063, so no `pip install` was needed). The
blocked 0059/0062 files were not touched: I did not edit them, their `iteration`, or their notes.
The preseeded database is intact (2 readings, 1 node) and no data was deleted or rewritten.
