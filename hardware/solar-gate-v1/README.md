# solar-gate-v1 — solar cold-charge gate (EDA flow test vehicle)

**TEST BOARD ONLY. Not production, not reviewed, no Gerbers, no fab output.**

This project exists to cold-test the schematic → layout → autoroute rung of the
toolchain against a real circuit. The circuit itself is the approved solar
cold-charge gate spec, but nothing here has been bench-validated and nothing
should be built from it until Stephen has reviewed both the schematic and the
layout.

## Circuit

Warm → NTC low → `VA < VTH` → comparator output pulls `GATE` low → Q1 on.
Cold / no sun → `GATE` floats to `Panel+` through Rg → Q1 off.

| Ref | Part | Package | Notes |
|-----|------|---------|-------|
| Q1  | AO3401 (P-ch MOSFET) | SOT-23 | 1=G, 2=S, 3=D |
| U1  | TLV3401 (nanopower comparator, open-drain) | **SOT-23-5** | see pinout note below |
| D1  | TLV431 (shunt reference) | SOT-23-3 | 1=REF, 2=CATHODE, 3=ANODE |
| TH1 | 10k NTC B=3950, **off-board** → 2-pin JST GH 1.25 mm | — | board-edge connector |
| Rs  | 12k | 0805 | Panel+ → REF2V5 |
| R1, R2, R3 | 30k | 0805 | VA / VTH dividers |
| Rf  | 270k | 0805 | GATE → VA feedback |
| Rg  | 100k | 0805 | Panel+ → GATE pull-up |
| R4, R5 | 100k | 0805 | telemetry divider, GATE → GPIO |
| C1  | 100nF | 0805 | across D1 |
| C2  | 10nF | 0805 | VA → GND |
| J1, J2, J4 | JST GH 1.25 mm 2-pin | — | panel in / charger out / telemetry out |

Nets: `Panel+`, `GND`, `REF2V5`, `VA`, `VTH`, `GATE`, `CHG+`, `TELEM`.

## Board

2-layer, 40 × 30 mm, 1 oz copper, all parts on the front. Power path along the
top edge (J1 → Q1 → J2); TH1 and J4 on board edges; NTC is off-board.

Track widths (IPC-2221, external layer, 1 oz, 10 °C rise):

| Net class | Nets | Width | Rationale |
|-----------|------|-------|-----------|
| `POWER_1A` | `Panel+`, `CHG+`, `GND` | **0.80 mm** | 1 A needs 0.30 mm minimum; 0.8 mm ≈ 2.0 A capability |
| `SIGNAL_0R25` | `VA`, `VTH`, `GATE`, `REF2V5`, `TELEM` | 0.25 mm (routed at 0.20 mm, see below) | signal only |

`GND` is in the high-current class because it carries the 1 A return
(panel− → charger−).

## How it was built

Entirely through the KiCad MCP server (mixelpixx/KiCAD-MCP-Server 2.7.0) —
project/schematic creation, symbol authoring, wiring, ERC, board sync,
placement, autoroute via Freerouting 2.4.1, and DRC. No hand-edited KiCad
files, no hand-routing.

- ERC: `solar-gate-v1-erc.txt` — **0 errors, 0 warnings**
- DRC: `solar-gate-v1_drc_violations.json` — **2 errors, 0 warnings**
  (both are Freerouting fanout stubs at 0.15 mm on net `VA`, below the
  declared 0.20 mm minimum — router output, left as-is and reported)
- Autoroute: `solar-gate-v1.dsn` / `solar-gate-v1.ses`. Freerouting reported
  `0 unrouted and 0 violations`, final score 999.99; fanout escaped 39/39 SMD
  pins. 117 segments, 11 vias on the board; all 8 nets have copper.

## Open items for review

1. **D1 is spec'd as a 2.5 V reference but TLV431's VREF is 1.24 V.** With REF
   tied to CATHODE (the only sensible wiring, and required to make the net
   complete) this node is ~1.24 V, so R2/R3 halve it to ~0.62 V, not the
   specified 1.25 V. Either the reference should be a 2.5 V part (TL431) or the
   divider must be re-scaled.
2. **U1 pin 5 = VCC, pin 2 = GND, pin 3 = IN+, pin 4 = IN−** (TLV3401 has no
   SC-70-5 package; the spec's SC-70-5 call was corrected to SOT-23-5).
3. **J4 (telemetry out) is an addition** — the spec listed only panel in,
   GND and charger out, but `TELEM` has to leave the board somehow.
4. `SIGNAL_0R25` never took effect: those five nets fell back to the board's
   `Default` class and were routed at 0.20 mm (still fine for signals).
5. Two DRC `track_width` errors are unresolved router output, deliberately not
   hand-fixed.

## Environment notes

The Flatpak KiCad 10 sandbox has its own `/usr`, so host library paths are
invisible to it:

- `fp-lib-table` in this directory points at the Flatpak **KiCad 10 footprint
  runtime** (`/var/lib/flatpak/runtime/.../footprints/...`), a host path both
  the host `pcbnew` backend and the sandboxed `kicad-cli` 10 can read. Pointing
  at `/usr/share/kicad/footprints` instead makes ERC/DRC report every footprint
  library as "not enabled in the current configuration".
- The MCP server's symbol loader needs `KICAD_SYMBOL_DIR` set to the same
  runtime's `symbols/` directory, or the schematic's cached symbols are written
  from the host KiCad 9 libraries and every symbol then mismatches the KiCad 10
  library the ERC engine reads.
