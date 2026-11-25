---
task_id: "0257"
status: inbox
iteration: 0
proof: []
notes: ""
expect-reply-within: 24h
---

# 0257 — Living frame: silent hold-to-delete for the photo on screen

## Assignment

- **Hermes — firmware + bench (this task):** implement the silent hold-to-delete below, verify it on the bench with the real frame, and push the branch. Every step below is yours except button presses during verification, which are Stephen's.
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Stephen — bench side:** hold/press the button during the verification runs when Hermes says ready.

## Context

- Follow-up to 0254 (keep-or-discard prompt) and 0256 (camera-not-responding notice), Stephen's idea, Nov 25. Photos already in the library can currently only be removed by pulling the SD card. This adds a way to delete the photo on screen, as a silent menu: nothing on screen advertises it — only people who know the gesture know it exists.
- Reuses the 0254 confirm-prompt and notice machinery (pre-rendered prompt strip + notice assets, same as the keep prompt and the discard notice).
- Branch from `hermes/0256-camera-notice` so the notice machinery is present (if the latest frame branch differs when picked up, base on that one and say so in the reply). **Do not touch splash code.**
- Record the outcome in the crew-vault living-frame bench notes (bench/frame-storage-and-link.md).

## Task

Add a silent hold-to-delete for the photo currently on screen. One deliverable: the frame firmware on branch `hermes/0257-silent-delete` in cabin-play, with the behavior below working on the bench.

1. While the slideshow is showing a photo (not during the keep prompt), a 2-second hold of the button opens a delete confirm for the photo on screen. The hold itself is unadvertised — nothing on screen hints the gesture exists.
2. The confirm prompt shows the photo with "Delete this picture?" below it and states how to answer, in the same style as the keep prompt: short press = yes (delete), hold ~2 s = no (cancel). The prompt must ignore the button until it has been released after the triggering hold (the 0254 lesson — a still-held button must never be read as an answer).
3. Short press = Yes: show a brief "Deleting..." / "Image being deleted" notice, delete the file from the SD library, then return to the slideshow on the next photo.
4. Hold ~2 s = No (cancel): keep the photo, return to the slideshow on the same photo.
5. No answer within ~30 s (same as the keep prompt) = cancel: keep the photo, return to the slideshow.
6. Deleting leaves a hole in the numbering; the slideshow already tolerates holes (0249).
7. Verify on the bench: display a known throwaway photo (a fresh shot is fine), hold 2 s, confirm the delete, check the file is gone, the library count dropped by one, and the slideshow continues; also verify the cancel-by-hold and the timeout-cancel paths leave the photo in place.

## Success criteria

- A 2 s hold during the slideshow opens the delete confirm for the photo on screen; nothing on screen advertises the gesture beforehand.
- Short press deletes the file (library N→N−1), shows the deleting notice, and the slideshow continues on the next photo.
- A hold at the confirm cancels (library unchanged, same photo resumes).
- A timeout at the confirm cancels (library unchanged).
- No regression: the 0254 keep/discard/timeout-keep flows still work, the 0256 camera notice still works, and the splash is untouched.

## Constraints

- Reuse the 0254/0256 prompt + notice machinery. One button only (GPIO14). No new hardware.
- The hold alone must NEVER delete — only the confirm press deletes. Timeout never deletes.
- The confirm prompt must wait for a button release after the triggering hold before accepting an answer.
- Do not touch the boot-splash code.
- Do not change the 0254 confirm flow or the 0256 camera notice — this only adds the delete path in the slideshow state.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Branch `hermes/0257-silent-delete` pushed to origin in cabin-play, tip SHA quoted in the staged reply.
- Bench run log for all three cases (delete / cancel-by-hold / timeout-cancel) with the library count before/after each and confirmation the file is gone after a delete, quoted in the staged reply.
- Copilot peer-review result quoted in the staged reply.

## Reply format

Stage the reply in this file with these fields filled in:

- `branch`: hermes/0257-silent-delete
- `tip_sha`: commit SHA on origin
- `confirm_shows`: yes / no + one line (what the delete confirm looks like on the panel)
- `delete_works`: library count before → after + file confirmed gone (yes/no)
- `deleting_notice_text`: exact wording shown while deleting
- `cancel_by_hold`: library count before → after (unchanged) + same photo resumed (yes/no)
- `timeout_cancels`: library count before → after (unchanged) + timeout used (seconds)
- `hold_alone_deletes`: no + one line (hold without a confirm press never deletes)
- `regressions_checked`: keep / discard / timeout-keep / camera notice / splash — each pass / fail + one line
- `copilot_review`: result quoted
- `notes`: anything unexpected
