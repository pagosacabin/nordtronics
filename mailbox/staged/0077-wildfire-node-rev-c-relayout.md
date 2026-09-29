---
task_id: "0077"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  branch: hermes/0077-wildfire-node-rev-c-relayout
  sha: 5b629753c0cadcd0dbb44543f17924d11add4700
  run: none — no workflow in this repo triggers on this branch or these paths (enumeration below)
  files:
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sch
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_pro
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c-erc.txt
    - hardware/wildfire-node-v1/README.md
  renders:
    pdf: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf (vector, 1 A4 landscape page, 841.896 x 595.296 pt)
    png: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png (7016x4961 px = 600 dpi at A4 landscape, RGB)
  erc: 0 errors / 30 warnings — all [lib_symbol_mismatch]; title-block ERC line matches the committed report
  netlist: 19 multi-pin nets / 77 connected pins / 7 NC singletons — identical to the 0076 revision, member-for-member
  model: deepseek-flash
---

# 0077 — Re-layout the Wildfire Node Rev C schematic (readability pass)

**Status: staged.** Deliverable is on `hermes/0077-wildfire-node-rev-c-relayout` @
`5b629753c0cadcd0dbb44543f17924d11add4700`. Verified this run, after the push:
`git ls-remote --heads origin hermes/0077-wildfire-node-rev-c-relayout` and
`gh api repos/pagosacabin/nordtronics/branches/hermes/0077-wildfire-node-rev-c-relayout`
both report that SHA as the tip, and local `HEAD` equals it.

## What changed

Layout only — same 30 components, same 45 net labels, same pins, same values, same
nets. The sheet was re-placed and re-routed so the drawing uses the whole page:

- **The sheet is no longer left-heavy.** Wires span x 17.8–215.9 mm of the 297 mm width
  (0076 stopped at 168.9 mm) and y 19.1–166.4 mm; text midpoints now fall in *every*
  sheet-width decile from 0.0 to 0.9, and 13 module bodies have their left edge in the
  right half (U2, U3, R3, C3–C6, Q7, R21, R22, TP2, TP5, TP6). 0076 had nothing drawn
  beyond 0.57 of the width apart from the title block.
- **The notes block moved to the bottom-left band with a real routing keep-out.**
  Rect (11.43, 136.43)–(123.19, 193.5) mm: measured **0** wire segments and 0 bodies
  inside it. In 0076 the same block had 11 wire segments and the TP5 body running
  through it.
- **Every Ref/Value field was placed against the render.** All 60 fields were measured
  in the exported vector and moved to a free site (free of text, wire, body and the
  keep-outs); they end up within ~3 mm of their own symbol. Measured **0** text-vs-text
  overlaps > 0.4 mm² — 0076's headline defect — and **0** wire segments crossing a text
  glyph.
- **The three rotated symbols (J1, F1, R3) now render their Ref/Value horizontally.**
  See deviation 1: on F1 this removed a 40 mm vertical value string that ran through
  J1's value and off the top of the frame.
- Zone map implemented as specified: J1/F1/TP1/TP7+A1 top-left; U5+C1/C2+R2+TP2
  top-centre; U1 Heltec centre with GPIO wires radiating to its neighbours; U2 PMS5003
  +C3/C4+R3+TP6 to the right of Heltec; Q7/R21/R22 → U3 AS3935+C5/C6+TP5 below Heltec;
  U4 BME680+C9/C10+TP4 bottom-left; notes block bottom-left above the title block;
  KiCad title block bottom-right.
- Tightest module-to-module clearance is 7.62 mm (U3–Q7); every other pair of modules is
  ≥ 10.16 mm apart, so the ~5 mm rule holds with margin.

## How it was verified (what I actually ran)

1. **Netlist — the contract.** `kicad-cli sch export netlist --format kicadxml` on the
   branch `.kicad_sch` and on the 0076 revision (`68c6e2f:hardware/wildfire-node-v1/
   wildfire-node-v1-rev-c.kicad_sch`), then a programmatic comparison of the net
   *partition*: **19 multi-pin nets / 77 connected pins on both sides**;
   `set(partition_0076) == set(partition_0077)` → True; for every net present on both
   sides the member sets are equal → True; nets only in 0076 → `[]`, only in 0077 →
   `[]`; the 7 NC singletons are the same pins (U2.3/4/6/7/8, U4.5/6). A merged net
   (short) or a split net (open) would show up in exactly this check — one did during
   development (GND merging into 3V3 via a label on a foreign wire) and the router was
   fixed rather than the netlist.
2. **ERC.** `kicad-cli sch erc --severity-all --format report` → "Found 30 violations",
   histogram `30 [lib_symbol_mismatch]` — i.e. **0 errors / 30 warnings**, identical to
   0076 (Flatpak KiCad 10 library copies vs the embedded cache; not a connectivity
   fault). Committed as `wildfire-node-v1-rev-c-erc.txt`; the title-block comment line
   reads "ERC 2026-09-29: 0 errors / 30 warnings (all [lib_symbol_mismatch])" and
   matches the report.
