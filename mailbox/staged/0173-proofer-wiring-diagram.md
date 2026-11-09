---
task_id: "0173"
protocol_version: 1.2.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: hermes/0173-proofer-wiring
  sha: "f1c48a6fda9e6e3db27bb964a0b0fca8a1cff56b"
  run: none
  run_note: "No workflow can fire on this branch. Enumerated: ls .github/workflows/ gives
    android-build.yml (push branches [android-toolchain-setup]), android-companion-v0.yml
    (branch allow-list + paths android/companion-v0/**), backend.yml (paths backend/**),
    detection-sim.yml (paths python/detection-sim/**), platformio.yml (paths firmware/**),
    website-check.yml (push branches [main, hermes/0068-..., hermes/0070-..., hermes/0093-...]).
    None lists hermes/0173-proofer-wiring and none carries a docs/** path filter, so the
    push starts no run. Confirmed live: gh run list -R pagosacabin/nordtronics
    --branch hermes/0173-proofer-wiring returns empty (rc 0, zero rows) while the pickup
    commit on main did trigger Website Check 42700781128 (success)."
  files:
    - docs/proofer-j7239/j7239-wiring.kicad_sch
    - docs/proofer-j7239/j7239-wiring.pdf
  file_note: "main carries 0 files under docs/proofer-j7239/ -- the whole directory was
    created on the task branch, so both paths exist only at the branch tip above."
  ntfy: "not applicable -- no compiled/firmware artifact. The proofer sheet is a KiCad
    schematic plus a PDF; the companion topic nordtronics-build-ed05a663 is for APK builds.
    No receipt was published."
  verified:
    netlist: "kicad-cli 10 sch export netlist --format kicadxml, parsed and compared to the
      spec's pin groups: /120 V HOT (L) = {SW1.1}; /SWITCHED HOT = {SW1.2, TH1.1, M1.1};
      /SWITCHED HOT (THERMOSTAT) = {TH1.2, R1.1}; /NEUTRAL (N) = {J1.1, J1.2, J1.3, M1.2,
      R1.2}; J1.4 = unconnected (spare). Five nets, no merged and no split group."
    pdf: "pdfinfo: Pages 1, Page size 841.896 x 595.296 pts (A4 landscape). Rasterised at
      150 dpi and checked pixelwise: 0 of 1,754x1,241 pixels carry any colour (R!=G or
      G!=B) -- the sheet is strictly black and white."
    geometry: "kicad-cli sch export svg, then every <g class=\"stroked-text\"> glyph box
      parsed: 87 unique strings, 0 outside the page, 0 overlapping each other, 0 sitting on
      a symbol body, 0 rotate(-180) transforms (nothing upside down)."
    erc: "1 error + 6 warnings, all explained below and none a wiring defect."
cost:
  model: deepseek-flash
  api_calls: 101
  input_tokens: 64296
  input_cache_hit_tokens: 11050368
  input_cache_miss_tokens: 64296
  output_tokens: 77616
  total_tokens: 11172280
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.089365
  cost_note: "Read from this profile's state.db session_model_usage for session
    local-a48d7a99... (101 API calls, input 64,296, cache-read 11,050,368, output 77,616,
    reasoning 50,460), snapshot taken at 21:25Z before the staging transition. Arithmetic:
    64296x0.15e-6 + 11050368x0.003e-6 + 77616x0.60e-6 = 0.08936510. Run 21:15Z onward
    Monday = off-peak (PEAK: OFF-PEAK 21:15 UTC). The staging calls after the snapshot are
    not included."
notes: |
  STAGED. Run 2026-11-09 14:15-14:26 MST (21:15-21:26Z), off-peak (PEAK: OFF-PEAK 21:15
  UTC), PROTOCOL MATCH 1.2.0. Active pair skipped: 0142 carries status: superseded (Juno's
  2026-10-19 resolution) and 0159 is decision-blocked (controller address owed) -- both left
  untouched, per rule 6. 0173 was the lowest actionable task.

  WHAT WAS BUILT. One KiCad 10 schematic drawn through the kicad-mcp server and exported by
  kicad-cli 10.0.6: docs/proofer-j7239/j7239-wiring.kicad_sch (5 symbols: Switch:SW_SPST x2,
  Motor:Fan, Device:Heater, Connector:Screw_Terminal_01x04) and j7239-wiring.pdf. A4
  landscape, single page, black and white. Ladder layout: HOT from the wall enters the MAIN
  SWITCH, the switch output taps down to the FAN and continues to the THERMOSTAT, the
  thermostat feeds the HEATING ELEMENT, and all three neutrals drop to the 4-terminal block.
  Eight wires plus the neutral rail; four net labels carry the wire functions (120 V HOT (L),
  SWITCHED HOT, SWITCHED HOT (THERMOSTAT), NEUTRAL (N)). Plain-word part names are text
  annotations, the five notes blocks give "how it works" and "replacing parts" verbatim in
  plain language, and no wire colour is stated anywhere.

  VERIFIED, not assumed. The load-bearing check is the exported netlist: it reproduces the
  spec's pin partition exactly (see proof.verified.netlist) -- a merged net would mean an
  accidental short, a split one an open, and neither occurred. Then: PDF is one A4-landscape
  page with 0 coloured pixels; the SVG export was measured so no text falls off the sheet,
  overlaps another string, or lands on a symbol body; and every exported string was checked
  for an upside-down rotate(-180) transform (none).

  ERC IS NOT CLEAN, AND HERE IS WHY (declared rather than hidden): 1 error
  [pin_not_connected] on SW1 pin 1 plus 1 warning [isolated_pin_label] on the label "120 V
  HOT (L)", and 5 [lib_symbol_mismatch] warnings. The first two are the same fact: the wall
  supply enters on a labelled wire stub, not on a connector symbol, because the task forbids
  adding parts that are not in the bench circuit -- so the hot net contains exactly one pin
  and ERC calls that pin unconnected. J1 pin 4 (the spare terminal) is deliberately wired to
  nothing and is expected to read as an unconnected pin. The lib_symbol_mismatch warnings are
  environmental, not schematic: the embedded symbol cache came from the host KiCad 9 library
  (/usr/share/kicad/symbols/Switch.kicad_sym, header version 20241209 / generator 9.0) while
  the Flatpak kicad-cli 10 ERC compares against its own runtime library (version 20251024 /
  generator 10.0). The file renders from its embedded cache, so the PDF is unaffected.

  ONE MECHANICAL CORRECTION, declared: the thermostat is drawn with Switch:SW_SPST, the same
  symbol family as the main switch, because a thermostat IS a single-pole switch and KiCad's
  library has no thermostat symbol. Its plain-word label reads THERMOSTAT and its qualifier
  line says "heating element on/off only", so it cannot be mistaken for the main switch.

  LAYOUT CHOICES, so a reviewer does not read them as omissions: (1) the terminal block is
  drawn with its four pins in a row, pins 1-3 landing the wall, fan and element neutrals and
  pin 4 marked "SPARE (not used)"; (2) the horizontal rail joining those three risers is the
  schematic representation of the block's own commoning (the block is one solid bus), and the
  note says so in plain words -- no jumper wire is being invented; (3) the part name uses an
  ASCII hyphen in "TERMINAL BLOCK - NEUTRAL BUS" rather than the em dash of the task text, to
  stay inside the stroke font's safe glyphs; (4) Value fields are hidden and references are
  kept small (SW1/M1/TH1/R1/J1) -- the sheet is for a non-electrician, so the plain-word
  annotations carry the meaning and the refs stay as a technician's cross-reference. The
  schematic's own Value strings still hold the ratings (16 A MIN, FAN MOTOR, THERMOSTAT,
  ~10 OHM 1500 W, NEUTRAL BUS).

  Nothing may have been reported: Stephen did not report wire colours, so none are printed;
  no connection in the spec was ambiguous, so nothing was guessed. Deliverable is on the
  branch only -- main still carries 0 files under docs/proofer-j7239/. The three untracked
  worktree items (firmware/wildfire-node-v1/bench-override.ini, hardware/solar-gate-v2/,
  take_screenshots.py) pre-existed this run and were not touched or committed.
---


# 0173 — Proofer J7239 wiring diagram (KiCad, handoff sheet for Black Bart's)

## Assignment

- Hermes: draw the KiCad schematic, export the PDF, push to the branch.
- Newton: not involved — KiCad schematic capture is Hermes's proven rung; no software step.

## Context

Alliance Manufacturing proofer, model J7239, serial 50539A9603, 120 VAC, 1500 W, 13.5 A, at Black Bart's in Pagosa Springs. Stephen rebuilt the wiring box on the bench on 2026-11-09 and bench-proved it the same day: the wires were tangled together, corroded and broken, the terminals corroded beyond use, and the original main switch was bad. Live test passed: element ~10 Ω, at 200°F setpoint the element pulls current, turned down below 60°F it cuts off, fan spins. Remaining work is buttoning up the wiring box and reinstalling at Black Bart's.

Purpose of this drawing: a one-page sheet that travels WITH the proofer, so the crew at Black Bart's can rewire it themselves if it ever fails again. The audience is not dumb, but they are not electricians — the sheet must be readable by a non-electrician. Plain-word labels, not symbols alone.

Bench-proven wiring (Stephen, 2026-11-09 — this is the whole circuit, nothing else):

- Wall power 120 VAC in: hot (L) goes to the main switch; neutral (N) goes to the terminal block.
- Main switch: 16 A rated replacement (the original switch failed). A 10 A switch is underrated for the 13.5 A draw and must NOT be substituted.
- Switch output splits two ways:
  - Fan motor → neutral bus. The fan runs whenever the switch is on.
  - Thermostat → heating element (~10 Ω) → neutral bus. The thermostat cycles the heating element only; it does not control the fan.
- Terminal block: 4 terminals, commoned into one neutral bus. Three neutrals land on it: wall neutral in, fan neutral, heating-element neutral. One terminal is spare. (Bare-block continuity across all four terminals after sanding confirms the block is commoned.)

## Task

Draw this circuit in KiCad as one schematic and export a one-page PDF, both pushed to branch `hermes/0173-proofer-wiring` under `docs/proofer-j7239/`:

- `docs/proofer-j7239/j7239-wiring.kicad_sch`
- `docs/proofer-j7239/j7239-wiring.pdf` (one page, printable, black-and-white friendly)

## Success criteria

1. The schematic matches the bench-proven wiring above exactly: switch, fan, thermostat, heating element, neutral-bus terminal block with three neutrals and one spare.
2. Every part is labeled in plain words on the sheet (MAIN SWITCH, FAN, THERMOSTAT, HEATING ELEMENT, TERMINAL BLOCK — NEUTRAL BUS), alongside the schematic symbols, so a non-electrician can follow it.
3. The sheet carries a short "How it works" note in plain language: fan runs whenever the switch is on; thermostat turns the heating element on and off only; at 200°F the element heats, below 60°F it cuts off.
4. The sheet carries a "Replacing parts" note: main switch must be 16 A or higher (never 10 A); heating element ~10 Ω (1500 W at 120 V).
5. The PDF is one page and prints cleanly in black and white.

## Constraints

- Push only to branch `hermes/0173-proofer-wiring`. Do NOT merge to main.
- Draw it in KiCad. Do not substitute an AI-generated image diagram.
- Label every wire by function (hot / neutral / switched hot). Stephen did not report wire colors, so do not invent wire colors on the sheet.
- Do not add parts that are not in the circuit above. If any connection is unclear from this spec, say so in the staged reply notes instead of guessing.

## Proof

- branch: `hermes/0173-proofer-wiring`, tip SHA
- files: the two files above, present on the branch
- run: none expected (no workflow covers `docs/**`); confirm live with `gh run list` and state the result.

## Reply format

```
DONE — 0173 proofer wiring diagram
branch: hermes/0173-proofer-wiring @ <sha>
files:
  - docs/proofer-j7239/j7239-wiring.kicad_sch
  - docs/proofer-j7239/j7239-wiring.pdf
checks: <what you verified — PDF renders one page, labels present, wiring matches spec>
notes: <anything unclear, any assumption made, CI run-list result>
cost: <per protocol 1.1.0 cost format>
```

## Reply

```
DONE — 0173 proofer wiring diagram
branch: hermes/0173-proofer-wiring @ f1c48a6fda9e6e3db27bb964a0b0fca8a1cff56b
files:
  - docs/proofer-j7239/j7239-wiring.kicad_sch
  - docs/proofer-j7239/j7239-wiring.pdf
checks: netlist partition matches the bench wiring exactly (5 nets, no merge, no split,
  J1.4 spare unconnected); PDF one page A4 landscape; 0 coloured pixels at 150 dpi
  (black-and-white safe); 87 strings measured from the SVG export, 0 off-page, 0
  overlapping, 0 on a symbol body, 0 upside-down; ERC 1 error + 6 warnings, each explained
  in notes (labelled wall stub instead of a connector part; KiCad 9-vs-10 library mismatch)
notes: KiCad 10 schematic + kicad-cli PDF, no AI image. Ladder layout, four functional net
  labels, plain-word part names, "how it works" and "replacing parts" notes in plain
  language, no wire colours invented. no CI run: no workflow covers docs/** or this branch
  (gh run list --branch hermes/0173-proofer-wiring = 0 rows).
cost: 101 API calls, 64,296 in / 77,616 out / 11,050,368 cache-hit, $0.089365, off-peak.
```
