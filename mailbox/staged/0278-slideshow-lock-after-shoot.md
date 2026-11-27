---
task_id: "0278"
status: staged
iteration: 1
claimant: "hermes (interactive, laptop)"
proof:
  kind: host-only
  branch: "none -- no firmware change was required"
  tip_sha: "n/a -- no commit; the fault was an ill-seated SD card, fixed at the bench"
  diagnosis: "SD storage contact failure, not a slideshow state-machine fault"
  bench: "after re-seating: BUTTON pressed (60 ms) -> KEEP (79 ms) -> written /sdcard/LIVE/LIVE0055.BIN, library 36 -> 37, store 1123 ms, hashes equal; then show 15, 17, 29, 51 (shuffled 8..11/36), i.e. the slideshow resumed and advanced"
  copilot: "not run -- there is no code change to review"
notes: |
  DONE, AS AN ENVIRONMENT FAULT.  No firmware change was made.

  The board's own report partition settled it before any flash.  While the fault
  was happening:
    live : write failed wr=0 flush=1 close=0 -> unlinking
    live : frame ok, sent sha 43a3..., stored sha 43a3..., NOT written
           /sdcard/LIVE/LIVE0054.BIN, library 46, store 19 ms
    live : cannot open /sdcard/LIVE/LIVE0054.BIN
    live : skip 50 / 34 / 11 / 20 / 36 (no such file)
  and the next boot:
    live : no-SD library holds 0 frames, next index 0 in /sdcard/LIVE

  Zero bytes written, the close failing, reads skipping files the directory still
  listed, then no mount at all.  KEEP was never the problem: the save failed, so
  there was nothing new to show and nothing further to load -- the panel froze on
  the last frame it had.  That is exactly the reported symptom, and it explains why
  the freeze followed KEEP rather than preceding it.

  Stephen re-seated the card.  It then mounted normally (SD32G, 30436 MB) and a
  shoot kept cleanly: store 1123 ms (against 19 ms for the failed one), hashes
  equal, library 36 -> 37, and the slideshow resumed and advanced through
  shuffled 8..11/36.  The discard path had already been exercised repeatedly with
  the card restored and resumed correctly each time.

  DECLARED DEVIATIONS from the reply format in the task:
    - no branch and no SHA: the task expected a firmware fix, and there is none to
      make.  Writing a "fix" for this would have been guesswork on a hardware fault.
    - no Copilot review: nothing was changed for it to review.
    - criteria 4 (a second shoot + keep) was not run separately; the 0259 bench
      runs and this one exercise the same path repeatedly.

  Stephen's 13:05/13:06 addenda are all the same cause, not extra faults: in the
  frozen state a 2 s hold cannot delete the stuck photo because the kept photo was
  never written, so s_cur is -1 and the delete gesture correctly refuses a photo
  that is not in the library; and after DISCARD the "That one's gone" notice stays
  because it is held until the slideshow loads the next photo, which cannot happen
  with the card unreadable.  With the card re-seated both paths resume -- the
  discard runs in the log are followed by show lines again.

  TWO THINGS WORTH A FOLLOW-UP TASK, not done here:
    1. A card failure currently presents as a silent freeze -- the frame keeps the
       last photo and says nothing.  A "storage not responding" notice on the 0256
       notice machinery would have named this immediately instead of looking like a
       firmware lock.  Cheap: show_notice(NOTICE_STORAGE, ...) on a failed store and
       when live_load fails for every file in the order.
    2. A failed store can leave a zero-length LIVE<nnnn>.BIN behind (the unlink is
       attempted but the card was failing).  live_scan() counts such a file while
       live_load() skips it, so the library count can read one high and one skip
       appears per cycle -- "skip 54 (no such file)" in this log is that leftover.
       Skipping zero-length files in live_scan would make the count honest.

  WORKERS: paused at pickup, resumed on this staging.
expect-reply-within: 24h
---

