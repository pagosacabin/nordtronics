---
task_id: "0254"
status: verified
iteration: 1
proof:
  kind: host-only
  branch: "hermes/0254-keep-prompt"
  tip_sha: "e18479a"
  base: "branched from hermes/boot-splash (3182a68) so the in-flight splash could be flashed alongside; no splash file modified by this task"
  files: ["living-frame/firmware/n16r8-live/src/live.c", "living-frame/firmware/n16r8-live/src/prompt_strip.h", "living-frame/firmware/n16r8-live/src/notice_deleted.h"]
  build: "pio run -e n16r8-photo SUCCESS; flashed to /dev/ttyUSB1, 'Hash of data verified'"
  keep: "library 41 -> 42, LIVE0041.BIN, sent sha e7552f26… == stored sha e7552f26…, store 1151 ms"
  discard: "library 42 -> 42, nothing written, notice shown for 4 s"
  timeout: "library 42 -> 43, LIVE0042.BIN, sent sha 314dfa25… == stored sha 314dfa25…, store 1148 ms, 30 s timeout"
  copilot: "pass 1 NO SHIP (finding 1: countdown-digit restore), fixed in e18479a, pass 2 SHIP"
notes: |
  DONE AND BENCH-VERIFIED — Hermes (laptop, interactive), 2026-11-25 12:00-13:00 MST.
  All three cases run on the real frame and camera; every number below is quoted from
  the board's own report partition (esptool read_flash 0x310000 0x10000), not from the
  console.

  Copilot peer review ran twice. Pass 1 returned NO SHIP with one MINOR finding: on a
  capture timeout or hash mismatch, restore_previous() republished a draw buffer that
  the three countdown frames had overwritten, so the panel could show a countdown digit
  instead of the previous photo — the same artefact Stephen had already spotted on the
  discard path. Fixed in e18479a with an s_hold snapshot taken before the countdown.
  Pass 2 returned SHIP, confirming the fix and finding no regressions.

  Not bench-exercised: the capture-timeout/hash-mismatch restore path itself. It is
  code-reviewed and the fix is the same mechanism the discard notice uses, but
  inducing a capture failure needs the camera unplugged and that run has not happened.
  Declared rather than implied.

  Workers: paused at pickup, resumed on this staging, verified with cron list.
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

# Reply — 0254: keep-or-discard confirm

**Done. All three cases bench-verified on the frame + camera. Copilot: SHIP on the second pass.**

| Field | Value |
|---|---|
| `branch` | `hermes/0254-keep-prompt` |
| `tip_sha` | `e18479a` |
| `prompt_shows` | **yes** — the photo scaled to 533x400 above a full-width strip reading "Keep this picture?" with "press = keep / hold 2 s = discard"; a grey track under it fills orange while the button is held |
| `text_rendering` | pre-rendered strip assets, not a live font: `prompt_strip.h` (640x80) blitted below the photo, `notice_deleted.h` (640x140) for the discard frame. Generated with the same RGB222 packing/gamma path as the photo |
| `yes_stores` | **41 → 42** (`LIVE0041.BIN`, sent sha == stored sha, store 1151 ms) |
| `no_discards` | **42 → 42** — nothing written, notice shown for 4 s |
| `timeout_keeps` | **42 → 43** (`LIVE0042.BIN`, sent sha == stored sha, store 1148 ms), timeout **30 s** |
| `shoot_not_retriggered` | **yes** — a press still held from the shoot is ignored until release, and a press during the prompt answered it; the log shows one `SHOOT sent` per button press, never two |
| `regressions_checked` | slideshow **pass** (resumes after every case, cycling the holes 0249 allows); existing library **pass** (40 → 43 across runs, no file disturbed); shoot→hash→store **pass** (both keeps stored with sent == stored) |
| `copilot_review` | pass 1: *"MINOR … restore_previous() republishes a buffer overwritten by the three countdown frames … Verdict: NO SHIP"*; after the fix, pass 2: *"s_hold snapshots the currently displayed frame before the countdown … I found no regressions … Verdict: SHIP"* |

## The bench log, verbatim from the report partition

```
confirm : library 41 frames before the answer
confirm : KEEP (short press, 139 ms)
confirm : keeping (answered), storing
live    : frame ok, sent sha e7552f26…, stored sha e7552f26…, written /sdcard/LIVE/LIVE0041.BIN, library 42, store 1151 ms

confirm : library 42 frames before the answer
confirm : DISCARD (held 2000 ms, bar full)
live    : discarded on hold, nothing written, library still 42
live    : notice on panel for 4 s; a press starts a new shot

confirm : library 42 frames before the answer
confirm : KEEP (no answer in 30 s; timeout never discards)
confirm : keeping (30 s timeout), storing
live    : frame ok, sent sha 314dfa25…, stored sha 314dfa25…, written /sdcard/LIVE/LIVE0042.BIN, library 43, store 1148 ms
```

## Two things the first bench run taught, both now fixed

1. **The shot's own press discarded the photo.** The first build read the still-held shoot
   press as a 2 s hold, so a photo could be deleted before the question was read. The prompt
   now waits for a release before accepting an answer.
2. **Discard republished a countdown digit.** On discard the panel showed the last countdown
   frame instead of returning to a picture; the discard path now renders a notice —
   "That one's gone. / Press the button to take another." — for 4 s, and a press during it
   starts a new shot. Copilot independently found the same fault on the timeout/hash-mismatch
   paths, which is what e18479a fixes.

## Scope

The boot splash was **not touched**: this branch simply sits on top of `hermes/boot-splash`
so both could be flashed together, and the splash still runs 6 s before the photograph.

## Closure — verified by Juno, 2026-11-25

- Branch tip verified against origin: hermes/0254-keep-prompt @ e18479af7367c960a8620c9c15eed3ff4badd1b7. Matches the staged reply.
- All success criteria met on the bench: prompt shows; short press stores (41→42, sha match); 2 s hold discards (42→42, nothing written); 30 s timeout keeps (42→43, sha match); no pre-confirmation writes; no regressions (slideshow, library, shoot-hash-store).
- Copilot: NO SHIP pass 1 (countdown-digit restore), fixed in e18479a, SHIP pass 2.
- Note: branch stacks on hermes/boot-splash (3182a68), not main — deliberate, so the logo and the confirm flow flash together; splash code untouched by this task. Merge planning must account for the splash branch.
- Open (declared gap, not a criterion): capture-timeout/hash-mismatch restore path is code-reviewed but never bench-exercised. Close it with the unplug-CH340 test Hermes described (unplug, press once, wait ~12 s, panel should return to the photo that was showing).
- Chunk NACK/resend remains unimplemented (out of scope since 0249).
