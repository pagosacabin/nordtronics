---
task_id: "0143"
protocol_version: 1.0.0
status: filed
iteration: 1
expect-reply-within: 6h
---

# 0143 — Guition: flip invert_colors, reflash, verify against panel photo

## Context
0141's staged reply proved the palette complaint does not reproduce in any
artifact: the renders are canonical orange (#FF9900), but Stephen's panel
photos show blue/lime. The mechanism is `invert_colors: true` in the display
block → the driver sends INVON (0x21) → every color displays as its complement.
ESPHome's own JC4827W543 preset sets it true; the vendor's factory image drives
the panel with INVOFF (no 0x20/0x21 record in the init table). Stephen has
approved the flip experiment.

Stephen's photo of the current screen is committed alongside this task at
`mailbox/files/0143-panel-photo.jpg` — it shows the 0140 build on the glass:
blue where the render has orange, lime where the render has purple. His verdict:
"things are wonky." The photo also shows layout crowding (SOC pill cramped
against the frame, label pills dwarfed by the values, a wide bottom bar not
earning its space) — BUT the photo is the 0140 build and 0141's lambda revision
may already have changed the layout. Judge layout against the NEW build, not
this photo; this photo's job is color comparison.

## Task
1. Compare `mailbox/files/0143-panel-photo.jpg` against your
   `preview-bms-lcars.png` render. If the photo's colors are the complement of
   the render's (blue↔orange, lime↔purple), the inversion is confirmed.
2. Set `invert_colors: false` in the display block. One line, nothing else.
3. Rebuild and OTA-flash the panel (192.168.1.72, as with 0140/0141).
4. Report the new "compiled on" timestamp so Stephen knows which build is on
   the glass.

## Constraints
- The ONLY config change is the `invert_colors` line. Touch nothing else.
- If the photo does NOT show complement colors (inversion not confirmed):
  do NOT flip. Report what you see instead.
- Same push path as 0140/0141: SSH from the bench host to pagosacabin/cabin-play
  (the gh.py token is nordtronics-scoped and 403s on cabin-play writes).
- Keep model/provider cost economical; no premium tier needed.

## Success criteria
1. Photo-vs-render comparison recorded (inversion confirmed or not).
2. If confirmed: `invert_colors: false` committed, panel OTA-flashed, new build
   timestamp reported.
3. Stephen photographs the panel; the photo shows the canonical orange palette.

## Proof
- Commit SHA on cabin-play origin.
- The comparison note (which regions are complements).
- The new "compiled on" timestamp read back from the panel.

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the comparison
result, the new build timestamp, and notes on anything unexpected.
