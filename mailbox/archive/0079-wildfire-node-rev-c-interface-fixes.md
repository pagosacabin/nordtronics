---
task_id: "0079"
protocol_version: 1.0.0
status: verified
iteration: 2
expect-reply-within: 6h
proof:
  branch: hermes/0079-wildfire-node-rev-c-interface-fixes
  sha: 62e64adf4b49da1b50c123e6297cad05418dde90
  run: none — no workflow in this repo triggers on this branch or these paths (enumeration below)
  files:
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sch
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sym
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c-erc.txt
    - hardware/wildfire-node-v1/README.md
  renders:
    pdf: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf — vector, 1 page, 841.896 x 595.296 pt (A4 landscape)
    png: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png — 7016 x 4961 px, density tag 236.22 px/cm = 600 dpi, sRGB, 8-bit
  erc: 0 errors / 31 warnings — every warning [lib_symbol_mismatch] (one per symbol instance; 30 instances in 0078, 31 with J2)
  netlist: 19 -> 20 multi-pin nets, 7 -> 35 NC singletons, exactly three changes vs the 0078 baseline; the 14 previously connected U1 nets are member-for-member identical
  model: deepseek-flash
---

# 0079 — Rev C: A1 solar-gate interface fix + full Heltec pinout

**Status: staged.** Deliverable is on `hermes/0079-wildfire-node-rev-c-interface-fixes` @
`62e64adf4b49da1b50c123e6297cad05418dde90`. Re-read from the remote after the
push: `git ls-remote --heads origin hermes/0079-wildfire-node-rev-c-interface-fixes`
and `gh api repos/pagosacabin/nordtronics/branches/hermes/0079-wildfire-node-rev-c-interface-fixes`
both report that SHA as the tip, and the local branch tip equals it. 0078 was
already archived when this task started, so the branch was cut from the 0078 tip
`7dc24165d20d7a99fdccda366e70716824e7a3ce`, not from main (main carries **0**
files under `hardware/wildfire-node-v1/` — that project lives only on the feature
branch; a verifier diffing against main will see every path as "new").

## Reply — what changed

Three changes on the Rev C sheet, all inside
`hardware/wildfire-node-v1/`:

**1. A1 is now a three-pin series gate.** `SolarGate_v2_TempGate` was two pins
(left = `GND`, right = `SOLAR_OUT`), i.e. the temp-gate board drawn as a shunt.
It now has exactly three pins, as specified:

| pin | name | side | symbol coords | sheet coords |
|---|---|---|---|---|
| 1 | `SOLAR_IN` | left | `(-17.78, 0)` | (46.99, 46.99) |
| 2 | `SOLAR_OUT` | right | `(17.78, 0)` | (82.55, 46.99) |
| 3 | `GND` | bottom | `(0, -10.16)` | (64.77, 57.15) |

`SOLAR_OUT` still reaches the Heltec `SOLAR` pin (U1 pin 3) on the unchanged
unnamed net; `GND` reaches the ground star (TP7 / the `GND` label net).

**2. J2 is the panel input** — `Connector_Generic:Conn_01x02` at (33.02, 46.99),
rotation 180, left of A1, value/description `PANEL IN (13W 5V panel)`. Pin 1 is
wired to A1 pin 1 on the new net `SOLAR_IN`; pin 2 returns to the ground star
(routed left, then down under the A1 block and up into A1 pin 3, label `GND`).
The net `SOLAR_IN` therefore has exactly two members: `J2.1` and `A1.1`.

**3. U1 shows the full 42-pin Heltec V4.2 pinout.** All 42 names from the spec
are present, exact, and in the spec's grouping (J2 header 18, J3 header 18, the
four XTAL/OLED pins, `BAT` + `SOLAR`), verified as a multiset against the spec
text and again in the raster:

