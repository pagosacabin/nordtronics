---
task_id: "0062"
protocol_version: 1.0.0
status: superseded  # superseded by 0064 (verified)
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

  ADDENDUM 2026-09-25 (later tick, ~18:20 CEST): the blocking failure has MOVED
  to a THIRD, independent defect (C). The sentence above — that the worker "now
  fails later, in the TLS handshake" — stopped being true at 17:18:45 CEST and
  is corrected here.

  C) WAL sidecar ownership: wildfire-ingest can no longer write the database,
     and now dies BEFORE the TLS handshake. Every restart since 17:18:45 CEST:
       File "/opt/nordtronics/backend/ingest/ingest/worker.py", line 82, in open_database
         self.conn = db_module.init_db(self.config.db_path)
       File "/opt/nordtronics/backend/common/db.py", line 53, in init_db
         conn.executescript(SCHEMA_PATH.read_text(encoding="utf-8"))
       sqlite3.OperationalError: attempt to write a readonly database
       wildfire-ingest.service: Main process exited, code=exited, status=1/FAILURE

     Journal counts (journalctl -u wildfire-ingest, 14:17:20 -> 18:19:34; 1420
     "Started" lines, one restart per RestartSec=10s; nothing else appears):
        1050  PermissionError on chain.pem      14:17:20 -> 17:16:32  (what 0061 fixed)
          12  ssl.SSLCertVerificationError      17:16:42 -> 17:18:35  (defects A/B)
         358  attempt to write a readonly db    17:18:45 -> now       (defect C)

     Live file state (sudo ls -la /var/lib/nordtronics/):
       -rw-r----- wildfire-ingest wildfire-data 40960  wildfire.db
       -rw-r----- wildfire-api    wildfire-data 32768  wildfire.db-shm
       -rw-r----- wildfire-api    wildfire-data     0  wildfire.db-wal
     The -shm/-wal sidecars are owned by wildfire-api at mode 0640, so group
     wildfire-data gets READ only and wildfire-ingest (uid 110, group member but
     not owner) cannot write the WAL — hence "attempt to write a readonly
     database". WAL readers AND writers need write access to -shm (db.py's own
     docstring says so), so whichever process creates the sidecars locks the
     other one out.

     Why 0640 and not group-writable: SQLite's unix VFS creates the database and
     its sidecars with base mode 0644, and a umask can only CLEAR bits, so
     0644 & ~0007 = 0640. The units' UMask=0007 ("group-rw for everything the
     worker creates") and DEPLOY.md step 6 ("every file the ingest worker
     creates — including the -wal and -shm sidecars — stays in the
     wildfire-data group, which is how the API reads them") both assume a
     group-write bit that is never set.

     Trigger: wildfire-api opens the DB read-only per request
     (api/api/app.py read_connection() -> db.connect(read_only=True)) and
     /healthz reads schema_version through it, so ANY API request materialises /
     re-locks the api-owned sidecars. The API's first-ever DB touch was the
     GET /healthz at 17:18:35 CEST issued by this task's own verification; the
     worker's next restart (RestartSec=10s) at 17:18:45 was already locked out.
     Step 10's health check is what kills step 8's worker, and the lockout
     persists.

     Mechanism proven this tick, non-destructively, on a byte copy of the three
     live files (cp -a into /var/tmp, owners and modes preserved), as
     wildfire-ingest, BEGIN IMMEDIATE then rollback:
       copy as-is (api-owned 0640 sidecars) -> OperationalError: attempt to
         write a readonly database
       same copy, sidecars chmod 0660     -> write lock acquired
     The live files were not chmod'ed, chown'ed or written; the copy was deleted.

     Fix verified but NOT applied (a runbook decision, same class as A/B): the
     sidecars must be group-writable. Options, each changing DEPLOY.md and/or
     the units — (i) run both services under one uid; (ii) chmod 0660 the
     database and its sidecars at unit start (ExecStartPre as root); (iii) chmod
     them in db.py immediately after connect (each process can chmod only the
     files it owns — exactly the set that locks the other out). A umask change
     cannot work: the base mode has no group-write bit to preserve.

     C is independent of A and B and blocks harder — with A and B fixed the
     worker still cannot open the database. A was re-verified this tick as
     wildfire-ingest: `openssl s_client -connect 127.0.0.1:8883 -servername
     nordtronics.io -CAfile .../chain.pem -verify_return_error` ->
     "depth=3 CN = ISRG Root X2", "verify error:num=2:unable to get issuer
     certificate". B was not re-run this tick (the worker never reaches the
     handshake).

     Blind spot: /healthz answers {"status":"ok","database":"ok"} (HTTP 200)
     while the writer is locked out — it only exercises a reader.

     Nothing was changed this tick — no code, config, unit, ACL or runbook file.
     ingest.env still carries WILDFIRE_MQTT_HOST=127.0.0.1 and
     WILDFIRE_MQTT_CA=/etc/letsencrypt/live/nordtronics.io/chain.pem, and the
     VPS tree is still hermes/0061-ingest-ca-acl @ 9eba1cec9716f2. Steps 9 and 10
     still not run; no test reading published. Decision now owed: A/B (trust
     anchor + host) and C (how the two service users share the WAL sidecars).
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

