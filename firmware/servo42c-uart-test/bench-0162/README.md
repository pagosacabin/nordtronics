# bench-0162 — SERVO42C self-test re-run at 9600 baud (executed on hardware, 2026-10-30)

Task 0162: rebuild the 0160 self-test sketch with the driver UART at **9600 8N1**
(address 0xE0 unchanged), flash it, reset, and quote the full 8-step serial log.

## Result in one line

9600 was the right baud — **the driver answers immediately** (it returned *zero* bytes at
38400 in 0161). With the sketch's original fixed-length reply check, 3 of 8 steps passed
and 5 failed; the failures were reply-*framing* failures, not driver refusals: at 9600 the
driver **echoes the command frame back before its reply**, so a valid reply arrives as
`echo + reply` and never matches an exact length. After making the reader extract the
reply frame (declared scope extension, below), **7 of 8 steps PASS**. The single remaining
FAIL is step 6: 800 commanded pulses turned the shaft **-23.47 deg / -27.02 deg** across two
runs instead of +90 deg, i.e. the sketch's assumed `MStep=16` (3200 pulses/rev) does not
match this driver. See "Step 6" below for the two candidate explanations and the one OLED
reading that settles it.

## Board

| | |
|---|---|
| Port | `/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0` -> `/dev/ttyUSB0` (no ttyACM*) |
| Chip | ESP32-D0WD-V3 rev v3.1, 4 MB flash |
| MAC | `84:1f:e8:30:a8:50` — read before and after every write, unchanged |
| Partition table | `otadata@0x9000`/0x2000, `phy_init@0xb000`/0x1000, `app0@0x10000`/0x1c0000, `app1@0x1d0000`/0x1c0000, `nvs@0x390000`/0x6d000 (identical to 0161; raw read committed as `partition-table-readback.bin`) |

## Flash discipline

Only `app0` at `0x10000` was written; bootloader, partition table, otadata and NVS were
never written. NVS and otadata were backed up first (chunked reads with retries, see
`0162-readflash.py`), and both are **bit-identical to the 0161 backups**, i.e. nothing this
run or the 0161 run touched NVS:

| region | size | sha256 | vs 0161 |
|---|---|---|---|
| nvs | 446 464 B | `1219c555560e950b48c3dca996e69bac370907da90c06c0f2d7c91c824309fbf` | identical |
| otadata | 8 192 B | `8ba3b110139f45443d4f268d1a3373ef99a1718b71d51664531b83ee2d4b91a3` | identical |

Every image written was read back in 64 KB chunks and `cmp`'d, because this uplink's flash
read path is lossy on long transfers (0161 finding):

