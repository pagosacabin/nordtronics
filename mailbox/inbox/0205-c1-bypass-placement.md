# 0205 — Temp-cutoff board: move C1 tight to U1 (LM393) supply pins

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
