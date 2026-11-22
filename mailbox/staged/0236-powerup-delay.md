---
task_id: '0236'
protocol_version: 1.3.0
status: staged
iteration: 2
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0236-powerup-delay
  sha: 6d7431133bfc94717725eacab3b45b1c061c8d6a
  run: none
  run_note: >-
    cabin-play carries no .github/workflows at all (ls fails; git ls-tree -r
    --name-only origin/hermes/0236-powerup-delay .github and the same on
    origin/main both return nothing), so no CI run exists or is expected for
    this branch. ntfy not applicable: the task names no topic and nothing was
    published.
  files:
    - living-frame/firmware/n16r8-sdprobe/src/main.c
    - living-frame/firmware/n16r8-sdprobe/README.md
    - living-frame/firmware/n16r8-sdprobe/logs/09-0236-delay-000ms-report.txt
    - living-frame/firmware/n16r8-sdprobe/logs/10-0236-delay-010ms-report.txt
    - living-frame/firmware/n16r8-sdprobe/logs/11-0236-delay-050ms-report.txt
    - living-frame/firmware/n16r8-sdprobe/logs/12-0236-delay-100ms-report.txt
    - living-frame/firmware/n16r8-sdprobe/logs/13-0236-delay-250ms-report.txt
claimed_by: "cronrunner/8b1c9e1323c5"
claimed_at: "2026-11-22T02:31:00+00:00"
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
notes: |
  Filed 2026-11-21 09:55 MST at Stephen's direction in chat ("can we try a
  test with a delay after activating?"). Stephen is on shift at Bart's; this
  is Hermes's bench. Card in the holder is the 512 GB SanDisk Recalbox card
  from 0234 — READ-ONLY, no writes, no format, nothing that touches its
  contents. If the card has been swapped since, run the same test on whatever
  card is seated and say which.
  WINDOW CORRECTED 2026-11-21 19:20 MST: originally filed at 24h (Juno's
  error — the house standard is 6h); the task sat unclaimed ~9.5h and the
  watch never flagged it. Stephen directed a manual nudge (237) and this
  RUN 2026-11-21 19:30-20:40 MST (02:30-03:40 UTC 11-22) — completed and staged.
    Pre-run digest: PEAK OFF-PEAK 02:30Z, PROTOCOL MATCH 1.3.0. Pickup commit
    6d5e777 (iteration 2). active/ held 0216 and 0230, both decision-blocked and
    left untouched (no edit, no iteration change); inbox 0176 is `superseded`
    and 237 is a nudge for this task, so 0236 was the only actionable task.
  WHAT CHANGED: one file, src/main.c. A build-time `SD_FIRSTPASS_DELAY_MS`
  (default 0) inserted between the standard 74+ dummy clocks (CS high) and the
  first CMD0 of the FIRST raw-init attempt only; guarded by #if so delay 0
  compiles the wait out, and an out-of-range value is a compile error. Set per
  build with PLATFORMIO_BUILD_FLAGS=-DSD_FIRSTPASS_DELAY_MS=<ms>, so no source
  edit per value and a fresh build stamp per value (which also starts a fresh
  NVS first-pass table, the table being keyed on a hash of __DATE__/__TIME__).
  The value is printed in the report header and above the table, so each
  committed log names its own delay.
  THE DELAY IS ON TOP OF an existing 250 ms post-boot settle in app_main, so
  the 250 ms row is ~500 ms from reset release to first CMD0. Stated because it
  changes how the ceiling should be read.
  MEASUREMENT: 12 driven resets per value (esptool hard_reset per reset, 42 s
  settle, report read back from 0x310000), five values, 13 slots each (the
  flash boot plus the 12 driven resets), 65 boots total. Control reproduced:
  0/13 with CMD0 answering 0x3f/0x47 — the 0234 result. All five values 0/13.
  Junk signature unchanged with delay: attempt 1 CMD0 = `ff ff f8 47` (a 0xf8
  lead before a plausible R1), attempt 1 CMD8 = `ff 00 00 00 01 aa` (R7 echo
  present but read one byte late), attempt 2 = clean `ff 01 …`. So the first
  exchange is MISALIGNED, not slow — settling time is not the mechanism. The
  CS/framing hypothesis the task names as the fallback is a separate task and
  was not touched (one variable only: no clock, CS-handling or pull-up change).
  CARD: SanDisk SR512, CSD capacity 511,868,665,856 B (512 GB), label
  RECALBOX — the 0234 card, so it had not been swapped. Read-only throughout:
  raw init and CMD17 reads only, mount with format_if_mount_failed=false and no
  write test, nothing written to the card.
  BOARD PRE-STATE (found, not caused): the frame board was NOT on the accepted
  build. The app region matched
  /home/astroboy/wt/0235-goal-trial/…/n16r8-photo-800x600/firmware.bin (built
  2026-11-21 10:07, 536,864 B) and its report partition held `FATAL: embedded
  image is 307200 B, framebuffer needs 480000 B` — an 800x600 env carrying the
  640x480 raster, so it aborted at boot. Identified by reading the app head and
  comparing against every firmware.bin under /home/astroboy (the committed
  0230/0234 restore artifact at n16r8-photo/.pio/build/n16r8-photo-800x600 did
  NOT match: verify_flash digest mismatch).
  RESTORE: regenerated the 800x600 raster from committed source
  (tools/convert_photo64.py --source data/hummingbird.jpg --space gamma --mode
  800x600 → sha256 204578cc…), built env:n16r8-photo-800x600, flashed, and
  reverted the generated headers afterwards (git status clean for that dir).
  Verified from the board's own report: `mode 800x600`, `embedded 480000 B`,
  framebuffer `sha256 204578cc…` == tool, `MATCH`, `362 vsync in 5.998 s ->
  60.35 Hz`, `14473 fills -> 40.0 per frame`; plus app region 0x10000..0xC5000
  read back = df402554…b7211, byte-identical to the firmware.bin flashed, and
  `esptool verify_flash` = digest matched. DEVIATION DECLARED: the flashed
  image's own banner reads `build Nov 20 2026 17:23:09` because the object
  cache for that exact build was reused — the regenerated header is
  byte-identical to the one it was built from, so no TU recompiled. The raster
  sha256 chain and the read-back are what prove the image, not the banner date.
  PEER REVIEW: Copilot (Herdr pane t17) on 21ceff2 before the bench run, one
  pass, one NIT (the two extra `FIRSTPASS_DELAY_MS:` report lines at delay 0),
  Verdict: SHIP. No GitHub PR exists for this branch (writes to cabin-play
  return 403 from this account); the review record is the README section, as
  0227/0228/0230 did. The only commits after the review add logs and docs, no
  code.
  COST: the cron harness does not expose per-call DeepSeek usage objects, so
  the token and cost fields are `unknown` rather than estimated.
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

