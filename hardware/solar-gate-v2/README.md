# Solar gate — temperature cutoff, Mark 1 Rev 2

An add-in module for the wildfire-node power path: `panel (+) → this board → Heltec V4
solar input`. It switches the panel feed off when the cell is too cold or too hot, using a
10 k B3950 NTC bonded to the cell, an LM393 window comparator and a TL431 reference. The
pass element is two back-to-back FQP27P06 pairs (common drain at `M`), so the board blocks
current in both directions when off — that is what makes `P` never beep to `SOLAR_OUT`.

- **Board designation:** Mark 1 Rev 2 (silkscreen, title block)
- **Outline:** 85 × 60 mm, 2 layers, 1.6 mm, through-hole, 1 oz copper (Option B from
  `FORM-FACTOR-OPTIONS.md`, picked by Stephen 2026-11-16)
- **Off-board connections:** solder pads only — `P`, `GND`, `SOLAR_OUT`, `TH1` (NTC leads).
  JST GH is deferred to a future revision by Stephen's call.
- **Design current:** 3 A worst case through the Q1/Q2 pass paths (normal run ≈ 540 mA)

## Schematic source

The circuit is Stephen's as-built bench build. The card records it on 2026-11-16 as
**confirmed equal to the drawing**, with one qualification: the card's own LM393 pin 1 ↔
pin 5 note cannot be reconciled with the drawing from anything in the repo, and that
discrepancy is carried, unresolved, in "Open item for Stephen" below. Equivalence is
therefore asserted by the card for the circuit as a whole and *not* independently
established by this capture for that one node. Everything else here reproduces the
drawing net for net.

This capture starts from the last capture in the repo
(`origin/hermes/0075-solar-gate-v2-io-labels` @ `397f17d`,
`hardware/solar-gate-v2/solar-gate-v2.kicad_sch`) and re-draws it with the as-built parts:
four **FQP27P06** (TO-220) instead of AO3401, **BAT85** (DO-35) instead of BAT54, **LM393**
DIP-8, **TL431LP** TO-92, THT resistors/caps, and pad footprints on the four off-board nets.

Two numbering corrections came out of that re-draw and are deliberate:

1. **FQP27P06 pin numbers differ from AO3401.** AO3401 is G/S/D = 1/2/3; the TO-220
   FQP27P06 is **G/D/S = 1/2/3**. The net assignment follows the beep-out checklist, not the
   old symbol: all pin 2s (drains) at `M`, all pin 1s (gates) at `G`, Q1/Q2 pin 3s (sources)
   at `P`, Q3/Q4 pin 3s at `SOLAR_OUT`.
2. **TL431 pin numbers follow the TO-92 datasheet** (1 = REF, 2 = ANODE, 3 = CATHODE). The
   0075 capture used a project-local symbol numbered 1 = K, 2 = REF, 3 = A; the net result is
   identical (REF and K both on `VREF`, A on `GND`), but the standard KiCad symbol is correct
   for the physical part.

The connectivity was checked against the 0075 capture's netlist, net by net, after mapping the
pin numbers above: **14 nets, identical member sets** (see `docs/net-map.md`).

## Files

| Path | What |
|---|---|
| `solar-gate-v2.kicad_pro` | KiCad project (net classes: `Power_3A` 1.5 mm / 0.3 mm, `Gate` 0.5 mm) |
| `solar-gate-v2.kicad_sch` | Schematic, A3, Mark 1 Rev 2 title block |
| `solar-gate-v2.kicad_pcb` | PCB, 85 × 60 mm, routed 2-layer |
| `sym-lib-table`, `fp-lib-table` | Project library tables (comment-free; `KICAD10_*_DIR`) |
| `drc-report.rpt` | DRC run on the delivered board, `--severity-all --schematic-parity` |
| `fab/gerbers/` | Gerbers (F/B copper, mask, silkscreen, Edge.Cuts) + `.gbrjob` |
| `fab/drill/` | Excellon drill, separate PTH / NPTH |
| `fab/bom.csv` | BOM grouped by value + footprint (C3 = 47 µF 16 V, `CP_Radial_D5.0mm_P2.00mm`) |
| `renders/board-top.png`, `renders/board-bottom.png` | 3D renders, 2200 × 1500, for a phone |
| `renders/schematic.png` | Schematic sheet, 4961 × 3508 (A3 @ 300 dpi) |
| `docs/net-map.md` | Net list and the bench-name → designator map |

