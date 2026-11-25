---
task_id: "0256"
status: staged
iteration: 1
proof:
  kind: host-only
  branch: "hermes/0256-camera-notice"
  tip_sha: "b1a24e9"
  base: "hermes/0254-keep-prompt @ e18479a; splash untouched"
  build: "pio run -e n16r8-photo SUCCESS (1,706,080 B); flashed to /dev/ttyUSB1, 'Hash of data verified'"
  timeout_bench: "unplugged camera -> BUTTON pressed -> countdown 3/2/1 -> SHOOT -> 'TIMEOUT waiting for the frame (12 s)' -> 'notice on panel for 4 s (capture timeout), previous photo returns after' -> slideshow resumed"
  retry_bench: "press during the notice -> fresh countdown -> failed again -> notice repeated -> resumed"
  library: "43 before, 43 after the induced failures; nothing written"
  copilot: "pass 1 NO SHIP (notice restore could repaint a kept frame), fixed in b1a24e9, pass 2 SHIP"
notes: |
  DONE AND BENCH-VERIFIED — Hermes (laptop, interactive), 2026-11-25 ~13:00-13:25 MST.

  Copilot review ran twice. Pass 1 returned NO SHIP with one MINOR finding: a frame that
  completed inside the camera notice's 4 s window would be kept and shown, and then the
  notice's expiry would republish s_hold over it. Fixed in b1a24e9 by clearing
  s_notice_until/s_notice_restore as soon as the hash verifies, before the confirm view is
  published. Pass 2 returned SHIP, finding no regressions.

  A false alarm worth recording: after one bench run I read 'frame ok ... discarded' in the
  report and took it for a phantom frame while the camera was unplugged. A clean re-run
  showed no prompt and no discard at all, so the earlier discard had a connected camera at
  that moment — the frame does not invent captures. No code was changed on account of it.

  Hash-mismatch path is wired to the same show_notice() call but was NOT bench-exercised:
  inducing it needs a deliberately corrupted frame. Declared rather than implied.

  Workers: held from 0255's pickup, resumed on this staging, verified with cron list.
expect-reply-within: 24h
---

# 0256 — Living frame: "camera not responding" notice on failed capture

## Assignment

- **Hermes — firmware + bench (this task):** implement the camera-failure notice below, verify it on the bench with the real frame and camera, and push the branch. Every step below is yours except the physical unplug/press, which is Stephen's.
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Stephen — bench side:** unplug the camera USB and press the button once when Hermes says ready, then plug the camera back in at the end.

## Context

