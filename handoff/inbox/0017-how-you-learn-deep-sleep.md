# 0017 — How you learn, then deep-sleep lesson 1

Stephen's standing rule for this task: never assume how much you know. State what you know, say what you don't.

## Part A — how do you learn new technical tasks? (answer first, in your own words)

1. When I hand you something you haven't done before — like ESP32-S3 deep sleep — what inputs actually help you learn it? Datasheets, example code, step-by-step instructions, something else? Be specific.
2. What do you do when you hit something you don't know in the middle of a task?

Short and honest. Don't perform knowledge you don't have.

## Part B — what do you already know about S3 deep sleep?

Before writing any code, write down what you already know: the sleep modes, the wake sources, what `esp_sleep_enable_timer_wakeup()` and `esp_deep_sleep_start()` do, and what RTC memory (`RTC_DATA_ATTR`) is for. If you don't know something, write "I don't know" — that's the point of this task. If you need source material, fetch the ESP32-S3 Technical Reference Manual (low-power management chapter) yourself and cite the sections you used.

## Part C — lesson 1 firmware: timer-wake deep sleep

Write PlatformIO firmware (ESP32-S3, Arduino framework, same board definition as `firmware/node-v1`) in `firmware/lessons/lesson-01-timer-wake/`:

1. On boot: increment a counter held in RTC memory, print `boot #N` over serial.
2. Print `sleeping 10s`, then enter deep sleep with a 10-second timer wakeup.
3. On wake the counter must have survived — the serial log should show it climbing once per ~10s.

Then update `.github/workflows/platformio.yml` on your branch so CI also builds this lesson directory: add the path trigger and a build step following the existing pattern.

- Branch: `hermes/0017-deep-sleep-lesson-1`
- Push the branch. No PR, no merge.

## What you can't do — say it plainly

You have no USB hardware: you cannot flash this or watch serial output. Stephen will flash it and read the serial log. Your proof is a pushed branch with CI green on the lesson build. Include a note on what Stephen should expect to see on the serial monitor.

## Reply (outbox file `0017-*.md`)

- Status: done | blocked
- Branch:
- Commit SHA:
- Part A answers
- Part B: your stated pre-existing knowledge, plus what you had to look up
- Part C: what the firmware does, and what Stephen should expect on the serial monitor
- Anything you were unsure about while writing it
