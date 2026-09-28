# solar-gate-v2 — temperature-gated solar charging switch (schematic capture)

**SCHEMATIC ONLY. No PCB layout, no fab outputs, no bench validation, no footprints on
the passives.** Layout deliberately waits for the trip points to be validated on the
bench (task 0073). Nothing here supersedes `hardware/solar-gate-v1/`, which is a
different topology and is untouched by this project.

**0075 revision (current state of the files here):** the wired capture gained its I/O
connectors — **J1 "SOLAR IN"** on `P`/`GND` and **J2 "TO HELTEC"** on
`SOLAR_OUT`/`GND` — plus the NTC lead note and six functional-block annotations. No
component value, reference, net name or pre-existing wire was changed; the only netlist
difference is the two connectors (verified net-by-net, see Verification).

## Provenance

- Circuit: the Claude clean-sheet design selected from the four-way AI design review
  (ChatGPT, Grok, Claude, Copilot), Juno-verified, captured here verbatim in task 0073.
- Revision history of this directory: **0073** captured the circuit with net labels only
  (0 wires); **0074** redrew it so every connection is a real wire; **0075** added the
  I/O connectors, the NTC lead note and the functional-block annotations. Each revision
  is a separate branch; the files here are the 0075 state.
- Tooling: built entirely through the KiCad MCP server (mixelpixx/KiCAD-MCP-Server)
  driving KiCad 10; ERC executed by the Flatpak KiCad 10 `kicad-cli`. No KiCad file was
  hand-edited (the two library tables were written by tool calls, see Library
  conventions).
- Reference project for conventions: `hardware/solar-gate-v1/` (task 0039), whose
  `${KIPRJMOD}` symbol-table convention follows the 0043 fix.

## Topology

Warm cell → NTC resistance falls → `N` falls → once `N < Vb` the hot comparator pulls
the fault-OR node `C` low → Q3 off → gate node `G` floats up to `max(P, SOLAR_OUT)`
through `Rg` → all four pass FETs off. Cold cell → NTC resistance rises → `N` rises →
once `N > PA` the cold comparator pulls `C` low the same way. Charging is therefore
enabled only while the cell temperature sits between the two trip windows.

`Q1a`/`Q1b` (sources on `P`) and `Q2a`/`Q2b` (sources on `SOLAR_OUT`) are two
paralleled AO3401s each, wired **back to back** with their drains on the shared node
`M`, so neither body diode conducts in either direction while the pair is off. This is
the deliberate topology, not an oversight — do not "simplify" it to a single FET.

| Ref | Part | Value | Notes |
|-----|------|-------|-------|
| J1 | 2-pin header (`Connector_Generic:Conn_01x02`) | SOLAR IN | pin 1 → `P` (panel +), pin 2 → `GND`; JST GH 1.25 mm footprint |
| J2 | 2-pin header (`Connector_Generic:Conn_01x02`) | TO HELTEC | pin 1 → `SOLAR_OUT` (gated charge out), pin 2 → `GND`; JST GH 1.25 mm footprint |
| U1 | LM393 dual comparator | LM393 | drawn as both units + the power unit (pins 8/4) |
| U2 | TL431 shunt reference | TL431 | K = `VREF`, A = `GND`, REF tied to K → 2.5 V |
| Q1a, Q1b, Q2a, Q2b | AO3401 P-MOSFET | AO3401 | two paralleled pairs, gates on `G`, back to back on `M` |
| Q3 | 2N3904 NPN | 2N3904 | fault-OR driver, collector `G`, emitter `GND` |
| DA, DB, D1, D2 | BAT54 Schottky | BAT54 | fault-OR OR-ing (DA/DB) and gate max-selector (D1/D2) |
| NTC | 10k B3950 thermistor, 2-wire | 10k B3950 | thermally on the cell; open NTC fails safe to charging OFF |
| Rt | resistor | 34.0k | `VREF` → `N` |
| Rbias | resistor | 1.6k | `P` → `VREF` |
| R1, R2 | resistors | 100k, 95.3k | cold threshold divider, `PA` |
| Rfa, Rpua | resistors | 1.5M, 100k | cold hysteresis (OA→PA), cold pull-up (`VREF`→OA) |
| Rb1, Rb2 | resistors | 200k, 23.2k | hot threshold divider, `Vb` = 0.2599 V |
| Rfb, Rpub | resistors | 1.5M, 100k | hot hysteresis (OB→N), hot pull-up (`VREF`→OB) |
| Rpuc | resistor | 100k | fault-OR pull-up, `VREF` → `C` |
| Rb | resistor | 10k | `C` → Q3 base |
| Rg | resistor | 100k | `H` → `G` |
| C1, C2 | capacitors | 0.1uF | U1 supply decoupling (`P`→`GND`), U2 reference (`VREF`→`GND`) |
| #FLG01, #FLG02 | PWR_FLAG | — | ERC power-source markers on `P` and `GND` |

