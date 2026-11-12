---
task_id: '0178'
protocol_version: 1.2.0
status: staged
iteration: 1
proof:
  - branch: hermes/0178-s3-gate
    sha: 5995610c6f0df46c7f9902744f02d6d85ef0a103
    repo: pagosacabin/cabin-play
    files:
      - living-frame/firmware/gate-fabgl-s3/README.md
      - living-frame/firmware/gate-fabgl-s3/platformio.ini
      - living-frame/firmware/gate-fabgl-s3/platformio-shim.ini
      - living-frame/firmware/gate-fabgl-s3/src/main.cpp
      - living-frame/firmware/gate-fabgl-s3/shim/soc/frc_timer_reg.h
      - living-frame/firmware/gate-fabgl-s3/logs/build-1-no-shim.log
      - living-frame/firmware/gate-fabgl-s3/logs/build-2-with-shim.log
      - living-frame/firmware/gate-fabgl-s3/bench/probe-psram.cpp
      - living-frame/firmware/gate-fabgl-s3/bench/probe-platformio.ini
      - living-frame/firmware/gate-fabgl-s3/bench/capture_boot.py
      - living-frame/firmware/gate-fabgl-s3/bench/identity-and-ports.log
      - living-frame/firmware/gate-fabgl-s3/bench/console-boot-psram.txt
      - living-frame/firmware/gate-fabgl-s3/bench/console-boot-psram-octal.txt
      - living-frame/firmware/gate-fabgl-s3/bench/console-probe-no-psram-init.txt
      - living-frame/firmware/gate-fabgl-s3/.gitignore
  - run: none — cabin-play carries no CI (`.github/workflows` has 0 files on main), so no workflow run can exist for this branch; stated in place of a run URL as the task's Proof section allows
notes: |
  GATE FAILED — step 1 stops task 0178, and the alternative was not built, per the
  task's own stop clause. Nothing was added under photo-display-s3; the classic
  projects are untouched.

  TWO INDEPENDENT BLOCKERS, not one:

  1. FabGL 1.0.9 has no ESP32-S3 support. It fails to compile at fabutils.h:45
     (unconditional include of the ESP32-only soc/frc_timer_reg.h; the tree has zero
     CONFIG_IDF_TARGET/ESP32S3 guards). Stubbing that header still leaves 17 errors
     (PS/2 sensor registers SENS_*/RTC_CNTL_SAR_INT_ST, a conflicting ROM lldesc.h).
     The structural reason: FabGL's VGA scanout is I2S in LCD/parallel mode
     (swgenerator.cpp: I2S1.conf2.lcd_en = 1), and the S3's i2s_struct.h has no conf2
     register at all — parallel RGB on the S3 is the LCD_CAM peripheral. Upstream
     issue #330 / discussion #312 die at the same header; there is no S3 port.
     Custom pins DO exist as a documented 8-GPIO begin() overload and
     nativePixelFormat() is SBGR2222 (64 colours), so the pin map itself is fine.
     PSRAM is not a FabGL capability at all: the viewport is MALLOC_CAP_DMA and
     grep for SPIRAM/ps_malloc/MALLOC_CAP_SPIRAM over src/ returns 0 hits.

  2. The Heltec on the bench has NO PSRAM. esptool identification: the ESP32-S3 on
     /dev/ttyUSB1 is MAC 9c:13:9e:a1:37:40, QFN56 rev v0.2, 8 MB embedded flash (GD)
     — an ESP32-S3FN8, not the S3R2 the task assumes. A 25-line identification probe
     reports psram size 0 bytes and the ROM logs "PSRAM ID read error: 0x00ffffff,
     PSRAM chip not found"; rebuilt for octal PSRAM (qio_opi) the result is the same
     ("spiram: SPI RAM enabled but initialization failed"). 640x480 at 1 byte/pixel is
     307,200 B against a 335,860 B largest DMA block and a 399,188 B total heap, so a
     64-colour 640x480 frame buffer cannot live in this board's RAM either.

  ALTERNATIVES, evidenced but NOT built (decision owed, see the reply body):
   (1) LCD_CAM scanout on a PSRAM-equipped S3, modelled on PicoMite's
       vga_lcdcam_s3.c (640x480@60, RGB332 byte/pixel in PSRAM, internal bounce
       buffers, per-line data_gpio[] with -1 for unused lines so the 6-bit ladder maps
       directly). Needs a new driver AND a board with PSRAM — a purchase, which the
       task forbids, so it cannot proceed without a decision.
   (2) Same board at 640x480/16 colours (4 bpp = 153,600 B, fits); same driver work,
       no purchase.
   (3) Stay on FabGL and the classic ESP32 at 640x480/16 colours (VGA4Controller), no
       new driver, 16 colours.

  DEVIATIONS: (a) I flashed the target S3 with the identification probe, overwriting
  whatever firmware it held (content unknown, not preserved) — necessary because the
  PlatformIO board JSON has no PSRAM flags, so a build without -DBOARD_HAS_PSRAM
  reports 0 regardless; both quad and octal PSRAM modes were measured. (b) The
  evidence lives in a new directory living-frame/firmware/gate-fabgl-s3/, which is not
  photo-display-s3. (c) The shim build is a diagnostic only; its stub values are never
  executed. (d) Queue discipline: active/ 0159 (decision-blocked) and 0142
  (superseded), inbox/ 0176 (withdrawn) were all left untouched; 0178 was the only
  actionable task.

  COST: this cron profile's harness cannot see per-call DeepSeek usage objects, so the
  token and cost fields are `unknown` (per protocol) rather than estimated.
  `search_requests: 5` is the real count of web searches this run made (3 + 2).
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 5
  rate_window: off-peak
  rates_used: "unknown — the cron harness exposes no per-call usage object; nothing estimated"
  cost_usd: unknown
