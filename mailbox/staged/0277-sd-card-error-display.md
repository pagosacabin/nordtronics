---
task_id: "0277"
status: staged
iteration: 1
claimed_by: "hermes (interactive, laptop)"
proof:
  kind: host-only
  branch: "hermes/0277-sd-error"
  tip_sha: "9bbdef8"
  base: "hermes/0261-mirror-live-photo @ b5c0f52; splash asset and code untouched"
  build: "pio run -e n16r8-photo SUCCESS; flashed to /dev/ttyUSB1, 'Hash of data verified'"
  no_card_boot: "SD mount failed: ESP_ERR_TIMEOUT (263) x6 -> no-SD library holds 0 frames, next index 0 -> 'splash : logo buffer released (SD error on the panel)' -> 'sd : no card at boot -- error on panel, retrying every 10 s'; the panel showed 'SD card not found / Check the card, then power cycle' and stayed there (Stephen, bench)"
  normal_boot: "card back in: 'SD mounted, SD32G psn=0xc6bd0f92, 30436 MB', 'SD library holds 37 frames, next index 56' and photos on the panel"
  copilot: "NOT AVAILABLE -- Copilot returned 'You have exceeded your monthly quota' when the review was sent; no verdict exists. Juno is running the review instead."
notes: |
  DONE AND BENCH-VERIFIED, with one missing proof: the Copilot peer review.

  WHAT CHANGED (only the failure path):
    src/live.c
      - include notice_sd.h, #define SD_RETRY_MS 10000
      - after the boot mount loop, if !s_mounted: show_notice(NOTICE_SD ...) with a
        24 h hold, splash_released("SD error on the panel"), and a log line saying so
      - the live loop retries the card every 10 s while !s_mounted and, on success,
        rescans, rebuilds the shuffle order, clears the notice hold and back-dates
        last_show so the slideshow starts at once
      - splash_released() takes a reason now (it used to claim "after the first
        photo" even when there was no photo)
      - s_mount_quiet rate-limits a failed retry to one log line a minute, so the
        boot evidence is not pushed out of the 64 KB report while the card is out
    src/notice_sd.h  (new, 480x140 RGB222, 67,200 B, portrait space like the others)

  BENCH: with the card out the frame boots splash -> error screen and holds it; six
  mount attempts fail with ESP_ERR_TIMEOUT before the message appears.  With the card
  back every boot returns to normal (mounted, 37 frames, photos on the panel).
  Both shown above from the report partition on the staged build.

  BEYOND THE TASK, declared: the retry loop means a card re-seated while the frame is
  running is picked up within 10 s without a power cycle.  The task only required the
  next boot to work; this is a small extra that the retry needed anyway.

  NOT DONE, stated rather than implied: an empty but readable card still leaves the
  splash up (the error is for "cannot contact the card", not "no photos").  If that
  matters it wants its own task.

  copilot: the review could not run -- monthly quota exhausted at the time of the
  request.  This reply therefore quotes no verdict; Juno is reviewing.

  WORKERS: paused at pickup, resumed on this staging.
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

# Reply — 0277: the frame now says why it cannot see its card

**Status: DONE and bench-verified. No Copilot verdict — see the caveat at the end.**

| Field | Value |
|---|---|
| `branch` | `hermes/0277-sd-error` |
| `tip_sha` | **9bbdef8** |
| `what changed` | `src/live.c` (failure path only) + new `src/notice_sd.h` |
| `copilot` | **not available** — quota exceeded |

## What it does now

With no card, the frame boots through the splash and then says so, and stays saying it:

```
SD mount failed: ESP_ERR_TIMEOUT (263)   ×6
no-SD library holds 0 frames, next index 0 in /sdcard/LIVE
splash : logo buffer released (SD error on the panel)
sd     : no card at boot -- error on panel, retrying every 10 s
```

Panel: **"SD card not found / Check the card, then power cycle"** — held, no reboot loop, no hang on the
logo. With a good card, every boot is unchanged:

```
SD mounted, SD32G psn=0xc6bd0f92, 30436 MB
SD library holds 37 frames, next index 56
```

## The changes, precisely

- `src/live.c`: the boot mount loop is untouched (still six attempts); only what happens *after* it fails
  is new — a 24 h notice on the existing notice machinery, `splash_released()` so the logo gives way to
  the message, and a log line naming the cause.
- `src/live.c`: the live loop retries the card every 10 s while it is missing. On success it rescans,
  rebuilds the shuffle order, clears the notice and starts the slideshow immediately. That also means a
  card **re-seated while the frame is running** comes back within 10 s, without a power cycle — beyond
  what the task asked, declared because the retry made it free.
- `src/live.c`: `splash_released()` now takes a reason — it used to log "after the first photo" even when
  there was no photo, which is exactly the kind of line that misleads a future diagnosis.
- `src/live.c`: a failed retry logs once a minute, not every 10 s, so a frame left without a card for an
  hour does not flush the boot evidence out of the 64 KB report.
- `src/notice_sd.h`: new 480×140 portrait-space asset, same converter path and blit as the other notices.

Untouched: splash asset and code, photo library, slideshow, keep/discard, camera code, SPI pins, SD layer.
The 0261 mirror fix is untouched (`set_hmirror 0` still in the camera init).

## Caveats, plainly

- **No Copilot peer review.** The request was sent and Copilot returned *"You have exceeded your monthly
  quota"*; the review never ran, so this reply quotes no verdict rather than inventing one. Juno is
  reviewing instead, and Copilot is back Nov 30.
- **An empty but readable card** still holds the splash — the error covers "cannot contact the card", not
  "card has no photos". Out of scope here; say the word and it is a two-line change.
