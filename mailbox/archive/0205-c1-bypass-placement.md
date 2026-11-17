---
task_id: "0205"
protocol_version: 1.3.0
status: verified
iteration: 1
expect-reply-within: 24h
proof:
  branch: hermes/0199-temp-cutoff-pcb
  sha: f4f6edba476414723cae74d985f849b0cc65d799
  run: https://github.com/pagosacabin/nordtronics/actions/runs/44085883171
  pull_request: https://github.com/pagosacabin/nordtronics/pull/5
  review_comments:
    - https://github.com/pagosacabin/nordtronics/pull/5#issuecomment-6948527123
  files:
    - hardware/solar-gate-v2/solar-gate-v2.kicad_pcb
    - hardware/solar-gate-v2/drc-report.rpt
    - hardware/solar-gate-v2/README.md
    - hardware/solar-gate-v2/fab/gerbers/ (F_Cu, B_Cu, F_Mask, B_Mask, F_Silkscreen, B_Silkscreen, Edge_Cuts, .gbrjob)
    - hardware/solar-gate-v2/fab/drill/ (PTH + NPTH Excellon)
    - hardware/solar-gate-v2/renders/board-top.png
    - hardware/solar-gate-v2/renders/board-bottom.png
  files_note: >-
    15 files, all on the task branch (git show --stat f4f6edb). Nothing under
    hardware/solar-gate-v2/solar-gate-v2.kicad_sch changed — the schematic is the fixed input,
    so renders/schematic.png was not re-rendered. solar-gate-v2.kicad_pro is deliberately
    untouched: kicad-cli materialises a defaults block into it on every run and that block was
    reverted, so the project file is byte-identical to 7683739.
  run_note: >-
    This repo path has one applicable workflow, .github/workflows/website-check.yml (triggers:
    push to main + four named hermes/* branches, and pull_request to main). The run above is the
    pull_request run for this head (headSha f4f6edb, conclusion success). It is an htmlhint +
    linkchecker pass over website/index.html and it does NOT compile, plot or validate the KiCad
    tree — the board evidence is the committed drc-report.rpt plus the commands in the README's
    "Rebuilding" section, both reproducible from origin.
  ntfy_receipt: >-
    none — this task produces no compiled artifact and no build workflow runs for
    hardware/solar-gate-v2 (only the Website Check fires on this branch), so the
    nordtronics-build topic does not apply.
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
notes: |
  STAGED. Run 2026-11-17 12:30-12:5x UTC (05:30-05:5x MST), off-peak (PEAK: OFF-PEAK 12:30 UTC),
  PROTOCOL MATCH 1.3.0.

  PICKUP. 0205 was the lowest-numbered actionable task. In active/: 0142 and 0187 are
  `status: superseded`; 0159 is decision-blocked (untouched — file not edited, iteration not
  moved, blocker not re-declared). In inbox/: 0176 is `status: superseded` ("do not pick up").
  So 0205, claimed via mailbox-transition.py (status in_progress, iteration 1).

  ALL FOUR PARTS DONE on branch hermes/0199-temp-cutoff-pcb, head f4f6edb, PR #5 updated.
  Measurements, not eyeballing: pad positions, track lengths, clearances and text boxes were read
  from the board file with pcbnew; pours were verified by testing the filled polygons.

  ONE SPEC PREMISE CORRECTED, declared: on a DIP-8, pin 8 (VCC) is top-right and pin 4 (GND) is
  bottom-left — opposite corners, so "the pin-8/pin-4 end of the package" is not a single end.
  C1 is placed at the pin-8 end, because pin 8 is the supply pin the cap exists to serve; the
  ground side is the pour (C1.2 sits in it, 1.5 mm from a stitching via), not a 15 mm trace to
  pin 4. Both readings of the requirement cannot be met at once; this is the one that shortens the
  loop that matters.

  WHAT CHANGED (numbers from the committed board):
   - C1 (30,40) -> (43.8,48.6), rotation 0. C1.1 is 5.83 mm pad-centre from U1.8 (was 21.9 mm);
     edge-to-edge 4.23 mm; C1's courtyard clears U1's by 1.33 mm. The P copper from C1.1 to U1.8
     is now 5.8 mm of 1.5 mm B.Cu; before, C1.1 fed through J1.1 -> D1.2 -> U1.8, ~32 mm.
   - GND pours on F.Cu and B.Cu: clearance 0.3 mm, thermal gap 0.3 mm, spoke width 0.5 mm, min
     thickness 0.25 mm, pads thermal. F.Cu fills as ONE island (3863.9 mm2). B.Cu fills as one
     4029.3 mm2 island plus a 1.2 mm2 pocket attached to U2.2's pad. Both J1.2 and J2.2 touch the
     pour on both layers, and the top corridor is continuous copper at y=26 from x=30 to x=90 —
     i.e. the J1.2 <-> J2.2 return is plane copper, not the old 0.2 mm trace.
   - 6 GND stitching vias, 0.8/0.4 (board's existing via size): (48.2,47.2) beside C1.2/U1.8;
     (36,29) and (45,29) J1 side; (95,29) and (103.2,45) J2 side; (34.5,64) U1 pin-4 side.
   - U2 VREF strap pushed from y=45.0517 to y=45.9: clearance to U2.2's pad 0.20 -> 1.05 mm
     (measured: pad edge 44.75, trace edge 45.80).
   - The 0.20 mm F.Cu GND run that passed over U2 pins 1-3 was deleted — it was the second
     zero-margin item in the card's addition 3, and the pour (0.3 mm) now does its job. Q5.1 and
     TH1.2 keep their other GND path plus the pour.
   - Labels, measured distance to the pad each names (was -> now): "P +" 9.6 -> 0.38 mm (J1.1);
     "GND" (input) 17.9 -> 0.68 mm (J1.2); "SOLAR_OUT +5V" 15.8 -> 0.41 mm (J2.1) and 15.3 mm
     from TH1.1; "NTC CELL" 0.41 mm (TH1.1, unchanged position, moved 0.3 mm off the pad edge);
     "GND" (J2) left where it was at 1.48 mm. No silkscreen text overlaps another text or a pad
     (checked as boxes and by DRC's silk rules: zero silk violations).
   - /OA re-routed because C1's slots landed on it: U1.1 -> (40,50.9) -> (54.2,50.9) -> D3.1,
     same net, same four members. This is a required consequence of the move, not a circuit change.

  VERIFIED, with the command each claim comes from:
   - ERC: `kicad-cli sch erc --severity-all` -> "ERC messages: 0  Errors 0  Warnings 0".
   - DRC: `kicad-cli pcb drc --severity-all --schematic-parity` -> 2 violations, both the known
     [lib_footprint_mismatch] on J2 and TH1; "Found 0 unconnected pads"; 4 [extra_footprint]
     parity notes for MH1-MH4 (same four as 7683739). Committed verbatim as drc-report.rpt.
   - Netlist: exported `sch export netlist --format kicadxml` from f4f6edb and from 7683739 and
     compared the net partition: 14 nets, 70 members, 29 components, identical, /P = C1.1 D1.2
     J1.1 Q1.3 Q2.3 R3.1 U1.8. Pad->net map over the PCB also identical (70 pads, 0 differences).
   - Fab: Gerbers re-plotted from the updated board (F_Cu 10.0 kB -> 104.9 kB, B_Cu 9.9 -> 107.1 kB
     — the pours), same 8 files + .gbrjob; drill re-exported (PTH 1463 -> 1558 B, the six vias).
   - Renders: board-top.png and board-bottom.png re-rendered at the same 2184x1488. Renders are a
     judgement I cannot make from a cron tick — I have no vision tool — so the placement claim
     above is the measured one; the "looks right" call is Stephen's at the gate, as the card says.
   - Copilot peer review routed through the Copilot pane (hc t17, keyring OAuth, no gh-env.sh),
     posted on PR #5 as issuecomment-6948527123: four [note] entries, no findings, Verdict: SHIP.
     I read it back from the API rather than trusting the pane's echo.

  DEVIATIONS AND EXTRA SCOPE, all declared:
   1. /OA reroute (above) — forced by the C1 move.
   2. Three GND segments of the old C1 branch deleted (35,40)-(37.6,40), (37.6,40)-(37.6,45.77),
     (37.6,45.77)-(46.23,54.39): orphaned by the move.
   3. Silkscreen: besides the three labels the card names, "P +" was also moved (the card's own
     criterion is "every off-board pad's label sits adjacent to its own pads", and it was 9.6 mm
     out); J2's, U2's and Q5's reference designators were moved because the new label rows crossed
     them. J2's own "GND" was left where the card says it may be left.
   4. README: the open item became the pin-1<->5 resolution (as asked) and two factual lines were
     also corrected because the change had made them false — the routing count (165 segments/1 via
     -> 160/7 plus 2 pours) and a short "Layout change 2026-11-17" note. No other docs touched.
   5. solar-gate-v2.kicad_pro reverted after kicad-cli rewrote it with a defaults block.
   6. One extra stitching via at (43.5,62.5) was attempted and dropped: pcbnew saved it with a
     neighbouring track's net (/OB) even though SetNet/SetNetCode reported /GND in memory, twice.
     Rather than ship a mis-netted via I removed it; 6 stitching vias remain (the card asks for
     "several"). All 7 vias in the committed file were re-read from the file to confirm their nets.

  PR #5 was NOT merged and nothing was purchased or ordered.
---

# 0205 — Temp-cutoff board: C1 move, silkscreen label fixes, GND pour

**Branch:** `hermes/0199-temp-cutoff-pcb` — **head `f4f6edba476414723cae74d985f849b0cc65d799`**
**PR:** #5 (updated, still open, not merged) — **CI:** Website Check run 44085883171, `headSha` = head, conclusion success
**Copilot review:** [#issuecomment-6948527123](https://github.com/pagosacabin/nordtronics/pull/5#issuecomment-6948527123) — no findings, **Verdict: SHIP**

The schematic is untouched; everything below is layout, copper and silkscreen on the Mark 1 Rev 1
board.

## 1. C1 is now against U1's supply pin

C1 moved from **(30, 40)** to **(43.8, 48.6)**, rotation 0 — directly above U1, its pad 1 on the
same copper as pin 8:

| quantity | before | after |
|---|---|---|
| C1.1 ↔ U1.8 (pad centres) | 21.9 mm | **5.83 mm** (4.23 mm edge-to-edge) |
| `P` copper from C1.1 to U1.8 | ~32 mm via J1.1 → D1.2 | **5.8 mm** of 1.5 mm B.Cu |
| GND return | 0.2 mm trace to the J1 branch | the pour itself (C1.2 in-plane, stitching via 1.5 mm away) |
| C1 courtyard clearance to U1 | n/a (different part of the board) | 1.33 mm |

**One correction to the card, stated rather than worked around:** on a DIP-8, pin 8 (VCC) is
top-right and pin 4 (GND) is bottom-left — they are diagonal, so there is no single "pin-8/pin-4
end". I placed C1 at the pin-8 end, because pin 8 is the supply pin whose loop the cap exists to
shorten, and took the ground side into the pour (C1.2 → plane → U1.4's pad) instead of a 15 mm
trace. `/OA`, which ran exactly through the new position, was re-routed (U1.1 → down → across at
y=50.9 → D3.1); its four members are unchanged.

## 2. GND return is now copper, not a trace

The card's finding was that the whole 3 A return between J1.2 and J2.2 crossed the board in
0.2 mm traces. Both layers now carry a GND pour, stitched:

- **pours**: F.Cu and B.Cu, clearance **0.3 mm**, thermal gap 0.3 mm, **spoke width 0.5 mm**,
  min thickness 0.25 mm. F.Cu fills as one island of 3863.9 mm²; B.Cu as one of 4029.3 mm² plus a
  1.2 mm² pocket attached to U2's GND pad. J1.2 and J2.2 touch the pour on **both** layers, and the
  top corridor is unbroken copper from x=30 to x=90 at y=26 — the return no longer goes near U1,
  U2 or C2.
- **stitching vias** (0.8 mm pad / 0.4 mm drill, the board's existing size), six of them:
  (48.2, 47.2) at C1.2/U1.8; (36, 29) and (45, 29) on the J1 side; (95, 29) and (103.2, 45) on the
  J2 side; (34.5, 64) on U1 pin 4's side.
- **U2 clearance**: the VREF strap past pin 2 goes from **0.20 mm to 1.05 mm**. The 0.20 mm F.Cu
  GND run that also grazed U2's pins 1–3 was removed — the pour does that job at 0.3 mm now.

## 3. Silkscreen: every off-board pad labelled on its own pads

Measured box-to-pad distance, before → now:

| label | names | before | now |
|---|---|---|---|
| `P +` | J1.1 (`P`) | 9.6 mm | **0.38 mm** |
| `GND` | J1.2 (`GND`) | 17.9 mm | **0.68 mm** |
| `SOLAR_OUT +5V` | J2.1 (`SOLAR_OUT`) | 15.8 mm | **0.41 mm** (and 15.3 mm from TH1.1, was 15.5) |
| `NTC CELL` | TH1.1 (`N`) | 0.41 mm (but touching the pad box) | **0.41 mm**, 0.3 mm clear of the pad edge |
| `GND` | J2.2 (`GND`) | 1.48 mm | left there, as the card allows |

`TH1`'s reference no longer crowds `NTC CELL` (they are now 2.1 mm apart vertically, `NTC CELL`
staying by TH1's pads). `J2`, `U2` and `Q5` reference designators moved because the new label rows
crossed them. Checked two ways: box arithmetic over every silk text and pad (**0 text/text
overlaps, 0 text/pad overlaps**) and DRC's silk rules (**0 silk violations**).

## 4. Verification

- **ERC 0 / 0** — `kicad-cli sch erc --severity-all`.
- **DRC** — 2 violations, both the known `[lib_footprint_mismatch]` on J2 and TH1; **0 unconnected
  pads**; the same 4 `[extra_footprint]` parity notes for MH1–MH4 as the previous head. Committed
  as `drc-report.rpt`.
- **Netlist identical to 7683739** — exported both and compared the partition: **14 nets, 70
  members, 29 components, no difference**; `/P = C1.1, D1.2, J1.1, Q1.3, Q2.3, R3.1, U1.8`,
  `/GND` unchanged. The PCB's pad→net map is also identical (70 pads, 0 differences).
- **Fab re-exported** from the updated board: Gerbers (F_Cu 10.0 → 104.9 kB, B_Cu 9.9 → 107.1 kB —
  the pours are in the plotted copper), drill PTH 1463 → 1558 B (the six vias).
- **Renders re-exported** at the same 2184×1488; `renders/schematic.png` untouched (no schematic
  change).

## 5. Copilot peer review

Routed through the Copilot pane over Herdr with keyring OAuth (no `gh-env.sh`), and posted on the
PR as [issuecomment-6948527123](https://github.com/pagosacabin/nordtronics/pull/5#issuecomment-6948527123).
It independently re-exported the netlists, re-ran DRC, checked the zone parameters and the six
vias, and found nothing: four `[note]` entries, **Verdict: SHIP**. Read back from the API, not
taken from the pane's echo.

## 6. What CI does and does not cover

This repo path has no KiCad job: the only workflow that fires on this branch is `website-check.yml`
(htmlhint + linkchecker over `website/index.html`), and it went green at this head. It says nothing
about the PCB. The board's evidence is `drc-report.rpt` in the commit plus the four commands in the
README's "Rebuilding" section, which regenerate every number quoted above from the committed files.

PR #5 is left open for Juno to verify and archive; nothing was merged, purchased or ordered.