---

# 0178 — Living frame rung 3: S3 port, 640×480 in 64 colours (Heltec LoRa 32 V3)

## Assignment
- Hermes: all steps — bench firmware on Stephen's machine (build, flash the Heltec, verify).
- Newton: not assigned.
- Juno: verifies the staged reply; Stephen's photos of the screen are the acceptance gate.

## Context
Rungs 0–2 ran on a classic ESP32-WROOM devkit + a commercial VGA breakout that turned out to be a 2-bit-per-channel resistor DAC (per-channel 270 Ω = MSB landing, 470 Ω = LSB landing, buzz-verified by Stephen). Rung 2 (task 0177, cabin-play `living-frame/firmware/photo-display-64`) proved 64 colours work, but the classic chip's RAM forced 320×240 — Stephen's verdict on the result: still pixelated. His fix: move the frame to the **Heltec WiFi LoRa 32 V3** from his parts drawer (ESP32-S3R2 — 512 KB SRAM + **2 MB PSRAM**), bench-confirmed alive 2026-11-12. The DAC board and the 213T monitor carry over unchanged. At 640×480 and 1 byte/pixel the frame buffer is 307,200 B — it belongs in PSRAM.

Stephen has ALREADY WIRED the Heltec to the agreed pin map below. The map dodges everything the Heltec has soldered down: LoRa SX1262 on GPIO 8–14, OLED on 17/18/21/36, LED 35, battery sense 1/37, USB-UART 43/44, PRG button 0.

## Agreed pin map (FIXED — Stephen and Juno, 2026-11-12; Stephen has wired to this)
| Signal | GPIO |
|---|---|
| R MSB | 4 |
| R LSB | 5 |
| G MSB | 6 |
| G LSB | 7 |
| B MSB | 19 |
| B LSB | 20 |
| HSync | 38 |
| VSync | 39 |

DAC landings unchanged: ~270 Ω to the VGA pin = MSB, ~470 Ω = LSB, per channel (R = VGA pin 1, G = pin 2, B = pin 3).

