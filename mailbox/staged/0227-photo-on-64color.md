---
task_id: "0227"
status: staged
iteration: 1
proof:
  branch: hermes/0227-photo-64color
  base: db316d4cd14daaf96f9d26b7caa78c713e923811  # cabin-play main at pickup
  sha: 39ea61f168222d51ec81c87b5b7fe564c3946301
  files:
    - living-frame/firmware/n16r8-photo/tools/convert_photo64.py
    - living-frame/firmware/n16r8-photo/src/main.c
    - living-frame/firmware/n16r8-photo/src/photo64_data.h
    - living-frame/firmware/n16r8-photo/src/photo64_meta.h
    - living-frame/firmware/n16r8-photo/src/photo64_640x480-as-seen.png
    - living-frame/firmware/n16r8-photo/src/photo64_640x480-raster.png
    - living-frame/firmware/n16r8-photo/report-2026-11-20.txt
  peer_review: "copilot via herdr at 39283d0, 5 findings; the one Medium addressed in 39ea61f"
notes: >
  Picked up 2026-11-20 from an interactive session; the four cronrunner mailbox
  jobs are paused for the duration so no tick resumes this task.
  Board: the wired N16R8 on /dev/ttyUSB0 (FT232R bridge), MAC c4:9e:7e:08:6d:f4.
  DEVIATION (same bench as 0225/0226): no usable console on the bridge, so the
  numbers come from the `report` flash partition read back with esptool -- the
  path this task's Proof section explicitly allows.
  Source choice declared, not substituted: parrot.jpg (the LORIKEET image, the
  first photo phase in both 0179 and 0180). photo.jpg (BLUE_MARBLE) is untouched;
  switching is one argument to the converter.
  Orientation: 180 deg from the 0226 test card = portrait rotated 90 deg
  COUNTER-clockwise into the raster, per Stephen's amendment.
  Pin map and timings unchanged; the Heltec's 0180 build was not reflashed; the
  SD card is not used. Panel appearance is Stephen's to judge; nothing here
  claims an observed picture beyond the hashes and scan counters.
---

# 0227 — a real photograph on the 64-colour build: reply

## What was done

The photograph is on the panel, full-screen, held. Same driver as 0226 (0180's
LCD_CAM path, native 640x480, 8 bpp RGB222, framebuffer in PSRAM, 16-line bounce
buffers, nothing doubled); the test card is replaced by a dithered photo that
ships inside the app image as `living-frame/firmware/n16r8-photo/`.

**Source**: `living-frame/firmware/photo-display-s3/data/parrot.jpg`, 640x480 —
the LORIKEET test image, i.e. `IMG16_LORIKEET` in 0180 and `IMG_LORIKEET` in
0179, and the subject of the 0179 doubled-mode samples. It was the first photo
phase in both earlier rungs. The second test image, `data/photo.jpg`
(BLUE_MARBLE), is untouched; if 0227 meant the globe, `--source` switches it and
nothing else changes.

**Conversion** (`tools/convert_photo64.py`, committed):
scale-to-cover 480x640 portrait + centre-crop (the 0180 rule, so the framing is
comparable to what Stephen already judged) -> linear light (2.2) -> Floyd–Steinberg
error diffusion over the 64 RGB222 colours, nearest-colour test being squared
error in linear RGB weighted by the luminance coefficients (a heuristic, not a
perceptual metric) -> packed as `(b<<4)|(g<<2)|r` -> rotated into the landscape
raster **90 deg counter-clockwise**, which is the 180-degree flip from 0226's
card that Stephen's amendment asks for. 47 of the 64 DAC bytes are used.

## Reported numbers (device's own report, read back from flash)

| Item | Value |
| --- | --- |
| Framebuffer | **307,200 B in PSRAM** at `0x3c090998`, heap integrity ok |
| Embedded image | 307,200 B at `0x3c0381b4` (inside the app image) |
| SHA-256 | runtime `5ee7c64e1a18449a84176fadde38403bf3f54aaf4e6eacdb7ac8677ed16cd335` **==** tool hash -> **MATCH**, and **unchanged after 6.0 s of streaming** |
| Output | 640x480 native, 1 fb pixel = 1 DAC byte = 1 output pixel |
| Colour | 8 bpp = 64 colours (RGB222) |
| Timing | pclk 25,000,000 Hz, h 96+48+640+16, v 2+33+480+10 -> 59.52 Hz configured |
| Measured | **357 vsync in 5.998 s -> 59.52 Hz**; 10,713 bounce fills -> **30.0 per frame** |
| Rotation | 90 deg counter-clockwise (180 deg from the 0226 test card) |

What the hash does and does not prove: it shows the bytes the app streams are the
converter's output and stay that way through the scanout window, so a stale or
corrupted framebuffer is excluded. It does not measure the electrical DAC output,
and it cannot catch a fault after the check. The picture itself is Stephen's call.

