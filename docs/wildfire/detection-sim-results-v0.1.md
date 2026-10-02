# Detection Simulation Harness — v0.1 Results

**Task:** 0091 · **Branch:** `hermes/0091-detection-sim-harness` · **Date:** 2026-10-02
**Status:** Report of a pure-software experiment. No firmware, app, backend or hardware touched.

The v0.1 rules in `docs/wildfire/detection-logic-spec-v0.1-review-notes.md` (Section A) are the
**code under test** — transcribed verbatim, Review Notes #1–#9 deliberately *not* fixed. This
document reports what the rules do, measured, not what they should do.

## How to reproduce

```bash
python3 python/detection-sim/harness.py          # exits 0 when all assertions pass
python3 python/detection-sim/harness.py -v       # also dumps every detector event
```

Standard library only, no arguments, no network, deterministic (two consecutive runs produce
byte-identical output; every RNG is seeded from the scenario name and its index).

| File | Role |
|---|---|
| `rules_v01.py` | Section A, clause by clause (each function quotes its clause). Pure, no I/O. |
| `traces.py` | Synthetic multi-node PM2.5 traces with declared ground truth and synthesis parameters. |
| `harness.py` | Feeds traces through the rules, measures, and prints PASS/FAIL assertions. |

Trace model: `pm25(t) = diurnal background (4–8 µg/m³) + Σ plume(t) + Gaussian sensor noise
(σ = 1.5 µg/m³)`, nodes phase-staggered within the 12-minute interval. Ground truth (true rise
time, affected nodes, expected outcome) is declared per scenario in `traces.py`.

## Per-scenario results

