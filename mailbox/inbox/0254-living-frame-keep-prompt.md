---
task_id: "0254"
status: inbox
iteration: 0
proof: []
notes: ""
expect-reply-within: 24h
---

# 0254 — Living frame: keep-or-discard prompt after each photo

## Assignment

- **Hermes — firmware + bench (this task):** implement the keep-or-discard confirm flow on the frame (below), verify it on the bench with the real frame and camera, and push the branch. Every step below is yours.
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Stephen — bench side:** nothing needed from him to build this; the button-press checks below are yours to run at the bench, and he will press the button when asked.

## Context

- Device: living frame, YD-ESP32-S3 N16R8, VGA DAC, SD card holds the LIVE library (39 frames as of Nov 25 11:39). Button is GPIO14. Camera is the Freenove ESP32-WROVER CAM (GC0308), linked to the frame over ESP-NOW channel 6.
- Current flow (verified in 0244/0245): button press → SHOOT sent → hash verified → stored (library N→N+1) → shown. There is no way to reject a bad photo: a blurry or unwanted shot goes straight into the library and comes back in the slideshow.
- Frame + camera are both live on the bench as of Nov 25 11:39 (frame streaming 640x480, SD mounted, link established both directions). The SD-slot "physical failure" verdict from Nov 23 does NOT reproduce — the card mounts fine; it was most likely not seated.
- The slideshow already tolerates holes in the numbering (0249), so removing a file from the library is safe — the show skips it.
- Stephen is working with Hermes interactively on a boot splash (embedded Nordtronics logo) in parallel. **Do not touch splash code in this task** — keep this change to the photo-capture/confirm flow only.
- The long-press delete for photos already in the library is a separate idea, NOT in scope here. This task covers only the confirm step for a just-taken photo.
- Record the outcome in the crew-vault living-frame bench notes (same area as the NAS-CW01 bench page, ~/crew-vault/bench/).

## Task

Add a keep-or-discard confirm step between shooting and storing, so a bad photo never enters the library. One deliverable: the frame firmware on branch `hermes/0254-keep-prompt` in cabin-play, with the flow below working on the bench.

1. After a button press and a successful shoot, hold the new photo WITHOUT storing it (RAM/temp buffer — do not write it to the SD library yet) and display it with a "Keep this picture?" prompt below it. The prompt must state on screen how to answer.
2. Short press = Yes (keep): store the photo to the SD library (library N→N+1, same hash verification as today: stored sha == sent sha), then resume the slideshow showing it.
3. Hold ~2 seconds = No (discard): do not store it (library count unchanged), return to the slideshow.
4. No answer within 30 seconds = keep: store it (library N→N+1) and resume the slideshow. The timeout must NEVER discard — a photo is only lost by an explicit hold, never by walking away.
5. A press during the prompt must answer the prompt, never trigger a second shoot.
6. Text rendering is your choice: a small bitmap font in firmware, or the prompt pre-rendered as an image asset (like the splash logo). Whichever you pick, the prompt must be legible on the panel (640x480 native) and must fit without cropping the photo or the question.
7. Verify on the bench with the real frame and camera: run all three cases (Yes stores, No discards, timeout keeps) and record the library count before/after each.

## Success criteria

- After every successful shoot, the new photo displays with "Keep this picture?" and the answer instructions visible below it.
- Short press stores the photo (library N→N+1, stored sha == sent sha) and the slideshow resumes showing it.
- A ~2s hold discards the photo (library count unchanged) and the slideshow resumes.
- No answer for 30s stores the photo (library N→N+1) and the slideshow resumes.
- No photo is written to the SD library before confirmation (or the timeout keep).
- No regression: the slideshow, the existing library, and the shoot→hash→store verification all still work as before.

## Constraints

- One button only (GPIO14). No new hardware.
- Never store a photo before the user confirms it or the timeout keeps it.
- Never discard on timeout — timeout always keeps.
- Do not touch the boot-splash code (in flight interactively).
- Do not break the existing shoot→hash→store verification chain.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Branch `hermes/0254-keep-prompt` pushed to origin in cabin-play, tip SHA quoted in the staged reply.
- Bench run log for all three cases (Yes / No / timeout) with the library count before and after each, quoted in the staged reply.
- Copilot peer-review result quoted in the staged reply.

## Reply format

Stage the reply in this file with these fields filled in:

- `branch`: hermes/0254-keep-prompt
- `tip_sha`: commit SHA on origin
- `prompt_shows`: yes / no + one line (what the prompt looks like on the panel)
- `text_rendering`: how the prompt text is drawn (bitmap font / pre-rendered asset / other)
- `yes_stores`: library count before → after
- `no_discards`: library count before → after
- `timeout_keeps`: library count before → after (and the timeout used, seconds)
- `shoot_not_retriggered`: yes / no + one line (press during prompt answers, never shoots again)
- `regressions_checked`: slideshow / existing library / shoot-hash-store — each pass / fail + one line
- `copilot_review`: result quoted
- `notes`: anything unexpected (timing that needed tuning, legibility trade-off, splash interaction)
