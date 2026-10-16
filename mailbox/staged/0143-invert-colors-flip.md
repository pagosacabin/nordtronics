---
task_id: "0143"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - repo: pagosacabin/cabin-play
    branch: main
    sha: 2ee02d829e0f7a7dbe0e735a77b081f8fc3d2c82
    role: revision-compared-against (UNCHANGED — no commit was made to cabin-play)
  - run: none
  - files: []
proof_note: |
  This task's deliverable was conditional: flip `invert_colors` and re-flash ONLY if the
  photo shows the render's complement. The comparison says it does not, so NO config change
  was made and NOTHING was flashed. There is therefore no branch, no commit and no CI run to
  point at, and I am not manufacturing one (the same shape as the 0141 apply-task note).

  What `proof` CAN point at, and what a verifier can re-derive independently:

  - The two artifacts compared, both already committed and byte-identified here:
      mailbox/files/0143-panel-photo.jpg   sha256 dd9c5947c43dc620e9bee5a5b5f03eddfea6ba17a279d2db0343fd873418d5fb
      cabin-play guition-jc4827w543/preview-bms-lcars.png
                                           sha256 d9377704d97d6b470223ada1da5865fd5b6968848c619a707c03e170e2d1cd4e
      cabin-play guition-jc4827w543/jc4827w543.yaml
                                           sha256 c379f90629f5856ade42e1973fa78046269d7e5c3cf76c9858d54794f0849565
    `git ls-remote origin refs/heads/main` on pagosacabin/cabin-play returned
    2ee02d829e0f7a7dbe0e735a77b081f8fc3d2c82 both before and after this run — the revision the
    comparison was made against, not a deliverable. Nothing was pushed to cabin-play.
  - `run: none` is earned by enumeration, the same enumeration 0141 recorded: cabin-play ships no
    CI (`git ls-tree -r --name-only origin/main | grep -c '^\.github'` -> 0) and no nordtronics
    workflow covers that tree (their triggers are branch allow-lists and path filters under
    android/, backend/, firmware/, python/, website/). This run pushed nothing to either tree, so
    no run could exist even in principle.
  - `files: []` — no repository file changed anywhere: not in cabin-play, and not in nordtronics
    (only this mailbox file moves, which is the protocol transition, not the deliverable).
  - ARTIFACT-STATE EVIDENCE, read-only, re-checkable in one call: the panel at 192.168.1.72
    answered on its ESPHome API (keyless, port 6053) with name=jc4827w543,
    model=esp32-s3-devkitc-1, mac=44:BD:8D:7A:CC:CC (the board this repo documents),
    esphome_version=2026.9.1 and compilation_time="2026-10-16 14:28:11 -0600" — i.e. the glass is
    running the 0141 revision, which is what the photo claims to show and what makes this
    comparison meaningful. Six lines of Python against aioesphomeapi; nothing was written to the
    device.

  ntfy: not published, and the reason is in the reply body — this task produced no compiled
  artifact because nothing was built, so the build-green receipt the README requires of build
  tasks does not apply here.
