---
task_id: "0091"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
proof:
  branch: ""
  sha: ""
  run: ""
---

# 0091 — Detection simulation harness: run the v0.1 rules against synthetic traces before firmware

# Context

The consensus detection algorithm exists as a spec, not code:
`docs/wildfire/detection-logic-spec-v0.1-review-notes.md` in pagosacabin/nordtronics.
Section A is the frozen v0.1 rules (12-min packet interval, node rise = PM2.5 ≥ 25 AND
delta-vs-baseline ≥ 15 over two consecutive packets, Watch = ≥2 neighbor nodes confirmed
inside a 20-min window, clear after 30 min below thresholds, ≤25 min claimed worst-case
latency). Section B lists nine known issues with it. Section C defines what the simulation
must measure.

This task builds the pure-software harness. No firmware, no hardware, no app changes.
The v0.1 rules are the **code under test** — implement them exactly as written in Section A,
warts and all. Do NOT "fix" Review Notes #1–#9 in the implementation; the harness exists to
measure them.

# Task

1. Create `python/detection-sim/` (next to `python/ultrasonic-sensors/`):
   - `rules_v01.py` — the Section A rules implemented verbatim: rolling-median baseline
     (last 8 readings / 2 h), two-packet confirmation, 20-min consensus window, 30-min
     auto-clear, 2-hour post-ack suppression, escalation on third node or ≥ 55 µg/m³.
     Pure functions, no I/O, unit-testable.
   - `traces.py` — synthetic multi-node PM2.5 trace generators with known ground-truth
     start times and spatial correlation. Document every synthesis parameter in the file
     header (peak values, ramp rates, which nodes see it, noise model).
   - `harness.py` — feeds traces through `rules_v01`, records per-scenario results.
2. Implement at least these five scenarios (from the spec):
   1. Neighbor burn pile: slow ramp, limited spatial extent (the baseline-chasing case).
   2. Road/agricultural dust gust: short, high-amplitude, single node, poorly correlated.
   3. BBQ / cooking plume directly under ONE node, sustained, high concentration
      (expected outcome per Review Note #5: **no Watch** — assert this, don't just observe it).
   4. Wildfire smoke plume crossing the property: multi-node, sustained rise.
   5. Diurnal background + sensor noise only (expected: zero Watches).
3. For each scenario report: detection delay (first ground-truth rise → Watch raised),
   false positives, false negatives, nodes involved, severity path taken. Additionally, per
   Section C, explicitly report:
   - the **true worst-case latency path** (staggered two-node confirmation — Review Note #1),
   - whether the burn-pile case is detected or missed via baseline chasing (Note #2),
   - flap / re-arm count after auto-clear (Note #4),
   - the asserted no-Watch on the single-node extreme case (Note #5).
4. Write `docs/wildfire/detection-sim-results-v0.1.md`: per-scenario table, the four Section C
   measurements, and a recommendation list for v0.2 numbers (which placeholders the data
   suggest changing, with the measured evidence for each). Recommendations only — do not
   write v0.2.

# Success criteria

1. `rules_v01.py` matches Section A rule-for-rule (quote the section letter/number above each
   function so a reviewer can check the transcription).
2. All five scenarios run end-to-end from one command and produce the Section C measurements.
3. The four explicit assertions (worst-case latency, burn-pile outcome, flap count, single-node
   no-Watch) appear as PASS/FAIL lines in the harness output, not buried in prose.
4. Results report committed at `docs/wildfire/detection-sim-results-v0.1.md` with a v0.2
   recommendation list grounded in the measured numbers.
5. No firmware, app, or backend files touched.

# Constraints

- Pure Python 3, standard library only. No numpy/pandas unless already vendored — keep it
  dependency-free so it runs anywhere.
- Keep it cheap: DeepSeek Flash, minimal reasoning. This is a test rig, not a product.
- The v0.1 rules are frozen. If a rule looks wrong while implementing it, implement it as
  written and note the suspicion in the results report — the sim is how we find out.
- If a scenario can't be synthesized honestly with the tools at hand, say which one and why
  rather than faking the trace.

# Proof

Branch name and SHA on origin. The staged reply is the deliverable; the results report path
in the repo is part of the proof. Paste the harness's PASS/FAIL summary lines verbatim.

# Reply format

```yaml
branch: "<branch name>"
sha: "<origin SHA>"
harness: "<one-command run + PASS/FAIL summary lines, verbatim>"
scenarios: "<5 scenarios — each: detected/missed, delay, false positives>"
section_c: "<worst-case latency measured; burn-pile outcome; flap count; single-node assertion>"
v02_recommendations: "<which placeholders the data suggest changing, with evidence>"
notes: "<anything Stephen should know>"
```
