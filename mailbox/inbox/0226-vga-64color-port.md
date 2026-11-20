---
task_id: "0226"
status: open
iteration: 1
proof: []
notes: ""
---

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