## Verification (as delivered)

- **ERC: 0 errors, 0 warnings** (`kicad-cli sch erc --severity-all --format report`).
- **DRC: 0 errors, 0 unconnected pads**, 2 warnings — both
  `lib_footprint_mismatch` on the rotated solder-pad footprints (J2, TH1); see below.
- **Schematic parity: 4 notes** — the four mounting holes are board-only (no schematic
  symbol), which is expected for a mechanical part.
- **Copper for 3 A:** IPC-2221 at 10 °C rise wants 1.37 mm of 1 oz external copper; the
  `Power_3A` net class routes `P`/`M`/`SOLAR_OUT` at **1.5 mm** and the `M` node is a
  **4 mm** bar between the two FET pairs (hand-drawn, not autorouted), with 2 mm stubs from
  each drain. The gate node `G`/`H` routes at 0.5 mm.
- Routing: 161 segments, 7 vias (6 of them GND stitching vias), 2 GND copper pours
  (F.Cu + B.Cu), both layers; all 14 nets connected (0 unconnected pads).

### Layout change 2026-11-17 (task 0205)

C1 (U1's supply bypass) moved from (30, 40) to (43.8, 48.6), against U1 pin 8: 5.83 mm
pad-centre to pad-centre where it was 21.9 mm, and the `P` copper between C1.1 and U1.8 is
now 5.8 mm of 1.5 mm trace instead of a 32 mm run through J1/D1. GND return is the pour
itself (C1.2 sits in it, with a stitching via 1.5 mm away). GND pours were added on both
layers (0.3 mm clearance, 0.5 mm thermal spokes) because the whole GND return — up to 3 A
between J1.2 and J2.2 — was previously a 0.2 mm trace run. Off-board labels were moved onto
their own pads, and U2's VREF strap was pushed to 1.05 mm clearance from pin 2 (was 0.20 mm).


### Rev 2 changes (task 0206, 2026-11-17)

Four review items, all Stephen's 2026-11-17 decisions:

1. **J1.2 and J2.2 are solid-connected to the GND pours** (pad-level `zone_connect 2`), so the
   full return no longer passes through four thermal spokes. Every other GND pad keeps thermal
   relief; the pours themselves are untouched (0.3 mm clearance, 0.3 mm gap, 0.5 mm spokes).
   Read back from the committed board: a 24-point sample ring at pad radius reports **24/24**
   points inside the fill on both layers for J1.2 and J2.2, and 6–12/24 (spokes only) for every
   other GND pad.
2. **C3, 47 µF 16 V input electrolytic, added at J1** — the only circuit change. `/P` gains
   C3.1 and `/GND` gains C3.2; nothing else moves (netlist diff below). It sits in the pocket
   between D1 (bottom edge y = 45.3) and U1 (top edge y = 51.4), centred at **(35.5, 48.3)**
   with pad 1 at (34.5, 48.3) and pad 2 at (36.5, 48.3) — i.e. the first clear pocket below the
   area C1 vacated at (30, 40), which is 0.3 mm from U1's pads and 1.8 mm from D1's pad and
   cannot take a 5 mm can. The `+` pad ties to the `P` trunk with one new 1.5 mm B.Cu segment
   (34.5, 48.3) → (30, 48.3); the `−` pad lands in the GND pour with a normal thermal relief.
   Courtyard gaps: 0.21 mm to D1, 0.35 mm to U1 — no overlap.
3. **Revision marks are REV 2**: silkscreen text `MARK 1 REV 2`, board and schematic title
   blocks (title and revision field).
4. **U2's reference designator** moved from (83.27, 51.0) to **(84.6, 47.0)** — 0.1 mm below the
   footprint courtyard, still under U2's right half, 2.5 mm clear of Q5's courtyard and 1.5 mm
   clear of the `MARK 1 REV 2` text.

**Footprint substitution, declared.** The card named `Capacitor_THT:C_Radial_D5.0mm_P2.00mm`;
KiCad 10 has no such footprint. The only 5 mm / 2 mm-pitch radial parts are
`C_Radial_D5.0mm_H{5,7,11}.0mm_P2.00mm` (unpolarized ceramic/film) and
`CP_Radial_D5.0mm_P2.00mm` (polarized electrolytic). A 47 µF electrolytic is polarized, so the
can assumed is **`CP_Radial_D5.0mm_P2.00mm` — 5 mm diameter, 7 mm height, 2 mm pitch**, with the
matching polarized symbol `Device:C_Polarized` (pin 1 = `+`). If Stephen's stocked part is a
taller can (5 × 11 mm), swap the footprint; the pads and the placement do not move. The
footprint's own silkscreen carries the `+` mark beside pad 1 (the 0.5 mm cross at
(32.95, 46.83)); no duplicate mark was added.

