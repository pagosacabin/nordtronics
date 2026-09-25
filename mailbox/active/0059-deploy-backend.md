---
task_id: "0059"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0059 — Deploy backend v1 to the VPS

## Context

SSH access is proven (0058: first-attempt key auth from your laptop as
`deploy`). The backend tree is on branch `hermes/0056-backend-v1`
(@ 697ab6647ee3faba3114d111d4a51f93c8a29d9d, CI green) and its deploy runbook
is `backend/DEPLOY.md` at that branch tip — 12 steps plus rollback and
troubleshooting, written for a second person. The VPS is 89.117.21.105,
Ubuntu 24.04, already hardened (UFW 22/80/443/8883, Let's Encrypt cert
covering nordtronics.io + api + mqtt).

## Task

SSH to the VPS as `deploy`, get the backend tree from the branch, and work
through `backend/DEPLOY.md` steps 1–10 in order, ending with the full
end-to-end verification in step 10:

1. All three services active: `systemctl is-active mosquitto wildfire-ingest wildfire-api`
2. Broker log shows the 8883 listen socket and no cert/key error
3. `curl http://127.0.0.1:8000/healthz` and `https://api.nordtronics.io/healthz` both answer
4. Publish one test reading as node-01 through the real TLS path and read it back from `/v1/nodes` and `/v1/nodes/node-01/readings`
5. Anonymous publish is refused; plaintext 1883 times out

## Success criteria

1. Every step-10 check passes, with the actual command outputs quoted in the reply.
2. Nothing runs as root or `deploy` — mosquitto, wildfire-ingest, wildfire-api each under its own service user.
3. The test reading round-trips: published via MQTT, visible in the API.

## Constraints

- Cost bound: deepseek-flash only, off-peak hours.
- MQTT passwords are generated on the VPS at deploy time and never committed anywhere.
- Follow the runbook. If a step fails, stop at the first blocking failure, report the exact output, and do not improvise around it — declare the deviation and wait.
- The node-01 test reading is synthetic and clearly labeled; delete it from the DB after verification so the database starts clean.

## Proof

The staged reply quoting the step-10 verification outputs (systemctl, journalctl lines, curl outputs, mosquitto_pub results including the two refusal cases).

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: each verification check with its output, and any deviations from
the runbook with reasons.
