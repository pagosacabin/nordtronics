---
task_id: "0281"
status: staged
iteration: 2
claimed_by: "cronrunner/8b1c9e1323c5"
claimed_at: "2026-11-28T13:00:20+00:00"
proof:
  branch: hermes/0281-skip-zero-length-files
  repo: pagosacabin/cabin-play
  sha: d46a6967a251315ea2f6f4c784bc0858e8a64e67
  base: "hermes/0277-sd-error @ 9bbdef8 (0261's mirror fix + 0277's SD-error path); one commit on top"
  run: none
  run_note: "cabin-play has no CI at all: the branch tree d46a696 carries 0 entries under .github/ (`git ls-tree -r --name-only d46a696 | grep -c '^.github/'` -> 0), `gh api repos/pagosacabin/cabin-play/actions/runs --jq .total_count` -> 0, and `gh run list -R pagosacabin/cabin-play --branch hermes/0281-skip-zero-length-files` is empty. There is no workflow to trigger and no green run to cite. No ntfy receipt either: the task names no topic and the deliverable is a bench-flashed firmware image, not a published build artifact."
  files:
  - living-frame/firmware/n16r8-live/src/live.c
  build: "pio run -e n16r8-photo SUCCESS; uploaded to /dev/ttyUSB1 with 'Hash of data verified'; sha256 583eeecb76b04a115487ee8f182be39867b1b090cb5d26b45450864bdbcfcde0; read back with `esptool verify_flash 0x10000` -> 'verify OK (digest matched)'"
  applied_to:
  - "frame board (YD-ESP32-S3 N16R8, VGA DAC, /dev/ttyUSB1 FT232): deliverable image flashed, digest re-read and matched; boots 'build: Nov 28 2026 06:02:06'"
  bench:
    before: "0277 build on the board: 'SD library holds 39 frames, next index 58'; every cycle burned one position on the empty file -- 'show 48 (shuffled 37/39)' -> 'skip 54 (no such file)' -> 'show 53 (shuffled 39/39)', 3 skips in a 4-cycle 143-show capture, shuffle denominator 39"
    after: "0281 build, three separate boots: 'SD library holds 38 frames, next index 58 in /sdcard/LIVE' each time, 0 skip lines, shuffle denominator 38, positions contiguous 1..35/38 per capture (the report is RAM-buffered and a capture holds ~35 show lines), and the union of the three boots is 38 distinct indices = live files, index 54 never shown"
  mirror: "checked live, mechanism only: the 0261 fix is in the camera firmware, which this branch does not touch; the camera board still carries 0280's build (esptool verify_flash digest matched) and its boot line reads 'sensor : power-on state hmirror 1, vflip 0 (reg 0x14) / sensor : after fix hmirror 0, vflip 0 (reg 0x14), write ok'. The visual half ('text reads correctly') was NOT judged -- see notes."
  sd_error: "NOT re-run: proving the 0277 no-card notice needs the card physically out of the cased frame and the worker has no hands on the bench. The code path is untouched -- the whole diff is 21 insertions and 1 deletion inside live_scan()/order_build() plus the new helper."
  review: "NONE AVAILABLE. Copilot CLI answers 'You have exceeded your monthly quota (Request ID: 3FE0:565B1:72589D5:7C72821:6B0AD8CA)'; Antigravity (the task's named substitute) reports both pools at 0% remaining, Gemini until 2026-12-04T15:02:05Z and Claude/GPT until 2026-12-04T20:13:28Z. No verdict exists, so none is claimed; a Copilot pass is owed when the monthly quota resets Nov 30 (the task says 5 PM) and Juno may review in the meantime, as she did for 0277."
  evidence:
  - mailbox/files/0281/bench-before-0277-build.txt
  - mailbox/files/0281/bench-after-fix-cycle.txt
  - mailbox/files/0281/analysis.txt
  - mailbox/files/0281/flash-and-board-verify.txt
  - mailbox/files/0281/camera-console.txt
expect-reply-within: 24h
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
  rates_used: unknown
  cost_usd: unknown
