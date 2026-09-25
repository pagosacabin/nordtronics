---
task_id: "0062"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
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
