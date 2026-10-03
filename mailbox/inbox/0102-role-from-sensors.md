---
task_id: "0102"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 72h
proof: []
notes: |
  Filed by Juno, 2026-10-03, at Stephen's direct direction from the bench.
  Stephen: role selection must be "a native board way to determine. No
  mods." The GPIO7 strap (jumper to GND = base) was Juno's 0094 spec, never
  Stephen's call — he did not know about it, and it requires a hardware mod.
  Replaced with the tank-monitor pattern: probe what's plugged in at boot.
  Stephen's architecture ruling: "Right now, a base station has no sensors"
  (whether a base should ever sense is parked as a future question).
---

# 0102 — wildfire-node-v1: role from sensor probe, strap pin removed

## Context

The unified firmware decides base vs node at boot. The current mechanism is
a GPIO7 strap (LOW = base, unstrapped = node, NVS override wins). Stephen
rejected any hardware mod for role selection. The replacement is the
tank-monitor pattern (`determineDeviceMode()` in
`firmware/tank-monitor/src/tank-monitor.ino`): probe the plugged-in
hardware at boot. For wildfire the probe target is the node sensor set:
BME680/BME688 on I2C (addresses 0x77 / 0x76 — the bench BME680 is at
0x77) and the PMS5003 on UART. Sensors found = node, none found = base.

## Task

On branch `hermes/0102-role-from-sensors` (from
`hermes/0099-wildfire-fix-set` tip `881ee68`; if 0100/0101 have verified by
pickup time, branch from the latest verified tip instead and say so in the
reply), in `firmware/wildfire-node-v1`:

1. Replace the strap logic in `src/role_detect.cpp` / `src/role_detect.h`
   with a sensor probe: NVS role override (when present and valid) still
   wins; otherwise probe I2C for BME680/688 (0x77, 0x76) and UART for the
   PMS5003 — any sensor found means `Role::Node`, none found means
   `Role::Base`. Reuse the existing bus init / probe style from the 0099
   OLED probe (`oled_probe_and_begin`) — probe, don't assume.
2. Remove all GPIO7 strap references: logic, `firmware_config.h` comments
   (GPIO7 is an ordinary pin again), and the portal label
   "Radio role (0 node / 1 base; blank = strap)" becomes
   "Radio role (0 node / 1 base; blank = auto-detect)".
3. Update `test/test_role_and_portal` to the new truth table (sensors
   present → node; absent → base; valid NVS wins both directions;
   out-of-range NVS falls through to the probe). **This task explicitly
   authorizes editing the test files** — the behavior change requires it.

Push the branch and take it green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and the
  `host-tests` job passes in the same run with the updated truth table.
- Falsifiable: `grep -rni "strap" firmware/wildfire-node-v1/src/` returns
  nothing in logic (historical comments aside, declare them); the role
  decision log line names the probe source.
- `git diff` contains no credential, SSID, or password.

## Constraints

- Role sources after this task, in priority order: NVS override >
  sensor probe > base default. Nothing else.
- Do not touch the LoRa receive path, the 0099 fixes, or the consensus
  logic. One deliverable: role detection. Nothing else.

## Proof

- Branch `hermes/0102-role-from-sensors` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- The grep output above, quoted verbatim.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
