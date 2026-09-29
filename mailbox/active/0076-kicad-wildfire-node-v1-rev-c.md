---
task_id: "0076"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
---

# 0076 — Draw Wildfire Node v1 Rev C in KiCad (schematic + PDF/PNG exports)

## Context

The Wildfire Node v1 Rev C schematic exists as a reviewed web drawing (inline
SVG, built 2026-09-29 from the full netlist below). Stephen wants a KiCad-native
version: a real `.kicad_sch` he can open, check, and eventually lay out from,
with PDF and PNG exports where every wire is visible.

You have proven this exact flow on your machine: 0073 drew the solar-gate-v2
schematic in KiCad, 0074 redrew it with real wires after Stephen's "there are no
connection lines" catch, and 0075 added I/O connectors and labels — delivering
`.kicad_sch`, `.kicad_pro`, vector PDF, high-dpi PNG, and an ERC report under
`hardware/solar-gate-v2/`. Do the same here under `hardware/wildfire-node-v1/`.

The netlist below is authoritative. The web drawing is a visual reference only;
where they disagree, the netlist below wins. The exact board is the Heltec
HTIT-WB32LAF V4.2 (ESP32-S3R2, 2 MB in-package quad PSRAM, 16 MB flash, SX1262
US915).

## Task

Draw the Wildfire Node v1 Rev C schematic in KiCad as one flat sheet and
commit these files on branch `hermes/0076-wildfire-node-v1-rev-c`:

- `hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sch`
- `hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_pro`
- `hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sym` (only if you
  create project-local symbols for the modules)
- `hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf` (vector export)
- `hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png` (phone-readable
  raster, minimum 200 dpi at A4/US-letter size)
- `hardware/wildfire-node-v1/wildfire-node-v1-rev-c-erc.txt` (ERC report)
- `hardware/wildfire-node-v1/README.md` (one paragraph on what/why, plus
  net and component counts)

### Netlist (authoritative)

**1. Power entry**
- J1 `BATT IN` — proposed JST-PH 2.0 mm, 2-pin (PROPOSED — confirm at bench).
  Pin 1: BAT+. Pin 2: BAT- (= GND, the single star-ground point).
- F1: Littelfuse RXEF075 PTC resettable fuse, 750 mA hold / 1.5 A trip / 72 V.
  In series with BAT+ immediately after J1 pin 1, before any split. Draw the
  proper fuse symbol.
- Net `VBAT_F`: the fused rail. It splits to MiniBoost IN+ and Heltec JP2 BAT.
- TP1 on `VBAT_F`. TP7 on GND (star point).

**2. MiniBoost 5 V (Adafruit PID 4654, TPS61023) + input decoupling**
- IN+ <- `VBAT_F`; IN- <- GND.
- C1 10 uF + C2 0.1 uF across the input (`VBAT_F` to GND), at the module.
- EN <- Heltec GPIO4. 4.7 kΩ pull-down resistor from EN to GND (the module
  already has a 100 kΩ EN→VIN pull-up; the pull-down guarantees a defined
  off-state).
- OUT+ = net `5V2`; OUT- = GND. TP2 on `5V2`.

