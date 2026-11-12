---
task_id: "0177"
protocol_version: 1.2.0
status: verified
iteration: 1
expect-reply-within: 48h
cost:
  model: deepseek-flash
  api_calls: 76
  input_tokens: 102966
  input_cache_hit_tokens: 9089920
  input_cache_miss_tokens: 102966
  output_tokens: 70897
  total_tokens: 9263783
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.085253
  cost_note: "Read from this profile's state.db session_model_usage, session
    local-590c55843ed203303ccd109ec25c0f6fec8e660ac2e3bb7cec8477471afdcec1,
    first_seen 2026-11-12T21:15:08Z / last_seen 2026-11-12T21:26:07Z: 76 calls, input_tokens 102966 (cache
    miss), cache_read 9089920, output 70897, reasoning 26884. Arithmetic:
    102966x0.15e-6 + 9089920x0.003e-6 + 70897x0.60e-6 = 0.08525286. Staging and
    reply composition are further API calls; the figure is not re-read afterwards.
    Whole run 21:15-21:45Z Thursday = off-peak (PEAK: OFF-PEAK 21:15 UTC)."
proof:
  - branch: main
    sha: "c03901e63bc9e9ba505b8e71a237ffb8b3492564"
    repo: "pagosacabin/cabin-play"
    note: "Tip of cabin-play main after this run's single commit (round 2). Verified
      equal by `git ls-remote --heads origin main` at 21:32Z, and independently by a
      fresh `git clone --depth 1`, whose HEAD is the same sha and whose `pio run` is
      [SUCCESS] (RAM 26,948 B / 8.2 %, flash 551,325 B / 42.1 %). Round 1's commits
      (5c3b027, 92ea471) remain in the history underneath it."
  - run: none
    run_note: "Earned by enumeration: cabin-play has no `.github` directory at any
      revision -- `ls -a .github` returns 'No such file or directory' and
      `git ls-tree -r --name-only HEAD | grep -c '^.github/workflows/'` returns 0 --
      so no workflow can start on main or on any branch. Build-green evidence is the
      regenerated build-esp32dev.log.txt ([SUCCESS], RAM 26,948 B / 8.2 %, flash
      551,325 B / 42.1 %, 'Hash of data verified' on upload), the clean-clone rebuild
      above, and the on-device console quoted in notes."
  - ntfy: "not applicable, and deliberately not published. The spec names no topic, and
      nordtronics-build-ed05a663 is the companion APK topic. A receipt in the established
      format needs Workflow and Artifacts URLs, which a repo with no CI cannot have;
      publishing one would be a false handoff signal about a different artefact."
  - files:
      - living-frame/firmware/photo-display-64/src/main.cpp
      - living-frame/firmware/photo-display-64/src/photo_render64.h
      - living-frame/firmware/photo-display-64/src/ramp_screen.h
      - living-frame/firmware/photo-display-64/platformio.ini
      - living-frame/firmware/photo-display-64/data/parrot.jpg
      - living-frame/firmware/photo-display-64/data/PHOTO-SOURCE.md
      - living-frame/firmware/photo-display-64/tools/render_check.cpp
      - living-frame/firmware/photo-display-64/tools/render-check.png
      - living-frame/firmware/photo-display-64/tools/render-check-lorikeet.png
      - living-frame/firmware/photo-display-64/tools/render-check-legacy.png
      - living-frame/firmware/photo-display-64/tools/render-check-lorikeet-legacy.png
      - living-frame/firmware/photo-display-64/tools/ramp_preview.cpp
      - living-frame/firmware/photo-display-64/tools/ramp-preview.png
      - living-frame/firmware/photo-display-64/README.md
      - living-frame/firmware/photo-display-64/build-esp32dev.log.txt
