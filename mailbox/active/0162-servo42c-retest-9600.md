---
task_id: "0162"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP 2026-10-31T00:15:32Z. This task arrived with NO front-matter
  (same family as 0071, rule 16), so the protocol fields were added at pickup:
  task_id, protocol_version, status, iteration, and the body-level
  `expect-reply-within: 6h` line was folded into the front-matter.
---
# 0162 — Re-run SERVO42C self-test at 9600 baud

## Context
Task 0161 flashed the 0160 self-test sketch (UART 38400 8N1, addr 0xE0) but the SERVO42C returned zero bytes on all baud/address combinations swept. Root cause found on the bench: the driver's OLED menu has the UART at **9600 baud, address 0xE0** — the sketch's 38400 never matched. Branch `hermes/0161-servo42c-bench-run` holds the 0161 work; the ESP32 is still the only one on the host USB.

## Task
1. Rebuild the self-test sketch with the driver UART at **9600 8N1** (address 0xE0 unchanged). Keep the 8-step structure and PASS/FAIL reporting identical.
2. Flash to the ESP32 over USB (by-id path + MAC check; NVS-safety rung as in 0161).
3. Reset, capture the full boot self-test serial log at 115200 USB, and report every step verbatim.

## Success criteria
- Sketch rebuilt at 9600, flashed, read-back verified.
- Full 8-step serial log quoted in the staged reply, each step PASS/FAIL.

## Constraints
- Default model tier.
- If the self-test STILL returns zero bytes at 9600/e0, stop and report that plainly — do not sweep further. The remaining suspect is the driver's control mode (CtrMode must be CR_UART), which Stephen is checking on the OLED menu; your report should say so.
- Read-only toward everything that isn't the target ESP32.

## Proof
- Staged reply quoting the full serial log.
- Branch + tip SHA for the rebuilt sketch.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.
