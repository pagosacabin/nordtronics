---
task_id: "0282"
status: filed
iteration: 1
proof: []
notes: "Filed 2026-11-27 on Stephen's word ('File them now'). Follow-up to 0277 (SD card error display). Hermes acknowledged this gap honestly in the 0277 staged reply and left it open: the error is 'cannot contact the card,' not 'no photos.'"
expect-reply-within: 24h
---

# 0282 — Empty-but-readable card: show a "no photos" notice, not the SD error (0277 follow-up)

## Context

0277 added the SD card error display: no card (or an unreadable card) ->
splash, then "SD card not found / Check the card, then power cycle" and it
holds. That covers "cannot contact the card."

What it does NOT cover: a card that mounts fine but has NO photos on it
(empty library). In that case the frame holds the splash (or, on a
remount, shows a stale "no SD card" notice — Juno's review finding #1 on
0277). The card is fine; the library is just empty. The frame should say
so plainly, with a distinct notice, instead of implying the card is broken
or hanging on the splash.

## Task

1. Bench up: frame on the proto board (tested and working Nov 27), SD
   card in. Confirm the current build on the board and the branch it came
   from before touching anything; do not re-flash a board that already
   runs the current build unless the change requires it.
2. In the living-frame firmware (n16r8-live, living-frame/firmware/
   n16r8-live/src/live.c + notice_sd.h from 0277), add the empty-library
   state: when the card mounts successfully but the library scan finds
   zero photos, show a distinct "no photos" notice (for example "No photos
   on card / Add photos, then power cycle") and hold it — do NOT show the
   SD error notice, and do NOT hold the splash. The notice must make it
   clear the card is readable and the problem is only that it is empty.
   Keep the 0277 behaviour unchanged for the other two states: no card /
   unreadable card -> the SD error notice; card with photos -> normal
   boot and slideshow.
3. Verify on the bench, all three states, same card, same build:
   - Empty but readable card (no photos) -> the "no photos" notice, held.
   - Card with photos -> normal boot, slideshow runs.
   - No card -> the 0277 SD error notice ("SD card not found / Check the
     card, then power cycle").
   Confirm the 0261 mirror fix still holds (text reads correctly).
4. Work on a new branch hermes/0282-empty-card-no-photos. Commit to
   cabin-play (play project — never nordtronics). Push via SSH; gh.py
   cannot write to cabin-play. No repo merge (merges need Stephen's word).

## Success criteria

- Empty but readable card -> the distinct "no photos" notice, held (not
  the SD error, not a held splash).
- Card with photos -> normal boot and slideshow.
- No card -> the 0277 SD error notice, unchanged.
- The 0261 mirror fix still holds.
- Branch hermes/0282-empty-card-no-photos on cabin-play with the fix.

## Constraints

- Bench-only task: frame + SD card must be on the bench. Do not run this
  while the bench is down.
- Do not touch the splash, the stored library, or convert_photo64.py.
- Do not change the DAC ladder tables, gamma, or the WB/saturation
  knobs — 0280 owns colour.
- Open lane (frame display): generic review only, not the locked-lane
  checklist. Copilot's monthly quota is exhausted until Nov 30 at 5 PM —
  use Antigravity (Claude Sonnet 4.6) as the substitute reviewer (as 0275
  did), and record that a Copilot pass over the new commits is owed when
  the quota resets Nov 30.

## Proof

- Bench log: all three states (empty card, card with photos, no card)
  with the notice shown in each, plus the mirror check.
- Branch/commit on cabin-play + review verdict (Antigravity substitute;
  Copilot pass owed Nov 30 recorded).

## Reply format

Short report: the three bench states and what each showed, branch/commit,
review verdict.

## Assignment

Hermes does the bench work himself. No Newton (firmware is Hermes's).
Antigravity substitutes for the Copilot review until the quota resets;
a Copilot pass is owed Nov 30. Juno verifies independently.
