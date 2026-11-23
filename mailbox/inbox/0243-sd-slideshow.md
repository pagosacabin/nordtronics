---
task_id: '0243'
status: pending
iteration: 1
proof: []
expect-reply-within: 12h
notes: |
  Filed 2026-11-22 19:12 MST at Stephen's direction in chat, with his
  amendment: Hermes collects the ten images himself (downloads them),
  puts them on the SD card, then builds the slideshow. Window is 12h
  (not the usual 6h) because filing lands inside the evening peak guard
  and the overnight guard would eat a 6h window before the worker can
  run — the window spans the stand-down, it is not a priority signal.
---

# 0243 — SD-card slideshow: ten images on the card, rotating on screen

## Assignment

- **Hermes:** all of it — image collection, conversion, staging onto the
  card, firmware, bench run, report. (Bench firmware stays yours;
  nothing here goes to Newton.)
- **Copilot:** peer-review the firmware change before the bench run, per
  the standing GitHub rule. Reviewer, not proof — Juno verifies the
  staged reply independently.
- **Juno:** verifies the reply (branch tip vs origin, log numbers vs the
  committed report), archives, pings Stephen.

## Context

The living frame's storage chain is proven end to end: 0239 folded the
pre-sacrifice into the stock driver (first-init 14/14), 0241 formatted
the new card and passed format → mount → write → read back → remount →
read back with matching hashes. The card now in the holder is the 32 GB
SanDisk from 0241 (CID PSN 0xc6bd0f92), formatted FAT32, with
/NEWCARD.BIN on it. The accepted display build is the 800x600 gamma
photo build (framebuffer sha256 204578cc…, 60.35 Hz), which today shows
one embedded image compiled in from data/hummingbird.jpg via
tools/convert_photo64.py --mode 800x600 --space gamma.

This task turns the frame into a slideshow: images live on the SD card,
the firmware cycles them on screen. It is a feature build, not a probe —
the end state is the board left running the slideshow for Stephen to
judge with his own eyes.

Base branch: `hermes/0241-newcard-write`
@ ceb31e1006d45187efb3aadbcefe49cefbf06545 in pagosacabin/cabin-play,
project `living-frame/firmware/n16r8-sdprobe/` (the proven SD stack and
the photo display build in one tree). Board: YD-ESP32-23 / N16R8, MAC
c4:9e:7e:08:6d:f4. SD pins SCK 12 / MOSI 11 / MISO 13 / CS 10, SPI mode
0, 400 kHz.

## Task

Read the board's pre-state from the report partition before flashing
anything, and say what was on the board when you arrived.

1. **Collect ten images.** Download ten varied, colourful, landscape-
   orientation photos that suit a photo frame (nature, landscapes,
   animals, places) from public-domain or freely-licensed sources
   (Wikimedia Commons or similar). Source resolution at least 800x600.
   Record the source of each in the reply. These are bench/stand-in
   photos for the mechanism — Stephen will curate real ones later.
2. **Convert.** Run each through the existing converter
   (tools/convert_photo64.py --mode 800x600 --space gamma) into the
   accepted build's framebuffer format. Name the outputs so filename
   order is the play order (e.g. FRAME01, FRAME02, …).
3. **Stage onto the card.** Write the ten frame files onto the card in
   the holder, into a dedicated directory (e.g. /FRAMES), through the
   proven write path. Leave /NEWCARD.BIN untouched — it is 0241's
   evidence.
4. **Slideshow firmware.** Extend the photo build so that at boot it:
   - initialises the SD card with the 0239 fold (pre-sacrifice + stock
     driver) and enumerates the frame files in the directory, sorted by
     filename;
   - loads the first frame into the framebuffer and displays it;
   - every ~10 seconds loads the next frame, looping to the first after
     the last;
   - skips a frame file that is the wrong size or fails to read, logs
     the skip, and moves on;
   - if the card is absent, unmountable, or holds no valid frames,
     falls back to the embedded hummingbird exactly as the accepted
     build behaves today — the screen never goes blank for a storage
     reason.
   Log every frame load (filename, bytes read, timestamp/elapsed) to the
   report, in the style the previous tasks established — and mind
   0241's lesson: keep DEBUG/VERBOSE out of the report buffer.
5. **Run it and leave it running.** Flash the slideshow build, let it
   cycle through at least one full loop of all ten frames plus the
   wrap-around, read the report back, and leave the board running the
   slideshow at the end (no restore this time — Stephen judges the
   result live).

## Success criteria

- Ten images collected (sources listed), converted, and present on the
  card in the frames directory.
- Report log shows each frame loading in order with ~10 s spacing, at
  least one full loop plus the wrap to the first frame.
- Fallback behaviour stated from the code (and demonstrated if it can
  be done without disturbing the card: e.g. a boot with the frames
  directory renamed is NOT required — a clear code path + log line is
  enough; do not risk the staged card for it).
- Board left running the slideshow build; the reply names the branch
  and tip.

## Constraints

- **The Recalbox card is never touched** — it is not in the holder and
  stays that way; the card in the holder is the 0241 SanDisk. If the
  identity check ever reads the SR512 or the kit card, stop and stage
  the reply saying so.
- Clock speed, CS handling, pull-ups unchanged; pre-sacrifice fold as
  proven in 0239.
- Do not overwrite or delete /NEWCARD.BIN.
- Push the branch via SSH (gh.py writes to cabin-play return 403). No
  PR needed; a pushed branch + the staged reply is the deliverable.
- Cost: DeepSeek off-peak rates; one bench session plus downloads.

## Proof

- Branch on pagosacabin/cabin-play at the pushed tip (Juno checks the
  SHA against origin).
- The committed report log showing the frame-load sequence and timing.
- The ten image sources listed in the reply.

## Reply format

Lead with the outcome in one sentence: the slideshow is running through
ten card images at ~10 s apiece, or it stopped at stage X with error Y.
Then: the ten image sources in brief, the card directory listing, the
frame-load log summary (order + spacing), the fallback behaviour, the
Copilot pass count and verdict, and the board's end state.