By group, exactly: `GND 5V Ve Ve RX TX RST GPIO0 GPIO36 GPIO35
GPIO34 GPIO33 GPIO47 GPIO48 GPIO26 GPIO21 GPIO20 GPIO19` (J2 header),
`GND 3V3 3V3 GPIO37 GPIO46 GPIO45 GPIO42 GPIO41 GPIO40 GPIO39 GPIO38 GPIO1
GPIO2 GPIO3 GPIO4 GPIO5 GPIO6 GPIO7` (J3 header), `GPIO18 GPIO17 GPIO16 GPIO15`
(XTAL/OLED), `BAT SOLAR` (power) — 42 pins, 42 unique pin numbers.

**Pin-numbering policy (the design decision in this task).** The 14 previously
connected pins keep their 0076/0077 **numbers (1–14) and their sites on the
symbol**, so every frozen net member is byte-identical in the netlist. The 28
added pins are numbered **15–42 in Heltec header order** (J2 header 15–26, J3
header 27–40, `GPIO16`/`GPIO15` 41/42) and each carries a no-connect flag. The
block grew from 30.48 × 30.48 mm to 30.48 × 55.88 mm (sheet y 80.01–135.89) and
the pin rows are *interleaved* — a block re-ordered by header would have moved
all 14 connected pins, which would have re-routed every frozen net and broken the
"same pins, same nets" criterion. Two note lines on the sheet state the policy
and the A1/J2 interface.

## Success criteria — checked one by one

- [x] **A1 has exactly three pins** `SOLAR_IN` (left) / `SOLAR_OUT` (right) /
      `GND` (bottom). Both surviving nets verified in the netlist (below).
- [x] **J2 present**, `PANEL IN (13W 5V panel)`, and `SOLAR_IN` runs panel pin 1
      → A1 pin 1 only: netlist net `/SOLAR_IN` = {`A1.1`, `J2.1`}, nothing else.
- [x] **U1 shows all 42 pins**; every one of the 28 added pins is NC-flagged
      (35 no-connect flags on the sheet = 28 new + the 7 carried from 0077), so
      there is no unmarked dangling pin. The 14 previously connected nets have
      identical members (same ref, same pin number, same pin function).
- [x] **Netlist diff vs the 0078 baseline** (which is member-identical to 0077 —
      0078 was text-only): only the expected additions, plus the two renumberings
      the task itself forces (see the diff below). No net lost, no other net
      touched.
- [x] **ERC re-run: 0 errors / 31 warnings** — every warning `[lib_symbol_mismatch]`,
      i.e. one per symbol instance (30 instances in 0078 → 31 with J2). Report
      committed as `wildfire-node-v1-rev-c-erc.txt`; the title-block ERC line was
      updated to match it.
- [x] **Fresh vector PDF + fresh ≥ 200 dpi PNG** committed, branch pushed, SHA
      reported.

## How it was verified (what was actually run)

1. **The netlist diff, programmatically.** `kicad-cli sch export netlist --format
   kicadxml` on the 0078 revision and on this one, parsed to a net→member-set
   partition and compared:

   | | baseline (0078) | this revision |
   |---|---|---|
   | nets | 26 | 55 |
   | multi-pin nets | 19 | 20 |
   | connected pins in multi-pin nets | 77 | 80 |
   | NC singletons | 7 | 35 |

   Changes in full (`-` base, `+` after):
   - `/GND`: `-A1.2(GND_2)`, `+A1.3(GND_3)`, `+J2.2(Pin_2_2)`
   - `Net-(A1-SOLAR_OUT)`: `-A1.1(SOLAR_OUT_1)`, `+A1.2(SOLAR_OUT_2)` — same two
     pins, `U1.SOLAR` unchanged
   - new `/SOLAR_IN`: `A1.1(SOLAR_IN_1)`, `J2.1(Pin_1_1)`
   - 28 new `unconnected-(U1-<NAME>-PadNN)` singletons for the added pins
   - everything else identical, member-for-member; no net name disappeared.

   The two renumberings are the direct consequence of the pin numbers the task
   specifies for A1 (pin 1 = `SOLAR_IN`), not an unrequested change; the U1 pin
   numbers 1–14 were deliberately preserved so the 14 frozen nets did not move.