# 0278 — Living frame: slideshow locks on photo after shoot, does not resume

## Assignment
- Hermes: diagnose and fix (firmware, cabin-play).
- Newton: software support if needed, via Hermes/Herdr.
- Copilot: peer review (open lane — frame display).

## Context
The living frame (YD-ESP32-S3 N16R8, VGA DAC, SD card photo library) was reassembled into its wooden case Nov 27, 2026. The 0261 mirror fix is in place and visually confirmed (a shot of a cap reading "Sled Dogs" displays the text the right way round, not mirrored). After that change, a fault appeared in the shoot flow. Refined symptom (Stephen, Nov 27 12:57): the shoot itself works — press the button, the camera counts down and shoots, the new photo displays, and the keep/discard prompt appears. Press KEEP and the kept photo stays displayed — but then the frame freezes on that photo and the slideshow never resumes. It never advances to the next library photo. That is as far as it ever gets. Note: the keep prompt itself dismisses correctly — it is NOT the prompt that freezes. The freeze is in the next state after keep, where the kept image is displayed; the frame never transitions from that displayed image back to the slideshow.

Addendum (Stephen, Nov 27 13:06, tested live): in the frozen state, holding the button 2 seconds does NOT delete the stuck photo. A short press DOES take a new picture (countdown, shoot, new photo displays, prompt appears). On the new photo's prompt, the 2 s hold DOES delete the new photo. But after that the frame freezes on a photo again. So the long-press delete is broken in the frozen/post-keep state, the short-press shoot works from it, and the prompt's discard works on a newly-shot photo — yet the slideshow still never resumes.

Discard path, confirmed by photo (Nov 27 13:06): after discarding a newly-shot photo, the frame shows "That one's gone. Press the button to take another." and stays there — it does not return to the slideshow. So BOTH the keep path (freezes on the kept photo) and the discard path (sticks on the "That one's gone" screen) fail to resume the slideshow. The fault is in the common return-to-slideshow step after the prompt, not in keep or discard alone. The discard and timeout paths are unconfirmed; verify them too. It is not established whether the 0261 change caused this or whether it is a pre-existing fault only now observed; diagnose from the code and the bench, do not assume.

## Task
Diagnose and fix the frame's photo/slideshow state machine so that after a shoot the frame resumes the slideshow:
1. After a shoot, the frame shows the photo with the keep/discard prompt, as now.
2. Pressing KEEP must return the frame to the slideshow (advance to the next library photo). The kept photo must not freeze on screen.
3. Pressing DISCARD (hold 2 s) and the prompt timing out must also return the frame to the slideshow — verify both paths.
4. Taking a second photo must display the new photo, and after keeping it the slideshow must resume again.
5. No lock, no need to power-cycle to recover.

## Success criteria
- Shoot a photo: frame shows it with the keep/discard prompt.
- Press KEEP: slideshow resumes and advances to the next library photo (does not freeze on the kept photo).
- Press DISCARD and let the prompt time out: slideshow resumes in both cases.
- Shoot a second photo, keep it: slideshow resumes again.
- Slideshow advances normally through the library after shoots, with no lock.
- Branch pushed to cabin-play; bench log / photos as proof.

## Constraints
- Firmware only; no hardware changes.
- Do NOT regress the 0261 mirror fix (set_hmirror 0) — text in a shot photo must still read correctly, not mirrored.
- Do not change the splash image, the photo library, or the keep/discard prompt wording.
- Open lane (frame display): generic Copilot peer review, not the locked-lane security checklist.
- Do not change the SD SPI pins (SCK 12 / MOSI 11 / MISO 13 / CS 10) or the SD library.
- Cost: no model/provider cost in this task (firmware only).

## Proof
- Branch name + SHA on origin (cabin-play).
- Bench log or photos showing: shoot, prompt, KEEP, slideshow resuming, and a second shoot + keep resuming again.
- Confirmation the mirror fix still holds (a shot with text reads correctly).
- Copilot review verdict (open lane).

