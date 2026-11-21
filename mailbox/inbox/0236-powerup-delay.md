---
task_id: '0236'
status: pending
iteration: 1
proof: []
expect-reply-within: 24h
notes: |
  Filed 2026-11-21 09:55 MST at Stephen's direction in chat ("can we try a
  test with a delay after activating?"). Stephen is on shift at Bart's; this
  is Hermes's bench. Card in the holder is the 512 GB SanDisk Recalbox card
  from 0234 — READ-ONLY, no writes, no format, nothing that touches its
  contents. If the card has been swapped since, run the same test on whatever
  card is seated and say which.
---

# 0236 — First-pass SD failure: does a power-up delay fix it?

## Assignment

- **Hermes:** all of it — firmware change, bench run, report. (Bench firmware
  stays yours; nothing here goes to Newton.)
- **Copilot:** peer-review the probe change before the bench run, per the
  standing GitHub rule. Reviewer, not proof — Juno verifies the staged reply
  independently.
- **Juno:** verifies the reply (branch tip vs origin, numbers vs the logged
  report read back from the board), archives, pings Stephen.

## Context

0234 measured the frame board's SD first-pass failure precisely: after every
reset, the first SPI exchange with the card returns junk (first CMD0 answers
0x3f/0x47 instead of R1=0x01; first CMD8 answers R1=0x00), the second attempt
is clean, 27 of 27 resets, on two different cards and two builds. The failure
is board-side and deterministic — exactly once per reset — which points at
settling/timing (rail still rising, CS framing, or the first command issued
before the card's power-up sequence has had its required clocks/time), not at
an intermittent contact. Stephen's hypothesis, in his words: try a delay
after activating.

The 0234 probe already has the measurement harness: first-pass slot table in
NVS (namespace sdprobe0234), `bench/reset-loop.sh` driving resets and reading
the report partition back. Base branch: `hermes/0234-sd-card-retest`
@ d17efa70e2fbe13a9b43525530b36c7f47705aa1 in pagosacabin/cabin-play, project
`living-frame/firmware/n16r8-sdprobe/`. Board: YD-ESP32-23 / N16R8,
MAC c4:9e:7e:08:6d:f4. SD pins SCK 12 / MOSI 11 / MISO 13 / CS 10, SPI mode 0,
400 kHz.

## Task

One variable: **a delay inserted after activation (reset release / power-up,
with the standard ≥74 dummy clocks, CS high) and before the first CMD0 of the
raw first-pass measurement.** Everything else in the probe stays byte-identical
in behaviour.

1. Add a build-time or NVS-set delay parameter to the probe's raw init path.
   The delay must apply ONLY to the first pass (attempt 1) measurement path —
   attempts 2+ run exactly as in 0234, so the comparison stays honest.
2. Run the reset loop at each delay value, ≥12 resets per value, reading the
   first-pass slot table after each run:
   - **0 ms (control)** — must reproduce ~0/12 first-pass, or the test setup
     is wrong; say so and stop.
   - **10 ms, 50 ms, 100 ms, 250 ms** — stop early at the first value that
     reaches 12/12 if the trend is already clean at a lower value; always run
     at least 0, 50, and 250 so the table has a floor, a middle, and a ceiling.
3. Record per value: first_pass_ok / total, plus the first CMD0 and CMD8 R1
   bytes for the failures (junk pattern may change with delay — that itself
   is evidence).
4. Restore the board to the accepted 800x600 build afterward and verify from
   the board (framebuffer sha256 204578cc…, MATCH), as in 0234.

## Success criteria

- A table: delay (ms) → first-pass successes / resets run, with the control
  at ~0.
- A verdict in one sentence: the smallest delay (if any) at which the first
  pass is reliable, OR "no delay up to 250 ms changes the first-pass rate."
- The full report read back from the board's report partition, committed as
  a log on the branch (same pattern as 0234's logs/06–08).

## Constraints

- Card is READ-ONLY: raw init commands and reads only; no mount with write
  access, no write test, no format. format_if_mount_failed=false if a mount
  is attempted at all.
- One variable at a time: do not also change clock speed, CS handling, or
  pull-ups in this task. If the delay test comes back flat, say so — the CS
  hypothesis is a separate task, not a rider on this one.
- Push the branch via SSH (gh.py writes to cabin-play return 403). No PR
  needed; a pushed branch + the staged reply is the deliverable.
- Cost: DeepSeek off-peak rates; this is a small probe change plus reset
  loops — keep it to one bench session.

## Proof

- Branch `hermes/0236-powerup-delay` on pagosacabin/cabin-play at the pushed
  tip (Juno checks the SHA against origin).
- The delay-vs-first-pass table, backed by the committed report-partition log.
- Board-restored line (800x600, framebuffer hash MATCH).

## Reply format

Lead with the table and the one-sentence verdict. Then: control result, any
change in the junk-byte pattern with delay, the Copilot pass count and
verdict, and the board-restored proof. If the control does not reproduce,
lead with that instead.
