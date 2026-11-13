---
task_id: '0179'
protocol_version: 1.2.0
status: verified
iteration: 1
expect-reply-within: 48h
proof:
  repo: pagosacabin/cabin-play
  branch: main
  sha: '03161305d28cbbe519a75e0606ea74bc27d45dfd'
  run: none
  files:
    - living-frame/firmware/photo-display-s3/.gitignore
    - living-frame/firmware/photo-display-s3/CMakeLists.txt
    - living-frame/firmware/photo-display-s3/README.md
    - living-frame/firmware/photo-display-s3/bench/build.log
    - living-frame/firmware/photo-display-s3/bench/capture.py
    - living-frame/firmware/photo-display-s3/bench/console-photo-display-s3.txt
    - living-frame/firmware/photo-display-s3/bench/host-render.log
    - living-frame/firmware/photo-display-s3/bench/identity.log
    - living-frame/firmware/photo-display-s3/bench/scanout-test.log
    - living-frame/firmware/photo-display-s3/bench/upload.log
    - living-frame/firmware/photo-display-s3/data/parrot.jpg
    - living-frame/firmware/photo-display-s3/data/photo.jpg
    - living-frame/firmware/photo-display-s3/platformio.ini
    - living-frame/firmware/photo-display-s3/sdkconfig.defaults
    - living-frame/firmware/photo-display-s3/src/CMakeLists.txt
    - living-frame/firmware/photo-display-s3/src/images.h
    - living-frame/firmware/photo-display-s3/src/ladder64.h
    - living-frame/firmware/photo-display-s3/src/main.cpp
    - living-frame/firmware/photo-display-s3/src/ramp_screen.h
    - living-frame/firmware/photo-display-s3/src/scanout.h
    - living-frame/firmware/photo-display-s3/tools/convert_images.py
    - living-frame/firmware/photo-display-s3/tools/host_render.cpp
    - living-frame/firmware/photo-display-s3/tools/img_blue_marble-doubled.png
    - living-frame/firmware/photo-display-s3/tools/img_lorikeet-doubled.png
    - living-frame/firmware/photo-display-s3/tools/make_previews.py
    - living-frame/firmware/photo-display-s3/tools/ramp-doubled.png
    - living-frame/firmware/photo-display-s3/tools/ramp-native.png
    - living-frame/firmware/photo-display-s3/tools/scanout_test.cpp
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
  rates_used: "unknown -- no per-call usage object was visible to this harness"
  cost_usd: unknown
notes: |
  STAGED, build + bench run done. Run 2026-11-12 16:20-16:45 MST (23:20-23:45 UTC),
  off-peak (PEAK: OFF-PEAK 23:15 UTC), PROTOCOL MATCH 1.2.0.

  WHAT IS ON THE BOARD. living-frame/firmware/photo-display-s3, ESP-IDF 5.5 via
  PlatformIO, flashed to the bench Heltec (read_mac 9c:13:9e:a1:37:40 on
  /dev/ttyUSB1, all three images hash-verified). esp_lcd's RGB panel runs in
  no_fb mode: a 76,800 B 320x240 byte buffer in internal RAM, two 10,240 B DRAM
  bounce buffers, and the 2x2 pixel doubling done inside on_bounce_empty. The
  driver accepted the 8-bit bus with the six ladder lines (two set to -1) and the
  640x480 timing; no error path was hit.

  MEASURED, NOT INFERRED. 78 s console capture across all three phases: VSYNC
  59.38-59.77 Hz (mean 59.54 over 15 samples), i.e. an effective pclk of
  59.54 x 800 x 525 = 25.01 MHz against the 25.000 MHz requested; bounce fills
  30.0-30.1 per frame, exactly the 307200/10240 the frame requires, so the DMA
  never starved and the driver's desync-restart path never fired. Frame buffer
  0x3fc99b00, 76,800 B; free heap after init 288,628 B internal (largest block
  245,760 B).

  DEFECT FOUND AND FIXED BEFORE THE FLASH, declared per the protocol. The first
  build's pair-expansion store used 0x00010001 instead of 0x0101, which puts the
  duplicate in the upper half of the word: every second output byte was 0x00, a
  black column between every pixel. The build was clean, the console was clean,
  and the scanout counters were perfect -- only the host unit test that drives
  the shipped ISR code (tools/scanout_test.cpp) caught it. Fixed, rebuilt, and
  re-flashed; the binary on the board is the tested one.

  NOT PROVEN, AND NOT CLAIMED. Whether the 213T locks, whether the sync levels
  are what the monitor wants, and whether the ramp levels look right on the
  panel. Nothing on this host can see the VGA cable -- that is Stephen's
  photographs plus his word, the acceptance gate this task names. The staged
  proof is of build, flash, scanout timing and pixel content only.

  DEVIATIONS. (1) The file arrived with `status: unassigned` (not one of the five
  protocol states); normalised to in_progress at pickup, as recorded then.
  (2) `pclk` and `de` are routed to no GPIO: a VGA DE-15 carries R/G/B/HS/VS
  only, so a 25 MHz pin would be EMI. The eight agreed pins are untouched.
  (3) The doubling was factored into src/scanout.h so the host test can compile
  the same code -- test tooling, not a firmware behaviour change.
  (4) cabin-play main was at c03901e with the 0178 gate commit (5995610) sitting
  only on branch hermes/0178-s3-gate; landing 0179 on main therefore
  fast-forwarded main to 0316130 and carried the 0178 gate tree with it.
  (5) sdkconfig.heltec-s3 is gitignored: PlatformIO reuses it and ignores later
  sdkconfig.defaults edits.

  RUN: none, earned. `ls .github` in cabin-play: no such directory; `git ls-tree
  -r origin/main | grep .github/workflows` -> 0 files; `gh run list -R
  pagosacabin/cabin-play` -> empty. There is no CI in this repo to point at.

  NTFY: no receipt, and none is owed here. The task's proof block names no topic
  and no artifact publication, and cabin-play has no workflow that produces a
  downloadable build artifact -- the image is the local `pio run` output, whose
  flash is recorded in bench/upload.log.

  COST: the harness exposed no per-call usage object for this run, so every token
  field is `unknown` rather than estimated. Wall clock 16:20-16:45 MST, all
  off-peak.
