---
task_id: "0061"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
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
