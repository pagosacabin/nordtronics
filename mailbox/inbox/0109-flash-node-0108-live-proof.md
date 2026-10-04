---
task_id: "0109"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 24h
proof: []
notes: |
  Filed by Juno, 2026-10-04, at Stephen's explicit "Send it" (07:21 MDT).
  Supersedes 0107's flash half (0105 artifact, BLOCKED on the Vext root
  cause): flash the 0108 artifact (Vext gate driven at boot) to the NODE
  board and prove the whole stack live. 0107's diagnostic notes stay in
  active/ as the honest record of the finding.
---

# 0109 — Flash the 0108 artifact to the bench NODE and prove live data

## Context

0108 (branch `hermes/0108-vext-and-pms` @
`255e0ce60926c9bfbf46d99056faac880cf5b722`, CI run 37201631922, artifact
id 11302294566) drives GPIO36 (Vext, active LOW) at the top of `setup()`
in both roles, before role detection. Root cause it fixes: the BME680 is
wired VIN->Vext, sat unpowered, clamped I2C, and the node probed as base.
Base board (`80:F1:B2:A7:47:EC`) already runs the 0105 build and reaches
the broker — do not touch it.

## Task

1. Confirm the NODE board by-id/MAC
   (`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`,
   esptool read_mac `b0:a6:04:c5:75:4c`) BEFORE any write. Use the by-id
   path for every step — never the bare ttyACM number (they swap).
2. Download the 0108 artifact, verify sha256, flash `firmware.bin` to the
   NODE only: app-only write at 0x10000, no erase flags, NVS untouched.
   Read the node's partition table first to confirm the app write cannot
   reach NVS (0107 found an 8 MB layout on this board).
3. Capture the node boot log and report the ROLE line — expect
   `ROLE: node (source=PROBE, sensors=present, ...)` with the BME680 now
   visible. Quote it. Also quote the BME680 probe line and, if the
   PMS5003's external power is on, its frame line; if that power is off,
   declare it (Stephen: it was off during 0107's testing) — no defect
   hunt either way.
4. Prove the stack end to end, quoting each hop:
   - Node console: LoRa TX lines with the new params (915/125/SF7/CR5,
     27-byte v1 frames).
   - Base console: `rx:` lines — it hears the node. (Non-perturbing
     capture: `stty -F <port> 115200 raw -echo -hupcl` + `cat`; do NOT
     reset the base — 0107's pyserial opens rebooted it.)
   - Broker journal: the telemetry publish arrives.
   - API: `curl -s https://api.nordtronics.io/v1/nodes` shows a fresh
     reading (`last_seen_utc` < 15 min, not `stale`). Quote the row and
     name the node ID it publishes under (expect "0" — unprovisioned;
     acceptable, declared not fixed).

## Success criteria

- Flashed binary matches the 0108 artifact (sha256 before, read-back
  after, `cmp` clean).
- Node boot log shows `ROLE: node` from the sensor probe — quoted.
- `GET /v1/nodes` shows a reading fresher than 15 minutes — quoted.
- The base board received no write and no reset.

## Constraints

- MAC/by-id identity check BEFORE flashing. Wrong board = failure.
- No NVS wipe on either board. No router changes. No credential
  reads/writes. No repo commits, no new branches — apply task only.

## Proof

- sha256 of the flashed binary + read-back match.
- Quoted node boot ROLE line and BME680 probe line.
- Quoted base `rx:` line(s).
- Quoted `GET /v1/nodes` row with fresh `last_seen_utc`.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