---

## Reply addendum — the blocker moved to a THIRD defect (2026-09-25, later tick)

Still not staged, still no `proof` block: the task is not done, so there is no
success claim to make. The file stays in `active/` with `status: in_progress`.
The entry above is accurate as of 17:18:35 CEST — the ACL fix worked and the
worker really did reach the TLS handshake — but ten seconds later its first
blocking failure changed, and it has been a different one ever since. This
addendum records that, and corrects the one sentence above that is now stale
("now fails later, in the TLS handshake").

### The failure at step 8 now

`wildfire-ingest` no longer reaches the MQTT client at all. Every restart dies
in `open_database()`, before any TLS code runs:

```
$ sudo journalctl -u wildfire-ingest -o short-iso -n 22
2026-09-25T18:18:53+02:00 python[24380]:  File "/opt/nordtronics/backend/ingest/ingest/worker.py", line 82, in open_database
2026-09-25T18:18:53+02:00 python[24380]:    self.conn = db_module.init_db(self.config.db_path)
2026-09-25T18:18:53+02:00 python[24380]:  File "/opt/nordtronics/backend/common/db.py", line 53, in init_db
2026-09-25T18:18:53+02:00 python[24380]:    conn.executescript(SCHEMA_PATH.read_text(encoding="utf-8"))
2026-09-25T18:18:53+02:00 python[24380]: sqlite3.OperationalError: attempt to write a readonly database
2026-09-25T18:18:53+02:00 systemd[1]: wildfire-ingest.service: Failed with result 'exit-code'.

$ systemctl show wildfire-ingest -p NRestarts -p SubState -p Result --value
363
auto-restart
exit-code
```

### The three failures in order, with counts

The journal is a clean sequence — 1420 `Started` lines, one restart every 10s
(`RestartSec=10s`), every start ending in exactly one of three errors, to within
10 seconds of the transition:

| # | Error | Window (CEST) | Starts |
|---|---|---|---|
| 1 | `PermissionError` on chain.pem | 14:17:20 -> 17:16:32 | 1050 |
| 2 | `ssl.SSLCertVerificationError` (unable to get issuer) | 17:16:42 -> 17:18:35 | 12 |
| 3 | `sqlite3.OperationalError: attempt to write a readonly database` | 17:18:45 -> now | 358 |

Failure 1 is what 0061 fixed — the worker ran the pre-0061 tree until this task
synced `/opt/nordtronics/backend` at 17:16:3x. Failure 2 is defects A/B from the
entry above, and it lasted 12 restarts. Failure 3 is new and is the current
first blocking failure.

### Defect C — evidence

```
$ sudo ls -la /var/lib/nordtronics/
-rw-r----- 1 wildfire-ingest wildfire-data 40960 Sep 25 14:17 wildfire.db
-rw-r----- 1 wildfire-api    wildfire-data 32768 Sep 25 18:17 wildfire.db-shm
-rw-r----- 1 wildfire-api    wildfire-data     0 Sep 25 17:18 wildfire.db-wal
```

