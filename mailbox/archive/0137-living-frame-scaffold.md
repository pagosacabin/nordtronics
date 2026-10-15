---
task_id: "0137"
protocol_version: 1.0.0
status: verified
expect-reply-within: 6h
iteration: 1
proof:
  branch: main (pagosacabin/cabin-play)
  sha: "0347ed30396283f47b956277effb1795c5df7c83"
  branch_note: "The deliverable is a cross-repo commit, so the proof points at the
    DESTINATION repo's main, not at a branch of nordtronics -- the same shape as
    0136. Verified on origin, not just locally: `git ls-remote origin
    refs/heads/main` = 0347ed30396283f47b956277effb1795c5df7c83 and
    `gh api repos/pagosacabin/cabin-play/commits/main` returns that same sha.
    Pushed over SSH (`git@github.com:pagosacabin/cabin-play.git`) exactly as 0136
    did, because the custom.github API credential is nordtronics-scoped and API
    writes to cabin-play return 403."
  run: none
  run_note: "Earned by enumeration, not asserted. cabin-play carries NO CI at all:
    `git ls-tree -r --name-only origin/main | grep -c workflows` = 0, the local
    checkout has no .github directory, and
    `gh api repos/pagosacabin/cabin-play/contents/.github` -> HTTP 404. `gh run
    list -R pagosacabin/cabin-play --limit 5` returns no rows. No nordtronics
    workflow covers the deliverable either -- the only files that changed are in
    cabin-play -- and the only nordtronics runs this invocation can produce are
    for its own mailbox transitions (pickup 45ffe5d, this staging commit), which
    check the website, not this task."
  files:
    - living-frame/README.md
  file_sha256_local: "36a9794aec0255636545ad70f52d3a430d860cd3131fae980af6ea9bff2552e1"
  file_sha256_origin: "36a9794aec0255636545ad70f52d3a430d860cd3131fae980af6ea9bff2552e1"
  file_sha256_check: "Identical, and the origin figure was hashed over bytes
    re-downloaded FROM ORIGIN after the push
    (raw.githubusercontent.com/pagosacabin/cabin-play/main/living-frame/README.md),
    so the scaffold on main is the scaffold that was written. Git blob sha
    3d8ec0bab846988fd7a0e208ebdccbe1202482d0, 2286 bytes, per the GitHub contents
    API at ref=main."
  ntfy: not_applicable
  ntfy_note: "No compiled artifact and no build workflow exists in cabin-play, so
    there is no ntfy receipt to publish; the companion-build topic in the worker
    prompt (nordtronics-build-ed05a663) does not apply. Stated rather than omitted."
  not_touched: "The cabin-play worktree carried two UNCOMMITTED local edits when
    this run started (guition-jc4827w543/jc4827w543.yaml and
    guition-jc4827w543/tools/preview.py -- Stephen's own work, 11 lines each).
    They were left exactly as found: the commit staged only living-frame/README.md
    (`git add living-frame/README.md`), and both modifications are still
    unstaged. No existing file in cabin-play was modified or deleted."
notes: |
  STAGED (iteration 1), 2026-10-15 18:17 UTC / 12:17 MDT. Off-peak
  (PEAK: OFF-PEAK 18:15 UTC). PROTOCOL: MATCH protocol_version=1.0.0
  (origin/main:mailbox/README.md). Gate: mailbox changed ->
  main-not-at-origin/main. Queue state at pickup: mailbox/active/ held only
  .gitkeep and mailbox/inbox/ held 0137 and 0138, so 0137 was the lowest
  actionable task. One task per run: 0138 was NOT opened and still sits in
  mailbox/inbox/ with status: inbox.

  DELIVERABLE: `living-frame/README.md` created in pagosacabin/cabin-play, commit
  0347ed30396283f47b956277effb1795c5df7c83 on main, pushed over SSH. The spec from
  the task body is carried in full and is the bulk of the file. One formatting
  adaptation is declared rather than hidden: a small `## Bench inventory` section
  was added at the end, because it is what rung 0 needs and it was already
  established fact (see the board inventory below). Everything else is the spec
  verbatim; the only other touch is that the spec's `Display unit:` /
  `Camera unit:` labels sit as short paragraphs with a blank line before their
  lists, so the markdown renders them as separate blocks.

  BOARD INVENTORY ON THE BENCH (required by the task, step 3):
  CLASSIC ESP32 -- the proven VGA path -- both on hand:
    - ESP32-WROOM-32 devkit module(s)
    - DFRobot FireBeetle ESP32-E (DFR0654)
  Source: Stephen confirming with photos on 2026-10-15, as relayed in the body of
  task 0138. I did not photograph the bench myself; this is Juno's own recorded
  inventory, not a claim I verified by eye.
  My own direct observation, as a cross-check: two ESP32-S3-family boards are
  attached to the bench host right now -- `lsusb` shows two `303a:1001 Espressif
  USB JTAG/serial debug unit` entries, i.e. the S3/C3-class NATIVE USB-Serial/JTAG
  peripheral, not a CP2102/CH340 classic-devkit bridge, with /dev/serial/by-id
  symlinks to ttyACM0 and ttyACM1. So the S3 boards are the ones plugged in, while
  the classic WROOM-32 and FireBeetle are the on-hand boards that rung 0 needs.
  The S3 has no proven VGA path in this project, which is why rung 0's target is
  the classic WROOM-32 devkit.

  HOW VERIFIED (read, not assumed):
    1. `git ls-remote origin refs/heads/main` on cabin-play =
       0347ed30396283f47b956277effb1795c5df7c83 == the pushed tip.
    2. `gh api repos/pagosacabin/cabin-play/commits/main --jq .sha` returns the
       same sha.
    3. `gh api repos/pagosacabin/cabin-play/contents/living-frame` lists
       `README.md`, path `living-frame/README.md`, 2286 bytes -- the API tree
       listing the task's proof section asks for.
    4. sha256 of the raw origin file equals sha256 of the local file
       (36a9794a...). This is the read-back that makes the push claim true.
  No CI run exists to cite (see proof.run_note); the evidence is the origin re-read
  of the pushed bytes.

  NOT DONE THIS RUN: 0138's proof firmware, wiring table and adapter work. That is
  the next task in the queue and was deliberately not started.

  VERIFIED BY JUNO 2026-10-15 ~12:47 MDT (independent):
    - Scaffold commit 0347ed30396283f47b956277effb1795c5df7c83 exists in
      cabin-play main's history (matches staged proof pointer).
    - living-frame/README.md present via API list-dir.
    - NOTE: main tip has since moved to ee5f40f (Stephen's own Guition UI
      pushes: dial/BMS/LCARS work) — the scaffold commit is intact in the
      ancestry, nothing overwritten. No action needed.
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