notes: |
  STAGED. Run 2026-11-12 20:15-20:30 UTC (13:15-13:30 MST), off-peak (PEAK: OFF-PEAK
  20:15 UTC), PROTOCOL MATCH 1.2.0. Queue: 0142 in active/ carries `status: superseded`
  (Juno's 2026-10-19 resolution) and 0159 is decision-blocked on the UniFi controller
  address; 0176 in inbox/ carries `status: superseded` and "Do not pick up". All three
  were left untouched, so 0177 is the lowest actionable task.

  STEP ONE, AS ASKED: DOES THE PINNED FABGL ACCEPT CUSTOM PINS FOR THE 64-COLOR
  CONTROLLER? Yes, and it is a documented overload, not a patch. Fabricated claim
  avoided by reading the library source rather than the docs: FabGL 1.0.9 has no
  `VGA64Controller` class -- the 64-colour bitmapped controller IS
  `fabgl::VGAController` (`src/fabgl.h:78-84` lists it separately from
  VGA2/4/8/16), and `VGABaseController::begin` has an 8-argument overload
  `begin(red1, red0, green1, green0, blue1, blue0, HSync, VSync)`
  (`vgabasecontroller.h:166`; `vgacontroller.h:78` documents exactly this call for
  64 colours). `vgabasecontroller.cpp:108-110` wires red1GPIO to `VGA_RED_BIT+1`,
  i.e. the `1` suffix is the heavy/MSB bit -- which is what the agreed table needs
  (270R = heavy). So the map below differs from FabGL's defaults on exactly the two
  strapping pins the spec says it moves, and the library needed no local edit.

  AGREED PIN MAP, compiled and confirmed. begin(GPIO22, GPIO21, GPIO19, GPIO18,
  GPIO27, GPIO4, GPIO23, GPIO33) = R MSB/LSB, G MSB/LSB, B MSB/LSB, HSync, VSync.
  Confirmed from the built ELF, not just from the source: the disassembly of
  `setup()` immediately before `call8 fabgl::VGABaseController::begin(gpio_num_t x8)`
  is `movi.n a11, 22 / a12, 21 / a13, 19 / a14, 18`, with a3=27 and the outgoing
  stack slots 4, 23, 33 -- i.e. the eight immediates 22,21,19,18,27,4,23,33 in
  argument order. The console also prints them from the same constants. Eight
  `static_assert`s pin the table at compile time, so a later edit that moves a pin
  without moving the table fails the build instead of lighting the wrong DAC bit.

  WHAT SHIPPED. `living-frame/firmware/photo-display-64` in pagosacabin/cabin-play,
  next to `photo-display`: env esp32dev, platform espressif32@6.12.0 (same as rung 1),
  **fabgl@1.0.9 pinned**, 320x240@60Hz (`QVGA_320x240_60Hz`), 64 colours.
  Mode choice and RAM: the frame buffer is one byte per pixel, so 320x240 = 76,800
  bytes of DMA-capable internal RAM (640x480 would need 307,200 B, more than the
  chip has; 400x300 would need 120,000 B). Measured after the draw: free heap
  172,224 bytes; static RAM 26,432 B (8.1 %), flash 448,025 B (34.2 %). The 640x480
  source is decoded at TJpgDec's 1/2 descale -- exactly 320x240 -- so the whole frame
  fills the screen with no crop, no letterbox and no rescale.

  JPEG DECODE PATH: **TJpgDec R0.03 (ChaN)**, reused in place from
  ../photo-display/lib/TJpgDec via `lib_extra_dirs` (no second copy of the vendored
  decoder; JD_FORMAT 0/RGB888 as rung 1 configured it). MCU block by MCU block
  straight into the frame buffer: no full-image buffer is ever allocated.

  QUANTISATION, and why it is not "4 even levels". The board is a two-path resistor
  ladder (270R heavy / 470R weak) into the monitor's 75R termination, so its four
  codes are 0.000, 0.452, 0.714, 1.000 of full scale -- not 0/0.33/0.67/1. The
  quantiser computes those fractions from the resistor values at startup and dithers
  between the real levels, so the average output voltage tracks the source. An
  even-quarter quantiser would have rendered the shadows about 12 % too bright.

  HOST RENDER CHECK (criterion 6). `tools/render_check.cpp` compiles the SAME
  `src/photo_render64.h` the firmware uses and swaps only the pixel sink; output
  `tools/render-check.png` is committed. Measured:
    pixels drawn: 76,800 / 76,800 (no dropped blocks)
    distinct colors in frame: 24 of 64 (counts 1..19,478)
    block RMS error, 8x8 blocks: R 2.63 G 2.57 B 2.46 (of 255)
    block RMS error, 2x2 blocks: R 11.77 G 10.56 B 11.17
    luma NCC 40x30 0.9997, source rolled 10 px -> -0.0245; NCC 160x120 0.9957
    palette reachability (synthetic 16x16x16 lattice, same quantiser): 64 of 64
  The last line matters: it separates "the firmware cannot use the palette" from
  "this photo does not need it". Per-pixel error is deliberately not quoted as
  fidelity -- for dithered output it is ~half a quantisation step by construction --
  so the block averages above are the honest measure.

  24 OF 64 IS BELOW THE SPEC'S TARGET, AND I AM SAYING SO RATHER THAN BURYING IT.
  Criterion 5 asks for "> 8 (target: most of the 64-color palette on this photo)".
  The > 8 is met. "Most of the 64" is NOT reached, and it is a property of the
  picture: the Blue Marble crop is a near-neutral subject (black space, desaturated
  ocean, white cloud), so channels track each other. Measured independently with
  numpy: nearest-level quantisation of the source to 4 levels per channel reaches
  only **17** of 64 combinations; the dither lifts it to 24. Forcing more palette
  entries would mean adding colour noise the photo does not contain, so I did not.
  The palette itself is fully addressable (64/64 synthetic), which is the part the
  firmware controls.

  SELF-CAUGHT DEFECT, declared because it is invisible in the final diff: the first
  version used ONE Bayer threshold for all three channels. That correlates them --
  in a smooth area R, G and B step up or down together, the frame collapses onto the
  grey diagonal -- and it measured 22 of 64. Giving each channel its own matrix
  offset (BAYER_DX/DY, one per channel) raised the photo to 24 and stops the dither
  pulling hue toward grey. The flashed build is the fixed one.

  FLASHED, NOT LEFT AS A FILE. Board identification before overwriting, because two
  serial devices were attached: /dev/ttyUSB1 is a CP2102 ESP32-D0WD-V3, MAC
  4c:c3:82:da:d5:68 -- exactly the devkit rung 1 was flashed to in 0175 -- and it
  printed `living-frame rung 1 -- photo display` with `frame drawn in 1611 ms` on a
  reset *before* I wrote anything, so there was no doubt which board it was.
  /dev/ttyUSB0 is a different ESP32 (MAC 84:1f:e8:30:a8:50); it was identified with
  `flash_id` only and was NOT flashed. Upload: 448,384 bytes at 0x10000,
  "Hash of data verified".

  ON-DEVICE PROOF (console over the CP2102 at 115200, 759-byte clean capture):
    living-frame rung 2 -- 64-color photo display
    pins: R=22/21 G=19/18 B=27/4 HSYNC=23 VSYNC=33 (msb/lsb)
    FabGL 1.0.9 (pinned), mode 320x240@60Hz, 64 colors (2 bits/channel)
    dac levels (fraction of full scale): 0.000 0.452 0.714 1.000
    photo.jpg embedded: 108557 bytes
    jpeg: 640x480, 3 component(s) -> 320x240 screen (1/2 descale)
    frame drawn in 419 ms (76800 pixels), free heap: 172224 bytes
    palette: 24 of 64 colors used (counts 0..19478)
    holding image
  The embedded JPEG size matches the file byte for byte, all 76,800 pixels reached
  the frame buffer, the draw takes 419 ms, and 172 kB of heap is still free. 419 ms
  of drawing while the DMA scans out means a visible tear for that first fraction of
  a second and a stable picture afterwards -- same caveat as rung 1, stated before
  Stephen sees it. The capture is byte-identical across a re-flash and re-reset.

  BUILD AND REPRODUCIBILITY. Local clean build `[SUCCESS] Took 10.20 s`; fresh
  `git clone --depth 1` of 92ea471, `pio run` there `[SUCCESS] Took 10.11 s`, same
  RAM/flash figures, and the host render check rebuilt from that clone prints the
  same census. The built .bin is NOT byte-reproducible across checkouts (sha256
  abb41eaf... here vs 74c0b722... for the identical sources before the pin-guard
  commit): the ESP-IDF app descriptor embeds the build path and a timestamp. Rung 1
  carries the same note; the quoted hash is not a universal hash.

  WHAT IS NOT PROVEN, stated plainly: nobody has seen rung 2 on a monitor. No VGA
  display and no capture device are attached to this host (`/dev/video*` is the
  laptop's integrated webcam only), so criterion 4's on-screen half -- the Earth,
  smoother than rung 1, recognizably the photo -- is Stephen's acceptance photo and
  remains open. Everything up to the DAC pins is verified; the DAC, the cable and
  the panel are not mine to observe. The compile-time trade is worth flagging before
  he looks: half of rung 1's linear resolution (320x240 vs 640x480) in exchange for
  8x the colour depth, which is what the "dot puzzle" complaint was about.

  DEVIATIONS / SCOPE ADDITIONS, all inside the new project directory (the diff
  7ab46c2..92ea471 touches 10 files, all under photo-display-64, and nothing else
  in the repo):
   - `src/photo_render64.h` + `tools/render_check.cpp` + `tools/render-check.png`
     were not requested. They exist so the decode/quantise/dither path could be
     verified at all without a monitor, and so the firmware and the check cannot
     drift (same file, not a copy).
   - `data/photo.jpg` is a byte-identical copy of rung 1's file
     (sha256 dc6de88234c4900dbbc1df2dac679f10afdcbecd01668218157d6df70192a464)
     rather than a ../ reference, because PlatformIO derives the embedded linker
     symbol from the path inside the project. `data/PHOTO-SOURCE.md` came with it.
   - The eight `static_assert`s are an addition beyond "put the table in the
     README": they are the mechanism that makes "the compiled pin map matches the
     agreed table" checkable by the compiler instead of by eye.
   - `build-esp32dev.log.txt` and `.gitignore` follow the rung-1 convention;
     `tools/render-check.ppm` is git-ignored and regenerable from the commands in
     the README.
   - PICKUP MECHANICS, declared because the history is visible: the first pickup
     commit (ebc3613) was made with a pathspec limited to the new active path, so it
     recorded the active file as a create and left the inbox deletion staged; a
     second commit (5b69427) completed the move. Net effect is the intended one
     (one file in active/, none in inbox/) but it is two commits, not one.
     `expect-reply-within: 48h` was a body-level line in the filed task; it was
     folded into the front-matter at pickup.
   - BENCH OBSERVATION, not a defect: `esptool flash_id` on ttyUSB1 reports
     "Detected crystal freq 15.55MHz is quite different to normalized freq 26MHz"
     before going on to read MAC and flash size normally. The board boots, the
     console is clean at 115200, and rung 1 flashed and ran on it the same way, so
     this is recorded rather than diagnosed -- it was not introduced by this task.

  NEXT: Stephen powers the board with a VGA monitor attached and photographs the
  screen. The board is already flashed and holding the frame. If the picture is
  rolled, clipped or colour-inverted, the signal order in the table above is the
  first thing to check against the wiring, since MSB/LSB is identified by measured
  resistance (270R heavy, 470R weak) rather than by silkscreen position.

  JUNO REJECTION (2026-11-12 ~13:45 MST) — staged -> active, iteration unchanged.
  Proof verified GREEN first: commit 92ea471 matches the cabin-play main tip,
  the build log and clean-clone rebuild check out, and the cost arithmetic
  checks ($0.0724). This rejection is ONLY on the acceptance criterion, which
  is Stephen's eyes — not on the proof.

  WHAT STEPHEN SAW (his photo of the 213T, 2026-11-12 13:37 MST): the Earth
  still reads as a dot puzzle and the black background is still speckled. His
  words: "Same as before something isn't right." The criterion "visibly
  smoother than rung 1 — recognizably the photo, not a dot puzzle" is not met
  on this evidence, so the task is not closable yet. Two structural causes are
  on the record (320x240 halves rung 1's linear resolution; this board's
  first DAC step is 45% of full scale, so dark areas dither coarsely) — but a
  bit-order or wiring fault would look similar from the couch, and that is
  unverified either way. Round 2 separates the two.

  ROUND 2 — DIAGNOSTIC SCREEN + A COLORFUL SUBJECT (same project,
  photo-display-64; agreed pin map and 320x240@60 mode unchanged unless the
  ramp proves a fault):
  1. Add a test display (separate env, alternate image, or a boot-time screen
     before the photo — your choice, stated in the reply) showing: (a) a
     4-step ramp for EACH channel separately (R, G, B), steps in the order
     0, LSB only, MSB only, both; (b) a combined grey ramp; (c) one smooth
     gradient. Stephen photographs the ramp: steps in order, progressing
     evenly, proves wiring + DAC + bit order per channel.
  2. Embed a second photo chosen for saturated color variety (public domain
     or CC — a macaw/parrot or autumn-foliage photo, your pick), run through
     the identical pipeline, so the 64-color benefit gets judged on a
     subject that uses the palette. Keep the Blue Marble build available
     unchanged for comparison.
  3. If the ramp exposes a bit-order or channel fault, name the exact
     channel and bit and fix it FIRMWARE-side (remap in code) — Stephen does
     not rewire unless you show the fault is physical.

  Acceptance for round 2: Stephen's photos of (a) the ramp with steps in
  order, and (b) the colorful photo — his verdict on (b) closes the task.
  Reply in the usual staged form; carry the round-1 proof forward and add
  what round 2 changed.

  ROUND 2 FILED — STAGED. Run 2026-11-12 21:15-21:45 UTC (14:15-14:45 MST), off-peak
  (PEAK: OFF-PEAK 21:15 UTC), PROTOCOL MATCH 1.2.0. Queue at pickup: 0142 in active/
  carries `status: superseded` (Juno's 2026-10-19 resolution), 0159 is decision-blocked on
  the UniFi controller address, 0176 in inbox/ carries `status: superseded` and "Do not
  pick up". All three left untouched. 0177 was in active/ with Juno's rejection note and
  her round-2 spec, so it is the lowest actionable task and this run implements that spec.

  STEP 1 — THE DIAGNOSTIC RAMP IS BUILT AND IS THE FIRST THING THE BOARD SHOWS. Boot
  order is ramp (held 30 s, so it can be photographed) -> lorikeet photo. Per channel the
  row is four patches left to right in the order 0, LSB only, MSB only, both; then the
  same four levels on all three channels at once (W row); then a dithered black->white
  gradient. Layout, font and gradient live in `src/ramp_screen.h`, which the firmware and
  a host preview (`tools/ramp_preview.cpp` -> `tools/ramp-preview.png`) BOTH compile, so
  the committed preview is the board's own drawing code, not a mock-up. Sampling each
  patch's centre in the preview returns:
      row R: 00 01 02 03      row G: 00 04 08 0C
      row B: 00 10 20 30      row W: 00 15 2A 3F
  i.e. bit 0 (LSB) then bit 1 (MSB) rise in that order on every channel, and the W row
  raises all three together. What the steps should measure, from the ladder:
      level 0 = 0.000 full scale, code 0      level 1 (LSB) = 0.452, code 115
      level 2 (MSB) = 0.714, code 182          level 3 (both) = 1.000, code 255
  and, with the panel's gamma, those codes show as 0 / 45 / 122 / 255 of perceived light —
  which is the second half of the diagnosis (see step 3).

  STEP 2 — SECOND PHOTO, CHOSEN BY MEASUREMENT, NOT BY EYE. The spec allowed a macaw or
  autumn foliage; five candidates were run through this project's own pipeline and scored
  on palette entries actually used and mean saturation of the 320x240 frame:
      Blue Marble (round 1)          24/64   12% saturated   mean sat 0.16
      scarlet macaw, zoo close-up    36/64   12%            0.16
      scarlet macaw, CC0             31/64   34%            0.28
      autumn foliage, CC0            42/64   97%            0.80
      rainbow lorikeet, CC BY 4.0    45/64   68%            0.42   <- shipped
  The lorikeet wins the metric the task named ("a subject that uses the palette") and is a
  parrot, as suggested. `data/parrot.jpg` is 640x480, sha256
  4eeed2ba3e7126963d93ce00f34586499488fdd872853c44fc86abbfcafc5415; provenance, licence
  (CC BY 4.0, attribution required, no ShareAlike) and the exact transform are in
  `data/PHOTO-SOURCE.md`. Blue Marble stays byte-identical (sha256 dc6de882...) and stays
  on screen: key 3 (new pipeline) and key 5 (round-1 pipeline).

  STEP 3 — THE SPECKLE HAS A NAMED CAUSE AND A FIX, AND IT IS THE THING THAT WAS WRONG.
  Round 1 dithered so the average *voltage* matched the source. A panel fed analogue VGA
  maps voltage to luminance as V^2.2, so matching average voltage displays average light
  of s^0.454 — everything too bright — and shadows are the worst case: the ladder's first
  step (0.452 of full scale) is sRGB 115, i.e. 18% of full light, a MID-GREY dot. Any
  shadow detail had to be drawn as a sprinkle of mid-grey dots on black. That is Stephen's
  "black background is still speckled", and round 1's byte-domain RMS metric could not see
  it because averaging sRGB codes is not averaging light.
  The quantiser now gamma-decodes the source and dithers in the LIGHT domain
  (`PHOTO64_MODE_PERCEPTUAL`, gamma 2.2 as the display model, one constant). Measured on
  the host check, same code the firmware runs:
      Blue Marble  light-domain 8x8 RMS 6.05/5.87/5.02 -> 2.36/2.46/2.75
                   near-black source pixels lit        532 of 17,291 (3.1%) -> 1 (0.0%)
      lorikeet     light-domain 8x8 RMS 6.87/6.96/6.51 -> 1.67/1.53/1.29
                   near-black source pixels lit       2,240 of 14,277 (15.7%) -> 30 (0.2%)
  The round-1 pipeline is retained and selectable (keys 4 and 5) so the two can be compared
  on the same monitor. This is a scope extension beyond the literal round-2 list and is
  declared as such: the rejection said "the black is still speckled", and this is the
  mechanism plus a measurement, not a guess.

  FLASHED AND EXERCISED, NOT LEFT AS A FILE. Board identified before writing: /dev/ttyUSB1,
  CP2102, MAC 4c:c3:82:da:d5:68 — the same ESP32-WROOM-32 devkit rung 1 and round 1 used.
  /dev/ttyUSB0 (a different chip) was not touched. Upload 551,696 bytes at 0x10000, "Hash of
  data verified". Console (clean capture, 755 bytes):
      living-frame rung 2b -- 64-color diagnostic + two photos
      pins: R=22/21 G=19/18 B=27/4 HSYNC=23 VSYNC=33 (msb/lsb)
      dac levels (fraction of full scale): 0.000 0.452 0.714 1.000
      level sRGB code sent to DAC: 0 115 182 255
      level luminance as sRGB code: 0 45 122 255  <- step 1 is mid-grey
      ramp order per channel, left to right: 0, LSB only, MSB only, both
      screen: diagnostic ramp, held 30 s (power-cycle to see it again)
      photo lorikeet: 99010 bytes embedded, 640x480 -> 320x240 (1/2 descale)
        drawn in 467 ms (76800 pixels), mode=perceptual, free heap: 171704 bytes
        palette: 45 of 64 colors used (counts 0..36423)
      holding lorikeet image; keys 1-5 switch screens, q cuts a wait short
  All five screens were then driven over the serial port one by one and each answered with
  its own log line: key 3 blue-marble perceptual (24/64), key 5 blue-marble legacy (24/64),
  key 4 lorikeet legacy (40/64), key 1 ramp, key 2 lorikeet perceptual (45/64). The board
  is left holding the lorikeet.

  REPRODUCIBILITY. Fresh `git clone --depth 1` of c03901e: `pio run` [SUCCESS] (RAM 26,948 B
  / 8.2%, flash 551,325 B / 42.1%), and the host checks rebuilt from that clone print the
  same numbers quoted above (45/64 and 1.67/1.53/1.29 for the lorikeet; 24/64 and
  6.05/5.87/5.02 for Blue Marble in legacy mode). `build-esp32dev.log.txt` is regenerated
  from a clean build and re-flash by this run.

  WHAT IS STILL NOT PROVEN, stated plainly: nobody on this machine has seen the screen. No
  VGA display and no capture device are attached (`/dev/video*` is the laptop webcam), so
  the ramp's *appearance*, and Stephen's verdict on the lorikeet, remain exactly the
  acceptance criteria the job now asks for. What this run can and does prove is that the
  board emits the intended bit order and level order, that both JPEGs decode to the full
  76,800-pixel frame, and that the new dither is measurably closer in light and 25x cleaner
  in the near-black than the pipeline he rejected.

  DEVIATIONS AND SCOPE ADDITIONS, all inside living-frame/firmware/photo-display-64:
   - `src/ramp_screen.h` + `tools/ramp_preview.cpp` + `tools/ramp-preview.png` are additions;
     they exist so the ramp layout is reviewable and cannot drift from what the board runs.
   - `tools/render-check-lorikeet.png`, `-legacy.png`, `-lorikeet-legacy.png` are additions:
     the A/B images for the two pipelines, so the claim about the dither is inspectable.
   - `data/parrot.jpg` is CC BY 4.0 rather than CC0; attribution is recorded in
     `data/PHOTO-SOURCE.md` as the licence requires (no ShareAlike obligation).
   - The commit lands on `main` of pagosacabin/cabin-play, matching round 1 and the previous
     accepted proof; that repo has no CI and no PR flow (`ls .github` → absent, 0 workflow
     files at any revision), which is why `run: none` is earned rather than asserted.
   - BENCH NOTE, declared because it is my own tooling and not the firmware: the DTR/RTS
     ladder I used to reset the board into a capture produced a ~30 s run of ROM "try
     0x400805e4" lines and repeated truncated banners before the app came up normally. The
     same firmware had already booted cleanly from esptool's reset seconds earlier (the 755
     -byte capture above), every key press afterwards answered immediately, and no reset
     artefact appears when the port is opened without touching the lines. Recorded as a
     capture artefact, not a boot defect — `stty`-free pyserial with the lines left alone is
     the method that produced the clean log.
   - The build log records PlatformIO's own warning that an obsolete PIO Core (6.1.19) is in
     use while 6.2.0 exists on the machine. It built and flashed correctly; recorded, not
     fixed, because changing the toolchain is outside this task.

---


# 0177 — Living frame rung 2: 64-color photo display (FabGL, existing DAC board)


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

---

# Reply — DONE, built, flashed and running (2026-11-12 20:30 UTC)

## What it is

`living-frame/firmware/photo-display-64` in `pagosacabin/cabin-play`, on `main` at
`92ea471`. A classic ESP32-WROOM-32 (env `esp32dev`) showing rung 1's Blue Marble
JPEG full-screen at **320×240 in 64 colours** — 2 bits per channel — through
**FabGL 1.0.9** on the existing 2-bit DAC board, to the agreed pin map.

| | |
|---|---|
| controller | `fabgl::VGAController` (the 64-colour one), `begin(22,21,19,18,27,4,23,33)` |
| mode | `QVGA_320x240_60Hz`, 1 byte/pixel → 76,800 B frame buffer |
| decoder | **TJpgDec R0.03**, 1/2 descale of the 640×480 source |
| colour | 4 levels/channel, 8×8 Bayer dither between the ladder's **real** levels |
| heap after draw | 172,224 bytes free |
| photo | rung 1's file, byte-identical (sha256 `dc6de882…`), embedded in flash |

## Step one, answered

FabGL 1.0.9 has no `VGA64Controller`; the 64-colour controller is `VGAController`,
and its 8-argument `begin()` overload is the documented way to give custom pins
(`vgabasecontroller.h:166`, example at `vgacontroller.h:78`). The `1` in `red1GPIO`
is the heavy bit (`vgabasecontroller.cpp:108`), which is the bit the 270 Ω path
wants. So the agreed map is a supported configuration, not a fork — and the built
ELF's call site carries the immediates 22,21,19,18,27,4,23,33 in argument order.

## Verified, and how

* **Build** — `pio run`: `[SUCCESS]`, RAM 8.1 % (26,432 B), flash 34.2 %
  (448,025 B). Log committed.
* **Clean clone** — `git clone --depth 1` of `92ea471`, `pio run`: `[SUCCESS]
  Took 10.11 seconds`, identical size figures; the host render check rebuilt there
  prints the same numbers.
* **Flash** — the rung-1 board (CP2102, MAC `4c:c3:82:da:d5:68`), identified by its
  own boot banner *before* being overwritten: 448,384 bytes at `0x10000`,
  `Hash of data verified`.
* **On the chip** —

  ```
  pins: R=22/21 G=19/18 B=27/4 HSYNC=23 VSYNC=33 (msb/lsb)
  FabGL 1.0.9 (pinned), mode 320x240@60Hz, 64 colors (2 bits/channel)
  dac levels (fraction of full scale): 0.000 0.452 0.714 1.000
  jpeg: 640x480, 3 component(s) -> 320x240 screen (1/2 descale)
  frame drawn in 419 ms (76800 pixels), free heap: 172224 bytes
  palette: 24 of 64 colors used (counts 0..19478)
  holding image
  ```

* **Off-target** — `tools/render_check.cpp` runs the same header on the host:
  24 of 64 palette entries on this photo, block-averaged RMS error 2.5 of 255,
  luma NCC 0.9997 (−0.02 with the source rolled 10 px), and 64 of 64 reachable with
  a synthetic colour lattice.

## The one criterion that came up short

"Most of the 64-color palette on this photo" is **not** met: the frame uses 24.
The picture is near-neutral, so 4 levels per channel only ever produce 17
combinations before dithering and 24 with it; the palette is fully addressable
(64/64 synthetic), so the limit is the subject, not the firmware. I did not add
colour noise to inflate the count.

## What is still open

**Nobody has seen it on a monitor.** No VGA display or capture device is attached
here, so the on-screen half of criterion 4 is Stephen's acceptance photo.
Everything up to the DAC pins is verified.

## For Stephen

The board is flashed and holding the frame — attach a monitor and power it. Rebuild
or re-flash from a clean checkout with:

```bash
cd living-frame/firmware/photo-display-64
pio run                 # build
pio run -t upload       # flash (ESP32-WROOM-32 devkit, CP2102, 460800)
```

Expect half of rung 1's resolution (320×240) and eight times the colour depth;
if the picture is rolled or colour-inverted, check the MSB/LSB order in the README
table against the wiring first.

---

## Round 2 reply

**Status: implemented, flashed, measured — waiting on Stephen's two photographs.**

Juno's round 2 asked for three things; all three ship in one firmware and one commit
(`c03901e` on `pagosacabin/cabin-play`):

1. **Diagnostic ramp** — first thing on screen at boot, held 30 s. Four patches per channel
   in the order 0 / LSB only / MSB only / both, then a grey row, then a dithered gradient.
   The layout is in `src/ramp_screen.h`, shared with `tools/ramp_preview.cpp`, so
   `tools/ramp-preview.png` is the board's own drawing code rather than a mock-up.
2. **A colourful subject** — rainbow lorikeet, CC BY 4.0, chosen by measurement: it is the
   only candidate that addresses **45 of the 64** palette entries (Blue Marble: 24). Blue
   Marble is unchanged and still selectable.
3. **The speckle, diagnosed** — the ladder's first step is sRGB 115, i.e. **18 % of full
   light**: a mid-grey dot. Round 1 dithered to match average *voltage*, so shadows became a
   sprinkle of mid-grey dots. The quantiser now dithers in the **light** domain: near-black
   pixels lit fall 3.1 % → 0.0 % (Blue Marble) and 15.7 % → 0.2 % (lorikeet), and
   light-domain 8×8 block RMS falls 6.05 → 2.36 and 6.87 → 1.67. Keys 4 and 5 still show the
   round-1 pipeline for A/B on the same monitor.

Five screens on the serial port (115200): `1` ramp, `2` lorikeet (default), `3` Blue Marble,
`4` lorikeet round-1 pipeline, `5` Blue Marble round-1 pipeline. Boot needs no terminal.

## For Stephen

The board is flashed and holding the lorikeet. Power-cycle it and it shows the **ramp for
30 seconds**, then the lorikeet. Two photographs close this task:

1. **the ramp** — the four steps on each of the R, G, B rows should get brighter in order,
   left to right, and the W row should look neutral grey. If one row's steps are out of
   order, that channel's two bits are swapped; if a row is markedly brighter or dimmer than
   its siblings, the resistor paths for that channel are suspect; if all rows step evenly,
   the digital side and the wiring are both good.
2. **the lorikeet** — the colourful subject, judged for whether it reads as the photo rather
   than a dot puzzle.

If the ramp looks right and the photo still disappoints, that narrows it to the panel at
320×240 with four levels per channel, and the next lever is a different mode (more pixels,
fewer colours) rather than more colour depth.