| # | Scenario | Expected | Observed | Delay | FP/FN | Nodes confirming | Severity path |
|---|---|---|---|---|---|---|---|
| 1 | `burn_pile_slow_ramp` (Note #2) | none | none | – | 0/0 | – | no events |
| 2 | `dust_gust_single_node` | none | none | – | 0/0 | – | no events |
| 3 | `bbq_single_node_extreme` (Note #5) | none | none | – | 0/0 | N3 | no events |
| 4 | `wildfire_plume_multi_node` | WATCH | WATCH | 27 min | 0/0 | N1,N2,N3 | Watch@99 → AutoClear@165 |
| 5 | `diurnal_background_noise` | none | none | – | 0/0 | – | no events |
| 6 | `staggered_worst_case` (Note #1) | WATCH | WATCH | 43 min | 0/0 | A,B | Watch@44 → Escalated@44 → AutoClear@92 |
| 7 | `flap_clear_rearm` (Note #4) | WATCH | WATCH | 27 min | 0/0 | A,B | Watch@51 → Watch@267 → AutoClear@108 → AutoClear@336 |
| 8 | `fresh_node_blind_window` (Note #6) | none | none | – | 0/0 | A | no events |
| 9 | `ack_suppression` (A.4 ack path) | WATCH ×1 | WATCH ×1 | 27 min | 0/0 | A,B | Watch@51 (second plume suppressed) |
| 10 | `three_node_escalation` (A.3 escalation) | WATCH | WATCH | 12 min | 0/0 | P1,P2,P3 | Watch@36 → Escalated@36 → AutoClear@84 |

Times are minutes from trace start. "Alert raised" means the network Watch of A.3; delay is
measured from the first ground-truth rise at any node. **No false positive and no false negative
in any scenario** (10 scenarios; the two `none`-expected scenarios that could plausibly have
produced one — the one-packet dust gust and 24 h of background noise — produced zero).

## Section C measurements

### 1. True worst-case latency (Review Note #1)

**Measured: 43 minutes from first rise to Watch**, on the phase-staggered two-node path
(`staggered_worst_case`): node A confirms at t=24, node B — whose packets are offset 8 minutes
and whose plume arrives at t=21 — confirms at t=44, exactly 20 minutes later, i.e. the last
instant A.3's window permits. Watch fires at t=44.

For contrast, the *nominal / best-aligned* case is **12 minutes** (`three_node_escalation`, all
nodes phase-aligned) and the ordinary staggered multi-node case is **27 minutes**
(`wildfire_plume_multi_node`).

The spec's ≤ 25 min (A.1, A.5) is therefore **18 minutes optimistic for the worst case and
~13 minutes optimistic as a general figure**; Review Note #1's ≈ 45 min arithmetic is confirmed
(our 43 min differs by the 1-minute phase offset chosen for the first rise). The ≤ 25 min figure
holds only when both nodes sample the plume on nearly the same cycle.

### 2. Burn-pile outcome (Review Note #2) — **MISSED**

Node N1's trace ramps linearly at 2.0 µg/m³ per packet (~10 µg/m³/h). Result:

```
N1 peaked at 64.1 ug/m3 (absolute floor 25 exceeded)
   but its largest relative rise was 13.1 ug/m3 against the 15 ug/m3 requirement
   -> the rolling median (last 8 readings / ~96 min) chased the ramp.
   No Node Rise Event, no Watch.
```

The absolute floor was satisfied for hours; the *relative* test never fired, because A.2.2
compares against a baseline that is itself climbing. Baseline chasing is confirmed, not
theoretical.

A noise-free single-node sweep makes the mechanism quantitative — **the relative test is a rate
gate**:

| Rise rate | Confirmed? | Peak PM2.5 reached |
|---|---|---|
| 2.5 µg/m³/h | no | 23.0 |
| 5.0 µg/m³/h | no | 38.0 |
| 10.0 µg/m³/h | no | 68.0 |
| 15.0 µg/m³/h | **no** | **98.0** |
| 17.5 µg/m³/h | yes | 113.0 |
| 20.0 µg/m³/h | yes | 128.0 |

A single clean node cannot confirm a linear rise slower than **3.5 µg/m³ per packet
(17.5 µg/m³/h)** no matter how high the absolute concentration climbs — at 15 µg/m³/h it sailed
past 98 µg/m³ with zero confirmations.

### 3. Flap / re-arm after auto-clear (Review Note #4)

**Measured: 1 re-arm after 2 auto-clears; both clears fired while the ground-truth plume was
still on** (`flap_clear_rearm`: Watch@51 → AutoClear@108 → Watch@267 → AutoClear@336, with N1/N2
plumes continuously lit from t=24 to t=684 apart from a designed 42-minute lull).

The design intends the first interval to be a clean "smoke, lull, smoke" sequence, and the lull
is indeed longer than A.4's 30 minutes. But the *first* clear happens 84 minutes into the first
plume (t=108, plume lit from t=24), while every node is still reading elevated PM2.5, because the
baseline absorbed the smoke, `current − baseline` fell under 15 µg/m³, and A.4 counts 30 minutes
"below thresholds". A.4's 2-hour suppression applies only to manual acknowledge, so the
auto-clear re-arms and the user sees the Watch disappear and come back with no change on the
ground.

The same failure reproduces on the design's flagship scenario: `wildfire_plume_multi_node` holds
its plume for 900 uninterrupted minutes, yet its Watch **auto-cleared at t=165 and never
re-armed**. A Watch that clears itself 66 minutes into a wildfire is the single most serious
result in this run.

### 4. Single node at extreme concentration (Review Note #5) — **asserted, no Watch**

The BBQ point source under N3 alone holds ~85 µg/m³ for 2 hours.

```
N3's Node Rise Event IS confirmed (confirmed_at=138)
   yet no Watch is raised -- A.3 needs two neighbours.
```

The assertion is shown to be non-vacuous: re-running the identical trace with a one-node
consensus rule raises a Watch at t=138, so "no Watch" is produced by A.3's two-neighbour
requirement and not by a trace that never gets close. This matches the spec's consensus
philosophy, and it is now stated outright rather than implied.

## Additional findings

- **Third-node escalation is unreachable at 12-minute cadence unless nodes are phase-aligned.**
  `three_node_escalation` (aligned) escalates through the "third neighbor node" branch at t=36.
  In `wildfire_plume_multi_node` (12-minute stagger) the three confirmations land at t=84, 99 and
  114 — a 30-minute span that can never fit A.3's 20-minute window, so the third-node branch
  cannot fire and escalation falls back to the ≥ 55 µg/m³ branch (which is noise-dependent: with
  seed 1003 N1's packet read 54.x µg/m³ and no escalation occurred at all). A.3's two escalation
  criteria are not interchangeable in practice.
- **The fresh-node blind window is not a fixed ~2 hours.** A node deployed into *clean* air gets
  a usable baseline after its second reading (~24 minutes) via A.2's "all readings in the last 2
  hours" fallback. A node deployed *into smoke* (`fresh_node_blind_window`: C powers up at t=0 in
  air already at ~46 µg/m³) locks its baseline to the smoke and never confirms a rise for the
  whole event — and because only the pre-existing node A confirms, A.3 can never reach a Watch:
  the miss is silent. The ~2 h figure in Note #6 understates the smoke case and overstates the
  clean case.
- **The two-packet confirmation filter does real work.** A single-packet 110 µg/m³ dust gust on
  one node produced 0 confirmed rises and 0 Watches — Review Note #5's "insects, dust gusts,
  brief cooking plumes" rationale is validated as written.
- **The false-positive floor is clean at these backgrounds.** 24 hours × 4 nodes of diurnal
  background plus 1.5 µg/m³ Gaussian noise produced 0 confirmed rises and 0 Watches. The 25 µg/m³
  absolute floor and the two-packet rule are not implicated by this data.
- **Manual acknowledge works as specified** (`ack_suppression`): acknowledging the first Watch
  suppressed a second, identical plume that arrived 33 minutes after the acknowledgement (well
  inside the 120-minute window) with the same node set — 1 Watch raised where 2 would otherwise
  fire.

## v0.2 recommendations (placeholders, with the measured evidence)

Recommendations only; v0.2 is not written here.

| Placeholder (A.2–A.4) | Recommendation | Evidence |
|---|---|---|
| `15 µg/m³` relative delta vs rolling median | **Change the comparison, not the number.** Compare against a baseline *frozen* when a rise is first suspected, or against `min(median of last 8, median of last 32)`. A.2.2 is currently a rate gate that ignores absolute level. | Burn pile missed at 64.1 µg/m³ peak, max delta 13.1; single-node sweep: nothing slower than 17.5 µg/m³/h ever confirms, and 15 µg/m³/h reached 98 µg/m³ unconfirmed. |
| `8-reading` rolling median | **Shorten, or make it the min of two windows.** 96 minutes of memory is what lets a sustained plume dissolve its own Watch. | `wildfire_plume_multi_node` Watch auto-cleared at t=165 with the plume still lit; 2 of 2 clears in `flap_clear_rearm` fired inside a live plume. |
| `30-minute` auto-clear | **Add a post-clear re-arm cooldown** (the same 2 hours A.4 already applies after manual ack), or require the clear to persist on absolute level rather than the relative test. | 1 re-arm in `flap_clear_rearm`; both clears occurred while PM2.5 was still elevated. |
| `20-minute` consensus window (escalation) | **Widen to ≥ 36 min for the third-node branch, or count distinct nodes across the whole Watch instead of inside the window.** | Aligned triple escalates via third node; staggered wildfire triple confirms 30 min apart and cannot. |
| `≤ 25 min` claimed worst-case latency (A.1, A.5) | **Relabel as "nominal / best-aligned (~12–24 min)" and publish ~45 min as the honest worst case** — or tighten the confirmation/window so the arithmetic matches. | 43 min measured on the staggered two-node path; 27 min typical; 12 min best-aligned. |
| `2 consecutive packets` confirmation | **Keep.** No evidence supports relaxing it. | 0 false positives on a 110 µg/m³ single-packet gust; 0 across 24 h of noise. |
| `25 µg/m³` absolute floor | **Keep at these backgrounds.** Not implicated by this run. | 0 false positives over 96 node-hours of synthetic clean air (σ = 1.5 µg/m³). |
| Fresh-node bootstrap (Note #6) | **Document the real window** (2 packets in clean air; the whole event plus ~96 min if deployed into smoke) and consider an absolute-only mode for the first N packets. | `fresh_node_blind_window`: C never confirms while A confirms at t=12; no Watch. |

## What this simulation does not prove

- The traces are synthetic. Real sensor artefacts (PMS5003 warmup per Note #7, humidity
  response, drift) are not modelled, and the noise is Gaussian and stationary.
- Wind-aware correlation is out of scope for v0.1 and is not modelled; every node in a scenario
  is a mutual neighbour (`A.1`'s static neighbour list).
- Absolute concentrations and rates are defensible as *shapes*, not as validated field values.
  These results bound the rules' behaviour; they do not replace field data.

## Verbatim harness output

```
====================================================================================================
DETECTION SIM v0.1 -- v0.1 rules (spec Section A) against synthetic traces
====================================================================================================
scenario                   expect   observed  delay    fp/fn    nodes            severity path
----------------------------------------------------------------------------------------------------
burn_pile_slow_ramp        none     none      -        0/0      -                no events
dust_gust_single_node      none     none      -        0/0      -                no events
bbq_single_node_extreme    none     none      -        0/0      N3               no events
wildfire_plume_multi_node  WATCH    WATCH     27 min   0/0      N1,N2,N3         Watch@99 -> AutoClear@165
diurnal_background_noise   none     none      -        0/0      -                no events
staggered_worst_case       WATCH    WATCH     43 min   0/0      A,B              Watch@44 -> Escalated@44 -> AutoClear@92
flap_clear_rearm           WATCH    WATCH     27 min   0/0      A,B              Watch@51 -> Watch@267 -> AutoClear@108 -> AutoClear@336
fresh_node_blind_window    none     none      -        0/0      A                no events
ack_suppression            WATCH    WATCH     27 min   0/0      A,B              Watch@51
three_node_escalation      WATCH    WATCH     12 min   0/0      P1,P2,P3         Watch@36 -> Escalated@36 -> AutoClear@84
----------------------------------------------------------------------------------------------------

THRESHOLD SENSITIVITY SWEEP (single clean node, linear ramp, noise-free)
----------------------------------------------------------------------------------------------------
per packet     per hour         confirmed? peak ug/m3   max rise vs baseline
0.5 ug/m3      2.5 ug/m3        no         23.0         2.7
1.0 ug/m3      5.0 ug/m3        no         38.0         4.9
1.5 ug/m3      7.5 ug/m3        no         53.0         7.2
2.0 ug/m3      10.0 ug/m3       no         68.0         9.4
2.5 ug/m3      12.5 ug/m3       no         83.0         11.7
3.0 ug/m3      15.0 ug/m3       no         98.0         13.9
3.5 ug/m3      17.5 ug/m3       yes        113.0        16.2
4.0 ug/m3      20.0 ug/m3       yes        128.0        18.4
5.0 ug/m3      25.0 ug/m3       yes        158.0        22.9
6.0 ug/m3      30.0 ug/m3       yes        188.0        27.4
-> slowest confirmable linear rise: 3.5 ug/m3 per packet (17.5 ug/m3/h); anything slower is invisible to A.2.2 no matter how high the absolute concentration climbs.
----------------------------------------------------------------------------------------------------
ASSERTIONS
----------------------------------------------------------------------------------------------------
PASS worst_case_latency         staggered two-node path measured 43 min from first rise to Watch
PASS burn_pile_outcome          burn pile MISSED: N1 peaked at 64.1 ug/m3 (absolute floor 25
PASS flap_rearm_count           flap count: 1 re-arm(s) after 2 auto-clear(s) (2 Watch event(s)
PASS premature_clear_in_smoke   Review Note #4 confirmed with a physical cross-check: 2 of 2
PASS sustained_plume_self_clear the multi-node plume runs uninterrupted for 900 min, yet its Watch
PASS single_node_no_watch       BBQ point source under N3 alone: N3's Node Rise Event IS confirmed
PASS dust_gust_rejected         single-packet 110 ug/m3 spike on N2 produced 0 confirmed rise
PASS wildfire_plume_detected    multi-node plume: Watch at t=99 (delay 27 min from first rise);
PASS third_node_escalation      phase-aligned triple: Watch at t=36 on 2 nodes (P1,P2), then
PASS background_zero_watches    24 h of diurnal background + 1.5 ug/m3 Gaussian noise across 4
PASS fresh_node_blind           Review Note #6 reproduced on the sample path: node C is deployed
PASS ack_suppression_holds      manual acknowledge at the first Watch suppressed the second
----------------------------------------------------------------------------------------------------
SUMMARY: 12/12 PASS, 0 FAIL
```

(The assertion lines are truncated by the harness's fixed column width; run the harness for the
full sentence of each.)