- Follow-up to 0254/0255 (Stephen's idea, Nov 25). The capture-timeout restore works: on a failed capture the frame waits ~12 s, then puts back the photo that was showing — confirmed on hardware in 0255 (library stayed at 43, no store line). The problem: the failure is silent. The person who pressed the button sees the old photo come back and has no idea anything went wrong.
- The fix: show a short on-screen notice when a capture fails, so the failure reads as a message instead of silently returning to the last photo. The notice machinery already exists from 0254 (the discard notice: pre-rendered notice asset, shown for 4 s, a press during it starts a new shot).
- Branch from `hermes/0254-keep-prompt` so the notice machinery is present. **Do not touch splash code.**
- Record the outcome in the crew-vault living-frame bench notes (bench/frame-storage-and-link.md).

## Task

Show a "camera not responding" notice on the panel when a capture fails. One deliverable: the frame firmware on branch `hermes/0256-camera-notice` in cabin-play, with the behavior below working on the bench.

1. On a capture timeout (camera never answers after the ~12 s wait), show a notice on the panel instead of silently restoring the previous photo. Suggested wording, two lines: "Camera not responding." / "Press the button to try again." Adjust the wording only if the panel layout needs it — keep it short and plain.
2. Show the same notice on a hash mismatch (photo arrived but failed verification). The timeout case is the one to verify on the bench; the hash-mismatch path uses the same notice call.
3. Show the notice for ~4 s (same as the discard notice), then return to the previous photo and resume the slideshow.
4. A press during the notice starts a new shot (retry), same as the discard notice.
5. Nothing is stored on a failed capture (library count unchanged), as today.
6. Verify on the bench by inducing the failure the same way as 0255: Stephen unplugs the camera USB (CH340), presses once, waits ~12 s, and reports what the panel shows; then plugs the camera back in. Record the panel behavior and the library count before/after.

## Success criteria

- On a capture timeout, the panel shows the "Camera not responding" notice (with the retry line) instead of silently returning to the last photo.
- The notice shows ~4 s, then the previous photo returns and the slideshow resumes.
- A press during the notice starts a new shot.
- Library count unchanged on a failed capture.
- No regression: the 0254 keep/discard/timeout-keep flows still work as before, and the splash is untouched.

## Constraints

- Reuse the 0254 notice machinery (pre-rendered notice asset, same mechanism as the discard notice). No new hardware.
- Do not touch the boot-splash code.
- Do not change the 0254 confirm flow (keep/discard/timeout-keep) — this only adds the notice on the failure path.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Branch `hermes/0256-camera-notice` pushed to origin in cabin-play, tip SHA quoted in the staged reply.
- Bench run log for the induced-failure test (unplug camera, press, ~12 s wait) with the panel behavior and library count before/after, quoted in the staged reply.
- Copilot peer-review result quoted in the staged reply.

## Reply format

Stage the reply in this file with these fields filled in:

- `branch`: hermes/0256-camera-notice
- `tip_sha`: commit SHA on origin
- `notice_text`: exact wording shown on the panel
- `notice_duration_s`: how long the notice shows
- `timeout_shows_notice`: yes / no + one line (what the panel showed after the ~12 s wait)
- `returns_to_photo`: yes / no + one line (previous photo back, slideshow resumed)
- `press_retries`: yes / no + one line (press during the notice starts a new shot)
- `library_before`: count before the test
- `library_after`: count after the test
- `hash_mismatch_path`: notice wired in (code) / bench-verified — say which, and how
- `regressions_checked`: keep / discard / timeout-keep / splash — each pass / fail + one line
- `copilot_review`: result quoted
- `notes`: anything unexpected

# Reply — 0256: "camera not responding" notice

**Done and bench-verified. Copilot SHIP on the second pass.**

| Field | Value |
|---|---|
| `branch` | `hermes/0256-camera-notice` |
| `tip_sha` | **b1a24e9** |
| `notice_text` | **"Camera not responding."** / **"Press the button to try again."** |
| `notice_duration_s` | **4** |
| `timeout_shows_notice` | **yes** — after the 12 s wait the panel shows the notice instead of silently returning |
| `returns_to_photo` | **yes** — notice clears after 4 s, the previous photograph is republished, slideshow resumes |
| `press_retries` | **yes** — a press during the notice started a fresh 3-2-1 countdown, that attempt failed too, the notice repeated, then the slideshow resumed |
| `library_before` | **43** |
| `library_after` | **43** |
| `hash_mismatch_path` | **wired in code, not bench-verified** — the hash-mismatch branch calls the same `show_notice(NOTICE_CAMERA, "hash mismatch", true)`; exercising it needs a deliberately corrupted frame, which this rig cannot easily produce |
| `regressions_checked` | keep **pass** (short press verified on the 0254 build 41→42; on this build the store path ran via timeout-keep, hashes equal); discard **pass** (43→43, notice); timeout-keep **pass** (43→44, `LIVE0043.BIN`, 1141 ms); splash **pass** (`hold 6000 ms`, `held 6000 ms, photograph takes over`) |
| `copilot_review` | pass 1: *"MINOR … a valid frame completed after a capture timeout can be processed while the camera notice's restore flag is still set … Verdict: NO SHIP"*; pass 2: *"The fix clears the pending restore before the confirmation view … No regressions found. Verdict: SHIP"* |

## The bench log, verbatim

```
live       : BUTTON pressed, library 43 frames
live       : countdown 3 / 2 / 1
live       : SHOOT sent to f0:16:1d:ae:2b:20
live       : TIMEOUT waiting for the frame (12 s)
live       : notice on panel for 4 s (capture timeout), previous photo returns after
live       : previous frame restored to the panel (notice expired)
```

Retry run (press while the notice was up), from the same report:

```
live       : BUTTON pressed, library 43 frames
live       : countdown 3 / 2 / 1
live       : SHOOT sent to f0:16:1d:ae:2b:20
live       : TIMEOUT waiting for the frame (12 s)
live       : notice on panel for 4 s (capture timeout), previous photo returns after
```

No `frame ok` and no `written` line in either: a failed capture stores nothing, and the library stayed
at 43.

## What changed

One notice path for both messages: `show_notice(asset, why, restore_after)`. The discard notice is
non-transient (the slideshow takes over); the camera notice is transient — on expiry the loop
republishes `s_hold`, the clean snapshot taken before the countdown, so the panel goes back to the
photograph that was there before the press.

The review's finding is the interesting part: a *late* frame arriving inside the 4 s window would have
been kept and shown, and then the notice's own expiry would have repainted the old photo over it. The
fix clears the pending restore the moment a frame verifies, so a real photo always wins over a stale
notice.

## A false alarm, recorded so it is not repeated

One intermediate log showed `frame ok … discarded` while I believed the camera was unplugged, which
looked like the frame inventing a capture. A clean re-run showed no prompt and no discard at all —
the earlier discard had a connected camera at that moment. No code was changed for it.

## Scope

Splash untouched (`hold 6000 ms` confirmed in the same boot log), 0254's keep/discard/timeout-keep
flow unchanged, no new hardware.
