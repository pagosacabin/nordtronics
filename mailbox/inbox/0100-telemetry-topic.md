---
task_id: "0100"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 72h
proof: []
notes: |
  Filed by Juno, 2026-10-03, from verification of staged 0099. Hermes's 0099
  reply honestly flagged this as "found, NOT fixed": the firmware builds the
  telemetry topic as `<mqtt_root>/node/<id>/telemetry`, i.e.
  `nordtronics/wildfire/node/<id>/telemetry`. Juno checked the deployed
  backend on main and the backend is unambiguous, so 0097's string is NOT the
  outlier — the firmware is:
  - backend/ingest/ingest/worker.py:86 extracts node id from
    `nordtronics/wildfire/<node-id>/telemetry`
  - backend/ingest/ingest/validation.py: TOPIC_TEMPLATE =
    `nordtronics/wildfire/+/telemetry`
  - backend/mosquitto/acl: `topic read nordtronics/wildfire/+/telemetry`
  The `+` wildcard matches exactly one level, so the firmware's two-level
  `node/<id>` form is silently dropped by ingest: even with WiFi joined and
  TLS working, no live data would flow. The firmware must change, not the
  backend.
---

# 0100 — wildfire-node-v1: telemetry topic must match the backend contract

## Context

Task 0099 delivered the TLS uplink and six bench defects, verified and
archived. Its reply flagged a topic-string mismatch and left it unfixed.
Verification against the deployed backend (ingest worker, validation
template, Mosquitto ACLs — all on main) proves the backend contract is
`nordtronics/wildfire/<node-id>/telemetry` (single level after
`wildfire/`). The firmware's `<mqtt_root>/node/<id>/telemetry` does not
match and its publishes would be dropped. Two docs repeat the wrong form
and additionally show stale broker defaults (`api.nordtronics.io:1883`).

## Task

On branch `hermes/0100-telemetry-topic` (from
`hermes/0099-wildfire-fix-set` tip `881ee68`), in
`firmware/wildfire-node-v1`:

1. Change the telemetry topic template from
   `<mqtt_root>/node/<id>/telemetry` to `<mqtt_root>/<node-id>/telemetry`,
   so a publish for node `bench-01` lands on
   `nordtronics/wildfire/bench-01/telemetry`.
2. Update `docs/wildfire/radio-protocol-v1.md` §8 and the
   `backend/DEPLOY.md` smoke test: correct topic form AND correct broker
   defaults (`mqtt.nordtronics.io`, `8883` — not `api.nordtronics.io`,
   `1883`).

Push the branch and take it green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and the
  `host-tests` job passes in the same run.
- Falsifiable: `grep -rn "wildfire/node/" firmware/wildfire-node-v1/src/`
  returns nothing; the topic builder emits
  `nordtronics/wildfire/<node-id>/telemetry` for a sample node id
  (quote the line).
- `grep -rn "api.nordtronics.io" docs/wildfire/radio-protocol-v1.md
  backend/DEPLOY.md` returns nothing.
- `git diff` contains no credential, SSID, or password.

## Constraints

- Do not touch the LoRa receive path, the 0099 fixes, or any test file.
  If a host test asserts the old topic form, do NOT edit the test —
  declare it in the reply and stop.
- One deliverable: the topic contract. Nothing else.

## Proof

- Branch `hermes/0100-telemetry-topic` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- The grep outputs above, quoted verbatim.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
