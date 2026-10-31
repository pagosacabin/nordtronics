# 0164 — Publish the SERVO42C code to cabin-play

## Context
The SERVO42C UART thread (0160 research, 0162 bench retest at 9600, 0163 move reconciliation) produced a working ESP32 sketch + PROTOCOL.md, currently living on nordtronics branches (`hermes/0160-servo42c-uart`, `hermes/0162-servo42c-retest-9600`). Stephen wants it published to the **pagosacabin/cabin-play** repo on GitHub — play projects live there, never in nordtronics. The bench test is proven working (driver answers at 9600 8N1, addr 0xE0; closed-loop moves executing).

## Task
1. Take the final working version of the sketch + PROTOCOL.md (the 0163-corrected state once that task stages; if 0163 hasn't staged yet, wait for it — publish the corrected version, not an intermediate).
2. Push them to **pagosacabin/cabin-play** under `servo42c-uart-test/` (sketch, PROTOCOL.md, plus a short README.md: what it is, the wiring — ESP32 TX2/RX2 to driver RX/TX, common GND, 12–24V VIN — the OLED settings it needs — UART mode enabled, 9600 baud, addr 0xE0, MStep as configured — and how to run the self-test).
3. Push from your laptop using a GitHub write path you can prove reaches cabin-play. SSH is proven (task 0136 went that way). Do NOT use the gh.py API credential — it is nordtronics-scoped and its API writes to cabin-play return 403. If you have your own API token, verify it with a read against cabin-play before writing.

## Success criteria
- `servo42c-uart-test/` live on cabin-play main with the sketch, PROTOCOL.md, and README.md.
- README lets someone with the same hardware wire it up and run the self-test without asking questions.

## Constraints
- Default model tier.
- Use your own proven write path for the cabin-play push (not the gh.py credential). Don't restructure anything else in the repo.

## Proof
- Commit SHA on cabin-play main + file listing of the new directory.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

expect-reply-within: 6h
