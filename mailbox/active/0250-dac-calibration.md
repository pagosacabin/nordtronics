---
task_id: "0250"
title: Per-channel DAC calibration — measure the real ladder levels, build per-channel quant tables
status: in_progress
iteration: 2
expect-reply-within: 24h
proof: []
notes: |
  BLOCKED at rung 2 -- no bench hands during this tick.  DECISION OWED: the 3x4
  voltage table needs a DC meter on the VGA connector with the monitor attached,
  and nothing in this run can produce one.  Rung 1 is complete and pushed; rung 3
  cannot start without rung 2.  Nothing is staged and there is no proof block.

  Rung 1 (done, branch hermes/0250-dac-calibration at f701ecf13a94f4890c3fefcf41a70e9940fecfd3,
  base 113c067, read back from origin after the push):
    - freenove-cam/src/main.cpp: build_quant_table now builds each channel's
      table from its own row of s_dac_v[3][4] (drive level of code v as a
      fraction of that channel's full scale V[3]), linearised with DAC_GAMMA 2.2.
      The rows ship as the ideal ladder {0, 1/3, 2/3, 1}; rung 3 replaces the
      numbers and touches nothing else.
    - n16r8-slideshow/tools/make_dac_frames.py generates the 12 solid test
      frames; bench/dac-cal/ carries them (480,000 B each, packed
      (B<<4)|(G<<2)|R) with SHA256SUMS, the rung-2 procedure (card backup,
      staging, probe pins, reading table, restore), and check_table_equivalence.c.

  Equivalence of the restructure, measured not asserted: 17 of the 256 table
  entries per channel move by one dither-threshold step, and the C harness builds
  both tables with the firmware's own float arithmetic and shows the 4x4 Bayer
  tile mean is bit-identical at all 256 inputs.  Builds: pio run -e freenove-cam
  SUCCESS 820,637 B (26.1%), pio run -e n16r8-slideshow SUCCESS 834,837 B (26.5%).

  Copilot peer review (Herdr pane t17), two passes.  Pass 1 Verdict: FIX with
  two findings: MINOR that the ideal rows are not byte-identical to the old
  literals (17 threshold changes per channel) -- fixed by quantifying the drift
  in the source comment and committing the harness that proves the emitted means
  are unchanged; NIT that the generator's shift field was unused -- removed.
  Pass 2 on the incremental diff: "No remaining findings.  Verdict: SHIP".

  Deviations, declared.  (a) The SD card reader is attached but holds no card
  (sda reports 0 B), so rung 1's FRAMES backup was not taken -- and no card was
  touched, so there is nothing to restore; the procedure's Step 1 does the backup
  at the bench before the frames go on.  (b) The frames are committed raw rather
  than gzipped: 5.6 MB in the worktree, ~20 KB in git objects because each is one
  repeated byte, and the bench step is then a plain copy with SHA256SUMS.  (c) The
  three code-0 frames are the same black file by construction (0x00 on every
  channel); they are kept as three files because the reading table has three
  code-0 cells and they double as the meter's zero check.

  No voltage is reported, estimated or inferred anywhere in this reply.
cost:
  model: deepseek-flash
  api_calls: 78
  input_tokens: 68311
  input_cache_hit_tokens: 7656576
  input_cache_miss_tokens: 68311
  output_tokens: 36297
  total_tokens: 7761184
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.0549946
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-23T19:46:00+00:00"
lease_until: "2026-11-23T21:16:00+00:00"
---

## Status: blocked at rung 2 — rung 1 is done and pushed, rung 3 cannot start

**Nobody was at the bench when this tick ran (12:45 MST), so rung 2 was not
measured and rung 3 was not built.** That is the assignment's stated outcome for
this case: rung 1 complete, procedure written, test frames ready, and the
measurement left to a human with a meter. **There is no `proof` block.** The
branch below is cited as a fact about what was pushed, not as evidence of
completion, and no voltage appears anywhere in this reply that was not measured.

### Rung 1 — done

Branch `hermes/0250-dac-calibration`, tip
`f701ecf13a94f4890c3fefcf41a70e9940fecfd3`, in `pagosacabin/cabin-play`, based on
`113c067` (0249's verified tip). Read back from origin after the push:
`git ls-remote --heads origin hermes/0250-dac-calibration` returns that same SHA.

What changed:

| file | what |
|---|---|
| `living-frame/firmware/freenove-cam/src/main.cpp` | `build_quant_table` builds each channel's table from its own row of `s_dac_v[3][4]` — the drive level of code v as a fraction of that channel's full scale `V[3]` — linearised with `DAC_GAMMA` (2.2). The three rows ship as the ideal ladder `{0, 1/3, 2/3, 1}`. |
| `living-frame/firmware/n16r8-slideshow/tools/make_dac_frames.py` | generates the 12 test frames deterministically from the packed-byte rule. |
| `living-frame/firmware/n16r8-slideshow/bench/dac-cal/` | the 12 frames (480,000 B each), `SHA256SUMS`, the rung-2 procedure, `check_table_equivalence.c`. |

Both firmware images build on the branch: `pio run -e freenove-cam` SUCCESS,
820,637 B of 3,145,728 (26.1 %); `pio run -e n16r8-slideshow` SUCCESS, 834,837 B
(26.5 %) — the measurement build compiles, so the bench step will not stall on
the tree.

**What the restructure does, and what it does not do.** With the ideal rows the
emitted picture is unchanged in level: 17 of the 256 table entries per channel
move by one dither-threshold step (computing `powf(1/3, 2.2) = 0.08919351` and
`powf(2/3, 2.2) = 0.40982577` instead of the old literals `0.0891` / `0.4097`),
and the C harness committed beside the frames builds both tables with the
firmware's own float arithmetic and shows the 4×4 Bayer tile mean is
bit-identical at **all 256** input codes — 0 differing means. The three channel
tables are identical to each other while the rows are ideal, which is the point:
the rows exist so that a measured per-channel error can be expressed at all.

### The 12 test frames

One solid frame per channel per 2-bit code, in the accepted format
(800×600, 1 byte per pixel = `(B<<4)|(G<<2)|R`). `n16r8-slideshow` streams the
file straight into the framebuffer, so the byte at the VGA pin is literally the
byte in the file:

| frame | byte | R | G | B |
|---|---|---|---|---|
| `DAC-R0/G0/B0.BIN` | `0x00` | 0 | 0 | 0 |
| `DAC-R1.BIN` | `0x01` | 1 | 0 | 0 |
| `DAC-R2.BIN` | `0x02` | 2 | 0 | 0 |
| `DAC-R3.BIN` | `0x03` | 3 | 0 | 0 |
| `DAC-G1.BIN` | `0x04` | 0 | 1 | 0 |
| `DAC-G2.BIN` | `0x08` | 0 | 2 | 0 |
| `DAC-G3.BIN` | `0x0c` | 0 | 3 | 0 |
| `DAC-B1.BIN` | `0x10` | 0 | 0 | 1 |
| `DAC-B2.BIN` | `0x20` | 0 | 0 | 2 |
| `DAC-B3.BIN` | `0x30` | 0 | 0 | 3 |

Names sort R0…B3, which is the measurement order; the slideshow shows each for
~10 s, so a pass is about two minutes. The manifest verifies (`sha256sum -c
SHA256SUMS`: 12 × OK) and re-running the generator reproduces the same 12 digests
to `/tmp` and against the committed manifest.

### Rung 2 — not measured, and why that is the honest state

The procedure is written out in
`living-frame/firmware/n16r8-slideshow/bench/dac-cal/README.md`, bench-ready:

1. Card in the laptop reader; back up `/sdcard/FRAMES` (count + `sha256sum`) —
   the LIVE library and `/NEWCARD.BIN` are not touched.
2. Move the originals to `FRAMES-CABIN`, copy in the 12 `DAC-*.BIN`, verify
   against `SHA256SUMS`, so `FRAMES` holds exactly twelve files.
3. Flash `n16r8-slideshow`, monitor on the VGA port.
4. Probe DB15 pin 1 (R), 2 (G), 3 (B), reference on pin 5/10, **monitor
   connected** (its termination is part of the divider). Two readings per cell,
   two passes; the table in the README is laid out to be filled in. Full scale
   per channel stated in volts.
5. Restore: remove the `DAC-*.BIN`, put the originals back, verify the count and
   the hash list against Step 1, reflash whichever build was on the board.

A DC meter averages over blanking, which scales every reading on a channel by the
same active-frame fraction — the tables need only the ratios `V[v]/V[3]`, so that
common scale cancels and no correction is applied.

The voltage table, as measured so far: **no cells read.** Unread, not zero.

### Rung 3 — not started

It consumes the rung-2 numbers (`V[v]/V[3]` into `s_dac_v[3][4]`, rebuild), so
it cannot begin before the meter work. The before/after panel photos under
`mailbox/files/0250/` go with it, not with this reply.

### Copilot peer review — done, two passes

Driven through Herdr pane `t17`, read-only against the branch.

- **Pass 1 (7990efb) — Verdict: FIX.** 1. MINOR: the ideal rows are not
  byte-for-byte the old table (its count: 17 threshold changes per channel);
  preserve the old constants or document the drift. 2. NIT: `shift` captured but
  unused in the generator. Both fixed: the source comment now carries the
  measured numbers and points at the committed harness, and the field is gone.
- **Pass 2 (f701ecf, incremental diff 7990efb..f701ecf) — "No remaining
  findings.", Verdict: SHIP.** It compiled and ran the harness itself before
  answering.

### Deviations, declared

- **No FRAMES backup was taken** because no card was available to back up: the
  card reader is attached but empty (`sda` reports 0 B) and no card was touched.
  The backup is Step 1 of the procedure, executed at the bench before the test
  frames go on.
- **The frames are committed raw, not gzipped.** 5.6 MB in the worktree but about
  20 KB of git objects (each file is one repeated byte), and the bench step stays
  a plain `cp` verified by `SHA256SUMS`.
- **The three code-0 frames are the same black file** by construction. They are
  kept as three files because the table has three code-0 cells and they double as
  the meter's zero check — not because the bytes differ.
- **The refactor is not bit-identical to the previous build** (17 thresholds per
  channel, means unchanged). Declared here rather than left for a diff to find.

### DECISION OWED

Someone with a DC multimeter at the frame board runs rung 2 (the README above is
the whole procedure, ~20 minutes plus the card copy). Rung 3 follows from its
table. Until then this task stays here: no proof block, no staging, nothing
invented.

---


## Assignment

- **Hermes:** rung 1 (test frames + code restructure), rung 3 (build the measured tables in, before/after panel photos), the Copilot step, and the reply.
- **Stephen:** the rung-2 meter readings — his hands on the probes, or Hermes's with Stephen at the bench. If nobody is at the bench when the worker picks this up, do rung 1, park in active/ with the procedure written and the test frames ready, and say so plainly. Never estimate or fabricate a voltage.
- **Copilot:** peer review of the cabin-play diff (GitHub-bound), driven by Hermes via Herdr. Reviewer, not the proof.
- **Juno:** verifies independently — branch SHA against origin, the voltage table against the reply, before/after photos present.

## Context

The living frame's colour pipeline (on the camera board, `living-frame/firmware/freenove-cam`) quantises through tables built from assumed ideal DAC levels: `L = {0, 0.0891, 0.4097, 1.0}`, i.e. `(v/3)^2.2` for codes 0–3, and all three channels get identical tables. The real resistor ladder on the frame board (2-bit DAC: R GPIO 4/5, G 6/7, B 15/16) will be off those ideals by a few percent, the GPIO output resistance shifts the steps further, and full scale may not be 0.7 V. A per-channel level error is a plausible mechanism for the cool cast seen on the panel (separate layer from white balance — 0249 shipped WB per-shot; the WB-strength knob is not touched here).

This task comes from an external review Stephen shared and Juno assessed in chat 2026-11-23; Stephen approved filing the calibration rung first. The display side itself is settled (0243 accepted; 800×600 at exact 2× on the SyncMaster 213T) and is not reopened here.

## Task

**Rung 1 — prepare (no bench hands needed).**
- Generate 12 test frames as .BIN files in the accepted packed format: for each channel (R, G, B) and each code (0–3), a solid frame with that code on that channel and 0 on the other two.
- Back up the frame SD's `/sdcard/FRAMES` set to the laptop before touching it; the test frames go in only for the measurement session and the original set is restored byte-for-byte afterwards (verify by file count and, where available, hashes). The LIVE library (36 frames at last count) is not touched.
- Restructure the quant-table build in freenove-cam so each channel's table is built from that channel's own level array, with the ideal `{0, 1/3, 2/3, 1}` voltage ratios as the shipped defaults — behaviour-identical until rung 3 supplies measured numbers.

**Rung 2 — measure (bench hands).**
- With the monitor connected (its termination is part of the divider), display each test frame and read the channel's voltage at the VGA connector pin (or DAC output node) with a DC multimeter. Take each reading at least twice; record both.
- A DC meter averages over blanking, which scales every reading by the same active-frame fraction — that is acceptable because the tables need only the ratios `V[v]/V[3]` within each channel. Record code 0 as a sanity floor (expect ~0 V).
- The deliverable of this rung is the 3×4 voltage table, recorded in the reply, with full scale per channel stated.

**Rung 3 — build and prove.**
- Build each channel's quant table as `L_c[v] = (V_c[v] / V_c[3])^γ` with γ = 2.2 (γ is not a variable in this task). Flash the camera board, shoot the same scene used for the recent panel photos, and attach before/after panel photos under `mailbox/files/0250/`.
- Confirm nothing else moved: transfer timings and stored-frame hashes behave as in 0249 (this change is camera-side processing only; the .BIN format and the link are untouched).

## Success criteria

- The reply carries the full 3×4 measured voltage table (two readings per cell) with the monitor connected, and full scale per channel.
- The three per-channel tables in the shipped firmware differ from each other exactly as the measurements dictate; γ = 2.2 stated.
- Before/after panel photos of the same scene are attached, and the FRAMES restore is verified.
- A null result is a valid result: if the measured ratios are within meter noise of ideal on every channel, ship the code restructure with the ideal defaults, report the table, and say plainly that the ladder is not the cast's cause.

## Constraints

- Camera firmware only (`living-frame/firmware/freenove-cam`) for the table change. Frame firmware, the .BIN format, and the ESP-NOW link are untouched.
- White balance behaviour and strength are not changed in this task — one variable at a time.
- Cabin-play branch `hermes/0250-dac-calibration`; Copilot peer review before the push is called done; `git fetch` before any push.
- No fabricated measurements: an unmeasured rung is reported as unmeasured.
- Secrets never in files. Flash tier is fine.

## Proof

- Branch SHA on origin (Juno reads it back).
- The voltage table in the reply.
- Before/after panel photos at `mailbox/files/0250/`.

## Reply format

Status line first. The voltage table. What the tables became. The photos. Deviations plainly.
