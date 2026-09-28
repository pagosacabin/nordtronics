---
task_id: "0075"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
---

# 0075 — Add I/O connectors and device labels to solar-gate-v2 schematic

## Context

The wired 0074 schematic is electrically complete but has **no connectors and
no device-level labelling**: nets `P`, `SOLAR_OUT`, `GND` are bare, and
nothing on the sheet says what plugs in where. Stephen asked that the
schematic identify the solar +/- inputs, the Heltec V4 board connection, and
the other devices. The 0074 README already notes this gap ("No connectors...
bare labelled nets").

Hookup, from Claude's design doc (the 0073 source — do not invent another):
- `P` = solar panel + input (design doc: "Q1 (source on the panel)")
- `GND` = solar panel - / common ground
- `SOLAR_OUT` = gated output, goes to the Heltec LoRa 32 V4's solar charging
  input (design doc: "Q2 (source on SOLAR_OUT)")
- Power flows panel -> P -> FET switch -> SOLAR_OUT -> Heltec solar input,
  which charges the 3.7 V LiPo. The gate draws from the panel side only.
- The 10k B3950 NTC is on leads and must be thermally coupled to the battery
  cell (design doc BOM: "10k B3950 NTC on leads").

## Task

One deliverable: the same schematic, made buildable and readable, in
`hardware/solar-gate-v2/`, on a new branch based on
`hermes/0074-solar-gate-v2-wires`:

1. Add two real 2-pin connector symbols (standard library, e.g.
   `Connector:Conn_02x2_OddEven` or equivalent — pick a common stocked
   footprint family, note the choice in the README):
   - **J1 "SOLAR IN"**: pin 1 -> `P`, pin 2 -> `GND`. Text label:
     "SOLAR PANEL IN (+/-) — 13 W, 5 V panel".
   - **J2 "TO HELTEC"**: pin 1 -> `SOLAR_OUT`, pin 2 -> `GND`. Text label:
     "TO HELTEC LoRa 32 V4 SOLAR INPUT — gated charge output".
2. Add a text note next to the NTC: "NTC on leads — thermally couple to the
   battery cell".
3. Add functional block text annotations so the sheet reads at a glance:
   cold comparator, hot comparator, fault-OR (DA/DB), Vref (U2/TL431),
   gate drive (Q3), pass switch (Q1A/Q1B/Q2A/Q2B back-to-back).
4. Re-export SVG + PNG (high-res, as in 0074) + PDF via `kicad-cli sch
   export`, committed on the branch.

Do NOT change any component value, reference, net name, or wire. Do NOT
touch `hardware/solar-gate-v1/` or anything outside `hardware/solar-gate-v2/`.
No PCB layout.

## Success criteria

- J1 and J2 present as connector symbols, wired to the nets above (verify
  with grep on the .kicad_sch: J1 pins on nets `P`/`GND`, J2 on
  `SOLAR_OUT`/`GND`).
- The four text labels/notes above are present on the sheet.
- Netlist unchanged apart from the two added connectors: still 14 nets, same
  membership for the original 27 components (paste the net list).
- ERC: 0 errors (list warnings).
- SVG, PNG, PDF re-exported and committed.
- Branch `hermes/0075-solar-gate-v2-io-labels` pushed to origin.

## Constraints

- Do NOT modify `hardware/solar-gate-v1/` or any other existing project.
- No PCB layout. Schematic only.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- Branch name and SHA on origin.
- grep output showing J1/J2 references and their net connections.
- Net list (14 nets) showing unchanged membership.
- ERC output (0 errors; warnings listed).
- PNG/PDF/SVG paths on the branch.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field listing ERC
warnings and the connector footprint family chosen.