The `-shm`/`-wal` sidecars belong to `wildfire-api`, mode `0640`. Group
`wildfire-data` therefore has **read** on them, and `wildfire-ingest` (uid 110,
member of the group but not the owner) cannot write the WAL — so SQLite answers
"attempt to write a readonly database" for what is meant to be the *writer*.
`db.py`'s own docstring states the requirement ("WAL readers still need write
access to the `-shm`/`-wal` sidecars"), and the units' `UMask=0007` comment
assumes it is satisfied; it is not. Sharing a *group* is not enough, because
SQLite's unix VFS creates the database and its sidecars with base mode `0644`,
and a umask can only clear bits: `0644 & ~0007 = 0640`. "Group-rw" was never
going to happen, in either direction — whichever process creates the sidecars
locks the other one out.

**Trigger, and it is step 10 doing it to step 8:** the API opens the database
read-only on *every* request (`api/api/app.py` -> `read_connection()` ->
`db.connect(read_only=True)`, and `/healthz` reads `schema_version` through it).
The API's first-ever DB touch in the journal is the `GET /healthz` at 17:18:35 —
the health check this task ran as part of its verification. The worker's next
restart, `RestartSec=10s` later at 17:18:45, was already locked out. Repeated
`GET /healthz` calls keep the sidecars api-owned (`-shm` mtime moved to 18:17 on
the next check).

### Mechanism proven this tick — non-destructively

On a byte copy of the three live files (`cp -a` into `/var/tmp`, owners and
modes preserved), as `wildfire-ingest`, `BEGIN IMMEDIATE` then rollback:

```
copy as-is (api-owned 0640 sidecars) -> OperationalError: attempt to write a readonly database
same copy, sidecars chmod 0660      -> write lock acquired
```

The live database was never chmod'ed, chown'ed or written; the copy was deleted.
`/var/lib/nordtronics` itself *is* writable by `wildfire-ingest`
(`sudo -u wildfire-ingest touch` on the directory succeeds), which is what
isolates the failure to the sidecar files rather than the directory.

### The fix, verified but not applied

Making the sidecars group-writable is sufficient (probe above). Applying it
means choosing between, e.g.: (i) run both services under one uid; (ii) `chmod
0660` the database and sidecars at unit start (`ExecStartPre` as root);
(iii) `chmod` them in `db.py` right after connect — each process can only chmod
files it owns, which is exactly the set that locks the other out. A umask change
alone cannot fix it. All three change DEPLOY.md step 6 / the unit files, i.e. the
runbook, so none was applied — the same call this task left to you for A and B.

Defect C also changes the order of the decision: it is independent of A and B and
blocks harder, so fixing A and B as previously proposed would still leave the
worker unable to open its database. A was re-verified as still true this tick as
`wildfire-ingest` (`openssl s_client -CAfile .../chain.pem -verify_return_error`
-> `depth=3 CN = ISRG Root X2`, `verify error:num=2:unable to get issuer
certificate`); B was not re-run, because the worker no longer reaches the
handshake.

### Also worth knowing

`curl http://127.0.0.1:8000/healthz` returns `200` with
`{"status":"ok","database":"ok", ...}` while the writer is locked out — the
endpoint only exercises a reader, so it cannot see this failure. Step 10's check
3 would pass on a dead pipeline.

### Not done, and why

Steps 9 (nginx) and 10 (end-to-end) were not run: step 8 is the first blocking
failure and it is not transient. No test reading was published, so no synthetic
row exists to delete. Nothing on the VPS was changed this tick — no code, config,
unit, ACL, database or runbook file; `ingest.env` still carries
`WILDFIRE_MQTT_HOST=127.0.0.1` and `WILDFIRE_MQTT_CA=/etc/letsencrypt/live/
nordtronics.io/chain.pem`, and the tree is still
`hermes/0061-ingest-ca-acl @ 9eba1cec9716f2`.

