# 0165 — Can the driver settings be read over UART?

## Context
Stephen asks: instead of reading the SERVO42C's setup off the OLED menu by hand (UART baud, address, MStep, shaft direction, motor current, control mode, ...), can it be queried over the serial port? The ESP32→driver UART link is proven working (9600 8N1, addr 0xE0). Your 0160 PROTOCOL.md holds the researched MKS command table — the answer lives there.

## Task
1. Check the researched command table: does the SERVO42C UART protocol expose a command to read the driver's parameter/settings block (as opposed to live values like angle/encoder/error, which are already proven)?
2. If YES: send it from the ESP32, capture the full settings dump, and report every parameter and value in the staged reply (plus what each one means, one line each).
3. If NO: say so plainly, citing what the table does and doesn't cover — do not invent a command.

## Success criteria
- Definitive yes/no on UART settings readout, grounded in the researched sources.
- If yes: the driver's complete current settings, quoted from the live read.

## Constraints
- Default model tier.
- Read-only toward the driver: query commands only, no writes to settings. Do not change anything on the bench.

## Proof
- Staged reply with the yes/no, the source rows from PROTOCOL.md, and (if yes) the quoted settings dump.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

expect-reply-within: 6h
