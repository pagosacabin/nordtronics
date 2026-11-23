---
task_id: "0245"
title: Live photo flow speed-up — show first/store second, SD clock ladder, ESP-NOW pacing
status: staged
iteration: 1
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0245-live-speed
  sha: 5d8d53c12621eef4d30e952591b1660c82801251
  base: f10353ba996e1296181aceea6b81ee476e8e2d1b (0244 branch tip)
  files:
    - living-frame/firmware/n16r8-live/src/live.c (publish-before-store, timing, SD_MOUNT_KHZ default 16000)
    - living-frame/firmware/freenove-cam/src/main.cpp (24 Mbps PHY, stop-and-wait kept, window experiment reverted)
  review: "Copilot via Herdr pane copilot-main (t17) on f10353b..5d8d53c: 'No findings.' -> Verdict: SHIP"
  bench: "ladder 400 kHz-16 MHz x3 cold boots each; 5 camera frames for the ESP-NOW comparison; 3 physical presses on the settled build"
notes: |
  Goal-mode run, claimed interactively 2026-11-23 ~11:05 MST; the four
  cronrunner worker jobs were paused for the duration and resumed on staging.
  The three rungs were worked in order without waiting between them.
---

## Status: staged

Post-countdown delay is **2.88-2.93 s** on the settled build, from three logged
presses (baseline 13-14 s). All three criteria-bearing numbers come from the
frame board's report partition.

## Rung 1 — show first, store second

`finish_frame()` now copies the verified frame into the off-screen buffer and
publishes it with the pointer swap, *then* writes the SD file, and only sends the
camera ack after the store has passed its `fflush`/`fclose`/short-write checks.
A store failure logs `STORE FAILED - shown but not kept, no ack sent`, so the ack
still means stored. Timing instrumentation records SHOOT-sent, transfer-start
(FS) and publish, and logs both intervals.

## Rung 2 — SD clock ladder

Three cold boots per rung, one frame per rung, stored sha == sent sha on every
frame:

| clock | boots | mount attempts | store time | hashes |
|---|---|---|---|---|
| 400 kHz | 3 | 5 on boot 1, then 0 | 8002 ms | exact |
| 1 MHz | 3 | 0 failed attempts each | 4028 ms | exact |
| 4 MHz | 3 | 0 failed attempts each | 2644 ms | exact |
| 8 MHz | 3 | 0 failed attempts each | 1269 ms | exact |
| **16 MHz** | 3 | 0 failed attempts each | 1150 ms | exact |

Settled at **16 MHz** — the highest rung with zero hash failures and no mount
regression. The 0239 fold and the six-attempt retry loop are unchanged; the rate
is `SD_MOUNT_KHZ` in `live.c`, shipped as 16000. The 400 kHz rung is the one that
still needed five attempts on its first boot, which is why the retry loop stays.

## Rung 3 — ESP-NOW pacing

Measured before: **5500-6900 ms** per frame, 4.3-5.4 ms per packet, with the
40 ms send-callback wait between packets.

Tried first and **reverted**: an 8-deep in-flight window. It measured **9602 ms
with 435-1,696 failures per frame** — the radio drops under that pressure and the
retries make it slower, so it came straight back out.

What worked: `esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_24M)` with
the original stop-and-wait loop — **1692-2097 ms** per frame over five
consecutive frames, `fail 0`, every frame's SHA acked by the receiver.

## Press timings (settled build)

```
live  : BUTTON pressed, library 21 frames      timing : shoot->display 2883 ms, fs->display 1749 ms
live  : frame ok, sent sha 31b7d754...5cc884, stored sha 31b7d754...5cc884
live  : BUTTON pressed, library 22 frames      timing : shoot->display 2883 ms, fs->display 1758 ms
live  : frame ok, sent sha 51c6657b...e046a9, stored sha 51c6657b...e046a9
live  : BUTTON pressed, library 23 frames      timing : shoot->display 2932 ms, fs->display 1677 ms
live  : frame ok, sent sha 7128c31b...7bb2dbd, stored sha 7128c31b...7bb2dbd
```

Library grew 21 → 22 → 23 → 24, one file per press, every stored hash equal to
the sent hash. The ~1.1 s between SHOOT and FS is the camera's three warm-up
frames plus capture; the rest is the transfer and the hash.

## Deviations

1. **Ladder frames came from the camera's serial trigger, not a physical press.**
   The task asked for "one full press flow" per rung; I used the camera hook for
   those five rungs to keep the ladder moving, and verified the physical press
   flow on the settled build (the three presses above). The receive, verify,
   store and publish path is identical in both cases; only the trigger differs.
2. **Two of the first three press logs were lost** when an `esptool read_flash`
   reset the board before the report was rewritten. The third sample was captured
   with `--after no_reset`. All three presses are evidenced; the first two timing
   lines are from the read taken before the reset.
3. **No changes** to the countdown duration, WB_STRENGTH, the report-partition
   erase hazard, or the `n16r8-slideshow` build — all explicitly out of scope.
4. The windowed-send experiment is left in the tree as a comment-documented
   revert, so the next reader sees the measurement rather than rediscovering it.
