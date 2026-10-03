---
task_id: "0104"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 72h
proof: []
notes: |
  Filed by Juno, 2026-10-03. Supersedes 0100, 0101, 0102 (deleted from the
  inbox unpicked): Stephen hates the hour-per-task cycle, and all three are
  small, independent changes to firmware/wildfire-node-v1 that ride one CI
  run. One pickup, one branch, one verification. The three checklists below
  are each falsifiable — verify all three before staging.
---

# 0104 — wildfire-node-v1: topic fix + DHCP hostname + sensor role (one build)

## Context

Three pending changes, all on `firmware/wildfire-node-v1`, stacked on the
verified 0099 tip (`881ee68`):

1. **Topic (was 0100, blocking live data):** the firmware builds telemetry
   as `<mqtt_root>/node/<id>/telemetry`, but the deployed backend
   (ingest worker, validation template, Mosquitto ACLs — all on main)
   subscribes `nordtronics/wildfire/<node-id>/telemetry`, single level.
   The `+` wildcard matches exactly one level, so the firmware's form is
   silently dropped. Two docs repeat the wrong form and stale broker
   defaults (`api.nordtronics.io:1883`).
2. **Hostname (was 0101, Stephen's ask):** every board gets a DHCP
   hostname `wildfire-<role>-<nn>` (role = base/node, nn = zero-padded
   node ID, `01` when unprovisioned) via `WiFi.setHostname()` before STA
   bring-up — identifiable in any router client list, no OLED or serial
   needed.
3. **Role (was 0102, Stephen's call — no hw mods):** the GPIO7 strap was
   Juno's 0094 spec, never Stephen's. Replace with the tank-monitor
   pattern: probe node sensors at boot (BME680/688 at 0x77/0x76,
   PMS5003 on UART) — found = node, none = base. NVS/portal override
   still wins. Portal label "blank = strap" becomes "blank = auto-detect".

## Task

On branch `hermes/0104-wildfire-combined` (from `881ee68`), apply all
three changes, push, and take green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

Test-file rule for this task: `test/test_role_and_portal` MAY be edited,
but ONLY for the role truth table (sensors present → node; absent → base;
valid NVS wins both directions; out-of-range NVS falls through to the
probe). No other test file is touched; if any other test asserts
behavior you must change, stop and declare it in the reply.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and
  the `host-tests` job passes in the same run.
- Topic: `grep -rn "wildfire/node/" firmware/wildfire-node-v1/src/`
  returns nothing; the topic builder emits
  `nordtronics/wildfire/<node-id>/telemetry` for a sample node id (quote
  the line). `grep -rn "api.nordtronics.io"
  docs/wildfire/radio-protocol-v1.md backend/DEPLOY.md` returns nothing,
  and both show `mqtt.nordtronics.io` / `8883`.
- Hostname: `grep -n "setHostname" firmware/wildfire-node-v1/src/main.cpp`
  shows the call before the STA connect sequence; format produces
  `wildfire-base-01` / `wildfire-node-02` style names (quote the lines);
  no new portal field, no new NVS key.
- Role: `grep -rni "strap" firmware/wildfire-node-v1/src/` returns nothing
  in logic; priority is NVS override > sensor probe > base default.
- `git diff` contains no credential, SSID, or password.

## Constraints

- Do not touch the LoRa receive path, the 0099 fixes, or the consensus
  logic. These three changes and nothing else.

## Proof

- Branch `hermes/0104-wildfire-combined` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- Every grep above, quoted verbatim.

## Reply format

Follow the mailbox staged-reply format: status line, the three checklists
with quoted evidence, deviations declared, cost line.
