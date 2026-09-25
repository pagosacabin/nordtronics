---
task_id: "0062"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  BLOCKED — a SECOND, independent runbook defect, hit at step 8 after 0061's CA
  permission fix worked. Declared, not improvised around; awaiting a decision.

  Steps 1-2 of this task are DONE (details and outputs in the reply body):
  the VPS tree is synced to hermes/0061-ingest-ca-acl @ 9eba1cec9716f2 and the
  new step-4 ACL pair is applied. The original failure is gone — the worker now
  reads the CA file. `sudo -u wildfire-ingest cat .../chain.pem | head -1`
  prints `-----BEGIN CERTIFICATE-----`, exactly as the new step 4 requires.

  What still blocks step 8 is the CA *path*, not its *permissions*:

  A) WILDFIRE_MQTT_CA=chain.pem cannot validate the broker's chain. chain.pem
     (and fullchain.pem) end at a CROSS-SIGNED ISRG Root X2 whose issuer is
     ISRG Root X1, which is in neither file, so no path can be built to a
     self-signed trust anchor:
       openssl s_client -connect 127.0.0.1:8883 -servername nordtronics.io \
         -CAfile .../chain.pem -verify_return_error
       depth=3 CN = ISRG Root X2
       verify error:num=2:unable to get issuer certificate
       issuer= CN = ISRG Root X1
     Same error with -CAfile fullchain.pem. The worker therefore dies with
       ssl.SSLCertVerificationError: [SSL: CERTIFICATE_VERIFY_FAILED]
       certificate verify failed: unable to get issuer certificate
     after logging "database ready" and "connecting to 127.0.0.1:8883".

  B) Independently of A, the host in step 7 breaks hostname verification. The
     certificate's SANs are DNS only (nordtronics.io, www., api., mqtt.) with
     no IP SAN, but WILDFIRE_MQTT_HOST=127.0.0.1 and paho uses Python's default
     SSLContext (check_hostname=True), so even with a CA bundle that DOES build
     the path the connection fails:
       ca=/etc/ssl/certs/ca-certificates.crt -> SSLCertVerificationError:
       certificate verify failed: IP address mismatch, certificate is not valid
       for '127.0.0.1'
     DEPLOY.md step 7's note "so the certificate name does not have to resolve
     here" is what invited 127.0.0.1; under Python 3.12 + paho it is wrong.
     Both A and B must change for the worker to connect.

  Investigated fix, verified by probe but NOT applied (a runbook decision, and
  DEPLOY.md step 7 is yours to change):
    WILDFIRE_MQTT_HOST=mqtt.nordtronics.io   (resolves to 89.117.21.105, is in
                                              the cert SANs, UFW already allows
                                              8883, broker listens 0.0.0.0:8883)
    WILDFIRE_MQTT_CA=<a bundle containing the self-signed ISRG Root X1>, e.g.
                     /etc/ssl/certs/ca-certificates.crt, or unset so the worker
                     takes its existing system-trust-store path.
  A read-only probe running the worker's own connect() with the ingest
  credentials (client_id probe-*, clean_session=True — no production session
  touched, nothing published) returned:
    host=mqtt.nordtronics.io + system bundle : CONNECTED (on_connect rc=0)
    host=mqtt.nordtronics.io + chain.pem     : FAILED unable to get issuer
                                               certificate
    host=127.0.0.1 + system bundle           : FAILED IP address mismatch
  i.e. the two changes together connect; either one alone does not.

  No config, unit, code or runbook file was changed this run. ingest.env,
  DEPLOY.md, the worker and the ACL set are exactly as the runbook leaves them.
  Steps 9 (nginx) and 10 (end-to-end) not run: step 8 is the first blocking
  failure and the ingest worker is still crash-looping (4 restarts in 40s,
  Result=exit-code, status=1) — now on the cert failure instead of EACCES.

  Observation, not a failure: 0061's new ACL grant on /etc/letsencrypt/live is
  what let the worker read chain.pem, but under either candidate fix the CA
  becomes a system-store bundle, so that grant (and step 4's ingest confirm)
  would no longer be load-bearing for the worker. Also unverified: mosquitto's
  own `cafile /etc/letsencrypt/live/nordtronics.io/chain.pem` would hit defect A
  if client certificates were ever required (require_certificate is unset, so
  nothing verifies against it today).
---

# 0062 — Resume the backend deploy from step 8 (CA gap fixed)

## Context

0059 is BLOCKED in `mailbox/active/0059-deploy-backend.md` at runbook step 8:
the ingest worker crash-looped because `wildfire-ingest` could not read
`/etc/letsencrypt/live/nordtronics.io/chain.pem`. That gap is now fixed on
branch `hermes/0061-ingest-ca-acl @
9eba1cec9716f22779e6014190b6ef93b1082ad2` (CI green, run 36146557608):
DEPLOY.md step 4 grants `wildfire-ingest` the same cert ACLs as `mosquitto`,
and the worker logs a clear FATAL on an unreadable CA.

VPS state left by 0059: mosquitto active on TLS port 8883; wildfire-api active
on 127.0.0.1:8000; wildfire-ingest in a crash loop; node-01 MQTT credentials
exist on the VPS only; the SQLite database is initialized. Steps 9 (nginx/API
public proxy) and 10 (end-to-end test) were not run.

## Task

Resume the deploy from the fixed branch tip
(`hermes/0061-ingest-ca-acl @ 9eba1cec9716f22779e6014190b6ef93b1082ad2`):

1. Sync the VPS checkout (`/opt/nordtronics/backend`) to that branch SHA.
2. Re-run the NEW step-4 lines (the `wildfire-ingest` setfacl pair and the
   new confirm: `sudo -u wildfire-ingest cat
   /etc/letsencrypt/live/nordtronics.io/chain.pem | head -1` must print
   `-----BEGIN CERTIFICATE-----`).
3. Runbook steps 8 through 10 to completion: start/enable wildfire-ingest,
   wire the nginx/API public proxy, and run the end-to-end test —
   publish one telemetry message as node-01 over TLS on port 8883, confirm
   the row lands in SQLite, and confirm the reading comes back through the
   public HTTPS API.

Do not touch 0059's active file; this task resumes where it blocked.

## Success criteria

1. `wildfire-ingest` is active with no crash loop.
2. End-to-end passes: MQTT (TLS 8883, node-01) -> SQLite row -> HTTPS API
   returns the reading.

## Constraints

- Cost bound: deepseek-flash only, off-peak hours.
- Never paste the node-01 credentials or any other secret into the reply.

## Proof

`systemctl is-active` output for all three services plus the exact commands
and output of the end-to-end verification (mosquitto_pub, sqlite3 query,
curl of the HTTPS endpoint).

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: what was resumed, the service states, and the end-to-end test
evidence.

---

## Reply — steps 1-2 done, step 8 BLOCKED on a second defect (2026-09-25)

Not staged: the task is not done, so there is no `proof` block and no success
claim. The file stays in `active/` with `status: in_progress`, and the blocking
failure plus the verified-but-unapplied fix are declared in the front-matter
`notes` above — per 0059's constraint "if a step fails, stop at the first
blocking failure, report the exact output, and do not improvise around it".

### Task step 1 — VPS tree synced to the fixed branch tip: DONE

```
cd ~/nordtronics-src
git fetch -q origin
git checkout -q hermes/0061-ingest-ca-acl
git reset -q --hard 9eba1cec9716f22779e6014190b6ef93b1082ad2
HEAD=9eba1cec9716f22779e6014190b6ef93b1082ad2
BRANCH=hermes/0061-ingest-ca-acl
sudo rsync -a --delete ~/nordtronics-src/backend/ /opt/nordtronics/backend/
sudo chown -R root:root /opt/nordtronics/backend
sudo /opt/nordtronics/venv/bin/pip install -q -r .../ingest/requirements.txt -r .../api/requirements.txt
imports ok
```

`git rev-parse HEAD` on the VPS re-checked after the run still reads
`9eba1cec9716f22779e6014190b6ef93b1082ad2`; `/opt/nordtronics/backend/ingest/
ingest/worker.py` is the 0061 version (it contains the new FATAL-on-EACCES
branch), so the tree the services run is the fixed one. Carried over from
0059's deviation (b): `chown -R root:root /opt/nordtronics/backend`, without
which `rsync -a` leaves the deploy-owned clone's ownership in place.

### Task step 2 — the new step-4 lines: DONE, and the 0061 fix works

```
sudo setfacl -R  -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive
sudo setfacl -R -d -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive

sudo -u wildfire-ingest cat /etc/letsencrypt/live/nordtronics.io/chain.pem | head -1
-----BEGIN CERTIFICATE-----

sudo -u mosquitto cat /etc/letsencrypt/live/nordtronics.io/privkey.pem | head -1
-----BEGIN PRIVATE KEY-----
```

`getfacl /etc/letsencrypt/live/nordtronics.io` now lists both
`user:mosquitto:r-x` and `user:wildfire-ingest:r-x`, plus the matching
`default:` entries so renewals inherit them. 0059's `PermissionError` is gone:
the worker reaches the CA file and now fails *later*, in the TLS handshake.
That confirms 0061's permission fix does what it claims.

### Task step 3 — step 8: BLOCKED, and it is not a transient

`wildfire-ingest` was restarted after `daemon-reload` and crash-loops on a
certificate-verification error. It is not stable — `systemctl is-active` can
read `active` mid-loop because the process lives ~1s before exiting, so the
restart counter is the honest measure:

```
$ systemctl show wildfire-ingest -p NRestarts --value     # t0      10
$ sleep 40
$ systemctl show wildfire-ingest -p NRestarts --value     # t+40s   14   (delta 4)
$ systemctl show wildfire-ingest -p SubState -p Result -p ExecMainStatus --value
auto-restart exit-code 1
```

Journal, current loop:

```
17:18:24 INFO wildfire.ingest database ready at /var/lib/nordtronics/wildfire.db (schema v1)
17:18:24 INFO wildfire.ingest connecting to 127.0.0.1:8883
17:18:24 ssl.SSLCertVerificationError: [SSL: CERTIFICATE_VERIFY_FAILED]
         certificate verify failed: unable to get issuer certificate (_ssl.c:1000)
17:18:24 systemd[1]: wildfire-ingest.service: Main process exited, code=exited, status=1/FAILURE
```

Two independent defects, both in DEPLOY.md step 7's values, neither reachable by
the ACL fix. Full analysis and quoted `openssl s_client` / paho output are in the
front-matter `notes`; the short version:

- **A — the CA file cannot build a path.** `chain.pem` and `fullchain.pem` end at
  a cross-signed ISRG Root X2 issued by ISRG Root X1, which neither file
  contains, so verification fails `unable to get issuer certificate`. The
  self-signed ISRG Root X1 is present in the system store, which is why a bundle
  of it verifies the same chain OK.
- **B — the host breaks hostname verification.** The certificate has DNS-only
  SANs, and `WILDFIRE_MQTT_HOST=127.0.0.1`, so Python/paho's default
  `check_hostname=True` rejects it: `IP address mismatch, certificate is not
  valid for '127.0.0.1'`. Verified to fail even with a CA bundle that does build
  the path.

A read-only probe using the worker's own `connect()` with the ingest
credentials — `client_id` `probe-*`, `clean_session=True`, nothing published, no
production session touched — shows both changes are needed and are sufficient:

```
host=mqtt.nordtronics.io + system bundle : CONNECTED (on_connect rc=0)
host=mqtt.nordtronics.io + chain.pem     : FAILED unable to get issuer certificate
host=127.0.0.1          + system bundle  : FAILED IP address mismatch
```

`mqtt.nordtronics.io` resolves to `89.117.21.105` from the VPS, is in the cert
SANs, UFW already permits 8883, and the broker listens on `0.0.0.0:8883`.

### What was not done, and why

- **No fix applied.** Changing `WILDFIRE_MQTT_CA` and `WILDFIRE_MQTT_HOST`
  changes DEPLOY.md step 7 (the runbook) and the ingest.env contract; 0061
  deliberately aimed step 4's ACL at `chain.pem`, so which file the worker should
  trust is a decision for you, not a repair to improvise. `ingest.env`,
  `DEPLOY.md`, the worker, the units and the ACL set are untouched by anything
  other than the steps above.
- **Steps 9 (nginx) and 10 (end-to-end) not run** — step 8 is the first blocking
  failure, and step 10's MQTT round-trip cannot pass while the ingest worker
  cannot connect. For the record, `curl -o /dev/null -w '%{http_code}'`: loopback
  `http://127.0.0.1:8000/healthz` -> **200**; public
  `https://api.nordtronics.io/healthz` -> **404** (nginx has no
  `api.nordtronics.io` server block yet, so the default site answers).
- **No test reading published**, so no synthetic row exists to delete and that
  constraint is untouched.

### Service states at the stop point

```
$ systemctl is-active mosquitto wildfire-ingest wildfire-api
active
activating        # crash loop: NRestarts +4 in 40s, Result=exit-code
active
```

`mosquitto` active on the 8883 TLS listener (unchanged since 0059); `wildfire-api`
active on 127.0.0.1:8000 (loopback health check 200); `wildfire-ingest`
crash-looping on the certificate error above. Success criterion 1
(`wildfire-ingest` active, no crash loop) does **not** hold, and criterion 2 is
not attempted. Nothing runs as root or `deploy`; step 8's units are still the
ones installed at 0059 (this branch changed `worker.py`, `DEPLOY.md` and the
ingest tests, not `mosquitto/` or `systemd/`).

### Corner left marked, not painted over

0061's ACL grant is now what allows the worker to read `chain.pem` — but under
either candidate fix the CA becomes a system-store bundle, which would make
step 4's ingest ACL and confirm line no longer load-bearing for the worker. Both
fixes also leave `mosquitto`'s own `cafile .../chain.pem` (used only for client
certificates, and `require_certificate` is unset, so nothing verifies against it
today) carrying defect A for any future client-cert setup.

No secret value appears in this file: the node-01 credentials were read only
from `/etc/nordtronics/mqtt-credentials.env` on the VPS, and only the ingest
worker's own `connect()` used the MQTT password.

