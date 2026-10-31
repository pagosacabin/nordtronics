# servo42c-read-settings-0165 — read-only settings-readout probe

One-shot probe that answers task 0165: **can the MKS SERVO42C's settings block be
read over UART?** It sends only single-function query frames from the driver's
read family and prints every raw reply byte, so the answer rests on the driver's
own traffic rather than on a datasheet.

Result: **no** — the driver has no settings readout; all six read commands return
live physical values and every parameter is write-only. Full run record, quoted
output and the row-by-row refutation of the third-party command table:
[`bench-0165/README.md`](bench-0165/README.md). Protocol context:
[`../servo42c-uart-test/PROTOCOL.md`](../servo42c-uart-test/PROTOCOL.md) §6.

## Wiring

Identical to `../servo42c-uart-test`: ESP32 UART2, GPIO16 = RX (driver TX),
GPIO17 = TX (driver RX), common GND, **9600 8N1**, driver address `0xE0`.

## Build / flash / capture

```bash
pio run                     # compile for esp32dev
pio run -t upload           # flash
python3 bench-0165/0165-capture.py bench-0165/serial-probe.log 45
```

## What it sends

| Code | Why |
|---|---|
| `0x30 0x33 0x36 0x39 0x3A 0x3E` | the six documented reads — the evidence that only live values exist |
| `0x31`, `0x32` | third-party claims of a multi-turn position / MStep read-back |
| `0x37`, `0x77` | **negative control**: undocumented codes. `0x37` answers exactly `E0 00 E0`, which proves `0x31`/`0x32` return the same generic failure byte |
| `0x3F` | **never sent** — the lead table calls it "firmware version", the manufacturer documents it as *restore defaults*. The console prints `0x3F NOT SENT` so the omission is visible in the log |

No `0x80`–`0x8B`, `0x90`–`0x94`, `0xA1`–`0xA5`, `0xF3`, `0xF6`, `0xFD` or `0x3F`
frame is ever transmitted: no setting is written, the motor is never enabled and
never moved.

Each code is attempted up to three times (the 9600 link drops the odd reply —
0162/0163), every attempt's raw bytes are printed, and the first valid
sum-checked frame decides the result. Output ends with `PROBE-COMPLETE`.
