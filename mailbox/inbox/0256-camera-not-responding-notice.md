---
task_id: "0256"
status: inbox
iteration: 0
proof: []
notes: ""
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