2. **The drawing, measured in the exported vector — 274 → 339 strings, whole
   sheet.** The SVG export was parsed back to ink boxes for every plotted string
   and every wire segment: **0** text-vs-text overlaps, **0** wire segments
   crossing a text glyph, **0** wires inside the notes-block keep-out, **0**
   wires inside the title-block keep-out. This covers the 42 new pin names, their
   42 pin numbers, the 28 NC flags, J2's and A1's fields and the two new note
   lines.

3. **The electrical map re-checked from the file.** U1's symbol parses to 42
   pins / 42 unique numbers with the spec's names as an exact multiset; the 35
   `no_connect` coordinates are computed from the same pin table the symbol was
   generated from, and every one of the 28 added pins has a flag at its own
   connection point.

4. **The render, read independently (a second agent, eyes on the pixels).** It
   read the 600 dpi raster and the vector: all 42 pin names legible, no name
   overlapping a name or a number, no name clipped or outside the block, row
   pitch 60 px vs 33–35 px glyph height, **14 X marks on the left edge and 14 on
   the right (28)**, so per side 21 pins = 7 wired + 14 X'd, zero ink of any other
   colour inside the body, and the densest 120×120 region only 24 % ink (normal
   text). For the J2/A1 crop: `PANEL IN (13W 5V panel)` fully readable with the
   nearest wire 73 px (≈3 mm) away, A1's three pin names readable, no wire through
   any string anywhere in the crop.

5. **Renders re-exported from the final `.kicad_sch` and re-opened from the
   branch tip** (not from the worktree): PDF 1 page, 841.896 × 595.296 pt (A4
   landscape); PNG 7016 × 4961 px, density tag 236.22 px/cm = **600 dpi**
   (requirement ≥ 200), sRGB 8-bit.

## CI — why `run: none` is correct here (enumerated, not assumed)

`ls .github/workflows/` → `android-build.yml`, `platformio.yml`,
`website-check.yml`. None can fire for this push:

- `android-build.yml`: `on.push.branches: [android-toolchain-setup]` — not this branch.
- `platformio.yml`: `on.push.paths` = `firmware/tank-monitor/**`, `firmware/node-v1/**`,
  `.github/workflows/platformio.yml` — this branch touches `hardware/wildfire-node-v1/**` only.
- `website-check.yml`: `on.push.branches: [main, hermes/0068-site-email-refresh,
  hermes/0070-site-rewrite]` — not this branch.

Confirmed live rather than assumed:
`gh api "repos/pagosacabin/nordtronics/actions/runs?branch=hermes/0079-wildfire-node-rev-c-interface-fixes"`
returns `total_count: 0`. The branch tip SHA above is the pointer for the verifier.

**No ntfy receipt:** the build topic in this repo is for CI builds that produce a
compiled artifact (APK / firmware image). This task compiles nothing — it is a
KiCad schematic plus its exports — and the spec names no topic for it.

## Deviations and scope notes

1. **Pin numbers 15–42 are a numbering choice, not a spec value.** The spec lists
   the 42 names but no numbers. Numbers 1–14 were preserved for the existing pins
   (so the frozen nets stay identical); 15–42 were assigned in header order. This
   is stated on the sheet in a new note line and in the README.
2. **`VBAT_F` label moved 2.54 mm left** (133.35,82.55) → (130.81,82.55), with
   U1's `BAT` stub re-routed to it. U1's new pin 15 (`5V`) lands on the label's old
   coordinate; leaving the label there attached `VBAT_F` to `5V` and orphaned
   `BAT` — the netlist diff caught it (that is exactly what this check is for).
3. **The GPIO4→U5-EN vertical moved off the block edge** (x 163.83 → 166.37, plus
   a 2.54 mm jog at the pin). The wider U1 block put the old run on the body
   outline; the net is unchanged.
