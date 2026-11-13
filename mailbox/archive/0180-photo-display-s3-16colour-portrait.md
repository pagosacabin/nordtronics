---
task_id: '0180'
protocol_version: 1.2.0
status: verified
iteration: 1
expect-reply-within: 48h
cost:
  model: deepseek-flash
  api_calls: 83
  input_tokens: 10269149
  input_cache_hit_tokens: 10184320
  input_cache_miss_tokens: 84829
  output_tokens: 68622
  total_tokens: 10337771
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output (protocol cost format; rate card not re-fetched this run)"
  cost_usd: 0.0845
proof:
  branch: main
  sha: c26b60a0bdc88df896f03caadec4b6d583b8f86c
  branch_note: >-
    Landed on cabin-play `main` as the task asks. The work was committed on
    hermes/0180-rung-3b-16colour (same sha, fast-forward into main), so both refs
    re-fetch to c26b60a. `git ls-remote --heads origin main` returns
    c26b60a0bdc88df896f03caadec4b6d583b8f86c.
  run: none
  run_reason: >-
    cabin-play has no CI to trigger: the repository has no .github directory at
    all (`ls -a .github` -> "No such file or directory"; `ls .github/workflows`
    likewise), so a push starts no run and there is no run URL to cite. The
    build, flash and console evidence is in bench/ instead.
  ntfy: >-
    no receipt: the task names no topic, and the receipt format needs a workflow
    and artifact URL, which this repo cannot produce. The console capture, build
    log and upload log carry the same information.
  files:
    - living-frame/firmware/photo-display-s3/src/main.cpp
    - living-frame/firmware/photo-display-s3/src/scanout.h
    - living-frame/firmware/photo-display-s3/src/ramp_portrait.h
    - living-frame/firmware/photo-display-s3/src/ramp_palette.h
    - living-frame/firmware/photo-display-s3/src/images16.h
    - living-frame/firmware/photo-display-s3/platformio.ini
    - living-frame/firmware/photo-display-s3/tools/convert_images16.py
    - living-frame/firmware/photo-display-s3/tools/render16.cpp
    - living-frame/firmware/photo-display-s3/tools/scanout_test4.cpp
    - living-frame/firmware/photo-display-s3/tools/compare_modes.py
    - living-frame/firmware/photo-display-s3/tools/img16_lorikeet-portrait.png
    - living-frame/firmware/photo-display-s3/tools/img16_lorikeet-scanout.png
    - living-frame/firmware/photo-display-s3/tools/img16_blue_marble-portrait.png
    - living-frame/firmware/photo-display-s3/tools/img16_blue_marble-scanout.png
    - living-frame/firmware/photo-display-s3/tools/ramp-portrait.png
    - living-frame/firmware/photo-display-s3/tools/ramp-portrait-raster.png
    - living-frame/firmware/photo-display-s3/bench/console-16colour.txt
    - living-frame/firmware/photo-display-s3/bench/build16.log
    - living-frame/firmware/photo-display-s3/bench/upload16.log
    - living-frame/firmware/photo-display-s3/bench/identity-16colour.log
    - living-frame/firmware/photo-display-s3/bench/unit-scanout4.log
    - living-frame/firmware/photo-display-s3/bench/host-render-16.log
    - living-frame/firmware/photo-display-s3/README.md