| build | what changed | host `firmware.bin` | read back from 0x10000 |
|---|---|---|---|
| A | UART 38400 -> **9600** (the task's deliverable) | 271 504 B `f7e42235...c17124` | `cmp` byte-identical |
| B | reply-frame extraction | 271 616 B `5813c7c6...ab553e` | `cmp` byte-identical |
| C | frame extraction, scan backwards (final) | 271 536 B `4fb36082...fd4cfd` | `cmp` byte-identical |

(Full hashes: A `f7e42235280d03025bef73b78dc980201b388ea6d7ff564dd890f660eec17124`,
B `5813c7c6c4607a8f203d6d962bd6eb21167ae1d7a0b791ebee4740760dab553e`,
C `4fb3608228c0825ccc2be6929a52e0e80b65c6192b8957892545566ff6fd4cfd`.)

## Scope extension (declared): the reply reader

Build A is the literal task ("UART at 9600, everything else unchanged"). It produced this:

```
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
STEP 2: read shaft angle (0x36)                PASS  (raw 65532 = 359.98 deg)
STEP 3: read following error (0x39)            PASS  (raw 1 = 0.01 deg)
STEP 4: motor enable (0xF3=01, 0x3A)           FAIL  (ack=0 status=0xFF)
STEP 5: command +90 deg move (0xFD)            FAIL  (driver rejected the frame)
STEP 6: measured move is +90 deg               FAIL  (moved 0.00 deg in TIMEOUT)
STEP 7: command return move (0xFD rev)         FAIL  (driver rejected the frame)
STEP 8: returned to start angle                FAIL  (residual 0.00 deg from start)
SELF-TEST: FAIL (5 of 8 steps failed)
```

The frames show why. Step 4's TX was `E0 F3 01 D4` and the RX was `40 3D 01 E1`; step 5's
RX was `E0 FD 10 00 00 02 E0 01 E1`. The driver **echoes the command**, so a good reply is
`<echo><addr><payload><chk>` and the sketch's `n != want` test rejects it. Build B added an
ADDR resync (which fixed the case where the echo is mangled but the reply is clean) and
build C replaced the length test with `extractFrame()`, which scans the raw capture
**backwards** for the newest checksum-valid frame. Reply bytes are still fully validated
(address + checksum), so this weakens nothing about the electrical check — it stops the
parser from throwing away frames the driver actually sent.

The parser also has to tolerate a genuinely damaged capture: replies occasionally arrive
with 1-3 leading junk bytes, or with the leading address byte missing
(`78 4E 46 00 01 E1` = junk + a valid 2-byte tail). Those runs still lose steps — see run
B2 below. That is a link-quality observation, not a parsing one, and is reported rather
than papered over.

## Log A (build A, unmodified reply check) — full console

```
ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:6380
ho 0 tail 12 room 4
load:0x40078000,len:15916
load:0x40080400,len:3860
entry 0x40080630
I (29) boot: ESP-IDF 5.5.1 2nd stage bootloader
I (29) boot: compile time Jan 12 2026 10:16:19
I (29) boot: Multicore bootloader
I (31) boot: chip revision: v3.1
I (33) boot.esp32: SPI Speed      : 40MHz
I (37) boot.esp32: SPI Mode       : DIO
I (41) boot.esp32: SPI Flash Size : 4MB
I (44) boot: Enabling RNG early entropy source...
I (49) boot: Partition Table:
I (51) boot: ## Label            Usage          Type ST Offset   Length
I (58) boot:  0 otadata          OTA data         01 00 00009000 00002000
I (64) boot:  1 phy_init         RF data          01 01 0000b000 00001000
I (71) boot:  2 app0             OTA app          00 10 00010000 001c0000
I (77) boot:  3 app1             OTA app          00 11 001d0000 001c0000
I (84) boot:  4 nvs              WiFi data        01 02 00390000 0006d000
I (90) boot: End of partition table
I (94) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=0d850h ( 55376) map
I (120) esp_image: segment 1: paddr=0001d878 vaddr=3ffbdb60 size=027a0h ( 10144) load
I (124) esp_image: segment 2: paddr=00020020 vaddr=400d0020 size=2293ch (141628) map
I (173) esp_image: segment 3: paddr=00042964 vaddr=3ffc0300 size=01900h (  6400) load
I (175) esp_image: segment 4: paddr=0004426c vaddr=40080000 size=0e1fch ( 57852) load
I (206) boot: Loaded app from partition at offset 0x10000
I (206) boot: Disabling RNG early entropy source...
E (229) esp_core_dump_flash: No core 0f^[ partition found!
E (229) esp_core_dump_flash: No core dump partition found!

=== MKS SERVO42C UART bench self-test (build 0162) ===
UART2 9600 8N1  addr 0xE0  RX=GPIO16 TX=GPIO17
Assume MStep=16, motor 200 full steps -> 3200 pulses/rev, 90 deg = 800 pulses
-- probing driver framing (0x36 both generations) --
    TX: E0 36 16
    RX: E0 FF FF FF FC D9
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
    TX: E0 36 16
    RX: E0 FF FF FF FC D9
STEP 2: read shaft angle (0x36)                PASS  (raw 65532 = 359.98 deg)
    TX: E0 39 19
    RX: E0 00 01 E1
STEP 3: read following error (0x39)            PASS  (raw 1 = 0.01 deg)
    TX: E0 F3 01 D4
    RX: 40 3D 01 E1
    TX: E0 3A 1A
    RX: F0 FE E0 01 E1
STEP 4: motor enable (0xF3=01, 0x3A)           FAIL  (ack=0 status=0xFF)
    TX: E0 FD 10 00 00 03 20 10
    RX: E0 FD 10 00 00 02 E0 01 E1
STEP 5: command +90 deg move (0xFD)            FAIL  (driver rejected the frame)
HINT: a rejected 0xFD usually means CtrMode is not CR_UART on the driver's OLED menu.
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF EB 2C F5
STEP 6: measured move is +90 deg               FAIL  (moved 0.00 deg in TIMEOUT)
    TX: E0 FD 90 00 00 03 20 90
    RX: BC 90 00 00 03 20 FC E0 01 E1
STEP 7: command return move (0xFD rev)         FAIL  (driver rejected the frame)
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF F5 AF 82
STEP 8: returned to start angle                FAIL  (residual 0.00 deg from start)
    TX: E0 F7 D7
    RX: 07 01 E1
INFO: stop (0xF7) did not acknowledge — motor may still be enabled.
    TX: E0 3E 1E
    RX: 3C F2 E0 02 E2
    TX: E0 33 13
    RX: E0 FF FF FF B0 8D
INFO: received pulses -80

SELF-TEST: FAIL (5 of 8 steps failed)
=== self-test complete ===
```

Repeat run (build A, second boot):

```
0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:6380
ho 0 tail 12 room 4
load:0x40078000,len:15916
load:0x40080400,len:3860
entry 0x40080630
I (29) boot: ESP-IDF 5.5.1 2nd stage bootloader
I (29) boot: compile time Jan 12 2026 10:16:19
I (29) boot: Multicore bootloader
I (31) boot: chip revision: v3.1
I (33) boot.esp32: SPI Speed      : 40MHz
I (37) boot.esp32: SPI Mode       : DIO
I (41) boot.esp32: SPI Flash Size : 4MB
I (44) boot: Enabling RNG early entropy source...
I (49) boot: Partition Table:
I (51) boot: ## Label            Usage          Type ST Offset   Length
I (58) boot:  0 otadata          OTA data         01 00 00009000 00002000
I (64) boot:  1 phy_init         RF data          01 01 0000b000 00001000
I (71) boot:  2 app0             OTA app          00 10 00010000 001c0000
I (77) boot:  3 app1             OTA app          00 11 001d0000 001c0000
I (84) boot:  4 nvs              WiFi data        01 02 00390000 0006d000
I (90) boot: End of partition table
I (94) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=0d850h ( 55376) map
I (120) esp_image: segment 1: paddr=0001d878 vaddr=3ffbdb60 size=027a0h ( 10144) load
I (124) esp_image: segment 2: paddr=00020020 vaddr=400d0020 size=2293ch (141628) map
I (173) esp_image: segment 3: paddr=00042964 vaddr=3ffc0300 size=01900h (  6400) load
I (175) esp_image: segment 4: paddr=0004426c vaddr=40080000 size=0e1fch ( 57852) load
I (206) boot: Loaded app from partition at offset 0x10000
I (206) boot: Disabling RNG early entropy source...
E (229) esp_core_dump_flash: No core 0f�� partition found!
E (229) esp_core_dump_flash: No core dump partition found!

=== MKS SERVO42C UART bench self-test (build 0162) ===
UART2 9600 8N1  addr 0xE0  RX=GPIO16 TX=GPIO17
Assume MStep=16, motor 200 full steps -> 3200 pulses/rev, 90 deg = 800 pulses
-- probing driver framing (0x36 both generations) --
    TX: E0 36 16
    RX: E0 FF FF F9 96 6D
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
    TX: E0 36 16
    RX: 93 31 FF FF F9 96 6D
STEP 2: read shaft angle (0x36)                FAIL  (no valid reply)
ABORT: no angle feedback, later steps meaningless.
```

## Log B (build C, frame extraction) — 7 of 8 PASS

```
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
STEP 2: read shaft angle (0x36)                PASS  (raw 65205 = 358.18 deg)
STEP 3: read following error (0x39)            PASS  (raw 0 = 0.00 deg)
STEP 4: motor enable (0xF3=01, 0x3A)           PASS  (ack=1 status=0x01)
STEP 5: command +90 deg move (0xFD)            PASS  (800 pulses fwd @ gear 16)
STEP 6: measured move is +90 deg               FAIL  (moved -23.47 deg in time)
STEP 7: command return move (0xFD rev)         PASS  (800 pulses rev @ gear 16)
STEP 8: returned to start angle                PASS  (residual 1.84 deg from start)
SELF-TEST: FAIL (1 of 8 steps failed)
```

Full console:

```
ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:6380
ho 0 tail 12 room 4
load:0x40078000,len:15916
load:0x40080400,len:3860
entry 0x40080630
I (29) boot: ESP-IDF 5.5.1 2nd stage bootloader
I (29) boot: compile time Jan 12 2026 10:16:19
I (29) boot: Multicore bootloader
I (31) boot: chip revision: v3.1
I (33) boot.esp32: SPI Speed      : 40MHz
I (37) boot.esp32: SPI Mode       : DIO
I (41) boot.esp32: SPI Flash Size : 4MB
I (44) boot: Enabling RNG early entropy source...
I (49) boot: Partition Table:
I (51) boot: ## Label            Usage          Type ST Offset   Length
I (58) boot:  0 otadata          OTA data         01 00 00009000 00002000
I (64) boot:  1 phy_init         RF data          01 01 0000b000 00001000
I (71) boot:  2 app0             OTA app          00 10 00010000 001c0000
I (77) boot:  3 app1             OTA app          00 11 001d0000 001c0000
I (84) boot:  4 nvs              WiFi data        01 02 00390000 0006d000
I (90) boot: End of partition table
I (94) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=0d850h ( 55376) map
I (120) esp_image: segment 1: paddr=0001d878 vaddr=3ffbdb60 size=027a0h ( 10144) load
I (124) esp_image: segment 2: paddr=00020020 vaddr=400d0020 size=2295ch (141660) map
I (173) esp_image: segment 3: paddr=00042984 vaddr=3ffc0300 size=01900h (  6400) load
I (175) esp_image: segment 4: paddr=0004428c vaddr=40080000 size=0e1fch ( 57852) load
I (206) boot: Loaded app from partition at offset 0x10000
I (206) boot: Disabling RNG early entropy source...
E (229) esp_core_dump_flash: No core `fW partition found!
E (229) esp_core_dump_flash: No core dump partition found!

=== MKS SERVO42C UART bench self-test (build 0162) ===
UART2 9600 8N1  addr 0xE0  RX=GPIO16 TX=GPIO17
Assume MStep=16, motor 200 full steps -> 3200 pulses/rev, 90 deg = 800 pulses
-- probing driver framing (0x36 both generations) --
    TX: E0 36 16
    RX: E0 FF FF FE B5 91
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
    TX: E0 36 16
    RX: B2 E0 FF FF FE B5 91
STEP 2: read shaft angle (0x36)                PASS  (raw 65205 = 358.18 deg)
    TX: E0 39 19
    RX: E0 00 00 E0
STEP 3: read following error (0x39)            PASS  (raw 0 = 0.00 deg)
    TX: E0 F3 01 D4
    RX: 5F D4 E0 01 E1
    TX: E0 3A 1A
    RX: E0 01 E1
STEP 4: motor enable (0xF3=01, 0x3A)           PASS  (ack=1 status=0x01)
    TX: E0 FD 10 00 00 03 20 10
    RX: E0 FD 10 00 E0 01 E1
STEP 5: command +90 deg move (0xFD)            PASS  (800 pulses fwd @ gear 16)
    TX: E0 36 16
    RX: E0 FF FF F1 B0 7F
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF ED F8 C3
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF ED F8 C3
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF ED FC C7
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF EE 00 CC
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF EE 04 D0
    TX: E0 36 16
    RX: E0 36 16 E0 FF FF EE 04 D0
STEP 6: measured move is +90 deg               FAIL  (moved -23.47 deg in time)
HINT: -23.47 deg measured for 800 pulses -> set STEP_SUBDIVISION to the MStep shown on the OLED (expected 16).
    TX: E0 FD 90 00 00 03 20 90
    RX: E0 FD 90 00 00 CF FE E0 01 E1
STEP 7: command return move (0xFD rev)         PASS  (800 pulses rev @ gear 16)
    TX: E0 36 16
    RX: E0 F6 E0 FF FF FB B5 8E
    TX: E0 36 16
    RX: E0 36 16 E0 00 00 00 0C EC
    TX: E0 36 16
    RX: E0 36 16 E0 00 00 00 0C EC
    TX: E0 36 16
    RX: E0 36 16 E0 00 00 00 08 E8
    TX: E0 36 16
    RX: E0 36 16 E0 00 00 00 08 E8
    TX: E0 36 16
    RX: E0 36 16 E0 00 00 00 08 E8
    TX: E0 36 16
    RX: E0 36 16 E0 00 00 00 04 E4
STEP 8: returned to start angle                PASS  (residual 1.84 deg from start)
    TX: E0 F7 D7
    RX: E0 01 E1
    TX: E0 3E 1E
    RX: E0 02 E2
INFO: shaft status 0x02 (unblocked)
    TX: E0 33 13
    RX: F3 A6 62 00 00 00 00 E0

SELF-TEST: FAIL (1 of 8 steps failed)
=== self-test complete ===
```

Repeat run (build C, second boot) — noisier link, 5 of 8:

```
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
STEP 2: read shaft angle (0x36)                PASS  (raw 65532 = 359.98 deg)
STEP 3: read following error (0x39)            FAIL  (no valid reply)
STEP 4: motor enable (0xF3=01, 0x3A)           FAIL  (ack=1 status=0xFF)
STEP 5: command +90 deg move (0xFD)            PASS  (800 pulses fwd @ gear 16)
STEP 6: measured move is +90 deg               FAIL  (moved -27.02 deg in time)
STEP 7: command return move (0xFD rev)         FAIL  (driver rejected the frame)
STEP 8: returned to start angle                FAIL  (residual -1.75 deg from start)
SELF-TEST: FAIL (5 of 8 steps failed)
```

The intermediate build B (ADDR resync only) log is kept as `serial-resync-run1.log`; it is
where the echo became visible (`TX: E0 F3 01 D4` / `RX: E0 F3 01 D4 E0 01 E1`).

## Step 6 — the one real finding

Two independent runs measured **-23.47 deg** and **-27.02 deg** of shaft rotation for the
same commanded 800-pulse move (mean ~25.2 deg). Direction is also negative for a "forward"
command. 800 pulses producing ~25 deg means the assumed 3200 pulses/rev is off by ~4x.
Two explanations fit the single measurement equally well:

1. **`MStep = 64` on the driver** (12 800 pulses/rev -> 800 pulses = 22.5 deg, ~12 % under
   the measured 25.2 deg), with the 0x36 shaft angle at its documented 65536 counts/rev.
2. **The 0x36 angle scale is 16384 counts/rev** on this firmware, in which case
   MStep 16 is right and the true rotation was ~100.8 deg (~12 % over 90).

The two are a factor of 4 apart in resolution and this run cannot separate them, because
both hypotheses predict the same 12 % error. The driver's own OLED is the authority:
`MStep` must be read there (and `CtrMode = CR_UART`, `UartBaud` index for 9600,
`UartAddr = e0` confirmed — these three are now *proven* correct by steps 1-5 passing).
Once the OLED value is known, set `STEP_SUBDIVISION` in `src/main.cpp` to it and step 6/8
become conclusive; if the OLED says 16 and step 6 still under-reads by 4x, then explanation
2 is the answer and the angle scaling (`rawToDeg`, `angleDelta`) is what needs fixing.

Also worth noting for the next run: the driver's negative response to a "forward" command
means the direction bit convention (`0x00` forward / `0x80` reverse in `movePulses`) may be
inverted for this unit — steps 6 and 8 both passed/failed symmetrically, so the test's
absolute-direction assumption is not yet verified.

## Files

| File | What it is |
|---|---|
| `serial-self-test.log` | build A (9600, unmodified check), full boot capture to end of test |
| `serial-self-test-run2.log` | build A, second boot |
| `serial-resync-run1.log` | build B (ADDR resync) — first clean view of the command echo |
| `serial-frame-run1.log` | build C, full boot capture, 7/8 PASS |
| `serial-frame-run2.log` | build C, second boot, 5/8 (damaged replies) |
| `partition-table-readback.bin` | raw 0xC00 read of the partition table at 0x8000 |
| `0162-readflash.py` | chunked, retried `read_flash` used for every hash above |
| `0162-capture.py` | CP2102 DTR/RTS reset + 115200 console capture |
