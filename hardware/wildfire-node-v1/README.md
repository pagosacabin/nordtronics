# Wildfire Node v1 — Rev C schematic

KiCad-native capture of the Wildfire Node v1 Rev C schematic, drawn from the
authoritative netlist in task 0076 (the earlier web drawing was the visual
reference only; where the two disagreed the netlist won). It exists so the
board can be opened, checked and eventually laid out from a real `.kicad_sch`
instead of an SVG drawing, and it is deliberately **schematic-only** — the
bench test gates any PCB layout, so no board file is committed. Every signal
connection is a drawn wire segment; the only net labels are on the five power
rails `VBAT_F`, `5V2`, `3V3`, `Vext` and `GND`. The release target board is the
Heltec HTIT-WB32LAF V4.2 (ESP32-S3R2, 2 MB in-package PSRAM, 16 MB flash,
SX1262 US915), fed from a LiPo through a resettable fuse into an Adafruit
MiniBoost 5 V module, with a PMS5003 particulate sensor, an AS3935 lightning
sensor breakout and a BME680 on the switched `Vext` rail.

Task 0077 re-laid the sheet out for readability at Stephen's request: 0076's
electrically correct drawing was crowded into the left ~58 % of the page, wires
crossed the notes block and labels overlapped. Placement and routing changed;
**the netlist did not** — see "How it was verified".

## Sheet layout (Rev C, 0077)

| Zone | Contents |
|---|---|
| top-left | J2 solar-panel input, J1 battery input, F1 polyfuse, TP1, TP7 (GND star point), A1 solar-gate-v2 temp-gate |
| top-centre | U5 MiniBoost 5 V + C1/C2 + R2 (4.7 kΩ EN pull-down), TP2 on `5V2` |
| centre | U1 Heltec module — the hub, GPIO wires radiating to its neighbours |
| right | U2 PMS5003 + C3/C4 + R3 (1 kΩ) + TP6, on the `5V2` rail |
| below centre | Q7/R21/R22 gate stage into U3 AS3935 + C5/C6 + TP5 |
| bottom-left | U4 BME680 + C9/C10 + TP4 on the `Vext` rail |
| bottom-left band, level with the title block | one dedicated notes block, with a routing keep-out: no wire and no component enters it |
| bottom-right | KiCad title block (default A4 frame), carrying the real ERC line |

Components are spread across the full sheet width (left, centre and right thirds
all carry modules); the notes block occupies the bottom-left band so no wire can
cross it, and every Ref/Value field was measured in the exported vector and moved
to a free site rather than left on a symbol body or on top of another string.

## Files

| File | What it is |
|---|---|
| `wildfire-node-v1-rev-c.kicad_sch` | the schematic (KiCad 10 format, one flat A4 sheet) |
| `wildfire-node-v1-rev-c.kicad_pro` | KiCad project |
| `wildfire-node-v1-rev-c.kicad_sym` | project-local module symbols (module boxes drawn from the netlist pin lists) |
| `sym-lib-table` | project symbol library table (nickname `wildfire-node-v1`) |
| `wildfire-node-v1-rev-c.pdf` | vector PDF export |
| `wildfire-node-v1-rev-c.png` | raster export, 7016 × 4961 px = 600 dpi at A4 landscape |
| `wildfire-node-v1-rev-c-erc.txt` | ERC report (`kicad-cli sch erc`, severities all) |

## Counts

- **31 components**: J1, J2 (panel input), F1, A1 (temp-gate block), U1–U5 (Heltec, PMS5003, AS3935, BME680, MiniBoost), C1–C10, R2 (4.7 kΩ EN pull-down), R3 (1 kΩ PMS-TX series), R21/R22 (TBD), Q7 (TBD), TP1–TP7.
- **20 nets** with more than one pin, plus 35 single-pin (intentionally unconnected) pins: PMS5003 `SET`/`RX`/`RESET`/`7`/`8`, BME680 `SDO`/`CS`, and the 28 unused module pins of U1, each carrying an explicit no-connect flag.
- **115 wire segments** and **46 net labels** — the five power rails (`VBAT_F`, `5V2`, `3V3`, `Vext`, `GND`) plus the signal net `SOLAR_IN`.
- **19 note lines** on the sheet (the 17 from 0077 plus two describing the U1 pin-numbering policy and the A1/J2 interface).
- **ERC 2026-09-29: 0 errors / 31 warnings**, every warning `[lib_symbol_mismatch]` — the schematic library cache embedded on the host differs from the copies the Flatpak KiCad 10 sandbox compares against. This is a library-path artefact of the Flatpak install, not a connectivity or design fault; the connectivity was independently confirmed with `kicad-cli sch export netlist` (see below).

