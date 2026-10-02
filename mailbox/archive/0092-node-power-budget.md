---
task_id: "0092"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 24h
proof:
  branch: "hermes/0092-node-power-budget"
  sha: "2f3a7ee9359e90923c8abbe4b4a5cd6f2b952a01"
  run: "none"
  files:
    - docs/wildfire/node-power-budget-v1.md
notes: "no workflow run on the deliverable branch — earned by enumeration, not assumed: .github/workflows/ holds exactly 3 files, android-build.yml triggers only on branch android-toolchain-setup, platformio.yml only on paths firmware/tank-monitor/** or firmware/node-v1/** (or its own file), website-check.yml only on branches main/hermes/0068-site-email-refresh/hermes/0070-site-rewrite; this commit touches docs/wildfire/ on a new branch, so zero runs start, and gh run list --branch hermes/0092-node-power-budget is empty. ntfy receipt: no topic applies (docs-only, no compiled artifact). No firmware/app/backend/hardware change (constraint 5). MAIN CARRIES 0 COPIES of docs/wildfire/node-power-budget-v1.md — the deliverable lives only on the task branch, so verify it with git show 2f3a7ee:docs/wildfire/node-power-budget-v1.md (sha256 cd75a895dea571411dc154fe1815c777cb4517c4e3e23602833054b9a3c9ef1b), not from a main worktree. Three errors found and corrected in the superseded draft; one mechanism in the task's own framing corrected; full detail in the Reply section."

---


# 0092 — Node power budget: prove the 12-minute cadence closes on paper (datasheets only)

# Context

The whole wildfire design hangs off a 12-minute node packet cadence: the detection spec's
latency claims (`docs/wildfire/detection-logic-spec-v0.1-review-notes.md`, Section A.5),
the 20-minute consensus window, and the sim scenarios in 0091 all assume it. That cadence is
currently asserted ("protects deep-sleep battery life"), not calculated. This task does the
mAh math from datasheets before any hardware is bought or firmware written.

Relevant prior art in the repo:
- Node schematic Rev C (`hardware/wildfire-node-v1/`, task 0079 branch) — the power path,
  what's always-on, the temp-gate charging switch block (Claude's clean-sheet design in
  `hardware/solar-gate-v2/`).
- Bench notes: PMS5003 needs ~30 s after power before trustworthy readings; BME680 heater
  pulses sag weak 3.3 V rails; Heltec LoRa 32 V4 (ESP32-S3 + SX1262) is the MCU/radio.

# Task

1. Build the per-wake energy table for ONE 12-minute cycle, every number traced to a
   datasheet (cite document + page/section) or labeled as an explicit assumption:
   - PMS5003: active current (fan + laser) × warmup + sample duration. Bench note says
     ~30 s to trustworthy; verify against the Plantower datasheet and state what you use.
     IMPORTANT: the PMS5003 is a 5 V part on a 3.7 V cell — do the real boost math
     (5/3.7 ÷ converter efficiency), not a blanket 15% allowance. A blanket factor
     understates this line by ~40%. (The latest draft still shows 92 mAh/day from the
     blanket factor — correct it.)
   - BME680: heater-profile current × duration per forced-mode reading (gas + T/RH/P).
     The first-cut budget omitted this sensor entirely — include it even if it lands at
     1–3 mAh/day. A budget with a missing sensor is a budget you can't trust.
   - AS3935 lightning sensor: listening-mode current × 24 h continuous (70 µA → ~1.7 mAh/day),
     PLUS the interrupt-service cost. Each lightning/disturber event wakes the MCU via the
     INT pin for register readout and possible TX — model per-event cost × events/day for a
     quiet day vs. an active storm day. The AS3935 is famous for disturbers; do not assume
     zero events.
   - Heltec V4: ESP32-S3 wake + sensor readout + LoRa TX energy per packet. State the
     assumed spreading factor, TX power, and payload size, and compute time-on-air —
     do not guess it. Cite the SX1262 datasheet TX current (≈120 mA @ +22 dBm) against
     any conservative figure you use — label which one the totals use so nobody later
     "finds" phantom margin.
   - Always-on loads from the Rev C schematic: deep-sleep quiescent, temp-gate switch
     quiescent, battery-sense divider, anything else that never sleeps.
2. Daily total at 12-minute cadence (120 wakes/day). Then the sensitivity table: same total
   at 6-min, 12-min, 30-min, and 60-min cadences — this is the input the detection-latency
   tradeoff needs.
3. Battery sizing: days of autonomy with **zero** solar input for the actual cells —
   3.7 V 3000 mAh li-ion, single AND 2P (6000 mAh). Stephen confirmed these are the cells
   and two may be paralleled if needed. State the usable-capacity derating you apply AND a
   cold-weather derating for Pagosa winter nights (li-ion gives back less below freezing) —
   label both, don't bury them.
4. Solar sizing for Pagosa Springs in **December** (worst month): state the peak-sun-hours
   figure you use and its source, apply a snow-cover/dirt derating you defend in one line,
   and compute the panel wattage needed for energy-neutral operation at 12-min cadence.
   The bench panel is **13 W / 5 V** — compare the requirement against it directly and
   state the headroom or the shortfall.
