---
task_id: "0064"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
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
