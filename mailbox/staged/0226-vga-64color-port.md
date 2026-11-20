---
task_id: "0226"
status: staged
iteration: 1
proof:
  branch: hermes/0226-vga-64color
  base: db316d4cd14daaf96f9d26b7caa78c713e923811  # cabin-play main at pickup
  sha: 8e756a1c569ae003195ae325eaef6f9b61aeb5e7
  files:
    - living-frame/firmware/n16r8-vga64/src/main.c
    - living-frame/firmware/n16r8-vga64/src/font5x7.h
    - living-frame/firmware/n16r8-vga64/partitions.csv
    - living-frame/firmware/n16r8-vga64/sdkconfig.defaults
    - living-frame/firmware/n16r8-vga64/platformio.ini
    - living-frame/firmware/n16r8-vga64/report-2026-11-20.txt
  peer_review: "copilot via herdr at f7600a8, 5 findings; 2 accepted, addressed in 8e756a1"
notes: >
  Picked up 2026-11-20 from an interactive session; the four cronrunner mailbox
  jobs are paused for the duration so no tick resumes this task.
  Board: the wired N16R8 on /dev/ttyUSB0 (FTDI FT232R bridge), MAC
  c4:9e:7e:08:6d:f4. DEVIATION (carried from 0225, same bench): the bridge
  delivers no usable console, so the port's numbers are also written to a
  `report` flash partition (data subtype 0x40 at 0x310000) and read back with
  `esptool read_flash`. The task's Proof section explicitly allows the flash
  read-back path.
  Not changed: agreed pin map, 0180's timings, orientation convention, and the
  Heltec's 0180 build (not reflashed). SD not used anywhere in this port.
  Panel appearance is Stephen's to judge; nothing in this reply claims an
  observed picture or an observed colour swap.
---

# 0226 — 64-colour port: reply

## What was done

0180's LCD_CAM RGB driver path was moved to the wired N16R8 board as
`living-frame/firmware/n16r8-vga64/` (ESP-IDF 5.5, PlatformIO 6.1.19). The
framebuffer is **8 bpp in PSRAM**; the only structural change from 0180 is that
the bounce-empty ISR now `memcpy`s a byte-per-pixel framebuffer straight out of
PSRAM instead of expanding 4-bit indices out of internal RAM.

Nothing is doubled: one framebuffer pixel is one DAC byte is one output pixel,
640x480 native. There is no 320x240 source, no line skip, no upscale path.

The board is left running the test card:

- separate **R, G, B ramps**, 4 steps each, each labelled by channel
- **64-colour grid**, cell `(r,g,b)` at index `r + 4g + 16b`, DAC byte `(b<<4)|(g<<2)|r`
- **1-pixel checkerboard**, `0x00`/`0x3f` alternating, 120x120 canvas pixels
- **native 1x text** (1 fb pixel = 1 output pixel)
- drawn on a 480x640 portrait canvas and rotated 90 deg clockwise into the
  landscape raster, the 0180 convention (monitor pivoted counter-clockwise)

## Reported numbers (firmware's own output, read back from the device)

| Item | Value |
| --- | --- |
| Resolution | **640x480 native** (1 fb pixel = 1 DAC byte = 1 output pixel) |
| Colour | **8 bpp = 64 colours**, RGB222 through the 6-bit DAC |
| Framebuffer | **307,200 B in PSRAM** at `0x3c030998`, `heap_caps_check_integrity` ok |
| Pixel clock | **25,000,000 Hz**, h 96+48+640+16, v 2+33+480+10 → **59.52 Hz configured** |
| Measured refresh | **357 vsync in 5.998 s → 59.52 Hz** |
| Bounce fills | **10,712 in 5.998 s → 30.0 per frame** (expected 480 lines / 16 = 30.0) |
| Sync | hsync GPIO17, vsync GPIO18, both active-low |
| Data pins | bit0 R_LSB/5, bit1 R_MSB/4, bit2 G_LSB/7, bit3 G_MSB/6, bit4 B_LSB/16, bit5 B_MSB/15 |

The measured figures come from the driver's own vsync and bounce-empty callbacks,
not from the configuration, so they are independent evidence that LCD_CAM is
scanning 30 bounce buffers per frame while reading the framebuffer out of PSRAM.

## Colour swap observed

**None observed — and not claimed.** I have not seen the panel; that judgement is
Stephen's. What the firmware does instead is log the expected DAC bytes so the
screen can be compared numerically:

| Element | Expected DAC bytes |
| --- | --- |
| Red ramp | `0x00 0x01 0x02 0x03` |
| Green ramp | `0x00 0x04 0x08 0x0c` |
| Blue ramp | `0x00 0x10 0x20 0x30` |
| Checker | `0x00` / `0x3f` |

A channel swap shows as the wrong bar lighting; an MSB/LSB swap inside a channel
shows as non-monotonic steps (e.g. red stepping `0x00 0x02 0x01 0x03`). Per the
constraint, any such swap is to be reported, not compensated in code.

## Peer review (Copilot, routed via Herdr)

Copilot reviewed `f7600a8` file by file (GitHub api reads) and returned five
findings, none blocking:

1. **1:1 raster path** (Low) — clean: `bounce_fill()` is a plain `memcpy` from the
   307,200 B framebuffer, no expansion or doubling.
2. **DAC packing / GPIO order** (Low) — matches the agreed map exactly.
3. **PSRAM read inside the bounce ISR** (Medium) — plausible but not formally
   proven safe; depends on the data cache being enabled and `memcpy` being
   ISR-safe for cached PSRAM. Accepted: the constraint and the expected failure
   symptom (bounce underrun / horizontal corruption, not a wrong size) are now in
   the firmware output and the README.
