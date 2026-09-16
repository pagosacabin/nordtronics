# Custom LiFePO4 Battery Builds

Custom lithium-iron-phosphate battery packs, designed and built in-house in
Pagosa Springs, CO: cell sourcing and grading, JK BMS selection and tuning,
balance-lead harnesses, custom busbars, DC disconnects, and enclosures.

## What goes into a build

- **Cells** — prismatic LiFePO4 cells (EVE 280Ah), sourced and graded
  before assembly; matched sets per pack.
- **BMS** — JK BMS units, configured and tuned per build (charge/discharge
  limits, balance thresholds, temperature protection).
- **Interconnects** — custom busbars designed for the EVE 280Ah cell
  footprint (`busbar/`, multiple iterated versions).
- **Main battery PCB** — custom KiCad board for the pack's core wiring and
  BMS integration, iterated over ~25 revisions (`battery-pcb/`).
- **Balance leads, DC disconnects, enclosure** — full DC-side assembly
  done in-house.

Packs are built to order — cell count, voltage, and capacity per project.
Build photos available on request.

## In this folder

- `battery-pcb/` — versioned KiCad project archives for the main battery
  PCB (JK BMS based LiFePO4 pack), Feb 2024 iterations
- `busbar/` — busbar designs for EVE 280Ah cells, several versions
