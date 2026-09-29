---
task_id: "0077"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 6h
---

# 0077 — Re-layout the Wildfire Node Rev C schematic (readability pass)

## Context

0076 delivered an electrically correct Rev C schematic: the netlist was verified
member-for-member (19 multi-pin nets / 77 connected pins / 7 NC singletons) and
ERC reports 0 errors. But the sheet layout is unusable — Stephen's verdict,
confirmed against the render: everything is smashed into the left third of the
A4 sheet, the right two-thirds sit empty, wires cross straight through the
notes text block, and labels overlap each other.

Front-load the exact state: branch `hermes/0076-wildfire-node-v1-rev-c` @
`68c6e2fcf1efc9335683b08c6273565e0ba35662`, files under
`hardware/wildfire-node-v1/`. The authoritative netlist is in the archived
`mailbox/archive/0076-kicad-wildfire-node-v1-rev-c.md` and does not change in
this task.

## Task

Re-layout the Rev C schematic for readability on one A4 landscape sheet.
Placement and routing only — same components, same pins, same nets, same
values. Commit the updated `.kicad_sch`, a fresh vector PDF, a fresh
phone-readable PNG (>= 200 dpi), a fresh ERC report, and the README on branch
`hermes/0077-wildfire-node-rev-c-relayout`, branched off
`hermes/0076-wildfire-node-v1-rev-c`.

Follow this placement map:

- Top-left zone: J1, F1, TP1, TP7 / GND star point.
- Top-center: U5 MiniBoost + C1/C2 + R2 (4.7 kΩ EN pull-down); TP2 / 5V2.
- Center, given room: U1 Heltec module — the hub; GPIO wires radiate outward
  to their neighbors.
- Right of Heltec: U2 PMS5003 + C3/C4 + R3 + TP6.
- Below Heltec: Q7/R21/R22 gate stage -> U3 AS3935 + C5/C6 + TP5.
- Bottom-left: U4 BME680 + C9/C10 + TP4 (Vext).
- A1 temp-gate block: top-left, adjacent to J1 and the JP3 SOLAR wire.
- Notes text: one dedicated block, bottom-left above the title block, with a
  clear keep-out — no wires and no components through it.
- Title block: bottom-right (KiCad default frame).

Layout rules:

- Use the full sheet width. If the right half ends up emptier than the left,
  rebalance — that is the failure being fixed.
- Minimum ~5 mm clear space between module boxes; no component body overlaps
  another component's pins or labels.
- No wire crosses a text note. No label overlaps another label or a wire (fix
  the 7 known overlaps from 0076 and find the rest by rendering and looking).
- Wires orthogonal only. Keep the 0076 rule: net labels only on `VBAT_F`,
  `5V2`, `3V3`, `Vext`, `GND`; every signal is a drawn wire.
- All schematic note texts from 0076 (PROPOSED connectors, schematic-only,
  still-open items, ERC line) must survive the move — drop none.

## Success criteria

- [ ] The PNG shows components spread across the whole A4 sheet; the right
      half is no longer empty.
- [ ] Zero wires cross the notes block; zero label overlaps (verified by
      rendering and looking, not by assertion).
- [ ] Netlist re-verified after the move: `kicad-cli` netlist export compared
      member-for-member against the 0076 netlist — 19 multi-pin nets /
      77 connected pins / 7 NC singletons, identical to 0076. Any difference
      is a failure: fix the wiring, not the netlist.
- [ ] ERC re-run; the report committed; the title-block ERC line matches the
      new report exactly.
- [ ] Fresh vector PDF and fresh >= 200 dpi phone-readable PNG committed.
- [ ] Branch `hermes/0077-wildfire-node-rev-c-relayout` pushed to origin; SHA
      reported.

## Constraints

- Placement and routing only. No component, value, pin, or net changes — the
  0076 netlist is frozen.
- Worker model is deepseek-flash (Stephen's call). KiCad CLI work costs
  nothing; keep model calls minimal.
- If a placement in the map above breaks a wire route, move the component
  within its zone — never change the netlist to suit the layout.
- Push only to the task branch. No secrets in the repo, ever.

## Proof

- Branch `hermes/0077-wildfire-node-rev-c-relayout` on origin + SHA, verified
  with `git ls-remote`.
- Netlist comparison PASS line (19 nets / 77 pins / 7 NC singletons,
  identical to 0076).
- ERC summary line; PNG dimensions and dpi.

## Reply format

Stage your reply as `mailbox/staged/0077-wildfire-node-rev-c-relayout.md`
with front matter carrying `task_id: "0077"`, `status: staged`, `iteration`,
and a `proof:` block (branch, sha, files, renders with dimensions, erc,
netlist check, model), then a Reply section noting any layout compromises you
had to make.
