---
task_id: "0128"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0128 — 0119 live-half proof: node is back on USB

## Context

- The wildfire node ran the battery discharge run 1 on battery (~14h, 3.949V → silence at 3.547V/9%). Stephen put it back on USB power 2026-10-08 ~08:04 MDT.
- Your 0119 battery-ADC work is done code-side; what remains is the live-half proof for Juno's formal verification: boot banner, GPIO1 ADC pin probe, and live API battery_v.
- The discharge run proved the API path end-to-end (2241 readings, battery_v live throughout), so the API leg is already green — the bench legs are what's missing.

## Task

With the node on USB: capture the boot banner from serial, probe the GPIO1 ADC pin voltage (compare against the OLED vbat reading and Stephen's meter — expect ±100 mV agreement per the 0119 bench note), and confirm the API is serving battery_v for the node. Stage the reply with all three proofs.

## Success criteria

- Staged reply includes: boot banner text, GPIO1 probe voltage + OLED vbat + meter comparison, and the live API battery_v reading.
- Juno can then run verify-staged.py and archive 0119 as verified.

## Constraints

- Bench discipline: USB by by-id path + MAC check (B0:A6:04:C5:75:4C).
- Do not flash the node — read-only bench work. The 0125 (0120 power-state-machine) task is code + CI only; no bench flashing until Juno says so.
- Cost: use your cheapest model tier for this task. Do not use premium/pro models.

## Proof

Per mailbox/README.md: staged reply with the three proof artifacts listed above (banner text, probe numbers, API reading).

## Reply format

Stage with front-matter proof. Body: the three proofs, the meter comparison, and anything that didn't match expectations.
