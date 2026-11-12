---
task_id: "0175"
protocol_version: 1.2.0
status: verified
iteration: 1
expect-reply-within: 48h
cost:
  model: deepseek-flash
  api_calls: 57
  input_tokens: 98729
  input_cache_hit_tokens: 5485440
  input_cache_miss_tokens: 98729
  output_tokens: 37783
  total_tokens: 5621952
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.053935
  cost_note: "Read from this profile's state.db session_model_usage, session local-8a1f4192...,
    read at 2026-11-12T18:34Z: 57 calls, input_tokens 98,729 (cache miss), cache_read
    5,485,440, output 37,783, reasoning 16,281. Arithmetic: 98729x0.15e-6 + 5485440x0.003e-6
    + 37783x0.60e-6 = 0.05393547. The staging commit and reply composition are further API
    calls; the figure is not re-read afterwards. Whole run 18:15Z Thursday = off-peak
    (PEAK: OFF-PEAK 18:15 UTC)."
proof:
  - branch: main
    sha: "7ab46c2959ad47ce672bf2bc9574ae296cbf2f43"
    repo: "pagosacabin/cabin-play"
    note: "Tip of cabin-play main after two commits by this run (673a200 adds the project,
      7ab46c2 adds the reproducibility note). Verified equal by `git ls-remote --heads
      origin main` at 18:33Z, and checked again from a fresh `git clone --depth 1`, whose
      HEAD is the same sha."
  - run: none
    run_note: "cabin-play has no CI at all -- there is no .github/workflows directory, so no
      workflow can start on main or on any branch (0138 precedent: this is the same repo).
      Build-green evidence is the committed build-esp32dev.log.txt (the pio run output,
      ending `[SUCCESS]`), a rebuild from a clean clone of this sha that also ends
      `[SUCCESS] Took 4.87 seconds`, and the on-device console quoted in the reply."
  - ntfy: "not applicable, and deliberately not published. The spec names no ntfy topic; the
      established bench topic nordtronics-build-ed05a663 is for companion APK builds, which
      this is not, and a receipt in the established format requires Workflow and Artifacts
      URLs that a repo with no CI cannot have. A notification there would be a false
      handoff signal about a different artefact."
  - files:
      - living-frame/firmware/photo-display/platformio.ini
      - living-frame/firmware/photo-display/src/main.cpp
      - living-frame/firmware/photo-display/src/photo_render.h
      - living-frame/firmware/photo-display/data/photo.jpg
      - living-frame/firmware/photo-display/data/PHOTO-SOURCE.md
      - living-frame/firmware/photo-display/lib/TJpgDec/tjpgd.c
      - living-frame/firmware/photo-display/lib/TJpgDec/tjpgd.h
      - living-frame/firmware/photo-display/lib/TJpgDec/tjpgdcnf.h
      - living-frame/firmware/photo-display/lib/TJpgDec/library.json
      - living-frame/firmware/photo-display/lib/TJpgDec/NOTICE.md
      - living-frame/firmware/photo-display/tools/render_check.cpp
      - living-frame/firmware/photo-display/tools/render-check.png
      - living-frame/firmware/photo-display/build-esp32dev.log.txt
      - living-frame/firmware/photo-display/README.md
      - living-frame/firmware/photo-display/.gitignore
