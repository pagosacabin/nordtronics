---
task_id: "0250"
title: Per-channel DAC calibration — measure the real ladder levels, build per-channel quant tables
status: staged
iteration: 2
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0250-dac-calibration
  sha: 7563ca0ce802ba83692eca3bbb39215f872446b8
  base: 113c067 (0247 tip)
  commits:
    - 2f6fca2 — measured rows in s_dac_v (camera side)
    - 7563ca0 — slideshow bench support (15 s dwell, mount retry, black gap)
  files:
    - living-frame/firmware/freenove-cam/src/main.cpp (s_dac_v rows + the volt table in the comment)
    - living-frame/firmware/n16r8-slideshow/src/main.c (bench support only; .BIN format, ESP-NOW and photo path untouched)
  run: "none — pagosacabin/cabin-play has no CI (gh api repos/pagosacabin/cabin-play/actions/workflows returns total_count 0, and the repo has no .github/workflows directory). No ntfy receipt for the same reason (no workflow, no artifact)."
  bench: "camera flashed with the measured table; esptool verify_flash 0x10000 digest matched on the board (ESP32-D0WD-V3, MAC f0:16:1d:ae:2b:20); one live shot sent 1280 pkts, 4162 ms, ok 1282 fail 0, peer ack SHA equal, library 38 -> 39"
cost:
  model: deepseek-flash
  api_calls: 61
  input_tokens: 5852020
  input_cache_hit_tokens: 5785472
  input_cache_miss_tokens: 66548
  output_tokens: 30836
  total_tokens: 5882856
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.045840
notes: |
  Resumed, finished and staged from a cron tick that found the task in active/ with
  its front-matter destroyed: the bench session that flashed rung 3 replaced the
  whole file with a 9-line note (commit ec1175a deleted 268 lines), so the pickup
  record and the whole rung-1/rung-2 history were gone from the working copy while
  the work itself was real and on the bench. This reply rebuilds the record.

  What this run did, in order:
   1. Read the earlier session's own transcript (default-profile state.db) to quote
      the bench evidence rather than the truncated note: the meter readings, the
      17:21 live shot log, and the fact that Stephen took ONE reading per cell.
   2. Committed the rung-3 table that was left UNCOMMITTED in the cabin-play
      worktree (2f6fca2) and the slideshow bench-support edits (7563ca0), then
      pushed: origin hermes/0250-dac-calibration = 7563ca0ce802ba83692eca3bbb39215f872446b8
      (read back with git ls-remote after the push). Before this run the branch tip
      on origin was f701ecf, i.e. rung 1 only — rung 3 existed solely on this disk.
   3. Rebuilt both envs from the committed tree (freenove-cam SUCCESS 820,637 B,
      n16r8-slideshow SUCCESS 834,985 B) and proved the rebuild is byte-identical
      to the image that was flashed (sha256 0a2fde95...a5d4a47 before and after).
   4. Verified the board itself rather than the note: esptool verify_flash 0x10000
      against that image on /dev/ttyUSB1 -> "verify OK (digest matched)".
   5. Ran the Copilot peer review the card requires (Herdr pane t17) on
      f701ecf..7563ca0: "No findings." Verdict: SHIP.
   6. Recomputed every ratio from the volts and checked the shipped rows against
      them (they match to the three decimals shipped) — table in the reply body.

  What this run did NOT do: the second meter pass, the before/after photos, and the
  FRAMES backup/restore were not performed. The first is Stephen's hands and the
  card never left its holder, so there was nothing to back up or restore; the
  photos need a camera and a same-scene comparison. All three are declared in the
  reply body, and success criteria 1 and 3 are marked NOT MET as written rather
  than counted as done. The bench state at the end of the earlier session: the
  camera is on the measured table, the frame board is on the live build, and the
  frame SD slot then stopped mounting any card at all (6/6 ESP_FAIL on the
  archived 0249 binary too) — a physical fault, not this change.

  The frame-side commit is a declared scope extension: the card said camera
  firmware only, and 7563ca0 touches n16r8-slideshow for measurement support. It
  changes no .BIN handling, no ESP-NOW path and no photo path; it was needed
  because the reader had no card and the test frames had to be shown from the
  board. If that is unacceptable, 7563ca0 can be dropped without touching 2f6fca2.

  Cost: the numbers above are the harness's own session counters
  (session_model_usage for this cron session), token counts as reported by the
  provider and cost computed from the rate card, not from a dashboard total.
