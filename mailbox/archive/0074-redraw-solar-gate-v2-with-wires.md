---
task_id: "0074"
protocol_version: 1.0.0
status: verified
iteration: 3
expect-reply-within: 6h
proof:
  - branch: hermes/0074-solar-gate-v2-wires
    sha: 7b2aa03b1febc6d94edb05a02d97b1f71105e484
  - files:
      - hardware/solar-gate-v2/solar-gate-v2.kicad_sch
      - hardware/solar-gate-v2/solar-gate-v2-schematic.svg
      - hardware/solar-gate-v2/solar-gate-v2-schematic.png
      - hardware/solar-gate-v2/solar-gate-v2-schematic.pdf
      - hardware/solar-gate-v2/solar-gate-v2-erc.txt
      - hardware/solar-gate-v2/README.md
  - no_ci_run: >
      deliberate and verified, not an omission: no workflow matches hardware/**.
      android-build.yml triggers only on branches:[android-toolchain-setup];
      platformio.yml only on paths firmware/**; website-check.yml only on
      branches:[main, hermes/0068-site-email-refresh, hermes/0070-site-rewrite].
      Confirmed live: `gh run list -R pagosacabin/nordtronics --branch
      hermes/0074-solar-gate-v2-wires` returns [] - no runs at all. The task also
      forbids touching anything outside hardware/solar-gate-v2/, so the workflow
      branch list was deliberately NOT extended: a run there would exercise the
      website job, not the schematic, and this repo has no KiCad/ERC job in CI.
  - main_transition_runs:
      - https://github.com/pagosacabin/nordtronics/actions/runs/36381151376
        # Website Check on main, success, headSha 97fe9bc = the 0074 pickup commit
        # this reply rides on. Read back with `gh run view`, not assumed.
  - wires: "grep -c '(wire' = 115 and grep -cE '^[[:space:]]*\(wire\b' = 115 (the card's literal '(wire ' with a trailing space = 0: KiCad 10 writes the token followed by a newline); grep -c '(junction' = 39; labels 14; no_connect 0; bus 0"
  - netlist: 14 nets / 27 components, membership identical to the 0073 capture - pasted below, and re-proven with the labels stripped
  - erc: 0 errors / 22 warnings, all rule [lib_symbol_mismatch]
  - erc_report: hardware/solar-gate-v2/solar-gate-v2-erc.txt (on the branch)
  - renders:
      - hardware/solar-gate-v2/solar-gate-v2-schematic.png  # 7016x4961, A4 landscape, 600 dpi
      - hardware/solar-gate-v2/solar-gate-v2-schematic.pdf
      - hardware/solar-gate-v2/solar-gate-v2-schematic.svg
notes: |
  Model: deepseek-flash (provider deepseek, profile cronrunner). Deliverable: the
  0073 circuit redrawn so every connection is a real wire. Branch tip verified on
  origin with `git ls-remote --heads origin hermes/0074-solar-gate-v2-wires`
  = 7b2aa03b1febc6d94edb05a02d97b1f71105e484 = local HEAD at commit time (2 commits: the schematic + this
  README update).

  Numbers, all measured on the committed revision, none assumed:
  (wire) 115 (81 routed segments, broken at T-points by the tool) and (junction) 39
  vs 0 / 0 in the 0073 capture; net labels down from 68 to 14, now supplements only.
  generate_netlist: 27 components, 14 nets, membership element-for-element identical
  to the 0073 file (compared programmatically). ERC via kicad-cli: 0 errors / 22
  warnings, all [lib_symbol_mismatch], i.e. the same single cosmetic rule 0073
  reported (cause unchanged: the MCP loader embeds host-RPM library bytes while ERC
  compares inside the Flatpak KiCad 10 sandbox). Geometry from the SVG export:
  0 real text-vs-text collisions and 0 text-over-wire overlaps, vs 85 colliding text
  pairs in the 0073 render; no symbol in the title-block region; no label crossing the
  frame. Renders: SVG + PDF via `kicad-cli sch export`, PNG 7016x4961 (600 dpi, A4
  landscape) via magick/RSVG from that SVG, all committed.

  ONE CORRECTION TO THE CARD'S SPEC, deliberate and proven: the card's criterion
  "a grep for `(wire ` returns non-zero" does not hold literally on any KiCad 10 file,
  wired or not - the serializer writes `(wire` + newline, so the trailing space never
  matches. `grep -c '(wire'` gives 115. Reported rather than quietly reworded; the
  token-boundary form is in the project README.

  DEVIATIONS:
  1. U1's six Reference/Value fields were moved by my own tooling. The MCP tools
     select a symbol by reference only (no unit selector), so they cannot address one
     unit of the three-unit LM393: the edit replaces the two `(at x y)` values inside
     each unit's property block, anchored on that unit's `(at 140.97 86.36 0)` etc.
     Everything else - the project, the schematic, all 31 symbol instances, all 115
     wires, every junction, every label and every other field - was written through the
     KiCad MCP server.
  2. Two placement traps found while doing that, now in the README: a field's stored
     angle is relative to the symbol's rotation (a rotated symbol needs 90/270 to
     render horizontal - 0 renders vertical and the ref/value then overlap), and
     justification carries over so on a `justify:left` field the stored `(at x y)` is
     the left edge of the ink, not the centre (this is what put C1/C2's values under
     the capacitor plates on the first pass; caught by measuring, fixed, re-measured).
  3. Not a build task and the spec names no ntfy topic: nothing was published to ntfy.
  4. Footprints still deferred (no fp-lib-table), unchanged from 0073 - layout waits for
     bench validation. No PCB work was started.
  5. The preserved 0073 capture sits in the worktree as solar-gate-v2.kicad_sch.0073 and
     is deliberately NOT committed (it is recoverable from
     origin/hermes/0073-solar-gate-v2-schematic); the branch carries only files under
     hardware/solar-gate-v2/.
  6. hardware/solar-gate-v1/ and everything outside hardware/solar-gate-v2/ is
     untouched. 0072 remains parked as decision-blocked and was not opened.

  Residual cosmetic artifacts, measured, none affecting connectivity or legibility:
  the SVG exporter draws six parts' fields and pin stubs twice at byte-identical
  coordinates (the .kicad_sch holds one instance of each - verified by uuid and property
  counts - so the raster is only ~6% bolder on 12 strings); the two `~` pin-name marks of
  each Device:C overlap each other by 0.02 mm2 (stock symbol shape). Both are documented
  in the README's render-verification section.
---


# 0074 — Redraw solar-gate-v2 schematic WITH wires (rework of 0073)

## Context

Task 0073 delivered a KiCad schematic of the solar-gate-v2 temperature-gate
circuit on branch `hermes/0073-solar-gate-v2-schematic`. It verified clean
(branch SHA matched, ERC 0 errors, netlist validated at 14 nets). But the
capture was drawn with **net labels only — zero `(wire)` entries** in
`solar-gate-v2.kicad_sch` (68 labels, 0 wires, 0 buses, confirmed by grep).
Electrically valid, but not human-readable: Stephen cannot trace signal flow
on a schematic with no connection lines, and he needs to read it for design
review and the bench build.

## Task

One deliverable: a redrawn, fully WIRED KiCad schematic of the same circuit,
in `hardware/solar-gate-v2/`, on a new branch.

Base the new branch on `hermes/0073-solar-gate-v2-schematic` (not main — the
project files only exist there). Redraw `solar-gate-v2.kicad_sch` so that
every connection currently made by a net label is drawn as a real wire.
Keep every component, reference designator, value, and net name identical to
the 0073 capture. Net labels may remain as supplements, but no connection may
rely on labels alone — the schematic must be traceable by eye.

Also export for phone viewing (lesson from 0073): commit a PNG render
(`hardware/solar-gate-v2/solar-gate-v2-schematic.png`, A4 landscape, high
resolution) and a PDF (`hardware/solar-gate-v2/solar-gate-v2-schematic.pdf`)
alongside the SVG. Use `kicad-cli sch export` for both.

Do NOT touch `hardware/solar-gate-v1/` or anything outside
`hardware/solar-gate-v2/`.

## Success criteria

- `solar-gate-v2.kicad_sch` contains real `(wire)` entries connecting the
  circuit; a grep for `(wire ` returns non-zero.
- Netlist identical to the 0073 capture: 14 nets, same membership
  (run the same `generate_netlist` check and paste the net list).
- ERC passes with zero errors (warnings listed and explained).
- All 27 references (Rbias, R1, R2, Rfa, Rpua, Rb1, Rb2, Rfb, Rpub, Rpuc,
  Rb, Rt, Rg, C1, C2, DA, DB, D1, D2, NTC, Q1A, Q1B, Q2A, Q2B, Q3, U1, U2)
  with the exact values from the 0073 spec.
- PNG and PDF renders committed on the branch.
- Committed on a branch named `hermes/0074-solar-gate-v2-wires`, pushed to
  origin.

## Constraints

- Do NOT modify `hardware/solar-gate-v1/` or any other existing project.
- Do not start PCB layout. Schematic only.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- Branch name and SHA on origin.
- `grep -c '(wire ' hardware/solar-gate-v2/solar-gate-v2.kicad_sch` output.
- Net list from `generate_netlist` (all 14 nets with membership).
- ERC output pasted (or report path on the branch).
- PNG and PDF paths on the branch.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field listing any ERC
warnings.

## The card's grep criterion, answered literally first

```
$ grep -c '(wire ' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
0
$ grep -c '(wire' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
115
$ grep -cE '^[[:space:]]*\(wire\b' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
115
$ grep -c '(junction' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
39
$ grep -c '(label ' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
14
$ grep -c '(bus' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
0
$ sed -n '4274,4276p' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
	(wire
		(pts
			(xy 35.56 39.37) (xy 35.56 30.48)
```

The card's pattern `(wire ` ends in a space. KiCad 10 serialises the token as `(wire`
followed by a **newline**, so that literal grep returns 0 on a fully wired sheet — the
one number in this reply that contradicts the card as written, flagged rather than
worked around. The same file on the 0073 branch: `(wire` = 0, `(label ` = 68.

## Connectivity — `generate_netlist`, verbatim

```
Components (27): C1 C2 D1 D2 DA DB NTC Q1A Q1B Q2A Q2B Q3 R1 R2 Rb Rb1 Rb2 Rbias
                 Rfa Rfb Rg Rpua Rpub Rpuc Rt U1 U2

Nets (14):
  /C: DA/2, DB/2, Rb/1, Rpuc/2
  /G: Q1A/1, Q1B/1, Q2A/1, Q2B/1, Q3/3, Rg/2
  /GND: C1/2, C2/2, NTC/2, Q3/1, R2/2, Rb2/2, U1/4, U2/3
  /H: D1/1, D2/1, Rg/1
  /M: Q1A/3, Q1B/3, Q2A/3, Q2B/3
  /N: NTC/1, Rfb/2, Rt/2, U1/2, U1/5
  /OA: DA/1, Rfa/1, Rpua/2, U1/1
  /OB: DB/1, Rfb/1, Rpub/2, U1/7
  /P: C1/1, D1/2, Q1A/2, Q1B/2, Rbias/1, U1/8
  /PA: R1/2, R2/1, Rfa/2, U1/3
  /Q3B: Q3/2, Rb/2
  /SOLAR_OUT: D2/2, Q2A/2, Q2B/2
  /VREF: C2/1, R1/1, Rb1/1, Rbias/2, Rpua/1, Rpub/1, Rpuc/1, Rt/1, U2/1, U2/2
  /Vb: Rb1/2, Rb2/1, U1/6
```

Identical to the 0073 capture: 27 components, 14 nets, membership equal
element-for-element (compared programmatically against the preserved 0073 file, not by
eye). Same 27 reference designators with the same values as the 0073 spec.

## No connection relies on a label — proven, not asserted

The card's clause is "no connection may rely on labels alone". Rather than argue it from
the counts, I stripped all 14 `(label ...)` blocks out of a scratch copy of the committed
file and re-netlisted it:

```
labels removed: 14 | remaining '(label' in the stripped copy: 0
label-stripped copy: components 27, nets 14
membership sets identical to the labelled file: True   (14 net groups, 27 refs)
```

The wires alone reproduce the circuit; the labels only supply the friendly net names
(`/C`, `/GND`, … instead of `Net-(DA-A)`).

## What I checked, and how

- Connectivity: `generate_netlist` on the committed file, compared programmatically with
  the 0073 netlist (27/27 components, 14/14 nets, identical membership), plus the
  label-stripped probe above.
- ERC: `kicad-cli sch erc --severity-all --format report`, report committed to the branch
  so it can be re-read. **0 errors**; 22 warnings, one rule: `[lib_symbol_mismatch]`
  (`** ERC messages: 22  Errors 0  Warnings 22`). That rule is the known cosmetic
  host-RPM-vs-Flatpak library-version mismatch — the same rule and the same count 0073
  reported, and no pin, net or connection is affected. The 0073 reply's explanation
  still applies unchanged.
- The drawing itself: measured from the sheet's own SVG export (KiCad's stroke-font glyph
  paths give exact mm boxes per string). Real text-vs-text collisions **0** (0073: 85
  pairs), text-over-wire overlaps **0**, no symbol body in the title-block region, no
  label crossing the frame border.
- The render: PNG is 7016x4961 (A4 landscape, 600 dpi), non-blank (mean 0.954, 4857
  colours, ~2 % ink). An independent visual pass over a 9-tile grid of the full PNG read
  every ref and value, found the wires traceable by eye and the left-to-right flow
  correct (flags and `P` at the left, reference and sensing dividers in the middle,
  comparators centre, output FETs and `SOLAR_OUT` at the right, GND rail along the
  bottom), and reported the two collisions I then fixed and re-measured (see deviation 2).
- Renders produced with `kicad-cli sch export svg` / `pdf` and magick's librsvg
  rasteriser at 600 dpi from that SVG, all committed on the branch.

## What CI does not cover

No workflow triggers on `hermes/0074-*` (three exist: android-build.yml is branch-pinned
to `android-toolchain-setup`, platformio.yml is path-pinned to `firmware/**`,
website-check.yml is branch-pinned to `main` and two other hermes branches), and the repo
has no KiCad or ERC job at all, so **nothing in CI compiles or checks this schematic**.
The evidence for this task is ERC + netlist parity + the measured geometry, all of it
re-readable from the branch. I did not extend website-check.yml's branch list: the task
forbids changes outside `hardware/solar-gate-v2/`, and that job would exercise the
website build, not the drawing.
