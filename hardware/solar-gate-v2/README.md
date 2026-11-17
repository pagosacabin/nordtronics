# Solar gate — temperature cutoff, Mark 1 Rev 1

An add-in module for the wildfire-node power path: `panel (+) → this board → Heltec V4
solar input`. It switches the panel feed off when the cell is too cold or too hot, using a
10 k B3950 NTC bonded to the cell, an LM393 window comparator and a TL431 reference. The
pass element is two back-to-back FQP27P06 pairs (common drain at `M`), so the board blocks
current in both directions when off — that is what makes `P` never beep to `SOLAR_OUT`.

- **Board designation:** Mark 1 Rev 1 (silkscreen, title block)
- **Outline:** 85 × 60 mm, 2 layers, 1.6 mm, through-hole, 1 oz copper (Option B from
  `FORM-FACTOR-OPTIONS.md`, picked by Stephen 2026-11-16)
- **Off-board connections:** solder pads only — `P`, `GND`, `SOLAR_OUT`, `TH1` (NTC leads).
  JST GH is deferred to a future revision by Stephen's call.
- **Design current:** 3 A worst case through the Q1/Q2 pass paths (normal run ≈ 540 mA)

## Schematic source

The circuit is Stephen's as-built bench build, confirmed 2026-11-16 as **equal to the
drawing**. This capture starts from the last capture in the repo
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
| `solar-gate-v2.kicad_sch` | Schematic, A3, Mark 1 Rev 1 title block |
| `solar-gate-v2.kicad_pcb` | PCB, 85 × 60 mm, routed 2-layer |
| `sym-lib-table`, `fp-lib-table` | Project library tables (comment-free; `KICAD10_*_DIR`) |
| `drc-report.rpt` | DRC run on the delivered board, `--severity-all --schematic-parity` |
| `fab/gerbers/` | Gerbers (F/B copper, mask, silkscreen, Edge.Cuts) + `.gbrjob` |
| `fab/drill/` | Excellon drill, separate PTH / NPTH |
| `fab/bom.csv` | BOM grouped by value + footprint |
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
- Routing: 165 segments, 1 via, both layers; all 14 nets connected (0 unconnected pads).

### The 2 remaining DRC warnings, justified

`[lib_footprint_mismatch]` on J2 and TH1 only (the same footprint at rotation 0 — J1 — is
clean). Probed, not assumed: a scratch copy with J2/TH1 forced to rotation 0 reports **no**
`lib_footprint_mismatch`, so the warning is triggered by the rotation of a pad-only
(no-copper-annulus) footprint during KiCad's library comparison. The board file itself is
authoritative for fabrication; the Gerbers are plotted from it, and the pad geometry
(4.0 mm pad, 3.0 mm drill, 7.2 mm pitch) is byte-identical to the library.

## Open item for Stephen (declared, not worked around)

The task card's bench narrative says the LM393 **pin 1 ↔ pin 5** node "stands as drawn",
while the drawing in the repo has **pin 1 on `OA`** and **pin 5 on `N`** — different nets.
The netlist is unambiguous (`/OA: U1.1 …`, `/N: U1.2, U1.5 …`), and this board reproduces the
drawing, because the card makes the as-built-equals-drawing its source of truth. If the
physical build really does tie pin 1 to pin 5, that is one cut-and-strap on this revision
(pin 1 to the `N` label at pin 5) and a schematic note for Rev 2 — it is not a layout change.
Nothing else on the board depends on it.

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
