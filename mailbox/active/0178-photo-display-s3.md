---
task_id: '0178'
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 48h
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
