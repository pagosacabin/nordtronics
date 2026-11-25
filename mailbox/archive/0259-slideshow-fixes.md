---
task_id: "0259"
status: verified
iteration: 1
proof:
  kind: host-only
  branch: "hermes/0259-slideshow-fixes"
  tip_sha: "1a21a3e"
  base: "hermes/0258-portrait-text @ 5400103; splash asset and code untouched"
  build: "pio run -e n16r8-photo SUCCESS (1,449,120 B, was 1,755,984 -- the placeholder is 306,864 B of it); flashed to /dev/ttyUSB1, 'Hash of data verified'"
  double_shoot_root_cause: "not contact bounce -- the loop's press bookkeeping (press_since/press_used/last_level) still described the press that answered the prompt, so the release could be re-read as a fresh shoot press; when the press began while the loop was blocked, press_since stayed 0 and the next real press was swallowed"
  double_shoot_fixed: "yes -- 'DELETE (short press, 140 ms)' -> 'removed /sdcard/LIVE/LIVE0052.BIN, library 45 frames' with no BUTTON/countdown line after it"
  normal_shoot_still_works: "yes -- BUTTON pressed (99 ms) -> countdown 3/2/1 -> KEEP (160 ms) -> LIVE0052.BIN, library 46, hashes equal, store 1118 ms"
  hummingbird_removed: "yes -- no PHOTO64 strings anywhere in the binary; boot logs 'boot : splash -> first card photo' then 'show 15 (shuffled 1/44)' then 'logo buffer released after the first photo'"
  shuffle_works: "yes -- 66 show lines: cycle 1 = 44 slides, k 1..44, zero duplicates, zero missing; cycle 2 starts with a different order (0/22 positions match cycle 1)"
  copilot: "six passes: NO SHIP (2 findings), NO SHIP (2), NO SHIP (1 off-by-one), NO SHIP (2), NO SHIP (1), then SHIP"
notes: |
  DONE AND BENCH-VERIFIED — Hermes (laptop, interactive), 2026-11-25 ~14:10-15:45 MST.

  ROOT CAUSE, established from the code and then from the bench log rather than
  assumed: ask_keep()/ask_delete() read the button themselves while the live loop is
  blocked inside them, so the loop's press_since/press_used/last_level could still
  describe the press that answered the prompt.  Two symptoms, one cause -- a
  release re-read as a fresh shoot press (the double-shoot) and, when the press
  began while the loop was blocked, a press_since of 0 that swallowed the next real
  press (seen at the bench: a tap that did nothing).  Fix: settle_button() waits for
  release after every prompt, the shoot requires a press the loop saw begin AND at
  least 60 ms long, and the delete prompt refuses anything under 60 ms so a bounce
  can never delete a photo.

  FIRST ATTEMPT AT A FIX WAS WRONG and is recorded because it matters: a fixed
  250 ms quiet window after each prompt swallowed a whole deliberate press.  Copilot
  caught it; the window is gone and bounce is filtered by press length instead.

  EMPTY LIBRARY (stated, not bench-verified -- testing it means deleting every
  photo): with no files the slideshow gate never runs, so the splash stays on the
  panel, order_build() returns false and the loop retries every 10 s.  Nothing
  dereferences a missing photo, so it cannot crash; the first photo taken brings
  the slideshow to life because s_lib_count becomes 1.

  REVIEW: six passes.  Findings fixed, in order: the shuffle scan ignored
  five-digit names; the 250 ms settle window swallowed a press; the index cap was
  enforced only in the order builder; a bounce could answer the delete prompt and
  the scan counted files the order could not show; and two off-by-one steps in the
  65535 bound until store, scan and order all agreed on 65535 files (indices
  0..65534).  Each was fixed at the source rather than papered over.

  Library after the bench: 45 frames, next index 53.

  WORKERS: paused at pickup, resumed on this staging, verified with cron list.
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

# Reply — 0259: double-shoot, placeholder, shuffle

**All three done and bench-verified. Copilot SHIP on the sixth pass.**

| Field | Value |
|---|---|
| `branch` | `hermes/0259-slideshow-fixes` |
| `tip_sha` | **1a21a3e** |
| `double_shoot_root_cause` | **Not contact bounce.** The prompts read the button themselves while the live loop is blocked inside them, so the loop still held the press that answered the prompt; its release could be re-read as a fresh shoot press. The same fault swallowed the *next* real press when the press began during the blocked window (seen at the bench: a tap that did nothing). |
| `double_shoot_fixed` | **yes** — `DELETE (short press, 140 ms)` → `removed /sdcard/LIVE/LIVE0052.BIN, library 45` and **no** countdown or shoot after it |
| `normal_shoot_still_works` | **yes** — `BUTTON pressed (99 ms)` → countdown → `KEEP (160 ms)` → `LIVE0052.BIN`, library 46, hashes equal |
| `hummingbird_removed` | **yes** — zero `PHOTO64` strings in the binary, 306,864 B smaller; boot logs `splash -> first card photo`, then `show 15 (shuffled 1/44)`, then `logo buffer released after the first photo`. Empty library: the splash stays and the loop retries — see notes. |
| `shuffle_works` | **yes** — over 66 slides: cycle 1 = 44 slides, positions 1..44, **zero duplicates, zero missing**; cycle 2 a different order (0/22 positions match) |
| `copilot_review` | six passes — NO SHIP ×5 (increasingly narrow findings), then **SHIP** |

