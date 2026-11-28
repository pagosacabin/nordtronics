---
task_id: "0280"
status: in_progress
iteration: 2
proof: []
notes: "Filed 2026-11-27 on Stephen's word ('File them now'). Drafted as 0269; bench is up — proto board (YD-ESP32-S3 + VGA + SD) tested and working Nov 27, frame + camera on the bench."
expect-reply-within: 6h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-28T03:00:50+00:00"
lease_until: "2026-11-28T04:30:50+00:00"
---

# 0280 — Live-photo colour: find the warmer look again (drafted as 0269)

## Context

Stephen (2026-11-26): the camera shots used to look warmer; at some point
they went duller. He does not know when it changed. The bench is back up
now (proto board tested and working Nov 27), so this is filed on his word.

What Juno verified in code (branch hermes/0261-mirror-live-photo
@ b5c0f525, file living-frame/firmware/freenove-cam/src/main.cpp):

- Sensor init: `set_brightness(s, 1)`, `set_saturation(s, -1)` — saturation
  one step below the driver's neutral 0 (driver scale -2..+2; confirm on
  the bench).
- Grey-world white balance, gains computed FRESH for every shot, strength
  `WB_STRENGTH 0.5` (line 44), gains clamped 0.5..2.0, NaN-safe, no
  carry-over between shots. Applied per pixel in build_raster via
  wb_code() before quantisation. The shoot log prints the gains:
  `wb r ... g ... b ... (strength ...)`.
- 256-entry gamma LUT, `DAC_GAMMA 2.2` (line 140); build_quant_table()
  linearises each channel's own DAC ladder with the panel gamma; level v
  emits (v/3)^2.2 of full scale. Raster then Bayer-dithers into the
  64-colour palette (2 bits/channel).

Timeline (commits on the 0261 branch): both colour changes landed Nov 23,
the same day the live shoot flow (0244) started producing photos.
- 0249 (113c0679, Nov 23): per-shot WB. Before it, gains were smoothed
  60/40 across frames (a streaming leftover) and a black frame could carry
  NaN until reboot.
- 0250 (7990efbf, f701ecf1 Nov 23; 2f6fca24 Nov 24): per-channel DAC level
  table + 12 calibration test frames, measured DAC ladder, gamma 2.2.
- Before Nov 23 there was no live shoot flow at all — photos came from
  convert_photo64.py (host-side: Floyd-Steinberg dither, no WB, no gamma
  tables). So the "warmer look" Stephen remembers is most likely the
  host-converted photos, or the very first live shots before 0249/0250.

Suspects for warmer -> duller, in order:
1. Grey-world WB at 0.5 strength pushes the scene average toward neutral
   grey — a warm-lit scene gets its warmth corrected away.
2. Saturation -1 (one step desaturated vs neutral).
3. Gamma 2.2 darkens midtones; can read as flatter vs a lower gamma.
The 0250 DAC ladder itself was measured per channel, so the quant tables
are calibrated to the actual panel — do not change them without measuring.

## Task

1. Bench up: frame + camera plugged in, 0261 build still on both boards
   (do not re-flash unless the build is not the 0261 one — check first).
2. Shoot a warm reference scene (warm light and/or warm-coloured subject,
   same scene for every test shot). Record the shoot-log wb gains for
   each shot.
3. A/B the knobs, one at a time, same scene each time:
   - WB_STRENGTH 0.5 -> 0.25 -> 0 (0 = no white-balance correction).
   - Saturation -1 -> 0 -> +1.
   Keep the DAC ladder tables and gamma 2.2 untouched during the A/B.
4. Show Stephen the candidates (photos on the panel, or captured frames);
   he picks the warmth by eye. Implement his pick.
5. Verify with a fresh shoot of the same scene, and confirm the 0261
   mirror fix still holds (text reads correctly).

## Success criteria

- A/B shots exist for each knob step with the shoot-log wb gains
  recorded.
- Stephen has picked a combination by eye, it is implemented, and a
  fresh shoot shows the warmer look with the mirror fix intact.
- Reply staged with the gains, the candidates, and his pick.

## Constraints

- Bench-only task: frame + camera must be plugged in. Do not run this
  while the bench is down.
- Do not change the DAC ladder tables or gamma without a measured
  reason — the dullness is most likely WB + saturation.
- Do not touch the splash, the stored library, or convert_photo64.py.
- Do not re-flash a board that already runs the 0261 build.

## Proof

- Shoot logs with wb gains per candidate, plus before/after captures or
  panel photos. Host-side build only for the A/B firmware variants; the
  chosen variant is flashed to the camera board with the flash log as
  proof. No repo merge (merges need Stephen's word).

## Reply format

Short report: gains per candidate, which combination Stephen picked, fresh
shoot result, mirror check.

## Assignment

Hermes does the bench work himself. No Newton (firmware is Hermes's),
no Copilot (not GitHub-bound until a change is picked — if a change is
picked, add the Copilot review step before any merge is proposed).