notes: |
  STAGED. Run 2026-11-12 18:15-18:37 UTC (11:15-11:37 MST), off-peak (PEAK: OFF-PEAK
  18:15 UTC), PROTOCOL MATCH 1.2.0. Queue: 0142 in active/ carries `status: superseded`
  (Juno's 2026-10-19 resolution) and 0159 is decision-blocked on the UniFi controller
  address; both were left untouched, so 0175 is the lowest actionable task.

  WHAT SHIPPED. `living-frame/firmware/photo-display` in pagosacabin/cabin-play, next to
  `vga-proof`, env `esp32dev`, same five pins (R D14, G D19, B D27, HSYNC D32, VSYNC D33).
  JPEG decoder: **TJpgDec R0.03 by ChaN** (vendored, lib/TJpgDec), not bodmer's Arduino
  wrapper -- see below for the one config change. Photo: **NASA/Apollo 17 "The Blue
  Marble" (AS17-148-22727), public domain** (Commons `LicenseShortName = Public domain`,
  re-verified through the Commons API during this run), downloaded at 960 px and cut to
  exactly 640x480 with `-resize 640x640^ -gravity center -crop 640x480+0+0`; source URL,
  licence and the exact magick command are committed in data/PHOTO-SOURCE.md.
  Quantisation: 4x4 ordered (Bayer) dither to the R1G1B1 palette, one bit per channel.
  The photo is embedded in flash (`board_build.embed_files`), so no SD, no filesystem, no
  WiFi -- and no full-image buffer is ever allocated: the decoder streams 16x16 MCU blocks
  straight into the 154 kB 3-bit frame buffer.

  FLASHED, NOT LEFT AS A FILE. The WROOM-32 devkit was attached (CP2102, MAC
  4c:c3:82:da:d5:68) and was still running rung 0 -- confirmed by reading the first 512
  bytes of 0x10000 and finding `living-frame rung 0 -- VGA proof` in it, so there was no
  doubt about which board this was before overwriting it. `pio run -t upload` wrote
  389,120 bytes at 0x10000, "Hash of data verified". The board is now running rung 1.

  ON-DEVICE PROOF (console captured over the CP2102 at 115200):
    living-frame rung 1 -- photo display
    pins: R=14 G=19 B=27 HSYNC=32 VSYNC=33
    I2S TX
    photo.jpg embedded: 108557 bytes
    jpeg: 640x480, 3 component(s) -> 640x480 screen
    frame drawn in 1611 ms (307200 pixels), free heap: 100348 bytes
    holding image
  i.e. the embedded JPEG arrived intact (size matches the file byte for byte), the decoder
  parsed 640x480 with 3 components, all 307,200 pixels reached the frame buffer, draw took
  1.6 s, and 100 kB of heap was still free. 1.6 s of drawing while the ISR scans out means
  a visible tear for that first second and a stable picture afterwards -- worth saying
  before Stephen sees it.

  BUILD AND RENDER EVIDENCE. Local `pio run` SUCCESS (RAM 26,608/327,680 = 8.1 %, flash
  388,753/1,310,720 = 29.7 %); log committed. A fresh `git clone --depth 1` of this sha also
  builds SUCCESS (4.87 s). Because the image cannot be photographed from here, the decode
  + dither code is in `src/photo_render.h` and compiled UNCHANGED on the host by
  `tools/render_check.cpp` (only the pixel sink differs: a buffer instead of vga.dotFast).
  Its output, `tools/render-check.png`, was measured against data/photo.jpg: 307,200/307,200
  pixels drawn, exactly 8 palette colours present, RMSE of the block-averaged render vs the
  source 3.2 %, luminance NCC 0.9992 at both 40x30 and 160x120, and the same NCC falls to
  -0.025 when the source is rolled 10 px -- so the number is measuring structure, not just
  agreeing with itself. This is a render check, not an observation of the VGA output.

  WHAT IS NOT PROVEN, stated plainly: nobody has seen the picture on a monitor. The
  SyncMaster died 2026-11-12 and no VGA display is attached to this host, so criterion 3
  (photo full-screen, recognizable, stable) is Stephen's acceptance photo and remains open.
  Everything up to the DAC is verified; the electrical path and the panel are not mine to see.

  SELF-CAUGHT DEFECT, declared because it is invisible in the final diff: the first version
  of the render glue stored the input stream and the output sink in two structs. TJpgDec
  hands BOTH callbacks the same `jd->device`, and `jd_prepare()` sets it to whatever it was
  given, so the output callback cast the stream to a render target, computed a nonsense
  screen size and dropped every pixel -- `jd_decomp` still returned JDR_OK and the host check
  reported "pixels drawn: 0". Fixed by putting the stream and the sink in one RenderTarget;
  the header now carries a comment saying exactly this. The flash above is of the fixed
  build.

  DEVIATIONS / SCOPE ADDITIONS, all inside the new project directory:
   - `src/photo_render.h` + `tools/render_check.cpp` + `tools/render-check.png` were not
     requested. They exist so the decode/dither path could be verified at all without a
     monitor, and so the firmware and the check cannot drift (same file, not a copy).
   - `data/PHOTO-SOURCE.md` and `build-esp32dev.log.txt` follow the vga-proof convention of
     recording provenance and the build log in-tree.
   - `lib_extra_dirs = ../vga-proof/lib` reuses the vendored ESP32Lib in place instead of
     copying 1.3 MB of it; the task said "reusing the vendored ESP32Lib copy", and one copy
     also means both rungs build identical library source. A clean clone resolves it.
   - The captured `.bin` is not byte-reproducible across checkouts (389,168 vs 389,120
     bytes): the ESP-IDF app descriptor embeds the build path and a timestamp. Noted in the
     README so the quoted sha256 is not read as a universal hash.

  ONE METHOD NOTE. The first console capture came back as 402 kB of 0x78/0xf8 pattern --
  wrong-baud garbage that looks like a dying board. Re-capturing through pyserial at 115200
  with the classic CP2102 ladder (DTR deasserted, RTS pulse) produced the clean log above,
  so the board was healthy the whole time and the first capture was a harness fault.

  NEXT: Stephen powers the board with a replacement VGA monitor and photographs the screen.
  If the picture is upside down, rolled, or clipped at the corners, WIRING.md's
  troubleshooting table applies (it is the same electrical path rung 0 proved), and the
  firmware puts nothing on screen if VGA init or the decode fails -- it halts in a visible
  loop only after printing the error on the serial console.