## 1. The double-shoot

One cause, two symptoms. `ask_keep()` and `ask_delete()` poll the button themselves while the loop is
blocked inside them, and the loop's state (`press_since`, `press_used`, `last_level`) was not reconciled
afterwards. So the release of the answering press could be re-read as a fresh shoot press — and, when a
press began while the loop was blocked, `press_since` stayed 0 and the *following* real press was
swallowed instead.

Fix: `settle_button()` waits for release after every prompt, the shoot branch only fires on a press the
loop saw begin and that lasted ≥ 60 ms, and the delete prompt refuses anything under 60 ms so a bounce
can never delete a photograph.

**My first fix was wrong and it is worth saying so:** a fixed 250 ms quiet window after each prompt
swallowed a whole deliberate press. Copilot caught it; the window is gone and bounce is filtered by
press length instead.

## 2. The hummingbird

`PHOTO64_IMAGE` is out of the firmware entirely — 306,864 bytes, no `PHOTO64` strings left. The splash
now *stays* on the panel until `live.c` publishes the first card photo, then its buffer is freed (it is
handed over rather than freed in `main.c`, because freeing the live scanout source would tear the
panel). The 6 s scanout measurement that made the logo linger is 1 s.

**Empty library — stated, not bench-verified** (testing it means deleting every photo): with no files
the slideshow gate never runs, so the logo simply stays on the panel, `order_build()` returns false, and
the loop retries every 10 s. Nothing dereferences a missing photo, so it cannot crash, and the first
photo taken brings the slideshow to life.

## 3. The shuffle

`order_build()` collects the card indices and Fisher-Yates shuffles them with `esp_random()`; each file
appears once per cycle and the order is rebuilt when the cycle ends. A deleted file is dropped from the
order **in place**, so "next photo" after a delete really is the next one in the shuffled order rather
than a fresh shuffle.

Bench evidence, 66 consecutive slides:

```
cycle 1: 44 slides, positions 1..44, duplicates=none, missing=0
cycle 2: different order (0/22 positions match cycle 1)
```

## Regressions

keep: **pass** (short press, stored, hashes equal). discard: **pass** (`DISCARD (held 2000 ms, bar full)`,
nothing written). silent delete: **pass** (the runs above). camera notice: **pass** — unchanged since
0258 where it was verified; not re-run on this branch. timeout-keep: **not re-run on this branch** — the
path is unchanged since 0257 where it passed (42→43); say so rather than imply otherwise. splash:
**pass** (sha `d19ef73a…`, `hold 6000 ms`, logo identical). portrait text: **pass** (every prompt and
notice in this task's runs rendered upright).

## Review history

Six passes, each finding real: five-digit card names ignored by the shuffle scan; the 250 ms window
swallowing a press; the index cap only in the order builder; a bounce able to answer the delete prompt;
the scan counting files the order could not show; and two off-by-one steps in the 65535 bound until
store, scan and order agreed on 65535 files (indices 0..65534).

## Closure — verified by Juno, 2026-11-25

- Branch tip verified against origin: hermes/0259-slideshow-fixes @ 1a21a3e1b32eb63309653d0119821b9111ee71c8. Matches the staged reply. Base hermes/0258-portrait-text @ 5400103; splash asset and code untouched (sha d19ef73a... unchanged).
- All success criteria met on the bench: delete-confirm release no longer fires a shoot (DELETE 140 ms -> removed LIVE0052.BIN, library 45, no countdown after it); normal shoot still works (BUTTON 99 ms -> countdown -> KEEP 160 ms -> LIVE0052.BIN, library 46, hashes equal); PHOTO64 gone from the firmware (zero PHOTO64 strings, 306,864 B smaller: 1,449,120 B vs 1,755,984 B; boot logs splash -> first card photo, logo buffer released after the first photo); shuffle verified over 66 slides (cycle 1 = 44 slides, positions 1..44, zero duplicates, zero missing; cycle 2 a different order, 0/22 positions match).
- Root cause (not bounce): ask_keep()/ask_delete() poll the button while the live loop is blocked inside them, and the loop's press_since/press_used/last_level were never reconciled afterwards — so the release of the answering press could be re-read as a fresh shoot press, and a press begun during the blocked window (press_since 0) swallowed the next real press. Fix: settle_button() waits for release after every prompt; shoot needs a press the loop saw begin, >= 60 ms; delete prompt refuses anything under 60 ms. First fix (fixed 250 ms quiet window) was wrong — swallowed a real press; Copilot caught it, replaced rather than patched.
- Copilot: six passes, NO SHIP x5 then SHIP; every finding fixed at the source (five-digit names, quiet window, index cap, bounce answering delete, scan/order disagreement, two off-by-ones in the 65535 bound; store/scan/order now agree on 65535 files, indices 0..65534).
- Declared gaps, recorded honestly: empty-library behavior (splash holds, loop retries every 10 s, order_build() false, no crash) stated from code, NOT bench-tested (testing means deleting every photo); camera notice and timeout-keep NOT re-run on this branch (unchanged since 0258/0257 where they passed).
- Library after the bench: 45 frames, next index 53. This also settles the open 0257 decision: shoot-on-release stays (press the loop saw begin, >= 60 ms), with the double-fire fixed by state reconciliation.