---

# 0179 — Living frame rung 3a: prove an LCD_CAM VGA driver on the Heltec V3 (320×240, 64 colours, pixel-doubled)

## Assignment
- Hermes: all steps — bench firmware on Stephen's machine (build, flash the Heltec, verify).
- Newton: not assigned.
- Juno: verifies the staged reply; Stephen's photos of the screen are the acceptance gate.

## Context
Task 0178's gate (staged, verified) established two facts: (1) FabGL is classic-ESP32 only — its VGA is the classic's I2S LCD mode, which does not exist on the S3; parallel RGB on the S3 is the **LCD_CAM** peripheral. (2) The bench Heltec WiFi LoRa 32 V3 is an **ESP32-S3FN8 — no PSRAM** (esptool MAC 9c:13:9e:a1:37:40; probe: psram 0 bytes). So 640×480 at 64 colours (307,200 B) cannot live in this board's RAM.

Stephen's decision (2026-11-12): prove the LCD_CAM driver FIRST at a mode this board can hold, then step up. This task is the driver proof only. The sequel (a later task) is 640×480 at 16 colours on this same board; a PSRAM-equipped S3 for full 640×480/64 remains the exit after that if 16 colours disappoints.

The DAC board and Stephen's wiring carry over: commercial 2-bit/channel VGA breakout (270 Ω landing = MSB, 470 Ω = LSB per channel), SyncMaster 213T monitor.

## Agreed pin map (FIXED — Stephen has wired to this)
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

## Task
1. **Write an LCD_CAM-based VGA scanout driver** for the ESP32-S3, modelled on the evidenced references from the 0178 gate: PicoMite's `vga_lcdcam_s3.c` (per-line `data_gpio[]` mapping with unused lines off, so the 6-bit ladder maps directly) and the S3-MSX-PC project (640×480@60 via LCD_CAM, R-2R ladder). ESP-IDF `esp_lcd` / LCD_CAM APIs are the foundation; no FabGL, no ESP32Lib.
2. **Mode: standard 640×480@60 Hz VGA timing carrying a 320×240 frame buffer, pixel-doubled** (each buffer pixel → 2×2 screen pixels), 64 colours, one byte per pixel (R2G2B2 layout matching the pin map). Buffer = 76,800 B in internal RAM. The monitor must see a genuine 640×480@60 signal and lock.
3. **Content cycle, same as 0178:** ramp screen first (30 s — per-channel 4-step ramps in the order 0 / LSB / MSB / both, grey ramp, gradient; reuse the layout from `photo-display-64/src/ramp_screen.h`), then the lorikeet, then the Blue Marble, ~20 s each, looping.
4. **Image pipeline — your choice, stated in the reply:** either adapt the on-device TJpgDec path from photo-display-64, or pre-convert on the laptop and embed byte arrays. If pre-converting: quantise to the ladder's MEASURED levels (0 / 0.452 / 0.714 / 1.000 of full scale — the 270/470 Ω pair is not evenly spaced, do not use 0/85/170/255) and dither in linear light, per the 0177 round-2 finding (light-domain dither measurably killed the near-black speckle).
5. **Flash the Heltec yourself** — identify it first (MAC 9c:13:9e:a1:37:40). Do NOT flash the classic WROOM (MAC 4c:c3:82:da:d5:68) or any other attached board.
6. **Console capture:** mode/timing, frame buffer address + size, free heap after init. Plus a host-side render check of the doubled output if the pipeline allows it.

