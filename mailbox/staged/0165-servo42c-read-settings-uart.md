---
task_id: "0165"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
proof:
  - branch: hermes/0165-servo42c-read-settings
    sha: 2cdbfa74a26d3869f33d0db1670c4a64594b565f
    run: https://github.com/pagosacabin/nordtronics/actions/runs/41168756769
    run_head_sha: 2cdbfa74a26d3869f33d0db1670c4a64594b565f
    run_conclusion: success
    files:
      - firmware/servo42c-read-settings-0165/platformio.ini
      - firmware/servo42c-read-settings-0165/src/main.cpp
      - firmware/servo42c-read-settings-0165/README.md
      - firmware/servo42c-read-settings-0165/bench-0165/README.md
      - firmware/servo42c-read-settings-0165/bench-0165/0165-capture.py
      - firmware/servo42c-read-settings-0165/bench-0165/serial-probe-final.log
      - firmware/servo42c-read-settings-0165/bench-0165/serial-probe-run1.log
      - firmware/servo42c-read-settings-0165/bench-0165/serial-probe-run2.log
      - firmware/servo42c-read-settings-0165/bench-0165/serial-probe-run3-controls.log
      - firmware/servo42c-uart-test/PROTOCOL.md
      - .github/workflows/platformio.yml
    reason: |
      Both jobs green (build, host-tests) at this exact head SHA. The build job's
      "Build firmware (servo42c-read-settings-0165)" step compiled and linked the
      probe on this branch ("Linking .pio/build/esp32dev/firmware.elf", then
      "SUCCESS Took 9.54 seconds"). The workflow was path-filtered and did not
      watch this new directory, so its path list plus a build step were added in
      the same commit - declared in notes as a scope extension.
    applied_to_hardware:
      - "ESP32 devkit 84:1f:e8:30:a8:50 flashed 3x (0165-A, -B, -C) via /dev/ttyUSB0"
      - "driver written to: nothing (read-only - no set/restore/enable/move frame sent)"
    no_ntfy_receipt: |
      the spec names no ntfy topic and there is no deployable artifact: CI compiles
      a bench probe whose evidence is the host-local serial output quoted below.
