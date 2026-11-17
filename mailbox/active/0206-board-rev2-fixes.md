---
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-17T15:45:30+00:00"
lease_until: "2026-11-17T17:15:30+00:00"
---
# 0206 — Temp-cutoff board: solid J1/J2 grounds, input cap, REV 2 marks, U2 label

## Assignment
- **Hermes:** all of it — schematic add, layout, re-exports, PR update. KiCad bench work is yours; no Newton build needed.
- **Copilot:** peer review on the new PR #5 head (GitHub-bound work, per standing rule).
- **Juno:** independent verification against the criteria below before archive.

## Context
The Mark 1 board (branch `hermes/0199-temp-cutoff-pcb`, PR #5, current head `f4f6edb` — the 0205 rev) went through a second outside review. Four items came back; Stephen has decided all four (2026-11-17). This task is the last layout pass before the fab order. The schematic is no longer a fixed input — item 2 adds one component to it. Everything else about the circuit stays exactly as it is.

Stephen's decisions, verbatim sense:
1. J1/J2 ground pads to solid pour connection (reviewer: the full 2.6 A return passes through their thermal spokes).
2. Add the input cap at J1 (reviewer: nothing on the panel input since C1 moved). Stephen stocks the part.
3. Keep the FQP27P06s — no FET change in this task. (Prototype board; Stephen load-tests the fabbed board later. Not your concern here.)
4. Bump the revision marks to REV 2 so fabbed boards can be told apart, and fix the U2 label position.

## Task

### 1. Solid ground connection on J1.2 and J2.2 only
Set the zone connection of pad 2 on J1 and pad 2 on J2 to **solid**. Every other GND pad keeps thermal relief. Refill zones. Nothing else about the pours changes (clearance 0.3, gap 0.3, spokes 0.5 stay as the zone defaults).

### 2. New input capacitor at J1 (schematic change)
Add **C3: 47 µF, ≥16 V, radial through-hole electrolytic**, + terminal to net P, − terminal to net GND, in the schematic and on the board.
- Footprint: standard KiCad radial, `Capacitor_THT:C_Radial_D5.0mm_P2.00mm` — if the part Stephen stocks needs a different can, say so in the reply with the can size you assumed instead; don't silently pick an exotic footprint.
- Placement: near J1, in the area C1 vacated (around x 30–40, y 40–48), clear of the FET load corridor along the top and clear of U1. Its GND pad takes thermal relief like the other component pads (only J1.2/J2.2 are solid, per item 1).
- Silkscreen: a "+" mark beside the + pad.
- This is the ONLY circuit change. New net memberships, exactly two: /P gains C3's + pad, /GND gains C3's − pad. No new nets, no other membership changes.

### 3. Revision marks to REV 2
- Board silkscreen text "MARK 1 REV 1" → "MARK 1 REV 2".
- Schematic title block revision → Rev 2. (The title block is the only schematic edit besides C3.)
- Re-render renders/schematic.png as well this time — the title changed.

### 4. U2 reference designator
Move the "U2" ref text from (83.27, 51.0) up beside the U2 footprint (part pads at y=44; ref should sit within ~3 mm of the footprint outline, unambiguously U2's, not drifting toward Q5). No overlap with any other visible text or pad.

### 5. Re-exports
Gerbers + drill + fab/bom.csv (gains the C3 line) re-exported from the finished board; renders board-top.png and board-bottom.png re-rendered; schematic.png re-rendered. Push to the same branch; PR #5 updates. Do NOT merge. Nothing is ordered.

## Success criteria
- ERC 0 errors / 0 warnings.
- DRC: only the two known [lib_footprint_mismatch] warnings (J2, TH1); 0 unconnected pads; the same 4 MH extra-footprint notes.
- Netlist vs f4f6edb: identical except /P and /GND each gain exactly one member (C3). State the comparison in the reply.
- J1.2 and J2.2 read back from the committed file as solid zone connection; all other GND pads still thermal.
- Board-top render shows: "MARK 1 REV 2", the "+" silk by C3, U2's ref beside U2, and every off-board label still adjacent to its own pads with no visible text overlaps.
- Copilot peer review posted on PR #5 for the new head.

## Constraints
- No other circuit, placement, or routing changes. If the cap's placement forces any local move, name it in the reply.
- Cost: cronrunner profile's usual model; no paid extras.
- Secrets: none needed; nothing to stage.

## Proof
Branch + head SHA, PR #5 link, Copilot review comment link, the DRC report committed at hardware/solar-gate-v2/drc-report.rpt, and the re-exported fab set — all on origin, verifiable without your local machine.

## Reply format
Staged reply in mailbox/staged/ per protocol 1.3.0: what changed (with the C3 footprint assumption stated), the netlist comparison result, ERC/DRC numbers, and the Copilot verdict. expect-reply-within: 24h.
