---
task_id: "0258"
status: inbox
iteration: 0
proof: []
notes: ""
expect-reply-within: 24h
---

# 0258 — Living frame: rotate all text displays to portrait

## Assignment

- **Hermes — firmware + bench (this task):** rotate the frame's text displays to portrait (below), verify each one on the panel, and push the branch. Every step below is yours except button presses during verification, which are Stephen's.
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Stephen — bench side:** press/hold the button during the verification runs when Hermes says ready.

## Context

- Stephen's idea, Nov 25. The photos display in portrait, but every text display on the frame was built as a landscape strip, so the text reads sideways next to the photos. All text needs rotating 90 degrees so it reads the same way up as the photos.
- Text elements in scope: the keep prompt strip ("Keep this picture? / press = keep / hold 2 s = discard"), the discard notice ("That one's gone. / Press the button to take another."), the camera-not-responding notice ("Camera not responding. / Press the button to try again."), the countdown digits (3-2-1), and the deleting notice ("Deleting… / Image being deleted").
- OUT OF SCOPE: the boot splash / Nordtronics logo. Stephen confirmed it displays perfectly — do not regenerate, rotate, or otherwise modify the splash asset.
- The text elements are pre-rendered strip assets (e.g. prompt_strip.h 640x80, notice_deleted.h 640x140) made through the same converter path as the photos (RGB222 packing/gamma). Rotating means regenerating the assets rotated and re-laying them out, or rotating at blit time — your choice, whichever keeps the text legible.
- Branch from `hermes/0257-silent-delete` so all the prompt/notice machinery is present (if the latest frame branch differs when picked up, base on that one and say so in the reply). **Do not touch splash code or the splash asset.**
- Record the outcome in the crew-vault living-frame bench notes (bench/frame-storage-and-link.md).

## Task

Rotate every text display on the frame 90 degrees to portrait. One deliverable: the frame firmware on branch `hermes/0258-portrait-text` in cabin-play, with every text element reading upright in portrait on the bench.

1. Rotate the keep prompt strip, the discard notice, the camera-not-responding notice, the countdown digits, and the deleting notice so each reads upright in portrait, the same way up as the photos. The rotation direction (clockwise vs counter-clockwise) must match the photos' portrait orientation — determine it from the panel, do not assume it.
2. Re-lay out each element so the text is fully legible on the panel: not clipped, not cropped, not overlapping the photo it belongs with.
3. Verify on the bench by showing each text element on the panel and confirming it reads upright: take a photo (keep prompt + countdown), discard a photo (discard notice), unplug the camera and press once (camera notice), and delete a fresh throwaway shot taken during the test (deleting notice — use the fresh shot so no real library photo is lost).
4. Confirm the splash/logo is unchanged.

## Success criteria

- Every text display reads upright in portrait, same orientation as the photos: keep prompt, discard notice, camera notice, countdown digits, deleting notice.
- Each element is fully legible on the panel: no clipping, no cropping, no overlap with its photo.
- Splash/logo untouched.
- No regression: the 0254 keep/discard/timeout-keep flows, the 0256 camera notice behavior, and the 0257 silent delete all still work as before.

## Constraints

- Splash/logo untouched — do not regenerate or modify the splash asset or splash code.
- Rotation direction must match the photos' portrait orientation, verified on the panel.
- Reuse the existing converter/asset path where possible. No new hardware.
- One button only (GPIO14). No behavior changes beyond orientation — same prompts, same timings, same gestures.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Branch `hermes/0258-portrait-text` pushed to origin in cabin-play, tip SHA quoted in the staged reply.
- Bench verification for each text element (which ones shown on the panel, orientation confirmed upright), quoted in the staged reply.
- Copilot peer-review result quoted in the staged reply.

## Reply format

Stage the reply in this file with these fields filled in:

- `branch`: hermes/0258-portrait-text
- `tip_sha`: commit SHA on origin
- `rotation_direction`: clockwise / counter-clockwise + one line on how it was determined (matched to the photos on the panel)
- `keep_prompt_portrait`: yes / no + one line
- `discard_notice_portrait`: yes / no + one line
- `camera_notice_portrait`: yes / no + one line
- `countdown_portrait`: yes / no + one line
- `deleting_notice_portrait`: yes / no + one line
- `legibility`: any clipping / cropping / overlap found (none, or what and where)
- `splash_untouched`: yes / no + one line
- `regressions_checked`: keep / discard / timeout-keep / camera notice / silent delete / splash — each pass / fail + one line
- `copilot_review`: result quoted
- `notes`: anything unexpected (asset sizes, layout trade-offs, direction surprises)