notes: |
  DONE AND BENCH-VERIFIED on the point the task exists for, with two criteria
  not re-run and no independent review.  All three gaps are declared below.

  THE CHANGE (one commit, one file, +21/-1):
    repo/branch  pagosacabin/cabin-play, hermes/0281-skip-zero-length-files
    base         hermes/0277-sd-error @ 9bbdef8
    tip          d46a6967a251315ea2f6f4c784bc0858e8a64e67 (git ls-remote
                 --heads origin -> same sha)
    file         living-frame/firmware/n16r8-live/src/live.c
      - new live_file_empty(): stat()s an entry and returns true only when the
        size is positively zero (a failed stat() returns false, so an entry that
        cannot be sized is counted exactly as before)
      - live_scan(): skips such an entry, so it is not counted and not folded
        into s_lib_max
      - order_build(): skips it too, so it never enters the shuffle
    Nothing else moves: no change to live_load, live_store, the 65535 cap, the
    keep/discard prompt, the splash, the DAC ladder, gamma or WB/saturation.

  BENCH (frame on /dev/ttyUSB1, card in, three boots of the deliverable):
    before (0277 build that was on the board, build stamp Nov 27 13:45:32):
      SD library holds 39 frames, next index 58
      show 48 (shuffled 37/39) / skip 54 (no such file) / show 53 (shuffled 39/39)
      -> 3 skips in a 143-show, 4-cycle capture; denominator 39
    after (0281 build), boots 1-3:
      SD library holds 38 frames, next index 58  (39 - 1, the card unchanged)
      0 skip lines; denominator 38; positions contiguous 1..35/38
      union of the three boots = 38 distinct indices, exactly the library count;
      index 54 never appears
    Read the count change plus the unbroken 1..35/38 as the proof: the empty
    file left the library (39 -> 38), the shuffle no longer contains it
    (denominator 38), and no position is missing (the old build always showed a
    one-position gap where the skip line went).

  WHY EACH CAPTURE IS A FULL CYCLE (~400-450 s): the report log is buffered in
  RAM (6000 B, flushed at boot and on photo/button events), so a capture taken
  ~90 s after a reset held only the boot record -- 0 show lines -- while a
  ~7-minute window held a contiguous 1..35/38.  Both were reproduced more than
  once, on the base build as well, so this is the logging path, not the fix.
  A single 38-photo cycle therefore cannot appear in one capture; the union of
  the three boots is what covers every file.

  NOT VERIFIED, and why:
   1. 0277's no-card notice: needs the card out of the cased frame.  No hands.
      The path is untouched by the diff (see above) and the frame booted
      normally three times with the card in.
   2. 0261 mirror, visual half: "text reads correctly" needs eyes on the panel
      or a capture path, and this worker has neither.  The mechanism IS proven
      live: the camera carries 0280's build (digest matched) whose boot line
      reports 'after fix hmirror 0, vflip 0, write ok'.
   3. Independent review: Copilot monthly quota exceeded (Request ID quoted in
      the proof block), Antigravity both pools 0% until 2026-12-04.  A Copilot
      pass is owed when the quota resets Nov 30 per the task's own constraint.

  DEVIATIONS, declared:
   - Two flashes, not one: the base build was flashed mid-session as a control
     when a 90 s capture showed no show lines, to prove the fix was not
     responsible.  The board now carries the deliverable (digest matched).
   - I reset the camera board (classic DTR/RTS ladder) to read its mirror boot
     line; it re-linked to the frame immediately.  No file on Stephen's card was
     created, deleted or modified -- the firmware only stat()s entries.
   - Consequence worth knowing, not exercised here: if an empty file is the
     highest index, s_lib_max no longer counts it, so the next stored photo
     overwrites that index and the stray file self-heals.  No shoot was taken
     in this task (0280 owns colour and the shoot flow).

  cost is 'unknown': this harness does not expose per-call API usage, so no
  token or dollar figure is estimated here.
---


# 0281 — Skip zero-length files in the slideshow library scan (0278 follow-up)

## Context

0278 resolved the slideshow lock after a shoot as an ill-seated SD card
(contact failure: a write failed, reads skipped files the directory still
listed). One leftover from that incident: a zero-length LIVE0054.BIN may
still sit on the card. It is harmless, but the slideshow counts it once
and skips it once per cycle, so the log shows "skip 54" every cycle until
Stephen pulls the card and deletes that file by hand.

The fix belongs in the firmware, not in Stephen's hands: the library scan
should ignore zero-length files entirely, so a stray empty file never
shows up as a skip and never inflates the library count.

## Task

1. Bench up: frame on the proto board (tested and working Nov 27), SD
   card in. Confirm the current build on the board and the branch it came
   from before touching anything; do not re-flash a board that already
   runs the current build unless the change requires it.
2. In the living-frame firmware (n16r8-live, living-frame/firmware/
   n16r8-live/src/live.c — the library scan / slideshow), make the scan
   skip zero-length files: a file with size 0 is not counted in the
   library, not added to the shuffle/cycle, and does not emit a "skip"
   log line. The file itself stays on the card (Stephen deletes it when
   he pulls the card); the firmware just ignores it.
3. Verify on the bench with a zero-length file present on the card
   (create one if LIVE0054.BIN is already gone): run a full slideshow
   cycle and confirm no "skip" line appears for the empty file, the
   library count excludes it, and normal files still play in order.
   Confirm the 0261 mirror fix still holds (text reads correctly) and
   the 0277 SD error display still works (no card -> the SD error notice,
   not a crash).