## Task
1. **STEP ONE — FabGL-on-S3 gate, before building anything.** Verify the pinned FabGL (1.0.9, as in 0177 — or a newer pin you justify in the reply) supports, on ESP32-S3: the bitmapped 64-colour VGA controller, CUSTOM pins via the documented overload, and a frame buffer allocated in PSRAM at 640×480. Cite the header/doc lines AND get a minimal S3 VGA build compiling for the Heltec target. If any of the three is not there, STOP: stage a reply with what you found and the best alternative you can evidence — do not build the alternative unasked.
2. **New project** `living-frame/firmware/photo-display-s3` in cabin-play. Copy the assets from `photo-display-64` (Blue Marble `photo.jpg`, lorikeet `parrot.jpg`, the ramp screen from `src/ramp_screen.h`, and the round-2 light-domain (gamma) dither pipeline — it measurably killed the near-black speckle, keep it). Do not modify the classic-ESP32 projects.
3. **Mode: 640×480@60 Hz, 64 colours, frame buffer in PSRAM.** The JPEGs are natively 640×480 — decode at full scale (no 1/2 descale as in 0177). Console must print where the frame buffer lives and its size, plus free heap after draw.
4. **Display cycle:** ramp screen first (30 s, same layout as 0177 — per-channel 4-step ramps in the order 0 / LSB / MSB / both, grey ramp, gradient), then lorikeet, then Blue Marble, ~20 s each, looping. State the cycle in the reply.
5. **Flash the Heltec yourself.** Identify it first (esptool chip_id / MAC, in the proof) — other ESP32s may be attached; do NOT flash the classic WROOM (ttyUSB*, MAC 4c:c3:82:da:d5:68) or any other board.
6. **Console capture + host render checks** at 640×480 for both photos (distinct palette colours used, RMSE/NCC vs source as in 0175/0177), committed with the project.

## Success criteria
- Clean build for the ESP32-S3 (Heltec V3) target; the gate in step 1 evidenced, not asserted.
- Console shows a 640×480 frame buffer in PSRAM, both photos drawn, heap reported.
- Ramp → lorikeet → Earth cycle running on the board Stephen wired.
- Acceptance is Stephen's photos: ramp steps in order, and his verdict on the photos at 640×480.

## Constraints
- The pin map above is fixed. If FabGL-on-S3 cannot drive any one of those pins, stop and report the exact pin and the evidence — do not substitute pins: Stephen has already wired to this map.
- Do not initialise or drive the LoRa radio or the OLED (no Vext/OLED/LoRa code) — they stay idle.
- cabin-play repo only; classic projects untouched; no purchases; nothing outside this project.

## Proof
- Commit on cabin-play `main` (branch + SHA Juno can re-fetch), file list including the build log, console capture, and both render-check images.
- run: none (cabin-play has no CI — say so, as before).

## Reply format
Staged reply per protocol 1.2.0: front-matter with status/iteration/cost block, proof pointers, and notes covering the step-1 gate evidence, the display cycle as built, console figures, and anything Stephen should know before photographing.

---

# Reply — GATE FAILED (2026-11-12 15:25 MST), step 1 stops the task

**status:** `staged` — the task's own step-1 clause applies: *"If any of the three is
not there, STOP: stage a reply with what you found and the best alternative you can
evidence — do not build the alternative unasked."* No firmware was written, no
`photo-display-s3` project was created, and the classic projects were not touched.

**Outcome in one line:** FabGL 1.0.9 **does not build for ESP32-S3 at all** — its VGA
scanout is I2S in LCD/parallel mode, a function the S3 moved to the LCD_CAM
peripheral — and the Heltec board on the bench has **no PSRAM** (0 bytes, verified in
two line modes), so 640×480 at 64 colours has nowhere to live on this part even with a
different library.

## The gate, criterion by criterion

| Step-1 criterion | Verdict | Evidence |
|---|---|---|
| 64-colour bitmapped VGA controller on ESP32-S3 | **ABSENT** | does not compile; scanout mechanism does not exist on S3 |
| Custom pins via the documented overload | present (but moot) | `vgacontroller.h` documents the 8-GPIO `begin()` and `nativePixelFormat()` is `SBGR2222` (6-bit colour) |
| Frame buffer in PSRAM at 640×480 | **ABSENT** | FabGL has no PSRAM path at all; *and* this board has no PSRAM |

### 1. It does not compile for the target

`living-frame/firmware/gate-fabgl-s3` is a minimal project — `fabgl::VGAController VGA;`
plus `VGA.begin(4,5,6,7,19,20,38,39)` and `setResolution(VGA_640x480_60Hz)` — built for
`board = heltec_wifi_lora_32_V3` on `espressif32@6.12.0` (arduino-esp32 2.0.17, the same
platform as 0177). `logs/build-1-no-shim.log`:

```
src/fabutils.h:45:10: fatal error: soc/frc_timer_reg.h: No such file or directory
```

That header is the classic-ESP32 FRC timer register map (the S3 replaced the FRC timer
with the LAC timer), and the include is unconditional in `fabutils.h` — the library's
core utility header has no S3 path. The tree contains **zero** target guards:
`grep -rn "ESP32S3\|CONFIG_IDF_TARGET_ESP32S3\|ARDUINO_ARCH_ESP32S3" src/` → 0 hits.

