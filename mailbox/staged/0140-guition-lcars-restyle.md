---
task_id: "0140"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - branch: main
    repo: pagosacabin/cabin-play
    sha: 655f25515aff52f3fa96c62908d8c16baa9e0f90
  - run: none
render: https://raw.githubusercontent.com/pagosacabin/cabin-play/655f25515aff52f3fa96c62908d8c16baa9e0f90/guition-jc4827w543/preview-bms-lcars.png
files:
  - guition-jc4827w543/jc4827w543.yaml
  - guition-jc4827w543/tools/preview.py
  - guition-jc4827w543/README.md
  - guition-jc4827w543/preview-bms-lcars.png
proof_note: |
  The deliverable is a cross-repo commit (cabin-play, not nordtronics), the same shape
  as 0136 / 0137 / 0138, so the branch + sha point at the DESTINATION repo's main.
  Verified on origin, not just locally: `git ls-remote origin refs/heads/main` =
  655f25515aff52f3fa96c62908d8c16baa9e0f90 and `gh api
  repos/pagosacabin/cabin-play/commits/main --jq .sha` returns the same sha (committer
  date 2026-10-16T19:24:07Z, 4 files changed). Blob-level check after the push: all four
  files re-fetched through the API and compared with `git hash-object` --
  jc4827w543.yaml f5b782bd1e882e1ef64b5945cc7878b78d3877fc,
  tools/preview.py 27974833dec4091bc5a2fb4c8ce157889cbb3be3,
  README.md cfc5dc0c0ba345116f1e46ae16a3e82d3f7766ab,
  preview-bms-lcars.png 7495cc1da0d333db83bf8314d785e4d921bff8fe -- all four identical.

  `run: none` is earned by enumeration, not asserted. cabin-play carries NO CI:
  `gh api repos/pagosacabin/cabin-play/contents/.github` -> HTTP 404,
  `git ls-tree -r --name-only origin/main | grep -c '^\.github'` -> 0,
  `gh run list -R pagosacabin/cabin-play -L 5` -> no rows. No nordtronics workflow can
  cover it either: the changed tree is not in nordtronics at all. The build evidence is
  therefore a local ESPHome compile plus an OTA flash of the built image, quoted in the
  reply body, not a run URL.

  The render is a durable layout pointer, committed with the change (the way 0138
  committed its build log): a 480x588 sheet, BMS page above the status fallback. It is a
  PIL mirror of the lambda, so it is layout evidence, not a photograph -- see the photo
  gap in the reply body.

  ntfy: nordtronics-build-ed05a663 @ 2026-10-16T19:28:13Z (id dmibMfluZjn1).
