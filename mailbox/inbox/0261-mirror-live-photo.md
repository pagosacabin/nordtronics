---
task_id: "0261"
status: inbox
iteration: 0
proof: []
notes: ""
expect-reply-within: 24h
---

# 0261 — Living frame: mirror live camera shots so text reads correctly

## Assignment

- **Hermes — firmware + bench (this task):** implement the mirror fix below, verify it on the panel with a real text shot, and push the branch. Button presses during verification are Stephen's.
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Camera firmware stays with Hermes.
- **Stephen — bench side:** button presses during verification; visual confirm that text reads correctly on the panel.

## Context

- Stephen held a hat with words on it up to the camera and shot it: the words display backwards on the frame. The live camera image is mirrored left-right.
- Juno read the code before filing. The mirror is in what the GC0308 sensor delivers: `freenove-cam/src/main.cpp` has no flip anywhere (`build_raster()` copies pixels straight through, `camera_start()` sets brightness/saturation only), and the frame side just publishes the received buffer. The 90-degree counter-clockwise rotation in `convert_photo64.py` is a rotation, not a mirror — rotation cannot produce backwards text.
- `convert_photo64.py` is NOT in the live-camera path. It is a host-side tool for converting still photos into .bin framebuffer files for the SD slideshow and embedded assets. Do NOT flip anything in `convert_photo64.py`: the pre-populated stills are normal photos and must stay un-mirrored.
- The gift plan (Stephen, Nov 25): the current SD library (45 frames) is being wiped. The card will be pre-populated with the Black Bart's photos Juno scraped from Facebook/Instagram plus whatever Christy sends, converted through `convert_photo64.py`, and that frame + card ships as the gift. This task does NOT touch the library — no deleting, no rewriting, no batch-flipping stored frames. The mirror fix is for new camera shots taken with the frame (people holding up hats/signs), so they read correctly going forward.
- OUT OF SCOPE: the boot splash / Nordtronics logo — do not regenerate, rotate, flip, or otherwise modify the splash asset. The 0259/0260 slideshow and swap work stays as-is.

## Task

Mirror live camera shots left-right so text reads correctly on the panel. One deliverable: the camera firmware on branch `hermes/0261-mirror-live-photo` in cabin-play, with a text shot reading correctly on the bench.

1. Establish the actual mirror mechanism from the code/datasheet before changing anything: confirm the GC0308 delivers a mirrored image (sensor register / mounting) and state it in the reply — do not assume.
2. Preferred fix: the sensor's own horizontal-mirror setting (`set_hmirror`) in `camera_start()` — one line, zero per-pixel cost, fixes every shot at the source. If the GC0308 driver ignores it on this board, or it proves not to be the mechanism, fall back to flipping x in `build_raster()` (read the source pixel from `FB_W-1-rx`). Say which one landed and why.
3. Verify on the panel with a real text object (hat, sign, or book with words): shoot it and confirm the words read correctly. Also shoot a normal scene and confirm it still looks right (not double-mirrored).
4. Confirm no regressions: shoot timing stays ~2.9 s shoot-to-display, the ESP-NOW chunk protocol and session-id gating are unchanged, and the 0259/0260 slideshow, keep/delete prompts, camera notice, and swap behavior are untouched.
5. Record the outcome in the crew-vault living-frame bench notes (name the vault page in the reply).

## Success criteria

- A shot of printed text displays with the words reading correctly on the panel (Stephen's visual confirm).
- A normal scene still looks correct, not mirrored the wrong way (Stephen's visual confirm).
- Shoot-to-display timing unchanged (~2.9 s); no chunk-protocol or slideshow regressions.
- No change to `convert_photo64.py`, no change to the splash, no change to the stored SD library.
- Branch `hermes/0261-mirror-live-photo` on origin in cabin-play; Copilot SHIP.

## Constraints

- Camera firmware only (`living-frame/firmware/freenove-cam/`), unless the frame side is proven necessary — say so in the reply if it is.
- Branch from `hermes/0260-refresh-jitter` @ ac8c40ae6bd1560515fefeabed4092a9f791a62c (latest verified frame branch on origin).
- No new hardware. No secrets in the commit (repo is public). Push cabin-play via SSH from the laptop (the gh.py credential is 403 on cabin-play writes).
- Cost: DeepSeek pinned tier as usual; no model choice without a stated bound.

## Proof

- Branch name + commit SHA on origin for pagosacabin/cabin-play.
- Bench log lines: the shoot, the shot count, and the send log for the verification shot.
- Before/after panel photos of the same text object (or a short video) showing the words backwards before the fix and reading correctly after — Stephen's eyes are the gate, his visual confirm closes it.
- Copilot review verdict.

## Reply format

- `branch`: name + SHA on origin
- `mirror_mechanism`: one line, what the sensor was doing and how it was confirmed
- `fix`: set_hmirror / build_raster flip + one line on why that one
- `text_shot`: what object, before/after result, Stephen's confirm (or pending)
- `timing`: shoot-to-display ms for the verification shot
- `regressions`: slideshow/prompts/notice/swap — pass or fail each, one line each
- `vault`: page updated
