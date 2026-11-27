---
task_id: "0278"
status: filed
iteration: 1
proof: []
notes: "Symptom refined 2026-11-27 12:57, 13:05, and 13:06 by Stephen: the new photo DOES display after a shoot; the keep prompt dismisses correctly (it is NOT the prompt that freezes); the freeze is in the post-keep state where the kept image is displayed — the frame never transitions back to the slideshow. Addendum 13:06: in the frozen state, a 2 s hold does NOT delete the stuck photo, but a short press DOES take a new picture, and the prompt's 2 s discard DOES delete a newly-shot photo — yet the frame freezes again afterward. Photo 13:06: after DISCARD the frame shows 'That one's gone. Press the button to take another.' and stays there — it does not return to the slideshow. So BOTH keep and discard paths fail to resume the slideshow."
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
