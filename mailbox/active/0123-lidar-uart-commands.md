---
task_id: "0123"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP (iteration 0 -> 1) by an interactive session at Stephen's request,
  2026-10-07 12:25 MDT. The four cron worker jobs (c0be50a686c6, 7aff6948c2c1,
  8b1c9e1323c5, 5c1532977f15) are PAUSED for the duration so no tick can resume
  this task against the same worktree; they are resumed once it is staged.
  Branch: hermes/0123-lidar-uart-commands, cut from hermes/0122-lidar-nodemcu
  (the branch the task names) @ 20b9bada1e7fe7097521389c264137eb5d0db788.
  Scope: the sketch changes exactly as specified -- Serial2 with both RX(16) and
  TX(17), LEDC removed, the three start sequences with their gaps, and a hex dump
  wrapped every 16 bytes -- then flash and observe 60 s. No rewiring, and no
  software role swap (the task forbids it: P17 direct to 4 V is unsafe).
  Model tier: DeepSeek Flash (this session).
---
# 0123 — LiDAR LDS-006: try UART start commands

## Context
Follow-up to 0122. The NodeMCU-32S is flashed and working. Findings so far:
- Turret spins freely by hand (motor mechanically OK) but never spins under power.
- Green wire: 4V DC idle, 2.8V at the P16 divider tap (confirmed by Stephen's meter).
- Blue wire: 4V DC idle, but shows 9kHz low-going pulses (4.2V→3.6V) on scope when ESP32 connected. Stephen measured 4.2V DC on P17.
- 5kHz PWM on blue did NOT spin the motor. DC high/low on either wire did nothing.
- No UART data on green (the 0x00/0x04 flood was an artifact, not real frames).
- Bench wiring is UNCHANGED: green→P16 via 10k/23k divider, blue→P17 via 1.6k.

## Theory
The LiDAR is likely command-controlled: it wants a UART "start" packet on its RX
line before it spins the motor and streams data. Blue or green is RX; the other
is TX (silent until started).

## Task
1. Modify firmware/lidar-lds006-bringup/src/main.ino (new branch from
   hermes/0122-lidar-nodemcu):
   - Init Serial2 with BOTH RX (GPIO16) and TX (GPIO17): 
     `Serial2.begin(115200, SERIAL_8N1, 16, 17);`
   - Remove the LEDC PWM code.
   - In setup(), after a 2s delay, send each of these start commands with 3s
     gaps, printing which one was sent:
     - YDLidar start: `A5 60` (bytes 0xA5, 0x60)
     - RPLidar start: `A5 20`
     - YDLidar stop then start: `A5 65`, delay 1s, `A5 60`
   - Continuously hex-dump anything received on Serial2 (with newlines every
     16 bytes this time).
2. Flash, observe 60s, report: did the turret spin after any command? Did any
   bytes arrive? Paste the hex.
3. If nothing: swap the logical roles in software (this tests whether green is
   RX and blue is TX without rewiring — but note the divider is on green, so
   3.3V TX into the divider is fine, and reading blue via P17 direct is OK
   since it's 4V... actually P17 direct to 4V is NOT safe. DO NOT swap in
   software without the divider. Instead, report "no response" and stop.)

## Success criteria
- Sketch sends all three command sequences, serial capture proves they went out.
- Clear report: motor spun or not, bytes received or not, after which command.

## Constraints
- Do NOT rewire. Do NOT put 4V directly into an ESP32 pin without the divider.
- If no command works, say so plainly — the unit may be dead or use an unknown
  protocol. Do not guess further commands beyond the three listed.

## Reply format
Stage in mailbox/staged/ per README. Include branch SHA, flash proof, serial log
showing commands sent and any response, motor observation (ask Stephen at the
bench if unsure — he can see the turret).
