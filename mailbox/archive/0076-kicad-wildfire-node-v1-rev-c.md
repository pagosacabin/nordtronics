---
task_id: "0076"
protocol_version: 1.0.0
status: verified
iteration: 2
expect-reply-within: 6h
proof:
  branch: hermes/0076-wildfire-node-v1-rev-c
  sha: 68c6e2fcf1efc9335683b08c6273565e0ba35662
  run: none — no workflow in this repo triggers on this branch or these paths (enumeration below)
  main_transition_run: https://github.com/pagosacabin/nordtronics/actions/runs/36634243804
  files:
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sch
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_pro
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.kicad_sym
    - hardware/wildfire-node-v1/sym-lib-table
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png
    - hardware/wildfire-node-v1/wildfire-node-v1-rev-c-erc.txt
    - hardware/wildfire-node-v1/README.md
  renders:
    pdf: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.pdf (vector, one A4 landscape page)
    png: hardware/wildfire-node-v1/wildfire-node-v1-rev-c.png (7016x4961 px = 600 dpi A4 landscape, RGB)
  erc: 0 errors / 30 warnings — all [lib_symbol_mismatch] (Flatpak KiCad 10 libraries vs host/RPM copies)
  netlist: 19 multi-pin nets / 30 components — exported partition compared member-for-member against the task netlist
  model: deepseek-flash
---

# 0076 — Draw Wildfire Node v1 Rev C in KiCad (schematic + PDF/PNG exports)

**Status: staged.** Deliverable is on `hermes/0076-wildfire-node-v1-rev-c` @
`68c6e2fcf1efc9335683b08c6273565e0ba35662`
(`git ls-remote --heads origin hermes/0076-wildfire-node-v1-rev-c` == local `HEAD`, verified this run).

## What changed

One new directory, `hardware/wildfire-node-v1/`, drawn from the authoritative netlist as a
single flat A4 sheet with real drawn wiring — not a label-only capture.

- **30 components.** J1 (BATT IN), F1 (Polyfuse, drawn with the proper resettable-fuse symbol),
  A1 (the solar-gate-v2 temperature-gate block, drawn as a labelled box with a wire to JP3/SOLAR),
  U1 Heltec HTIT-WB32LAF V4.2, U2 PMS5003, U3 AS3935 (SEN-15441), U4 BME680, U5 MiniBoost 5 V
  (TPS61023), C1–C10, R2 4.7 kΩ EN pull-down, R3 1 kΩ PMS-TX series, R21/R22 (TBD), Q7 (TBD),
  TP1–TP7. The five module parts are drawn as module boxes with exactly the netlist's pins,
  from a project-local symbol library (`wildfire-node-v1.kicad_sym` + `sym-lib-table`).
- **19 multi-pin nets**, plus the 7 documented NC pins (PMS5003 `SET`/`RX`/`RESET`/`7`/`8`,
  BME680 `SDO`/`CS`) which carry explicit no-connect flags and stay isolated.
- **Every signal connection is a drawn wire** (63 routed connections → 96 committed wire segments).
  Net labels appear **only** on `VBAT_F`, `5V2`, `3V3`, `Vext` and `GND` — 45 labels, and the
  distinct label-name set in the file is exactly those five (checked by grep).

## How it was verified (what I actually ran)

1. **Connectivity**: `kicad-cli sch export netlist --format kicadxml`, then a programmatic
   comparison of the exported net *partition* against the netlist in this task. All 19 multi-pin
   nets match member-for-member (77 connected pins) and the 7 NC pins are singletons → PASS.
   This is what catches both opens and accidental shorts; it is the check that mattered, because
   routing was generated rather than hand-drawn.
2. **ERC**: `kicad-cli sch erc --output …-erc.txt --severity-all --format report` on the final
   file → `0 errors / 30 warnings`, every one `[lib_symbol_mismatch]`. The report committed is
   this run on the committed revision. The 30 warnings are a library-path artefact of the Flatpak
   install (the sandbox's KiCad 10 libraries differ from the copies embedded on the host), not a
   connectivity or design fault — same class as 0075's warnings.
3. **Renders**: the committed PNG was re-opened with PIL — 7016 × 4961 px, 600 dpi, RGB.
   The exported SVG was parsed back and **all 30 reference designators and every value are present
   in the rendered text** (so nothing is off-sheet or unplaced), and the title block renders with
   the real ERC line.
4. **RC note**: the title block carries `ERC 2026-09-29: 0 errors / 30 warnings …` matching the
   committed report exactly, plus the PROPOSED-connector and schematic-only notes.

## CI — why `run: none` is correct here (enumerated, not assumed)

`ls .github/workflows/` → `android-build.yml`, `platformio.yml`, `website-check.yml`:

- `android-build.yml` — `on.push.branches: [android-toolchain-setup]` → this branch not matched.
- `platformio.yml` — `on.push.paths: firmware/tank-monitor/**, firmware/node-v1/**,
  .github/workflows/platformio.yml` (plus a matching `pull_request` and `workflow_dispatch`)
  → `hardware/**` not matched.
- `website-check.yml` — `on.push.branches: [main, hermes/0068-site-email-refresh,
  hermes/0070-site-rewrite]` → this branch not matched.

`gh run list -R pagosacabin/nordtronics --branch hermes/0076-wildfire-node-v1-rev-c` returns
empty (exit 0, no rows). So a push to this branch starts zero runs; there is no CI run to cite and
no artifact to notify. **No ntfy receipt for this task**: the topic is for companion *builds*
(`nordtronics-build-ed05a663`), and this task compiles nothing — its deliverables are a schematic
and its exports.

## Deviations, TBDs and corrections

- **TBD, by instruction**: Q7, R21 and R22 values are not given in the netlist, so they are marked
  `TBD` with the "CS low approx 0.3 V; P3 stays unpopulated" constraint carried as schematic text
  rather than guessed. Nothing was invented.
- **J1 pin 2 wired to GND.** The netlist's power-entry section says J1 pin 2 is BAT− and that it is
  the single star-ground point; my first pass left it unconnected, and the netlist check caught it.
  It is now a GND label stub, and TP7 sits on the same rail. Worth a look at bench: "the single
  star-ground point" is a layout statement, and on this flat sheet GND is distributed by label.
- **No PCB file committed.** The KiCad project helper emitted an empty `.kicad_pcb`; I deleted it,
  since the task is schematic-only and a stray empty board file invites the wrong reading.
- **`;`-free `sym-lib-table`.** Written without comment lines, which kicad-cli 10's symbol-library
  table parser rejects even though the footprint table tolerates them.
- **Cosmetic, declared**: 7 text-box overlaps remain in the render, all between 1–3 character
  strings (pin numbers and short power-rail labels adjacent to each other, e.g. U5 pin `4` against
  the `5V2` label at the TP2 stub). I moved the module/connector Value fields out of the symbol
  bodies and corrected the field angles on the three rotated parts (a field's angle is *relative*
  to the symbol's rotation, so F1's value was rendering 40 mm tall); the residue is label-density
  in the two busiest corners and I judged it not worth re-routing at the cost of connectivity risk.
- **Process correction (declared)**: the pickup commit `88f8919` staged only the `git mv` rename —
  the `git add` listed the renamed path *and* the now-missing inbox path, so the add aborted and the
  front-matter edit (`status: in_progress`, `iteration: 2`) stayed in the worktree. Follow-up commit
  `ddf88bb` on main records it. `iteration` was incremented once, at pickup, as the protocol requires;
  this was a staging slip, not a second pickup.

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
