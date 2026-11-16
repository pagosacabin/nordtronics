# 0199 — Temp-cutoff board: KiCad schematic capture + PCB layout

## Assignment
- **Hermes** (KiCad MCP on your machine): schematic capture + layout, driven per the gates below.
- **Copilot:** peer review on the PR, per the standing rule for GitHub-bound work (route via Herdr/`hc`, keyring OAuth — do NOT source `gh-env.sh`).
- **Juno:** verifies the pushed branch/PR, DRC output, and exports from origin; archives on green.
- **Stephen:** answers the open-questions gate (below) before layout starts; his as-tested bench build is the source of truth for the circuit.

## Context
The solar-gate temperature-cutoff circuit: Stephen's bench build, BAT85 diodes at DA/DB/D1/D2 (installed 2026-10-30), four FQP27P06 P-FETs in two paralleled pairs (Q1 pair, Q2 pair), nets per the 2026-10-24 beep-out checklist (all pin 2s common at M; all pin 1s common at G; Q1 pin 3s → P; Q2 pin 3s → SOLAR_OUT; P must never beep to SOLAR_OUT).

**PROVEN AS-BUILT — Stephen's confirmation, 2026-11-16:** beep-out clean per the checklist; powered test passed — gate on at mid-band (5.00 V in → 4.95 V out), and both trips switch under load (LED load, on video) with fixed resistors substituted for the NTC (~3K hot side cuts, >33K cold side cuts); the NTC itself verified standalone at 11.2K at room temperature (a healthy 10K part). Two build errors were found by the testing and fixed **to match the drawing**: (1) LM393 pin 2 (IN1−) had been tied into the pin 1↔pin 5 node — lifted; pin 5↔pin 1 stands as drawn; (2) the BAT85s were in backwards — corrected. **The as-built therefore equals the drawing.** Stephen's call closing the test (2026-11-16): the residual flakiness is the breadboard, not the design — he saw it work on both sides. Box-up and the dress rehearsal are deferred to the finished board; PCB work starts now.

## Open questions for Stephen (Hermes stops and reports if any is unanswered at pickup)
- Board outline / mounting: max dimensions, mounting holes, enclosure if any?
- Off-board connections: terminal blocks, JST, or solder pads — and for which nets (P, SOLAR_OUT, G, M, power)?
- Worst-case current through Q1/Q2 paths (sets trace width / copper weight)?
- Any parts already chosen for the board version (through-hole vs SMD for the BAT85s and MOSFETs)?

## Task
1. **Schematic capture** in KiCad from the as-built circuit (Stephen's confirmation is the source; if a schematic already exists in the repo, start from it and say which). Symbols/footprints via SnapMagic (SnapEDA) where the standard libraries lack them. Every net from the beep-out checklist must appear and match: M, G, P, SOLAR_OUT, DA/DB/D1/D2, Q1/Q2.
2. **PCB layout**: board per Stephen's answers above; sensible clearances for an outdoor-adjacent solar-gate install; silkscreen labels for every off-board connection.
3. **Verify**: ERC + DRC clean (quote the reports; any waived violation is named and justified, not silent). Export Gerbers + drill files.
4. **Deliver** in the nordtronics repo under `hardware/` (branch `hermes/0199-temp-cutoff-pcb`): KiCad project (schematic + PCB), exports, and a short README note (what the board does, the net map, the as-built source). PR to main; Copilot peer review on the PR; address its findings or rebut them in the reply.

## Success criteria (falsifiable)
- Schematic nets match the as-built checklist exactly (Hermes quotes the net list; Juno spot-checks against the checklist above).
- ERC/DRC reports quoted, clean or with named, justified waivers.
- Gerber + drill exports exist in the PR; Copilot review is attached to the PR; PR URL + head SHA in the reply.

## Constraints
- The as-built circuit is fixed input — if the schematic capture reveals an ambiguity or a likely bench error, stop and report it; do not "improve" the circuit in layout.
- No parts ordered, nothing ordered or purchased in this task.
- Do not source `gh-env.sh` (PR creation uses the keyring OAuth login).
- **Cost:** standard routing — KiCad work is Hermes's; Newton only if a scripted step genuinely needs him, stated in the reply. Report cost/quota as usual.

## Proof
PR on origin with head SHA, ERC/DRC output quoted, Copilot review attached. Juno verifies from origin; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: PR URL + head SHA at the top, net-list quote, ERC/DRC summary, Copilot verdict, open-question answers as received, cost block.