notes: |
  Built, flashed and captured on the bench. The Heltec on /dev/ttyUSB1 was
  identified before flashing (esptool read_mac -> MAC 9c:13:9e:a1:37:40, chip
  ESP32-S3 rev v0.2, 8 MB GD flash; bench/identity-16colour.log). The classic
  WROOM on /dev/ttyUSB0 and the C3 on /dev/ttyACM0 were not touched.

  THE TASK'S OWN RISK, ANSWERED. The open question was whether 640x480 with 16
  colours beats 320x240 with 64 on this DAC. It fits and it runs: 153,600 B at
  4 bpp in internal RAM (free heap after init 209,524 B, largest block 167,936 B),
  VSYNC 59.36-60.17/s over 100 s, and exactly 30.0 bounce fills per frame through
  two full cycles -- no starvation, no reset, no panic. Whether it looks better
  is Stephen's call; I have no camera on the panel.

  WHAT I COULD NOT VERIFY, stated plainly: the monitor lock, the picture itself,
  and the physical pivot direction. No instrument on this host sees the VGA cable.
  The console proves the peripheral is scanning a correct frame; the 213T locking
  at this timing was established in 0179 and the timing is unchanged, but I did
  not re-observe it. Nothing below is a photograph.

  WHAT IS ON THE BOARD NOW: rung 3b (PHOTO_MODE=1), still running. To put 3a back:
  `pio run -e heltec-s3-0179 -t upload`.

  PALETTE METHOD (step 3). Portrait cover-crop to 480x640, gamma-decode to linear
  light at 2.2, median cut into 16 boxes in linear light, then each box mean is
  replaced by one of the 64 colours the ladder can actually make via a greedy
  one-to-one assignment on n*||mean-colour||^2 (luminance-weighted). Snapping each
  mean independently collapsed 16 boxes onto 8 distinct colours on the first run,
  so the assignment is not cosmetic. A Lloyd refinement pass (8 rounds) then
  converged to the same palette on both photos -- a fixed point, which is why both
  error figures match in the log. Final dither is Floyd-Steinberg in linear light
  against that palette: error diffusion rather than an ordered threshold, because
  the palette is irregular and the error a pixel leaves has to be carried.

  PORTRAIT CONVERSION (step 3/4). The rotation is baked into the host frame: the
  480x640 canvas is rotated 90 deg CLOCKWISE into the 640x480 raster, so it reads
  upright on a monitor pivoted 90 deg COUNTER-CLOCKWISE (rotate-left). Nothing
  rotates on the device -- the ISR stays a flat index->DAC-byte expansion. The
  direction is asserted, not assumed: the converter asserts the marker mapping,
  and tools/render16.cpp reads the finished raster back at all 16 patch centres
  and checks the index found against the index the layout calls for (16/16 pass;
  a transposed map fails). If Stephen pivots the other way it is two lines
  (np.rot90 k in the converter, rpMapX/rpMapY in ramp_portrait.h) plus a re-flash.
  He should photograph the ramp FIRST: if the labels and the 0/1/2/3 under each
  patch read correctly the orientation is right, and if they read mirrored or
  upside down the pivot is the other way.

  THE RAMP. Same diagnostic as 3a on the 480x640 canvas: R/G/B patch rows in the
  order 0 / LSB / MSB / both, a W row where all three channels move together (a
  per-channel fault shows as a tint), each patch labelled with its level, and a
  dithered black->white gradient band at the bottom drawn with the same
  light-domain quantiser and 8x8 Bayer matrix. It reuses the 5x7 font and the
  layout idea from ramp_screen.h; it is a separate header (src/ramp_portrait.h)
  because rung 3a's header writes raw 6-bit DAC bytes and this mode writes 4-bit
  palette indices -- copying the file and changing its pixel unit would have made
  "the file is unchanged" untrue. src/ramp_screen.h and src/images.h are untouched.

  CONSOLE FIGURES (bench/console-16colour.txt, 100 s, /usr/bin/python3
  bench/capture.py /dev/ttyUSB1 100):
    frame buffer: 0x3fc9a000, 153600 B (4 bpp, internal RAM) = 640x480 pixels
    palettes    : 16-entry set per phase -> 256-entry expand table (3 x 512 B)
    ramp palette   16 entries: 00 01 02 03 00 04 08 0c 00 10 20 30 15 2a 3f 00
    lorikeet       16 entries: 00 01 04 05 06 0a 10 11 14 15 16 1a 1b 25 2a 2b
    blue-marble    16 entries: 00 05 0a 10 14 15 1a 25 26 29 2a 2f 39 3a 3b 3f
    heap        : free after init = 209524 B internal (largest block 167936 B)
    [   5.230 s] 302 vsync (+60.17/s),   9074 fills (+30.0/frame)
    [  50.316 s] 2986 vsync (+59.65/s), 89584 fills (+29.9/frame)
    [  95.377 s] 5668 vsync (+59.60/s), 170050 fills (+30.0/frame)
  Uptime runs to 95 s across ramp -> lorikeet -> blue-marble -> ramp with no
  reset, no panic and no E(/W( log lines; the boot banner is rst:0x1 (POWERON).

  HOST CHECKS. tools/scanout_test4.cpp drives the shipped 4bpp fill over a whole
  frame at chunk sizes 10240 / whole-frame / 1000 / 999 / 2 and byte-compares
  against a naive per-pixel reference: 0 differing bytes of 307,200 in every
  shape, with two live negative controls (nibble order swapped: 288,204 differ;
  palette entries swapped: 38,362 differ). tools/scanout_test.cpp (rung 3a) still
  passes, so the doubling path is untouched. tools/render16.cpp passes 16/16
  patch probes. tools/compare_modes.py, same portrait source both ways:
  lorikeet ME 29.34 (full16) vs 29.70 (doubled), blue marble 29.21 vs 36.14 --
  a lower error is not a verdict on colour count, only evidence the pipelines
  agree about the picture.

  A DEFECT THIS PROCESS CAUGHT, before it reached the board: scanout_index4_fill's
  tail loop took the low nibble unconditionally, so a chunk with an odd length
  duplicated the neighbouring even pixel. Invisible at the device's 10,240-byte
  chunks; the 999-byte and 2-byte chunk cases in scanout_test4 caught it (143 and
  144,102 differing bytes). Fixed, and the test now passes at every chunk shape.

  MODE SELECTION. Two PlatformIO envs: heltec-s3 = rung 3b (PHOTO_MODE=1, 538,311 B
  of the 1 MB app partition) and heltec-s3-0179 = rung 3a (PHOTO_MODE=2, 538,303 B).
  Both were built this run, and `strings` on each ELF shows its own banner, so the
  mode is verifiable in the artifact. 3a is selectable, not merely present.
---

# 0180 — Living frame rung 3b: full 640×480 at 16 colours, portrait (Heltec V3)

## Reply

**Staged candidate: rung 3b is built, flashed to the Heltec and running.** Commit
`c26b60a0bdc88df896f03caadec4b6d583b8f86c` on cabin-play `main`
(`git ls-remote --heads origin main` returns that sha; the same commit is on
`hermes/0180-rung-3b-16colour`).

### What was done

1. **Full-resolution mode** — a true 640×480 frame buffer at 4 bits per pixel,
   153,600 B in internal RAM, no doubling. Two pixels per byte, even column in the
   low nibble. The palette is folded into a 256-entry `expand[]` table at init, so
   the bounce-buffer ISR does two table loads and one aligned 32-bit store per four
   output pixels — the same cost shape as rung 3a, and the same 30 fills/frame.
   Three expand tables (ramp, lorikeet, blue marble) are built once at init; a phase
   switch flips a pointer rather than rewriting the table the ISR is reading.
2. **Rung 3a preserved and selectable** — `pio run -e heltec-s3-0179`
   (`PHOTO_MODE=2`) rebuilds the 320×240 doubled 64-colour firmware; it was built
   this run and its ELF carries its own banner. `src/ramp_screen.h` and
   `src/images.h` are byte-unchanged, and rung 3a's host test still passes.
3. **Portrait pipeline** (`tools/convert_images16.py`) — cover-crop to 480×640,
   median cut in linear light, distinct greedy snap to the ladder's 64 colours,
   Lloyd refinement, Floyd–Steinberg in linear light, then a 90° clockwise rotation
   into the landscape scanout raster. Per-photo palettes; no fixed system palette.
4. **Content cycle** — ramp 30 s, lorikeet 20 s, Blue Marble 20 s, looping, all
   portrait. The ramp is the 3a diagnostic laid out on the portrait canvas (R/G/B
   patch rows, a W row, level labels, dithered gradient band).
5. **Flashed and captured** — see the console figures in `notes:` and
   `bench/console-16colour.txt`.

### Before photographing — for Stephen

- **Check the orientation on the ramp first.** The picture should read upright with
  the monitor pivoted **counter-clockwise (rotate-left)**. The R/G/B/W labels and
  the 0/1/2/3 under each patch are there precisely so a mirrored or upside-down
  picture is obvious; if it is, tell Juno and the two-line flip (converter +
  `ramp_portrait.h`) plus a re-flash fixes it.
- **The wall is portrait, the signal is landscape.** The 213T still receives
  640×480@~59.5 Hz; the content is 480×640 rotated into it.
- **This is a colour-count trade, not a resolution trick.** Full 640×480 with 16
  colours per photo, against quarter resolution with 64. The ramp's gradient band
  and the W row are where banding or a tint would show first.
- The board currently runs rung 3b. `pio run -e heltec-s3-0179 -t upload` puts the
  doubled 64-colour rung back for an A/B comparison.

### What is not proven

The monitor lock, the picture, and the pivot direction. No instrument on this host
sees the VGA cable; the console proves the peripheral scans a correct frame at the
right rate with no starvation, and 0179 established the 213T locks at this exact
timing — but I did not re-observe it. **Stephen's photographs and his verdict are
the acceptance gate**, as the task states.

---

## Original task (unchanged, for reference)

# 0180 — Living frame rung 3b: full 640×480 at 16 colours, portrait (Heltec V3)

## Assignment
- Hermes: all steps — bench firmware on Stephen's machine (build, flash the Heltec, verify).
- Newton: not assigned.
- Juno: verifies the staged reply; Stephen's photos of the screen are the acceptance gate.

## Context
Rung 3a (task 0179, archived) proved the LCD_CAM driver on the Heltec WiFi LoRa 32 V3 (ESP32-S3FN8, no PSRAM): 320×240 doubled into a 640×480@60 signal, measured VSYNC 59.54 Hz mean, bounce fills exactly 30/frame, byte-verified doubling. The driver lives in cabin-play `living-frame/firmware/photo-display-s3`.

Stephen's decisions (2026-11-12): (1) the ladder continues on this board at full resolution with 16 colours — a 640×480 frame buffer at 4 bits/pixel is 153,600 B and fits internal RAM where the 64-colour buffer (307,200 B) did not; (2) **the living frame's final orientation is PORTRAIT** — the SyncMaster 213T pivots, and the gift will hang portrait. Every rung from here is built portrait-native: the VGA signal stays 640×480 landscape electrically; the content is rendered 480 wide × 640 tall and rotated into the scanout.

This rung answers the real question: does full resolution with 16 colours beat quarter resolution with 64 for a photo on this DAC? Stephen's eyes decide. If 16 colours disappoints, the next rung is the N16R8 board (8 MB PSRAM, arriving ~Nov 18) at full 640×480/64.

## Fixed facts
- Board: Heltec V3 on USB, MAC 9c:13:9e:a1:37:40 — identify before flashing; do NOT flash the classic WROOM (4c:c3:82:da:d5:68) or any other attached board.
- Pin map (Stephen has wired to this): R MSB 4 / LSB 5, G MSB 6 / LSB 7, B MSB 19 / LSB 20, HSync 38, VSync 39. DAC landings: ~270 Ω = MSB, ~470 Ω = LSB per channel.
- Ladder levels are MEASURED, not ideal: 0 / 0.452 / 0.714 / 1.000 of full scale per channel.

## Task
1. **Extend `photo-display-s3`** with a full-resolution mode. Keep the proven 0179 doubled-64-colour mode intact and selectable; this task adds the new mode alongside it.
2. **Mode: true 640×480 frame buffer, 4 bits/pixel** (153,600 B internal RAM), each nibble a palette index. Per-image palette of 16 entries chosen from the ladder's 64 colours. The bounce-buffer fill expands index → ladder byte via the image's palette. Console must print buffer address/size, free heap after init, VSYNC rate, and bounce fills per frame (the full-rate stability evidence, same discipline as 0179).
3. **Portrait pipeline:** the laptop-side converter takes each source photo, crops/scales to fill 480×640 portrait, rotates it into the landscape scanout orientation, picks the per-photo 16-colour palette (state the method — e.g. median-cut in linear light), and dithers in linear light against the measured ladder levels, per the 0177 round-2 finding. Palette optimisation happens per photo, never a fixed system palette.
4. **Content cycle:** ramp first (30 s), then lorikeet, then Blue Marble, ~20 s each, looping — all portrait. The ramp is the same diagnostic (per-channel 0 / LSB / MSB / both, grey ramp, gradient) rendered for portrait viewing; reuse `ramp_screen.h` content, rotated/adapted as needed, and say how in the reply.
5. **Flash the Heltec yourself** and capture the console (mode, buffer, heap, VSYNC, fills/frame, per-image palette size) plus host render checks of the portrait frames.

## Success criteria
- Stable full-rate 640×480 lock on the 213T over the whole cycle — VSYNC and fills/frame figures in the console capture, no starvation.
- Ramp reads correctly in portrait: steps in order on every channel.
- Stephen's photos: the two pictures recognisably finer than the doubled mode, and his verdict on whether 16 colours holds up.

## Constraints
- Pin map fixed; if LCD_CAM cannot drive any one of those pins in this mode, stop and report the exact pin and the evidence.
- Do not initialise or drive the LoRa radio or the OLED.
- If the full-res buffer plus driver does not fit, or DMA starves at full rate from internal RAM, stop and stage the evidence — do not silently drop to a lesser mode.
- cabin-play only; the 0179 mode and the classic projects stay intact; no purchases.

## Proof
- Commit on cabin-play `main` (branch + SHA Juno can re-fetch), file list including the build log, console capture, and portrait render checks.
- run: none (cabin-play has no CI — say so).

## Reply format
Staged reply per protocol 1.2.0: front-matter with status/iteration/cost block, proof pointers, and notes covering the palette method, the portrait conversion, console figures, and anything Stephen should know before photographing.
