## Build spec 0002: Hermes capability interview

Author: Juno

Date: 2026-09-18

Status: open

## Objective

Stephen wants a clear, honest inventory of what Hermes can and cannot do in
this loop — before any real build tasks are written. KiCad is explicitly out
of scope for this interview (Stephen may teach it separately later).

## Requirements

Answer each of the following in the outbox reply. Be concrete and honest;
"can't do that" is a perfectly good answer and saves everyone time.

1. **Tooling inventory.** Beyond PlatformIO CI builds and git (already
   confirmed in 0001), what can you actually drive on the local machine?
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

4. **Bench interface.** Your 0001 reply says no USB devices and no connected
   hardware in your environment. Confirm: is Stephen your hands and eyes for
   all physical testing? Can you interpret artifacts he provides — scope
   screenshots, multimeter readings, board photos, serial logs?

5. **Spec format.** What do you want in a Juno spec? Where is the line
   between enough detail and noise? Call out anything in the 0001 spec or
   the templates that was unhelpful or missing.

6. **Hard limits.** What should Juno never ask you to do?

## Deliverables

- Outbox reply at `handoff/outbox/0002-hermes-capability-interview.md`,
  opening with a `Status:` line per the handoff README.

## Constraints

- KiCad is out of scope for this interview. Do not list it as a capability
  or a gap here; it is being handled separately.
- No spending, no pushes to `main` outside `handoff/` without Stephen's say.

## Definition of done

- Outbox reply exists with `Status: done`.
- Reply contains an explicit can-do / can't-do / needs-Stephen inventory
  covering all six questions above.
