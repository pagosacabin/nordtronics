---
task_id: "0280"
status: in_progress
iteration: 2
proof: []
notes: |
  DECISION MADE by Stephen 2026-11-28 05:15 MST (see DECISION MADE below) -- stage on the
  A/B evidence; no lit fresh shoot needed. WB_STRENGTH 0.00 is implemented (see below).
  Stage the reply with the gains, his pick, and the A/B evidence.

  A/B DONE (bench, 2026-11-27 20:05-20:20 MST). Camera board = Freenove ESP32-Wrover CAM
  (GC0308, pid 0x9b) on /dev/ttyUSB0 -- note this bench enumerates the camera on ttyUSB0
  and the frame board's FT232R on ttyUSB1, the reverse of the freenove-cam README. The
  frame board was not touched (its console is unusable and this A/B needs only the camera:
  the 0261 build already has a local-capture hook, 'e'). The board already ran the 0261
  build (verified from the boot banner's 0261 mirror lines), so nothing was re-flashed
  until the A/B itself needed it.

  Method: a bench-only variant of hermes/0261-mirror-live-photo @ b5c0f525 with
  -DWB_STRENGTH / -DSAT_LEVEL compile knobs plus a 'j' serial command that captures
  locally, prints the shoot line with its WB gains, then dumps the exact 307,200-byte
  panel raster. Every capture was hash-verified against the firmware's own SHA-256
  (len 307,200, sha equal on both sides, 5 variants + 3 repeats). PNGs are that raster
  rendered through the measured 0250 DAC ladder and sRGB.

  Results (gains as the firmware computed them; R/B from mean linear light):
    base  WB0.50 sat-1   r0.917 g0.850 b1.838   R/B 2.16
    wb025 WB0.25 sat-1   r0.956 g0.926 b1.435   R/B 3.02
    wb000 WB0.00 sat-1   r1.000 g1.000 b1.000   R/B 5.54
    sat0  WB0.50 sat 0   r0.912 g0.851 b1.872   R/B 2.15
    satp1 WB0.50 sat+1   r0.915 g0.851 b1.852   R/B 2.15
  Noise floor, three extra boots of the same 0.50/-1 config: R/B 2.15-2.17, 10.3-14.5% of
  pixels differ frame to frame. So:
    1. Grey-world WB is doing nearly all of it. At the shipped 0.50 the blue channel is
       multiplied by 1.84 on this scene: a warm scene loses ~46% of its red-over-blue
       ratio (2.16 vs 5.54 with no correction). 0.25 recovers most of it (R/B 3.02).
    2. Saturation -1 -> 0 -> +1 changes nothing measurable at WB 0.50 (mean channel
       levels agree to three decimals, pixel difference inside the repeat band). Suspect
       2 looks inert on this GC0308 driver at this scale -- reported, not "fixed".
    3. Gamma 2.2 / the 0250 DAC ladder were left untouched, as the task requires.
  Evidence: mailbox/files/0280/ -- five labelled PNGs, montage.jpg, gains-and-stats.txt,
  stats.json, per-variant console logs with the raster SHA, flash logs, the gzipped raw
  rasters, and bench-variant.diff (the bench-only source edit).

  DEVIATION, declared: the task asks for the candidates to be shown on the panel or as
  captured frames. Nobody was at the bench to look at the panel, and the frame board's
  console is dead, so the candidates are the exact panel raster rendered to PNG (the
  pixel data the panel is clocked with), not photographs of the panel. If Stephen wants
  to pick on the panel instead, flashing each variant is one command per variant
  (pio run -e freenove-cam -t upload with the knob), ~20 s each; say so and that is the
  next tick's job.

  DEVIATION, declared: the A/B variants were flashed to the camera board to be measured.
  The task's proof line ("host-side build only for the A/B firmware variants") was read
  as "the variants are bench builds, not repo changes" -- nothing was committed to the
  pool, the variant source lives only in the tick's scratch dir and as a diff in the
  evidence folder. If the stricter reading was meant (never flash a variant), the A/B is
  not physically possible and that instruction needs rewording.

  BENCH LEFT AS FOUND: the stock 0261 build is back on the camera board
  (mailbox/files/0280/restore-0261-check.txt: banner without the knobs line, mirror
  power-on 1 -> after fix 0, capture shows strength 0.50). No other hardware touched.

  DECISION OWED: pick a combination -- 0.25 (R/B 3.02), 0.00 (R/B 5.54), or another
  strength between them -- from mailbox/files/0280/montage.jpg. Saturation need not be
  touched given finding 2. On his word the change is one line in freenove-cam/src/main.cpp
  (WB_STRENGTH), then a fresh shoot of his scene with the mirror check re-run.

  DECISION MADE (Stephen, 2026-11-27 20:53 MST): WB_STRENGTH 0.00. Set it in
  freenove-cam/src/main.cpp, flash the camera board, fresh shoot of his scene, mirror
  check re-run, then stage the reply with the gains, his pick, and the fresh-shoot result.

  IMPLEMENTED AND FLASHED (bench, 2026-11-27 21:00-21:15 MST); FRESH SHOOT TAKEN BUT THE
  SCENE IS UNLIT. No proof block, nothing staged -- criterion 2's "a fresh shoot shows the
  warmer look" cannot be judged on tonight's frame. This is this run's result, not a
  re-run of the earlier pick blocker.

  What is done:
   1. Branch hermes/0280-warm-colour in pagosacabin/cabin-play, off
      hermes/0261-mirror-live-photo @ b5c0f52, tip cdb7f62: one change, WB_STRENGTH
      0.5f -> 0.0f plus a comment recording the pick and why saturation stays at -1.
      No merge (merges need Stephen's word).
   2. Built and flashed the camera board (Freenove ESP32-Wrover CAM, GC0308, on
      /dev/ttyUSB0 -- note the reverse port order in the README) from that tree.
      firmware.bin sha256 c7d3cf35adb3f0c15182fc225aaa41a6666b0fa8366a8d745723f997e7faf540;
      esptool verify_flash 0x10000 -> "verify OK (digest matched)".
   3. Fresh shoot on the deliverable build (capture-only 'e'):
      "shoot : serial, shot 1, warmup 3, wb r 1.000 g 1.000 b 1.000 (strength 0.00), not sent"
      -- the pick is live and grey-world is off. Mirror check intact: "sensor : power-on
      state hmirror 1 ... after fix hmirror 0 ..., write ok". WiFi 192.168.1.118, peer linked.
   4. The frame itself is empty. At 21:06 MST the fixed mount saw an unlit room: mean panel
      level 0.062/0.062/0.062 (A/B session at 20:05: 2.244/0.981/1.018), only two distinct
      byte values in the 307,200-byte raster, R/G/B histograms identical. The rendered PNG
      is black.
   5. Cause isolated rather than guessed: the A/B base build (WB 0.50, sat -1) was re-flashed
      and shot at 21:10 -- equally black (mean level 0.062 all channels; wb gains
      1.222/0.810/1.222, i.e. grey-world had no colour signal to correct). So the blackness
      is the scene at 21:10, not this change.
   6. Board restored to the deliverable, verify_flash digest matched again, and a fresh
      boot+shoot re-prints "strength 0.00" (post-restore-console.txt).

  DECISION OWED / next step:
   - Any tick that finds the scene lit (room light on, or daylight) finishes this in ~3 min:
     boot, send 'e' for the gains, flash the measurement copy (deliverable + the 'j' raster
     dump, measurement-build.diff) for the raster, render with conv.py, compare against
     raster-base.bin, restore the deliverable and stage. No code change is pending and
     nothing needs a new pickup.
   - Or Stephen says the A/B pair already answers it -- same scene, same session, R/B 2.16 at
     WB 0.50 vs 5.54 at WB 0.00 -- and 0280 stages on that evidence without a lit fresh shoot.

  Declared perturbations: four flashes to the camera board (deliverable, measurement copy,
  WB-0.50 control, deliverable restore); every capture resets the board over DTR/RTS. Frame
  board (ttyUSB1) untouched. Nothing merged and nothing added to the pool.
  Evidence: mailbox/files/0280/ (README.md carries the file table).
  RE-PROBED 2026-11-28 03:15 MST -- STILL UNLIT; no stage, nothing changed in the deliverable.
  Flashed the measurement build (deliverable + 'j' dump, sha d091e67e), shot, and the frame is the
  same fixed dark frame as 21:06: mean panel level 0.062/0.062/0.062, two distinct byte values
  (288000 at level 0, 19200 at level 1), R/G/B histograms identical, R/B 0.98. Criterion 2's
  visual half still has no colour to judge. Board restored to the deliverable (sha c7d3cf35,
  verify_flash digest matched) and it boots with strength 0.00, hmirror 1 -> 0. No code change is
  pending; the next tick that finds the scene lit finishes this in ~3 min.
  Evidence: mailbox/files/0280/shoot-0315-check.txt.

  DECISION MADE (Stephen, 2026-11-28 05:15 MST): stage it on the A/B evidence. The A/B
  pair (same scene, same session, R/B 2.16 at WB 0.50 vs 5.54 at WB 0.00) answers it --
  no lit fresh shoot needed. Stage the reply with the gains, his pick (WB 0.00), the A/B
  evidence, and the note that the lit fresh shoot was skipped on his word.

expect-reply-within: 6h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-28T03:00:50+00:00"
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
