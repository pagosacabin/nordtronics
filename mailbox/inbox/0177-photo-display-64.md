---
task_id: "0177"
status: inbox
iteration: 0
proof: []
---

# 0177 — Living frame rung 2: 64-color photo display (FabGL, existing DAC board)

expect-reply-within: 48h

## Assignment

- **Hermes:** write, build, and flash the FabGL firmware to the agreed pin map below. Bench firmware is yours per protocol 1.2.0; Newton is not involved.
- **Stephen:** wiring is already done (repinned to the agreed map 2026-11-12). Your part is the acceptance photo when the firmware is flashed.
- **Juno:** verify the commit and build, then archive once Stephen's acceptance photo lands.

## Context

- Living frame (white-elephant gift build): rung 0 (3-bit color bars) and rung 1 (0175, one JPEG on screen, archived) are proven. Stephen's verdict on rung 1: the 8-color picture reads as a dot puzzle; the bar is a picture that reads as the photo. This rung goes to **64 colors — 2 bits per channel — with FabGL's VGA 64-color controller**.
- **The existing VGA breakout module is already a 2-bit-per-channel DAC board** (bench-verified by Stephen 2026-11-12; the 0176 new-board premise was withdrawn): silkscreen R3x = 270 Ω and R5x = 470 Ω per channel (B, G, R), both paths joining at the VGA pin; sync paths R3H/R5H and R3V/R5V = 68 Ω. Stephen ohmed DB15 pin 3 to both blue header landings: ~270 Ω and ~470 Ω, matching the silkscreen. Landing identification is **by measured resistance**: the ~270 Ω landing is the heavy bit (MSB), the ~470 Ω landing is the weak bit (LSB), per channel (R = VGA pin 1, G = pin 2, B = pin 3).
- **Stephen has already repinned the wiring to the agreed map below.** The hardware side is done and matches this spec; the firmware must be built to it exactly.
- Photo: reuse rung 1's exact source — NASA Blue Marble at `living-frame/firmware/photo-display/data/photo.jpg` in cabin-play — so the two rungs are directly comparable on screen.

## Agreed pin map (FIXED — Stephen and Juno, 2026-11-12; Stephen has wired to this)

| Signal | GPIO | Board landing |
|---|---|---|
| Red MSB (heavy) | 22 | R, ~270 Ω path → VGA pin 1 |
| Red LSB (weak) | 21 | R, ~470 Ω path → VGA pin 1 |
| Green MSB (heavy) | 19 | G, ~270 Ω path → VGA pin 2 |
| Green LSB (weak) | 18 | G, ~470 Ω path → VGA pin 2 |
| Blue MSB (heavy) | 27 | B, ~270 Ω path → VGA pin 3 |
| Blue LSB (weak) | 4 | B, ~470 Ω path → VGA pin 3 |
| HSync | 23 | H, 68 Ω path → VGA pin 13 |
| VSync | 33 | V, 68 Ω path → VGA pin 14 |

This is FabGL's 64-color default map with the two strapping pins moved: Blue MSB 5 → 27 and VSync 15 → 33. All other signals sit at FabGL defaults.

## Task

Create a new PlatformIO project in cabin-play at `living-frame/firmware/photo-display-64`, next to `photo-display`:

1. **Step one — verify the pin-map assumption:** confirm the FabGL version you pin accepts custom pins for the 64-color VGA controller (the map above differs from defaults on two signals). If it does not, STOP and report that in the reply — do not substitute pins. Stephen has already wired to this map; a silent change leaves the hardware wrong.
2. Classic ESP32-WROOM-32 devkit (`esp32dev`), **FabGL** as the graphics library, version pinned in platformio.ini. (ESP32Lib has no 64-color mode — do not try to stretch it.)
3. Display the rung-1 Blue Marble JPEG full-screen on the 64-color controller, decoding the JPEG on-device (reuse the vendored TJpgDec from `photo-display/lib` or FabGL's own path — your pick, named in the reply) and quantizing to 4 levels per channel with dithering, so the frame buffer genuinely holds up to 64 colors.
4. Pick a resolution the 64-color framebuffer and the decode actually fit in RAM (320x240 or similar; a candidate note suggests VGA_320x200_75Hz leaves RAM for the bitmap). State the mode chosen and the RAM figures in the reply.
5. Flash the devkit (attached to your host; it currently runs rung 1) and capture the serial console showing the frame drawn, as in 0175.
6. Extend the 0175 host render-check pattern: compile the decode + quantize path on the host and report the count of distinct colors in the rendered frame (must be > 8, up to 64), with the render committed next to the check.
7. Put the pin map table in the project README, copied verbatim from this spec.

## Success criteria

- `pio run` for env `esp32dev` ends in SUCCESS; clean-clone rebuild also succeeds (0175 standard).
- Project committed on cabin-play `main`; reply carries the commit SHA.
- Console shows the frame drawn with sane free heap after the draw; the firmware's compiled pin map matches the agreed table exactly.
- Host render check: distinct colors in the frame > 8 (target: most of the 64-color palette on this photo).
- Stephen's acceptance photo: the Earth on the 213T, visibly smoother than rung 1 — recognizably the photo, not a dot puzzle. His call is the gate; Juno archives after it.

## Constraints

- Cabin-play writes go over SSH from the laptop (gh.py is nordtronics-scoped, 403 on cabin-play — 0136 precedent).
- No purchases, no new hardware, no board modifications, no rewiring.
- **The pin map is fixed.** Any deviation is a stop-and-report, never a substitution.
- One deliverable: the 64-color photo firmware. Slideshow/SD/camera rungs stay out of scope.

## Proof

- Commit SHA on `pagosacabin/cabin-play` `main` covering `living-frame/firmware/photo-display-64`.
- cabin-play has no CI: `run: none` is the honest entry (0138/0175 precedent). Build-green evidence is the committed build log plus the clean-clone rebuild, as in 0175.

## Reply format

Staged reply in `mailbox/staged/` per protocol 1.2.0: front-matter with `status: staged`, proof list (`branch: main` + sha, `run: none` with the no-CI note), the cost block, and notes covering: step-one pin-map verification result, FabGL version pinned, the resolution/mode chosen + RAM, the JPEG decode path used, the host render check's distinct-color count, and confirmation that the compiled pin map matches the agreed table signal for signal.
