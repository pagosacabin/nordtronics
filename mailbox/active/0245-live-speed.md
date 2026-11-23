---
task_id: "0245"
title: Live photo flow speed-up — show first/store second, SD clock ladder, ESP-NOW pacing
status: in_progress
iteration: 1
expect-reply-within: 6h
proof: []
notes: Filed by Juno 2026-11-23 at Stephen's direction. Builds on 0244 (closed and verified the same morning). Amended the same morning at Stephen's direction: this task runs as a supervised Hermes goal-mode run (see Goal-mode run, below). CLAIMED INTERACTIVELY 2026-11-23 ~11:05 MST as a goal-mode run; the four cronrunner worker jobs (c0be50a686c6, 7aff6948c2c1, 8b1c9e1323c5, 5c1532977f15) are PAUSED for the duration and must be resumed on staging.
---

## Assignment

- **Hermes:** all bench work, both firmwares as needed, the repo landing. Push cabin-play via SSH from the laptop (the gh.py credential is 403 on cabin-play writes).
- **Copilot:** peer review of the cabin-play changes, driven by Hermes via Herdr (GitHub-bound work rule). Reviewer, not the proof.
- **Newton:** not assigned — firmware stays with Hermes.
- **Juno:** verifies independently on the staged reply — branch SHA on origin, repo paths, no secrets in the commit, the timing and ladder evidence.

## Goal-mode run (Stephen's direction, 2026-11-23)

This task runs as a **supervised interactive `/goal` run**, not a normal worker pickup — the second goal-mode run after the 0235 trial, and the first on a real deliverable.

- **Pickup:** Hermes claims this task in the interactive session before a worker tick takes it. Pause the four worker jobs for the duration, as with 0244, and resume them on staging.
- **The contract is the Success criteria section below, verbatim.** The goal is met when every criterion holds with its evidence, the Copilot findings are addressed, and the staged reply is written to the mailbox. Work the three rungs in order, taking as many turns as the ladder needs — do not stop between rungs to wait for Stephen; he is supervising, not starting each leg.
- **Flashing is in scope.** Unlike 0235's host-only job, this task requires flashing both boards and running the bench flows. Stephen is at the laptop for the run.
- **The judge's DONE is not the proof** (0235's lesson: the judge judges the claim, not the artifact). Copilot's review and Juno's independent verification still sit on top of the goal's own gate, exactly as in the Assignment block.
- **Gates can't stop a mid-turn action** (0235's other lesson), so the constraint lines below bind from the first turn: settle-lower-rather-than-flaky, no out-of-scope changes, secrets out of the repo.

## Context

0244 is closed and verified: the button flow works end to end — press, 3-2-1 countdown on the panel, camera shot, frame stored to the frame board's SD library, cycling resumes. Stephen's video of a press (2026-11-23) shows the countdown's "1" sitting on the panel for ~14 s before the photo lands, and he asked for that part to be faster.

Where the ~14 s goes, from the boards' own logs in 0244:

- Trigger + camera warm-up (3 discarded frames) + capture: ~1 s.
- ESP-NOW transfer: 1,280 packets in 6,881 ms (~47 KB/s) — the camera log's own number.
- The SD store gates the display: the live loop runs SHA → SD store → display → ack, and storing 307,200 B over the SD's 400 kHz SPI clock is ~6.1 s of raw clock time before overhead. The photo is in PSRAM and verified, but nobody sees it until the write finishes.

The 400 kHz clock is a leftover from the SD diagnosis chain (0236–0241), settled before the 0239 sacrificial-frame fold was proven. The fold is proven now, and the mount retry loop already carries this card's cranky cold starts (4–6 attempts observed across 0244's boots, all ending mounted).

## Task

Three rungs, in this order, each proven before the next:

1. **Show first, store second** (`n16r8-live`). When a frame completes and its SHA verifies, publish it to the panel immediately; the SD store and the camera ack follow. Keep the proof lines identical in content (sent sha, stored sha, filename, library count) — only their timing relative to the display changes. The ack must still mean the frame is stored: if a store fails after display, log it plainly as a store failure (shown but not kept) and do not ack it.
2. **SD clock ladder** (`n16r8-live`). Step the SD SPI clock 400 kHz → 1 MHz → 4 MHz → 8 MHz → 16 MHz, keeping the 0239 fold and the mount retry loop. At each rung: at least 3 cold boots with mount behaviour inside the existing retry envelope (≤6 attempts), one full press flow, and hash-exact verification (stored sha == sent sha, plus a read-back hash of the written file). Settle at the highest rung with zero hash failures and no mount regression; if a rung regresses, settle one below it and say so. The shipped build carries the settled rate.
3. **ESP-NOW pacing** (`freenove-cam` sender, receiver as needed). Measure the current per-frame transfer time from the logs first. Then tighten whatever is actually pacing it — an inter-packet delay if one exists, and the ESP-NOW PHY rate (`esp_wifi_config_espnow_rate` or the equivalent on each end) if the stack exposes it. Bar: at least 5 consecutive full frames at the new pacing with fail 0 and every frame's SHA ack matching. Anything that costs reliability comes back out. Report before/after transfer ms either way, including if it doesn't yield.

**Instrumentation:** log millis at SHOOT-sent and at display-publish on the frame board, so the post-countdown delay is a number in the report partition rather than an estimate. Report it for at least 3 presses on the final build.

## Success criteria

- Post-countdown delay (SHOOT sent → photo published on the panel) ≤ 5 s on the final build, from the logged millis over ≥3 presses (baseline ~13–14 s). The countdown itself is unchanged: 3 steps at ~1 s.
- Every press still grows the library by exactly one file; stored sha == sent sha on every run; reboot persistence still holds.
- The reply carries the SD ladder table: per rung — clock, boots, mount-attempt range, hash results — and names the settled rate with its evidence.
- ESP-NOW: before/after transfer ms over ≥5 frames with fail 0, or a plain statement of what was tried and why it didn't yield.
- No 0244 regressions: presses during countdown/transfer still ignored (release-edge rule intact), session-id and chunk-coverage gating intact, free-space preflight intact, camera still keeps no library.

## Constraints

- Branch `hermes/0245-live-speed` on pagosacabin/cabin-play, based on f10353ba996e1296181aceea6b81ee476e8e2d1b (the 0244 branch tip — the live projects are not merged to main yet). Paths: `living-frame/firmware/n16r8-live/`, `living-frame/firmware/freenove-cam/`.
- Out of scope: the countdown duration; WB_STRENGTH (Stephen has not called the 0.35 change); the report-partition erase structural fix (its own future task, per 0244 Deviation 4); the `n16r8-slideshow` build (it can adopt the proven SD rate in its own later change).
- Copilot peer review of the changes; address findings; then push. `git fetch` and read origin before any push.
- `secrets.h` and any WiFi credentials stay OUT of the repo. The repo is public.
- Start small, proven rungs: a faster-but-flaky build is a failure, not a trade-off. Settle lower and report rather than ship a regression.
- Model cost: flash tier is fine for this work.

## Proof

- Branch `hermes/0245-live-speed` on origin at the reported SHA (Juno checks).
- Report-partition quotes: the timing lines (SHOOT-sent and display-publish millis) and the store lines for the timed presses.
- Camera send logs: before/after transfer ms and fail counts.
- The SD ladder table from the bench runs.

## Reply format

Status line first. Then per rung: what changed and the numbers it produced. Then the proof mapping (repo / branch / sha / files / review / bench). Deviations declared plainly, including any rung that didn't yield.
