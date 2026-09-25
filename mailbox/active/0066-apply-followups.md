---
task_id: "0066"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0066 — Apply the 0065 follow-ups on the VPS

## Context

0065 is verified green (`hermes/0065-deploy-followups @
a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f`, CI run 36185130873 success).
Its runbook changes are the four deploy follow-ups from 0064's notes.
This task applies them on the VPS. Independent of 0067 (CI fixture),
which may run in any order.

## Task

SSH to the VPS as `deploy` and apply the 0065 runbook changes exactly:

1. Mosquitto file ownership: `chown mosquitto:mosquitto`
   `/etc/mosquitto/passwd` (mode 0600) and `/etc/mosquitto/acl` (mode
   0640); reload mosquitto; confirm the ownership warning is gone from
   the journal.
2. API vhost hardening: add the tls/headers snippet include lines to the
   api vhost in `/etc/nginx/sites-available/nordtronics.io` so it
   matches the site's hardening; `nginx -t`; reload nginx; verify
   `https://api.nordtronics.io/v1/nodes` still serves 200.
3. Operator bundle access: `setfacl -m u:deploy:r
   /etc/nordtronics/mqtt-ca.pem`; verify `deploy` can read the bundle
   (head -1 shows the PEM header).

## Success criteria

1. Mosquitto active with no ownership warning in the journal.
2. API vhost serves 200 with the hardened snippets included
   (`nginx -T` shows the includes in the api server block).
3. `deploy` can read `/etc/nordtronics/mqtt-ca.pem`.
4. All three services (`mosquitto`, `wildfire-api`, `wildfire-ingest`)
   still active afterwards.

## Constraints

- VPS changes only as the 0065 runbook directs. No source edits on the
  VPS — laptop builds, VPS serves.
- No secrets in the reply.
- Cost bound: deepseek-flash only, off-peak hours.

## Proof

The exact command output for each of the three items plus the service
states, quoted in the reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then
the reply body: per-item results with verbatim output.