## Nets

14 nets, exactly as they appear in the schematic:

| Net | Pins |
|-----|------|
| `P` | C1/1, D1/A, **J1/1**, Q1a/S, Q1b/S, Rbias/1, U1/8 (V+) |
| `SOLAR_OUT` | D2/A, **J2/1**, Q2a/S, Q2b/S |
| `GND` | C1/2, C2/2, **J1/2**, **J2/2**, NTC/2, Q3/E, R2/2, Rb2/2, U1/4 (V−), U2/A |
| `VREF` | C2/1, R1/1, Rb1/1, Rbias/2, Rpua/1, Rpub/1, Rpuc/1, Rt/1, U2/K, U2/REF |
| `N` | NTC/1, Rfb/2, Rt/2, U1/A-IN−, U1/B-IN+ |
| `PA` | R1/2, R2/1, Rfa/2, U1/A-IN+ |
| `Vb` | Rb1/2, Rb2/1, U1/B-IN− |
| `OA` | DA/K, Rfa/1, Rpua/2, U1/A-OUT |
| `OB` | DB/K, Rfb/1, Rpub/2, U1/B-OUT |
| `C` | DA/A, DB/A, Rb/1, Rpuc/2 |
| `G` | Q1a/G, Q1b/G, Q2a/G, Q2b/G, Q3/C, Rg/2 |
| `M` | Q1a/D, Q1b/D, Q2a/D, Q2b/D |
| `H` | D1/K, D2/K, Rg/1 |
| `Q3B` | Q3/B, Rb/2 |

`M` (the back-to-back middle node) and `Q3B` (the `C`→base node) are the two nodes the
task spec did not name; both carry explicit labels so nothing is left implicit.

## Trip points — independent cross-check

Recomputed from the captured values (TL431 at 2.5 V, NTC 10k/B3950 at 25 °C, 0.2 V
open-collector saturation), independent of the schematic tooling:

| Point | Spec | Recomputed | Delta |
|-------|------|-----------|-------|
| `Vb` divider | 0.260 V | 0.2599 V | −0.0001 V |
| cold cutoff | −0.05 °C | −0.52 °C | −0.47 °C |
| cold re-enable | +2.3 °C | +1.68 °C | −0.62 °C |
| cold hysteresis band | 2.35 °C | 2.20 °C | −0.15 °C |
| hot trip | +48.05 °C | +48.10 °C | +0.05 °C |
| hot re-arm | +47.45 °C | +47.50 °C | +0.05 °C |
| hot hysteresis band | 0.60 °C | 0.60 °C | 0.00 °C |

The hot pair reproduces the spec to 0.05 °C and the cold *band width* reproduces to
0.15 °C, but the cold pair sits ~0.5 °C lower than the spec's numbers. A constant
offset with a matching band width points at an assumption difference in the spec's
arithmetic (the open-collector saturation voltage `VOL`, or the NTC's B-reference
temperature) rather than a wiring difference. Flagged for Stephen: which assumption is
intended, because it moves the cold trip by half a degree.

## ERC

```
** ERC messages: 24  Errors 0  Warnings 24
```