---
## Status: staged — rungs 2 and 3 are done and verified on the board; two success criteria are NOT met and are marked so

Everything below is either measured at the bench, read back from the board, or
read from origin. Nothing is inferred from the truncated task note.

### 1. The voltage table (measured 2026-11-23 ~17:05–17:15 MST, Stephen at the meter)

DC meter on the VGA connector, **monitor attached** (its 75 Ω termination is part
of the divider), reference on the same connector's ground shell tied to board GND.
One reading per cell, in volts; the frame board painted the solid test frames
itself (12 frames, 15 s each with a 1 s black gap) because the card reader held
no card.

| channel | L0 | L1 | L2 | L3 (full scale) | L1/L3 | L2/L3 |
|---|---|---|---|---|---|---|
| R (DB15 pin 1) | 0.00 | 0.26 | 0.40 | 0.70 | 0.371 | 0.571 |
| G (DB15 pin 2) | 0.00 | 0.27 | 0.43 | 0.68 | 0.397 | 0.632 |
| B (DB15 pin 3) | 0.00 | 0.27 | 0.45 | 0.72 | 0.375 | 0.625 |
| ideal thirds | 0.00 | 0.333 | 0.667 | 1.000 | 0.333 | 0.667 |

Recomputed from the volts in this run and matching the shipped rows to three
decimals: 0.26/0.70 = 0.3714, 0.40/0.70 = 0.5714, 0.27/0.68 = 0.3971,
0.43/0.68 = 0.6324, 0.27/0.72 = 0.3750, 0.45/0.72 = 0.6250.

Level 1 reads **high** on all three channels (+3.8 %, +6.4 %, +4.2 % of full
scale); level 2 reads **low**, worst on red (−9.6 %). Full scales differ by 6 %
(0.68–0.72 V) and are recorded, not corrected — the table is normalised per
channel, so only the ladder's shape is used.

### 2. What the tables became

`s_dac_v[3][4]` in `living-frame/firmware/freenove-cam/src/main.cpp` now carries

```c
{ 0.0f, 0.371f, 0.571f, 1.0f },   /* R */
{ 0.0f, 0.397f, 0.632f, 1.0f },   /* G */
{ 0.0f, 0.375f, 0.625f, 1.0f },   /* B */
```

with the volts and the divisions in the comment beside them so the numbers can be
re-derived rather than trusted. `DAC_GAMMA` stays **2.2** and
`build_quant_table()` is unchanged — it already built each channel from its own
row (rung 1); rung 3 replaced numbers only.

What that does to the emitted drive (γ-space, i.e. what the dither averages to):

| | level 1 | level 2 |
|---|---|---|
| R | 8.92 % → 11.31 % (+26.9 %) | 40.98 % → 29.19 % (−28.8 %) |
| G | 8.92 % → 13.11 % (+47.0 %) | 40.98 % → 36.49 % (−11.0 %) |
| B | 8.92 % → 11.56 % (+29.6 %) | 40.98 % → 35.56 % (−13.2 %) |

So the darkest quarter gets lighter and the mid shadow gets darker — red most of
all. That is the shape of the correction the ideal thirds were missing. It is a
level correction, not a white-balance change; WB per shot (0249) is untouched.

### 3. Evidence

- **Branch:** `hermes/0250-dac-calibration`, tip
  `7563ca0ce802ba83692eca3bbb39215f872446b8`, read back with
  `git ls-remote --heads origin` after the push (before this run origin held only
  rung 1, f701ecf).
- **Builds from the committed tree:** `pio run -e freenove-cam` SUCCESS, 820,637 B
  of 3,145,728 (26.1 %); `pio run -e n16r8-slideshow` SUCCESS, 834,985 B (26.5 %).
- **Reproducible:** rebuilding the camera image gives a byte-identical file,
  `sha256 0a2fde95498eec83e1793353399b6dc8236e68d013b27ae3a792223dda5d4a47`,
  before and after the rebuild.
- **The board carries it:** `esptool verify_flash 0x10000 .pio/build/freenove-cam/firmware.bin`
  on `/dev/ttyUSB1` → `Verifying 0xc9f50 (827216) bytes @ 0x00010000 … verify OK
  (digest matched)`. Chip ESP32-D0WD-V3 rev v3.1, MAC f0:16:1d:ae:2b:20.
  That check was run by this tick; the flash itself was the earlier bench session.