3. **The drawing itself, measured from the exported vector — not asserted.** The
   committed SVG export was parsed back: 274 unique strings with their exact mm boxes,
   plus every wire segment. Results: text-vs-text overlaps > 0.4 mm² = **0**; wire
   segments crossing a text glyph box = **0**; wire segments inside the notes/title
   keep-outs = **0**; all 16 of 0076's note texts present (0 dropped, 1 added);
   all 30 reference designators and all 30 values present in the render; no drawing
   string outside the frame (the only boxes outside are KiCad's own frame zone numbers
   1–6). At *any* threshold, the only residual intersections are 6 sub-glyph `~`
   pin-decoration touches of ≤ 0.02 mm².
4. **Spread.** Wire extents and text-midpoint deciles as quoted above; 13 module bodies
   in the right half.
5. **Renders re-exported from the final `.kicad_sch` and re-opened.** PNG 7016×4961 px
   at 600 dpi (requirement ≥ 200), RGB; PDF 1 page, A4 landscape.
6. **Independent render check (vision).** A separate agent was handed the 0076 PNG and
   the 0077 PNG and asked the success-criteria questions blind. It measured on both
   the raster and the SVG: right-third occupancy (bodies to 0.821 of page width vs a
   maximum of 0.569 in 0076), **0** wire segments inside the notes keep-out rect (0076:
   11 wires *plus* the TP5 body inside it), 0 real text overlaps, no clipping, and no
   ink within 6 px of any page edge (0076 had the title-block ERC comment clipped at the
   right border).

## CI — why `run: none` is correct here (enumerated, not assumed)

`ls .github/workflows/` → `android-build.yml`, `platformio.yml`, `website-check.yml`.
None can fire for this push:

- `android-build.yml`: `on.push.branches: [android-toolchain-setup]` — not this branch.
- `platformio.yml`: `on.push.paths` / `on.pull_request.paths` = `firmware/tank-monitor/**`,
  `firmware/node-v1/**`, `.github/workflows/platformio.yml` — this branch touches
  `hardware/wildfire-node-v1/**` only.
- `website-check.yml`: `on.push.branches: [main, hermes/0068-site-email-refresh,
  hermes/0070-site-rewrite]`, `on.pull_request.branches: [main]` — not this branch.

Confirmed live rather than assumed: `gh run list -R pagosacabin/nordtronics --branch
hermes/0077-wildfire-node-rev-c-relayout` returns **no runs**. The branch tip SHA above
is the pointer for the verifier.

**No ntfy receipt:** the build topic in this repo is for CI builds that produce a
compiled artifact (APK / firmware image). This task compiles nothing — it is a KiCad
schematic plus its exports — and the spec names no topic for it.

## Deviations, TBDs and corrections

1. **Field angles on the three rotated symbols (J1, F1, R3) were changed.** A field's
   stored angle is relative to its symbol's rotation, so on F1 (rot 90) an angle-0
   Value renders as a 40 mm *vertical* string — it ran from y 4 to y 44, through J1's
   value and off the top of the frame. The stored angles now make those fields render
   horizontally. Presentation only: no value, reference, pin or net changed.
2. **`wildfire-node-v1-rev-c.kicad_pro` changed by one line — the root sheet UUID**,
   because the sheet was regenerated rather than nudged. No project setting changed.
   `.kicad_sym` and `sym-lib-table` are byte-identical to 0076 (the symbols did not
   change; only the layout did).
3. **Wire count 96 → 109** (`grep -c '(wire'`). More *segments*, not different
   connectivity: every connection is now an orthogonal polyline, so runs that were one
   multi-point block are serialised as individual horizontal/vertical segments.
   Token note for the verifier: KiCad 10 writes `(wire` followed by a newline, so the
   count is `grep -c '(wire'`, not `grep '(wire '`.
4. **One note line added** ("ERC: every warning is [lib_symbol_mismatch] …"). The
   criterion was "drop none": all 16 of 0076's note texts are present verbatim, and the
   added line documents the ERC warning class for the bench reader.
5. **Q7/R21/R22 remain TBD** — the frozen netlist does not specify them, and the
   "CS-low ≈ 0.3 V" and "P3 must remain unpopulated" constraints are carried as
   schematic text rather than invented values.
6. **Passives sit inside their functional cluster**, closer than the 5 mm module rule
   (C1/C2 by U5, C5/C6 by U3, C9/C10 by U4, R21/R22 by Q7). The rule was applied to
   *module* boxes, where the measured minimum clearance is 7.62 mm; separating passives
   as if they were modules would have forced signal wires across the sheet. TP1's and
   TP6's refdes were nudged 1.27 mm to clear the neighbouring long value strings (their
   glyph boxes were corner-touching, 0.048 mm² — sub-glyph, but free to remove).

