---
task_id: "0228"
status: open
iteration: 1
proof: []
notes: ""
---

expect-reply-within: 4h

# 0228 — Gamma-space dither test (living frame photo, one variable)

## Assignment

Hermes does this task. Firmware/converter work stays Hermes's (protocol 1.3.0). Copilot peer-reviews the committed change, routed by Hermes via Herdr like Newton; Copilot is a reviewer, not the proof — Juno verifies independently on top.

## Context

- 0227 put the lorikeet photo (data/parrot.jpg) on the 64-colour build: native 640x480, 8 bpp RGB222, framebuffer in PSRAM, measured 59.52 Hz, framebuffer hash matched the converter exactly. The pipe is proven honest.
- Stephen's verdict on 0227: "It's still pixelated." His close-up photos of the panel show why: in dark areas the colour is built from rare isolated single pixels — a starfield — because at 2 bits/channel the first step up from black is 1/3 of full scale, and the 0227 converter dithered in LINEAR light, which makes shadow steps perceptually even coarser and starves the shadows of dots.
- This task changes ONE variable to test that diagnosis: **the same photo, the same pipeline, but Floyd–Steinberg dithering performed in gamma (sRGB) space instead of linear light.** Gamma-space dithering distributes dots the way the eye sees brightness, so shadows should get denser, more even coverage.
- Everything else stays exactly as in 0227: same source photo, same 480x640 portrait cover-crop, same 180-degree-flipped orientation (reads upright from Stephen's side), same board, pin map, timings, and framebuffer path. The Heltec's 0180 build is not touched. SD is not used.

## Task

1. Take the 0227 converter (living-frame/firmware/n16r8-photo/tools/convert_photo64.py) and switch its dithering working space from linear light to gamma (sRGB) space. Nothing else in the conversion changes.
2. Convert parrot.jpg with the modified converter, build, and flash the wired N16R8 board. Leave the board displaying the photograph.
3. Commit the updated converter and the new as-seen preview PNG alongside the 0227 outputs, so the two renderings can be compared directly.
4. Report a number with the pictures: for a defined dark region of the image (e.g. the darkest quartile of pixels by source luminance), the percentage of non-black output pixels in the 0227 (linear) rendering vs this (gamma) rendering. The diagnosis predicts the gamma version's shadow coverage is markedly denser.
5. Report the firmware's own numbers via the flash read-back path, as in 0227, including the framebuffer hash match.

## Success criteria

- The panel displays the lorikeet photograph from the gamma-dithered conversion, same measured native path as 0226/0227 (640x480, 8 bpp, PSRAM framebuffer, hash match).
- The ONLY conversion change is the dithering working space; the reply states this explicitly.
- Shadow-coverage numbers for both renderings are reported (dark-region non-black pixel percentage, linear vs gamma).
- Stephen judges the panel with his own eyes — his verdict decides whether the diagnosis was right.

## Constraints

- The one wired board only. Do not reflash the Heltec's 0180 build.
- Do not change the source photo, crop, orientation, pin map, or timings.
- Do not stack other tweaks (no dither-strength changes, no palette changes, no sharpening) — one variable, or the test proves nothing.
- No new purchases.

## Proof

- Updated converter + firmware + previews committed to the cabin-play repo, living-frame path — branch SHA on origin. Push via SSH (gh.py writes to cabin-play return 403).
- The firmware's reported numbers in the reply via the flash read-back report, as in 0225–0227.
- Copilot peer-review step on the committed change (GitHub-bound work); if the pane has signed out, say so in the reply rather than skipping silently.

## Reply format

Standard mailbox reply format: what was done, the explicit statement that the dithering space is the only change, the shadow-coverage numbers (linear vs gamma), the reported firmware numbers, the repo pointer (branch SHA), and what blocks the next rung, if anything.
