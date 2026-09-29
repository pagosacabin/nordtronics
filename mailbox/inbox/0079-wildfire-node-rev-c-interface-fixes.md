---
task_id: "0079"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 6h
---

# 0079 — Rev C schematic: A1 solar-gate interface fix + Heltec full pinout

## Context

0077 (full-sheet re-layout) is verified and archived: branch
`hermes/0077-wildfire-node-rev-c-relayout` @
`5b629753c0cadcd0dbb44543f17924d11add4700`, files under
`hardware/wildfire-node-v1/`. Its netlist (19 multi-pin nets / 77 connected
pins / 7 NC singletons) was frozen as the baseline.

0078 (J1 upside-down text + TP1/F1 label nudge, text-only) is in active with
you, iteration 2, on branch `hermes/0078-wildfire-node-rev-c-text-fix`.

Stephen reviewed the Rev C render and found two interface defects that are
electrical, not cosmetic, so they are deliberately OUT of 0078's frozen-netlist
scope:

1. **A1 (SOLAR-GATE-V2 TEMP-GATE block) is wrong.** It is drawn with left pin
   = GND, right pin = SOLAR_OUT. The temp-gate is a series gate in the solar+
   path: the left side must be SOLAR_IN (panel + coming in) and the right side
   SOLAR_OUT (gated power to Heltec JP3 SOLAR). Additionally, the solar panel
   input appears nowhere in the node schematic — the system's power source is
   missing from the drawing.
2. **U1 (Heltec HTIT-WB32LAF V4.2) shows only the 14 used pins** (BAT, GND,
   SOLAR, 3V3, GPIO36, GPIO17, GPIO18, GPIO4, GPIO5, GPIO6, GPIO33, GPIO47,
   GPIO48, GPIO34). A module block in a system schematic must show the full
   module pinout so unused pins are visible and marked, not hidden.

Juno verified against the Heltec V4.2 schematic that the current GPIO
assignments are correct (GPIO36 = Vext_Ctrl, GPIO34 = VGNSS_Ctrl) — keep them.

## Task

On a new branch `hermes/0079-wildfire-node-rev-c-interface-fixes`, branched
off `hermes/0078-wildfire-node-rev-c-text-fix` at its current origin tip (if
0078 has already merged to main when you start, branch off main instead):

1. **Redefine the A1 symbol** (`wildfire-node-v1:SolarGate_v2_TempGate`) with
   three pins:
   - Pin 1, left side: `SOLAR_IN`
   - Pin 2, right side: `SOLAR_OUT`
   - Pin 3, bottom side: `GND`
   Wire pin 2 to the Heltec's SOLAR pin exactly as today (net unchanged).
   Wire pin 3 to the GND star exactly as today (net unchanged).
2. **Add the panel input.** Place a 2-pin connector symbol left of A1,
   reference `J2`, description `PANEL IN (13W 5V panel)`. Wire its pin 1 to A1
   pin 1 on a new net named `SOLAR_IN`; wire its pin 2 to the GND star net.
3. **Expand U1 to the full V4.2 pinout** — 42 pins total, using these exact
   names (sourced from the Heltec V4.2 schematic; keep the existing 14 nets
   wired exactly as they are today, mark every unused pin with an NC marker):
   - Header J2 (18): `GND`, `5V`, `Ve`, `Ve`, `RX` (GPIO44), `TX` (GPIO43),
     `RST`, `GPIO0`, `GPIO36`, `GPIO35`, `GPIO34`, `GPIO33`, `GPIO47`,
     `GPIO48`, `GPIO26`, `GPIO21`, `GPIO20`, `GPIO19`
   - Header J3 (18): `GND`, `3V3`, `3V3`, `GPIO37`, `GPIO46`, `GPIO45`,
     `GPIO42`, `GPIO41`, `GPIO40`, `GPIO39`, `GPIO38`, `GPIO1`, `GPIO2`,
     `GPIO3`, `GPIO4`, `GPIO5`, `GPIO6`, `GPIO7`
   - Additional pins (4): `GPIO18` (OLED_SCL), `GPIO17` (OLED_SDA), `GPIO16`
     (XTAL_32K_N), `GPIO15` (XTAL_32K_P)
   - Power (2): `BAT`, `SOLAR`
4. Keep the 0077 full-sheet layout style: notes block clear of wires, no text
   overlaps, no wires crossing text. Re-export the vector PDF and a >= 200 dpi
   PNG from the final `.kicad_sch`, re-run ERC, commit everything.

## Success criteria

- [ ] A1 has exactly three pins: `SOLAR_IN` (left), `SOLAR_OUT` (right),
      `GND` (bottom). `SOLAR_OUT` still reaches the Heltec SOLAR pin;
      `GND` still reaches the GND star.
- [ ] New `J2` panel connector present, labeled `PANEL IN (13W 5V panel)`;
      new `SOLAR_IN` net runs panel pin 1 -> A1 pin 1 only.
- [ ] U1 shows all 42 pins above. The 14 previously connected nets are
      unchanged (same pins, same nets). Every other U1 pin carries an NC
      marker — no unmarked dangling pins.
- [ ] Netlist diff against the 0077 baseline shows ONLY the expected
      additions: the new `SOLAR_IN` net and the new NC singletons. Any other
      net change is a failure. (This task deliberately changes the netlist —
      it is an electrical correction, not a layout pass.)
- [ ] ERC re-run: 0 errors; warnings noted with the count.
- [ ] Fresh vector PDF + fresh >= 200 dpi PNG committed under
      `hardware/wildfire-node-v1/`; branch
      `hermes/0079-wildfire-node-rev-c-interface-fixes` pushed to origin; SHA
      reported.

## Constraints

- Base branch: `hermes/0078-wildfire-node-rev-c-text-fix` tip (or main if
  0078 merged). Push only to the task branch.
- Schematic only — no PCB work.
- No changes to any other component, value, or net connection.
- Worker model stays deepseek-flash (Stephen's call) — do not escalate tier
  for this task.
- No secrets in the repo, ever.

## Proof

- Branch `hermes/0079-wildfire-node-rev-c-interface-fixes` on origin + SHA,
  verifiable with `git ls-remote`.
- Netlist diff output vs the 0077 baseline (19/77/7), showing only the
  `SOLAR_IN` net and NC-singleton additions.
- ERC summary line; PNG dimensions; PDF path in the branch.

## Reply format

Stage your reply as `mailbox/staged/0079-wildfire-node-rev-c-interface-fixes.md`
with front matter carrying `task_id: "0079"`, `status: staged`, `iteration`,
and a `proof:` block (branch, sha, files, renders, erc, netlist diff, model),
then a Reply section.
