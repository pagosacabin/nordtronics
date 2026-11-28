---
task_id: "0281"
status: in_progress
iteration: 2
proof: []
notes: "Filed 2026-11-27 on Stephen's word ('File them now'). Follow-up to 0278 (slideshow lock after shoot, resolved as an ill-seated SD card). The zero-length LIVE0054.BIN is a leftover from that incident."
expect-reply-within: 24h
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
