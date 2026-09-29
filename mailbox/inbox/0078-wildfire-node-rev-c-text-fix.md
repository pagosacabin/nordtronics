---
task_id: "0078"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 6h
---

# 0078 — Fix upside-down J1 description text on the Rev C schematic

## Context

0077 (re-layout) is verified and archived. Juno's visual verification of the
new render found one cosmetic defect: J1's description field,
`BATT IN (JST-PH 2.0mm 2-pin PROPOSED)`, renders rotated 180 degrees —
upside down — at the top-left of the sheet (above J1/F1, near the RXEF075
value string). A second nit in the same corner: the `TP1` and `F1` refdes
labels touch each other.

Front-load the exact state: branch `hermes/0077-wildfire-node-rev-c-relayout` @
`5b629753c0cadcd0dbb44543f17924d11add4700`, files under
`hardware/wildfire-node-v1/`. The 0077 netlist (19 multi-pin nets / 77
connected pins / 7 NC singletons) is frozen and does not change in this task.

## Task

On a new branch `hermes/0078-wildfire-node-rev-c-text-fix`, branched off
`hermes/0077-wildfire-node-rev-c-relayout`:

1. Fix J1's description field angle so the text reads right-side up
   (horizontal, 0 degrees as seen on the sheet — not 180).
2. Nudge the `TP1` and `F1` refdes labels apart so their glyph boxes no longer
   touch.
3. Re-export the vector PDF and the >= 200 dpi PNG from the final
   `.kicad_sch`, re-run ERC, and commit all of them.

Text and nudge only — no component, value, pin, or net changes.

## Success criteria

- [ ] `BATT IN (JST-PH 2.0mm 2-pin PROPOSED)` reads right-side up in the
      exported render (verify by opening the PNG and looking, not by
      asserting the angle value).
- [ ] `TP1` and `F1` labels no longer touch.
- [ ] Netlist re-verified: `kicad-cli` netlist export compared against the
      0077 netlist — 19 multi-pin nets / 77 connected pins / 7 NC singletons,
      identical. Any difference is a failure.
- [ ] ERC re-run: 0 errors expected; the title-block ERC line matches the new
      report.
- [ ] Fresh vector PDF and fresh >= 200 dpi PNG committed.
- [ ] Branch `hermes/0078-wildfire-node-rev-c-text-fix` pushed to origin; SHA
      reported.

## Constraints

- Text placement and field angles only. The 0077 netlist is frozen.
- Worker model is deepseek-flash (Stephen's call).
- Push only to the task branch. No secrets in the repo, ever.

## Proof

- Branch `hermes/0078-wildfire-node-rev-c-text-fix` on origin + SHA, verified
  with `git ls-remote`.
- Netlist comparison PASS line (19 nets / 77 pins / 7 NC singletons,
  identical to 0077).
- ERC summary line; PNG dimensions.

## Reply format

Stage your reply as `mailbox/staged/0078-wildfire-node-rev-c-text-fix.md`
with front matter carrying `task_id: "0078"`, `status: staged`, `iteration`,
and a `proof:` block (branch, sha, files, renders, erc, netlist check,
model), then a Reply section.
