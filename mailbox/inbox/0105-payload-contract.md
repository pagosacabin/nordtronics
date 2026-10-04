---
task_id: "0105"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 72h
proof: []
notes: |
  Filed by Juno, 2026-10-03. From 0104's staged out-of-scope findings,
  independently verified by Juno against backend/ingest/ingest/validation.py
  and events.py on main: 0104 fixed the telemetry TOPIC, but the PAYLOAD
  does not satisfy the backend's strict validation, so no reading would be
  stored. Same story for events. Until this is fixed, the app cannot show
  live data no matter how healthy the MQTT session is. This is the last
  code item before the final flash.
---

# 0105 — wildfire-node-v1: payload + events match the backend contract

## Context

The backend validates strictly: a reading that fails is not stored at all
(`backend/ingest/ingest/validation.py`: "a reading that cannot be trusted
is not stored at all"). The firmware's telemetry payload does not satisfy
it, and its event topics/payloads do not satisfy `events.py`. Read both
files on main before writing code — they are the contract.

## Task

On branch `hermes/0105-payload-contract` (from the 0104 tip `11de2cc`),
in `firmware/wildfire-node-v1`:

1. **Telemetry payload** — emit exactly what `validation.py` requires:
   - `pm25` (µg/m³, already present)
   - `temperature_c` (not `temp_c`)
   - `humidity_pct` (not `rh`)
   - `battery_v` in VOLTS (not `batt_mv` — millivolts would fail the
     0–30 V range check; divide by 1000)
   - `node_id` as a STRING (not `node`), matching the node ID in the
     publish topic (the worker checks this; mismatch is rejected)
   - `observed_utc` ISO-8601 Z (the NTP clock is live since 0099, so the
     board can stamp it)
   Extra keys are ignored by validation (never fatal), so keep or drop
   the rest on your judgment — but the required keys must be exact.
2. **Events** — read `backend/ingest/ingest/events.py` first:
   - Publish to `<root>/<base-id>/events` (the backend subscribes
     `nordtronics/wildfire/+/events`; the firmware's `<root>/event/alert`
     shape is dropped).
   - Use the backend's event kinds: `watch_raised`, `watch_cleared`,
     `alert_raised`, `alert_cleared` (not `{"event":"alert",…}`).
3. **Offline notice** — `<root>/node/<id>/state` is ACL-denied (the ACL
   grants write only on `+/telemetry` and `+/events`). Either move it
   onto a permitted topic or remove it; your call, declared in the reply.

Push the branch and take it green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and the
  `host-tests` job passes in the same run. If a host test asserts the old
  payload shape, you MAY update it (behavior change) — declare it.
- Falsifiable: a scratch host harness (same tactic as 0104's probe)
  prints a sample telemetry JSON containing exactly `pm25`,
  `temperature_c`, `humidity_pct`, `battery_v` (volts, e.g. 4.05 not
  4050), `node_id` as a string, and `observed_utc` matching
  `YYYY-MM-DDTHH:MM:SSZ` — quote the output.
- Falsifiable: the event topic builder emits
  `<root>/<base-id>/events` and the kinds are the four `*_raised` /
  `*_cleared` strings — quote them.
- `git diff` contains no credential, SSID, or password.

## Constraints

- The backend is the authority. Do not "fix" validation.py or events.py
  to match the firmware — the backend is deployed and the app renders
  from it.
- Do not touch the LoRa receive path, the consensus thresholds, or the
  0104 items. One deliverable: the contract. Nothing else.

## Proof

- Branch `hermes/0105-payload-contract` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- The two quoted outputs above.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