## Reply 2026-11-22 03:40 UTC (20:40 MST) — STAGED

**Verdict: no delay up to 250 ms changes the first-pass rate.** 65 boots across
five delay values, **0** of them got through the first init pass. The first
exchange after a reset is *misaligned*, not slow, so extra settling time before
the first CMD0 does not reach the fault.

| delay (ms) | first-pass successes / boots | CMD0 first response | CMD8 R1, first attempt |
| ---: | :---: | :--- | :--- |
| **0 (control)** | **0 / 13** | `0x3f` x6, `0x47` x7 | `0x00` x13 |
| 10 | **0 / 13** | `0x3f` x10, `0x47` x3 | `0x00` x13 |
| 50 | **0 / 13** | `0x3f` x10, `0x47` x3 | `0x00` x13 |
| 100 | **0 / 13** | `0x3f` x11, `0x47` x2 | `0x00` x13 |
| 250 | **0 / 13** | `0x3f` x11, `0x47` x2 | `0x00` x13 |

**Control.** 0 ms reproduced 0234 exactly: 0/13, CMD0 answering `0x3f`/`0x47`
instead of `0x01`, second attempt clean. So the test setup is sound and the
comparison is honest.

**The junk pattern does not change with delay, and that is the evidence.** On
attempt 1 CMD0 still returns `ff ff f8 47` (a `0xf8` lead byte in front of a
plausible R1) and CMD8 still returns `ff 00 00 00 01 aa` — the R7 echo is
*there*, just read one byte late — while attempt 2 returns a clean
`ff 01 ff ff…` / `ff 01 00 00 01 aa`. The first frame after a reset is shifted
by one byte, not arriving too early for the card's power-up. A settle delay
cannot fix an alignment fault, and it does not.

