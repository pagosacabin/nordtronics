---
task_id: '0180'
protocol_version: 1.2.0
status: unassigned
iteration: 0
expect-reply-within: 48h
---

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
