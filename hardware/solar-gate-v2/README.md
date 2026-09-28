# solar-gate-v2 — temperature-gated solar charging switch (schematic capture)

**SCHEMATIC ONLY. No PCB layout, no fab outputs, no bench validation, no footprints on
the passives.** Layout deliberately waits for the trip points to be validated on the
bench (task 0073). Nothing here supersedes `hardware/solar-gate-v1/`, which is a
different topology and is untouched by this project.

## Provenance

- Circuit: the Claude clean-sheet design selected from the four-way AI design review
  (ChatGPT, Grok, Claude, Copilot), Juno-verified, captured here verbatim in task 0073.
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
| `P` | C1/1, D1/A, Q1a/S, Q1b/S, Rbias/1, U1/8 (V+) |
| `SOLAR_OUT` | D2/A, Q2a/S, Q2b/S |
| `GND` | C1/2, C2/2, NTC/2, Q3/E, R2/2, Rb2/2, U1/4 (V−), U2/A |
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
** ERC messages: 22  Errors 0  Warnings 22
```

Full report: `solar-gate-v2-erc.txt` (written by `kicad-cli sch erc --severity-all`).

**Zero errors.** All 22 warnings are the single rule `[lib_symbol_mismatch]`
("Symbol 'R' / 'C' / 'D_Schottky' / 'Thermistor_NTC' doesn't match copy in library
'Device'", and `PWR_FLAG` in `power`). This is a cache-vs-library version skew, not a
schematic fault: the MCP symbol loader embeds symbol copies from the host RPM KiCad
9.0.7 libraries, while ERC runs inside the Flatpak KiCad 10 sandbox and compares
against its own KiCad 10 copies of `Device`/`power`. The project's own symbol (TL431)
and the comparator/transistor libraries raise no warning, which is the tell: only the
libraries whose content differs between 9.0.7 and 10 mismatch. No pin, net or
connection is affected.

The systemic fix is to point the kicad MCP server's environment at the Flatpak symbol
runtime (`KICAD_SYMBOL_DIR=/var/lib/flatpak/runtime/org.kicad.KiCad.Library.Symbols/x86_64/stable/active/files/symbols`),
which is how task 0039 took the same warning class to zero — that variable is no longer
present in the server's `env` block, so it has been lost to a config rewrite. It is not
restored here because the task is a schematic deliverable and an MCP-server restart
mid-run was judged the larger risk. Adding stock-library entries to a project
`sym-lib-table` does **not** work around it: the loader caches a resolved library path
per library name, so `update_symbol_from_library` simply re-injects the already-cached
host bytes.

## Render verification (geometry measured on the committed revision)

The sheet was checked against its own SVG export (KiCad stroke-font glyph paths give
exact mm boxes for every string) and the raster render, and the first layout was
rejected and rebuilt because of what the check found:

| Check | First layout | Committed revision |
|-------|--------------|--------------------|
| Symbol bodies inside the title-block region (x>177, y>166) | **4** (DB, Rb, Q3, Q2B; Q3 bisected by the rule at y=166) | **0** |
| Labels crossing the frame's inner border | **1** (`SOLAR_OUT` through x=285) | **0** |
| Symbol bodies found | 30 (+ U1C, which KiCad draws as pins only) | 30 (+ U1C) |
| Drawing extents (material only) | x 34.5–275.6, y 40.6–180.3 mm | x 33.1–279.7, y 36.1–162.5 mm |

Residual cosmetic overlaps, measured, all inside the border and none affecting
connectivity: the `#FLG02` `PWR_FLAG` value crossing its own vertical `GND` label
(1.27 × 1.39 mm, 41 % of the smaller box), the `U1C` pin name `V-` sitting inside that
unit's `LM393` value (0.79 × 1.27 mm), `U2` clipping a `VREF` label (0.45 × 0.47 mm),
and one `M`/`SOLAR_OUT` label pair at the `Q2B` drain overlapping by 0.26 mm². A future
polish pass should also nudge the auto-placed Reference/Value fields, which sit on the
body outlines. None of these is visible at normal zoom.

## Library conventions

- `sym-lib-table` — the project symbol library is registered with a **`${KIPRJMOD}`**
  URI (`(uri "${KIPRJMOD}/solar-gate-v2.kicad_sym")`), per the 0043 fix. Never give a
  project library an absolute path: it resolves only on the checkout that wrote it.
- `solar-gate-v2.kicad_sym` — the project symbol library. Currently holds `TL431`
  (imported from the v1 project library, the 0043 SOT-23 DBZ pinout:
  1 = K, 2 = REF, 3 = A).
- **No `fp-lib-table` yet** — no footprint has been assigned, so no footprint table is
  needed. Layout (and the track-width classes v1 documents) is a later task.
- Note for whoever edits `sym-lib-table` next: `kicad-cli` 10 rejects `;` comment lines
  in a symbol library table. A commented table makes ERC report *"The current
  configuration does not include the symbol library 'solar-gate-v2'"* instead of
  resolving it. Keep this file comment-free.

## Deferred / open items

1. **Footprints are not assigned** to the passives, diodes or NTC. Q1a/Q1b/Q2a/Q2b
   (SOT-23), Q3 (TO-92) and U2 (SOT-23) carry the symbol-default footprints only.
   Package selection (0805 vs 0603) belongs with layout, after bench validation.
2. **No footprint library table** — deliberately absent while no footprints are set.
3. **22 `lib_symbol_mismatch` warnings** — see ERC above; a server-env fix, not a
   schematic fix.
4. **Cold trip ~0.5 °C below spec** — see the trip-point table; needs a decision on the
   assumption, not a schematic change.
5. **No connectors.** The spec lists no connectors, so `P`, `SOLAR_OUT` and `GND` are
   bare labelled nets. Whoever lays this out needs to choose the panel/charger/NTC
   terminations; v1's JST GH 1.25 mm choice is a reasonable precedent.
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
