---
task_id: "0078"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  branch: hermes/0078-wildfire-node-rev-c-text-fix
  sha: 7dc24165d20d7a99fdccda366e70716824e7a3ce
  run: none — no workflow in this repo triggers on this branch or these paths (enumeration below)
  main_transition_run: https://github.com/pagosacabin/nordtronics/actions/runs/36646354501
  files:
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sch
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c-erc.txt
  renders:
    pdf: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf — vector, 1 page, 841.896 x 595.296 pt (A4 landscape)
    png: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png — 7016 x 4961 px, density tag 236.22 px/cm = 600 dpi at A4 landscape, sRGB
  erc: 0 errors / 30 warnings — all [lib_symbol_mismatch]; the title-block ERC line matches the committed report
  netlist: 19 multi-pin nets / 77 connected pins / 7 NC singletons — identical to the 0077 revision, member-for-member
  model: deepseek-flash
---

# 0078 — Fix upside-down J1 description text on the Rev C schematic

**Status: staged.** Deliverable is on `hermes/0078-wildfire-node-rev-c-text-fix` @
`7dc24165d20d7a99fdccda366e70716824e7a3ce`. Re-read from the remote after the push:
`git ls-remote --heads origin hermes/0078-wildfire-node-rev-c-text-fix` and
`gh api repos/pagosacabin/nordtronics/branches/hermes/0078-wildfire-node-rev-c-text-fix`
both report that SHA as the tip, and the local branch tip equals it.

## Reply — what changed

Three numeric values in the `.kicad_sch` — text and position only:

1. **J1's `BATT IN (JST-PH 2.0mm 2-pin PROPOSED)` string now plots upright.** The
   field's stored angle was `180` on a symbol whose own rotation is `180`, and that
   combination made the plotter emit the text under `rotate(-180)` — a true 180°
   rotation, not a mirror. The angle is now `0`, and the exported vector carries **no
   rotate transform** for the string.
2. **The same one-token fix on J1's `Reference` (`J1`)** — it was rotated 180° by the
   same mechanism and sits in the same field pair. Declared as a scope note below.
3. **`TP1`'s refdes nudged one 1.27 mm grid step up-left**: stored
   `(at 37.16 15.13 0)` -> `(at 35.89 13.86 0)`. Its glyph box previously shared the
   `y = 15.71 mm` edge with `F1`'s refdes box (measured gap **0.000 mm**, ~0.08 mm
   between painted strokes); it is now **1.270 mm** clear.

No component, value, pin, wire, label or net changed; the file diff is 3 lines
(`git show --stat`: 6 insertions / 3 deletions in one file = the three values).

## How it was verified (what I actually ran)

1. **The mechanism, by probe — not by assertion.** A copy of the sheet with J1's angle
   set to `0` was exported and compared: angle `180` -> `<g transform="rotate(-180 …)">`
   on the plotted string; angle `0` -> no transform. The fix is the value the probe
   proved, not a guess.
2. **The render, measured.** The committed SVG export was parsed back: 274 plotted
   strings with their exact mm glyph boxes and their plotter rotation. **No string on
   the sheet plots at 180° any more** (the only non-zero rotations are the `-90`
   vertical net labels and pin decorations, which are correct). The `BATT IN …` string
   is at x 12.70–54.36 mm, y 28.13–30.06 mm with rotation 0.
3. **TP1/F1 separation, measured before and after.** 0077: TP1 box
   (35.557, 14.440)–(38.642, 15.710), F1 box (34.793, 15.710)–(36.547, 16.980), gap
   **0.000 mm**. 0078: TP1 box (34.287, 13.169)–(37.372, 14.440), F1 unchanged, gap
   **1.270 mm**. Minimum gap between *any* TP1 label and *any* F1 label: 1.270 mm.
