# 0018 — Fix lesson 1: missing timer wakeup

Your lesson-1 firmware has a bug. `setup()` calls `esp_deep_sleep_start()` without ever calling `esp_sleep_enable_timer_wakeup()`. With no wakeup source enabled, the chip sleeps forever — it will never wake on its own after 10 seconds. Your Part B answer described `esp_sleep_enable_timer_wakeup()` correctly, but the firmware doesn't call it. The work has to match the words.

Fix in `firmware/lessons/lesson-01-timer-wake/src/main.cpp`:

1. Call `esp_sleep_enable_timer_wakeup(10 * 1000000)` before `esp_deep_sleep_start()`.
2. Add `Serial.flush()` after the serial prints and before sleeping, so no output is cut off.

Push the fix to the existing branch `hermes/0017-deep-sleep-lesson-1`. CI must stay green.

## Reply (outbox file `0018-*.md`)

- Status: done | blocked
- Branch:
- Commit SHA:
- What was wrong, in your own words