**3. PMS5003 particulate sensor**
- Module pins: VCC, GND, SET, RX, TX, RESET, 7, 8.
- VCC <- `5V2`; GND <- GND.
- C3 47 uF bulk + C4 0.1 uF at the connector (VCC to GND).
- TX -> 1 kΩ series resistor -> Heltec GPIO5 (the MCU's RX line).
- TP6 on the PMS-TX line, MCU side of the 1 kΩ.
- SET, RX, RESET, pins 7-8: NC — place no-connect flags.
- Schematic note: allow >= 30 s warm-up; validate the data-frame checksum in
  firmware.

**4. Heltec HTIT-WB32LAF V4.2 (draw as a module box with these pins)**
- JP2 `BAT` <- `VBAT_F`; JP2 GND <- GND.
- JP3 `SOLAR` <- output of the existing temperature-gate block
  (`hardware/solar-gate-v2/`); draw the temp-gate as a labeled block with a
  wire to JP3.
- 3V3 rail: C7 0.1 uF + C8 10 uF (3V3 to GND). TP3 on 3V3.
- GPIO4 -> MiniBoost EN.
- GPIO5 <- PMS5003 TX (via the 1 kΩ).
- GPIO6 <- AS3935 INT (deep-sleep wake source).
- GPIO33 -> AS3935 MOSI.
- GPIO47 -> AS3935 SCK.
- GPIO48 <- AS3935 MISO.
- GPIO34 (VGNSS_Ctrl) -> Q7/R21-R22 gate stage -> AS3935 CS (active-low).
  Draw the Q7/R21/R22 stage between GPIO34 and CS; note "CS low approx 0.3 V;
  P3 must remain unpopulated". If any value in that stage is not given here,
  mark it TBD on the schematic rather than guessing.
- GPIO17 / GPIO18 -> I2C bus (SDA/SCL) shared by the BME680 and the onboard
  OLED (no separate OLED component — it is on the Heltec board).
- GPIO36 (Vext control) -> switched sensor rail; net `Vext` feeds BME680 VIN.

**5. AS3935 lightning sensor (SparkFun SEN-15441 breakout)**
- VCC <- 3V3; GND <- GND.
- C5 0.1 uF + C6 10 uF at the breakout (3V3 to GND).
- MOSI <- GPIO33; SCK <- GPIO47; MISO -> GPIO48; CS <- Q7 gate stage
  (active-low); INT -> GPIO6.
- TP5 on the INT line.
- Schematic notes: SPI mode 1, 1 MHz (max 2 MHz; avoid 500 kHz). MISO
  tri-states while CS is high. Read/clear IRQ before entering sleep.
  Listening current approx 60-80 uA.

**6. BME680 (breakout)**
- VIN <- `Vext` (provisional — see warning).
- C9 1 uF on `Vext` (Vext to GND); C10 0.1 uF at breakout VIN (VIN to GND).
- TP4 on `Vext`.
- SDA/SCL <- GPIO17/GPIO18. SDO: NC. CS: NC (no-connect flags).
- Schematic warning: verify the breakout's `3Vo` rail and I2C pull-up rail at
  the bench with 3.3 V `Vext` into VIN — all I2C pull-ups must die with `Vext`.

**7. Title block and schematic notes**
- Title: `Wildfire Node v1 — Schematic Rev C (draft)`; date `2026-09-29`.
- ERC status note in the title block reflecting the actual ERC run — write the
  real result (e.g. `ERC 2026-09-29: 0 errors / N warnings`), not "not checked".
- Note: `Connectors marked PROPOSED — confirm at bench.`
- Note: `Schematic only — bench test gates any PCB layout.`
- Carry these still-open items as schematic notes: verify the LiPo pouch has
  built-in over-discharge protection (MiniBoost UVLO alone is inadequate);
  physically verify GPIO47/48 reach usable header pads; analyze PMS5003
  UART/control-line back-power with the boost off.

## Success criteria

- [ ] `hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sch` opens in
      KiCad with every component, refdes, and value from the netlist above.
- [ ] Every signal connection in the netlist is a drawn wire segment. Net
      labels are used only for the power nets `VBAT_F`, `5V2`, `3V3`, `Vext`,
      and `GND`. (Stephen's explicit "make sure to see wiring" — the 0074
      lesson.)
- [ ] TP1-TP7, F1, C1-C10, the 4.7 kΩ EN pull-down, and the 1 kΩ PMS-TX series
      resistor are all present with correct values.
- [ ] PDF (vector) and PNG (phone-readable, >= 200 dpi) exports are committed.
- [ ] ERC has been run; the report is committed; the title block states the
      real ERC result.
- [ ] Branch `hermes/0076-wildfire-node-v1-rev-c` is pushed to origin; the SHA
      is reported.

## Constraints

- Schematic only. No PCB layout — the bench test gates any layout (standing
  rule).
- Worker model is deepseek-flash (Stephen's call). Keep the run cheap: KiCad
  CLI exports cost nothing, model calls cost money. Schedule heavy compute for
  DeepSeek off-peak if it matters.
- Every connection drawn as a wire (see success criteria). No floating pins
  except the documented NCs, which get no-connect flags.
- Connector choices marked PROPOSED stay marked PROPOSED. Do not invent
  manufacturer part numbers.
- If any pin or value in the netlist is ambiguous, put a visible TBD note on
  the schematic rather than guessing.
- Push only to the task branch. No secrets in the repo, ever.

## Proof

- Branch `hermes/0076-wildfire-node-v1-rev-c` on origin + SHA, verified with
  `git ls-remote`.
- File list as in the Task section, all present on the branch.
- ERC summary line (errors / warnings).
- PNG dimensions and dpi.

## Reply format

Stage your reply as `mailbox/staged/0076-kicad-wildfire-node-v1-rev-c.md` with
front matter carrying `task_id: "0076"`, `status: staged`, `iteration`, and a
`proof:` block (branch, sha, files, renders with dimensions, erc, model), then
a Reply section with net/component counts and any TBD notes or deviations from
the netlist above.
