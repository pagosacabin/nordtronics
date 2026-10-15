---
task_id: "0137"
protocol_version: 1.0.0
status: in_progress
expect-reply-within: 6h
iteration: 1
---

# 0137 — Scaffold the living-frame project in cabin-play

## context

Stephen's Christmas gift project: an old Magnavox 22MD311B/F7 22" LCD TV
(Funai, CCFL-backlit, 70W) becomes a wooden-framed LIVING photo frame —
slideshow on the TV, plus a big-button camera unit that lets guests add
their own photos to the rotation (press button -> 3-2-1 countdown ->
capture -> photo joins the slideshow).

Play projects live in `pagosacabin/cabin-play`, never nordtronics (Stephen's
ruling 2026-10-15). The repo currently holds only `guition-jc4827w543/`.

IMPORTANT — pushing: the `custom.github` API credential is nordtronics-scoped.
API writes to cabin-play return 403 (learned on task 0136). Push via SSH from
the laptop instead, exactly as 0136 did (commit b7ad32bb on cabin-play main).

## task

Create the project scaffold in `pagosacabin/cabin-play`:

1. Create `living-frame/README.md` with the spec below (verbatim is fine,
   adapt formatting as needed).
2. Commit and push to cabin-play main via SSH.
3. In your staged reply's `notes:` field, report which ESP32 boards are on
   hand on the bench (classic ESP32-WROOM-32 devkits vs ESP32-S3 boards) —
   the proof rung needs a classic ESP32 for the proven VGA path.

---

# Living Frame (working title)

Old Magnavox 22MD311B/F7 22" LCD TV -> wooden-framed living photo frame.
A Christmas gift.

## How it works

- A classic ESP32 drives the TV through its VGA/PC input (bitluni ESP32Lib
  or FabGL) and shows a JPEG slideshow from a microSD card.
- A separate ESP32-CAM unit with a big arcade button: press -> 3-2-1
  countdown (buzzer/LED) -> captures a photo -> sends the JPEG over ESP-NOW
  to the display unit -> display unit saves it to SD and it joins the
  rotation. "Added!" pops up on screen.
- Why two units: two MCUs cannot safely share one SD card. The ESP-NOW
  wireless hop is the proven fix (JPEG sent in chunks).
- The TV is driven through its inputs, NOT by ripping the panel (LVDS is a
  dead end for ESP32). Final form: plastic bezel removed, panel + boards
  mounted in finished wood.

## Parts

Display unit:
- Classic ESP32 devkit (ESP32-WROOM-32 style — the proven VGA path)
- microSD card module + card
- VGA connector + resistor ladder (per bitluni ESP32Lib wiring)
- 5V supply, VGA cable

Camera unit:
- ESP32-CAM with OV2640 (usually bundled)
- Big arcade button, small buzzer or bright LED
- USB 5V supply (or LiPo for a roaming photo button)

## Build order — proof first

- Rung 0 (THE PROOF): VGA test pattern / static image from the ESP32 on the
  Magnavox. Proves the entire display path. Nothing else starts until this
  holds a stable, correct-color image.
- Rung 1: JPEG slideshow from SD on the TV.
- Rung 2: ESP32-CAM captures on button press with countdown beeps.
- Rung 3: ESP-NOW camera -> display transfer; photo joins the rotation.
- Rung 4: Wood frame — remove plastic bezel, mount panel + boards in
  finished wood.
- Rung 5: Polish — "Added!" overlay, power management, gift wrap.

## Acceptance bars

- Rung 0: recognizable test image on the TV, stable, correct colors.
- Rung 3: press button -> photo appears in the slideshow within ~30s.

---

## success_criteria

- `living-frame/README.md` exists on `pagosacabin/cabin-play` main with the
  spec above.
- Commit SHA on cabin-play main cited.
- Staged reply notes report the ESP32 boards on hand (classic vs S3).

## constraints

- One deliverable: the scaffold. No firmware yet, no bench work.
- Push via SSH (API writes 403 on this repo). No secrets in the task file
  or staged reply.

## proof

- Commit SHA on `pagosacabin/cabin-play` main.
- `git ls-remote` or API tree listing showing `living-frame/README.md`.

## reply_format

Stage the reply under `mailbox/staged/0137-living-frame-scaffold.md` with
front matter (`task_id: 0137`, `status: staged`, `iteration: 1`), the proof
pointers above, and a `notes:` field (including the board inventory).
