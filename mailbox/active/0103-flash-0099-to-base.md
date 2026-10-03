---
task_id: "0103"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 24h
proof: []
notes: |
  Filed by Juno, 2026-10-03, at Stephen's "Yes" from the bench. This is
  Hermes's option (c), Juno's recommendation: flash the verified 0099 CI
  artifact to the base board so the TLS path, the OLED bring-up fix, and the
  WiFi join all get their real hardware test. The running v2.0-base image is
  rebuildable from ~/Documents/PlatformIO/Projects/heltec-v3-bme680/
  (FW_TAG "v2.0-base", per 0099's carried correction), so overwriting it is
  safe. 0099 did not rename any NVS field, so the WiFi credentials Stephen
  just saved in the portal persist across the flash.
---

# 0103 — Flash the 0099 artifact to the bench base and report the trifecta

## Context

Task 0099 is verified and archived: branch
`hermes/0099-wildfire-fix-set` @ `881ee68`, CI run 37158144211 green. The
CI artifact `wildfire-node-v1-unified-firmware` contains `firmware.bin`
(1216688 bytes, sha256
`5fdde258ba9fb574993bc39278d36ef5b9dec387f61af5cd57d52e90dc1be623`).
The bench base board is on Stephen's machine at
`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00`
(usually `/dev/ttyACM0`; do NOT trust the ACM number — resolve by-id and
confirm the MAC `80:F1:B2:A7:47:EC` before touching anything).

## Task

1. Download `firmware.bin` from the 0099 artifact
   (`gh run download 37158144211 -n wildfire-node-v1-unified-firmware`),
   verify its sha256 matches the value above, and flash it to the BASE
   board only, via the by-id path.
2. Capture the serial boot log.
3. Report the trifecta, each with quoted log lines:
   a. OLED: alive? What does it show? (0099 added the GPIO21 reset +
      0x3C probe + guarded `sendBuffer`.)
   b. WiFi: did the STA join the stored cabin network — IP address, or
      the disconnect reason code? (This is the 0098 gate: a fresh
      reason 202 with the retyped password means the platform migration
      is real.)
   c. MQTT/TLS: does it reach `mqtt.nordtronics.io:8883` — `mqtt:
      connected`, or the connect-failure line with `state=n`? (The
      ESP32-side TLS handshake was explicitly unverified in 0099.)

## Success criteria

- The flashed binary's sha256 matched the artifact before flashing.
- Serial boot log captured and quoted for all three report items
  (OLED / WiFi / MQTT) — answers, not silence.
- The node board (`B0:A6:04:C5:75:4C`) was never touched.

## Constraints

- Confirm the board's MAC/by-id identity BEFORE flashing. Flashing the
  wrong board is the one unforgivable error here.
- Do NOT wipe NVS: no erase flags, no `erase_flash`. The stored WiFi
  credentials must survive.
- No router changes, no credential reads/writes. One deliverable: the
  flashed base and its report. Nothing else.

## Proof

- sha256 of the binary that was flashed.
- Quoted serial lines for (a), (b), (c).
- `esptool` / `pio run -t upload` output showing the by-id port used.

## Reply format

Follow the mailbox staged-reply format: status line, the three answers
with quoted evidence, deviations declared, cost line.
