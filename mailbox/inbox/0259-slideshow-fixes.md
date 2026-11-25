---
task_id: "0259"
status: inbox
iteration: 0
proof: []
notes: ""
expect-reply-within: 24h
---

# 0259 — Living frame: fix delete double-shoot, remove hummingbird placeholder, shuffle the slideshow

## Assignment

- **Hermes — firmware + bench (this task):** make the three changes below, verify each on the panel, and push the branch. Every step below is yours except button presses during verification, which are Stephen's.
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Stephen — bench side:** press/hold the button during the verification runs when Hermes says ready.

## Context

- Stephen's bundle, Nov 25 — three things in one change, all found while using the frame at the bench:
  1. **Delete double-shoot (bug):** long press to open the delete confirm, then a short press to delete the file — and the frame immediately tried to take a picture again. Likely cause (Juno's hypothesis, to be verified from the log, not assumed): since 0257 the shoot fires on the RELEASE of a short press, and the release after the delete-confirming press is not being consumed, so the frame reads it as a fresh shoot press. A contact bounce is the other candidate — the log should show whether there was one press or two.
  2. **Hummingbird placeholder (remove):** the image shown right after the splash is PHOTO64_IMAGE, 307,200 B compiled into the firmware, shown as the boot placeholder until the slideshow loads its first card photo (up to a 10 s dwell). It has no library index, so the delete gesture correctly refuses it — but it looks like a photo you can delete. Stephen wants it gone entirely.
  3. **Randomized slideshow:** the slideshow currently shows library photos in sequence. Stephen wants them in random order instead.
- Branch from `hermes/0258-portrait-text` so the portrait text is present (if the latest frame branch differs when picked up, base on that one and say so in the reply). **The Nordtronics splash/logo stays — only the PHOTO64_IMAGE placeholder goes.**
- Record the outcome in the crew-vault living-frame bench notes (bench/frame-storage-and-link.md).

## Task

Make all three changes. One deliverable: the frame firmware on branch `hermes/0259-slideshow-fixes` in cabin-play, with each change working on the bench.

1. **Fix the delete double-shoot.** After a delete is confirmed, the frame must return to the slideshow on the next photo and NOT start a shoot. First establish the real cause from the log (release-triggered shoot vs contact bounce) and fix that cause. The normal shoot must keep working: a plain short press, released, still starts the countdown and takes a photo.
2. **Remove the hummingbird placeholder.** PHOTO64_IMAGE comes out of the firmware and the boot flow entirely. After the splash, the panel loads the first card photo immediately — no placeholder image, no dwell waiting it out. If the library is empty, state the fallback behavior in the reply (e.g. hold the splash until the first photo is taken) and verify it does not crash or show the hummingbird.
3. **Shuffle the slideshow.** The slideshow shows library photos in a shuffled order: each photo once per cycle, then the order is reshuffled for the next cycle. Deleted files (holes) are skipped, as now. The delete confirm still acts on the photo actually on screen, and the "next photo" after a delete is the next one in the shuffled order.

## Success criteria

- Delete-confirm release does not fire a shoot: hold → confirm → short press deletes (library N→N−1) → slideshow continues on the next photo, no countdown, no shoot. Root cause stated in the reply.
- Normal shoot still works: plain short press, release, countdown runs, photo taken and kept/discarded as before.
- No PHOTO64_IMAGE anywhere in the firmware or boot flow; after the splash the first card photo appears immediately with no placeholder dwell; empty-library behavior stated and verified.
- Slideshow order is shuffled: bench run shows photos in non-sequential order, each photo once per cycle, reshuffled for the next cycle.
- No regression: the 0254 keep/discard/timeout-keep flows, the 0256 camera notice, the 0257 silent delete (other than the fixed double-shoot), the splash, and the 0258 portrait text all still work as before.

## Constraints

- The Nordtronics splash/logo is untouched — only the PHOTO64_IMAGE placeholder is removed.
- The bug fix must preserve the intended shoot-on-release behavior for normal shooting.
- One button only (GPIO14). No new hardware. No behavior changes beyond the three items above.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Branch `hermes/0259-slideshow-fixes` pushed to origin in cabin-play, tip SHA quoted in the staged reply.
- Bench verification for each of the three items (delete flow with no follow-on shoot, boot with no placeholder, shuffled order over a cycle), quoted in the staged reply.
- Copilot peer-review result quoted in the staged reply.

## Reply format

Stage the reply in this file with these fields filled in:

- `branch`: hermes/0259-slideshow-fixes
- `tip_sha`: commit SHA on origin
- `double_shoot_root_cause`: what the log showed (release-trigger / bounce / other) + one line
- `double_shoot_fixed`: yes / no + how verified (delete run, no countdown followed, library count)
- `normal_shoot_still_works`: yes / no + one line
- `hummingbird_removed`: yes / no + what shows after the splash + empty-library behavior + one line
- `shuffle_works`: yes / no + how verified (cycle length, order seen, no repeats within a cycle)
- `regressions_checked`: keep / discard / timeout-keep / camera notice / silent delete / splash / portrait text — each pass / fail + one line
- `copilot_review`: result quoted
- `notes`: anything unexpected
