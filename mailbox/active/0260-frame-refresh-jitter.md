---
task_id: "0260"
status: in_progress
iteration: 1
proof: []
notes: ""
expect-reply-within: 24h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-25T22:51:15+00:00"
lease_until: "2026-11-26T00:21:15+00:00"
---

# 0260 — Living frame: eliminate refresh jitter/tearing on image swaps

## Assignment

- **Hermes — firmware + bench (this task):** find and fix the torn-frame jitter below, verify it, and push the branch. Stephen is away from the bench for this filing — implement the fix without button presses; the visual confirmation is his step when he's back (see Task 4).
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Stephen — bench side:** visual confirm on return — one boot (watch the splash→first-photo handover) plus a few button presses (prompt/notice→photo transitions).

## Context

- Stephen's video, Nov 25 ~15:18. The splash holds steady for its 6 s, then the handover to the first photo shows a torn frame — part of the screen has already switched while the rest is still catching up — and then the photo settles. The same artifact appears on button presses, when the frame goes back to a photo after a prompt or a notice (keep prompt, discard notice, camera notice, delete confirm). **Video of the symptom: `mailbox/attachments/0260-refresh-jitter.mp4` in this repo** (trimmed to the splash→photo handover, ~4.6 s; the full clip is 8.6 s). The text description here stands on its own if the video won't play.
- Likely cause (Juno's hypothesis, to be confirmed from the code, not assumed): the buffer swap happens mid-scan while the VGA DAC is reading the raster out, so one refresh is half old image, half new. Fix direction: swap only during vertical blank (or equivalent — finish the new frame before flipping, or blank during the transition — Hermes's choice, as long as the visible result is a clean swap).
- This is a timing/sync issue, not the content issue 0254 fixed with the s_hold snapshot (that was about restoring the right photo; this is about when the swap lands).
- Branch from `hermes/0258-portrait-text` (latest frame branch on origin; 0259 is filed but not yet built — if 0259 lands first, rebase onto it and say so in the reply). **Do not touch splash code or the splash asset.**
- Record the outcome in the crew-vault living-frame bench notes (bench/frame-storage-and-link.md).

## Task

Eliminate the visible jitter/tearing when the frame swaps images. One deliverable: the frame firmware on branch `hermes/0260-refresh-jitter` in cabin-play, with clean swaps on the bench.

1. Establish the actual cause from the code: where and when the active buffer is swapped relative to VSYNC/scanout. State it in the reply — do not assume Juno's mid-scan hypothesis is right.
2. Make swaps land cleanly: only during vertical blank, or by an equivalent method (complete the new frame before flipping; blank the output during the transition). The visible result must be no torn frame.
3. Instrument the swap timing if the firmware can (log swap vs VSYNC) and include that log as proof in the reply.
4. Verify visually with Stephen on his return: one boot — the splash→first-photo handover shows no tear; and button-press transitions back to a photo (keep prompt→photo, notice→photo, delete→next photo) show no tear. Stephen is away from the bench while this is filed, so implement first and confirm when he's back.

## Success criteria

- No visible tearing/jitter on the splash→first-photo transition (Stephen's visual confirm).
- No visible tearing/jitter on button-press transitions back to a photo (Stephen's visual confirm).
- Swap timing proven at/near vertical blank (code + log if instrumented).
- No regression: the 0254 keep/discard/timeout-keep flows, the 0256 camera notice, the 0257 silent delete, and the 0258 portrait text all still work as before.

## Constraints

- Splash/logo untouched — do not regenerate or modify the splash asset or splash code.
- One button only (GPIO14). No new hardware. No behavior changes beyond the swap timing.
- Stephen is away from the bench for this filing: implement without button presses; the visual confirm is his step on return. Do not block the implementation on it, but do not claim the fix verified until he has seen it.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Branch `hermes/0260-refresh-jitter` pushed to origin in cabin-play, tip SHA quoted in the staged reply.
- The cause as found in the code, quoted in the staged reply.
- Swap-timing evidence (log vs VSYNC if instrumented) and Stephen's visual confirm, quoted in the staged reply.
- Copilot peer-review result quoted in the staged reply.

## Reply format

Stage the reply in this file with these fields filled in:

- `branch`: hermes/0260-refresh-jitter
- `tip_sha`: commit SHA on origin
- `root_cause`: what the code showed (mid-scan swap / other) + one line
- `fix`: what changed (swap at blank / other) + one line
- `swap_timing_proof`: log/code evidence the swap lands at blank + one line
- `boot_transition_clean`: yes / no + one line (Stephen's visual confirm)
- `button_transitions_clean`: yes / no + one line (Stephen's visual confirm)
- `regressions_checked`: keep / discard / timeout-keep / camera notice / silent delete / portrait text / splash — each pass / fail + one line
- `copilot_review`: result quoted
- `notes`: anything unexpected