notes: |
  STAGED 2026-10-16 22:35 UTC / 16:35 MDT. Off-peak (PEAK: OFF-PEAK 22:15 UTC). PROTOCOL: MATCH
  protocol_version=1.0.0 (origin/main:mailbox/README.md). Gate: mailbox changed ->
  main-not-at-origin/main + inbox-non-empty.

  QUEUE STATE AT PICKUP, and why 0143 rather than 0142. mailbox/active/ held only .gitkeep. The
  lowest task number in the queue was 0142, but its own front-matter carries
  `hold_until: "2026-10-17T08:00:00-06:00"` and its body says "Do NOT execute before Saturday
  2026-10-17, 08:00 MDT. Picking this up Friday defeats the purpose." At pickup the clock read
  Fri 2026-10-16 16:15 MDT, ~16 h short of the hold, so 0142 was left in inbox/ untouched by
  design (the same call the 0141 run made) and 0143 — the next task number — was picked. 0142
  remains the lowest actionable task for Saturday 08:00 MDT.

  THIS TASK RESOLVED TO A REPORT, NOT A CONFIG CHANGE, and that difference is how the reply
  should be read. Success criterion 1 is "Photo-vs-render comparison recorded (inversion
  confirmed or not)"; criteria 2 and 3 are explicitly conditional on confirmation; and the
  constraint block settles the rest: "If the photo does NOT show complement colors (inversion not
  confirmed): do NOT flip. Report what you see instead." I record NOT CONFIRMED, so there is no
  invert_colors edit, no rebuild, no OTA, no flash, and the panel's running image is untouched
  (still the 0141 build, verified above). The panel was contacted read-only. No deviation was
  taken: the only physical action available was the one the spec forbade, and I took none.

  WHAT DECIDES IT. I rectified the photo onto the page (PIL QUAD perspective warp, corners
  hill-climbed against the render's bright/dark mask: screen at x 195-1232, y 245-842 of the
  1536x2048 frame, aspect 1.74 vs the page's 1.765; mask agreement 0.777) rather than eyeballing
  it. Then: (1) the structure matches feature for feature — elbow column, header bar, the
  89 %-wide meter bar at 78-86 % of the height, footer row, and the 80 pt SOC digits as bright
  strokes in the dark field — so the glass is running this page; (2) the chroma is wrong and
  reproduces Stephen's complaint deterministically (peach -> light cyan, orange -> pale lime,
  lavender -> pale lime, black field -> lifted slate blue); but (3) the image is NOT inverted. An
  RGB polarity inversion — exactly what `invert_colors: true` sends, ESPHome `qspi_dbi` writing
  0x21/INVON from reset_params_ after the init sequence — flips lightness as well as hue, so a
  complement of this render is a near-white field with dark navy/azure bars, because the render's
  field is 75 % black. The photo is the other way round: at mid-panel the render's brightest large
  area (the peach elbow) is BRIGHT on the glass (L 190-198) and the render's black field is the
  DARKER of the two (L 97-112); and the meter bar, which spans 89 % of the page width, is the
  BRIGHTEST band on the panel (L 235) where its complement (azure #0066FF, L 89) would be one of
  the darkest things on a white page. Bright bars on a dark field is the render's structure, not
  its inverse. So the hue moved and the polarity did not — the flip is not the fix the evidence
  points at, and a true INVOFF against an already-non-inverted image would plausibly make the
  panel worse.

  TWO THINGS WORTH KEEPING, because they cost time here and will cost it again. (a)
  `preview-bms-lcars.png` is a 480x588 TWO-PAGE SHEET: `tools/preview.py` has `head = 22` and
  pastes the BMS page at y = 22, so the page is rows 22-293. My first pass compared against row N
  of the PNG and produced a nonsense per-colour correspondence; every number in the reply is from
  the corrected crop (rows 22-294). Anyone re-deriving this should crop first. (b) The render's
  committed state is soc=86 while the glass is at soc=100, so the panel's meter fill covers the
  whole track and no lavender track is visible on it — a state difference, not a palette one.

  NOT APPLIED, offered as the next discriminating step (I verified the shape, I did not do it): a
  single full-screen known colour, flashed and photographed, maps each channel unambiguously in
  one frame. Today's 7-colour page cannot separate "the panel's channels are crossed" from "the
  capture pipeline is doing something", because every page colour differs from every other in
  more than one channel. A lambda that fills 480x272 with #FF0000 / #00FF00 / #0000FF (or one
  frame of three bands) settles it in one photo, and the OTA path is proven. That is a task-sized
  change to the lambda, so it belongs in a task rather than in this reply's own authority.
---

# 0143 — Guition: flip invert_colors, reflash, verify against panel photo