### 2. Stubbing that header does not get close

`platformio-shim.ini` puts a stub `soc/frc_timer_reg.h` on the include path (diagnostic
only; the stub's values are never executed). `logs/build-2-with-shim.log`: **17 errors**,
now from the PS/2 driver and the ROM DMA descriptor —

```
ps2controller.cpp:1154: error: 'SENS_SAR_START_FORCE_REG' was not declared in this scope
ps2controller.cpp:1175: error: 'ETS_RTC_CORE_INTR_SOURCE' was not declared in this scope
ps2controller.cpp:1350: error: 'RTC_CNTL_SAR_INT_ST' was not declared in this scope
esp32/rom/lldesc.h:67: error: redefinition of 'struct lldesc_s'
```

### 3. The scanout mechanism itself is ESP32-only

FabGL generates VGA by putting **I2S in LCD/parallel mode**
(`src/devdrivers/swgenerator.cpp:120-141`, reached from
`VGABaseController::startGPIOStream()`):

```c
periph_module_enable(PERIPH_I2S1_MODULE);
// LCD mode
I2S1.conf2.val            = 0;
I2S1.conf2.lcd_en         = 1;
I2S1.conf2.lcd_tx_wrx2_en = 1;
```

The S3's `soc/i2s_struct.h` has **no `conf2` register at all** (`grep -n "conf2\|lcd"`
over the esp32s3 copy → 0 hits); parallel RGB video on the S3 is the separate `LCD_CAM`
block. This is a pixel-path rewrite, not a pin or header fix.

Upstream agrees: `fdivitto/FabGL` issue **#330** and discussion **#312** both die at the
same `frc_timer_reg.h` error, with no S3 port in the project.

### 4. Frame buffer in PSRAM is not a FabGL feature

`vgacontroller.cpp:96-99` allocates the viewport as
`VGABaseController::allocateViewPort(MALLOC_CAP_DMA, …)` →
`vgabasecontroller.cpp:448` `heap_caps_malloc(…, allocCaps)`; `MALLOC_CAP_DMA` is
internal RAM on IDF 4.4. `grep -rn "SPIRAM\|ps_malloc\|MALLOC_CAP_SPIRAM" src/` → **0
hits** — the library has no PSRAM path whatsoever.

## Bench: the board is not an S3R2, and has no PSRAM

Read-only identification (`bench/identity-and-ports.log`, esptool `chip_id` on all three
ports):

| Port | Chip | MAC |
|---|---|---|
| `/dev/ttyUSB1` | **ESP32-S3 (QFN56) rev v0.2, 8 MB embedded flash (GD)** | `9c:13:9e:a1:37:40` ← the Heltec |
| `/dev/ttyUSB0` | ESP32-D0WD-V3 rev v3.1 | `84:1f:e8:30:a8:50` (a classic ESP32, **not** the 0177 board `4c:c3:82:da:d5:68`; not touched) |
| `/dev/ttyACM0` | ESP32-C3 (QFN32) rev v0.4, 4 MB | `9c:cc:01:d0:64:6c` |

Note for whoever flashes next: **the target S3 is on a CP2102 bridge at `/dev/ttyUSB1`,
not on the native USB port** — the Espressif USB-JTAG device plugged in is the C3.

`bench/probe-psram.cpp` (25 lines, no VGA code) was flashed to `/dev/ttyUSB1` and its
boot log captured (`bench/console-boot-psram.txt`):

```
E (90) psram: PSRAM ID read error: 0x00ffffff, PSRAM chip not found or not supported, or wrong PSRAM line mode
chip model      : ESP32-S3
flash size      : 8388608 bytes
psram size      : 0 bytes
psram free      : 0 bytes
heap total      : 399188 bytes
heap free       : 373460 bytes
largest DMA blk : 335860 bytes
```

Rebuilt for octal PSRAM in case it were an R8 part (`bench/console-boot-psram-octal.txt`):
`E (72) opi psram: PSRAM ID read error … chip not found`, `spiram: SPI RAM enabled but
initialization failed. Bailing out.`, `psram size: 0`.

An ESP32-S3 with 8 MB *embedded* flash is the **ESP32-S3FN8** ordering code — flash, no
stacked PSRAM. The task's premise (ESP32-S3R2, 2 MB PSRAM) does not match the part on
the bench.