- **End to end through the live link** (from the 17:21 bench session — quoted from
  its transcript, not re-run by this tick):

      camera  : pid 0x9b, VGA RGB565, no library on this side
      rate    : espnow 24M -> ESP_OK
      shoot   : shot 1, warmup 3, wb r 1.068 g 0.844 b 1.252 (strength 0.50)
      sent    : 1280 pkts, 4162 ms, ok 1282 fail 0, sha 57800c27ba7a47cd0a72b0fc53dff4aa3c1e93c7dc8a467062efdf20a0f85f27
      peer ack: sha 57800c27ba7a47cd0a72b0fc53dff4aa3c1e93c7dc8a467062efdf20a0f85f27
      live    : SD library holds 38 frames, next index 38 in /sdcard/LIVE
      timing  : shoot->display -1 ms, fs->display 4200 ms
      live    : frame ok, sent sha 57800c27…, stored sha 57800c27…, library 39

  1280 packets, 4162 ms, 2 harmless stragglers, zero failures, every hash acked,
  frame stored — the .BIN format and the ESP-NOW link behave as in 0249, which is
  the "nothing else moved" criterion on the camera side.
- **CI:** none to cite. `gh api repos/pagosacabin/cabin-play/actions/workflows`
  → `{"total_count":0,"workflows":[]}`; the repo carries no `.github/workflows`.
  So `run: none` above is enumerated, not assumed, and there is no ntfy receipt
  because there is no workflow and no artifact to name.
- **Copilot peer review** (Herdr pane t17, read-only, diff `f701ecf..7563ca0`):
  asked specifically about ratio arithmetic, whether anything but numbers moved in
  the camera path, the retry loop's correctness, and whether the black gap or
  dwell change anything the shipped path depends on. Answer: **"No findings.
  Verdict: SHIP."**

### 4. Success criteria, honestly

| criterion | state |
|---|---|
| 3×4 voltage table with **two readings per cell**, full scale per channel | **NOT MET as written** — the table is complete and measured once per cell; the second (repeatability) pass was not taken |
| the three per-channel tables differ exactly as the measurements dictate; γ = 2.2 stated | MET — rows are V[v]/V[3] to three decimals, γ 2.2 unchanged, tables now differ from the ideal thirds |
| before/after panel photos of the same scene attached; FRAMES restore verified | **NOT MET** — no photos attached (no camera in this tick, and the honest comparison is one press per build); the card never left its holder, so no backup was taken and nothing needed restoring — that is a skip, not a verification |
| a null result is a valid result | n/a — the ladder is not ideal; the measurements are 4–10 % off in level 2 |

### 5. Deviations and gaps, plainly

1. **One reading per cell.** Stephen took one pass and stopped there; the earlier
   session's own words were "one is enough to start". Repeatability is therefore
   unproven — every number above could be a one-off. Running the same 12 frames a
   second time is a ~4-minute job next bench session and needs nothing but the
   meter.
2. **No before/after photos.** With the measured table this is a level shift of a
   few percent in two of four codes per channel; a photo pair is the only way to
   say whether it shows. Not attempted, not claimed.
3. **Frame-side firmware was touched** (`7563ca0`) although the card said camera
   firmware only. It is the bench support that made rung 2 possible with no card
   in the reader: DWELL_MS 10 s → 15 s, fold+mount retried up to six times (the
   workaround 0239/0245 established for this card), and a 0.8 s black gap between
   frames so the sequence is countable on screen. No .BIN handling, no ESP-NOW, no
   photo path in that diff. Declared as a scope extension; droppable on its own.
4. **The DAC test-pattern build is not in the tree.** The frames the meter was
   read against were painted by a temporary build; the committed, reproducible
   path is the 12 `.BIN` frames under `bench/dac-cal/` with `SHA256SUMS` and the
   written procedure. So the measurement is reproducible with a card, not from
   the exact binary that produced it.
5. **The task file was truncated by the earlier session** (`ec1175a` replaced 268
   lines with 9). The record in this file is rebuilt from that session's
   transcript, the rung-1 commit messages and the board itself — not from the
   note that survived.
6. **The frame board's SD slot failed after the measurement** — every mount fails
   now, including the archived 0249 binary that mounted the same card at noon. That
   is a physical fault on the card/holder and is unrelated to this change, but it
   is the state of the bench as this task closes.
