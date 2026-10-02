# Nordtronics Wildfire Companion — Detection Logic Spec v0.1 + Review Notes

**Status:** Draft for simulation · 2026-10-02 · Not yet field-validated

Transcribed from the reviewed PDF into the repo so the simulation task has a readable,
versioned input. Section A is the frozen v0.1 rules (code under test — do not edit to
"fix" the review notes). Section B is the severity-ordered punch list. Section C is the
simulation acceptance criteria.

---

## A. Original v0.1 Rules (unchanged — for simulation)

### A.1 Operating Assumptions

- Nominal packet interval: **12 minutes** (target duty cycle that protects deep-sleep battery life).
- Nodes report at least: PM2.5 (µg/m³), temperature (°C), humidity (%RH), battery voltage,
  and a monotonic packet sequence or timestamp.
- Base station has reliable local time and can compute time deltas between packets from
  different nodes.
- "Nearby" is initially defined statically by property membership and a configurable
  neighbor list (or simple distance threshold if GPS is later added). Wind-aware correlation
  is out of scope for v0.1.
- Worst-case end-to-end detection latency target: ≤ **25 minutes** from the first significant
  rise at any participating node. (See Review Note #1 — this claim does not hold under the rules.)

### A.2 Node-Level Rise Event

A single node generates a **Node Rise Event** when both of the following are true on a new packet:

1. **Absolute floor**: current PM2.5 ≥ 25 µg/m³
2. **Relative rise**: current PM2.5 − baseline ≥ 15 µg/m³

where **baseline** is the rolling median of the most recent 8 valid readings (or all readings
in the last 2 hours if fewer than 8 exist). The rise must be observed on at least **two
consecutive packets** before the Node Rise Event is considered confirmed.

Rationale for placeholders: 25 µg/m³ sits above typical clean-air background and many EPA
"Good" AQI breakpoints while remaining sensitive enough for early smoke. A 15 µg/m³ delta
filters minor sensor noise and slow diurnal drift. Two consecutive packets reduce
single-packet spikes (insects, dust gusts, brief cooking plumes).

### A.3 Network Consensus (Watch / Alert)

A network-level **Watch** is raised only when all of the following hold:

- At least **two distinct nodes** that are defined as neighbors each have a confirmed Node
  Rise Event.
- The timestamps of those confirmed events fall inside a **20-minute sliding window**.
- Both nodes have reported at least one packet in the last 30 minutes (i.e. they are
  considered live).

Severity escalates from Watch to a higher severity (future: Warning / Critical) if either:

- A third neighbor node also generates a confirmed Node Rise Event inside the same window, or
- Any participating node reports PM2.5 ≥ 55 µg/m³ (hard absolute threshold).

### A.4 Clearing

- **Automatic clear**: all nodes that contributed to the current Watch have remained below
  their individual rise thresholds for a continuous period of **30 minutes**.
- **Manual acknowledge**: an operator acknowledges the event in the app. This suppresses
  re-alerting for the same set of nodes for **2 hours**, after which the normal rules resume.
- A new Watch can still be raised during the suppress window if a previously uninvolved
  neighbor joins or a hard threshold (≥ 55 µg/m³) is crossed.

### A.5 Latency Budget Table (as published in v0.1)

| Stage | Budget | Notes |
|---|---|---|
| Node sample interval | 12 min nominal | Deep-sleep / solar budget |
| Consecutive-packet confirmation | +12 min | Two packets required |
| Inter-node window | 20 min | Allows staggered reporting |
| Base-station + app delivery | ≤ 1 min | Local LoRa + local API assumed |
| Claimed worst-case | ≤ 25 min | See Review Note #1 |

---

## B. Review Notes — Severity Ordered (must resolve before or during simulation)

The simulation harness should **measure** the impact of #1, #2 and #4 rather than treat them
as settled design decisions.

### 1. Latency budget does not add up (Critical)

Claimed worst-case ≤ 25 min from first rise to Watch is inconsistent with the rules themselves.
Walk-through: Node A rises just after a packet → first showing at +12 min, confirmed at
+24 min. Node B's confirmed event can land up to 20 min later (the consensus window) →
+44 min, plus delivery. True worst case under the written rules is approximately **45 minutes**.
The ≤ 25 min figure only holds when both nodes sample the plume on nearly the same cycle.
**Action:** Either relabel ≤ 25 min as "nominal / best-aligned" and publish ~45 min as honest
worst-case, or tighten the confirmation requirement / window so the arithmetic matches the
claim. This is the headline performance number; it must be honest.

### 2. Baseline chases slow ramps (Critical)

Baseline is the rolling median of the last 8 readings (~96 min). On a slow incursion (the
burn-pile test case) elevated readings enter the window, the median climbs, and
(current − baseline) ≥ 15 may never fire even while absolute values climb past 25 µg/m³.
Classic adaptive-baseline failure.
**Action:** Simulation must explicitly assert the burn-pile case. Fix options for v0.2: freeze
the baseline once a rise is first suspected, or take the delta against the minimum of recent
medians.

### 3. "Watch" means two different things (High)

In the current app, Node 02 shows "Watch · awaiting confirmation" for a single elevated node.
In this spec, Watch is the network-level event that requires two confirmed nodes. Same word,
different thresholds — firmware and app will talk past each other.
**Action:** Rename one. Recommendation: keep network-level "Watch"; change the single-node
chip to "Elevated".

### 4. Clear / re-arm can flap (High)

Clear requires 30 min below thresholds, but a contaminated baseline (see #2) can make the rise
condition clear early while smoke persists, then re-trigger as the baseline decays. The 2-hour
suppression applies only after manual acknowledge, not after auto-clear.
**Action:** Add a post-clear re-arm cooldown, or the user will see Watch flickering. Measure
flap rate in simulation.

### 5. Single node at extreme concentration produces silence (High)

Per the letter of the rules, a BBQ or point source directly under one node with no neighbor
confirmation raises nothing — no Watch, no escalation ("participating" implies the node is
already inside an event). This is probably the intended consensus philosophy, but it must be
stated outright.
**Action:** Make the BBQ / single-node high-concentration test case assert "no Watch" as the
expected outcome, not merely list it.

### 6. Fresh nodes are blind for ~2 hours (Medium)

With only 1–2 readings the baseline ≈ current value, so no relative rise can ever trigger.
Fail-silent is the safer direction, but it is a known limitation: a node deployed at noon will
not catch a 13:00 fire.
**Action:** Document as known limitation; consider a temporary absolute-only mode for the
first N packets if product requirements demand it.

### 7. PMS5003 warmup not in the sample path (Medium)

Bench notes indicate ~30 s after power before trustworthy readings. If the detection reading
is the first sample after wake, the two-packet confirmation is comparing garbage to garbage.
**Action:** One line in the spec: detection logic uses only post-warmup readings.

### 8. Taxonomy drift with the app contract (Medium)

The existing app / API contract uses alert type "watch" with severity "watch". This spec speaks
of escalation to future "Warning / Critical".
**Action:** Align severity vocabulary before any screen or firmware work so both sides emit and
display the same terms.

### 9. PDF character encoding (Cosmetic)

The previous PDF export rendered ≤ / ≥ as incorrect glyphs because of font embedding. This
transcription uses the correct characters.

---

## C. Recommended Next Step

Proceed with the pure-software simulation harness. The harness treats the v0.1 rules (Section A)
as the code under test and reports, for each scenario:

- Detection delay (including the true worst-case path identified in Note #1)
- Whether the burn-pile case is detected or missed because of baseline chasing (Note #2)
- Flap / re-arm behavior after auto-clear (Note #4)
- Explicit assertion that single-node extreme events produce no Watch (Note #5)

Once those measurements exist, the numeric thresholds and the latency claim can be revised with
evidence instead of argument. That becomes Detection Logic Spec v0.2.
