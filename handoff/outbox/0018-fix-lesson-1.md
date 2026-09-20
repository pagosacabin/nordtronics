# 0018-fix-lesson-1: Outbox Report

**Status:** done

**Local Repo Path:** /home/astroboy/nordtronics

**Environment:** Linux (Fedora 43, x86_64), 412G free disk

## What was fixed

The lesson-1 firmware had a bug: `setup()` called `esp_deep_sleep_start()` without ever calling `esp_sleep_enable_timer_wakeup()`. Without a wakeup source enabled, the chip sleeps forever — it will never wake on its own. The work had to match the Part B answer which correctly described `esp_sleep_enable_timer_wakeup()`.

### Changes made

1. Added `esp_sleep_enable_timer_wakeup(10 * 1000000)` before `esp_deep_sleep_start()` — enables the 10-second timer wakeup source
2. Added `Serial.flush()` after the serial prints and before sleeping, so no output is cut off

### Branch and commit

- **Branch:** hermes/0017-deep-sleep-lesson-1
- **Commit SHA:** dbd9763
- **Push URL:** `git push origin HEAD:hermes/0017-deep-sleep-lesson-1`

## CI

CI was not triggered (no PR pipeline), but the commit pushes the fix to the existing branch `hermes/0017-deep-sleep-lesson-1` which already has the lesson-1 timer-wake deep sleep firmware.

## Definition of done

Outbox report written, commit pushed to `hermes/0017-deep-sleep-lesson-1` on origin, firmware matches the stated fix (timer wakeup enabled + flush before sleep).

**Author:** Hermes Agent  
**Date:** 2026-09-19