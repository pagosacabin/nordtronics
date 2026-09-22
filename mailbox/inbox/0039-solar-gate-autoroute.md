---
task_id: "0039"
status: inbox
---

# 0039 — Solar gate v1: schematic + first autoroute shakedown

## Context

This is a cold test of the full EDA rung: schematic → layout → autoroute
on a real circuit, through MCP tools only. The solar cold-charge gate
bench test has NOT run yet — this board is a TEST vehicle for the flow,
not a production board, and nothing here goes to fab regardless.

Reference circuit (from the approved build spec):

- Q1: AO3401 P-ch MOSFET, SOT-23 (pins: 1=gate, 2=source, 3=drain)
- U1: TLV3401 nanopower comparator, open-drain, SC-70-5
- D1: TLV431 shunt reference 2.5V, SOT-23
- TH1: 10k NTC thermistor (B=3950), wired OFF-BOARD — use a 2-pin
  connector footprint (SH1.25 or pin header) on the board
- R1 30k, R2 30k, R3 30k, Rf 270k, Rg 100k, Rs 12k
- Telemetry divider: 100k / 100k from GATE node to GND, tap to GPIO
- C1 100nF across D1 (REF2V5 to GND); C2 10nF from VA node to GND
- Connectors: Panel+ in, GND, CHG+ out (switched solar+ to charger)

Nets:

- Panel+: panel input; feeds Q1 source, Rg top, Rs top, U1 VCC
- GND: panel-, D1 anode, TH1 bottom, R3 bottom, C1 bottom, C2 bottom,
  U1 GND, telemetry divider bottom
- REF2V5: Rs/D1-cathode/R1-top/R2-top junction (= 2.5V)
- VA: R1-bottom / TH1-top / C2-top / U1 IN+ / Rf-top
- VTH: R2-bottom / R3-top / U1 IN− (= 1.25V)
- GATE: U1 OUT / Q1 gate / Rg-bottom / Rf-bottom / telemetry 100k-top
- CHG+: Q1 drain → charger solar input+
- TELEM: telemetry divider tap → MCU GPIO

Logic: warm → NTC low → VA < 1.25V → OUT pulls GATE low → Q1 on.
Cold/no-sun → GATE floats to Panel+ via Rg → Q1 off.

## Task

1. Check the autoroute prerequisites on your machine: `java -version`
   and whether the Freerouting JAR the server expects is present. If
   either is missing, install/set it up (Java + download the Freerouting
   JAR) and report what you did. If you cannot get it running, stop and
   report the exact blockage — do not skip to hand-routing.
2. Verify the pinouts of Q1, U1, D1 against their datasheets before
   drawing anything. Report any correction you had to make.
3. Through MCP tools only, create project `solar-gate-v1` and draw the
   schematic exactly per the nets above. No extra parts, no substitutions
   without reporting them.
4. Run ERC. Fix every violation; report the final ERC output verbatim.
5. Create the board: 2-layer, ~40x30mm (your choice, report it), all
   footprints placed sensibly — Q1 near the panel/charger connectors,
   TH1 connector at a board edge. Passives 0805 or 0603, your choice,
   report it. The Panel+ → Q1 → CHG+ path carries up to 1A: size those
   traces for the current (state the widths you used) — Stephen will
   verify them in review.
6. Run the autoroute through the server's freerouting bridge. Report the
   router's completion stats (routed %, vias, unrouted nets).
7. Run DRC. Report the full DRC output verbatim — including violations
   if any. Do NOT hand-fix routing to hide router failures; report what
   the router actually produced.

## Success criteria

1. Schematic matches the netlist above; ERC 0 errors.
2. Board exists with every footprint placed; the autorouter ran to
   completion (or failed with a reported exact error).
3. DRC was run and its full output is in the reply.
4. Branch `hermes/0039-solar-gate-route` is on origin at the SHA you cite.

## Constraints

- TEST BOARD ONLY. No Gerbers, no fab outputs, no ordering, no JLCPCB.
- No power-board layout work beyond this task — high-current boards
  stay with Stephen.
- Stephen reviews the schematic and layout before anything real is built
  from it — say so in your reply; do not present it as production-ready.
- If the autorouter cannot run, the deliverable is the exact error, not
  a hand-routed board.

## Proof

- Branch `hermes/0039-solar-gate-route` on origin, full tip SHA quoted.
- ERC output verbatim, DRC output verbatim, autorouter stats.
- Pinout verifications (what the datasheets said).

## Reply format

Move this file to `mailbox/staged/` with `status: staged` (the staged file IS
the reply). Include:

- Status: done | blocked
- Branch + full SHA
- ERC output, DRC output, autorouter stats (verbatim)
- Any pinout corrections or part substitutions
- The exact blocking error, if blocked

expect-reply-within: 6h
