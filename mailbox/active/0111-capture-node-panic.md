---
task_id: "0111"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
proof: []
notes: |
  Filed by Juno, 2026-10-05 ~06:35 MDT. Supersedes 0110 (stuck in_progress
  for 12h, worker picked it up before the key updates landed and likely
  went into stand-down). Do NOT touch 0110; this task stands alone.
  PICKED UP (iteration 1 -> 2) by the mailbox worker, 2026-10-05 13:1x UTC.
  Off-peak (PEAK: OFF-PEAK 13:15 UTC). Predecessors 0097/0098/0106/0107/0109/0110
  remain decision-blocked in active/ and were not touched. The filed front-matter
  carried iteration: 1 and status: staged (README says iteration starts at 0 and
  an inbox task is status: inbox); the pickup increment is applied literally, so
  this task now reads iteration: 2. See the reply body for the run result.
  07:41 MDT Stephen: tapped RST once (no PRG hold). The board should be
  booting the app NOW — watch the bus immediately; if the ~12-min
  metronome from 0110's kernel log is still the shape, the up-window is
  only 3-5 s, so poll /dev/serial/by-id/ tight and capture on appearance.
---

# 0111 — Capture the node board's crash panic

## Context

The wildfire node board (Heltec WiFi LoRa 32 V4, MAC B0:A6:04:C5:75:4C)
is running the 0108 unified firmware (verified flashed + read-back
2026-10-04, branch hermes/0108-vext-and-pms, SHA
255e0ce60926c9bfbf46d99056faac880cf5b722). After the flash the board
boot-then-dies: enumerates on USB for ~1 s, drops off, repeats.

2026-10-04 18:25 MDT Stephen parked the board in ROM download mode
(PRG hold + RST tap). It STAYED UP — hardware and USB power are
exonerated. The 0108 app is the killer: likely a crash-loop once the
role probe finds the hard-wired BME680 (VIN->Vext, SCL->GPIO17,
SDA->GPIO18, addr 0x77) and enters the node path. The board has not been
touched since; it should still be sitting in download mode.

## Task

1. Confirm the node's by-id path is present:
   `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`
   (by-id and MAC only — never /dev/ttyACM0/ACM1).
2. Serial-reset the board out of download mode into the app. An RTS/EN
   pulse is enough — no physical action needed. If the board is NOT in
   download mode and already crash-looping, just proceed to step 3.
3. Capture the serial output through at least one full boot-then-die
   cycle (115200 baud). Non-perturbing capture only — never reset the
   base board, never touch NVS.
4. Read the panic/backtrace. It names the crashing code.

Cost: flash tier, small diagnostic task. Off-peak preferred if the
worker supports scheduling, but do not delay past the reply window.

## Success criteria

- Quoted panic/backtrace text from the node's serial output, verbatim.
- One-line read of what the panic says (which function/file/line if the
  backtrace resolves, or the exception cause if not).
- The base board received no write and no reset. No NVS writes anywhere.

## Constraints

- DIAGNOSIS ONLY. Do NOT attempt a fix, do NOT reflash, do NOT write
  NVS, do NOT change wiring. A fix needs a new decision.
- No repo commits, no new branches. One deliverable: the panic report.

## Proof

- `ls /dev/serial/by-id/` showing the node path.
- Quoted serial capture including the panic block.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
