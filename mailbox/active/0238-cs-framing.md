---
task_id: '0238'
status: in_progress
iteration: 2
proof: []
expect-reply-within: 6h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-22T04:01:08+00:00"
lease_until: "2026-11-22T05:31:08+00:00"
notes: |
  Filed 2026-11-21 20:50 MST at Stephen's direction in chat ("Yes" to Juno's
  offer to file the CS/framing test). Card in the holder is the 512 GB
  SanDisk Recalbox card from 0234/0236 — READ-ONLY, no writes, no format,
  nothing that touches its contents. If the card has been swapped since, run
  the same test on whatever card is seated and say which.
---

# 0238 — First-pass SD failure: is the first frame shifted, and can a sacrificial frame absorb it?

## Assignment

- **Hermes:** all of it — firmware change, bench run, report. (Bench firmware
  stays yours; nothing here goes to Newton.)
- **Copilot:** peer-review the probe change before the bench run, per the
  standing GitHub rule. Reviewer, not proof — Juno verifies the staged reply
  independently.
- **Juno:** verifies the reply (branch tip vs origin, numbers vs the logged
  report read back from the board), archives, pings Stephen.

## Context

0236 killed the timing hypothesis: a power-up delay of 0–250 ms (on top of
the probe's existing 250 ms post-boot settle) changed nothing — 0/13 first
passes at every value, 65 boots. What the delay sweep exposed is the shape of
the fault: on attempt 1, CMD0 returns `ff ff f8 47` (a `0xf8` lead byte in
front of a plausible R1) and CMD8 returns `ff 00 00 00 01 aa` — the R7 echo
is present but read **one byte late**. Attempt 2 is clean. The first frame
after a reset is shifted by one byte, not slow. Stephen's remaining named
hypothesis is CS/framing.

Base branch: `hermes/0236-powerup-delay`
@ 6d7431133bfc94717725eacab3b45b1c061c8d6a in pagosacabin/cabin-play, project
`living-frame/firmware/n16r8-sdprobe/` (delay parameter compiled at 0 — the
control configuration). Board: YD-ESP32-23 / N16R8, MAC c4:9e:7e:08:6d:f4.
SD pins SCK 12 / MOSI 11 / MISO 13 / CS 10, SPI mode 0, 400 kHz. The
first-pass slot table (NVS, keyed by build stamp) and `bench/reset-loop.sh`
harness from 0234/0236 are the measurement tools; reuse them.

## Task

One hypothesis family — framing of the first transaction after reset — in
two rungs. Run rung 1; run rung 2 only if rung 1 does not cure the first
pass. ≥12 resets per variant, control first.

0. **Control** — unmodified 0236 configuration. Must reproduce ~0/13
   first-pass, or the setup is wrong; say so and stop.
1. **Sacrificial frame** — after the standard ≥74 dummy clocks (CS high),
   issue ONE throwaway transaction whose response is discarded: pull CS low,
   clock out a single CMD0 frame, read and log its raw response bytes, pull
   CS high. Then run the normal first-pass measurement as the next
   transaction. For this variant, "first pass" means the first REAL init
   attempt after the sacrifice: record whether its CMD0 returns a clean
   R1=0x01 on time and its CMD8 echo arrives in frame.
2. **Clean CS re-assert** (only if rung 1 fails) — no sacrificial command.
   Instead, between the dummy clocks and the real CMD0: drive CS high,
   hold ≥1 ms, then assert CS low and issue CMD0 within the same CS-low
   period, exactly as the control does otherwise. This tests the CS edge /
   state, not settling (0236 already killed settling).

Record per variant: first-pass successes / resets, and the raw first-frame
bytes (CMD0 and CMD8 responses) for both the sacrificial frame (rung 1) and
the first real attempt — if the sacrifice absorbs the shift, its bytes
should show the `f8`-lead pattern and the real attempt should be clean;
that pairing is the evidence.

Restore the board to the accepted 800x600 build afterward and verify from
the board (framebuffer sha256 204578cc…, MATCH), as in 0234/0236. Verify
the pre-state before flashing anything: 0236 found the board carrying a
wrong image once already — read the report partition first and say what
was on the board when you arrived.

## Success criteria

- A table: variant → first-pass successes / resets run, control at ~0.
- A verdict in one sentence: which variant (if any) makes the first pass
  reliable, OR "neither framing variant changes the first-pass rate."
- The full reports read back from the board's report partition, committed
  as logs on the branch (same pattern as 0236's logs/09–13).

## Constraints

- Card is READ-ONLY: raw init commands and reads only; no mount with write
  access, no write test, no format. format_if_mount_failed=false if a mount
  is attempted at all.
- Framing only: do not change the SPI clock speed, the delay parameter
  (stays 0), or pull-ups in this task. The 3.3 V rail question is a meter
  task for Stephen's hands, not a rider here.
- Push the branch via SSH (gh.py writes to cabin-play return 403). No PR
  needed; a pushed branch + the staged reply is the deliverable.
- Cost: DeepSeek off-peak rates; one bench session.

## Proof

- Branch `hermes/0238-cs-framing` on pagosacabin/cabin-play at the pushed
  tip (Juno checks the SHA against origin).
- The variant-vs-first-pass table, backed by the committed report-partition
  logs.
- Board-restored line (800x600, framebuffer hash MATCH).

## Reply format

Lead with the table and the one-sentence verdict. Then: control result, the
sacrificial frame's raw bytes vs the first real attempt's (rung 1), whether
rung 2 was needed and its result, the board pre-state you found, the
Copilot pass count and verdict, and the board-restored proof. If the
control does not reproduce, lead with that instead.
