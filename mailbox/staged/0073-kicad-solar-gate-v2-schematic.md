---
task_id: "0073"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - branch: hermes/0073-solar-gate-v2-schematic
    sha: 2e9e591d37b92e0f8707c50428446180412b223b
  - files:
      - hardware/solar-gate-v2/solar-gate-v2.kicad_sch
      - hardware/solar-gate-v2/solar-gate-v2.kicad_sym
      - hardware/solar-gate-v2/sym-lib-table
      - hardware/solar-gate-v2/solar-gate-v2.kicad_pro
      - hardware/solar-gate-v2/solar-gate-v2.kicad_prl
      - hardware/solar-gate-v2/solar-gate-v2.kicad_pcb
      - hardware/solar-gate-v2/solar-gate-v2-erc.txt
      - hardware/solar-gate-v2/solar-gate-v2-schematic.svg
      - hardware/solar-gate-v2/README.md
  - no_ci_run: >
      deliberate and verified, not an omission: no workflow matches hardware/**.
      android-build.yml triggers only on branches:[android-toolchain-setup];
      platformio.yml only on paths firmware/**; website-check.yml only on
      branches:[main, hermes/0068-site-email-refresh, hermes/0070-site-rewrite].
      Confirmed live: `gh run list -R pagosacabin/nordtronics --branch
      hermes/0073-solar-gate-v2-schematic` returns no runs.
  - main_transition_runs:
      - https://github.com/pagosacabin/nordtronics/actions/runs/36377143523
        # Website Check on main, success - the 0073 pickup commit this reply rides on
      - https://github.com/pagosacabin/nordtronics/actions/runs/36377111310
        # Website Check on main, success - the 0072 blocked declaration
  - erc: 0 errors / 22 warnings, all rule [lib_symbol_mismatch] - see the reply
  - erc_report: hardware/solar-gate-v2/solar-gate-v2-erc.txt (on the branch)
  - render: hardware/solar-gate-v2/solar-gate-v2-schematic.svg (on the branch)
notes: |
  Model: deepseek-flash (provider deepseek, profile cronrunner). Deliverable: a
  KiCad 10 schematic capture of the specified circuit in hardware/solar-gate-v2/,
  committed on hermes/0073-solar-gate-v2-schematic. Branch tip verified on origin
  with `git ls-remote --heads origin hermes/0073-solar-gate-v2-schematic`
  = 2e9e591d37b92e0f8707c50428446180412b223b = local HEAD at commit time.

  Built entirely through the KiCad MCP server; no KiCad file was hand-edited
  except the two library-table URIs set by set_library_table_uri and the
  project sym-lib-table's own text (see below). Sequence: create_project ->
  create_schematic -> import_symbol TL431 from the v1 project library into
  solar-gate-v2.kicad_sym -> register_symbol_library (project scope) ->
  set_library_table_uri to the ${KIPRJMOD} form the spec requires -> 31 symbol
  instances via batch_add_components (U1 placed as units 1/2/3) -> 68 net labels
  via batch_connect (0 failed) -> validate_schematic -> run_erc -> generate_netlist
  -> kicad-cli sch erc / sch export svg.

  VERIFIED, not assumed: (1) generate_netlist returns exactly the specified
  connectivity - 14 nets, membership pasted in the reply. (2) validate_schematic
  reports valid: true, 0 errors, 31 components, kicad-cli "Successfully saved
  schematic file using the latest format". (3) ERC 0 errors / 22 warnings with the
  rule name confirmed independently from the CLI report file (22 x
  [lib_symbol_mismatch], "** ERC messages: 22  Errors 0  Warnings 22"). (4) the
  render was measured geometrically against its own SVG export by an independent
  pass, which is why the layout was rebuilt once - see the deviations.

  The 22 warnings are one rule, not 22 problems: [lib_symbol_mismatch], i.e. the
  lib_symbols cache embedded in the schematic does not byte-match the library copy
  ERC compares against. Cause: the MCP symbol loader embeds symbols from the host
  RPM KiCad 9.0.7 libraries while ERC runs inside the Flatpak KiCad 10 sandbox and
  compares against KiCad 10's own Device/power copies. The tell is that the
  project's own symbol (solar-gate-v2:TL431) and the Comparator/Transistor_* 
  libraries raise no warning. No pin, net or connection is affected. The systemic
  fix is the kicad MCP server env KICAD_SYMBOL_DIR pointing at the Flatpak KiCad 10
  symbol runtime - the fix 0039 used, now absent from the server's env block. I did
  not restore it: it is a global config write plus a mid-run MCP restart, not a
  schematic change, and the task's criterion is zero errors with warnings explained.
  Two things learned and worth carrying: pointing the project sym-lib-table's stock
  Device/power entries at the Flatpak runtime does NOT work around it (the loader
  caches a resolved library path per nickname, so update_symbol_from_library just
  re-injects the cached host bytes), and kicad-cli 10 rejects `;` comments in a
  sym-lib-table - a commented table produced an extra warning, "The current
  configuration does not include the symbol library 'solar-gate-v2'", instead of
  resolving it.

  DEVIATIONS AND DECISIONS, all deliberate:
  1. `M` (the back-to-back middle node) and `Q3B` (the C -> Q3-base node) are the
     two nodes the spec did not name. Both carry explicit labels so nothing is
     implicit; every other net is named exactly as specified.
  2. The four BAT54 positions are drawn as Device:D_Schottky symbols valued BAT54.
     Stock KiCad has no single-diode BAT54 symbol - only the duals BAT54A/C/S and
     the SOT-323 BAT54W, whose third pin is NC and would need a no-connect flag.
     Topology and anode/cathode orientation are exactly as specified (DA/DB anode
     on C, cathode on OA/OB; D1/D2 anode on P/SOLAR_OUT, cathode on H).
  3. U1 is drawn as three units: A = cold comparator (pins 3/2/1), B = hot
     comparator (5/6/7), C = the power unit (8 = V+ = P, 4 = V- = GND). The power
     pins ARE drawn; there is no "not shown" note anywhere on the sheet. U1C is
     pins-only because that is how KiCad's own LM2903/LM393 library draws the power
     unit; a text annotation on the sheet states the pin mapping, and C1/C2 are both
     drawn.
  4. Q1a/Q1b/Q2a/Q2b are four separate AO3401 symbols referenced Q1A/Q1B/Q2A/Q2B,
     with sources on P / SOLAR_OUT and drains shared on M. This is deliberate: the
     KiCad AO3401A symbol is single-unit, so the spec's a/b notation is carried in
     the reference. Checked in generate_netlist that the four stay four separate
     components rather than being collapsed into one multi-unit part.
  5. Footprints: deferred, as the spec allows, because layout waits for bench
     validation. The passives, diodes and NTC have no footprint assigned and there
     is no fp-lib-table. Q1A/Q1B/Q2A/Q2B carry Package_TO_SOT_SMD:SOT-23, Q3
     Package_TO_SOT_THT:TO-92_Inline and U2 SOT-23, inherited from the symbols
     themselves, not chosen here.
  6. The project directory keeps the empty solar-gate-v2.kicad_pcb and .kicad_prl
     that create_project emits (v1 tracks its .kicad_prl too). Nothing was laid out:
     the board file has no outline and no footprints, and no layout tool was run.
  7. Layout was REBUILT once. The first capture placed DB, Rb, Q3 and Q2B inside the
     sheet's title-block region (Q3 bisected by it) and ran one SOLAR_OUT label
     through the frame's inner border. Both were found by measuring the render
     against its own SVG geometry, not by eye; the committed revision has 0 bodies
     in the title-block region and 0 labels crossing the border, re-measured. The
     measurements, and the residual cosmetic overlaps that remain, are in the
     project README's "Render verification" section.
  8. Not a build task: no artifact is built by CI and the spec names no ntfy topic,
     so nothing was published to ntfy. Declared so it is not read as an omission.
  9. Trip-point cross-check (the spec invited it): recomputed independently from the
     captured values, Vb reproduces to 0.0001 V and the hot pair to 0.05 degC
     (+48.10 / +47.50 vs spec +48.05 / +47.45). The cold pair is ~0.5 degC below the
     spec (-0.52 / +1.68 vs spec -0.05 / +2.3) while the cold hysteresis BAND
     reproduces (2.20 vs 2.35 degC). A constant offset with a matching band points
     at an assumption in the spec's arithmetic (open-collector saturation voltage,
     or the NTC's B-reference), not a wiring difference - needs a decision, not a
     schematic change. Numbers and method in the README.

  No repository file outside hardware/solar-gate-v2/ was touched, and
  hardware/solar-gate-v1/ is untouched. Blocked: nothing.
---

# 0073 — KiCad schematic capture: solar gate v2 (temperature-gated charging switch)

## Context

Stephen selected the winning design from a four-way AI design review (ChatGPT,
Grok, Claude, Copilot). The winner is the Claude clean-sheet design below.
A previous KiCad schematic exists at `hardware/solar-gate-v1/` (task 0039 did
the first capture); this is a new topology, so it goes in a NEW project
`hardware/solar-gate-v2/`. Follow the v1 project's library conventions
(sym-lib-table entries using `${KIPRJMOD}`, per the 0043 fix).
Model: deepseek-flash. Report the model used in your reply.

## Task

One deliverable: a complete, ERC-clean KiCad schematic of the circuit
specified below, in `hardware/solar-gate-v2/`, pushed to a branch.
No PCB layout — layout waits for bench validation of the trip points.

## The circuit (Claude design, Juno-verified)

Nets: P (panel +5V in), SOLAR_OUT, VREF (2.5V), N (sense node), PA (cold ref),
Vb (hot ref), OA (cold comp output), OB (hot comp output), C (fault-OR node),
G (all FET gates), H (gate max-selector node), GND.

Reference: Rbias 1.6k P to VREF. U2 TL431: K=VREF, A=GND, REF tied to K.
C2 0.1uF VREF to GND at U2.
U1 LM393: VCC=P, GND=GND. C1 0.1uF P to GND at U1 pin 8. DRAW the power
pins and both caps — no "not shown" notes.

Sense: Rt 34.0k VREF to N. NTC 10k B3950 N to GND (2-wire, thermally on the
cell). N falls as temperature rises. Open NTC pulls N to VREF = extreme
cold = charging OFF (fail safe).

Cold comparator U1A: N on (-), PA on (+), output OA.
R1 100k VREF to PA. R2 95.3k PA to GND. Rfa 1.5M OA to PA (positive
feedback, OUT to the + input). Rpua 100k VREF to OA (open-drain pull-up).

Hot comparator U1B: N on (+), Vb on (-), output OB.
Rb1 200k / Rb2 23.2k divider VREF to Vb to GND (Vb = 0.260V).
Rfb 1.5M OB to N (positive feedback to the + input).
Rpub 100k VREF to OB (open-drain pull-up).

Fault OR: Rpuc 100k VREF to C. DA, DB BAT54: anode to C, cathode to OA / OB
respectively. Rb 10k C to Q3 base.
Either comparator output low pulls C down and turns Q3 off.

Gate drive: Q3 2N3904, collector to G, emitter to GND, base via Rb.
Q1a and Q1b AO3401 in parallel: source to P.
Q2a and Q2b AO3401 in parallel: drain to middle node M, source to SOLAR_OUT.
All four gates tied to G. Q1 and Q2 are back to back: no body diode conducts
in either direction when off. This is deliberate — do not "simplify" it to
a single FET.
D1 BAT54: anode P, cathode H. D2 BAT54: anode SOLAR_OUT, cathode H.
Rg 100k H to G. G rests at max(P, SOLAR_OUT) minus a diode drop = FETs OFF.
Q3 on pulls G near GND = FETs ON (Vgs approx -4.8V).

Trip points (for your own cross-check, not a substitute for ERC):
cold cutoff -0.05 C, re-enable +2.3 C; hot trip 48.05 C, re-arm 47.45 C.

## Success criteria

- Every component and net above appears in `hardware/solar-gate-v2/`
  with the exact values shown. E96 values (95.3k, 23.2k, 34.0k) are standard
  stock — use them as specified.
- ERC passes with zero errors (warnings listed and explained in the reply).
- Committed on a branch named `hermes/0073-solar-gate-v2-schematic`,
  pushed to origin.

## Constraints

- Do NOT modify `hardware/solar-gate-v1/` or any other existing project.
- Do not start PCB layout. Schematic only.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- Branch name and SHA on origin.
- ERC output pasted (or screenshot path on the branch).
- The `.kicad_sch` file path on the branch.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field listing any ERC
warnings and any footprint assignments you made (or deliberately deferred).

---

# Reply — Hermes → Juno

**Status: staged.** Deliverable is on the branch; proof pointers in the front matter.
**Model: `deepseek-flash`** (provider `deepseek`, profile `cronrunner`).

| Item | Value |
|---|---|
| Branch | `hermes/0073-solar-gate-v2-schematic` |
| Branch tip on origin | `2e9e591d37b92e0f8707c50428446180412b223b` (`git ls-remote --heads origin …`) |
| Schematic | `hardware/solar-gate-v2/solar-gate-v2.kicad_sch` |
| ERC report | `hardware/solar-gate-v2/solar-gate-v2-erc.txt` (on the branch) |
| Render | `hardware/solar-gate-v2/solar-gate-v2-schematic.svg` (on the branch) |
| CI run | none exists — no workflow matches `hardware/**`; see `no_ci_run` above |
| ntfy | none published — not a build task, no topic named in the spec |

31 symbol instances, 14 nets, `sym-lib-table` using `${KIPRJMOD}` (0043 convention).

## ERC — pasted verbatim

```
$ kicad-cli sch erc --severity-all --format report solar-gate-v2.kicad_sch
Found 22 violations
Saved ERC Report to solar-gate-v2-erc.txt

 ** ERC messages: 22  Errors 0  Warnings 22
```

**Zero errors.** All 22 warnings are the single rule `[lib_symbol_mismatch]`:

```
[lib_symbol_mismatch]: Symbol 'R' doesn't match copy in library 'Device'
    ; warning
@(35.56 mm, 69.85 mm): Symbol R1 [R]
… 21 more, all of the same rule (R, C, D_Schottky, Thermistor_NTC in 'Device';
  PWR_FLAG in 'power')
```

This is a symbol-cache version skew, not a schematic defect: the MCP loader embeds
copies from the host RPM KiCad 9.0.7 libraries, while ERC runs inside the Flatpak
KiCad 10 sandbox and compares against KiCad 10's own `Device`/`power` copies. The
project's own symbol (`solar-gate-v2:TL431`) and the Comparator/Transistor libraries
raise no warning, which is the tell. No pin, net or connection is affected. The
systemic fix is `KICAD_SYMBOL_DIR` on the kicad MCP server (0039's fix, since lost
from the server's env block); I did not apply it because it is a global config write
plus a mid-run MCP restart, and this task's criterion is zero errors with warnings
explained. Detail in `notes:` and in the project README.

## Connectivity — `generate_netlist`, verbatim

```
Nets (14):
  /C: DA/2, DB/2, Rb/1, Rpuc/2
  /G: Q1A/1, Q1B/1, Q2A/1, Q2B/1, Q3/3, Rg/2
  /GND: C1/2, C2/2, NTC/2, Q3/1, R2/2, Rb2/2, U1/4, U2/3
  /H: D1/1, D2/1, Rg/1
  /M: Q1A/3, Q1B/3, Q2A/3, Q2B/3
  /N: NTC/1, Rfb/2, Rt/2, U1/2, U1/5
  /OA: DA/1, Rfa/1, Rpua/2, U1/1
  /OB: DB/1, Rfb/1, Rpub/2, U1/7
  /P: C1/1, D1/2, Q1A/2, Q1B/2, Rbias/1, U1/8
  /PA: R1/2, R2/1, Rfa/2, U1/3
  /Q3B: Q3/2, Rb/2
  /SOLAR_OUT: D2/2, Q2A/2, Q2B/2
  /VREF: C2/1, R1/1, Rb1/1, Rbias/2, Rpua/1, Rpub/1, Rpuc/1, Rt/1, U2/1, U2/2
  /Vb: Rb1/2, Rb2/1, U1/6
```

Every specified part, value and net is present and where the spec puts it. `U1/4` and
`U1/8` are the drawn power unit; `U2/2` (REF) ties to `U2/1` (K) as required; `M` and
`Q3B` are the two unnamed nodes, labelled so nothing is implicit.

## What I checked, and how

- Connectivity: `generate_netlist` above, compared net by net against the spec.
- Structure: `validate_schematic` → `valid: true`, 0 errors, 31 components, and
  kicad-cli loads the file ("Successfully saved schematic file using the latest format").
- ERC: run through the MCP server **and** independently through
  `kicad-cli sch erc`, with the report file committed so it can be re-read.
- The drawing itself: measured geometrically against the sheet's own SVG export
  (exact mm boxes per string). The **first capture failed that check** — DB, Rb, Q3 and
  Q2B sat inside the sheet's title block and a `SOLAR_OUT` label crossed the frame
  border — so the layout was rebuilt and re-measured: 0 bodies in the title-block
  region, 0 labels crossing the border. Residual cosmetic text overlaps are listed in
  the project README; none is legible at normal zoom and none affects connectivity.
- Trip points, recomputed independently: `Vb` 0.2599 V (spec 0.260); hot trip +48.10 /
  re-arm +47.50 °C (spec +48.05 / +47.45); cold cutoff −0.52 / re-enable +1.68 °C
  (spec −0.05 / +2.3). Hot agrees to 0.05 °C and the cold hysteresis *band* reproduces
  (2.20 vs 2.35 °C), but the cold pair sits ~0.5 °C low — a constant offset with a
  matching band, so it looks like an assumption in the spec's arithmetic rather than a
  wiring difference. **Flagged for a decision before layout**, since it moves the cold
  trip half a degree.

## Footprints — deliberately deferred

None assigned to the passives, diodes or NTC, and no `fp-lib-table`: layout waits for
bench validation (spec). Q1A/Q1B/Q2A/Q2B (SOT-23), Q3 (TO-92) and U2 (SOT-23) carry
footprints inherited from their library symbols, not chosen here.

## Deviations, in one place

`M` and `Q3B` named; BAT54 drawn as `Device:D_Schottky` valued BAT54 (stock KiCad has
no single BAT54 symbol); U1's power pins drawn as unit C (pins-only, as KiCad's own
LM393 draws them) with an on-sheet annotation; Q1a/Q1b/Q2a/Q2b as four separate
single-unit AO3401 symbols (verified not collapsed into one component); empty
`.kicad_pcb`/`.kicad_prl` kept as the template emits them; no CI run and no ntfy, both
declared above with the evidence. `hardware/solar-gate-v1/` is untouched.

