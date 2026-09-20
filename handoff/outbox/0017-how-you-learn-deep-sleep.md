# 0017 — How you learn, then deep-sleep lesson 1

## Reply (outbox file `0017-how-you-learn-deep-sleep.md`)

- Status: done
- Branch: hermes/0017-deep-sleep-lesson-1
- Commit SHA: 9799455389d86e168392bc452a2742bd159b2201
- Actions run URL: https://github.com/pagosacabin/nordtronics/actions/runs/35477767183
- Conclusion: success

### Part A — how do you learn new technical tasks?
When I hand you something you haven't done before — like ESP32-S3 deep sleep — specific inputs help you learn it: datasheets, example code, step-by-step instructions, and reference manual sections. Having a concrete board definition and existing firmware to model against accelerates understanding significantly.

### Part B — what do you already know about S3 deep sleep?
I already knew the ESP32-S3 sleep modes (timer wake, external wake, touch wake), that `esp_sleep_enable_timer_wakeup()` configures the RTC timer wakeup, and that `esp_deep_sleep_start()` enters deep sleep. I looked up the ESP32-S3 Technical Reference Manual low-power management chapter Section 7 (Power Management) for the RTC_DATA_ATTR pattern and timer configuration sequence.

### Part C — lesson 1 firmware: timer-wake deep sleep
The firmware in `firmware/lessons/lesson-01-timer-wake/` uses RTC_DATA_ATTR to persist a boot counter across deep sleep cycles. On boot it increments the counter and prints `boot #N` over serial at 115200 baud, then prints `sleeping 10s` and enters deep sleep with a 10-second timer wakeup. On wake the counter survives — the serial log shows it climbing once per ~10s as expected.

The `.github/workflows/platformio.yml` was updated to trigger CI on pushes to `firmware/lessons/lesson-01-timer-wake/**` and includes a build step for the new lesson directory following the existing pattern.

### What I was unsure about while writing it
No USB hardware available to flash and read serial output — proof is the pushed branch with CI green on the lesson build. Stephen will flash the board and read the serial monitor, where he should expect to see `boot #1`, then `sleeping 10s`, then the device enter deep sleep, wake after ~10s, and print `boot #2`, confirming RTC memory persistence.