4. **A1's Ref and Value fields were repositioned** (`A1` to the left of the block,
   the value string above it) because A1's new bottom pin and its GND return
   occupy the space the value string used to sit in. Text moved, values unchanged.
5. **U1's Ref/Value fields moved** above/below the taller block.
6. **`README.md` updated** (component/net/wire/label/note/ERC counts, the sheet-layout
   table, and a "Rev C interface fixes (0079)" section explaining the change and
   its verification). Not named in the spec, declared here as a scope extension —
   its counts would otherwise contradict the committed artifacts.
7. **The `.kicad_sch`/`.kicad_sym` were edited by a scoped, asserting generator
   script** (every anchored replacement asserts its match count; the script was
   run from outside the repo and is *not* committed). The delivered artifact is
   the KiCad file itself.
8. **Pre-existing, deliberately not changed:** U1's symbol draws its pin stubs
   *inside* the body outline and its pins at the body edge (the 0076/0077
   convention, so the 28 NC marks straddle the outline). Moving the pins outside
   the body would move every connection point and re-open the 14 frozen nets.
   Likewise all 31 `[lib_symbol_mismatch]` warnings are the Flatpak
   library-path artefact of the 0074/0075 setup, unchanged in kind from 0078.


<!-- original task spec, preserved -->

# 0079 — Rev C schematic: A1 solar-gate interface fix + Heltec full pinout

## Context

0077 (full-sheet re-layout) is verified and archived: branch
`hermes/0077-wildfire-node-rev-c-relayout` @
`5b629753c0cadcd0dbb44543f17924d11add4700`, files under
`hardware/wildfire-node-v1/`. Its netlist (19 multi-pin nets / 77 connected
pins / 7 NC singletons) was frozen as the baseline.

0078 (J1 upside-down text + TP1/F1 label nudge, text-only) is in active with
you, iteration 2, on branch `hermes/0078-wildfire-node-rev-c-text-fix`.

Stephen reviewed the Rev C render and found two interface defects that are
electrical, not cosmetic, so they are deliberately OUT of 0078's frozen-netlist
scope:

1. **A1 (SOLAR-GATE-V2 TEMP-GATE block) is wrong.** It is drawn with left pin
   = GND, right pin = SOLAR_OUT. The temp-gate is a series gate in the solar+
   path: the left side must be SOLAR_IN (panel + coming in) and the right side
   SOLAR_OUT (gated power to Heltec JP3 SOLAR). Additionally, the solar panel
   input appears nowhere in the node schematic — the system's power source is
   missing from the drawing.
2. **U1 (Heltec HTIT-WB32LAF V4.2) shows only the 14 used pins** (BAT, GND,
   SOLAR, 3V3, GPIO36, GPIO17, GPIO18, GPIO4, GPIO5, GPIO6, GPIO33, GPIO47,
   GPIO48, GPIO34). A module block in a system schematic must show the full
   module pinout so unused pins are visible and marked, not hidden.

Juno verified against the Heltec V4.2 schematic that the current GPIO
assignments are correct (GPIO36 = Vext_Ctrl, GPIO34 = VGNSS_Ctrl) — keep them.

## Task

On a new branch `hermes/0079-wildfire-node-rev-c-interface-fixes`, branched
off `hermes/0078-wildfire-node-rev-c-text-fix` at its current origin tip (if
0078 has already merged to main when you start, branch off main instead):

1. **Redefine the A1 symbol** (`wildfire-node-v1:SolarGate_v2_TempGate`) with
   three pins:
   - Pin 1, left side: `SOLAR_IN`
   - Pin 2, right side: `SOLAR_OUT`
   - Pin 3, bottom side: `GND`
   Wire pin 2 to the Heltec's SOLAR pin exactly as today (net unchanged).
   Wire pin 3 to the GND star exactly as today (net unchanged).