## Success criteria
- The 213T locks a stable 640×480@60 signal from the S3: no rolling, tearing, or sync loss over the full cycle (Stephen's photos + his word).
- Ramp screen renders with steps in the wired order on every channel — driver, pin map, and DAC proven together on S3 silicon.
- This rung does NOT need to beat 0177's look (same unique resolution) — it proves the driver. Acceptance is sync lock + ramp order + the photos displaying recognisably.

## Constraints
- Pin map fixed; if LCD_CAM cannot drive any one of those pins, stop and report the exact pin and the evidence rather than substituting.
- Do not initialise or drive the LoRa radio or the OLED.
- New project `living-frame/firmware/photo-display-s3` in cabin-play; classic projects untouched; no purchases; nothing outside this project.
- If LCD_CAM pixel-doubled output at 640×480 timing proves impossible on this chip, stop and stage the evidence — do not fall back to a different mode unasked.

## Proof
- Commit on cabin-play `main` (branch + SHA Juno can re-fetch), file list including the build log and console capture.
- run: none (cabin-play has no CI — say so).

## Reply format
Staged reply per protocol 1.2.0: front-matter with status/iteration/cost block, proof pointers, and notes covering the driver approach (which reference, what you changed), the image pipeline choice, console figures, and anything Stephen should know before photographing.

---

# Reply — rung 3a driver proof (Hermes, iteration 1)

## Verdict

An LCD_CAM VGA scanout is running on the Heltec now: 640×480@60 timing carrying a
320×240 frame that is pixel-doubled in the DMA ISR, cycling ramp → lorikeet → Blue
Marble. Everything the machine can check is checked and quoted below. Whether the
213T locks on it is Stephen's photograph, and this reply does not claim it.

## Driver approach — which reference, what changed

Shape taken from **PicoMite's `drivers/vga_lcdcam_s3/vga_lcdcam_s3.{c,h}`**, the
precedent the 0178 gate evidenced: one RGB332 byte per pixel, `data_gpio[8]` with
`-1` for unused lines, small internal-RAM bounce buffers feeding LCD DMA, 640×480@60.

What changed, and why it had to: PicoMite needs **PSRAM** for a full 640×480 byte
frame. This part is an ESP32-S3FN8 with none. So the frame buffer is 320×240 and
`esp_lcd`'s `no_fb` mode does the rest — the driver allocates no screen-sized
buffer and calls `on_bounce_empty` for each 10,240-byte chunk; that callback
expands each source byte to two adjacent output bytes and each source line to two
output lines. 307,200 / 10,240 = **30 fills per frame**, which is exactly what the
device counts.

No FabGL, no ESP32Lib, no library patch: plain ESP-IDF 5.5 `esp_lcd` RGB panel,
built with PlatformIO `framework = espidf` (the Arduino core bundles IDF 4.4,
whose RGB driver has neither `no_fb` nor bounce buffers).

## Pin map → data byte

The buffer byte is packed `(B<<4)|(G<<2)|R` with each field `(MSB,LSB)` — rung 2's
packing — so bit *n* goes to `data_gpio_nums[n]` and the byte reaches the ladder
with **no translation**:

```
data_gpio_nums[0..7] = { 5, 4, 7, 6, 20, 19, -1, -1 }
                        R  R  G  G  B   B
                        LS MS LS MS LS  MS
```

so bit0 = R LSB on GPIO5, bit1 = R MSB on GPIO4, bit2 = G LSB on 7, bit3 = G MSB
on 6, bit4 = B LSB on 20, bit5 = B MSB on 19; bits 6–7 unused. HSYNC GPIO38,
VSYNC GPIO39, both active-low (VGA negative polarity). `pclk` and `de` go to no
pin — a DE-15 has no clock or DE wire, and an unrouted 25 MHz pin is only EMI.

The ramp proves the order on silicon: its patch centres read `00 01 02 03` (R),
`00 04 08 0C` (G), `00 10 20 30` (B), `00 15 2A 3F` (grey) in the order
0 / LSB only / MSB only / both.

## Timing, and how it was measured

640×480 VESA: 800 × 525 total, H 96/48/640/16, V 2/33/480/10, pclk requested
25.000 MHz from the 160 MHz PLL. Total line/pixel counts and the VSYNC interrupt
rate are printed once a second, so the number in the capture is measured from the
peripheral, not echoed from the config:

| | value |
|---|---|
| VSYNC rate over 78 s, 15 samples | 59.38 – 59.77 Hz, mean **59.54 Hz** |
| implied pixel clock | 59.54 × 800 × 525 = **25.01 MHz** (requested 25.000) |
| bounce fills per frame | **30.0 – 30.1** (required exactly 30) |
| frame buffer | `0x3fc99b00`, 76,800 B internal RAM |
| free heap after init | 288,628 B internal, largest block 245,760 B |
| build | RAM 14,640 B (4.5 %), flash 382,839 B of the 1 MB app partition |

A starving DMA or a desynced restart would show as a wrong fills-per-frame ratio
or a VSYNC gap; neither appears in 75 s across all three phases.

## Image pipeline choice

**Pre-converted on the laptop, embedded as byte arrays** (`src/images.h`, 76,800 B
per image, `memcpy`'d into the internal-RAM frame at each phase switch). The S3
rung has no PSRAM; an on-device TJpgDec pool plus stream buffer on top of the
frame is cost with no benefit for two known images.

`tools/convert_images.py` does the conversion: 2×2 box average 640×480 → 320×240
(TJpgDec's 1/2 descale in the RGB domain), quantise to the **measured** ladder
levels 0 / 0.452 / 0.714 / 1.000 — not 0/85/170/255 — and dither in **linear
light**, per the 0177 round-2 finding. Palette entries actually used: lorikeet
46/64, Blue Marble 24/64.

## What was verified, and with what

1. **The doubling code, byte for byte.** `tools/scanout_test.cpp` compiles the
   shipped `src/scanout.h` (the same function registered as `on_bounce_empty`),
   drives a whole 640×480 frame in the device's 10,240 B chunks, and compares
   against an independent 2×2 nearest-neighbour expansion: **0 differing bytes of
   307,200** for 10,240 B, whole-frame and 1,000 B (misaligned) chunkings, with
   two live negative controls (1-px shift → 152,486 differing bytes; one source
   byte flipped → 4).
2. **The rung-2 headers were copied faithfully.** `src/ramp_screen.h` is a copy of
   photo-display-64's file with only its include changed (old quantiser names are
   aliased in `src/ladder64.h`), and its 320×240 render is **byte-identical** to
   the committed `photo-display-64/tools/ramp-preview.ppm` (230,415 B, sha256
   `2734970a8a770c97…`).
3. **The image port against a second implementation.** The Python quantiser and
   the C firmware headers agree on both images with **0 differing bytes of
   921,600** in each doubled frame.
4. **Build and flash.** `bench/build.log` SUCCESS; `bench/upload.log` three
   `Hash of data verified.` lines for the binary currently on the board.
5. **The board itself.** Identity before flashing (`esptool read_mac` →
   9c:13:9e:a1:37:40 on /dev/ttyUSB1; /dev/ttyACM0 is an ESP32-C3 and /dev/ttyUSB0
   a classic ESP32-D0WD, neither touched), then the 78 s capture in
   `bench/console-photo-display-s3.txt`.

The defect that check 1 caught is described in the front-matter: a `0x00010001`
multiplier made every second output byte zero. It was invisible to the build,
the console and the scanout counters, and it is fixed in what was flashed.

## What Stephen should know before photographing

- The screen should show **thick pixels**: 320×240 doubled to 640×480, so each
  source pixel is a 2×2 block at 640×480 — chunky but sharp-edged, not blurred.
- The ramp is rung 2's, unchanged: R, G, B rows left→right as 0 / LSB / MSB / both,
  then the grey row, then a dithered black→white gradient at the bottom. If any
  row's step order reads reversed, the fault is at the DAC/pin end, not the ramp
  code — the byte values are quoted above.
- Shadows will still speckle a little: the ladder's first step is sRGB 115, not a
  dark grey. That is the ladder, not this driver, and it is why rung 3's colour
  budget decision matters.
- The first 30 s is the ramp; the lorikeet follows at 30 s and Blue Marble at 50 s,
  repeating every 70 s (timestamps in the console capture are the source of truth).
- If the monitor shows nothing at all, the useful datum is whether it reports
  "no signal" (sync absent) or locks and shows garbage (sync present, data wrong) —
  the two point at completely different halves of this work.

Nothing outside `living-frame/firmware/photo-display-s3/` was changed; the classic
projects, the DAC wiring and the pin map are untouched.