5. Write `docs/wildfire/node-power-budget-v1.md`: the tables, every source citation, every
   assumption labeled, the cadence sensitivity table, and a verdict section.
6. Record as v2 opportunities (not required now): deep-sleeping the MCU through the PMS5003
   fan warmup and waking only to sample + transmit — estimated to roughly halve the
   MCU-active line.

Cross-check: Juno's independent pass over the budget landed at ~220 mAh/day
(~11–12 days single cell, ~23 days 2P, no sun) after correcting the 5 V boost math and
adding the BME680 and AS3935 (~1.7 mAh/day listening + interrupt service). If your total
differs by more than ~20%, reconcile the difference explicitly in the reply notes.

# Success criteria

1. Every current × duration number traces to a cited datasheet section or a labeled
   assumption — no orphan numbers.
2. Verdict is explicit: **does 12-minute cadence close?** If yes, state the margin
   (days of autonomy, December solar headroom). If no, state which cadence does close
   and what the detection-latency cost is (cross-reference the v0.1 latency table).
3. Cadence sensitivity table (6/12/30/60 min) present — the detection spec's next revision
   needs it.
4. Budget document committed at `docs/wildfire/node-power-budget-v1.md`.
5. No firmware, app, backend, or hardware changes.

# Constraints

- Datasheets only. Vendor marketing pages are not sources; link the PDF and section.
- State assumptions where datasheets are silent (e.g. exact PMS5003 warmup); never present
  an assumption as a measured value.
- Keep it cheap: DeepSeek Flash, minimal reasoning. This is arithmetic, not research.
- If a datasheet is unobtainable (e.g. Plantower publishes no CAD and limited docs),
  use the best available source, label the confidence, and move on.

# Proof

Branch name and SHA on origin. The budget document path in the repo is part of the proof.
Paste the per-day mAh totals (6/12/30/60 min) and the verdict verbatim.

# Reply format

```yaml
branch: "<branch name>"
sha: "<origin SHA>"
per_wake_mah: "<12-min cycle total, with the three biggest contributors named>"
per_day_mah: "<6-min: X | 12-min: X | 30-min: X | 60-min: X>"
battery_days: "<days of autonomy, zero solar — single 3.7V/3000mAh cell AND 2P 6000mAh>"
december_solar: "<panel watts needed for energy-neutral December vs the 13 W / 5 V bench panel: headroom or shortfall>"
verdict: "<does 12-min close? margin or the cost of the cadence that does>"
sources: "<datasheets cited>"
notes: "<anything Stephen should know>"
```

---

## Reply