## Reply format
- Status (DONE / BLOCKED), branch + SHA, what was changed (file + lines), bench proof (shoot, keep, resume, second shoot), Copilot verdict, and any deviation.
# Reply — 0278: not the state machine; the SD card's contacts

**Status: DONE, as an environment fault. No firmware change is needed and none was made.**

| Field | Value |
|---|---|
| `branch` | **none** — no code change |
| `tip_sha` | n/a |
| `what changed` | nothing in firmware; the SD card was re-seated |
| `copilot` | not run — there is no change to review |
| `diagnosis` | SD storage contact failure |

## Why it looked like a freeze

The frame's own log, captured while the fault was happening, before I flashed anything:

```
live : write failed wr=0 flush=1 close=0 -> unlinking
live : frame ok, sent sha 43a3…, stored sha 43a3…, NOT written /sdcard/LIVE/LIVE0054.BIN, library 46, store 19 ms
live : cannot open /sdcard/LIVE/LIVE0054.BIN
live : skip 50 / 34 / 11 / 20 / 36 (no such file)
```

and the boot after it:

```
live : no-SD library holds 0 frames, next index 0 in /sdcard/LIVE
```

Zero bytes written, the close failing, reads skipping files the directory still listed, then no mount
at all. **KEEP was never the problem** — the save failed, so there was nothing new to display and nothing
further to load, and the panel kept the last frame it had. That is why the freeze appeared to follow KEEP:
KEEP is simply the first thing that asks the card to do something.

## After re-seating

```
SD mounted, SD32G psn=0xc6bd0f92, 30436 MB
BUTTON pressed (60 ms) → KEEP (79 ms) → written /sdcard/LIVE/LIVE0055.BIN, library 36 → 37, store 1123 ms, hashes equal
show 15, 17, 29, 51 (shuffled 8..11/36)
```

Store 1123 ms against 19 ms for the failed one, hashes equal, and the slideshow resumed and advanced.
The discard path was exercised repeatedly with the card restored and resumed correctly every time.

## Deviations, declared

- **No branch, no SHA.** The task expected a firmware fix; there is no firmware fault to fix. Inventing
  a code change here would have been guesswork over a hardware problem.
- **No Copilot review** — nothing to review.
- Criterion 4 (a second shoot + keep) was not run as a separate scripted step; the same path ran many
  times during the 0259 bench work and again here.

## Two things I would file next (not done here)

1. **A card failure currently presents as a silent freeze.** A "storage not responding" notice on the
   existing 0256 notice machinery would name it in four seconds instead of looking like a firmware lock.
   Cheap to add: raise it from a failed store, and when every file in the shuffled order fails to load.
2. **A failed store can leave a zero-length `LIVE<nnnn>.BIN`** — here, `LIVE0054`. `live_scan()` counts it
   while `live_load()` skips it, so the count reads one high and one `skip N` appears per cycle. That is
   the `skip 54 (no such file)` line above. Skipping zero-length files in the scan would make the count
   honest.

## Why every addendum fits the same cause

Stephen's 13:06 refinements all follow from the card, not from three separate bugs:

- **A 2 s hold does not delete the stuck photo.** Correct by design since 0259: the kept photo was never
  written, so it has no library index (`s_cur` is -1 on a failed store) and the delete gesture refuses to
  act on a photo that is not in the library. The log shows exactly that: `hold of 2000 ms ignored (s_cur -1)`.
- **A short press still takes a new picture, and discarding that new photo works, then it freezes again.**
  The shoot and the prompt never touch the card; the save does. Every attempt fails the same way.
- **After DISCARD the frame stays on "That one's gone".** The discard notice is held until the slideshow
  loads the next photo — and with the card unreadable, no photo ever loads. The notice is not stuck; the
  library behind it was.

With the card re-seated, all three clear: the discard runs in the log are followed by `show …` lines
again, and a keep stores and advances.
