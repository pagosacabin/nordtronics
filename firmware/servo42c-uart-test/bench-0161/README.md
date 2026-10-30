# bench-0161 — SERVO42C UART bench run (executed on hardware, 2026-10-30)

Task 0161: flash the 0160 self-test sketch to the physically attached ESP32 and
capture its boot self-test from the driver.

## Result in one line

The sketch flashed, verified byte-identical, and ran. **Step 1 FAILED** — the
driver returned no bytes on either framing generation, and a follow-up
read-only sweep found **no bytes at any of six baud rates or four addresses,
with zero idle-line noise**. Steps 2-8 were not attempted because the sketch
returns early when it cannot establish the link (by design).

## Board

| | |
|---|---|
| Port | `/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0` → `/dev/ttyUSB0` |
| Chip | ESP32-D0WD-V3 (revision v3.1), 4 MB flash |
| MAC | `84:1f:e8:30:a8:50` (read with `esptool read_mac` before and after the write) |
| Partition table | `otadata@0x9000`, `phy_init@0xb000`, `app0@0x10000` (0x1c0000), `app1@0x1d0000` (0x1c0000), `nvs@0x390000` (0x6d000) |

## Flash

Only `app0` was written — the bootloader, partition table, otadata and NVS were
left untouched.

```bash
pio run                       # from firmware/servo42c-uart-test @ 564cbfd
esptool.py --port /dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0 \
  --before default_reset --after no_reset write_flash \
  --flash_mode dio --flash_freq 40m --flash_size detect 0x10000 firmware.bin
```

| | sha256 |
|---|---|
| host `firmware.bin` (271 504 B) | `f0a3b3549fc62b67916727811fd0d26f409e6655f9c0b3917f8b56b36a58d491` |
| read back from `0x10000` (271 504 B) | `f0a3b3549fc62b67916727811fd0d26f409e6655f9c0b3917f8b56b36a58d491` |
| pre-flash `app0` backup (1 835 008 B) | `c5b6e071aa594418a2e9ad5e0d2d66ef667c91c45818900f467acdc79bc69ea3` |
| pre-flash `nvs` backup (446 464 B) | `1219c555560e950b48c3dca996e69bac370907da90c06c0f2d7c91c824309fbf` |
| pre-flash `otadata` backup (8 192 B) | `8ba3b110139f45443d4f268d1a3373ef99a1718b71d51664531b83ee2d4b91a3` |

`cmp` on the read-back is byte-identical to the host image.

### Flash read path is lossy on long transfers

A single `read_flash` of 446 KB aborted at ~65 % with
`Corrupt data, expected 0x1000 bytes but received 0x10 bytes`, and a 64 KB
chunk of `app0` failed once at `0x80000`. The same 4 KB reads repeated three
times were byte-identical each time, and the retried 64 KB chunk succeeded, so
this is a transfer-length/uplink fault rather than a bad flash region. Every
read in this directory that backs a hash was taken in 64 KB chunks with up to
four retries and a size check per chunk; `esptool`'s own "Hash of data verified"
was **not** treated as the verification.

## Files

| File | What it is |
|---|---|
| `serial-self-test.log` | full boot capture, 115200, from ROM banner through the failed step 1 |
| `serial-diagnostic-sweep.log` | the follow-up read-only sweep (6 bauds × 4 addresses, plus idle-noise counts) |
| `diagnostic-sweep-main.cpp` | source of that sweep sketch — read commands only (`0x36`, `0x30`), no driver writes |

## What the console shows

```
STEP 1: uart framing / protocol                FAIL  (no valid reply on either framing)
HINTS: check TX/RX swap, common GND, UartAddr=0xE0, UartBaud=38400, CtrMode=CR_UART on the OLED.
```

The sweep rules out baud and address as the cause of that silence. What it cannot
distinguish from the ESP32 side:

1. driver not powered (no 12-24 V on `V+`/`Gnd`) — the header's `3V3` pin is
   floating by design and does not power the logic;
2. the driver's `TX` not actually landed on GPIO16 (or `RX`/`TX` swapped);
3. `CtrMode` still at its `CR_vFOC` default, where the driver answers no UART
   frames at all;
4. a sign the wiring is a different electrical layer than expected (e.g. an
   RS485 driver) — an idle line producing *zero* noise is consistent with a
   quiet powered TTL output, but also with a line tied to a fixed level.

Checks that settle it, cheapest first: read the driver's OLED menu
(`CtrMode = CR_UART`, `UartBaud` index 04, `UartAddr` e0), confirm the motor-stage
supply is on, then buzz the header `TX` to ESP32 GPIO16 and `G` to GND.