4. Work on a new branch hermes/0281-skip-zero-length-files. Commit to
   cabin-play (play project — never nordtronics). Push via SSH; gh.py
   cannot write to cabin-play. No repo merge (merges need Stephen's word).

## Success criteria

- With a zero-length file on the card, a full slideshow cycle shows no
  "skip" line for it, the library count excludes it, and all normal
  files play.
- The 0261 mirror fix still holds and the 0277 SD error display still
  works (no card -> SD error notice).
- Branch hermes/0281-skip-zero-length-files on cabin-play with the fix.

## Constraints

- Bench-only task: frame + SD card must be on the bench. Do not run this
  while the bench is down.
- Do not delete files from Stephen's card. Do not touch the splash, the
  stored library, or convert_photo64.py.
- Do not change the DAC ladder tables, gamma, or the WB/saturation
  knobs — 0280 owns colour.
- Open lane (frame display): generic review only, not the locked-lane
  checklist. Copilot's monthly quota is exhausted until Nov 30 at 5 PM —
  use Antigravity (Claude Sonnet 4.6) as the substitute reviewer (as 0275
  did), and record that a Copilot pass over the new commits is owed when
  the quota resets Nov 30.

## Proof

- Bench log: a full slideshow cycle with a zero-length file present,
  showing no "skip" for it, the library count, and normal files playing.
- Branch/commit on cabin-play + review verdict (Antigravity substitute;
  Copilot pass owed Nov 30 recorded).

## Reply format

Short report: bench log summary (cycle clean, count, mirror check, SD
error check), branch/commit, review verdict.

## Assignment

Hermes does the bench work himself. No Newton (firmware is Hermes's).
Antigravity substitutes for the Copilot review until the quota resets;
a Copilot pass is owed Nov 30. Juno verifies independently.


# Reply — 0281: an empty LIVE file is no longer counted, ordered, or skipped

**Status: DONE and bench-verified. Two criteria not re-run and no review verdict — all declared.**

| Field | Value |
|---|---|
| `branch` | `hermes/0281-skip-zero-length-files` |
| `sha` | **d46a6967a251315ea2f6f4c784bc0858e8a64e67** |
| `base` | `hermes/0277-sd-error` @ 9bbdef8 |
| `files` | `living-frame/firmware/n16r8-live/src/live.c` (+21/-1) |
| `review` | **none available** — see the last section |

## Bench log, before and after

The build that was on the board (step 1: confirmed, not assumed — read back and
byte-compared against a build of the base tree; 92 bytes differ in 1.5 MB and all
of them are the version string `b5c0f52-dirty`, the `Nov 27 2026 13:45:33`
stamp and a segment hash, i.e. the 0277 tree built 13:45 that day):

```
live : SD library holds 39 frames, next index 58 in /sdcard/LIVE
show 48 (shuffled 37/39)
skip 54 (no such file)
show 53 (shuffled 39/39)
```

Three skips in one 143-show, four-cycle capture, always at the same place: the
empty `LIVE0054.BIN` took a position in the order and burned it.

The deliverable, three separate boots:

```
live : SD library holds 38 frames, next index 58 in /sdcard/LIVE
show 53 (shuffled 1/38) ... show 56 (shuffled 35/38)      <- contiguous, no gap
```

* library count **39 → 38** with the card untouched: the empty file is no longer
  counted;
* shuffle denominator **39 → 38**: it is no longer ordered;
* **0 skip lines** in all three captures, and no one-position gap where the skip
  used to sit;
* the union of the three boots is **38 distinct indices** — exactly the library
  count — and index 54 never appears;
* every shown index has a matching `swap ... mixed lines 0`, so the normal files
  still play at the 10 s cadence.

A capture taken ~90 s after a reset holds only the boot record (the report log is
RAM-buffered and flushed at boot and on photo/button events); ~7-minute windows
hold a contiguous 1..35/38. That is why a single 38-photo cycle cannot appear in
one capture, and why the coverage above is the union of three boots.

## Not verified, declared

1. **0277's no-card notice** — proving it needs the card physically out of the
   cased frame; the worker has no hands on the bench. The path is untouched: the
   whole diff is the new helper plus two call sites inside `live_scan()` and
   `order_build()`. The frame booted normally three times with the card in.
2. **0261 mirror, visual half** — "text reads correctly" needs eyes on the panel
   or a capture path. The mechanism is proven live: the camera board still
   carries 0280's build (`esptool verify_flash` digest matched) and its boot line
   reads `sensor : after fix hmirror 0, vflip 0 (reg 0x14), write ok`.
3. **Review** — Copilot CLI: *"You have exceeded your monthly quota"* (Request ID
   quoted in the proof block). Antigravity, the task's named substitute: both
   pools at 0 % remaining, Gemini until 2026-12-04T15:02:05Z and Claude/GPT until
   2026-12-04T20:13:28Z. **No verdict exists, so none is claimed.** A Copilot
   pass is owed when its monthly quota resets Nov 30 per the task's own
   constraint; Juno may review in the meantime, as she did for 0277.

## Deviations

* Two flashes: the base build went on briefly as a control after a short capture
  showed no show lines, to prove the fix was not the cause. The board now carries
  the deliverable and its digest was read back and matched.
* The camera board was reset (DTR/RTS ladder) to read its mirror boot line; it
  re-linked to the frame at once.
* No file on Stephen's card was created, deleted or modified — the firmware only
  `stat()`s entries, so `LIVE0054.BIN` is still there for him to delete when he
  next pulls the card.
* Consequence, not exercised here: when an empty file is the highest index,
  `s_lib_max` no longer counts it, so the next stored photo overwrites that
  index and the stray file self-heals. No shoot was taken — 0280 owns colour and
  the shoot flow.
