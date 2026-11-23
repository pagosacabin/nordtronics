---
task_id: "0244"
title: Button photo flow — countdown on the frame, library on the frame board's SD, land the camera work in cabin-play
status: pending
iteration: 1
expect-reply-within: 6h
proof: []
notes: Filed by Juno 2026-11-23 at Stephen's direction. Supersedes the camera-side library in the interactive cam-button build.
---

## Assignment

- **Hermes:** all bench work, both firmwares, the repo landing. Push cabin-play via SSH from the laptop (the gh.py credential is 403 on cabin-play writes).
- **Copilot:** peer review of the cabin-play changes before the push, driven by Hermes via Herdr (GitHub-bound work rule). Reviewer, not the proof.
- **Newton:** not assigned — firmware stays with Hermes.
- **Juno:** verifies independently on the staged reply — branch SHA on origin, repo paths, no secrets in the commit, bench proof.

## Context

This morning's interactive session (Stephen at the laptop) proved the whole camera chain on the Freenove ESP32-WROVER CAM board (ESP32-D0WD-V3, GC0308 sensor, MAC f0:16:1d:ae:2b:20, WROVER_KIT pin map in board_config.h): capture works, the ESP-NOW chunked link works (1,280 packets, ~5.5–6 s per frame, no failed packets), and the camera board now runs the full colour treatment — grey-world white balance at strength 0.5 smoothed across frames, the 256-entry gamma LUT, 4×4 ordered Bayer dither. The cam-button build (env cam-button) verified the button path over serial with the button on GPIO13 of the CAMERA board and the photo library held on the camera side (6-frame PSRAM ring).

Stephen's corrections and additions, which this task implements:

1. **The library lives on the frame (video) board's SD card** — not the camera side. Press → capture → process → send → the frame board writes the frame to its own SD → the frame board cycles the images on its card.
2. **The button moves to the frame board.** Pressing it shows a **3-2-1 countdown on the panel** (so people can get ready and smile), then the frame board signals the camera to take the shot, and the picture comes back. Stephen's words: keep it simple, don't convolute it.
3. **The work lands in the repo.** Everything so far lives in scratch (~/.hermes/cache/scratch/cam-bringup, frame-freenove). It goes into pagosacabin/cabin-play under living-frame/firmware/.

The frame board is the YD-ESP32-S3 N16R8 (VGA DAC + SD over SPI, SCK 12 / MOSI 11 / MISO 13 / CS 10; GPIO 14 is the recorded spare). Its SD stack is proven (0241 write chain, 0243 slideshow). The live build runs 640×480 native like the photo build (n16r8-photo); camera frames are 640×480, 307,200 B packed.

## Task

1. **Land the code in cabin-play**, branch `hermes/0244-button-photo-flow`:
   - `living-frame/firmware/freenove-cam/` — the camera firmware (from cam-bringup, reworked per step 2).
   - `living-frame/firmware/n16r8-live/` — the frame firmware: ESP-NOW receiver + SD library + slideshow cycle + button + countdown.
   - Any unpushed living-frame work from this morning (n16r8-photo, the convert_photo64.py preview-LUT fix, commit 6e229d5) lands on the same branch.
   - `secrets.h` and any WiFi credentials stay OUT of the repo — gitignore them. The repo is public.
2. **Rework the flow** (this replaces the cam-button behaviour):
   - Button wired to the **frame board** — GPIO 14 unless the build's pin map says otherwise; confirm the pin against the pin map before wiring. Internal pullup, button to GND, active-low, software debounce. (Stephen's orange panel-mount button mounts on the physical frame.)
   - Press → the panel shows a clear **3-2-1 countdown, ~1 s per step**, simple rendering (block digits or number cards — no font project) → the frame board sends a SHOOT trigger to the camera over ESP-NOW → the camera captures a settled frame (discard warm-up frames if the sensor was idle — first frames after wake are badly exposed), runs the existing WB 0.5 + gamma LUT + Bayer dither chain → sends the packed frame back over the existing chunked transfer → the frame board **writes it to its SD card as the next file in the live library**, shows the new photo, then resumes cycling the whole card library at the existing ~10 s dwell.
   - The camera keeps **no library**: no PSRAM ring retention, no camera-side SD storage.
   - Presses during a countdown or a transfer are ignored.
   - Keep the live library separate from the 0243 slideshow files: the card's /sdcard/FRAMES holds 800×600 rasters from 0243, a different size from this build's 640×480. Use a separate directory for the live library and leave the 0243 frames and /NEWCARD.BIN untouched.
3. **Copilot peer review** of the repo changes; address findings; then push.

## Success criteria

- Physical press → countdown visible on the panel → the new photo is on the panel within ~15 s of the press, and the live library on the frame board's SD has grown by exactly one file (307,200 B); the stored file's sha256 equals the sent frame's sha256 (log both).
- Frame-board reboot → the library persists and cycling resumes over all its files, including ones added before the reboot.
- Camera-side storage is gone: the camera log shows no ring stores and no camera SD writes.
- Repo: branch `hermes/0244-button-photo-flow` on pagosacabin/cabin-play carries both projects at the paths above; `git ls-files` on the branch shows no secrets.h and no credentials.
- Simplicity guard: if the countdown fights the core flow, stage the core working without it and say so plainly in the reply. Do not ship a broken hybrid.

## Constraints

- Start small, proven rungs: the chunked transfer, the colour chain, and the frame SD write path are all proven — do not redesign them. The new pieces are only: button on the frame board, the countdown, the trigger message, and the receive-to-SD write.
- Pin discipline: camera pin map unchanged (board_config.h stays the single source of truth — delete any duplicate pin selection in the .ino if one survives from the kit sketch). Frame button pin confirmed against the pin map before wiring.
- Secrets never in the task file, the repo, or the staged reply.
- Model cost: flash tier is fine for this work.

## Proof

- Branch name + commit SHA on origin for pagosacabin/cabin-play.
- Log excerpts: press event, countdown steps, trigger sent, camera capture (WB values), packets ok/fail, both sha256s (sent vs stored), SD listing of the live library before/after the press (count + new file size), and the post-reboot listing.
- Photos of the panel: one countdown number, and the resulting photo on screen. Stephen's eyes are the visual proof.

## Reply format

Standard staged reply: status line, what changed, proof pointers as above, deviations declared (including Copilot's findings and their disposition).
