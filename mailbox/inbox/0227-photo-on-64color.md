---
task_id: "0227"
status: open
iteration: 1
proof: []
notes: ""
---

expect-reply-within: 6h

# 0227 — A real photograph on the 64-colour build (living frame, N16R8)

## Assignment

Hermes does this task. Firmware stays Hermes's (protocol 1.3.0). Copilot peer-reviews the committed code (firmware and the conversion tool), routed by Hermes via Herdr like Newton; Copilot is a reviewer, not the proof — Juno verifies independently on top.

## Context

- 0226 is verified: the wired N16R8 board runs 0180's LCD_CAM path at native 640x480, 8 bpp RGB222, framebuffer 307,200 B in PSRAM, measured 59.52 Hz, no doubling anywhere. Stephen has seen the test card on the panel.
- Stephen's direction (2026-11-20): the next step is an actual photograph on it — "photo quality." A test card proves the path; a photograph is what the gift actually displays.
- Use the SAME source photograph as the earlier picture test (the 0179/0180-era test image, in the repo's living-frame area) so Stephen can compare this rendering against the ones he has already judged. If that image is not in the repo, stop and report — do not substitute a different picture; Stephen will supply it.
- The SD card still does not answer (0225) and the FTDI console is still dead (0225/0226) — neither blocks this task: the converted image ships inside the firmware/flash, and the flash read-back path is the established proof channel.
- Agreed pin map, timings, orientation convention (portrait canvas rotated 90 deg clockwise into the raster, monitor pivoted counter-clockwise), and the Heltec's 0180 build all stay as they are.

## Task

Put the photograph on the panel, full-screen, at the build's native best:

1. Convert the source photo on the laptop to the panel's format: 480x640 portrait canvas (the raster stays 640x480 native, 1:1 — no doubling, no upscale of a smaller source), 64-colour RGB222.
2. Photo quality at 6-bit colour lives or dies on dithering: use Floyd–Steinberg (or a comparable error-diffusion) dither for the conversion, and commit the conversion tool/script with the firmware so the pipeline is reproducible.
3. Embed the converted image in the firmware/flash (307,200 B raw framebuffer fits the 16 MB flash) and display it full-screen on boot, held on screen.
4. Report: source image identity (filename, dimensions), the conversion settings used, and the firmware's own numbers (framebuffer size/location, refresh) via the flash read-back path.
5. Leave the board running the photograph when done — Stephen judges it with his own eyes. His verdict is the acceptance bar.

## Success criteria

- The panel displays the photograph full-screen at native 640x480, 8 bpp, framebuffer in PSRAM — the same measured path as 0226.
- The conversion is dithered (error diffusion), and the tool that produced the image data is committed alongside the firmware.
- Reply names the source photo used and its repo path; if the earlier test photo could not be found in the repo, the reply says so and nothing was substituted.

## Constraints

- The one wired board only. Do not reflash the Heltec's 0180 build.
- Do not depend on the SD card.
- Do not change the pin map, timings, or orientation convention.
- One deliverable: the photograph on the panel. No slideshow, no image rotation, no UI — those are later rungs if Stephen asks.
- No new purchases.

## Proof

- Firmware + conversion tool committed to the cabin-play repo, living-frame path — branch SHA on origin. Push via SSH (gh.py writes to cabin-play return 403).
- The firmware's reported numbers in the reply via the flash read-back report, as in 0225/0226.
- Copilot peer-review step on the committed code (GitHub-bound work); if the pane has signed out, say so in the reply rather than skipping silently.

## Reply format

Standard mailbox reply format: what was done, the source photo and conversion used, the reported numbers, the repo pointer (branch SHA), and what blocks the next rung, if anything.
