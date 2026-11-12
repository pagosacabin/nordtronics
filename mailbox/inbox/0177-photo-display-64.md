---
task_id: "0177"
status: inbox
iteration: 0
proof: []
---

# 0177 — Living frame rung 2: 64-color photo display (FabGL, existing DAC board)

**HOLD — DO NOT PICK UP YET (Juno, 2026-11-12 ~12:10 MST).** Stephen's call: the GPIO pin map gets decided with him BEFORE any build, because he does the rewiring — the spec's "Hermes sets the pin map" section is being replaced with an agreed map baked into this file. Juno will lift the hold and update the pin-map section once Stephen approves the map in chat. Leave the file in inbox/ untouched meanwhile.

expect-reply-within: 48h

## Assignment

- **Hermes:** write, build, and flash the FabGL firmware, and publish the final pin map in your reply. Bench firmware is yours per protocol 1.2.0; Newton is not involved.
- **Stephen:** after your reply stages, wire the 8 connections per your pin map (heavy bit to the 270 Ω landing, weak bit to the 470 Ω landing, per channel) and take the acceptance photo.
- **Juno:** verify the commit and build, then archive once Stephen's acceptance photo lands.

## Context

- Living frame (white-elephant gift build): rung 0 (3-bit color bars) and rung 1 (0175, one JPEG on screen, archived) are proven. Stephen's verdict on rung 1: the 8-color picture reads as a dot puzzle; the bar is a picture that reads as the photo. This rung goes to **64 colors — 2 bits per channel — with FabGL's VGA 64-color controller**.
- **The existing VGA breakout module is already a 2-bit-per-channel DAC board** (bench-verified by Stephen 2026-11-12, supersedes the 0176 new-board premise — withdrawn): silkscreen R3x = 270 Ω and R5x = 470 Ω per channel (B, G, R), both paths joining at the VGA pin; sync paths R3H/R5H and R3V/R5V = 68 Ω. Stephen ohmed DB15 pin 3 to both blue header landings: ~270 Ω and ~470 Ω, matching the silkscreen. Rung 0/1 only ever wired ONE bit per channel (header landings B, G, R, H, V), so the board has been running as a 3-bit DAC. No new board, no added resistors — the second bit per channel just gets wired.
- Landing identification is **by measured resistance, not by header position**: for each color, the landing that ohms ~270 Ω to that channel's VGA pin (R = pin 1, G = pin 2, B = pin 3) is the heavy bit (MSB); the ~470 Ω landing is the weak bit (LSB).
- Photo: reuse rung 1's exact source — NASA Blue Marble at `living-frame/firmware/photo-display/data/photo.jpg` in cabin-play — so the two rungs are directly comparable on screen.
- The board is a commercial module on a 2-row header (landings labeled B, G, R, H, V); spare DB15 pins are broken out separately (GND, 4, 9, 11, 12, 15) and are not used in this rung.

## Task

Create a new PlatformIO project in cabin-play at `living-frame/firmware/photo-display-64`, next to `photo-display`:

1. Classic ESP32-WROOM-32 devkit (`esp32dev`), **FabGL** as the graphics library (pin the version in platformio.ini; note that ESP32Lib has no 64-color mode — do not try to stretch it).
2. Display the rung-1 Blue Marble JPEG full-screen using FabGL's 64-color VGA controller, decoding the JPEG on-device (reuse the vendored TJpgDec from `photo-display/lib` or FabGL's own path — your pick, named in the reply) and quantizing to 4 levels per channel with dithering, so the frame buffer genuinely holds up to 64 colors.
3. Pick a resolution the 64-color framebuffer and the decode actually fit in RAM (320x240 or similar; the candidate note Stephen found suggests VGA_320x200_75Hz leaves RAM for the bitmap). State the mode you chose and the RAM figures in the reply.
4. **Pin map — you set it, it is the contract Stephen wires to.** Start from FabGL's 64-color defaults (R: 22 MSB / 21 LSB, G: 19 / 18, B: 5 / 4, H 23, V 15). Then review the strapping pins: **GPIO5 and GPIO15 are strapping pins** — if the pinned FabGL version's begin() accepts custom pins, move those two signals to safer output GPIOs (keep the rest at defaults unless there's a reason). Publish the final map in the reply AND in the project README as a table: signal → GPIO → board landing (by resistance: 270 Ω = MSB, 470 Ω = LSB; syncs to the H/V landings).
5. Flash the devkit (it is attached to your host; it currently runs rung 1) and capture the serial console showing the frame drawn, as in 0175.
6. Extend the 0175 host render-check pattern: compile the decode + quantize path on the host and report the count of distinct colors in the rendered frame (must be > 8, up to 64) with the render committed next to the check, so the 64-color claim is verified off-target as well.

## Success criteria

- `pio run` for env `esp32dev` ends in SUCCESS; clean-clone rebuild also succeeds (0175 standard).
- Project committed on cabin-play `main`; reply carries the commit SHA.
- Console shows the frame drawn with sane free heap after the draw.
- Host render check: distinct colors in the frame > 8 (target: most of the 64-color palette on this photo).
- Stephen's acceptance photo: the Earth on the 213T, visibly smoother than rung 1 — recognizably the photo, not a dot puzzle. His call is the gate; Juno archives after it.

## Constraints

- Cabin-play writes go over SSH from the laptop (gh.py is nordtronics-scoped, 403 on cabin-play — 0136 precedent).
- No purchases, no new hardware, no board modifications. If the pinned FabGL version cannot do something this spec assumes (custom pins, the resolution, TJpgDec coexistence), say so in the reply and state what you did instead — do not silently change the pin map after publishing it.
- The pin map in your staged reply is final for Stephen's wiring. If you must change a pin after staging, that is a new reply, not an edit.
- One deliverable: the 64-color photo firmware + pin map. Slideshow/SD/camera rungs stay out of scope.

## Proof

- Commit SHA on `pagosacabin/cabin-play` `main` covering `living-frame/firmware/photo-display-64`.
- cabin-play has no CI: `run: none` is the honest entry (0138/0175 precedent). Build-green evidence is the committed build log plus the clean-clone rebuild, as in 0175.

## Reply format

Staged reply in `mailbox/staged/` per protocol 1.2.0: front-matter with `status: staged`, proof list (`branch: main` + sha, `run: none` with the no-CI note), the cost block, and notes covering: FabGL version pinned, the resolution/mode chosen + RAM, the JPEG decode path used, the host render check's distinct-color count, and the **final pin map table** Stephen wires to (signal, GPIO, landing-by-resistance).
