# Mark 1 Rev 1 — board form-factor options (gate: Stephen picks)

Task 0199, gate answer of 2026-11-16 ~14:19 MST: the outline is **not** a fixed size from
Stephen. His direction, quoted from the task file: *"see how we could make this actually fit in
an easy-to-work-with form factor first, but still keeping it small — it's just supposed to be a
small cutoff circuit that we can add into this current existing setup."*

So this document is the proposal that gate asks for: three candidate outlines, each with a
dimensioned placement envelope, a mounting approach, the connector/pad assumption behind it,
and the trade-offs. **No layout has been started.** Stephen picks one; layout proceeds on that
one only.

## Design basis (fixed by the task; nothing here is a new decision)

| Input | Value | Source |
|---|---|---|
| Circuit | solar-gate temperature-cutoff: 4 × FQP27P06 in two back-to-back pairs, BAT85 OR-ing, LM393 + TL431 control, 10 k B3950 NTC on the cell | task 0199, as-built confirmed 2026-11-16 |
| Construction | **through-hole**, "Mark 1, Revision 1 — I have the piece parts" | Stephen via Juno, 2026-11-16 ~14:17 MST |
| Board designation | **Mark 1, Revision 1** in the title block and on the silkscreen | same |
| Off-board connections | **solder pads**, no connector footprints; JST deferred to a future revision | Stephen via Juno, ~14:20 MST |
| Design current | **3 A** worst case through the Q1/Q2 pass paths (~540 mA normal run) | Stephen via Juno, 15:02 MST |
| Role | add-in module: panel → this gate → Heltec V4 solar input | task context |

Part population — 26 placements plus off-board pads: 4 × TO-220 (FQP27P06), 1 × DIP-8 (LM393),
1 × TO-92 (2N3904), 1 × TO-92 (TL431), 4 × DO-35 axial (BAT85), 13 × 1/4 W axial resistors,
2 × radial 5 mm capacitors, and pads for `P`, `GND`, `SOLAR_OUT` and the two NTC leads.

## Dimension basis

Board area is driven by the through-hole envelopes, not by preference. Figures below are the
standard footprint courtyards (body + pad + solder-iron access), rounded up:

| Part | Board envelope | Height above board |
|---|---|---|
| TO-220 upright (leads formed) | 10.2 × 5.0 mm, 3 pads on 2.54 mm | ~16.5 mm (body 15.9 + tab) |
| TO-220 laid flat | 10.2 × 15.5 mm, pads on 2.54 mm | ~6 mm (incl. bent leads) |
| DIP-8 (socketable) | 10.2 × 7.6 mm, 7.62 mm row pitch | ~5 mm (socket) |
| TO-92 | 5.0 × 4.2 mm | ~5 mm |
| DO-35 axial (BAT85) | 26 × 2.5 mm on a 10.16 mm pad pitch | ~2.5 mm |
| 1/4 W axial resistor | 10.2 × 3.0 mm on a 10.16 mm pad pitch | ~2.5 mm |
| Radial 5 mm capacitor | 5.0 × 4.0 mm | ~7 mm |

Summed component envelope: **≈1390 mm²** with the MOSFETs upright, **≈1820 mm²** laid flat.
A hand-solderable board needs two to three times that area once pad access, silkscreen, 3 A
copper and the edge pads are counted — that is what separates the three options.

## Copper sizing for the 3 A path

IPC-2221 at a 10 °C rise: 3 A needs **1.37 mm** of 1 oz external copper (3.56 mm on an inner
layer); the 2.6 A panel nominal needs 1.12 mm. Traces and pads on `P`, `M` and `SOLAR_OUT` will
be drawn at **2.0 mm minimum** on 1 oz, the `G` gate node at 0.5 mm. 2 oz is available from the
same fabricator at no tooling cost if Stephen prefers extra margin; every option below has room
for it, so copper weight is not a differentiator between them.

## The three options

Dimensioned drawings: `option-a-70x50-compact.svg`, `option-b-85x60-hand-friendly.svg`,
`option-c-100x70-low-profile.svg` (PNG renders alongside each). They are **placement
envelopes** — part boxes at real footprint size, pad positions and keep-outs — not routed
layouts.

### Option A — 70 × 50 mm, upright MOSFETs

- **Size:** 70 × 50 mm, single-sided placement, 2 layers. Component envelope fills ~40 % of the
  board.
- **Height:** ~16.5 mm (the four upright TO-220 bodies dominate).
- **Power row:** Q1a/Q1b and Q2a/Q2b upright along the top edge — `P` pad pair at the left
  edge, `SOLAR_OUT` pad pair at the right edge, `M` node between the pairs, ~10 mm of copper
  between them. The 3 A path is ~35 mm long.
- **Control:** LM393 DIP-8, TL431, 2N3904, the four BAT85s and 13 resistors on a 10.16 mm grid
  below; two 100 nF caps beside U2. 2.8 mm between part bodies in both axes.
