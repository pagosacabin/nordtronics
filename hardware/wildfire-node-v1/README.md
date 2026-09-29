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

- **30 components**: J1, F1, A1 (temp-gate block), U1–U5 (Heltec, PMS5003, AS3935, BME680, MiniBoost), C1–C10, R2 (4.7 kΩ EN pull-down), R3 (1 kΩ PMS-TX series), R21/R22 (TBD), Q7 (TBD), TP1–TP7.
- **19 nets** with more than one pin, plus 7 single-pin (intentionally unconnected) pins: PMS5003 `SET`/`RX`/`RESET`/`7`/`8` and BME680 `SDO`/`CS`, each carrying an explicit no-connect flag.
- **96 wire segments** (63 routed connections, no junction-less taps) and **45 net labels**, spread over the five power rails only.
- **ERC 2026-09-29: 0 errors / 30 warnings**, every warning `[lib_symbol_mismatch]` — the schematic library cache embedded on the host differs from the copies the Flatpak KiCad 10 sandbox compares against. This is a library-path artefact of the Flatpak install, not a connectivity or design fault; the connectivity was independently confirmed with `kicad-cli sch export netlist` (see below).

## How it was verified

1. `kicad-cli sch export netlist --format kicadxml` and a programmatic
   comparison of the exported net *partition* against the netlist in the task —
   all 19 multi-pin nets match member-for-member (77 connected pins), and the 7
   documented NC pins are isolated. This is the check that catches both opens
   and accidental shorts.
2. `kicad-cli sch erc --severity-all --format report` → 0 errors / 30 warnings.
3. `kicad-cli sch export svg` parsed back: all 30 references and every value are
   present in the rendered output as text.
4. PNG re-checked with PIL: 7016 × 4961 px, 600 dpi, RGB.

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