4. **Independent visual verification (a second agent, eyes on the pixels).** It read the
   glyphs out of the raster and the SVG: the AFTER string is upright and left-to-right
   (`B` bowls right, `A` apex up, `T` bars on top); the BEFORE string is a true 180°
   rotation — patch IoU of AFTER vs BEFORE **0.19** unrotated but **0.93** with BEFORE
   rotated 180°, and 0.43 / 0.20 for the two single-axis mirrors, so it is a rotation and
   not a mirror. It also measured TP1/F1 as **1.14 mm** apart stroke-to-stroke in AFTER
   (0.077 mm in BEFORE — visually one collided block), confirmed `J1` and `TP1` read
   upright, and found no text clipped by the frame, no string over another string, and no
   wire crossing a label in that corner. Full-sheet check: A4 landscape, title block
   present and populated, every frame zone digit present, and all nine page regions at
   the same ink density as BEFORE.
5. **Nothing else moved — proved on the raster.** The 0077 and 0078 PNGs differ in
   **16,561 px = 0.0476 %** of the sheet (threshold 20/255), in exactly **3** clusters:
   J1's value string (12.61–54.44 x 27.85–30.14 mm), J1's refdes (16.34–18.37 x
   16.76–18.33 mm) and TP1's refdes (34.20–38.73 x 13.08–15.79 mm — the union of its old
   and new positions). Zero unexpected changed regions. The string-set diff of the two
   vectors shows the same three labels and nothing else.
6. **Netlist re-verified against 0077** (`kicad-cli sch export netlist --format
   kicadxml` on both revisions, partition compared programmatically): 26 nets on both
   sides, **19 multi-pin nets / 77 connected pins / 7 single-pin NC nets**, identical net
   names, identical member sets for every shared net, identical NC pins (U2.3/4/6/7/8,
   U4.5/6). **NETLIST CONTRACT: PASS.** A merged net (short) or a split net (open) would
   show up in exactly this check. No regressions either: text-vs-text overlaps > 0.4 mm²
   = 0, wires crossing a text glyph box = 0, wire segments inside the notes/title
   keep-outs = 0, and all 30 refdes, all 17 note lines and every net label still present
   in the render.
7. **ERC re-run**: `kicad-cli sch erc --severity-all --format report` -> "Found 30
   violations", histogram `30 [lib_symbol_mismatch]` — i.e. **0 errors / 30 warnings**,
   identical to 0077 (Flatpak KiCad 10 library copies vs the embedded cache; not a
   connectivity fault). Committed as `wildfire-node-v1-rev-c-erc.txt`; the title-block
   line still reads "ERC 2026-09-29: 0 errors / 30 warnings (all [lib_symbol_mismatch])"
   and matches it.
8. **Renders re-exported from the final `.kicad_sch` and re-opened**: PDF 1 page,
   841.896 x 595.296 pt (A4 landscape); PNG 7016 x 4961 px, 236.22 px/cm = **600 dpi**
   (requirement ≥ 200), sRGB.

## CI — why `run: none` is correct here (enumerated, not assumed)

`ls .github/workflows/` -> `android-build.yml`, `platformio.yml`, `website-check.yml`.
None can fire for this push:

- `android-build.yml`: `on.push.branches: [android-toolchain-setup]` — not this branch.
- `platformio.yml`: `on.push.paths` = `firmware/tank-monitor/**`, `firmware/node-v1/**`,
  `.github/workflows/platformio.yml` — this branch touches `hardware/wildfire-node-v1/**`
  only.
- `website-check.yml`: `on.push.branches: [main, hermes/0068-site-email-refresh,
  hermes/0070-site-rewrite]` — not this branch.

Confirmed live rather than assumed:
`gh api "repos/pagosacabin/nordtronics/actions/runs?branch=hermes/0078-wildfire-node-rev-c-text-fix"`
returns `total_count: 0`. The branch tip SHA above is the pointer for the verifier.

**No ntfy receipt:** the build topic in this repo is for CI builds that produce a
compiled artifact (APK / firmware image). This task compiles nothing — it is a KiCad
schematic plus its exports — and the spec names no topic for it.

## Deviations and scope notes

1. **J1's `Reference` was fixed too.** The task named the description/value string;
   `J1`'s refdes carried the identical defect (it also plotted under `rotate(-180)`,
   verified in the exported vector). Leaving it would have left the same corner
   half-fixed, so the one-token angle change was applied to both fields of J1. Text only;
   nothing semantic.