- **NTC pads:** right edge, 2 pads on 5.08 mm — the NTC lead goes to the cell, which is the
  point of bringing it out at an edge.
- **Mounting:** 4 × M3 at the corners, 3.5 mm in from each edge.
- **Trade-off:** smallest, and it fits an existing enclosure add-on with little room to spare.
  Costs: the resistor grid is dense, and there is no spare area — a Rev 2 JST footprint will
  not fit without growing the board.

### Option B — 85 × 60 mm, upright MOSFETs (recommended)

- **Size:** 85 × 60 mm; envelope fills ~27 % of the board — ~3700 mm² not occupied by parts.
- **Height:** ~16.5 mm, same as A (the FETs are the tall parts).
- **Power row:** same electrical floorplan as A, but with 15 mm between the Q1 and Q2 pairs so
  the `M` node copper can run 4 mm wide for its whole length and the two pairs' dissipation has
  somewhere to spread.
- **Control:** same grid with 2.8 mm between resistor bodies and 2.0 mm between rows; the
  comparator, reference and driver sit in a labelled block with room for a socket on U1.
- **NTC pads:** right edge, 5.08 mm pitch, clear of the power pads and the mounting holes.
- **Mounting:** 4 × M3, 5 mm in from the corners.
- **Trade-off:** 30 % more board area than A for a materially easier hand-build and room to
  rework. It still fits the "small add-in module" brief (85 × 60 × 17 mm). I recommend it: it is
  the smallest option that does not make the build fussy, and it leaves free copper for a future
  JST revision without a re-spin.

### Option C — 100 × 70 mm, MOSFETs laid flat

- **Size:** 100 × 70 mm; envelope fills ~26 %.
- **Height:** ~8 mm — the four TO-220s lie flat (tab down) in a row along the bottom edge; the
  only option that fits under a low lid.
- **Power row:** pads at both ends, `M` node a wide bar between the pairs, pass path in one
  straight line.
- **Control:** most spacious of the three — 3.8 mm between resistor bodies, 2.5 mm between rows.
- **NTC pads:** bottom edge. A **30 × 7 mm area is reserved (not placed)** for the Rev 2 JST GH
  1.25 connectors, so the future revision is a footprint change, not a re-layout.
- **Mounting:** 4 × M3, 6 mm in from the corners.
- **Trade-off:** largest board, lowest profile. Laying the FETs flat costs board area and
  lengthens the drain/source leads, but it puts the pass path in one line and gives the most
  room for 3 A copper. Pick this if the module has to slide under a lid.

## Comparison

| | A — 70 × 50 | B — 85 × 60 (rec.) | C — 100 × 70 |
|---|---|---|---|
| Board area | 3500 mm² | 5100 mm² | 7000 mm² |
| Envelope fill | 40 % | 27 % | 26 % |
| Height | ~16.5 mm | ~16.5 mm | ~8 mm |
| Gap between part bodies | 2.8 mm | 2.8 mm + spare area | 3.8 mm |
| 3 A copper room | adequate | comfortable | most |
| Mounting | 4 × M3 | 4 × M3 | 4 × M3 |
| Rev 2 JST room | none | free area | reserved area |
| Hand-build difficulty | fussy | easy | easiest |

## What a pick changes — and what it does not

Changes: board outline, part coordinates, copper lengths, mounting holes. Does **not** change:
the schematic, the part list, the net names or any electrical value. The schematic capture can
therefore be done independently of this pick and applied to whichever outline is chosen.

## Assumptions made here (declared, not silently adopted)

1. **TL431 package:** the card fixes the MOSFETs and BAT85s as through-hole but does not name
   the TL431 package. Assumed **TO-92** (through-hole). If the bench build used a SOT-23 TL431
   on a carrier, say so and the footprint changes to a SOT-23 pad pattern (no outline change).
2. **Mounting holes:** M3 (3.2 mm drill, 6.5 mm annulus) at four corners. Stephen did not
   specify holes or an enclosure; if the module is to be cable-tied or taped down, the holes can
   be dropped (option A then shrinks by ~12 mm in each axis).
3. **NTC:** treated as **off-board** (two pads, lead to the cell) because the card says the NTC
   is thermally on the cell. If Stephen wants the NTC on the board, the pads move mid-board and
   no outline changes.
4. **Test pads on `M` and `G`:** not drawn. Both are useful for the next bench check and cost no
   area in B or C.
5. **Copper weight:** 1 oz assumed in every figure above; 2 oz is an option, not a requirement.

## Next step

Stephen picks A, B or C, and corrects any of the five assumptions only where he wants to differ.
Then: schematic capture into a real KiCad project, layout on the picked outline, ERC/DRC,
Gerbers and drill, PR with Copilot review — per task 0199 steps 1-4.