## How it was verified

1. **Connectivity (the check that matters).** `kicad-cli sch export netlist
   --format kicadxml` on the re-laid-out sheet and on the 0076 revision, then a
   programmatic comparison of the net *partition*: identical — 19 multi-pin nets /
   77 connected pins, the same members under the same net names, and the 7
   documented NC pins still isolated. A merged net (short) or a split net (open)
   would both show up here.
2. **ERC.** `kicad-cli sch erc --severity-all --format report` → 0 errors / 30
   warnings, all `[lib_symbol_mismatch]`, matching the 0076 revision. The
   title-block ERC line states the same counts as the committed report.
3. **The drawing itself, measured in the exported vector.** The SVG export was
   parsed back and every string measured in millimetres: all 30 reference
   designators and values are present in the render, **0** text-vs-text overlaps
   (0076's headline defect), **0** wire segments crossing a text glyph, **0**
   wire segments inside the notes-block or title-block keep-outs, and no field
   further than ~3 mm from its own symbol. All 17 note lines are present.
4. **Renders.** PNG re-opened with ImageMagick: 7016 × 4961 px, 600 dpi, RGB.
5. **Layout neutrality.** Placement and routing only: same 30 components, same
   values, same pins, same nets. The only non-geometric change is the field
   *angle* on the three rotated symbols (J1, F1, R3), which makes their Ref/Value
   render horizontally instead of vertically.

## Rev C interface fixes (0079)

Review of the 0077/0078 render found two interface defects that were electrical,
not cosmetic. Task 0079 fixed both:

1. **A1 is now a three-pin series gate.** It was drawn with the left pin = `GND`
   and the right pin = `SOLAR_OUT`, which modelled the temp-gate board as a
   shunt, and the panel input appeared nowhere on the sheet. A1 is now
   `SOLAR_IN` (pin 1, left) → `SOLAR_OUT` (pin 2, right) → the Heltec `SOLAR`
   pin, with `GND` (pin 3, bottom) to the ground star.
2. **J2 is the panel input** — `PANEL IN (13W 5V panel)`, a 2-pin connector left
   of A1. Its pin 1 feeds A1's `SOLAR_IN` on the new net `SOLAR_IN`; pin 2
   returns to the ground star.
3. **U1 shows the full Heltec V4.2 pinout — 42 pins.** The 14 previously
   connected module pins keep their 0076/0077 *sites on the symbol* and their
   original pin numbers 1–14, so every frozen net member is byte-identical; the
   28 added pins are numbered 15–42 (Heltec header order: J2 header, then J3
   header, then the XTAL pins) and each carries a no-connect flag. The block grew
   from 30.48 × 30.48 mm to 30.48 × 55.88 mm with the pin rows interleaved — a
   block re-ordered by header would have moved all 14 connected pins and
   re-routed every frozen net.

Netlist against the 0077/0078 baseline: 19 → 20 multi-pin nets, 7 → 35 NC
singletons, and exactly three changes — the new `SOLAR_IN` net (`J2.1` + `A1.1`),
the `GND` net gaining `J2.2` and renumbering `A1.2` → `A1.3`, and the unnamed
`SOLAR_OUT` net renumbering `A1.1` → `A1.2` (the pin renumbering the task
specifies). No other net changed and no net was lost.

Layout was re-verified by measuring the exported vector: 0 text-vs-text overlaps,
0 wire segments crossing a text glyph, 0 wires inside the notes-block or
title-block keep-outs — over the whole sheet, including the 42 pin names, their
42 pin numbers and the 28 no-connect flags.

## Open items carried on the sheet

- Verify the LiPo pouch has built-in over-discharge protection (MiniBoost UVLO
  alone is inadequate).
- Physically verify GPIO47/GPIO48 reach usable header pads on the Heltec V4.2.
- Analyse PMS5003 UART/control-line back-power with the boost off.
- Verify the BME680 breakout's `3Vo` rail and I2C pull-up rail at the bench with
  3.3 V `Vext` into VIN; all I2C pull-ups must die with `Vext`.
- Q7/R21/R22 values are **TBD** — the netlist does not specify them, and the
  CS-low ≈ 0.3 V note plus "P3 must remain unpopulated" are carried as schematic
  text rather than guessed.
- J1 is a **PROPOSED** JST-PH 2.0 mm 2-pin connector — confirm at bench.
