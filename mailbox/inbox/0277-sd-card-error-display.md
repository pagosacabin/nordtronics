---
task_id: "0277"
status: filed
iteration: 1
proof: []
notes: ""
expect-reply-within: 24h
---

# 0277 — Living frame: SD card error display

## Assignment
- Hermes: build and bench-verify (firmware, cabin-play).
- Newton: software support if needed, via Hermes/Herdr.
- Copilot: peer review (open lane — frame display).

## Context
The living frame (YD-ESP32-S3 N16R8, VGA DAC, SD card photo library) boots to the Nordtronics splash, then loads the first photo from the SD card. If the SD card cannot be contacted (not seated, bad contact, corrupted, missing), the frame hangs silently on the splash screen with no indication of what is wrong. This happened Nov 27, 2026: after the monitor was reassembled into its wooden case the frame stuck on splash; the cause turned out to be USB power (wrong port), not the SD card — but the silent hang made diagnosis a guess. The frame must report an SD failure on screen instead of hanging silently.

## Task
Add SD-card error reporting to the frame firmware (cabin-play, frame firmware):
1. On boot, after the splash, attempt SD card init. If init fails (card not found / cannot be contacted / cannot be read), display a clear error on the screen instead of hanging on the splash. Suggested text: "SD card not found" (or "Cannot read SD card"), large enough to read at a glance, in the frame's existing text style.
2. The error must persist on screen (not flash past) until the card is readable or the frame is power-cycled. Do not loop or reboot on SD failure.
3. Do not change the splash, the photo library, the slideshow, the keep/discard flow, or any other behavior. Only the SD-failure path changes.
4. If the SD card becomes readable on a later boot, the frame must boot normally (splash to first photo). No state change needed.

## Success criteria
- With no SD card (or an unreadable card) inserted, the frame boots to splash, then shows the SD error message on screen and stays there (no hang on splash, no reboot loop).
- With a good SD card inserted, the frame boots normally (splash to first photo) exactly as before.
- The error text is legible on the 640x480 portrait display.
- Branch pushed to cabin-play; bench log / photo of the error screen as proof.

## Constraints
- Firmware only; no hardware changes.
- Do not touch the splash image, the photo library, the slideshow, keep/discard, or the camera code.
- The 0261 mirror fix (set_hmirror 0) must remain in place — do not regress it.
- Open lane (frame display): generic Copilot peer review, not the locked-lane security checklist.
- Do not change the SD SPI pins (SCK 12 / MOSI 11 / MISO 13 / CS 10) or the SD library.
- Cost: no model/provider cost in this task (firmware only).

## Proof
- Branch name + SHA on origin (cabin-play).
- Bench log or photo showing the error screen with no SD card inserted.
- Bench log or photo showing normal boot with a good SD card.
- Copilot review verdict (open lane).

## Reply format
- Status (DONE / BLOCKED), branch + SHA, what was changed (file + lines), bench proof (error screen + normal boot), Copilot verdict, and any deviation.