2. **TP1's refdes was the label that moved** (not F1's). The pair was separated by
   nudging TP1 one grid step up-left, which keeps both labels near their own symbols and
   introduces no new collision — re-measured, not assumed.
3. **The `.kicad_sch` was edited with a scoped, asserting text editor** (three anchored
   values, refusing if a value was already applied) rather than through the KiCad MCP
   field tools, exactly as the 0077 re-layout did. The property tools rewrite the whole
   sheet on save and the delivered artifact is the KiCad file itself; the diff is 3 lines.
4. **Pre-existing, not caused by this change** (identical in the 0077 render, confirmed
   by the independent visual pass): J1's refdes sits inside its own symbol body outline,
   and TP7's refdes has symbol ink within ~2 px. They are 0076/0077 layout artefacts; this
   task was scoped to the two named defects and did not touch them.
5. **`.kicad_pro`, `.kicad_sym` and `sym-lib-table` are untouched** — the sheet was
   edited in place this time, not regenerated, so no root-sheet UUID changed.

---

<!-- original task spec, preserved -->

# 0078 — Fix upside-down J1 description text on the Rev C schematic

## Context

0077 (re-layout) is verified and archived. Juno's visual verification of the
new render found one cosmetic defect: J1's description field,
`BATT IN (JST-PH 2.0mm 2-pin PROPOSED)`, renders rotated 180 degrees —
upside down — at the top-left of the sheet (above J1/F1, near the RXEF075
value string). A second nit in the same corner: the `TP1` and `F1` refdes
labels touch each other.

Front-load the exact state: branch `hermes/0077-wildfire-node-rev-c-relayout` @
`5b629753c0cadcd0dbb44543f17924d11add4700`, files under
`hardware/wildfire-node-v1/`. The 0077 netlist (19 multi-pin nets / 77
connected pins / 7 NC singletons) is frozen and does not change in this task.

## Task

On a new branch `hermes/0078-wildfire-node-rev-c-text-fix`, branched off
`hermes/0077-wildfire-node-rev-c-relayout`:

1. Fix J1's description field angle so the text reads right-side up
   (horizontal, 0 degrees as seen on the sheet — not 180).
2. Nudge the `TP1` and `F1` refdes labels apart so their glyph boxes no longer
   touch.
3. Re-export the vector PDF and the >= 200 dpi PNG from the final
   `.kicad_sch`, re-run ERC, and commit all of them.

Text and nudge only — no component, value, pin, or net changes.

## Success criteria

- [ ] `BATT IN (JST-PH 2.0mm 2-pin PROPOSED)` reads right-side up in the
      exported render (verify by opening the PNG and looking, not by
      asserting the angle value).
- [ ] `TP1` and `F1` labels no longer touch.
- [ ] Netlist re-verified: `kicad-cli` netlist export compared against the
      0077 netlist — 19 multi-pin nets / 77 connected pins / 7 NC singletons,
      identical. Any difference is a failure.
- [ ] ERC re-run: 0 errors expected; the title-block ERC line matches the new
      report.
- [ ] Fresh vector PDF and fresh >= 200 dpi PNG committed.
- [ ] Branch `hermes/0078-wildfire-node-rev-c-text-fix` pushed to origin; SHA
      reported.

## Constraints

- Text placement and field angles only. The 0077 netlist is frozen.
- Worker model is deepseek-flash (Stephen's call).
- Push only to the task branch. No secrets in the repo, ever.

## Proof

- Branch `hermes/0078-wildfire-node-rev-c-text-fix` on origin + SHA, verified
  with `git ls-remote`.
- Netlist comparison PASS line (19 nets / 77 pins / 7 NC singletons,
  identical to 0077).
- ERC summary line; PNG dimensions.

## Reply format

Stage your reply as `mailbox/staged/0078-wildfire-node-rev-c-text-fix.md`
with front matter carrying `task_id: "0078"`, `status: staged`, `iteration`,
and a `proof:` block (branch, sha, files, renders, erc, netlist check,
model), then a Reply section.