**Arithmetic:** a 640×480 frame buffer at the 64-colour depth (1 byte/pixel) is
**307,200 B** against a largest internal DMA block of **335,860 B** and a total heap of
**399,188 B** — 91 % of the only block that could hold it, leaving ~28 KB for TJpgDec,
task stacks and everything else. 64 colours cannot be stored at 640×480 on this board,
and there is no PSRAM to store it in.

## Alternatives (evidence attached, nothing built — decision owed)

1. **LCD_CAM scanout on a PSRAM-equipped S3.** The S3's RGB-panel peripheral is the
   right engine and there is a working precedent for exactly this shape: PicoMite's
   `drivers/vga_lcdcam_s3/vga_lcdcam_s3.{c,h}` — *"Generates a 640x480@60 VGA signal out
   of the ESP32-S3 LCD_CAM (RGB panel) … The peripheral owns a single PSRAM frame buffer
   of HRes*VRes bytes (one RGB332 byte per pixel), then feeds LCD DMA through small
   internal-RAM bounce buffers."* Its pin struct takes `data_gpio[8]` individually and
   permits `-1` for unused lines, so a **6-bit** ladder maps straight onto it. Cross
   references: `bitluni/ESP32-S3-VGA` (its README warns *"Higher resolution lose sync on
   PSRAM"* and *"Only compatible with 8MB psram version ESP32 S3 for high resolutions"*)
   and `Cvarc-Xtal/esp32s3-vga` (640×480 only, PSRAM double buffers, needs arduino-esp32
   ≥ 3.1.0-RC1 — this PlatformIO platform pins 2.0.17, so it would need pioarduino).
   Cost: a new driver (~300-500 lines), no FabGL API, **plus a board that has PSRAM** —
   a purchase, or a different part from the drawer. The 0177 render path survives: the
   palette/dither/JPEG glue is already behind a `put()` callback in `photo_render64.h`,
   so only the scanout end changes.
2. **This board, 640×480 at 16 colours.** 640×480×4 bpp = 153,600 B — it fits (largest
   DMA block 335,860 B). Same LCD_CAM driver work, no purchase, 16 colours instead of 64.
   Resolution, which is what Stephen objected to, is fixed.
3. **Stay on FabGL, classic ESP32, 640×480 at 16 colours** (`VGA4Controller`, 4 bpp).
   No new driver, proven library and existing wiring; rung 1 already ran 640×480 on that
   chip, so the memory arithmetic is known-good. 16 colours, not 64.
4. **Port FabGL to the S3.** Not recommended: FRC timer, PS/2 sensor registers and the
   I2S LCD mode are all ESP32-only; upstream has no S3 port and this is fork-scale work.

**Decision owed (Juno/Stephen):** pick between (1) — 64 colours on a PSRAM-equipped S3,
needs a different board — and (2)/(3) — 16 colours on hardware already in hand. The task
explicitly forbids purchases, so (1) cannot proceed without that decision. I did not
build any of them.

## Deviations, all declared

1. **I flashed the target S3**, overwriting whatever firmware it previously held
   (content unknown, not preserved). The flashed image is the 25-line identification
   probe only — no VGA code, no driver. This was necessary: the PlatformIO board JSON
   for `heltec_wifi_lora_32_V3` carries no PSRAM flags, so a probe built without
   `-DBOARD_HAS_PSRAM` reports 0 regardless; I measured both quad (`qio_qspi`) and octal
   (`qio_opi`) PSRAM modes before concluding "no PSRAM".
2. **New directory in cabin-play:** `living-frame/firmware/gate-fabgl-s3/` holds the gate
   project, the shim probe, the build logs, the PSRAM probe and every console capture.
   It is not `photo-display-s3` and does not pre-empt the successor task's project name.
3. **Two builds, not one:** the second (`platformio-shim.ini`) exists solely to prove the
   port is codebase-scale rather than a one-header fix; its stub values are never run.
4. **Queue discipline:** `mailbox/active/` holds 0159 (decision-blocked) and 0142
   (superseded); `mailbox/inbox/` also held 0176 (withdrawn/superseded by 0177). All
   three were left untouched; 0178 was the only actionable task.
5. Not done, by instruction: no `photo-display-s3` project, no assets copied, no
   render-check images, no console capture of a display cycle — all of those depend on a
   working scanout that does not exist yet.
