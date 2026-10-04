---
task_id: "0107"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 24h
proof: []
notes: |
  Filed by Juno, 2026-10-04, at Stephen's explicit "Go ahead flash away"
  (04:13 MDT). Authorizes what 0106 forbade: flashing the NODE board.
  Context: 0106 proved the base runs the 0105 build and reaches the broker,
  but the node still transmits the legacy link.h waveform (BW250/SF11/CR8,
  26-byte LinkPacket) which the unified base (BW125/SF7/CR5, 27-byte v1
  frames) cannot demodulate. Hop 1 is down. The fix is the unified design's
  own answer: the same 0105 artifact on the node, which self-detects
  role=node from its plugged-in sensors (BME680 at 0x77, PMS5003 on UART).
---

# 0107 — Flash the 0105 artifact to the bench NODE and prove the stack

## Context

Base board (`80:F1:B2:A7:47:EC`) runs the 0105 build (run 37167784900,
artifact 11290351978) and reaches the broker. Node board
(`B0:A6:04:C5:75:4C`) still runs the legacy bench image and its LoRa
waveform is incompatible with the base. Flash the SAME 0105 artifact to
the node. The node carries the BME680 and PMS5003, so the 0104 sensor
probe must resolve it as node.

## Task

1. Confirm the node board by-id/MAC
   (`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`)
   BEFORE any write. Flashing the base again is the unforgivable error.
2. Download the 0105 artifact, verify sha256, flash `firmware.bin` to the
   NODE board only: app-only write at 0x10000, no erase flags, NVS
   untouched.
3. Capture the node boot log and report the ROLE line — expect
   `ROLE: node (source=PROBE, sensors=present, ...)`. If it resolves as
   anything else, stop and report it as the finding (do not "fix" it
   with NVS writes).
4. With both boards up, prove the stack end to end:
   - Node console: LoRa TX lines with the new params.
   - Base console: `rx:` lines (it hears the node).
   - Broker journal: the telemetry publish arrives.
   - API: `curl -s https://api.nordtronics.io/v1/nodes` shows a fresh
     reading (`last_seen_utc` < 15 min, not `stale`). Quote the row.
   - Note which node ID it publishes under (expect "0" — unprovisioned;
     that is acceptable for this proof, declared not fixed).

## Success criteria

- Flashed binary matches the 0105 artifact (sha256 before, read-back
  after).
- Node boot log shows `ROLE: node` from the sensor probe — quote it.
- `GET /v1/nodes` shows a reading fresher than 15 minutes — quote it.
- The base board was never reflashed or reset for this task.

## Constraints

- MAC/by-id identity check BEFORE flashing. Wrong board = failure.
- No NVS wipe on the node. No router changes. No credential reads/writes.
- No repo commits, no new branches — this is an apply task. One
  deliverable: the flashed node and the live-data proof. Nothing else.

## Proof

- sha256 of the flashed binary + read-back match.
- Quoted node boot ROLE line.
- Quoted base `rx:` line(s).
- Quoted `GET /v1/nodes` row with fresh `last_seen_utc`.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
