---
task_id: "0106"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 24h
proof: []
notes: |
  Filed by Juno, 2026-10-03. The final flash: 0105 is verified and archived
  (payload + events match the deployed backend contract — actual firmware
  bytes were fed through validation.py/events.py on main, ok=True). This
  task puts that build on the bench base and proves the whole stack:
  physical node -> physical base over LoRa -> MQTT -> API -> app, with a
  fresh reading and no "Stale". Stephen's explicit requirement.
---

# 0106 — Flash the 0105 artifact to the bench base and prove live data

## Context

The code chain is complete and verified: 0099 (TLS + fixes, flashed in
0103 — hop 2 live), 0104 (topic + hostname + sensor role + MQTT
resilience), 0105 (payload + events match the backend contract). The CI
artifact `wildfire-node-v1-unified-firmware` from run 37167784900
(artifact id 11290351978, 769746 bytes zipped) is the build to flash.
The bench base board is at
`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00`
(MAC `80:F1:B2:A7:47:EC` — confirm by-id/MAC before touching anything;
the node `B0:A6:04:C5:75:4C` is never the target).

## Task

1. Download the 0105 artifact (`gh run download 37167784900 -n
   wildfire-node-v1-unified-firmware`), flash `firmware.bin` to the BASE
   board only via the by-id path. App-only write at 0x10000, no erase
   flags, NVS untouched (stored WiFi credentials must survive).
2. With the node board transmitting nearby (it already is), watch for
   the whole stack to complete:
   - Serial: base logs LoRa receive, then `mqtt: connected`, then a
     telemetry publish to `nordtronics/wildfire/<node-id>/telemetry`.
   - Broker: `journalctl -u mosquitto` shows the publish arriving.
   - API: `curl -s https://api.nordtronics.io/v1/nodes` shows a node
     with `last_seen_utc` within the last 15 minutes and
     `status` anything but `stale`.
   - Session: does the MQTT session hold now (0104's keepalive fix),
     or does it still cycle on keepalive timeout? Quote the broker
     journal.
3. Report which node ID appeared (the topic's node ID), the fresh
   `last_seen_utc`, and the session-hold observation.

## Success criteria

- The flashed binary matches the 0105 artifact (sha256 before flashing,
  read-back after).
- `GET /v1/nodes` shows a reading with `last_seen_utc` less than 15
  minutes old — the app will flip off "Stale" on its own from this.
  Quote the API row.
- The node board was never touched; NVS was not wiped.

## Constraints

- Confirm the board's MAC/by-id identity BEFORE flashing.
- No NVS wipe, no router changes, no credential reads/writes. One
  deliverable: the flashed base and the live-data proof. Nothing else.

## Proof

- sha256 of the flashed binary + read-back match.
- Quoted serial lines (LoRa receive, MQTT connect, publish).
- Quoted `GET /v1/nodes` row with fresh `last_seen_utc`.
- Broker journal lines showing session hold or continued cycling.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