Full report: `solar-gate-v2-erc.txt` (written by `kicad-cli sch erc --severity-all`).

**Zero errors.** All 24 warnings are the single rule `[lib_symbol_mismatch]`
("Symbol 'R' / 'C' / 'D_Schottky' / 'Thermistor_NTC' / 'Conn_01x02' doesn't match copy in
library 'Device' / 'Connector_Generic'", and `PWR_FLAG` in `power`). 22 of them are the
same warnings 0074 reported; the two new ones are J1 and J2, i.e. the connector symbol
inherits the same cache-vs-library version skew and no new *class* of warning appeared.
This is a cache-vs-library version skew, not a schematic fault: the MCP symbol loader
embeds symbol copies from the host RPM KiCad 9.0.7 libraries, while ERC runs inside the
Flatpak KiCad 10 sandbox and compares against its own KiCad 10 copies of
`Device`/`power`/`Connector_Generic`. The project's own symbol (TL431) and the
comparator/transistor libraries raise no warning, which is the tell: only the libraries
whose content differs between 9.0.7 and 10 mismatch. No pin, net or connection is
affected.

The systemic fix is to point the kicad MCP server's environment at the Flatpak symbol
runtime (`KICAD_SYMBOL_DIR=/var/lib/flatpak/runtime/org.kicad.KiCad.Library.Symbols/x86_64/stable/active/files/symbols`),
which is how task 0039 took the same warning class to zero — that variable is no longer
present in the server's `env` block, so it has been lost to a config rewrite. It is not
restored here because the task is a schematic deliverable and an MCP-server restart
mid-run was judged the larger risk. Adding stock-library entries to a project
`sym-lib-table` does **not** work around it: the loader caches a resolved library path
per library name, so `update_symbol_from_library` simply re-injects the already-cached
host bytes.

## Verification (geometry measured on the committed revision)

The sheet was checked against its own SVG export (KiCad stroke-font glyph paths give
exact mm boxes for every string) and the raster render. Every revision passed that check
on its own committed state; the columns are 0073 (labels only), 0074 (wired) and 0075
(wired + connectors + annotations):

| Check | 0073 capture | 0074 wired revision | 0075 revision |
|-------|--------------|---------------------|---------------|
| `(wire` entries in the `.kicad_sch` | **0** | **115** (81 routed segments, broken at T-points by the tool) | **125** (7 new segments drawn, 3 existing wires split at the new T-points) |
| `(junction` entries | **0** | **39** | **43** |
| Net labels (supplements only since 0074) | 68 (the sole connectivity) | 14 | 14 |
| Free-form `(text` annotations | 0 | 0 | **9** (J1/J2 notes, NTC note, 6 block labels) |
| Wire-wire crossings (meeting strictly inside both segments) | 0 | 0 | **0** |
| Collinear wire overlaps | 0 | 0 | **0** |
| Real text-vs-text collisions (distinct strings, distinct places) | **85 pairs** | **0** | **0** |
| Text-over-wire overlaps | not measured | 0 | **0** |
| Text-over-symbol-body overlaps | not measured | 0 | **0** |
| Symbol bodies inside the title-block region (x>177, y>166) | 0 | **0** | **0** |
| Labels/text crossing the frame's inner border | 0 | **0** | **0** |
| Symbol instances | 30 (+ U1C, pins only) | 30 (+ U1C) | 32 (+ U1C, J1, J2) |

### 0075 connector wiring — why it is routed the way it is

Both connectors are `Connector_Generic:Conn_01x02` (2-pin, pins pointing left) at the
right-hand edge, and both are connected by **real wires**, not labels:

| Connector | Position | Signal pin | Ground pin |
|-----------|----------|------------|------------|
| J1 `SOLAR IN` | (283.21, 40.64) | pin 1 → `P`: wire west to a T-tap at (254.00, 40.64) on the existing `P` riser | pin 2 → wire west to a riser at x = 273.05, down to the extended bottom `GND` rail |
| J2 `TO HELTEC` | (276.86, 96.52) | pin 1 → `SOLAR_OUT`: wire west to a T-tap at (260.35, 96.52) on the existing `SOLAR_OUT` riser | pin 2 → wire west to a riser at x = 266.70, down to the same rail |

The two connectors are deliberately **staggered by 6.35 mm in x**: J2's pins end at
x = 271.78 and its body starts at 275.59, which leaves a 3.81 mm lane for J1's `GND`
riser to descend past J2 without touching anything. Aligning J1 with J2 instead forces
that riser to cross J2's `SOLAR_OUT` tap wire (electrically harmless in KiCad, but it
reads as a junction-free crossing on a sheet whose whole point is traceability), and any
shared riser left of the `SOLAR_OUT` column crosses the `M` net twice. The staggering is
therefore the crossing-free solution, and the checker above confirms **0 crossings and 0
collinear overlaps** on the whole sheet, 115-segment baseline included.