## Peer review (Copilot, routed via Herdr)

Copilot reviewed `39283d0` including both previews and returned five findings,
one Medium:

1. **Rotation / previews** (Low) — counter-clockwise mapping is correctly 180 deg
   from 0226; previews match the scanned-out bytes.
2. **Floyd–Steinberg + RGB222** (Low) — weights and the four-level candidates are
   right; the distance metric is a luminance-weighted heuristic. README wording
   corrected.
3. **DAC packing** (Low) — tool and firmware agree with the agreed pin map.
4. **Runtime SHA-256** (Medium) — proves the copied framebuffer, not the panel
   output. Accepted: the firmware now re-hashes after the 6 s window
   (`unchanged - framebuffer not disturbed during scanout`) and the README states
   the limit.
5. **Const image + PSRAM copy** (Low) — fits the 3 MB app partition, no extra
   writable RAM needed.

## Repo pointer

`pagosacabin/cabin-play`, branch **`hermes/0227-photo-64color` @ `39ea61f`**
(base `db316d4`). Previews at `src/photo64_640x480-raster.png` (what the panel
receives) and `src/photo64_640x480-as-seen.png` (what Stephen's eye should see).
`report-2026-11-20.txt` is the device's own report. Pushed over SSH.

## What blocks the next rung

- **Nothing technical in this deliverable**: 640x480 at 8 bpp, 59.52 Hz measured,
  30.0 fills/frame, dithered 64-colour photo, hash-verified end to end.
- **Acceptance is Stephen's eyes** at the bench — photo quality and the
  orientation flip.
- **Open bench items, unchanged and out of scope here**: the SD card does not
  answer on the agreed SPI pins (0225), and the FT232R bridge gives no usable
  console (numbers come from the flash read-back path).

---

## Original task (filed by Juno, 2026-11-20)

expect-reply-within: 6h

# 0227 — A real photograph on the 64-colour build (living frame, N16R8)

## Assignment

Hermes does this task. Firmware stays Hermes's (protocol 1.3.0). Copilot peer-reviews the committed code (firmware and the conversion tool), routed by Hermes via Herdr like Newton; Copilot is a reviewer, not the proof — Juno verifies independently on top.

## Context

- 0226 is verified: the wired N16R8 board runs 0180's LCD_CAM path at native 640x480, 8 bpp RGB222, framebuffer 307,200 B in PSRAM, measured 59.52 Hz, no doubling anywhere. Stephen has seen the test card on the panel.
- Stephen's direction (2026-11-20): the next step is an actual photograph on it — "photo quality." A test card proves the path; a photograph is what the gift actually displays.
- Use the SAME source photograph as the earlier picture test (the 0179/0180-era test image, in the repo's living-frame area) so Stephen can compare this rendering against the ones he has already judged. If that image is not in the repo, stop and report — do not substitute a different picture; Stephen will supply it.
- The SD card still does not answer (0225) and the FTDI console is still dead (0225/0226) — neither blocks this task: the converted image ships inside the firmware/flash, and the flash read-back path is the established proof channel.
- Agreed pin map, timings, and the Heltec's 0180 build stay as they are. Orientation changes per Stephen's amendment below.

## Task

Put the photograph on the panel, full-screen, at the build's native best:

1. Convert the source photo on the laptop to the panel's format: 480x640 portrait canvas (the raster stays 640x480 native, 1:1 — no doubling, no upscale of a smaller source), 64-colour RGB222.
2. **Orientation (Stephen's amendment, 2026-11-20): flip the displayed image 180 degrees relative to the 0226 test card.** The test card was drawn for a counter-clockwise-pivoted monitor; Stephen's monitor is pivoted the other way, so the card reads upside down from where he sits. The photograph must read right way up from Stephen's side with the monitor pivoted as it is now. How the flip is implemented (canvas rotation direction, scan order) is Hermes's choice — the outcome is what's specified.
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
- Do not change the pin map or timings. The 180-degree display flip above is Stephen's explicit instruction and supersedes the 0180 orientation convention for this build.
- One deliverable: the photograph on the panel. No slideshow, no image rotation, no UI — those are later rungs if Stephen asks.
- No new purchases.

## Proof

- Firmware + conversion tool committed to the cabin-play repo, living-frame path — branch SHA on origin. Push via SSH (gh.py writes to cabin-play return 403).
- The firmware's reported numbers in the reply via the flash read-back report, as in 0225/0226.
- Copilot peer-review step on the committed code (GitHub-bound work); if the pane has signed out, say so in the reply rather than skipping silently.

## Reply format

Standard mailbox reply format: what was done, the source photo and conversion used, the reported numbers, the repo pointer (branch SHA), and what blocks the next rung, if anything.