```yaml
branch: "hermes/0092-node-power-budget"
sha: "2f3a7ee9359e90923c8abbe4b4a5cd6f2b952a01"
per_wake_mah: "1.80 mAh per 12-min cycle — PMS5003 warm-up 1.325, PMS5003 sample 0.442, ESP32-S3 active window 0.025 (three biggest; 98.5% of the wake is the particulate sensor)"
per_day_mah: "6-min: 435.2 | 12-min: 219.3 | 30-min: 89.8 | 60-min: 46.6"
battery_days: "10.5 days zero-sun single 3.7V/3000mAh cell (usable 2295 mAh = 3000 x 0.90 usable x 0.85 cold); 20.9 days on 2P (6000 mAh, usable 4590 mAh)"
december_solar: "0.372 W needed for energy-neutral December vs the 13 W / 5 V bench panel -> 35x headroom, no shortfall (3.88 in-plane peak-sun-hours/day from PVGIS 5.2 at 37.269N 107.010W, x0.75 snow/soiling, /0.75 charge path)"
verdict: "Yes — 12-minute cadence closes. 219.3 mAh/day for 10.5 days of zero-sun autonomy on one cell (20.9 days on 2P) and 35x December solar headroom. It closes on the battery side; December solar is not the constraint and never was. The cost of 12 min is latency, not energy: the honest worst case is ~45 min, not the published <=25 min. 30-min cadence would give 25.6 days single-cell for ~81 min worst case."
sources: "Plantower PMS5003 manual V2.3 (active current, 30 s stability requirement); Bosch BME688 BST-BME688-DS000 Table 2 (heater 12 mA @ 320 C); ScioSense AS3935 datasheet v2 (70 uA listening); Semtech SX1261/2 DS_SX1261-2 V1.1 (118 mA @ +22 dBm); TI TPS61023 (Iq 20 uA); Adafruit MiniBoost 4654 (the Rev C module); Espressif ESP32-S3 datasheet Table 21; PVGIS 5.2 December in-plane insolation"
notes: "Full document: docs/wildfire/node-power-budget-v1.md on the branch (339 lines). Every number cited or labelled as an assumption (19-item assumption register, A1-A19). Cross-check vs your ~220 mAh/day: mine is 219.3, within 0.3% — no reconciliation owed on the total, but the composition is starker than the note implies: 97% of the daily energy is the PMS5003 and the whole MCU+radio+always-on composite is ~7.3 mAh/day. Your 11-12 / 23 days vs my 10.5 / 20.9: the 9% gap is the two deratings made explicit (0.90 usable x 0.85 cold) rather than folded into one. Five items you should know are in the document itself: (1) the superseded draft's 92 mAh/day is reproduced exactly by 80 mA x 30 s x 120 x (5/3.7) x 0.85, i.e. three errors — the efficiency applied as a multiplier instead of a divisor, the 10 s sample window omitted, and the bench's 80 mA impression used instead of the datasheet's <=100 mA; corrected it is 211.98 mAh/day, 2.3x the draft. (2) The BME688 does NOT land at 1-3 mAh/day at the datasheet heater duty: 12 mA for 150 ms x 120 wakes = 0.05 mAh/day. It only reaches 1-3 mAh/day if the heater is held on for seconds per wake or Vext is left on through the whole active window — both are now quantified in the sensitivity table, so the sensor is present in the budget either way. (3) The task's '~40% understatement for the blanket factor' is confirmed in kind: treating the 5 V-rail figure (133 mAh/day) as cell current understates the correct 212 by 37%. (4) ESP32-S3 datasheet Table 21's 7 uA deep-sleep figure carries a footnote excluding parts with SiP flash/PSRAM, and this node's module is the S3R2 with 2 MB in-package PSRAM — so 7 uA is used only as a floor and 30 uA board-level is the assumption; the failure mode (UART bridge not in reset, observed at 1.4 mA on sibling Heltec boards) costs 33 mAh/day, is 15% of the total, and is the single highest-value thing to measure on the bench. (5) Interface observation, not a budget item: R2 on the Rev C is a 4.7k pull-down on the MiniBoost EN pin, which only DISABLES the boost — the MCU has to actively drive EN high or the 5V2 rail never comes up. Worth a check when firmware is written. Also flagged: Rev C does not enumerate a battery-sense divider or the solar temp-gate quiescent (A13/A14, 0.92 mAh/day combined, both assumptions; the 20 uA temp-gate figure needs the solar-gate-v2 board to confirm). Latency cross-reference for the spec revision: honest worst case = 2T + 20 min window + 1 min delivery, so 33/45/81/141 min at 6/12/30/60 min — the <=25 min claim does not hold at any cadence on that list, which agrees with Review Note #1. Recommended cadence: keep 12 min."
```

### What was checked, and how

- **Arithmetic**: every table and every sensitivity row was produced by a script over the
  assumption register, not by hand. The cross-variable rows were re-derived after the document was
  written and three of them were off in the first draft (6-min total, the 90 %-efficiency row, the
  UART-bridge failure row); they are corrected in the committed version. Query the numbers
  yourself from the register in §3 — the totals are reproducible from it without the script.
- **Sources**: all eight document URLs in §2 were fetched (HTTP 200, `application/pdf`) from this
  host except the two Mouser-hosted SX1262 mirrors, which this host cannot reach; the SX1262 value
  is therefore corroborated by two independent documents (Semtech DS_SX1261-2 V1.1 and the
  eRIC-SX1262-HCI module datasheet) rather than by one reachable link, and that is stated in the
  source table itself. The PVGIS December figure was fetched live from the PVGIS API at a
  URL that is printed in §2 so it can be re-run.
- **No CI run is claimed.** The deliverable branch started zero workflow runs; the enumeration is
  in the front-matter `notes`. Separately, the *mailbox pickup* commit pushed to main did start
  Website Check run 37039497388 (conclusion: success) — that is a main-push run, not a run at the
  deliverable's branch tip, and it is not offered as proof of the document.
- **Nothing outside `docs/` was touched.** One file added: `docs/wildfire/node-power-budget-v1.md`.
  Constraint 5 (no firmware, app, backend or hardware change) holds.
- **A re-file would not be legitimate here**: this budget was produced by this run, not by an
  earlier tick. There was no pre-existing branch for 0092 and no prior artifact to inspect.

### Corrections I made to the task's own framing (declared, not silently applied)

1. The task names **BME680**; the 2026-10-02 decision recorded in task 0093 makes **BME688**
   authoritative (the 680 is EOL). The budget uses the BME688 and cites BST-BME688-DS000 Table 2,
   with the BME680 DS001 figure listed alongside to show the heater current is the family value.
2. The task expects the **BME680 line at 1-3 mAh/day**. At the datasheet's heater duty it is
   **0.05 mAh/day**. Both numbers are now in the document, with the condition that produces each
   (heater plateau length / how long `Vext` is held on) rather than one being quietly preferred.
3. The task says the PMS5003 boost error is a "blanket 15 % allowance"; the superseded draft's
   *specific* 92 mAh/day figure implies two further errors beyond the blanket factor. All three are
   itemised with the arithmetic that reproduces 92 exactly.
4. The task's "AS3935 ... 70 µA → ~1.7 mAh/day" is confirmed (1.68) and the interrupt service is
   costed separately for a quiet day, a storm day, and with disturbers masked.