---

# 0175 — Living frame rung 1: display a photo image (JPEG → VGA)

expect-reply-within: 48h

## Assignment

- **Hermes:** write, build, and push the firmware. Flash it yourself if the WROOM-32 devkit is attached to the laptop; otherwise leave the built .bin plus the exact flash command in your reply. Bench firmware is yours per protocol 1.2.0 — Newton is not involved in this task.
- **Stephen:** wire the display ESP32 to a VGA monitor (proven rung-0 wiring), power it, and take the acceptance photo.
- **Juno:** verify the commit and build, then archive once Stephen's acceptance photo lands.

## Context

- The living frame is Stephen's white-elephant gift build. Rung 0 is PROVEN (2026-10-29): classic ESP32-WROOM-32 devkit + bitluni ESP32Lib (VGA3BitI, 640x480) showing color bars in all 8 colors on a VGA monitor. That firmware lives in `pagosacabin/cabin-play` at `living-frame/firmware/vga-proof` (PlatformIO, env `esp32dev`, ESP32Lib vendored in that project).
- Rung 1, this task, is the smallest next step: display ONE real photographic image on the VGA output. The full SD-card slideshow is a later rung — do not build it here. No SD hardware is needed for this task; the photo ships with the firmware (embedded in flash).
- The original SyncMaster monitor died 2026-11-12 (capacitor swap did not fix it). Stephen will prove this on a replacement thrift-shop VGA monitor. Nothing in this task depends on a specific display — any working VGA monitor closes it.
- Wiring is the proven 0138 map. Do not renumber: RED → D14, GREEN → D19, BLUE → D27, HSYNC → D32, VSYNC → D33, GND → GND.

## Task

Create a new PlatformIO project in cabin-play at `living-frame/firmware/photo-display`, next to `vga-proof` and following its structure, that:

1. Targets the classic ESP32-WROOM-32 devkit (`esp32dev`), reusing the vendored ESP32Lib copy from `vga-proof` and the same VGA pin map above.
2. Displays one real photographic JPEG image full-screen at 640x480, decoded on-device (TJpgDec or an equivalent ESP32 JPEG decoder — your pick; name it in the reply), quantized or dithered down to the 3-bit palette.
3. Includes the source photo inside the project in the repo (e.g. `photo.jpg` — a real photo with color variety from a freely usable source; no personal photos), so a clean clone reproduces the exact build.
4. Holds the image static and stable. No test pattern, no slideshow, no SD card, no WiFi in this rung.

## Success criteria

- `pio run` for env `esp32dev` ends in SUCCESS on your machine.
- Project, source photo, and firmware are committed on cabin-play `main`; the reply carries the commit SHA.
- The flashed firmware shows the photo full-screen, recognizable as that photo (not bars, not noise), stable with no roll or flicker.
- Stephen's acceptance photo of the monitor showing the image is the final gate; Juno archives after it.

## Constraints

- Cabin-play writes go over SSH from the laptop. The gh.py credential is nordtronics-scoped and returns 403 on cabin-play — do not retry it; SSH push is the proven path (0136 precedent).
- No purchases and no new hardware. If your chosen approach needs something that is not on the bench, stop and say so in the reply instead of substituting silently.
- 3-bit color is the proven hardware limit of this display path. A quantized/dithered look is expected and is not a defect.
- One deliverable: the photo-display firmware. Slideshow, SD support, and camera/ESP-NOW features are explicitly out of scope — later rungs.

