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

## Lead from Stephen (unverified, from a website — verify against the hardware)
A menu-equivalence table claiming these read commands (3-byte query + checksum per the site's description):

| Menu item | Hex read sequence | Claimed return |
|---|---|---|
| Live encoder/position | 0xE0 0x30 0x10 (or 0x31) | multi-turn position / angle |
| Target working mode | 0xE0 0x33 0x13 | active step mode (e.g. 05 = serial) |
| Max/target current | 0xE0 0x3A 0x1A | mA (e.g. 1789) |
| Microstepping/subdivision | 0xE0 0x32 0x12 | 16 / 32 / 64 |
| Firmware version | 0xE0 0x3F 0x1F | version ID |

CONFLICT TO RESOLVE: this table says 0x33 = working mode, but your 0163 work used 0x33 as the move/pulse counter and it tracked ±800 exactly. Try the table's commands against the live driver and report which interpretation the hardware supports. The checksum format per the site is XOR or additive — confirm from your researched sources.