notes: |
  STAGED 2026-10-16 19:28 UTC, off-peak (PEAK: OFF-PEAK 19:15 UTC), protocol MATCH
  (1.0.0). The task was filed as `status: filed`, which is not one of the protocol's
  states -- normalised to `in_progress` and `iteration` 1 -> 2 at pickup (commit
  d83a853), as the README's pickup transition requires.

  FOUR OF FIVE SUCCESS CRITERIA MET, ONE NOT: criterion 4 (a photograph of the physical
  panel) is not met and is not claimed -- this host has no camera that can see the
  panel. Everything else is done and verified in the reply body. Staged rather than
  parked because nothing here needs a decision: code, build, push and flash are
  complete, and the photo is a minute's work for someone at the bench. Same pattern as
  0134, where the panel-side photo arrived as `photo evidence in chat` after staging.

  CHANGED (inside guition-jc4827w543/jc4827w543.yaml, plus three declared extensions):
  the BMS page section of the display lambda -- base lines 290-394 -- and the file's own
  top-of-file display comment (base lines 8-10). A structural diff against the base
  revision proves the blast radius: every non-equal line lies in one of those two
  regions, and the blocks the constraints name are BYTE-IDENTICAL (page-select fallback
  base 266-287, spi/touch/qspi_dbi block base 115-150, font definitions base 396-414,
  watchdog blocks base 532-663, api/wifi/ota base 58-73). sensors, wifi, api, ota,
  ble_client, esp32_ble_tracker, globals, interval, on_boot and on_value were not
  touched; no Home Assistant import changed.

  DECLARED SCOPE EXTENSIONS (three files beyond the spec's one, each with its reason):
  tools/preview.py -- it is the repo's own layout harness and the README documents it as
  mirroring the lambda's geometry; left stale it renders a page that no longer exists.
  README.md -- it documents this page's palette and construction and would otherwise
  describe the previous frame. preview-bms-lcars.png -- the durable layout pointer above.
  Not touched: living-frame/, secrets, and the tracked tools/__pycache__/*.pyc (restored
  to its committed bytes rather than re-committed).

  CORRECTION MADE IN THE README: its dial bullet described 3-degree quads with filled
  triangles and separately drawn end caps; the lambda has drawn overlapping discs since
  ee5f40f. The bullet now matches the code and says so. That is a pre-existing
  documentation defect, not a change I made to the drawing.

  PHYSICAL ACTION DECLARED: the built image was OTA-flashed to the panel (192.168.1.72)
  so the panel is actually displaying the restyle -- a photo of the new page is
  impossible otherwise. OTA only: no USB connection, no erase_flash, no NVS write, and
  the previous image is intact in git history. Afterwards the device reports
  `compiled on 2026-10-16 13:20:44 -0600`, the build time of this revision, and
  `page=bms` on the once-a-minute line.

  The one thing NOT verified is what the glass looks like. The render and the text-box
  audit prove the geometry the lambda draws; whether it reads well at arm's length is a
  human judgement, and the photo is the only evidence for it.
---

# 0140 — Guition panel LCARS restyle (Star Trek TNG feel)

## Context
The Guition JC4827W543 panel (480x272, ESPHome) shows a BMS page fed by four Home
Assistant imports (`bms_soc`, `bms_v`, `bms_i`, `bms_t`) plus local panel state
(`batt_pct`, `batt_state` "CHG"/"BATT", `ha_time`). Stephen wants the page restyled
with a Star Trek TNG LCARS feel. Current look: black background, pastel pill labels,
a circular SOC gauge, three stat pills (PACK V / CHARGE / TEMP), segmented color
bars at the bottom, top bar with panel-battery pill + clock + date.

The file is `guition-jc4827w543/jc4827w543.yaml` in the `pagosacabin/cabin-play`
repo — NOT nordtronics. Play projects live in cabin-play; push via SSH from your
laptop as with 0136 (the gh.py token is nordtronics-scoped and 403s on cabin-play
writes).

## Task
Restyle the BMS page display lambda in LCARS fashion:
- LCARS palette: orange #FF9900 / #FFCC99, salmon #CC6666, purple #CC99CC,
  periwinkle #9999FF / #9999CC, pale yellow #FFFFCC, on black #000000.
- Signature LCARS elbow bars (L-shaped rounded frames) framing the sections.
- Keep the big SOC readout prominent — keep the dial or convert it to an LCARS
  bar, your call.
- Keep all data on the page: PACK SOC %, PACK V, CHARGE current, TEMP, panel
  battery pill (e.g. PANEL CHG), clock, date.
- Preserve existing behaviors: "--" for NAN values, the low-battery page-select
  logic (`batt_pct` < `low_batt_pct` selects the status page), inactivity dimming,
  and the once-a-minute repaint trigger.

## Success criteria
1. `jc4827w543.yaml` compiles under ESPHome 2026.9.1 with no errors or new
   warnings.
2. The BMS page shows all seven data points (SOC, V, A, temp, panel state, time,
   date) in LCARS styling.
3. NAN still renders "--"; page-select and dimming behaviors unchanged — verify
   by reading the lambda, not just by compiling.
4. Photo of the physical panel displaying the new LCARS BMS page.
5. Committed and pushed to cabin-play; the staged reply states the commit SHA.

## Constraints
- Touch the display section only (plus color/font definitions as needed). Do NOT
  touch sensors, wifi, api, ota, ble_client, esp32_ble_tracker, the watchdog
  (`globals` / `interval` / `on_boot` / `on_value`), or the Home Assistant imports.
- 480x272: keep the primary numbers legible — no LCARS micro-text for SOC/V/A/temp.
- Keep model/provider cost economical; no premium tier needed for this task.
- Do not disturb the living-frame project files in the same repo.

## Proof
- Commit SHA on cabin-play origin containing the restyled lambda.
- Photo of the panel showing the LCARS BMS page (physical render proof, not just
  a compile log).
- ESPHome compile output showing zero errors.

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the photo, the
compile result, and notes on any behavior intentionally changed.

---

# Reply — 0140 staged: LCARS elbow frame on the BMS page; photo not obtainable from this host

**Status: staged. Criteria 1, 2, 3, 5 met. Criterion 4 (a photograph of the physical
panel) is NOT met, and I am not claiming it** — this host has no camera that can see the
panel. Details and the one-minute fix are in "The photo gap" below.

Deliverable: `655f25515aff52f3fa96c62908d8c16baa9e0f90` on `pagosacabin/cabin-play@main`,
pushed over SSH (the github skill's API credential is nordtronics-scoped and 403s on
cabin-play writes, as 0136 and 0138 recorded).

## What changed

The BMS page lambda is now framed as a TNG console. The signature shapes are **elbow
bars**, and each one is drawn as the union of two bars **minus a black disc at the inner
corner** — that concave cut is the whole idiom; two bars merely crossed at a right angle
read as a plus sign, not an elbow. Both bars are 27 px across, so their end caps are the
same 13 px disc and the outer corner is a single round arc.

| element | geometry | colour |
|---|---|---|
| left column | x 8-34, y 4-263: turns into the header bar at the top and the footer bar at the bottom | `#FF9900` |
| header bar | panel segment x 8-209, clock segment x 240-471, joined by a small bar x 206-243 | `#FF9900` / `#FFCC99` join |
| right elbow | x 445-471, y 4-63, turning down off the clock segment | `#9999FF` |
| section divider | x 44-436, y 35-41 (stops short of the right elbow so the elbow stays one shape) | `#FFCC99` |
| dial | centre (258, 118), 300° ring drawn as overlapping discs every 3° | `#FF9900`, `#CC6666` below 25 %, unlit `#2C2C50` |
| data columns | three pills at y 192-213, x 44 / 189 / 334, widths 139 / 139 / 138, values 28 pt below | `#9999FF`, `#FF9900`, `#CC99CC` |
| footer | five blocks spanning x 8-471 at y 255-263 | all five frame colours |

The palette is the task's hex list verbatim, declared in the lambda per colour: `#FF9900`,
`#FFCC99`, `#CC6666`, `#CC99CC`, `#9999FF`, `#FFFFCC`, on black. The one colour left in
service that is *not* on that list is `#2C2C50` for the dial's unlit arc — a functional
colour, not a console one, and declared as such in the README.

The SOC dial is **kept** rather than converted to a bar (the task left that open): it is
the primary readout. The number inside it is now `#FFFFCC`, the brightest thing on the
page, which is where the eye should land first. All seven data points are on the page:
SOC number, PACK V, CHARGE/DISCHG current, TEMP, the panel's own state pill, clock, date.

## 1. Compiles under ESPHome 2026.9.1, no errors, no new warnings — MET

The baseline was measured first, so "no new warnings" is a comparison rather than a
claim, and it was forced to a real rebuild (`rm -rf .esphome/build/jc4827w543/src`) rather
than trusting the cached no-op build — so the generated `main.cpp`, the translation unit
my change lives in, is genuinely compiled in both runs:

```
BASE  esphome compile jc4827w543.yaml   exit 0   97 CXX objects incl. main.cpp.obj
      compiler warnings 0;  config warnings 2
NEW   esphome compile jc4827w543.yaml   exit 0   main.cpp.obj rebuilt, linked
      compiler warnings 0;  config warnings 2   (byte-identical text to the baseline pair)
```

Both config warnings are pre-existing and deliberate — the file's own comment keeps the
`qspi_dbi` platform because `mipi_spi` drove this panel into rainbow noise on 2026-10-13,
and `board_build.flash_mode` is a vendor-setting note that esp-idf ignores. The final
compile after the comment/README edits: exit 0, `Successfully compiled program`,
config_hash `0xc50f5cb9`, build_time_str `2026-10-16 13:20:44 -0600`.

## 2. All seven data points, LCARS styling — MET (geometry + device evidence)

`tools/preview.py` was updated to mirror the new geometry and re-rendered, and a separate
audit script re-derives every box from the lambda's own numbers and checks the pixels in
that render. 43 checks, all PASS:

```
palette   all six hexes present as exact fills -- orange 17173 px, periwinkle 9949,
          gold 3901, purple 3338, pale yellow 1357, salmon 540
elbows    top-left / right / footer inner corners black (the concave cut applied);
          outer corners filled; column 27 px wide (x 8..34, x 35 black); right elbow ends y 63
dial      soc 0 / 25 / 50 / 86 / 100 % -> arc 0.0 / 24.8 / 48.5 / 84.2 / 100.0 %
          soc 18 % -> 1297 salmon px (the below-25 % alarm)
text      PANEL 91% (58,7)-(161,21) inside its segment; clock (331,7)-(381,21) inside its
          segment; 48 pt "100" (218,94)-(299,128) inside the ring (inner radius 54);
          label, date and all three pill labels inside their boxes; all three 28 pt values
          inside their columns and above the footer; "--" at 28 pt (105,218)-(121,228)
          inside the column -- i.e. the NAN state fits too
canvas    ink spans x 8..471, y 4..263: nothing is drawn off the 480x272 canvas
```

Device side, after the flash, from the panel's own console:
`[13:27:00.916][I][bms:536]: page=bms soc=100 V=56.83 I=+0.0 T=63.3 local=100%` — the
once-a-minute repaint ran and the page select chose the BMS page, with the four
HA-imported values live.

Render: `preview-bms-lcars.png`, raw URL in the proof block, sha256
`5edaaf74a48469ddc8aea7f5b2b50d8f23dba8c4c97f9597840779a5185082d1`. It is a 480x588
sheet with the BMS page above the unchanged status fallback, so both pages are visible in
one artifact.

## 3. NAN still renders "--"; page select, dimming and the minute repaint unchanged — MET

Read out of the lambda, not inferred from the compile — which is what the task asked for.
Every value the page shows has its own guard, and each guard is unchanged from the base
revision:

```
line 338  if (isnan(local)) snprintf(buf, sizeof(buf), "PANEL --");        panel state
line 357  if (have_time) ... "%H:%M" ... else "--:--"                      clock
line 389  if (isnan(soc)) snprintf(buf, sizeof(buf), "--");                SOC number
line 395  date printed via ha_time.strftime("%a %d %b") -- blank when there is no time
line 408  if (isnan(id(bms_v).state)) ... "--"                             pack volts
line 413  if (isnan(cur)) ... "--"                                         current
line 417  if (isnan(id(bms_t).state)) ... "--"                             temperature
```

The page-select fallback (`if (!isnan(local) && local < ${low_batt_pct})`, base lines
266-287) is **byte-identical**; so are the inactivity-dimming script/number/button blocks,
the `time:` once-a-minute repaint block, the `light`/`output` backlight blocks and the
entire watchdog region. The structural diff is the proof of that: no line outside base
8-10 and base 290-394 differs, and both of those regions are the display section I was
told to touch (the top comment describes the display).

One behavioural detail worth naming, because it moved with the labels and not with the
logic: `CHARGE` / `DISCHG` still follows the sign of the BMS current, and the header still
prints `PANEL CHG` instead of a percentage while the charger holds the BAT net — neither
condition changed, only the coordinates it is drawn at.

## 4. The photo gap — NOT MET, declared

Facts, in the order I checked them:

- The host has exactly one camera: `/dev/video0`, `Syntek Integrated Camera` — the
  laptop's own lid webcam, aimed at the room (`/dev/video1` is its metadata node).
- Four 1280x720 frames were captured around the flash (one before, three after, while the
  panel's log showed the backlight at 100 %): **0 pixels** match the page's `#FF9900` at
  any reasonable tolerance, and only 3.0-3.7 % of each frame is near-black (<45). The BMS
  page is a black field carrying a 27x260 solid orange column and orange arcs, so even a
  small, oblique or partly occluded view of it would put thousands of orange pixels and a
  large contiguous black region in frame. A loosened warm-hue test finds 3.4-4.0 % of
  pixels spread across the *entire* frame bbox — scene warmth (skin, wood, room light),
  not a display.
- The panel has no screenshot path of its own: it is an ESPHome `qspi_dbi` panel driven
  write-only over QSPI — no framebuffer readback, no web server, no camera. "Show the
  pixels to the host" does not exist on this board.

So the photo needs a person with a camera standing at the bench. It is a minute's work:
touch the glass (or press **Wake screen** in Home Assistant) to bring the backlight back
to full — it dims to the Idle brightness setting 60 s after the last touch — and then
photograph the panel. Nothing else is required: the flash is already done.

What the flash *does* prove about the physical device is that the new image is the one
running on it: OTA reported `Update took 9.00 seconds (prepare 0.10, upload 8.60, commit
0.30)` and `OTA successful` for `jc4827w543.bin` (876576 bytes, md5
`3bf3dba789714fdeee56bd0e4ef9bc91`, sha256
`2e33d46043021458a2577084209466d16a4b46784b1dda0e74198f22a922f056`); the boot banner
afterwards reads `[13:26:18.008][I][app:151]: ESPHome version 2026.9.1 compiled on
2026-10-16 13:20:44 -0600` — the exact build time of this revision — followed by
`[13:26:53.129][I][safe_mode:154]: Boot seems successful; resetting boot loop counter`.
Before the flash the panel was at `Battery 100 %`, `Panel battery state CHG`,
`Display Backlight ON 100 %`.

## 5. Committed and pushed to cabin-play — MET

Commit `655f25515aff52f3fa96c62908d8c16baa9e0f90` on `main`, four files, pushed over SSH
and re-verified on origin at blob level (hashes in the proof block).
`raw.githubusercontent.com` serves both the yaml and the render at HTTP 200, and their
sha256 values equal the local files byte for byte.

## ntfy receipt

Topic `nordtronics-build-ed05a663`, id `dmibMfluZjn1`, published 2026-10-16T19:28:13Z. The
spec names no topic; this is the topic the worker prompt and every prior build reply use.
`Workflow` and `Artifacts` read `none` with the reason inline, because cabin-play has no CI
(the enumeration is in the proof block) — the same shape 0138 used.

## Corrections and declared deviations

1. The filed task carried `status: filed`, which is not a protocol state; normalised to
   `in_progress` at pickup and declared in the pickup commit (`d83a853`).
2. Three files beyond the spec's one — `tools/preview.py`, `README.md` and the committed
   render PNG — each with its reason in `notes:`.
3. The README's dial bullet was already stale (it described quads; the code has drawn
   overlapping discs since `ee5f40f`). Corrected here and marked as a correction.
4. The panel was OTA-flashed. Declared above: OTA only, nothing erased, NVS untouched,
   no USB connection.
5. What I did *not* verify is what the glass actually looks like. The render and the
   text-box audit prove the geometry the lambda draws; whether it reads well at arm's
   length is a human judgement, and criterion 4's photo is the only evidence for it.
