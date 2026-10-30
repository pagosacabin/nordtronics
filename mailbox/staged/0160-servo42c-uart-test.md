---
task_id: "0160"
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
  search_requests: 3
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
proof:
  branch: hermes/0160-servo42c-uart
  sha: 564cbfd93215d81f4526c8c9f1861d47f520728b
  run: https://github.com/pagosacabin/nordtronics/actions/runs/41137322512
  files:
    - firmware/servo42c-uart-test/src/main.cpp
    - firmware/servo42c-uart-test/platformio.ini
    - firmware/servo42c-uart-test/PROTOCOL.md
    - firmware/servo42c-uart-test/README.md
    - .github/workflows/platformio.yml
notes: |
  WHAT CHANGED. New firmware tree firmware/servo42c-uart-test (platformio.ini,
  src/main.cpp, PROTOCOL.md, README.md) plus one workflow edit so CI builds it.
  The sketch runs an 8-step PASS/FAIL bench self-test over UART2 at the driver's
  documented defaults (38400 8N1, addr 0xE0): framing probe, shaft angle 0x36,
  following error 0x39, enable 0xF3 + En status 0x3A, +90 deg move 0xFD, settle
  and measured-angle check, reverse move, return-angle check.

  HOW IT WAS VERIFIED. Local `pio run` (PlatformIO Core 6.1.18, esp32dev) ->
  SUCCESS, RAM 6.6%, Flash 20.7%, zero warnings with -Wall (the first build
  surfaced one -Wsequence-point in the checksum append; fixed before the commit).
  CI run 41137322512 on branch tip 564cbfd -> success, and `gh run view --json
  headSha` equals the tip `git ls-remote --heads origin` reports; the step
  "Build firmware (servo42c-uart-test)" compiled the new tree (RAM 6.8%, Flash
  22.4%). Nothing was run against hardware - the task assigns the live motor test
  to Stephen and forbids live hardware interaction here, so the PASS/FAIL sequence
  is compiled and framed but unexercised.

  PROTOCOL CORRECTION DECLARED (the spec asked for absolute move and set-zero).
  SERVO42C firmware has NEITHER: 0xFD is a relative move of N pulses from the
  current position, and 0x91 writes the zeroMode homing datum (used by 0x94), not
  a re-zero of the live position counter. Implemented the literal intent with the
  only true absolute reference available: command +/-800 pulses (90 deg at MStep
  16) and judge the result from the measured shaft angle 0x36 within +/-5 deg.
  0x91 was deliberately NOT sent - it would overwrite the stored datum, and the
  test must be setting-preserving. There is also no version/identity command
  (0x3F is restore-defaults), so the sketch reports the detected framing
  generation as its identity result.

  SECOND CORRECTION. Firmware V1.0 vs V1.1 are not frame-compatible: V1.1 appends
  a sum-check byte and widens the 0x33/0xFD payloads from int16 to int32. The
  sketch probes both framings at boot and builds the matching frame, rather than
  assuming one; PROTOCOL.md records both generations and cites a source per row.

  SCOPE EXTENSION (declared). .github/workflows/platformio.yml gained one path
  filter entry and one build step for the new tree. Reason: the tree otherwise has
  no CI coverage at all, and the task's proof bar accepts "CI run URL proving a
  clean compile"; without the filter a push to this branch starts no run, so the
  proof would be local-only. No other firmware tree or job was touched.

  NTFY RECEIPT: not published - the spec names no topic and this deliverable has
  no compiled artifact to hand out (the run's own log is the build proof).
---

# 0160 — MKS SERVO42C UART protocol + ESP32 test sketch

## Context
Stephen has an MKS SERVO42C closed-loop stepper driver on a NEMA 17 (OLED live, showing shaft angle / following error / encoder clocks) wired to a classic ESP32 devkit: ESP32 TX2/RX2 to the driver's RX/TX, common GND, 12–24V VIN to the driver power stage. He is moving from step/dir pulse control to the driver's UART serial interface so the encoder feedback is actually used. The UART header is a JST-SH 1.0mm 4-pin (3V3, G, TX, RX). The MKS APT V1.0 adapter is NOT in the serial path — wire the sketch for direct ESP32-UART2 to driver-header connection.

## Task
1. Research the MKS SERVO42C UART protocol from authoritative sources (Makerbase/MKS official docs, wiki, or repo). Determine the default baud rate, the frame/command format, and the usable command set — at minimum: query shaft angle, query following error, absolute move, relative move, set-zero, motor enable/disable. Do not guess the baud rate or command bytes; every protocol claim must cite its source.
2. Write an ESP32 Arduino/PlatformIO test sketch on UART2 that runs a bench self-test on boot: open UART at the researched baud rate, query version/identity if the protocol supports it, read and print shaft angle + following error, command a +90 degree move, wait for settle, read back the angle and check it moved, command return to zero, and print a PASS/FAIL line per step over USB serial at 115200.
3. Compile-verify with PlatformIO (esp32dev board). Stephen runs the live motor test on the bench himself — no live hardware interaction in this task.

## Success criteria
- Sketch compiles clean under PlatformIO for esp32dev (no errors).
- Baud rate and all command bytes trace to cited MKS sources; PROTOCOL.md lists the command table with one source URL per row.
- Each test step prints an unambiguous PASS or FAIL line.

## Constraints
- Default model tier (keep the spend ordinary).
- Read-only with respect to everything except the deliverable branch. No credentials, no secrets, no network changes.
- 3V3 logic on the UART lines; nothing 5V touches them.

## Proof
- Branch `hermes/0160-servo42c-uart` pushed to origin containing the sketch and PROTOCOL.md.
- PlatformIO build log (paste the tail) or CI run URL proving a clean compile.

## Reply format
Staged reply with: branch name + tip SHA, build proof, the determined baud rate, and any protocol ambiguities found (note where sources disagree, if they do).

---

# Reply — STAGED (2026-10-30)

**Status:** staged. Branch `hermes/0160-servo42c-uart`, tip
`564cbfd93215d81f4526c8c9f1861d47f520728b`, PlatformIO Build run
[41137322512](https://github.com/pagosacabin/nordtronics/actions/runs/41137322512)
green at that exact SHA.

**Baud rate: 38400 8N1**, slave address **0xE0** — Makerbase's documented
defaults for `UartBaud` / `UartAddr`, not a guess
([format page](https://github.com/makerbase-mks/MKS-SERVO42C/wiki/Serial-communication-format)).

## What was built

`firmware/servo42c-uart-test/` — PlatformIO `esp32dev` sketch + `PROTOCOL.md`:

- **`src/main.cpp`** — one-shot 8-step bench self-test on UART2 (GPIO16 RX ←
  driver TX, GPIO17 TX → driver RX), USB serial at 115200, one `PASS`/`FAIL`
  line per step, every frame echoed as `TX:`/`RX:` hex:
  1. framing probe, 2. shaft angle (`0x36`), 3. following error (`0x39`),
  4. enable (`0xF3 01`) + En-status confirm (`0x3A`), 5. command +90° (`0xFD`),
  6. settle + measured-angle check (±5°), 7. reverse move, 8. return-angle
  check (±5°). Ends with `SELF-TEST: PASS|FAIL (n of 8 steps failed)`.
- **`PROTOCOL.md`** — command table with **one cited Makerbase source URL per
  row**; the additive-checksum rule reproduced against the manufacturer's own
  examples; the V1.0/V1.1 incompatibilities; the operation mapping below.
- **`README.md`** — wiring table for the direct ESP32↔driver-header connection
  (MKS APT V1.0 adapter out of the path), menu prerequisites, build/flash
  commands, and the failure hints the sketch prints itself.
- **`platformio.yml`** — path filter + `pio run -d firmware/servo42c-uart-test`
  build step, so this tree is CI-guarded like the others (scope extension, see
  notes).

## Protocol, in one screen

| Item | Result |
|---|---|
| link | 38400 8N1, addr `0xE0`, big-endian payloads |
| frame | `<addr> <func> [data…] <chk>`, `chk` = low byte of the byte sum (**firmware V1.1 only**) |
| shaft angle | `e0 36 tCHK` → i32, one rev = 0…65535 (`40 00` = 90°) |
| following error | `e0 39 tCHK` → i16, 65536 units/rev (`00 B7` = 1°) |
| enable / disable | `e0 f3 01` / `e0 f3 00` → `01` ok; verify with `e0 3a` (`01` enabled) |
| relative move | `e0 fd <dir|speed> <i32 pulses> tCHK` from the current position |
| absolute move | **does not exist on the SERVO42C** |
| set zero | no "re-zero live position" command; `0x91` writes the zeroMode *homing* datum |

Worked frames this sketch sends (sums verified against the manual's examples):
`e0 36 16`, `e0 39 19`, `e0 f3 01 d4`, `e0 fd 10 00 00 03 20 10` (+800 pulses
= +90° at MStep 16), `e0 fd 90 00 00 03 20 90` (reverse), `e0 f7 d7`.

## Protocol ambiguities found (as asked)

1. **Two incompatible firmware generations.** V1.1.x appends a sum check byte;
   V1.0 has none — and `0x33`/`0xFD` payloads widen from int16 to int32 between
   them. A V1.0 driver silently ignores a V1.1 frame. The sketch **probes both
   framings at boot** and builds the matching `0xFD`, then prints which
   generation answered, so the bench run cannot be misled by version drift.
2. **"Absolute move" and "set-zero" as specified are not in this firmware.**
   `0xFD` is relative-in-pulses only; `0x91` sets the zeroMode homing datum
   (used by `0x94`), not a re-zero of the live counter. Implemented honestly:
   +90° / return are ±800-pulse relative moves, judged by the *measured shaft
   angle* (`0x36`), which is the only true absolute read available. Not writing
   `0x91` is deliberate — it would overwrite the driver's stored datum.
3. **No version/identity query exists.** `0x3F` is *restore defaults*, not a
   firmware read. The framing generation stands in as the identity result
   instead of inventing a command.
4. **Most community "SERVO42C" drivers on the web implement a different
   protocol** (ZDT/Emm42 `FA … / FB …` frames with a fixed `0x6B` checksum).
   Those target SERVO42D/57D and the ZDT X42S — the 42D manual shows
   `FA 01 30 2B` for its MODBUS-RTU mode. Copying them to a 42C yields silence.
   `0x6B` also happens to be the correct sum for `e0 84 07`, so "works with
   0x6B" is not evidence of the ZDT family.
5. **`MStep` is write-only** (`0x84`), yet the pulse count for a given angle
   depends on it. The sketch takes it as a compile-time constant (default 16)
   and prints the corrective hint when the measured move disagrees, rather than
   silently rewriting the driver's configuration.
6. **The V1.0 wiki page contradicts itself** — its `0x36` line claims a trailing
   `tCHK`, while every V1.0 example is a 5-byte reply with no check byte and
   the V1.0 format page defines no checksum. The examples win; hence the probe.

## Build proof

Local (`pio run`, PlatformIO Core 6.1.18):

```
RAM:   [=         ]   6.6% (used 21472 bytes from 327680 bytes)
Flash: [==        ]  20.7% (used 271141 bytes from 1310720 bytes)
========================= [SUCCESS] Took 2.75 seconds ========================
```

CI (the run cited in `proof`), step `Build firmware (servo42c-uart-test)`:

```
RAM:   [=         ]   6.8% (used 22124 bytes from 327680 bytes)
Flash: [==        ]  22.4% (used 294092 bytes from 1310720 bytes)
Building .pio/build/esp32dev/firmware.bin
Successfully created ESP32 image.
========================= [SUCCESS] Took 15.36 seconds ========================
```

Zero compiler warnings from this tree in both builds (`-Wall` is on; the first
local build surfaced one `-Wsequence-point` in the checksum append, which was
fixed before the commit).

## What is NOT verified

The sketch has **not been run against a driver** — the task says Stephen does
the live motor test on the bench and forbids live hardware interaction here.
So the compile is proven and the protocol is cited, but the PASS/FAIL sequence
itself is unexercised: a frame-level bug would only surface on hardware (the
console prints every `TX:`/`RX:` pair precisely so that the first bench run
localises it). No board was attached; the README's sample console block is
labelled as the print *format*, not a captured run.