The bottom `GND` rail was extended from its old end (226.06, 147.32) east to
(273.05, 147.32); the two risers land on it as T-junctions. `(junction` went 39 → 43:
(254.00, 40.64), (260.35, 96.52), (266.70, 147.32), plus (226.06, 147.32) which the
extension turned from a 2-endpoint corner into a 3-way.

### 0075 annotation placement — measured clearances

The six block labels are placed in the widest free gap nearest each block (two of them
rotated 90° because the only gap local to their block is a narrow vertical lane), and
the two connector notes sit in the free strips above/below the right-hand column:

| Text | Position (mm) | Nearest other text | Nearest wire | Nearest symbol/frame ink |
|------|---------------|--------------------|--------------|--------------------------|
| `SOLAR PANEL IN (+/-) — 13 W, 5 V panel` | right-justified at (284.48, 26.67) | 6.93 | 33.43 | 0.80 (frame line x=285) |
| `TO HELTEC LoRa 32 V4 SOLAR INPUT — gated charge output` | right-justified at (284.48, 151.13) | 6.17 | 28.78 | 0.74 (frame line x=285) |
| `NTC on leads — thermally couple to the battery cell` | (56, 155) | 16.46 | 21.34 | 5.54 |
| `Vref (U2/TL431)` | (56, 86) | 9.27 | 8.56 | 2.88 |
| `cold comparator` | (157, 74) | 6.39 | 17.17 | 3.71 |
| `hot comparator` | (150, 138) | 4.94 | 10.66 | 9.31 |
| `fault-OR (DA/DB)` | rotated 90° at (221.6, 92) | 4.29 | 5.54 | 2.08 |
| `gate drive (Q3)` | rotated 90° at (207.5, 135) | 3.94 | 16.50 | 5.23 |
| `pass switch (Q1A/Q1B/Q2A/Q2B back-to-back)` | (204, 158.75) | 6.17 | 36.30 | 7.19 |

All distances are ink-to-ink in mm from the committed SVG export; every one is
positive (no overlap of any kind), and the numbers are reproducible from the SVG the
same way the 0074 checks were. The two long notes are `justify right` and end 0.74/0.80 mm
inside the frame's inner border line at x = 285 — close, but not on it.

Every Reference and Value is placed explicitly (`batch_set_schematic_property_positions`),
so the "auto-placed fields sitting on the body outlines" noted for 0073 is fixed: a value
now overlaps neither a symbol body nor a wire. Two placement traps worth remembering:
a field's **stored angle is relative to the symbol's rotation**, so a field on a rotated
symbol needs angle 90/270 to render horizontal (0 renders *vertical* on a rotated symbol),
and the **justification carries over**, so on a `justify:left` field the stored `(at x y)`
is the left edge of the ink, not its centre.

Residual, measured, all cosmetic and none affecting connectivity:

1. The SVG exporter draws six parts' fields and pin stubs **twice at identical
   coordinates** (12 strings + 12 stubs) — the `.kicad_sch` holds one instance of each
   (verified by uuid and property counts), the ink is byte-identical, and the effect is
   only ~6 % bolder glyphs in the raster.
