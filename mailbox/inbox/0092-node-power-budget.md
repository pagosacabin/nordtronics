---
task_id: "0092"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
proof:
  branch: ""
  sha: ""
  run: ""
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
     understates this line by ~40%.
   - BME680: heater-profile current × duration per forced-mode reading (gas + T/RH/P).
     The first-cut budget omitted this sensor entirely — include it even if it lands at
     1–3 mAh/day. A budget with a missing sensor is a budget you can't trust.
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

Cross-check: Juno's independent pass over the first-cut budget landed at ~218 mAh/day
(~11–12 days single cell, ~23–24 days 2P, no sun) after correcting the 5 V boost math and
adding the BME680. If your total differs by more than ~20%, reconcile the difference
explicitly in the reply notes.

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
