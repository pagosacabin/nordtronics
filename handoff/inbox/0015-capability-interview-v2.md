# 0015 — Capability re-interview (new model)

## The goal in one sentence

Stephen replaced your base model since the first interview. Give a fresh,
honest inventory of what YOU can and cannot do in this loop. The old answers
are void — answer from your own actual capabilities, not from anything the
previous model claimed.

## Steps

1. Read this whole spec.
2. Answer the six questions below concretely and honestly. "Can't do that" is
   a perfectly good answer and saves everyone time.
3. Write the reply to `handoff/outbox/0015-capability-interview-v2.md` on main
   (commit + push the reply on main; no branch needed for a Q&A task).
   Write the reply last, after the answers are complete.

## Questions

1. **Tooling inventory.** Beyond PlatformIO CI builds and git, what can you
   actually drive on the local machine?
   - VS Code editing / refactoring?
   - Circuit simulation (ngspice, LTspice, other)?
   - 3D printer slicer (which one)?
   - Python scripting for data analysis, log parsing, test automation?
   - Web access for datasheets, stock/pricing checks?
   - Give versions where it matters.

2. **Design review.** Can you read a datasheet and do real design review —
   catch errors, check operating ranges, question component choices — or do
   you execute specs exactly as written? Where is the line?

3. **Firmware depth.** ESP32-S3 via PlatformIO is confirmed. What about:
   - ESP32 classic / other variants?
   - LoRa radio configuration (SX1262, SX1276)?
   - MQTT / Home Assistant integration work?
   - Deep sleep and power-management firmware?

4. **Bench interface.** Confirm: is Stephen your hands and eyes for all
   physical testing (no USB devices, no connected hardware in your
   environment)? Can you interpret artifacts he provides — scope screenshots,
   multimeter readings, board photos, serial logs?

5. **Spec format.** What do you want in a Juno spec? Where is the line
   between enough detail and noise? Call out anything in the recent task
   specs that was unhelpful or missing.

6. **Hard limits.** What should Juno never ask you to do?

## Constraints

- KiCad is out of scope for this interview. Do not list it as a capability
  or a gap here; it is being handled separately.
- No spending, no pushes to `main` outside `handoff/` without Stephen's say.

## How it will be checked

Juno reads the answers. No trick questions — but vague claims will get
follow-up tasks that test them, so be precise now.