---

<!-- original task spec, preserved -->

# 0077 — Re-layout the Wildfire Node Rev C schematic (readability pass)

## Context

0076 delivered an electrically correct Rev C schematic: the netlist was verified
member-for-member (19 multi-pin nets / 77 connected pins / 7 NC singletons) and
ERC reports 0 errors. But the sheet layout is unusable — Stephen's verdict,
confirmed against the render: everything is smashed into the left third of the
A4 sheet, the right two-thirds sit empty, wires cross straight through the
notes text block, and labels overlap each other.

Front-load the exact state: branch `hermes/0076-wildfire-node-v1-rev-c` @
`68c6e2fcf1efc9335683b08c6273565e0ba35662`, files under
`hardware/wildfire-node-v1/`. The authoritative netlist is in the archived
`mailbox/archive/0076-kicad-wildfire-node-v1-rev-c.md` and does not change in
this task.

## Task

Re-layout the Rev C schematic for readability on one A4 landscape sheet.
Placement and routing only — same components, same pins, same nets, same
values. Commit the updated `.kicad_sch`, a fresh vector PDF, a fresh
phone-readable PNG (>= 200 dpi), a fresh ERC report, and the README on branch
`hermes/0077-wildfire-node-rev-c-relayout`, branched off
`hermes/0076-wildfire-node-v1-rev-c`.

Follow this placement map:

- Top-left zone: J1, F1, TP1, TP7 / GND star point.
- Top-center: U5 MiniBoost + C1/C2 + R2 (4.7 kΩ EN pull-down); TP2 / 5V2.
- Center, given room: U1 Heltec module — the hub; GPIO wires radiate outward
  to their neighbors.
- Right of Heltec: U2 PMS5003 + C3/C4 + R3 + TP6.
- Below Heltec: Q7/R21/R22 gate stage -> U3 AS3935 + C5/C6 + TP5.
- Bottom-left: U4 BME680 + C9/C10 + TP4 (Vext).
- A1 temp-gate block: top-left, adjacent to J1 and the JP3 SOLAR wire.
- Notes text: one dedicated block, bottom-left above the title block, with a
  clear keep-out — no wires and no components through it.
- Title block: bottom-right (KiCad default frame).

Layout rules:

- Use the full sheet width. If the right half ends up emptier than the left,
  rebalance — that is the failure being fixed.
- Minimum ~5 mm clear space between module boxes; no component body overlaps
  another component's pins or labels.
- No wire crosses a text note. No label overlaps another label or a wire (fix
  the 7 known overlaps from 0076 and find the rest by rendering and looking).
- Wires orthogonal only. Keep the 0076 rule: net labels only on `VBAT_F`,
  `5V2`, `3V3`, `Vext`, `GND`; every signal is a drawn wire.
- All schematic note texts from 0076 (PROPOSED connectors, schematic-only,
  still-open items, ERC line) must survive the move — drop none.

## Success criteria

- [ ] The PNG shows components spread across the whole A4 sheet; the right
      half is no longer empty.
- [ ] Zero wires cross the notes block; zero label overlaps (verified by
      rendering and looking, not by assertion).
- [ ] Netlist re-verified after the move: `kicad-cli` netlist export compared
      member-for-member against the 0076 netlist — 19 multi-pin nets /
      77 connected pins / 7 NC singletons, identical to 0076. Any difference
      is a failure: fix the wiring, not the netlist.
- [ ] ERC re-run; the report committed; the title-block ERC line matches the
      new report exactly.
- [ ] Fresh vector PDF and fresh >= 200 dpi phone-readable PNG committed.
- [ ] Branch `hermes/0077-wildfire-node-rev-c-relayout` pushed to origin; SHA
      reported.

## Constraints

- Placement and routing only. No component, value, pin, or net changes — the
  0076 netlist is frozen.
- Worker model is deepseek-flash (Stephen's call). KiCad CLI work costs
  nothing; keep model calls minimal.
- If a placement in the map above breaks a wire route, move the component
  within its zone — never change the netlist to suit the layout.
- Push only to the task branch. No secrets in the repo, ever.

## Proof

- Branch `hermes/0077-wildfire-node-rev-c-relayout` on origin + SHA, verified
  with `git ls-remote`.
- Netlist comparison PASS line (19 nets / 77 pins / 7 NC singletons,
  identical to 0076).
- ERC summary line; PNG dimensions and dpi.

## Reply format

Stage your reply as `mailbox/staged/0077-wildfire-node-rev-c-relayout.md`
with front matter carrying `task_id: "0077"`, `status: staged`, `iteration`,
and a `proof:` block (branch, sha, files, renders with dimensions, erc,
netlist check, model), then a Reply section noting any layout compromises you
had to make.