## Proof

- Commit SHA on `pagosacabin/cabin-play` `main` covering `living-frame/firmware/photo-display`.
- cabin-play has no CI: `run: none` is the honest entry (0138 precedent). The build-green evidence is your quoted local `pio run` SUCCESS line plus the commit Juno can check out and rebuild.

## Reply format

Staged reply in `mailbox/staged/` per protocol 1.2.0: front-matter with `status: staged`, the proof list (`branch: main` + sha, `run: none` with the no-CI note), the cost block, and notes naming: the JPEG decoder used, the source photo and where it came from, whether you flashed the board or left the .bin for Stephen, and anything that blocked you.
# Reply — DONE, built, flashed and running (2026-11-12 18:37 UTC)

## What it is

`living-frame/firmware/photo-display` in `pagosacabin/cabin-play`, on `main` at
`7ab46c2`. A classic ESP32-WROOM-32 (env `esp32dev`) showing one photographic JPEG
full-screen at 640×480 on the rung-0 wiring.

| | |
|---|---|
| decoder | **TJpgDec R0.03** (ChaN), vendored at `lib/TJpgDec` |
| photo | NASA/Apollo 17, *The Blue Marble* (AS17-148-22727), **public domain** |
| photo file | `data/photo.jpg`, 640×480, 108,557 bytes, embedded in flash |
| colour | R1G1B1 (8 colours), 4×4 ordered Bayer dither |
| image buffer | none — 16×16 MCU blocks streamed into the 154 kB frame buffer |
| VGA pins | unchanged: R D14, G D19, B D27, HSYNC D32, VSYNC D33 |

One configuration change to TJpgDec, recorded in its NOTICE.md: `JD_FORMAT 0`
(RGB888) instead of upstream's `1` (RGB565). The dither needs each channel at full
range; at RGB565 the decoder would already have truncated green to 6 bits and red
and blue to 5 before the palette decision.

## Verified, and how

* **Build** — `pio run` for `esp32dev`: `[SUCCESS]`, RAM 8.1 % (26,608 B), flash
  29.7 % (388,753 B of 1,310,720). Log committed as `build-esp32dev.log.txt`.
* **Clean clone** — fresh `git clone --depth 1` of `7ab46c2`, `pio run` there:
  `[SUCCESS] Took 4.87 seconds`. The project needs no network fetch and no manual
  library step.
* **Flash** — the devkit was attached (CP2102, MAC `4c:c3:82:da:d5:68`, previously
  running rung 0 — checked by reading its app header before overwriting): 389,120
  bytes written at `0x10000`, `Hash of data verified`.
* **On the chip** — serial console after the flash:

  ```
  photo.jpg embedded: 108557 bytes
  jpeg: 640x480, 3 component(s) -> 640x480 screen
  frame drawn in 1611 ms (307200 pixels), free heap: 100348 bytes
  holding image
  ```

* **Decode + dither path, off-target** — `src/photo_render.h` is compiled unchanged on
  the host by `tools/render_check.cpp` (same callbacks, same dither; only the pixel
  sink differs). Output `tools/render-check.png`: exactly 8 palette colours, RMSE vs
  the source photo 3.2 %, luminance NCC **0.9992** at 40×30 and 160×120, and −0.025
  when the source is rolled 10 px — so the correlation is measuring structure.

## What is still open

**Nobody has seen it on a monitor.** The SyncMaster died on 2026-11-12 and no VGA
display is attached to this host, so criterion 3 — photo full-screen, recognizable,
stable — waits on Stephen's acceptance photo. Everything up to the VGA pins is
verified; the DAC, the cable and the panel are not mine to observe. Expect a visible
tear during the first ~1.6 s while the frame is drawn, then a stable picture.

## For Stephen

The board is **already flashed**; powering it with a VGA monitor attached should show
the picture. To rebuild or re-flash from a clean checkout:

```bash
cd living-frame/firmware/photo-display
pio run                 # build
pio run -t upload       # flash (board: ESP32-WROOM-32 devkit, CP2102 @ 460800)
```

If it is upside down, rolled, or the corners are clipped, `../vga-proof/WIRING.md`
§troubleshooting applies — it is the same electrical path rung 0 proved.
