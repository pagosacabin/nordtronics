# bench-0165 — can the SERVO42C's settings be read over UART? (executed on hardware, 2026-10-31)

Task 0165: answer definitively whether the driver's settings block (UART baud,
address, MStep, Dir, current, CtrMode, …) can be queried over the serial port
instead of being read off the OLED by hand, resolve the conflicting command table
that Stephen supplied, and quote the live read if the answer was yes.

## Answer in one line

**No. The SERVO42C exposes no settings readout over UART.** Its command set has
exactly six read commands and every one of them returns a *live physical value*
(encoder, pulse counter, shaft angle, following error, En-pin status, shaft-blocked
status). Every parameter is **write-only**: the driver accepts a new value and
answers `01` (ok) / `00` (failed), and there is no frame that returns the stored
value. The website table Stephen found is wrong on every row, and one of its rows
(`0x3F` "firmware version") is actively destructive — it is **restore defaults**.

## 1. The manufacturer's command set (the authority)

<https://github.com/makerbase-mks/MKS-SERVO42C/wiki/Serial-communication-description>
(42C **V1.1** firmware; fetched again this run) is organised into five sections.
The read section is complete — six commands, no seventh:

| # | Frame | Returns | Is a setting? |
|---|---|---|---|
| 1 | `e0 30 tCHK` | encoder value `u16` | no — live |
| 2 | `e0 33 tCHK` | received pulses `i32` | no — live |
| 3 | `e0 36 tCHK` | shaft angle `i32` (0–65535/rev) | no — live |
| 4 | `e0 39 tCHK` | following error `i16` | no — live |
| 5 | `e0 3a tCHK` | En pin status `u8` (01/02/00) | no — live |
| 6 | `e0 3e tCHK` | shaft status `u8` (01/02/00) | no — live |

Every parameter sits in a **set** section, with no read counterpart anywhere in
the page — the return of these frames is always `addr, result, chk`, i.e. an
acknowledgement byte, never the value:

| Setting (OLED menu) | Write frame | Read frame |
|---|---|---|
| `Cal` encoder calibration | `e0 80 00` | none |
| `MotType` | `e0 81 xx` | none |
| `CtrMode` | `e0 82 xx` | none |
| `Ma` (current gear) | `e0 83 xx` | none |
| `MStep` | `e0 84 xx` | none |
| `En` | `e0 85 xx` | none |
| `Dir` | `e0 86 xx` | none |
| `AutoSDD` | `e0 87 xx` | none |
| `Protect` | `e0 88 xx` | none |
| `MPlyer` | `e0 89 xx` | none |
| `UartBaud` | `e0 8a xx` | none |
| `UartAddr` | `e0 8b xx` | none |
| `0_Mode` / `Set 0` / `0_Speed` / `0_Dir` / `Goto 0` | `e0 90…94` | none |
| `Kp` / `Ki` / `Kd` / `ACC` / `MaxT` | `e0 a1…a5` | none |
| **`Restore` (defaults)** | **`e0 3f`** | none — and it wipes the block |

`0x3F` is the dangerous row: the wiki's own note is *"After restoring the default
parameters, you need to power on again and reset the serial port baud rate before
serial communication"* — i.e. it resets `UartBaud` (and `CtrMode`, `MStep`, …) to
factory values, which would break this bench link until the OLED was used to fix
it. **It was deliberately not sent.** That is the one cell of the lead table that
cannot be tested on hardware without violating the task's read-only constraint,
so it is settled from the manufacturer's page alone.

## 2. What the live probe returned (build 0165-C, `serial-probe-final.log`)

ESP32 devkit (ESP32-D0WD-V3, MAC 84:1f:e8:30:a8:50) → driver, UART2 GPIO16/17,
**9600 8N1**, addr `0xE0`. Three attempts per code; every raw reply is in the log.

| Code | Reply (valid frame) | Payload | Meaning |
|---|---|---|---|
| `0x30` | `E0 FF FF FF FF FE CD A7` | **6 B** | not the documented `u16` — see §5 |
| `0x33` | `E0 00 00 00 00 E0` | **4 B** `i32` = 0 | pulse counter — **not** a 1-byte work mode |
| `0x36` | `E0 00 00 00 00 E0` | 4 B `i32` = 0 → 0.00° | shaft angle |
| `0x39` | `E0 00 00 E0` | 2 B `i16` = 0 | following error |
| `0x3A` | `E0 01 E1` | **1 B** = 0x01 | En status: enabled — **cannot** be a current in mA |
| `0x3E` | `E0 02 E2` | 1 B = 0x02 | shaft unblocked |
| `0x31` | `E0 00 E0` | 1 B = 0x00 | claimed "multi-turn position" — generic failure byte |
| `0x32` | `E0 00 E0` | 1 B = 0x00 | claimed "microstep read-back" — generic failure byte |
| `0x32` again | `E0 00 E0` | 1 B = 0x00 | repeatable, not a timing artefact |
| `0x37` **control** | `E0 00 E0` | 1 B = 0x00 | undocumented code → **same frame as 0x31/0x32** |
| `0x77` **control** | — | none | no valid frame (line noise / no answer) |
| `0x3F` | **not sent** | — | restore defaults = a write |

The `0x37` control is the load-bearing one: an undocumented function code answers
`E0 00 E0`, byte-identical to what `0x31` and `0x32` return. So those two replies
are the driver's generic "unknown/failed" answer — the `00` is a result code, not
a value of any width. There is no MStep or multi-turn payload hiding behind them.

## 3. The lead table, row by row