4. **Swap detection** (Medium) — the card catches direct pin swaps and gross
   misrouting but is a visual judgement, not a machine assertion. Accepted: the
   expected DAC bytes are now logged, and this reply says plainly that the panel
   is Stephen's to judge.
5. **Canvas→raster mapping** (Low) — correct 90 deg clockwise.

Items 3 and 4 landed in `8e756a1`.

## Repo pointer

`pagosacabin/cabin-play`, branch **`hermes/0226-vga-64color` @ `8e756a1`**
(base `db316d4`). `report-2026-11-20.txt` in that directory is the device's own
read-back report. Pushed over SSH (`gh.py` not used).

## What blocks the next rung

- **Nothing in the port itself.** 640x480 at 8 bpp runs at 59.52 Hz measured with
  30.0 fills/frame, framebuffer in PSRAM, no doubling anywhere.
- **Acceptance is Stephen's eyes**: not pixelated, correct channel order, at the
  bench. The expected DAC bytes above make a mismatch stateable.
- **Open bench items, unchanged**: the SD card does not answer on the agreed SPI
  pins (0225, out of scope here), and the FT232R bridge delivers no usable
  console (this reply's numbers come from the flash read-back path the task
  allows).

---

## Original task (filed by Juno, 2026-11-20)

expect-reply-within: 8h

# 0226 — 64-colour port at full 640x480 (living frame, N16R8)

## Assignment

Hermes does this task. Firmware stays Hermes's (protocol 1.3.0). Copilot peer-reviews the committed port, routed by Hermes via Herdr like Newton; Copilot is a reviewer, not the proof — Juno verifies independently on top.

## Context

- 0180 is the approved ship build: full native 640x480 at 16 colours, portrait-native, LCD_CAM driver path, on the Heltec. Stephen's verdict: "The last test was the best test" — resolution beats colour depth on this DAC + panel.
- This rung adds the 64 colours back at that same full native resolution. It does NOT re-decide resolution, orientation, or the palette pipeline.
- **Full resolution means native 640x480 — no pixel doubling, no 320x240 upscale.** 0179's doubled mode and 0177's FabGL build both read as pixelated to Stephen and were superseded for exactly that reason. A doubled or upscaled result is a failed rung, not a partial pass.
- 0225 (verified by Juno) unblocked the memory question on the board Stephen wired: ESP32-S3 rev v0.2, 16 MB physical flash, **8,388,608 B octal PSRAM, full-range pattern test PASS, 0 mismatches**. The 8 bpp framebuffer is 307,200 B and lives in PSRAM.
- Agreed pin map (Stephen wired to this Nov 20; 0225 verified all 12 pads are live GPIOs): VGA DAC red GPIO 4, 5; green GPIO 6, 7; blue GPIO 15, 16; HSYNC GPIO 17; VSYNC GPIO 18. SD on SPI (SCK 12, MOSI 11, MISO 13, CS 10) — **SD failed its probe in 0225 (card does not answer) and is OUT of scope here; do not make this port depend on the SD card.**
- Known caveat from 0225: the pin test proved the pads are working GPIOs, not that the silkscreen labels sit on the right header positions. A swapped label would appear as a wrong colour channel or bit order. The test display below is designed to make that diagnosable at a glance.

## Task

Port the 0180 build (LCD_CAM driver path) to the wired N16R8 board with the framebuffer in PSRAM at 8 bpp:

1. Native 640x480 output, portrait-native as in 0180, 64 colours (RGB222 through the 6-bit DAC as wired). No framebuffer doubling, no line skipping, no upscale path.
2. Framebuffer allocated in PSRAM; report its size and location from the firmware's own output.
3. On boot, display a diagnostic test card, held on screen until Stephen replaces it:
   - Separate red, green, and blue ramps, each labelled by channel, so a swapped channel or bit order is visible immediately.
   - A full-range 64-colour grid.
   - Fine detail that exposes pixelation: 1-pixel checkerboard region and small text rendered at native resolution.
4. Leave the board running the test card when done — Stephen judges the panel with his own eyes, as with 0180. His verdict is the acceptance bar for "not pixelated."
5. Report from the firmware/driver: actual output resolution and pixel clock as configured, framebuffer bytes, and refresh behaviour if the driver reports it.

## Success criteria

- Firmware reports native 640x480 at 8 bpp with the framebuffer in PSRAM — no doubling anywhere in the path.
- Test card displays and holds: channel-labelled R/G/B ramps, 64-colour grid, 1-pixel checkerboard, native-res small text.
- Any colour-channel or bit-order swap visible on the test card is reported as observed (which channels/bits), not silently compensated in code.
- Port committed in the repo (see Proof).

## Constraints

- Work on the one wired board only. Do not reflash the Heltec's 0180 build — it stays the fallback ship build.
- Do not depend on the SD card (0225: it does not answer; that is a separate bench item).
- Do not change the agreed pin map. If a swap is suspected, report it; rewiring or remapping is Stephen's call.
- No new purchases.

## Proof

- Port committed to the cabin-play repo, living-frame path (same area as 0179/0180 and the 0225 probe) — branch SHA on origin. Push via SSH as before (gh.py writes to cabin-play return 403).
- The firmware's own reported numbers in the reply (resolution, bpp, framebuffer size/location). Given the FTDI console fault found in 0225, the flash read-back report path is acceptable proof of the numbers; the panel itself is Stephen's to judge.
- Copilot peer-review step on the committed port (GitHub-bound work). The pane was signed in for 0225; if it has signed out again, say so in the reply rather than skipping silently.

## Reply format

Standard mailbox reply format: what was done, the measured/reported numbers (resolution, pixel clock, bpp, framebuffer size and location), any colour swap observed and which channels/bits, the repo pointer (branch SHA), and what blocks the next rung, if anything.