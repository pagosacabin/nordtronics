---
task_id: "0073"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
---

# 0073 — KiCad schematic capture: solar gate v2 (temperature-gated charging switch)

## Context

Stephen selected the winning design from a four-way AI design review (ChatGPT,
Grok, Claude, Copilot). The winner is the Claude clean-sheet design below.
A previous KiCad schematic exists at `hardware/solar-gate-v1/` (task 0039 did
the first capture); this is a new topology, so it goes in a NEW project
`hardware/solar-gate-v2/`. Follow the v1 project's library conventions
(sym-lib-table entries using `${KIPRJMOD}`, per the 0043 fix).
Model: deepseek-flash. Report the model used in your reply.

## Task

One deliverable: a complete, ERC-clean KiCad schematic of the circuit
specified below, in `hardware/solar-gate-v2/`, pushed to a branch.
No PCB layout — layout waits for bench validation of the trip points.

## The circuit (Claude design, Juno-verified)

Nets: P (panel +5V in), SOLAR_OUT, VREF (2.5V), N (sense node), PA (cold ref),
Vb (hot ref), OA (cold comp output), OB (hot comp output), C (fault-OR node),
G (all FET gates), H (gate max-selector node), GND.

Reference: Rbias 1.6k P to VREF. U2 TL431: K=VREF, A=GND, REF tied to K.
C2 0.1uF VREF to GND at U2.
U1 LM393: VCC=P, GND=GND. C1 0.1uF P to GND at U1 pin 8. DRAW the power
pins and both caps — no "not shown" notes.

Sense: Rt 34.0k VREF to N. NTC 10k B3950 N to GND (2-wire, thermally on the
cell). N falls as temperature rises. Open NTC pulls N to VREF = extreme
cold = charging OFF (fail safe).

Cold comparator U1A: N on (-), PA on (+), output OA.
R1 100k VREF to PA. R2 95.3k PA to GND. Rfa 1.5M OA to PA (positive
feedback, OUT to the + input). Rpua 100k VREF to OA (open-drain pull-up).

Hot comparator U1B: N on (+), Vb on (-), output OB.
Rb1 200k / Rb2 23.2k divider VREF to Vb to GND (Vb = 0.260V).
Rfb 1.5M OB to N (positive feedback to the + input).
Rpub 100k VREF to OB (open-drain pull-up).

Fault OR: Rpuc 100k VREF to C. DA, DB BAT54: anode to C, cathode to OA / OB
respectively. Rb 10k C to Q3 base.
Either comparator output low pulls C down and turns Q3 off.

Gate drive: Q3 2N3904, collector to G, emitter to GND, base via Rb.
Q1a and Q1b AO3401 in parallel: source to P.
Q2a and Q2b AO3401 in parallel: drain to middle node M, source to SOLAR_OUT.
All four gates tied to G. Q1 and Q2 are back to back: no body diode conducts
in either direction when off. This is deliberate — do not "simplify" it to
a single FET.
D1 BAT54: anode P, cathode H. D2 BAT54: anode SOLAR_OUT, cathode H.
Rg 100k H to G. G rests at max(P, SOLAR_OUT) minus a diode drop = FETs OFF.
Q3 on pulls G near GND = FETs ON (Vgs approx -4.8V).

Trip points (for your own cross-check, not a substitute for ERC):
cold cutoff -0.05 C, re-enable +2.3 C; hot trip 48.05 C, re-arm 47.45 C.

## Success criteria

- Every component and net above appears in `hardware/solar-gate-v2/`
  with the exact values shown. E96 values (95.3k, 23.2k, 34.0k) are standard
  stock — use them as specified.
- ERC passes with zero errors (warnings listed and explained in the reply).
- Committed on a branch named `hermes/0073-solar-gate-v2-schematic`,
  pushed to origin.

## Constraints

- Do NOT modify `hardware/solar-gate-v1/` or any other existing project.
- Do not start PCB layout. Schematic only.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- Branch name and SHA on origin.
- ERC output pasted (or screenshot path on the branch).
- The `.kicad_sch` file path on the branch.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field listing any ERC
warnings and any footprint assignments you made (or deliberately deferred).
