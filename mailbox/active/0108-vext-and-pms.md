---
task_id: "0108"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 72h
proof: []
notes: |
  PICKED UP (iteration 1 -> 2) by the mailbox worker, 2026-10-04 12:15 UTC.
  Off-peak (PEAK: OFF-PEAK 12:15 UTC). Predecessors 0097/0098/0106/0107 remain
  decision-blocked in active/ and were not touched. This task carries the fix
  0107's blocker named (Vext gate, GPIO36), so it is the work to do.

  Filed by Juno, 2026-10-04. From 0107's BLOCKED findings (verified by Juno):
  the flashed node resolves as BASE because the unified firmware never drives
  the Heltec Vext power gate (GPIO36, active LOW). The bench BME680 is wired
  VIN->Vext (see bench-connection-list-2026-10-02.md), so it sits unpowered at
  0 V; an unpowered I2C chip clamps SDA and takes the whole bus down, so
  probe_node_sensors() sees nothing and the role falls through to Default =
  base. The legacy bench firmware drove it (node.cpp: PIN_VEXT 36, OUTPUT,
  LOW); the unified tree defines the pin but never drives it. Multi-source:
  legacy source, bench wiring list, this repo's heltec-lora-board-bringup
  skill (GPIO36 = Vext_Ctrl), and the node's own boot log (0x77/0x76/PMS5003
  all silent, 0x3C silent too — same clamp).

  2026-10-04 ~05:29 MDT, Stephen: the PMS5003's external power was OFF
  during Hermes's testing — that explains `pms5003 -> no frame`. There is
  no firmware defect to hunt on the PMS5003; the task's PMS5003 item is now
  verification-only (confirm the probe sees frames with the sensor powered),
  not diagnosis.
---

# 0108 — wildfire-node-v1: drive Vext at boot, verify PMS5003 framing

## Context

Heltec LoRa 32 V4: GPIO36 gates the Vext 3.3 V rail (active LOW). The bench
BME680's VIN is on Vext. The unified firmware never drives GPIO36, so on
sensor-carrying boards the BME680 is unpowered, clamps the shared I2C bus,
and the sensor probe + OLED probe both fail. Separately, the probe reports
`pms5003 -> no frame` on the node even though the bench wiring (PMS5003
VCC->5V, TX->1k->GPIO5, bench-proven 2026-10-02) is good — diagnose that
too; `pm25` is a required backend field and the role probe is BME-OR-PMS.

## Task

On branch `hermes/0108-vext-and-pms` (from the 0105 tip `951cbd3`), in
`firmware/wildfire-node-v1`:

1. Drive the Vext rail ON (GPIO36, OUTPUT, driven LOW) EARLY in `setup()`,
   before `probe_node_sensors()`, before `oled_probe_and_begin()`, in
   BOTH roles. On sensorless boards the rail powers nothing — harmless.
   Name it, comment the why (unpowered I2C chip clamps SDA).
2. PMS5003: Stephen confirms its external power was OFF during the
   "no frame" observation, so no firmware defect is suspected. Verify the
   probe sees valid 32-byte frames with the sensor powered (bench wiring:
   TX->GPIO5); if it does not, diagnose pins/baud/warm-up then. Do not
   rewire the bench in the spec — this is verification, then
   firmware-side diagnosis only if verification fails.
3. Keep the probe's OR logic (BME680/688 I2C OR PMS5003 frame => node);
   the SSD1306 stays excluded (both roles carry the panel).

Push the branch and take it green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and the
  `host-tests` job passes in the same run.
- Falsifiable: `grep -n "36" firmware/wildfire-node-v1/src/main.cpp`
  shows the Vext drive placed before the first I2C/Sensor/OLED init
  (quote the lines with their order).
- Falsifiable: the PMS5003 verification is reported (frames seen with
  sensor powered, or root cause + fix if not).
- `git diff` contains no credential, SSID, or password.

## Constraints

- Do not touch the LoRa receive path, the consensus logic, the MQTT
  code, or the 0104/0105 items. One deliverable: sensor power + PMS5003
  verification. Nothing else.

## Proof

- Branch `hermes/0108-vext-and-pms` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- The grep output and the PMS5003 diagnosis, quoted.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
