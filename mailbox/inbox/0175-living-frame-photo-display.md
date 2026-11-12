---
task_id: "0175"
status: inbox
iteration: 0
proof: []
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
