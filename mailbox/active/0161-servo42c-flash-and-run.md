---
task_id: "0161"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  Picked up 2026-10-30 23:15 UTC (17:15 MDT), off-peak (PEAK: OFF-PEAK 23:15 UTC),
  PROTOCOL MATCH 1.0.0. This file shipped without front-matter except a body-level
  `expect-reply-within` line; the protocol fields were added at pickup and that line
  was folded into the front-matter (declared here as required by the worker rules).

  Hardware identified before any write: one serial device on the host,
  /dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0
  -> /dev/ttyUSB0, and a read-only esptool probe answered ESP32-D0WD-V3 (rev v3.1),
  MAC 84:1f:e8:30:a8:50. 0142 is skipped (status: superseded, closed by Juno's
  2026-10-19 resolution); 0159 is skipped (decision-blocked, not resumable).
---
# 0161 — Flash the SERVO42C test sketch and run the bench self-test

## Context
Task 0160 staged the MKS SERVO42C UART research + ESP32 bench self-test sketch on branch `hermes/0160-servo42c-uart` (CI run 41137322512 green; UART 38400 8N1, address 0xE0, relative-move-only 0xFD, 8-step PASS/FAIL self-test over USB serial at 115200). 0160's spec wrongly assumed Stephen runs the hardware step himself — correction: the ESP32 devkit is plugged into YOUR host's USB (Stephen's laptop, the machine you run on), and the SERVO42C driver is wired to it (ESP32 TX2→driver RX, RX2→driver TX, common GND; driver VIN on 12–24V; driver OLED alive showing shaft angle). You have USB-flash tooling proven (by-id path + MAC check).

## Task
1. Get the sketch + PROTOCOL.md from branch `hermes/0160-servo42c-uart` (check it out or reuse as staged).
2. Flash the sketch to the ESP32 devkit over USB using the by-id path + MAC check (NVS-safety rung: read partition table first, sha256 read-back).
3. Open the USB serial console at 115200, reset the board, and capture the FULL boot self-test output (8 steps, PASS/FAIL per step).
4. Report every step's result verbatim from the serial log. If a step FAILs, quote the exact failing lines plus your read of what the driver answered (or didn't).

## Success criteria
- Sketch flashed to the physically attached ESP32; sha256 read-back matches.
- Full serial log of the 8-step self-test captured and quoted in the staged reply.
- Each step marked PASS or FAIL with the driver's actual responses.

## Constraints
- Default model tier (keep the spend ordinary).
- Stephen confirms this is the ONLY ESP32 currently plugged into the host — everything else is unplugged. If no ESP32 appears on USB (by-id), STOP and report that — do not guess at ports or flash anything else.
- Read-only toward everything that isn't the target ESP32. Do not touch the driver's wiring or power.
- If the self-test hangs (no output for 60s after reset), report the hang point and stop.

## Proof
- Staged reply quoting the full serial log.
- The esptool/PlatformIO flash command used + read-back hash.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

