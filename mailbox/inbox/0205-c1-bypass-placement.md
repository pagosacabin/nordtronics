# 0205 — Temp-cutoff board: C1 move, silkscreen label fixes, GND pour

## Assignment
- **Hermes** (KiCad on your machine): the placement change below, on the existing 0199 branch.
- **Copilot:** peer review on PR #5's new head, per the standing rule for GitHub-bound work (route via Herdr/`hc`, keyring OAuth — do NOT source `gh-env.sh`).
- **Juno:** verifies the new head from origin; archives 0199 and this task together once green.
- **Stephen:** nothing owed — he directed this change (2026-11-17 02:51 MST) after reviewing the board renders.

expect-reply-within: 24h

## Context
Task 0199 delivered the temp-cutoff board (Mark 1 Rev 1) on branch `hermes/0199-temp-cutoff-pcb`, PR #5, head `7683739eb181f9b1506c888ed4f76b85ca2d1b6b`, staged and Copilot-SHIPped. Reviewing the renders, Stephen spotted a placement weakness: **C1 — the bypass capacitor across U1's supply (C1.1 on net `P` with U1 pin 8/VCC, C1.2 on `GND` with U1 pin 4) — sits roughly 13 mm from U1 and not at the pin-8/pin-4 end of the DIP-8 package.** A bypass cap earns its keep in proportion to how short its loop to the IC's power pins is; on a comparator switching a gate node, a long loop invites supply bounce and chatter at the trip point. The board is ~27% full, so space is not a constraint.

Also for the record, so it stops travelling as an open item: **the LM393 pin 1↔pin 5 question from 0199 is RESOLVED by Stephen (2026-11-17).** The tie was a jumper on his bench build from his own misread while wiring the breadboard; he removed it. It was never in the schematic — "not a schematic issue." The capture as delivered is correct.

## Task
1. **Move C1** so it sits tight against the end of U1's package where pins 8 and 4 are: pads within a few millimetres of that package end, shortest practical copper loop from C1.1 to the pin-8 net and from C1.2 to ground. Placement and local routing only — C1's value, footprint, and net connections do not change, and no other component moves unless a clearance forces it (name any such move in the reply).
2. **Record the pin 1↔5 resolution** in the board README's open-item section: one or two lines stating Stephen's resolution (bench jumper from his own misread, removed; never a schematic feature; capture correct as delivered). No other doc changes.
3. **Re-verify:** ERC (expect 0 errors / 0 warnings) and DRC (expect 0 unconnected, 0 footprint errors, only the two known `[lib_footprint_mismatch]` warnings on J2/TH1). Export the netlist and confirm it is identical to the 7683739 netlist — same 14 nets, same memberships.
4. **Re-export the fab set** from the updated board (Gerbers + drill under `fab/`, same discipline as 0199: re-plot and confirm the committed set matches the board as delivered) and **re-render** `renders/board-top.png` and `renders/board-bottom.png` at the same dimensions. The schematic is unchanged — do not re-render `renders/schematic.png`.
5. **Push to the same branch** `hermes/0199-temp-cutoff-pcb` so PR #5 updates, and route the new head through Copilot peer review; address findings or rebut them in the reply.

## Success criteria (falsifiable)
- New head SHA on `hermes/0199-temp-cutoff-pcb`; PR #5 open on that SHA.
- C1's new position stated in the reply (distance from U1's package end or board coordinates), visibly adjacent to U1 in the re-rendered top view.
- ERC quoted at 0/0; DRC quoted with 0 unconnected and only the two known J2/TH1 warnings.
- Netlist stated identical to the 7683739 netlist (14 nets, same memberships) — the circuit did not change.
- Copilot verdict on the new head, linked from the reply.

## Constraints
- No circuit changes: schematic, nets, values, and footprints are fixed input — placement of C1 (and its local routing) is the only layout change.
- No parts ordered, nothing purchased.
- Do not merge PR #5 — Juno verifies and archives.
- Do not source `gh-env.sh` (PR work uses the keyring OAuth login).

## Proof
New branch head SHA on origin, PR #5 updated, ERC/DRC output quoted, Copilot review attached. Juno verifies from origin; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: branch + head SHA at the top, C1's new position, ERC/DRC summary, netlist-parity statement, Copilot verdict, cost block.

## Task addition (2026-11-17 ~03:01 MST) — silkscreen label fixes

From Stephen's review of the board renders; positions confirmed by Juno against the board file. On a board whose off-board connections are solder pads, the silkscreen IS the wiring instruction — these are functional fixes, not cosmetics.

1. **"SOLAR_OUT +5V" label** (board text, currently at (84.0, 47.0)) floats midway between J2 (pads: (99, 42) SOLAR_OUT, (99, 34.8) GND) and TH1 (pads: (99, 58) N, (99, 50.8) GND) — ~15.5 mm from TH1's nearest pad, ~15.8 mm from J2's. It reads as belonging to the wrong connector. Move it to sit immediately beside J2's pads.
2. **"TH1" reference text** (at (95.8, 54.4)) overlaps/crowds the **"NTC CELL"** board text (at (86.0, 56.0)) beside TH1's pads. Separate them so both read cleanly; "NTC CELL" stays by TH1's pads (it names the NTC connection), TH1's reference moves clear of it.
3. **Input "GND" label** (board text at (21.5, 37.0)) sits 18.0 mm from J1's GND pad (39.2, 34.0) — and closer to J1's P pad (10.9 mm) than to the pad it names. Move it immediately beside J1 pad 2. (The second "GND" text at (96.5, 30.0) serves J2's GND pad at ~5 mm; leave it, or nudge for consistency — your call, state which in the reply.)

Success criteria for this addition: in the re-rendered `renders/board-top.png`, every off-board pad's identifying label sits adjacent to its own pads, and no silkscreen text overlaps another text or a pad. Stephen re-checks the render visually before fab.

## Task addition 2 (2026-11-17 ~03:05 MST) — GND return path: add copper pours

**Finding (outside Gerber review, verified by Juno against the board file):** GND is routed entirely in 0.2 mm traces (~155 mm of it across both layers, no pour anywhere). But both off-board negatives land on this board — panel in at J1 pad 2 (39.2, 34.0), load out at J2 pad 2 (99, 34.8) — so the full load return current crosses the board's GND between those pads, up to the 3 A design ceiling. 0.2 mm on 1 oz copper is a ~0.5–0.7 A class trace; the routed path is roughly 0.15 Ω, i.e. ~0.4 V lost and ~1 W of heat at 2.6 A. The 0199 spec sized P/M/SOLAR_OUT for 3 A and never gave GND the same treatment — this corrects that.

**Fix:** add a GND copper pour (zone) on **both** copper layers, stitched together with several vias — at minimum near J1, J2, and U1 — so the J1.2 ↔ J2.2 return path is pour copper end to end. Keep the C1 relocation's short loop to U1's pins working over the pour. Zone clearances follow the board's existing design rules.

**Success criteria for this addition:**
- Board file carries GND zones on F.Cu and B.Cu, filled; stitching vias present (reply states count and locations).
- Netlist still identical to the 7683739 netlist (14 nets, same memberships) — the pour changes copper, not connectivity.
- ERC 0/0 and DRC 0 unconnected / only the two known J2/TH1 warnings, re-quoted after the change.
- Re-exported Gerbers show the pours on both layers (Stephen's reviewer will re-check GND connectivity from the new zip).