**What was built.** One variable, one file (`src/main.c`): a build-time
`SD_FIRSTPASS_DELAY_MS` (default 0) inserted between the standard 74+ dummy
clocks (CS high) and the first CMD0 of the **first** raw-init attempt only.
Attempts 2+ and the second probe call are unchanged; delay 0 compiles the wait
out; an out-of-range value is a compile error (which also proves the `-D`
reached the file). Set per build with
`PLATFORMIO_BUILD_FLAGS=-DSD_FIRSTPASS_DELAY_MS=<ms>`, and the value is printed
in the report header and above the table so each committed log names its own
delay. Nothing else moved: no clock, CS-handling or pull-up change.

**Two numbers to read carefully.** (1) The delay sits *on top of* the existing
250 ms post-boot settle in `app_main`, so the 250 ms row is about half a second
between reset release and the first CMD0. (2) The report's `FIRSTPASS_VERDICT:`
line is 0234's fixed wording and says nothing about the delay —
`FIRSTPASS_SUMMARY` is the number to read.

**Card.** SanDisk `SR512`, CSD capacity 511,868,665,856 B (512 GB), label
`RECALBOX` — the 0234 card, so it had not been swapped. Read-only throughout
(raw init + CMD17 only, `format_if_mount_failed=false`, no write test).

**Not done, on purpose.** The CS/framing hypothesis the task names as the
fallback is a separate task: with the delay flat, no CS-handling change was made
here.

**Board state found before the run — and the restore.** The frame board was not
on the accepted build when this task started: its app region matched a build
from `/home/astroboy/wt/0235-goal-trial/…/n16r8-photo-800x600/firmware.bin`
(2026-11-21 10:07) and its report held `FATAL: embedded image is 307200 B,
framebuffer needs 480000 B` — an 800x600 env carrying the 640x480 raster, so it
aborted at boot. Restored from the committed source at the end (regenerate the
800x600 gamma raster → `sha256 204578cc…`, build `env:n16r8-photo-800x600`,
flash, revert the generated headers). **Board-restored proof, read from the
board:** `mode 800x600`, `embedded 480000 B`, framebuffer `sha256 204578cc…` ==
tool `204578cc…`, `MATCH`, `362 vsync in 5.998 s -> 60.35 Hz`, `14473 fills ->
40.0 per frame`; app region `0x10000..0xC5000` read back =
`df4025540ff0ad6a950eca25fc13607245bdbb95ef79422d6b2407a6d78b7211`,
byte-identical to the `firmware.bin` flashed, `esptool verify_flash` = digest
matched. The flashed image's banner still reads `build Nov 20 2026 17:23:09`
because the object cache for that exact build was reused (the regenerated header
is byte-identical), so the banner is not the evidence — the raster hash chain
and the read-back are.

**Peer review.** Copilot (Herdr pane t17) reviewed `21ceff2` before the bench
run: one pass, one NIT (the two extra report lines at delay 0), **Verdict:
SHIP**. No PR exists for this branch and none was needed; the review record is
the README section, as 0227/0228/0230 did. The commits after the review add logs
and docs only.

**Proof.** Branch `hermes/0236-powerup-delay` at
`6d7431133bfc94717725eacab3b45b1c061c8d6a` (`git ls-remote` confirmed); the full
report partition read-backs are `logs/09`–`13` in
`living-frame/firmware/n16r8-sdprobe/`. `run: none` is enumerated, not assumed:
cabin-play has no `.github/workflows` at all, on this branch or on main. ntfy
not applicable (the task names no topic; nothing was published).