2. The two `~` pin-name marks of each `Device:C` overlap each other by 0.02 mm² (stock
   symbol shape, not this capture).
3. The `H` net label clears the `Rb` value by 2.5 mm.

**Counting trap:** KiCad 10 serialises the token as `(wire\n`, so the obvious
`grep '(wire '` (trailing space) returns **0 on a fully wired sheet**. Count with
`grep -c '(wire'` or `grep -cE '^[[:space:]]*\(wire\b'` — both give 125 here (115 in the
0074 revision).

## Library conventions

- `sym-lib-table` — the project symbol library is registered with a **`${KIPRJMOD}`**
  URI (`(uri "${KIPRJMOD}/solar-gate-v2.kicad_sym")`), per the 0043 fix. Never give a
  project library an absolute path: it resolves only on the checkout that wrote it.
- `solar-gate-v2.kicad_sym` — the project symbol library. Currently holds `TL431`
  (imported from the v1 project library, the 0043 SOT-23 DBZ pinout:
  1 = K, 2 = REF, 3 = A).
- **Connectors use the stock `Connector_Generic:Conn_01x02` symbol**, so nothing was
  added to the project library for them. The footprint family chosen (0075) is
  **JST GH 1.25 mm, 2-pin, horizontal** —
  `Connector_JST:JST_GH_SM02B-GHS-TB_1x02-1MP_P1.25mm_Horizontal` — because v1 already
  standardises on JST GH 1.25 mm, the part is stocked (Digi-Key/Mouser/JLC), and it is a
  latching connector suited to a cable that plugs in at the panel. That reference
  resolves through KiCad's **global** `fp-lib-table` (shipped with KiCad), which is why
  no project `fp-lib-table` is needed yet even though J1/J2 carry footprints.
- **No `fp-lib-table` yet** — the passives/diodes/NTC still have no footprint assigned,
  so a project footprint table would have nothing project-specific to register. Layout
  (and the track-width classes v1 documents) is a later task.
- Note for whoever edits `sym-lib-table` next: `kicad-cli` 10 rejects `;` comment lines
  in a symbol library table. A commented table makes ERC report *"The current
  configuration does not include the symbol library 'solar-gate-v2'"* instead of
  resolving it. Keep this file comment-free.

## Deferred / open items

1. **Footprints are not assigned** to the passives, diodes or NTC (J1/J2 are the
   exception — see Library conventions). Q1a/Q1b/Q2a/Q2b (SOT-23), Q3 (TO-92) and U2
   (SOT-23) carry the symbol-default footprints only. Package selection (0805 vs 0603)
   belongs with layout, after bench validation.
2. **No project footprint library table** — J1/J2's footprints resolve through KiCad's
   global table; the passives have no footprints yet.
3. **24 `lib_symbol_mismatch` warnings** — see ERC above; a server-env fix, not a
   schematic fix.
4. **Cold trip ~0.5 °C below spec** — see the trip-point table; needs a decision on the
   assumption, not a schematic change.
5. **Connectors are schematic-level only.** J1/J2 exist as symbols with a chosen
   footprint family; no connector part has been bought or placed, and the panel/Heltec
   cable pinout (which wire goes to pin 1) is only implied by the sheet, not yet
   confirmed against the physical harness. NTC termination is still unusual: the
   10k B3950 is specified "on leads", so it is not going to be a board connector.
6. `solar-gate-v2.kicad_pcb` and `.kicad_prl` are what the project template emits; no
   layout exists in them.

## Files

| File | What it is |
|------|------------|
| `solar-gate-v2.kicad_sch` | the schematic (the deliverable) |
| `solar-gate-v2.kicad_sym` | project symbol library (TL431) |
| `sym-lib-table` | project symbol library registration (`${KIPRJMOD}`) |
| `solar-gate-v2.kicad_pro` | project file |
| `solar-gate-v2-erc.txt` | ERC report, `--severity-all` |
| `solar-gate-v2-schematic.svg` | rendered schematic |
