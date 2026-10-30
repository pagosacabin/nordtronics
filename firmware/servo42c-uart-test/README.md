# servo42c-uart-test — bench self-test sketch

MKS SERVO42C closed-loop stepper driven over its UART header from a classic
ESP32 devkit (`esp32dev`), replacing step/dir pulse control so the encoder
feedback is actually used. Protocol research and citations:
[`PROTOCOL.md`](PROTOCOL.md).

## Wiring (direct ESP32 UART2 ↔ driver header, no MKS APT V1.0 adapter)

| Driver header (JST-SH 1.0 mm) | ESP32 devkit |
|---|---|
| `TX` (driver → host) | GPIO16 — UART2 RX |
| `RX` (host → driver) | GPIO17 — UART2 TX |
| `G` | GND (common ground is mandatory) |
| `3V3` | leave open — the MKS manual lists it as floating; do not back-feed |
| `V+` / `Gnd` (screw terminal) | 12–24 V bench supply for the motor stage |

All UART logic is 3.3 V TTL. Nothing 5 V may touch TX/RX.

Driver menu settings that must be true before the test: `CtrMode = CR_UART`,
`UartBaud = 38400`, `UartAddr = e0`, and `MStep` matching
`STEP_SUBDIVISION` in [`src/main.cpp`](src/main.cpp) (default 16 — MKS's
default; the value is shown on the OLED but cannot be read over UART).

## Build

```bash
pio run                 # compile for esp32dev
pio run -t upload       # flash
pio device monitor -b 115200
```

## What it does on boot

One-shot self-test, USB serial at 115200, one unambiguous `PASS`/`FAIL` line per
step:

1. framing probe — checksummed (V1.1 firmware) vs plain (V1.0)
2. read shaft angle (`0x36`)
3. read following error (`0x39`)
4. enable motor (`0xF3 01`) and confirm via En status (`0x3A`)
5. command +90° (800 pulses at MStep 16) with `0xFD`
6. poll the angle until motion settles; check it moved 90° ± 5°
7. command the reverse 800 pulses
8. check the shaft lands back on the start angle ± 5°

then stops the motor and prints `SELF-TEST: PASS|FAIL (n of 8 steps failed)`.

Every frame is echoed as `TX:` / `RX:` hex, so a failure is diagnosable from the
console alone.

### Expected console shape (first run on hardware)

```
=== MKS SERVO42C UART bench self-test (build 0160) ===
UART2 38400 8N1  addr 0xE0  RX=GPIO16 TX=GPIO17
Assume MStep=16, motor 200 full steps -> 3200 pulses/rev, 90 deg = 800 pulses
-- probing driver framing (0x36 both generations) --
    TX: e0 36 16
    RX: e0 00 00 40 00 20
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
...
SELF-TEST: PASS (0 of 8 steps failed)
```

That block is the *format* the sketch prints; it is not a captured run — no board
was attached while this was written (the live motor test is Stephen's, per the
task).

## Failure hints the sketch prints itself

| Symptom | Printed hint |
|---|---|
| no reply on either framing | check TX/RX swap, common GND, `UartAddr`, `UartBaud`, `CtrMode` |
| `0xFD` rejected (`result 00`) | `CtrMode` is probably not `CR_UART` |
| measured move ≠ commanded 90° | set `STEP_SUBDIVISION` to the `MStep` shown on the OLED |
