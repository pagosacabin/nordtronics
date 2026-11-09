---
task_id: "0173"
protocol_version: 1.2.0
status: inbox
iteration: 0
expect-reply-within: 6h
proof: []
---

# 0173 — Proofer J7239 wiring diagram (KiCad, handoff sheet for Black Bart's)

## Assignment

- Hermes: draw the KiCad schematic, export the PDF, push to the branch.
- Newton: not involved — KiCad schematic capture is Hermes's proven rung; no software step.

## Context

Alliance Manufacturing proofer, model J7239, serial 50539A9603, 120 VAC, 1500 W, 13.5 A, at Black Bart's in Pagosa Springs. Stephen rebuilt the wiring box on the bench on 2026-11-09 and bench-proved it the same day: the wires were tangled together, corroded and broken, the terminals corroded beyond use, and the original main switch was bad. Live test passed: element ~10 Ω, at 200°F setpoint the element pulls current, turned down below 60°F it cuts off, fan spins. Remaining work is buttoning up the wiring box and reinstalling at Black Bart's.

Purpose of this drawing: a one-page sheet that travels WITH the proofer, so the crew at Black Bart's can rewire it themselves if it ever fails again. The audience is not dumb, but they are not electricians — the sheet must be readable by a non-electrician. Plain-word labels, not symbols alone.

Bench-proven wiring (Stephen, 2026-11-09 — this is the whole circuit, nothing else):

- Wall power 120 VAC in: hot (L) goes to the main switch; neutral (N) goes to the terminal block.
- Main switch: 16 A rated replacement (the original switch failed). A 10 A switch is underrated for the 13.5 A draw and must NOT be substituted.
- Switch output splits two ways:
  - Fan motor → neutral bus. The fan runs whenever the switch is on.
  - Thermostat → heating element (~10 Ω) → neutral bus. The thermostat cycles the heating element only; it does not control the fan.
- Terminal block: 4 terminals, commoned into one neutral bus. Three neutrals land on it: wall neutral in, fan neutral, heating-element neutral. One terminal is spare. (Bare-block continuity across all four terminals after sanding confirms the block is commoned.)

## Task

Draw this circuit in KiCad as one schematic and export a one-page PDF, both pushed to branch `hermes/0173-proofer-wiring` under `docs/proofer-j7239/`:

- `docs/proofer-j7239/j7239-wiring.kicad_sch`
- `docs/proofer-j7239/j7239-wiring.pdf` (one page, printable, black-and-white friendly)

## Success criteria

1. The schematic matches the bench-proven wiring above exactly: switch, fan, thermostat, heating element, neutral-bus terminal block with three neutrals and one spare.
2. Every part is labeled in plain words on the sheet (MAIN SWITCH, FAN, THERMOSTAT, HEATING ELEMENT, TERMINAL BLOCK — NEUTRAL BUS), alongside the schematic symbols, so a non-electrician can follow it.
3. The sheet carries a short "How it works" note in plain language: fan runs whenever the switch is on; thermostat turns the heating element on and off only; at 200°F the element heats, below 60°F it cuts off.
4. The sheet carries a "Replacing parts" note: main switch must be 16 A or higher (never 10 A); heating element ~10 Ω (1500 W at 120 V).
5. The PDF is one page and prints cleanly in black and white.

## Constraints

- Push only to branch `hermes/0173-proofer-wiring`. Do NOT merge to main.
- Draw it in KiCad. Do not substitute an AI-generated image diagram.
- Label every wire by function (hot / neutral / switched hot). Stephen did not report wire colors, so do not invent wire colors on the sheet.
- Do not add parts that are not in the circuit above. If any connection is unclear from this spec, say so in the staged reply notes instead of guessing.

## Proof

- branch: `hermes/0173-proofer-wiring`, tip SHA
- files: the two files above, present on the branch
- run: none expected (no workflow covers `docs/**`); confirm live with `gh run list` and state the result.

## Reply format

```
DONE — 0173 proofer wiring diagram
branch: hermes/0173-proofer-wiring @ <sha>
files:
  - docs/proofer-j7239/j7239-wiring.kicad_sch
  - docs/proofer-j7239/j7239-wiring.pdf
checks: <what you verified — PDF renders one page, labels present, wiring matches spec>
notes: <anything unclear, any assumption made, CI run-list result>
cost: <per protocol 1.1.0 cost format>
```