Verification for this revision: ERC **0 errors / 0 warnings**; DRC **2 warnings, 0 errors,
0 unconnected pads, 4 extra-footprint notes** — the same two `lib_footprint_mismatch` warnings
on J2/TH1 as Rev 1 and nothing new. Netlist versus Rev 1 (`f4f6edb`): 14 nets both sides,
`/P` +C3.1, `/GND` +C3.2, no other membership change. Render diff against the Rev 1 renders
confines every changed pixel cluster to the four intended areas (C3, the `REV 2` digit, U2's
label at its old and new positions, and the J1.2/J2.2 pad copper).

### The 2 remaining DRC warnings, justified

`[lib_footprint_mismatch]` on J2 and TH1 only (the same footprint at rotation 0 — J1 — is
clean). Probed, not assumed: a scratch copy with J2/TH1 forced to rotation 0 reports **no**
`lib_footprint_mismatch`, so the warning is triggered by the rotation of a pad-only
(no-copper-annulus) footprint during KiCad's library comparison. The board file itself is
authoritative for fabrication; the Gerbers are plotted from it, and the pad geometry
(4.0 mm pad, 3.0 mm drill, 7.2 mm pitch) is byte-identical to the library.

## LM393 pin 1 ↔ pin 5 — resolved (2026-11-17)

Stephen resolved the open item: the pin 1 ↔ pin 5 tie was a jumper he had added on his own
bench build from a misread while wiring the breadboard, and he removed it. It was never a
schematic feature — the drawing (pin 1 on `OA`, pin 5 on `N`) is right and this capture is
correct as delivered. No board or schematic change follows from it.

## Fabrication notes

2-layer, 1.6 mm FR-4, 1 oz copper, HASL, green mask. Minimum trace 0.2 mm, minimum clearance
0.2 mm, 0.3 mm on the 3 A nets. Plated through holes for all parts; the 3.0 mm wire-pad holes
take 1.5–2.5 mm² wire. Four M3 mounting holes (3.2 mm, no annular ring) 5 mm in from the
corners. Silkscreen marks every off-board pad (`P +`, `GND`, `SOLAR_OUT +5V`, `NTC CELL`) and
the board designation.

## Rebuilding

```bash
cd hardware/solar-gate-v2
kicad-cli sch erc --severity-all --format report --output erc.rpt solar-gate-v2.kicad_sch
kicad-cli pcb drc --severity-all --schematic-parity --output drc-report.rpt solar-gate-v2.kicad_pcb
kicad-cli pcb export gerbers --output fab/gerbers/ \
  -l F.Cu,B.Cu,F.SilkS,B.SilkS,F.Mask,B.Mask,Edge.Cuts solar-gate-v2.kicad_pcb
kicad-cli pcb export drill --output fab/drill/ --format excellon --excellon-units mm \
  --excellon-separate-th solar-gate-v2.kicad_pcb
kicad-cli pcb render --output renders/board-top.png --side top --width 2200 --height 1500 \
  --quality high --background opaque solar-gate-v2.kicad_pcb
```

Built with KiCad 10.0.8 and the KiCad MCP server; geometry and connectivity were verified
programmatically (netlist diff, DRC, courtyard/pad collision checks), not by eye.
