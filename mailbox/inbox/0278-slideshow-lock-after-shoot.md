---
task_id: "0278"
status: filed
iteration: 1
proof: []
notes: ""
expect-reply-within: 24h
---

# 0278 — Living frame: slideshow locks on photo after shoot, does not resume

## Assignment
- Hermes: diagnose and fix (firmware, cabin-play).
- Newton: software support if needed, via Hermes/Herdr.
- Copilot: peer review (open lane — frame display).

## Context
The living frame (YD-ESP32-S3 N16R8, VGA DAC, SD card photo library) was reassembled into its wooden case Nov 27, 2026. The 0261 mirror fix is in place and visually confirmed (a shot of a cap reading "Sled Dogs" displays the text the right way round, not mirrored). After that change, a new fault appeared: after taking a picture, the frame locks on that picture and does not go past it. The "Keep this picture?" prompt appears, but the slideshow never resumes. Pressing the button takes another picture (the camera shoots) but the new photo does not display and the slideshow does not advance. The frame stays locked on the last photo. It is not established whether the 0261 change caused this or whether it is a pre-existing fault only now observed; diagnose from the code and the bench, do not assume.

## Task
Diagnose and fix the frame's photo/slideshow state machine so that after a shoot the frame resumes the slideshow:
1. After a shoot, the frame shows the photo with the keep/discard prompt, as now.
2. Pressing keep, pressing discard (hold 2 s), or the prompt timing out must all return the frame to the slideshow.
3. Taking a second photo must display the new photo (not stay stuck on the first).
4. The slideshow must advance through the library normally after shoots.
5. No lock, no need to power-cycle to recover.

## Success criteria
- Shoot a photo: frame shows it with the keep/discard prompt.
- Keep (or discard, or timeout): slideshow resumes and advances to the next library photo.
- Shoot a second photo: the new photo displays; the frame does not stay stuck on the first.
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
- Bench log or photos showing: shoot, prompt, slideshow resuming, and a second shoot displaying the new photo.
- Confirmation the mirror fix still holds (a shot with text reads correctly).
- Copilot review verdict (open lane).

## Reply format
- Status (DONE / BLOCKED), branch + SHA, what was changed (file + lines), bench proof (shoot, resume, second shoot), Copilot verdict, and any deviation.