| Lead claim | Verdict | Why |
|---|---|---|
| `0x30` → live encoder/position | **right** (documented) | answers, but with a 6-byte payload that does not match the `u16` doc — §5 |
| `0x31` → multi-turn position | **wrong** | 1-byte `00`, identical to the undocumented `0x37` control |
| `0x33` → "active step mode, e.g. 05 = serial" | **wrong** | 4-byte `i32` counter, currently 0; 0163 measured it tracking ±800 pulses exactly. A mode would be 1 byte |
| `0x3A` → "max/target current, e.g. 1789 mA" | **wrong** | 1-byte En status (`01` = enabled). A 1789 mA figure needs at least 2 payload bytes, and `0x3A` is documented as the En pin |
| `0x32` → microstep/subdivision | **wrong** | generic `00` failure, same as the `0x37` control; the manual documents `0x84` as the only MStep frame and it is a *write* |
| `0x3F` → firmware version ID | **wrong and dangerous** | manufacturer documents `e0 3f` as **restore defaults**; not sent |
| "checksum is XOR or additive" | **additive** | every frame sent here used the additive low-byte sum and was answered; XOR would have made `e0 33` → `d3`, not the `13` the driver accepts |

Where the table comes from: its `0x3F`-as-version and per-setting read rows belong
to a **different protocol family** (ZDT/Emm42-style firmwares on the SERVO42D/57D
line, which do expose some parameter reads). Copying them onto a 42C yields either
a generic failure byte or, for `0x3F`, a settings wipe.

## 4. What can still be established without a settings read

| Setting | How it is known on this bench | Confidence |
|---|---|---|
| `UartBaud` | the link answers at **9600 8N1** (every frame in these logs) | proven by use |
| `UartAddr` | driver answers to `0xE0` | proven by use |
| `CtrMode` | `CR_UART`: it accepts `0xF3`/`0xFD` and answers UART frames (0163) | proven by use |
| `MStep` | *measured*, not read: 0163 recorded **20 480 angle counts over 3200 pulses = 20.499 counts/pulse**; at the documented 65536 counts/rev that is 3200 pulses/rev = **MStep 16** | derived from 0x33 + 0x36 |
| `Ma` (current), `Dir`, `Protect`, `MPlyer`, `AutoSDD`, `MotType`, `Kp/Ki/Kd/ACC/MaxT` | **not obtainable over UART at all** — OLED only | — |

Consequence for the firmware: `STEP_SUBDIVISION` stays a compile-time constant that
must match the OLED (as `PROTOCOL.md` §5.4 already says), and current/direction
settings can only be confirmed by behaviour (e.g. the `0xFD` direction convention
measured in 0163), never by a read.

## 5. Unexplained: `0x30`

`0x30` answers `E0 FF FF FF FF FE CD A7` — 8 bytes, which validates only as
addr + **6** payload bytes + chk (`sum = 0x6A7`, low byte `A7`). The manual
documents `e0 30` as `addr + u16 + chk` (5 bytes), and its own example is
`e0 40 00 20`. Byte-identical across all four runs today. `PROTOCOL.md` §5.7
records the same shape from 0160 with a different middle byte (`… FE DD B7`);
today's captures are consistently `… FE CD A7`. Unresolved, and `0x30` is **not**
used as evidence for anything in this task — `0x33` and `0x36` carry the proof.

## 6. Bench discipline

- **Read-only toward the driver.** The only frames transmitted were the 3-byte
  queries listed in the table plus the two framing probes; no `0x80`–`0x8B`,
  `0x90`–`0x94`, `0xA1`–`0xA5`, `0xF3`, `0xF6`, `0xFD` or `0x3F` frame was sent.
  No enable, no move, no save.
- **Settings unchanged, shown by state:** En still reads `01` (enabled), shaft
  still `02` (unblocked), and the pulse counter reads **0** — the position 0163's
  return move ended on (`0x33` 3200 → 0). The angle reads `0` raw where 0163's
  last read was `20` raw (~0.11°), a sub-degree difference at power-up between
  two readings of the same untouched shaft; no frame here asks it to move.
- **The ESP32 was flashed** (`0165-A`, `0165-B`, `0165-C`) and reset over the
  CP2102 auto-reset ladder, which is the normal bench operation for this rig.
- The capture logs are the ASCII console portion only: the classic ESP32's boot
  ROM emits an unreadable binary preamble at 115200 on reset, which has been
  stripped. No probe output was removed — each file has exactly one `SERVO42C
  settings-read probe` banner and one `PROBE-COMPLETE` line (checked).
- `serial-probe-run3-controls.log` is the **single-attempt** build; two of its
  reads came back corrupted (`78 27 23 01 E1` for `0x3A`), the damaged-reply
  fault already recorded in 0162/0163. That run is why the shipped build retries
  every code three times and prints each attempt.

## Files

| File | What it is |
|---|---|
| `../src/main.cpp` | the probe firmware (read-only; retries, controls, `0x3F` refusal printed on the console) |
| `../platformio.ini` | `esp32dev`, 115200 console |
| `0165-capture.py` | reset + capture helper (CP2102 ladder, same as 0162) |
| `serial-probe-run1.log`, `serial-probe-run2.log` | build `0165-A`, single attempt, two runs — identical results |
| `serial-probe-run3-controls.log` | build `0165-B`, first build with the `0x37`/`0x77` controls; single attempt (one damaged reply) |
| `serial-probe-final.log` | build `0165-C`, shipped: 3 attempts per code, all retries visible |
