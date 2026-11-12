---
task_id: '0179'
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 48h
notes: |
  Picked up 2026-11-12 16:20 MST (23:20 UTC, off-peak). The file arrived with
  `status: unassigned`, which is not one of the protocol's five states
  (inbox|in_progress|staged|verified|failed); normalised to `in_progress` at
  pickup and noted here rather than treating the task as malformed.
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