notes: |-
  ANSWER: NO. The SERVO42C has no settings readout over UART. Its command set has
  exactly six read commands and all six return live physical values; every menu
  parameter is write-only (a set answers 01/00 and no frame returns a stored
  value). Verified two ways: the manufacturer's own command list (re-fetched this
  run) and a live read-only probe on the bench.

  SUCCESS CRITERION 2 (the settings dump) is not applicable - there is no settings
  block to dump, so none is fabricated. Criterion 1 (definitive yes/no grounded in
  sources) is met in the reply body and in PROTOCOL.md section 6.

  THE LEAD TABLE IS WRONG ON EVERY ROW, and one row is destructive:
   0x33 "work mode"      -> 4-byte i32 pulse counter, live value 0; 0163 measured
                            it tracking +/-800 exactly. A mode would be 1 byte.
   0x3A "current, mA"    -> 1-byte En status, E0 01 E1 (enabled). A 1789 mA figure
                            needs >=2 payload bytes; 0x3A is documented as the En pin.
   0x32 "microstep"      -> E0 00 E0, byte-identical to the UNDOCUMENTED control
                            code 0x37 (also E0 00 E0) = generic failure byte, not data.
   0x31 "multi-turn"     -> same generic E0 00 E0 as the 0x37 control.
   0x3F "firmware ver"   -> DANGEROUS, NOT SENT: MKS documents e0 3f as RESTORE
                            DEFAULTS (resets UartBaud/CtrMode/MStep and breaks the
                            bench link). Untestable on hardware without a write, so
                            settled from the manufacturer's page alone.
   0x30                  -> documented and answering, but with a 6-byte payload
                            (E0 FF FF FF FF FE CD A7) instead of the documented u16;
                            byte-identical across four runs. Same shape as PROTOCOL.md
                            section 5.7. Not used as evidence anywhere.
   checksum              -> additive low-byte sum confirmed: every frame sent used it
                            and was answered (XOR would give d3 for e0 33, not 13).

  HARDWARE EVIDENCE (classic ESP32 devkit -> driver, 9600 8N1, addr 0xE0):
    0x30 E0 FF FF FF FF FE CD A7 (6B payload, anomaly)  0x33 E0 00 00 00 00 E0 (i32 0)
    0x36 E0 00 00 00 00 E0 (0.00 deg)                   0x39 E0 00 00 E0 (i16 0)
    0x3A E0 01 E1 (En enabled)                          0x3E E0 02 E2 (unblocked)
    0x31 E0 00 E0 | 0x32 E0 00 E0 (twice) | 0x37 E0 00 E0 (control) | 0x77 no frame
  Four captures in bench-0165/. The shipped build (0165-C) retries each code 3x and
  prints every attempt, because an earlier single-attempt build returned two
  corrupted replies (the 0162/0163 damaged-reply fault) - that run is kept as
  evidence rather than deleted. In the final 0165-C run itself, 0x30/0x33/0x39 each
  needed a second attempt after a corrupted first byte, so the retry is what makes
  the quoted values true rather than lucky.

  READ-ONLY, PROVEN BY CONSTRUCTION: the only frames transmitted were the 3-byte
  queries above plus the framing probe. No 0x80-0x8B, 0x90-0x94, 0xA1-0xA5, 0xF3,
  0xF6, 0xFD or 0x3F frame was ever sent - no setting written, motor never enabled
  or moved. Post-run driver state unchanged: En 01 (enabled), shaft 02 (unblocked),
  pulse counter 0 (where 0163's return move ended).

  WHAT THIS LEAVES STEPHEN: the settings that matter for the link can be established
  without a readout - baud 9600, addr 0xE0 and CtrMode=CR_UART are proven by the link
  working, and MStep 16 is *measured* (0163: 3200 pulses = 65 536 angle counts).
  Ma/current, Dir, Protect, MPlyer, AutoSDD, MotType and the PID/ACC/MaxT block
  cannot be read at all: OLED only, behaviour at most.

  DEVIATIONS, all declared:
   1. FLASHED THE ESP32 three times (0165-A/B/C) and reset it over the CP2102 ladder.
      Bench-side only; the driver link stayed read-only.
   2. SCOPE EXTENSION: .github/workflows/platformio.yml path filters + a build step
      for the new directory, because a push of the new tree started no run at all
      (path-filtered workflow) and the probe could not be compile-verified. One path
      line in two lists plus one build step.
   3. LOG TRIMMING: each capture keeps the ASCII console portion; the classic ESP32
      boot ROM emits an unreadable binary preamble at 115200 on reset, which is
      stripped. No probe output removed - each file has exactly one banner and one
      PROBE-COMPLETE line (counted).
   4. The probe lives in its own project instead of extending the 0163 sketch: the
      0163 self-test is a verified 8-step PASS artifact and was left untouched.
      PROTOCOL.md (the researched table) was extended with the new section 6.
---


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

---

# Reply — NO: there is no settings readout (2026-10-31, bench-verified)

**Answer:** the SERVO42C UART protocol has **six read commands and none of them
reads a setting**. All six return live physical values; every menu parameter
(`UartBaud`, `UartAddr`, `MStep`, `Dir`, `Ma`, `CtrMode`, `MPlyer`, `Protect`,
`AutoSDD`, `MotType`, PID/ACC/MaxT) is **write-only** — the driver acknowledges a
set with `01`, and no frame returns the stored value.

## Source rows (PROTOCOL.md §3, manufacturer's page re-fetched this run)

| # | Read frame | Returns | Setting? |
|---|---|---|---|
| 1 | `e0 30 tCHK` | encoder `u16` | no — live |
| 2 | `e0 33 tCHK` | received pulses `i32` | no — live |
| 3 | `e0 36 tCHK` | shaft angle `i32` | no — live |
| 4 | `e0 39 tCHK` | following error `i16` | no — live |
| 5 | `e0 3a tCHK` | En pin status `u8` | no — live |
| 6 | `e0 3e tCHK` | shaft status `u8` | no — live |

Everything else is a *set* command (`0x80`–`0x8B`, `0x90`–`0x94`, `0xA1`–`0xA5`),
answered `addr, result, chk`. `PROTOCOL.md` gained **§6** recording this.

## The conflicting table, resolved on hardware

| Lead claim | Verdict | Live reply |
|---|---|---|
| `0x30` live encoder | yes (documented) | `E0 FF FF FF FF FE CD A7` — 6-byte payload, *not* the documented `u16` (unexplained, §5.7; not used as evidence) |
| `0x31` multi-turn position | **no** | `E0 00 E0` — generic failure |
| `0x33` "work mode, 05 = serial" | **no** | `E0 00 00 00 00 E0` — 4-byte `i32` pulse counter = 0; 0163 measured it tracking ±800 exactly |
| `0x3A` "current, 1789 mA" | **no** | `E0 01 E1` — 1-byte En status (`01` = enabled) |
| `0x32` microstep read-back | **no** | `E0 00 E0` — generic failure |
| `0x3F` firmware version | **no, and destructive** | **not sent**: MKS documents `e0 3f` as *restore defaults* |
| checksum "XOR or additive" | **additive** | every frame sent used the additive sum and was answered (`XOR` would give `d3` for `e0 33`, not `13`) |

The decisive control: `0x37` — a code the manufacturer never defines — answers
**`E0 00 E0`**, byte-identical to `0x31` and `0x32`. So their one-byte `00` is a
result code, not data of any width.

## Full live dump (build 0165-C, `bench-0165/serial-probe-final.log`)

```
===
SERVO42C settings-read probe — 0165-C (read-only settings probe, 3 attempts/code, controls)
link: UART2 9600 8N1, addr 0xE0
READ-ONLY: queries only, no set/restore/enable/move frames
==========================================================
framing: V1.1 (checksum, 6-byte 0x36 reply)

PROBE func=0x30  documented: encoder u16
  TX: E0 30 10
  RX[1]: 11 FF FF FF FF FE CD A7  (raw_len=8)
    -> no valid checked frame (damaged reply)
  RX[2]: E0 FF FF FF FF FE CD A7  (raw_len=8)
    -> reply_frame_start=0  payload_len=6

PROBE func=0x33  documented: received pulses i32 | CLAIMED: work mode
  TX: E0 33 13
  RX[1]: 0C 00 00 00 00 E0  (raw_len=6)
    -> no valid checked frame (damaged reply)
  RX[2]: E0 00 00 00 00 E0  (raw_len=6)
    -> reply_frame_start=0  payload_len=4
  decode: pulses=0

PROBE func=0x36  documented: shaft angle i32
  TX: E0 36 16
  RX[1]: E0 00 00 00 00 E0  (raw_len=6)
    -> reply_frame_start=0  payload_len=4
  decode: angle_raw=0  deg=0.00

PROBE func=0x39  documented: following error i16
  TX: E0 39 19
  RX[1]: 67 00 00 E0  (raw_len=4)
    -> no valid checked frame (damaged reply)
  RX[2]: E0 00 00 E0  (raw_len=4)
    -> reply_frame_start=0  payload_len=2
  decode: error=0

PROBE func=0x3A  documented: En pin status u8 | CLAIMED: motor current mA
  TX: E0 3A 1A
  RX[1]: E0 01 E1  (raw_len=3)
    -> reply_frame_start=0  payload_len=1
  decode: en_status=0x1

PROBE func=0x3E  documented: shaft status u8
  TX: E0 3E 1E
  RX[1]: E0 02 E2  (raw_len=3)
    -> reply_frame_start=0  payload_len=1
  decode: shaft_status=0x2

PROBE func=0x31  CLAIMED: multi-turn position (not in MKS table)
  TX: E0 31 11
  RX[1]: E0 00 E0  (raw_len=3)
    -> reply_frame_start=0  payload_len=1
  decode: raw_payload= 00

PROBE func=0x32  CLAIMED: microstep read-back (not in MKS table)
  TX: E0 32 12
  RX[1]: E0 00 E0  (raw_len=3)
    -> reply_frame_start=0  payload_len=1
  decode: raw_payload= 00

PROBE func=0x32  CLAIMED 0x32, second attempt (repeatability)
  TX: E0 32 12
  RX[1]: E0 00 E0  (raw_len=3)
    -> reply_frame_start=0  payload_len=1

PROBE func=0x37  CONTROL: undocumented code 0x37
  TX: E0 37 17
  RX[1]: E0 00 E0  (raw_len=3)
    -> reply_frame_start=0  payload_len=1

PROBE func=0x77  CONTROL: undocumented code 0x77
  TX: E0 77 57
  RX[1]: 0B 00 00 FF 00 00 00 FE 00 C0 01 00 F8 00 00 C0 FF 01 00 00 00 00 00  (raw_len=23)
    -> no valid checked frame (damaged reply)
  RX[2]: (no reply, 400 ms)
  RX[3]: (no reply, 400 ms)
  payload_len: none

----------------------------------------------------------
SUMMARY
  0x30 payload_len=6
  0x31 payload_len=1
  0x32 payload_len=1
  0x33 payload_len=4
  0x36 payload_len=4
  0x39 payload_len=2
  0x3A payload_len=1
  0x3E payload_len=1
  0x3F NOT SENT: MKS documents it as RESTORE DEFAULTS (write).
PROBE-COMPLETE
```

## What to do instead of a readout

- `UartBaud` 9600, `UartAddr` `0xE0`, `CtrMode = CR_UART` — proven by the link
  working (nothing to read, but nothing needed).
- `MStep` — **measure** it: 0163 recorded 3200 pulses spanning 20 480 angle counts
  (= 65 536/rev) ⇒ MStep 16. Keep `STEP_SUBDIVISION` a compile-time constant.
- `Ma`/current, `Dir`, `Protect`, `MPlyer`, `AutoSDD`, `MotType`, PID/ACC/MaxT —
  OLED only. `Dir` in particular does not even change the reported encoder sign
  (0163), so it cannot be inferred from the wire either.

Read-only was preserved: the only frames sent were the 3-byte queries above plus
the framing probe; no set/restore/enable/move frame was transmitted and the motor
never moved.
