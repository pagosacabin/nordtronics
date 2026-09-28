---
task_id: "0074"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
---

# 0074 — Redraw solar-gate-v2 schematic WITH wires (rework of 0073)

## Context

Task 0073 delivered a KiCad schematic of the solar-gate-v2 temperature-gate
circuit on branch `hermes/0073-solar-gate-v2-schematic`. It verified clean
(branch SHA matched, ERC 0 errors, netlist validated at 14 nets). But the
capture was drawn with **net labels only — zero `(wire)` entries** in
`solar-gate-v2.kicad_sch` (68 labels, 0 wires, 0 buses, confirmed by grep).
Electrically valid, but not human-readable: Stephen cannot trace signal flow
on a schematic with no connection lines, and he needs to read it for design
review and the bench build.

## Task

One deliverable: a redrawn, fully WIRED KiCad schematic of the same circuit,
in `hardware/solar-gate-v2/`, on a new branch.

Base the new branch on `hermes/0073-solar-gate-v2-schematic` (not main — the
project files only exist there). Redraw `solar-gate-v2.kicad_sch` so that
every connection currently made by a net label is drawn as a real wire.
Keep every component, reference designator, value, and net name identical to
the 0073 capture. Net labels may remain as supplements, but no connection may
rely on labels alone — the schematic must be traceable by eye.

Also export for phone viewing (lesson from 0073): commit a PNG render
(`hardware/solar-gate-v2/solar-gate-v2-schematic.png`, A4 landscape, high
resolution) and a PDF (`hardware/solar-gate-v2/solar-gate-v2-schematic.pdf`)
alongside the SVG. Use `kicad-cli sch export` for both.

Do NOT touch `hardware/solar-gate-v1/` or anything outside
`hardware/solar-gate-v2/`.

## Success criteria

- `solar-gate-v2.kicad_sch` contains real `(wire)` entries connecting the
  circuit; a grep for `(wire ` returns non-zero.
- Netlist identical to the 0073 capture: 14 nets, same membership
  (run the same `generate_netlist` check and paste the net list).
- ERC passes with zero errors (warnings listed and explained).
- All 27 references (Rbias, R1, R2, Rfa, Rpua, Rb1, Rb2, Rfb, Rpub, Rpuc,
  Rb, Rt, Rg, C1, C2, DA, DB, D1, D2, NTC, Q1A, Q1B, Q2A, Q2B, Q3, U1, U2)
  with the exact values from the 0073 spec.
- PNG and PDF renders committed on the branch.
- Committed on a branch named `hermes/0074-solar-gate-v2-wires`, pushed to
  origin.

## Constraints

- Do NOT modify `hardware/solar-gate-v1/` or any other existing project.
- Do not start PCB layout. Schematic only.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- Branch name and SHA on origin.
- `grep -c '(wire ' hardware/solar-gate-v2/solar-gate-v2.kicad_sch` output.
- Net list from `generate_netlist` (all 14 nets with membership).
- ERC output pasted (or report path on the branch).
- PNG and PDF paths on the branch.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field listing any ERC
warnings.