## Context
0141's staged reply proved the palette complaint does not reproduce in any
artifact: the renders are canonical orange (#FF9900), but Stephen's panel
photos show blue/lime. The mechanism is `invert_colors: true` in the display
block → the driver sends INVON (0x21) → every color displays as its complement.
ESPHome's own JC4827W543 preset sets it true; the vendor's factory image drives
the panel with INVOFF (no 0x20/0x21 record in the init table). Stephen has
approved the flip experiment.

Stephen's photo of the current screen is committed alongside this task at
`mailbox/files/0143-panel-photo.jpg` — CORRECTION (Stephen, 15:23 MDT): the photo
(sent 14:47 MDT) shows the CURRENT 0141 build on the glass (flashed 14:28 MDT,
"compiled on 2026-10-16 14:28:11 -0600"), not the 0140 build. So the inversion is
eating 0141's canonical palette too: blue where the render has orange, lime where
the render has purple. His verdict: "things are wonky." The layout crowding is
therefore LIVE in the current build: SOC pill cramped against the frame, label
pills dwarfed by the values, a wide bottom bar not earning its space. Sequence:
flip the colors first (this photo's primary job is color comparison), then the
layout gets its own pass against the corrected-color build.

## Task
1. Compare `mailbox/files/0143-panel-photo.jpg` against your
   `preview-bms-lcars.png` render. If the photo's colors are the complement of
   the render's (blue↔orange, lime↔purple), the inversion is confirmed.
2. Set `invert_colors: false` in the display block. One line, nothing else.
3. Rebuild and OTA-flash the panel (192.168.1.72, as with 0140/0141).
4. Report the new "compiled on" timestamp so Stephen knows which build is on
   the glass.

## Constraints
- The ONLY config change is the `invert_colors` line. Touch nothing else.
- If the photo does NOT show complement colors (inversion not confirmed):
  do NOT flip. Report what you see instead.
- Same push path as 0140/0141: SSH from the bench host to pagosacabin/cabin-play
  (the gh.py token is nordtronics-scoped and 403s on cabin-play writes).
- Keep model/provider cost economical; no premium tier needed.

## Success criteria
1. Photo-vs-render comparison recorded (inversion confirmed or not).
2. If confirmed: `invert_colors: false` committed, panel OTA-flashed, new build
   timestamp reported.
3. Stephen photographs the panel; the photo shows the canonical orange palette.

## Proof
- Commit SHA on cabin-play origin.
- The comparison note (which regions are complements).
- The new "compiled on" timestamp read back from the panel.

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the comparison
result, the new build timestamp, and notes on anything unexpected.

---

# Reply — 0143 staged: comparison recorded, **inversion NOT confirmed**; `invert_colors` NOT flipped and the panel was NOT re-flashed

**Status: staged. Success criterion 1 met and recorded below. Criteria 2 and 3 are conditional on
confirmation and do not apply: the photo is not the render's complement. No config change, no
rebuild, no OTA, no flash — the glass still carries the 0141 image.**

The constraint block is explicit — "If the photo does NOT show complement colors (inversion not
confirmed): do NOT flip. Report what you see instead" — so this is that report: the measurement
that decides it, and the evidence behind each step.

## Verified before anything was concluded

**The panel is running the 0141 build** — read back from the device itself, read-only, over its
ESPHome API (keyless, port 6053 on 192.168.1.72), not inferred from the mailbox:

```
name: jc4827w543
esphome_version: 2026.9.1
compilation_time: 2026-10-16 14:28:11 -0600
model: esp32-s3-devkitc-1 | mac: 44:BD:8D:7A:CC:CC
Display Backlight = True | Panel battery state = CHG | Battery = 100.0 | Battery voltage = 4.28
```

`14:28:11 -0600` is the 0141 build window, so Juno's correction in the Context block is right: the
photo shows 0141, not 0140. The MAC is the board this repo documents. That is what makes the
comparison meaningful — the photo is of the revision under discussion.

The photo itself: 1536x2048 JPEG, Exif `DateTime 2026:10:16 14:46:42`, `Galaxy A35 5G` — the
14:47 MDT capture the task refers to. sha256
`dd9c5947c43dc620e9bee5a5b5f03eddfea6ba17a279d2db0343fd873418d5fb`.

## The comparison, and how it was made

I warped the photo onto the page rather than eyeballing it: PIL `QUAD` perspective transform, the
panel's border located by hill-climbing the four corners against the render's bright/dark mask.
The screen occupies roughly **x 195-1232, y 245-842** of the 1536x2048 frame — aspect **1.74**
against the page's **1.765** — and the bright/dark mask then agrees with the render on **0.777**
of pixels. Features line up individually, which is what makes the per-colour numbers below worth
quoting.

One trap worth writing down, because it invalidated my first pass: **`preview-bms-lcars.png` is a
480x588 two-page sheet, not the page.** `tools/preview.py` sets `head = 22` and pastes the BMS
page at `y = 22`, so the page is **rows 22-293**. Comparing a photo against "row N" of that PNG is
22 rows off, and manufactures exactly the kind of wrong correspondence I got on the first attempt.
Every number below uses the corrected crop.

### 1. The panel is showing this page — MET (structure matches)

Feature for feature in the rectified pair: the full-height elbow column at the left edge, the
full-width header bar, the 89 %-wide meter bar at 78-86 % of the height, the footer block row, and
the 80 pt state-of-charge digits as bright strokes inside the dark field. Not a stale image, not
the fallback page.

### 2. The photograph reproduces Stephen's complaint — "blue/lime, not orange" is REAL

Measured at independent locations, deterministic and repeatable:

| render (cabin-play @2ee02d8) | panel in the photo | pixels sampled |
|---|---|---|
| peach `#FFCC99` — elbow column + header bar | light cyan ~`#7CE1FD` / `#82E4FD` | 16 800 |
| orange `#FF9900` — SOC pill, meter fill | pale lime ~`#DFFDA4` / `#E0FEA4` | 11 000 |
| lavender `#9999CC` — meter track | pale lime ~`#DEFE8F` | 2 200 |
| black `#000000` — the field | lifted slate blue ~`#4C6982` | 79 600 |

The column reading was spot-checked at two heights (page y 12, y 150) and the orange reading at
six points across the meter band and the SOC pill; each returned the same answer. So warm in, cool
out — and it is not an artefact of one region.

### 3. But it is NOT the complement — NOT MET (the verdict)

An RGB polarity inversion — precisely what `invert_colors: true` sends: ESPHome's `qspi_dbi`
writes command **0x21 (INVON)** from `reset_params_()` *after* the init sequence
(`qspi_dbi.cpp:98`; `INVERT_ON = 0x21`, `qspi_dbi.h:21`) — flips **lightness as well as hue**. The
complement of this render is a **near-white field with dark navy/azure bars**, because the
render's field is 75 % black. The photo is the other way round:

- **Largest-area test, no registration needed.** The render's brightest large area is the peach
  elbow (L 213), its darkest the field (L 0). Along a line at mid-panel the photo shows the
  column band **bright** (L 190-198, `#7DE0FD`) and the field band the **darker** of the two
  (L 97-112, `#4C6583`). In the complement the field would be the brightest thing on the glass.
  It is not.
- **Second, independent check.** The meter bar spans 89 % of the page width. On the panel it is
  the **brightest band on the screen** (L 235, `#E1FDA5`). The complement of that orange bar is
  azure `#0066FF` (L 89) on a white page — the band would be among the *darker* things.
- **Third.** The 80 pt SOC digits read as bright strokes on a darker field; their pale colour
  `#FFFF99` complements to `#000066`, so an inverted page would show them dark on a white field.
  The photo shows the render's arrangement.

Bright bars on a dark field is the render's structure, not its inverse. The hue moved; the
polarity did not.

## What that means for the flip

The Context has two parts and the photo supports only the first. "Stephen's panel photos show
blue/lime" — **true, reproduced here.** "The mechanism is `invert_colors: true` → every color
displays as its complement" — **not supported by this photo.** The one colour-polarity knob in the
config does not explain what the glass is doing, and a flip is not the fix the evidence points at:
if the glass is currently non-inverted, a true INVOFF renders the white-field/dark-bar picture —
plausibly *worse*.

So: `invert_colors` was **not** touched, nothing was rebuilt, nothing was flashed, and
`pagosacabin/cabin-play` is still at `2ee02d829e0f7a7dbe0e735a77b081f8fc3d2c82` (verified on
origin before and after — the revision compared against, not a deliverable). No commit exists
anywhere for this task, so `proof.run` is `none` and `proof.files` is empty, with the reason
enumerated in the proof block rather than left as a gap.

## What I did not determine, and the one test that would

I can say what the panel is *not* doing (inverting polarity) more confidently than what it *is*
doing. The chroma shift is real but fits no single simple model I could test: it is not the RGB
complement; not a hue rotation (peach and orange share almost the same render hue yet come out
cyan and lime); and not an R/B channel swap (that puts orange at azure; the meter is lime).
Candidates still live, and this photo cannot separate them: the panel module's own colour-filter
order, a COLMOD/MADCTL `BGR` mismatch in the display pipeline, or a capture-side effect.

**One frame would settle it.** A full-screen page of pure `#FF0000`, then `#00FF00`, then
`#0000FF` (or all three as bands in one frame), flashed and photographed, maps each channel
unambiguously — every colour on today's page differs from every other in more than one channel,
which is exactly why this photo is ambiguous. The OTA path to 192.168.1.72 is proven, so it is a
small change to the lambda. That is a new task, not something to invent inside this one's
authority.

## ntfy

Not published: this is not a build task and no compiled artifact was produced, because nothing was
built. Quoting a build receipt here would be manufacturing evidence for work that did not happen.

## Summary for Stephen

Your photo is doing its job — the panel really is showing cool colours where the page is
canonically orange, and that is now measured rather than described. But the flip you approved would
not fix it: the panel is not displaying an inverted image (its dark field is still the dark field
and its bright bars are still the bright bars; a true inversion would make the field white). I left
the config and the panel alone rather than apply a change the photo argues against. The likely next
step is a one-frame pure-R/G/B test page, not another flip.
