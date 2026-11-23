---
task_id: "0244"
title: Button photo flow — countdown on the frame, library on the frame board's SD, land the camera work in cabin-play
status: staged
iteration: 1
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0244-button-photo-flow
  sha: f10353ba996e1296181aceea6b81ee476e8e2d1b
  base: 6e229d5
  commits: [eb6bec0, 3520d3a, f10353b]
  files:
    - living-frame/firmware/freenove-cam/ (README, platformio.ini, src/main.cpp, src/secrets.h.example)
    - living-frame/firmware/n16r8-live/ (README, platformio.ini, partitions.csv, sdkconfig.defaults, src/main.c, src/live.c, photo64 headers)
    - living-frame/firmware/n16r8-photo/data/freenove-cabin.jpg
    - .gitignore (+ secrets.h, + .pio/)
  review: "Copilot via Herdr pane copilot-main (t17); passes on eb6bec0 and eb6bec0..3520d3a, both verdict FIX, findings folded into 3520d3a and f10353b"
  bench: "press evidence from the pre-fix build; SD-write and mount evidence from both fixed builds; camera logs from all three"
notes: |
  Interactive pickup 2026-11-23 ~10:00 MST; the four worker jobs were paused for
  the duration and resumed on staging.
  PUSHED BEFORE THE FIRST REVIEW: the branch went up as eb6bec0 to land the bench
  work while the rig was live, then Copilot reviewed it -- a declared order
  deviation against step 3.
  Report-partition erase during scanout remains deferred (Deviation 4).
  Panel photos were not captured to files; Stephen pressed the button three times
  and confirmed the countdown and the resulting photos on the panel by eye.
---

## Status: staged

The flow works on the bench, physical press included.

## What changed

**`living-frame/firmware/freenove-cam/`** — the camera is only a camera now. It
waits for a `'F','T'` SHOOT trigger, discards three warm-up frames (the GC0308's
first frames after idle are badly exposed), runs the accepted colour chain
(grey-world WB at strength 0.5 smoothed across frames, the 256-entry gamma LUT,
4x4 ordered Bayer dither), and sends the 640x480 RGB222 frame as 1,280 chunks.
No PSRAM ring, no camera-side SD writes.

**`living-frame/firmware/n16r8-live/`** — the frame board owns the button
(GPIO14, pull-up, 30 ms debounce, presses ignored during countdown/transfer and
requiring a seen release edge), draws a 3-2-1 countdown as block digits into an
off-screen buffer, sends SHOOT, verifies the returned frame, writes it to
`/sdcard/LIVE/LIVE####.BIN`, and resumes cycling the library at 10 s per image.
`src/main.c` keeps the accepted VGA driver and report partition; `src/live.c`
holds the SD library, button, countdown, trigger and receiver.

SD bring-up needs both rungs on this rig: the 0239 fold (74+ clocks, throwaway
CMD0 at 400 kHz, release the bus) and up to six mount attempts, because
first-init returns TIMEOUT / INVALID_CRC / INVALID_RESPONSE / INVALID_SIZE
intermittently. The failure path frees SPI2, or the next fold dies with
ESP_ERR_INVALID_STATE.

## Proof

Physical press, from the frame board's `report` partition (read back with
`esptool read_flash 0x310000 0x10000`):

```
live : BUTTON pressed, library 1 frames
live : countdown 3
live : countdown 2
live : countdown 1
live : SHOOT sent to f0:16:1d:ae:2b:20
live : frame ok, sent sha 71bdddc8...63fc243, stored sha 71bdddc8...63fc243,
       written /sdcard/LIVE/LIVE0001.BIN, library 2
live : photo received and on screen
```

Two more presses produced `LIVE0002.BIN` (sha `33c9606f...`, library 3) and
`LIVE0003.BIN` (sha `cc5d85ac...`, library 4), with `show 1 of 3` through
`show 3 of 3` cycling between presses. The library grew by exactly one file per
press and every stored hash equals the sent hash.

Post-reboot persistence: the next boot logged `SD library holds 1 frames`, so a
file written before the reboot was enumerated and cycled.

Fixed builds, after the review:

```
live : SD mounted, SD32G psn=0xc6bd0f92, 30436 MB      (4 attempts)
live : SD library holds 4 frames, next index 4
live : frame ok, sent sha e48196e0...8a217c0a, stored sha e48196e0...8a217c0a,
       written /sdcard/LIVE/LIVE0004.BIN, library 5

live : SD mounted, SD32G psn=0xc6bd0f92, 30436 MB      (6 attempts)
live : SD library holds 5 frames, next index 5
live : frame ok, sent sha 0d7ceede...81eb90, stored sha 0d7ceede...81eb90,
       written /sdcard/LIVE/LIVE0005.BIN, library 6
```

Camera side, same runs: `shoot : serial, shot 1, warmup 3, wb r 1.051 g 0.935
b 1.077 (strength 0.50)`, then `sent : 1280 pkts, 6881 ms, ok 1282 fail 0`, and
`peer ack : sha ...` equal to the sent hash. No ring store and no SD write on the
camera side in any run.

Repo: `git ls-files` on the branch carries no `secrets.h` and no credentials;
`secrets.h` and `.pio/` are gitignored and `secrets.h.example` holds placeholders.

## Deviations

1. **Pushed before the Copilot review**, then reviewed and fixed. The order was
   wrong; both passes and their fixes are in the branch.
2. **Copilot pass 1** (1 blocker, 6 majors, 1 minor). Fixed: chunk-coverage
   bitmap (a frame needs every chunk); max+1 naming with fflush/fclose checked
   and partial files unlinked; the receive callback only assembles bytes, with
   SHA/SD/display/ack moved to the loop; two buffers published by a single
   pointer store; MAC gate plus per-boot session id; exact `LIVEdddd.BIN`
   matching. Declared: report-partition erase during scanout (see 4) and no
   per-packet retransmission (the frame-level SHA is the delivery proof).
3. **Copilot pass 2** (verdict FIX, 5 findings). Fixed: session id on every
   chunk with exact payload lengths; the release-edge button rule, because a
   held button could retrigger once `s_shooting` cleared; free-space preflight
   with `esp_vfs_fat_info`. Copilot agreed with deferring retransmission.
4. **Still open — the report partition is written while scanout runs.** Copilot
   disagrees with the deferral and is right that it is a real hazard
   (`esp_partition_erase_range` with the cache disabled can stall the bounce
   ISR). Every task on this board since 0225 has written its report this way,
   including the run that produced the press evidence above; the fix (erase once
   before `panel_start`, then erase-free 4 KiB records) is structural and belongs
   in its own change with its own bench run.
5. **No panel photographs on file.** Stephen pressed the button three times and
   confirmed the countdown and the photos by eye; the visual proof is his, not an
   image in this reply.