2. **Add the panel input.** Place a 2-pin connector symbol left of A1,
   reference `J2`, description `PANEL IN (13W 5V panel)`. Wire its pin 1 to A1
   pin 1 on a new net named `SOLAR_IN`; wire its pin 2 to the GND star net.
3. **Expand U1 to the full V4.2 pinout** — 42 pins total, using these exact
   names (sourced from the Heltec V4.2 schematic; keep the existing 14 nets
   wired exactly as they are today, mark every unused pin with an NC marker):
   - Header J2 (18): `GND`, `5V`, `Ve`, `Ve`, `RX` (GPIO44), `TX` (GPIO43),
     `RST`, `GPIO0`, `GPIO36`, `GPIO35`, `GPIO34`, `GPIO33`, `GPIO47`,
     `GPIO48`, `GPIO26`, `GPIO21`, `GPIO20`, `GPIO19`
   - Header J3 (18): `GND`, `3V3`, `3V3`, `GPIO37`, `GPIO46`, `GPIO45`,
     `GPIO42`, `GPIO41`, `GPIO40`, `GPIO39`, `GPIO38`, `GPIO1`, `GPIO2`,
     `GPIO3`, `GPIO4`, `GPIO5`, `GPIO6`, `GPIO7`
   - Additional pins (4): `GPIO18` (OLED_SCL), `GPIO17` (OLED_SDA), `GPIO16`
     (XTAL_32K_N), `GPIO15` (XTAL_32K_P)
   - Power (2): `BAT`, `SOLAR`
4. Keep the 0077 full-sheet layout style: notes block clear of wires, no text
   overlaps, no wires crossing text. Re-export the vector PDF and a >= 200 dpi
   PNG from the final `.kicad_sch`, re-run ERC, commit everything.

## Success criteria

- [ ] A1 has exactly three pins: `SOLAR_IN` (left), `SOLAR_OUT` (right),
      `GND` (bottom). `SOLAR_OUT` still reaches the Heltec SOLAR pin;
      `GND` still reaches the GND star.
- [ ] New `J2` panel connector present, labeled `PANEL IN (13W 5V panel)`;
      new `SOLAR_IN` net runs panel pin 1 -> A1 pin 1 only.
- [ ] U1 shows all 42 pins above. The 14 previously connected nets are
      unchanged (same pins, same nets). Every other U1 pin carries an NC
      marker — no unmarked dangling pins.
- [ ] Netlist diff against the 0077 baseline shows ONLY the expected
      additions: the new `SOLAR_IN` net and the new NC singletons. Any other
      net change is a failure. (This task deliberately changes the netlist —
      it is an electrical correction, not a layout pass.)
- [ ] ERC re-run: 0 errors; warnings noted with the count.
- [ ] Fresh vector PDF + fresh >= 200 dpi PNG committed under
      `hardware/wildfire-node-v1/`; branch
      `hermes/0079-wildfire-node-rev-c-interface-fixes` pushed to origin; SHA
      reported.

## Constraints

- Base branch: `hermes/0078-wildfire-node-rev-c-text-fix` tip (or main if
  0078 merged). Push only to the task branch.
- Schematic only — no PCB work.
- No changes to any other component, value, or net connection.
- Worker model stays deepseek-flash (Stephen's call) — do not escalate tier
  for this task.
- No secrets in the repo, ever.

## Proof

- Branch `hermes/0079-wildfire-node-rev-c-interface-fixes` on origin + SHA,
  verifiable with `git ls-remote`.
- Netlist diff output vs the 0077 baseline (19/77/7), showing only the
  `SOLAR_IN` net and NC-singleton additions.
- ERC summary line; PNG dimensions; PDF path in the branch.

## Reply format

Stage your reply as `mailbox/staged/0079-wildfire-node-rev-c-interface-fixes.md`
with front matter carrying `task_id: "0079"`, `status: staged`, `iteration`,
and a `proof:` block (branch, sha, files, renders, erc, netlist diff, model),
then a Reply section.
