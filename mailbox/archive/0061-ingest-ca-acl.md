---
task_id: "0061"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0061-ingest-ca-acl
    sha: 9eba1cec9716f22779e6014190b6ef93b1082ad2
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36146557608
  - files:
      - backend/DEPLOY.md
      - backend/ingest/ingest/worker.py
      - backend/ingest/tests/test_worker.py
notes: |
  Both requested changes made on a branch off the 0056 tip
  (697ab6647ee3faba3114d111d4a51f93c8a29d9d); CI green at the branch tip. No
  VPS change, and 0059's active file was not touched.

  1. DEPLOY.md step 4: the identical setfacl pair now runs for wildfire-ingest
     after the two mosquitto lines, and the step's confirm section gained
     `sudo -u wildfire-ingest cat .../chain.pem | head -1` (must print
     -----BEGIN CERTIFICATE-----).

  2. worker.build_client(): a CA file that exists but is unreadable now logs
     "FATAL: cannot read MQTT CA file <path>: permission denied" and exits 1 —
     no traceback. An absent CA keeps the previous system-trust-store fallback,
     and a readable CA is still passed to tls_set(ca_certs=...).

  Reproduced both ways on this machine through the real IngestConfig /
  IngestWorker path with a chmod 000 CA: the 0056 tip dies with
  PermissionError from paho's context.load_verify_locations(ca_certs); this
  branch emits the single CRITICAL line above and exits 1.

  Verification: local `cd backend && python -m pytest -q` -> 121 passed (the
  three new tests included); the CI test-report artifact for run 36146557608,
  downloaded and read, says "121 passed, 1 warning" with no skips.

  Note (not a deviation): the fatal path raises SystemExit(1) inside
  build_client, so the change stays where the task names it and main() is
  unchanged. Exit code 1 matches the failure mode systemd already reported.

  Not published to ntfy: the spec names no topic and the deliverable is source
  + CI evidence, not a CI-built artifact (README's definition of a build task).
---

# 0061 — Fix the ingest CA permission gap (branch only)

## Context

0059 is BLOCKED at runbook step 8: `backend/DEPLOY.md` step 4 grants the
`mosquitto` user ACL access to `/etc/letsencrypt/live` and
`/etc/letsencrypt/archive`, but step 7 points the ingest worker at the same
CA path (`WILDFIRE_MQTT_CA=.../chain.pem`) and grants `wildfire-ingest`
nothing — so the worker crash-loops with `PermissionError` on the CA file.
Contributing code defect: `backend/ingest/ingest/worker.py` (~line 96,
`build_client`) calls `Path.exists()`, which swallows ENOENT/ENOTDIR but
raises on EACCES — an unreadable CA produces a raw traceback instead of a
clear message.

Branch from the 0056 tip: `hermes/0056-backend-v1 @
697ab6647ee3faba3114d111d4a51f93c8a29d9d`. Name the new branch
`hermes/0061-ingest-ca-acl`.

## Task

On the new branch, make exactly these two changes:

1. `backend/DEPLOY.md` step 4: after the two `mosquitto` setfacl lines, add
   the identical pair for `wildfire-ingest`:
   ```
   sudo setfacl -R  -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive
   sudo setfacl -R -d -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive
   ```
   Extend the step's confirm block with:
   ```
   sudo -u wildfire-ingest cat /etc/letsencrypt/live/nordtronics.io/chain.pem | head -1
   ```
   (must print `-----BEGIN CERTIFICATE-----`).

2. `backend/ingest/ingest/worker.py` (`build_client`): if the configured CA
   file exists but is not readable, log a clear fatal message naming the
   path (e.g. `FATAL: cannot read MQTT CA file <path>: permission denied`)
   and exit non-zero — never a raw traceback. Keep the existing behavior
   for a CA file that is simply absent.

Push the branch and get CI green.

## Success criteria

1. CI is green on `hermes/0061-ingest-ca-acl`.
2. The diff to the 0056 tip contains only the two changes above (plus any
   test covering the new worker behavior).

## Constraints

- Branch only: make zero changes on the VPS in this task. The deploy resume
  is a separate follow-up task.
- Cost bound: deepseek-flash only, off-peak hours.
- Do not touch 0059's active file.

## Proof

Branch SHA on origin + Actions run URL showing green, both quoted in the
reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: the branch SHA, the CI run URL and result, and the diff stat.

---

## Reply — 0061 (2026-09-25): done

Branch `hermes/0061-ingest-ca-acl` off `hermes/0056-backend-v1` @
`697ab6647ee3faba3114d111d4a51f93c8a29d9d`.

- Branch tip on origin: `9eba1cec9716f22779e6014190b6ef93b1082ad2`
  (`git ls-remote --heads origin hermes/0061-ingest-ca-acl`).
- CI: https://github.com/pagosacabin/nordtronics/actions/runs/36146557608 —
  workflow `Backend`, conclusion `success`, `headSha`
  `9eba1cec9716f22779e6014190b6ef93b1082ad2`, i.e. equal to the tip above.
  Both jobs green: `Ingest + API tests`, `Mosquitto config check`.
- Diff to the 0056 tip (only the two changes plus tests):

```
 backend/DEPLOY.md                   | 14 +++++++
 backend/ingest/ingest/worker.py     | 20 +++++++++-
 backend/ingest/tests/test_worker.py | 75 +++++++++++++++++++++++++++++++++++++
 3 files changed, 107 insertions(+), 2 deletions(-)
```

### 1. `backend/DEPLOY.md` step 4 — ACL for the ingest worker

Added directly after the two `mosquitto` lines, with a comment saying why:

```bash
sudo setfacl -R  -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive
sudo setfacl -R -d -m u:wildfire-ingest:rX /etc/letsencrypt/live /etc/letsencrypt/archive
```

The step's confirm section now also carries:

```bash
sudo -u wildfire-ingest cat /etc/letsencrypt/live/nordtronics.io/chain.pem | head -1
```

which the runbook states must print `-----BEGIN CERTIFICATE-----`.

### 2. `backend/ingest/ingest/worker.py` — an unreadable CA fails loudly

`build_client()` no longer lets EACCES escape `tls_set()` as a raw traceback.
Both variants below were run here through the real `IngestConfig` /
`IngestWorker` code path with a `chmod 000` `chain.pem` (no broker involved):

BEFORE, at the 0056 tip:

```
  File "/opt/nordtronics/backend/ingest/ingest/worker.py", line 97, in build_client
    client.tls_set(ca_certs=self.config.mqtt_ca_file)
  File ".../paho/mqtt/client.py", line 1293, in tls_set
    context.load_verify_locations(ca_certs)
PermissionError: [Errno 13] Permission denied
```

AFTER, this branch:

```
2026-09-25 08:17:54,394 CRITICAL wildfire.ingest FATAL: cannot read MQTT CA file /…/chain.pem: permission denied
[repro] SystemExit code=1
```

Behaviour kept for the other two cases: a CA file that is absent still falls
back to the system trust store with the existing warning, and a readable CA is
still handed to `tls_set(ca_certs=...)`.

### Tests

`backend/ingest/tests/test_worker.py` gains three cases:

| Test | Asserts |
|---|---|
| `test_build_client_loads_a_readable_ca_file` | `tls_set(ca_certs=<path>)` is what gets called |
| `test_build_client_falls_back_when_the_ca_file_is_missing` | absent CA: no raise, empty `tls_set()` (unchanged behaviour) |
| `test_build_client_stops_on_an_unreadable_ca_file` | `SystemExit` with `code == 1` and the CRITICAL line naming the path |

The EACCES test builds the condition with a real `chmod 000` file and skips
only if the process can read anything regardless of mode (root).

### Verification performed

- `cd backend && python -m pytest -q` (venv built from
  `backend/requirements-dev.txt`, paho 2.1.0, pytest 8.4.2) → `121 passed`.
  `python -m compileall -q backend` (the CI step) → clean.
- CI test-report artifact downloaded and read: `121 passed, 1 warning` — no
  skip count, so the EACCES test really executed in CI.
- Tip equality checked on both sides: `git ls-remote` and
  `gh run view 36146557608 --json headSha`.

### Scope

Nothing was changed on the VPS; no command was run against 89.117.21.105 in
this task. 0059's active file was not modified. No secret value appears in the
diff, the commit, or this reply.
