---
task_id: "0228"
status: staged
iteration: 1
proof:
  branch: hermes/0227-photo-64color
  base: 39ea61f168222d51ec81c87b5b7fe564c3946301  # 0227's staged tip
  sha: 866af7f0754dc5ff453a31a55327f8a984c7da86
  files:
    - living-frame/firmware/n16r8-photo/tools/convert_photo64.py
    - living-frame/firmware/n16r8-photo/tools/shadow_coverage.py
    - living-frame/firmware/n16r8-photo/src/main.c
    - living-frame/firmware/n16r8-photo/src/photo64_640x480-gamma-as-seen.png
    - living-frame/firmware/n16r8-photo/src/photo64_640x480-gamma-raster.png
    - living-frame/firmware/n16r8-photo/data/hummingbird.jpg
    - living-frame/firmware/n16r8-photo/data/ATTRIBUTION.md
    - living-frame/firmware/n16r8-photo/report-gamma-2026-11-20.txt
    - living-frame/firmware/n16r8-photo/report-800x600.txt
  peer_review: "copilot via herdr at 87c4e40, 4 findings (2 Medium); both addressed"
notes: >
  Picked up 2026-11-20 from an interactive session; the four cronrunner mailbox
  jobs were paused for the duration and are resumed after this staging.
  DEVIATION (same bench as 0225-0227): no usable console on the FT232R bridge, so
  all numbers come from the `report` flash partition read back with esptool.
  ONE VARIABLE, verified rather than asserted: regenerating with --space linear
  reproduces 0227's framebuffer exactly (sha256 5ee7c64e...16cd335), so crop,
  orientation, rotation, packing and the source are provably unchanged.
  Image swap: at Stephen's request the source moved from the lorikeet
  (photo-display-s3/data/parrot.jpg) to a Wikimedia Commons hummingbird
  (CC BY 3.0, Charles J. Sharp) because the lorikeet judged badly. That is a
  second change, made by the user mid-task, and it is declared here rather than
  folded into the one-variable claim: the coverage table reports both images.
  Resolution work was added after Stephen's verdict that the gamma render was
  "still pixelated"; it is build-time only (envs), the shipped 0227/0228 raster
  remains 640x480. Pin map, timings of 640x480, the Heltec's 0180 build and the
  SD card are untouched.
---

# 0228 — gamma-space dither test: reply

## What was done

`tools/convert_photo64.py` gained `--space {linear,gamma}`: the Floyd–Steinberg error
is now measured and carried in **gamma (sRGB code) space** instead of linear light.
Selected levels, the 64 RGB222 candidates, crop, rotation, packing and the source
are untouched. Default is gamma; `--space linear` reproduces 0227 byte-for-byte
(sha256 `5ee7c64e…16cd335`), which is the check that only the dithering space moved.

`tools/shadow_coverage.py` measures a defined dark region — the darkest quartile of
source-luminance pixels — and reports coverage, mean DAC byte, truly isolated dots
(0 non-black neighbours), sparse dots (≤1) and mean neighbour count.

## Shadow-coverage numbers (linear vs gamma), lorikeet — 0227's image

Darkest quartile = 76,816 of 307,200 canvas px, luminance threshold 0.0073:

| Rendering | cover% | mean DAC byte | alone% | sparse% | mean non-black nbrs |
| --- | --- | --- | --- | --- | --- |
| linear (0227) | **1.47** | 0.06 | 85.0 | 98.9 | 0.16 |
| gamma (0228) | **21.36** | 1.28 | 15.2 | 49.8 | 1.56 |

14.5x more dark-region coverage, and truly isolated specks drop from 85% to 15% —
the starfield signature the photographs showed.

## The image swap, and its numbers

Stephen judged the lorikeet rendering badly and asked for a better photograph, so
the source became `data/hummingbird.jpg` (*Antillean crested hummingbird feeding*,
Charles J. Sharp, CC BY 3.0, Wikimedia Commons; attribution in
`data/ATTRIBUTION.md`). It is a bright subject on warm bokeh with no large black
fields. Same image, both spaces:

Darkest quartile threshold 0.1642 — note it: this photo has no deep blacks, so its
"dark" quartile is defocused background:

| Rendering | cover% | mean DAC byte | alone% | sparse% | mean non-black nbrs |
| --- | --- | --- | --- | --- | --- |
| linear | **49.40** | 2.51 | 0.8 | 2.9 | 5.29 |
| gamma | **71.79** | 6.23 | 0.3 | 0.6 | 6.98 |

Stated plainly: **the picture choice moved this metric further than the dither
space did** (49.4% baseline vs 1.5%), because the starfield came from deep-black
regions that this image does not have.

## Stephen's verdict, and what it means

On the gamma-dithered hummingbird at 640x480, Stephen's verdict was **"still
pixelated"**. So the diagnosis in the task was partly right — gamma space does what
it predicted to the pixel statistics — but it is not the cause of what he is seeing.
The remaining blockiness is the display path: 640x480 onto a 22" 1366x768 TV is a
2.13x horizontal and 1.6x vertical **non-integer** upscale, so source pixels become
uneven 2- and 3-pixel blocks. No dither can fix that.

Consequently the port now carries build-time raster modes, same driver and pins:
`n16r8-photo-800x600` (measured **60.18 Hz, 40.1 fills/frame**, framebuffer
480,000 B) and `n16r8-photo-1024x768` (65 MHz pclk, 786,432 B). One real constraint
found and now a compile-time guard: the RGB driver rejects a mode whose frame size
is not an exact multiple of the bounce buffer, so 800x600 needs 15-line buffers
(600 does not divide by 16). The shipped 0227/0228 raster is still 640x480.

## Firmware numbers (flash read-back)

640x480 gamma, hummingbird: framebuffer 307,200 B in PSRAM at `0x3c090998`, heap
integrity ok, runtime sha256 `5454395c…9196f` == tool hash (**MATCH**), unchanged
after 6 s of streaming, 59.52 Hz measured, 30.0 fills/frame.
800x600 gamma, same photo: 480,000 B framebuffer, sha256 `204578cc…3bd306` **MATCH**,
60.18 Hz measured, 40.1 fills/frame.

## Peer review (Copilot)

Reviewed `87c4e40`: gamma branch changes the working space only (Low), `--space
linear` reproduces 0227 exactly (Low, reproduced independently), mask mapping
correct (Low) — but the "isolated" label was broader than its rule (Medium) and
coverage is a proxy metric, not proof of better shadows (Medium). Both addressed:
the tool now reports **alone** (0 neighbours) and **sparse** (≤1) separately, and the
README states the metric's limits.

## Repo pointer

`pagosacabin/cabin-play`, branch **`hermes/0227-photo-64color` @ `866af7f`**
(base `39ea61f` = 0227's staged tip, so the diff is 0228 plus the mode support).
Previews: `src/photo64_640x480-gamma-as-seen.png` and `-raster.png`. Reports:
`report-gamma-2026-11-20.txt` (640x480) and `report-800x600.txt`.

## What blocks the next rung

- **Stephen's eyes** on the higher modes — that is the live question now, not dither.
- **SD still fails, with better evidence than before**: at 400 kHz/mode 0 and
  100 kHz/mode 3 the raw CMD0/CMD8/CMD55/ACMD41 replies are byte-identical
  (`CMD8 → ff 01 f0 00 01` in both), which is not card behaviour — MISO is held
  high externally and nothing is driving it. That points at the card or holder
  contacts/power, not the init stack.

---

## Original task (filed by Juno, 2026-11-20)

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