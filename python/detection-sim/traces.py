"""traces.py -- synthetic multi-node PM2.5 trace generators with known ground truth.

Every synthesis parameter of every scenario is declared here, in code, so the
numbers in `docs/wildfire/detection-sim-results-v0.1.md` can be reproduced and
audited. Nothing in this file knows about the detection rules; it only produces
(ground truth, packets).

Synthesis model
---------------
Concentration at time t for node n is

    pm25(t) = background_n(t) + sum(plume_k(t)) + noise_n(t)

* `background_n(t)`: diurnal sinusoid around a clean-air floor, peak-to-trough
  4 ug/m3 in every scenario unless stated (mid-latitude clean-air background).
* `plume_k(t)`: one of three shapes, each with explicit rise/plateau/fall edges
  (see `Plume`).
* `noise_n(t)`: Gaussian, per-node sigma, drawn from a per-node seeded RNG so a
  scenario is byte-reproducible from its seed.

Reporting convention: all times are **minutes from trace start**. Node packet
times are `phase + k * interval`, so nodes are deliberately phase-staggered --
A.1's "Allows staggered reporting" 20-minute consensus window exists because of
exactly that stagger (see the `staggered_worst_case` scenario).
"""

from __future__ import annotations

import math
import random
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Sequence, Tuple

PACKET_INTERVAL_MIN = 12          # A.1
CLEAN_AIR_UGM3 = 6.0              # clean-air floor for the synthetic background
DIURNAL_AMPLITUDE_UGM3 = 2.0      # background swings 4..8 ug/m3 over a day
MINUTES_PER_DAY = 1440.0


# --------------------------------------------------------------------------- #
# plume shapes
# --------------------------------------------------------------------------- #
@dataclass
class Plume:
    """One additive concentration contribution at one node.

    shape:
      "step"     peak immediately at `start`, held for `duration`, then zero
                 (no fall edge modelled -- the trace simply ends or the node
                 falls to background on the next packet).
      "ramp"     linear 0 -> peak over `ramp_min` minutes starting at `start`,
                 held to `start + duration`, then linear back down over
                 `ramp_min`.
      "gauss"    peak * exp(-0.5 * ((t - start) / sigma)^2); `duration` unused.
    """
    start: float
    peak: float
    duration: float
    shape: str = "step"
    ramp_min: float = 0.0
    sigma: float = 0.0

    def value(self, t: float) -> float:
        if self.shape == "gauss":
            if self.sigma <= 0:
                return 0.0
            return self.peak * math.exp(-0.5 * ((t - self.start) / self.sigma) ** 2)
        if self.shape == "step":
            return self.peak if self.start <= t < self.start + self.duration else 0.0
        if self.shape == "ramp":
            r = self.ramp_min
            if r <= 0:
                return self.peak if self.start <= t < self.start + self.duration else 0.0
            if t < self.start or t >= self.start + self.duration:
                return 0.0
            up = t - self.start
            down = (self.start + self.duration) - t
            return self.peak * min(1.0, up / r, down / r)
        raise ValueError("unknown plume shape %r" % self.shape)


# --------------------------------------------------------------------------- #
# scenario description
# --------------------------------------------------------------------------- #
@dataclass
class NodeSpec:
    node_id: str
    phase_min: float = 0.0            # packet phase inside the 12-min interval
    noise_sigma: float = 1.5          # ug/m3, Gaussian sensor noise
    background_phase_min: float = 0.0 # diurnal phase
    plumes: List[Plume] = field(default_factory=list)


@dataclass
class Trace:
    """A generated trace plus its ground truth."""
    name: str
    packets: List[Tuple[float, str, float]]     # (t, node_id, pm25) sorted by t
    interval_min: float
    duration_min: float
    nodes: List["NodeSpec"] = field(default_factory=list)
    truth: Dict[str, object] = field(default_factory=dict)
    notes: str = ""

    def packets_by_node(self) -> Dict[str, List[Tuple[float, float]]]:
        out: Dict[str, List[Tuple[float, float]]] = {}
        for t, n, v in self.packets:
            out.setdefault(n, []).append((t, v))
        return out

    def plume_value(self, node_id: str, t: float) -> float:
        """Ground-truth additive plume concentration at `node_id` at time t."""
        for n in self.nodes:
            if n.node_id == node_id:
                return sum(p.value(t) for p in n.plumes)
        return 0.0

    def in_smoke(self, node_id: str, t: float, thresh: float = 10.0) -> bool:
        return self.plume_value(node_id, t) >= thresh


def background(node: NodeSpec, t: float) -> float:
    """Diurnal clean-air background: floor plus a 12-h sinusoid."""
    theta = 2.0 * math.pi * ((t + node.background_phase_min) % MINUTES_PER_DAY) / MINUTES_PER_DAY
    return CLEAN_AIR_UGM3 + DIURNAL_AMPLITUDE_UGM3 * math.sin(theta)


def synth(spec: Dict[str, object], seed: int) -> Trace:
    """Generate one trace from a scenario spec dict.

    spec keys: name, duration_min, nodes (list[NodeSpec]), truth, notes, and the
    optional clean_air_floor override.
    """
    name = str(spec["name"])
    duration = float(spec["duration_min"])          # type: ignore[arg-type]
    nodes: Sequence[NodeSpec] = spec["nodes"]       # type: ignore[assignment]
    interval = float(spec.get("interval_min", PACKET_INTERVAL_MIN))

    packets: List[Tuple[float, str, float]] = []
    for idx, node in enumerate(nodes):
        rng = random.Random("%s|%s|%d" % (name, node.node_id, seed + idx))  # deterministic per node
        t = node.phase_min
        while t <= duration + 1e-9:
            value = background(node, t)
            for pl in node.plumes:
                value += pl.value(t)
            value += rng.gauss(0.0, node.noise_sigma)
            packets.append((round(t, 6), node.node_id, max(0.0, round(value, 3))))
            t += interval
    packets.sort(key=lambda p: (p[0], p[1]))
    return Trace(name=name, packets=packets, interval_min=interval,
                 duration_min=duration, nodes=list(nodes),
                 truth=dict(spec.get("truth", {})),  # type: ignore[arg-type]
                 notes=str(spec.get("notes", "")))


# --------------------------------------------------------------------------- #
# scenarios
# --------------------------------------------------------------------------- #
def scenario_burn_pile(seed: int = 1001) -> Trace:
    """Scenario 1 -- Neighbour burn pile: slow ramp, limited spatial extent.

    The Review Note #2 baseline-chasing case. Node N1 sits next to the pile and
    sees a slow linear rise; N2 barely sees it; N3/N4 are clean. The rise rate is
    the point: 2.0 ug/m3 per packet (~10 ug/m3/h) is a slow incursion, and the
    8-reading rolling median (~96 min of memory) tracks it, so the *relative*
    rise (>= 15 ug/m3 above baseline) may never fire even while the absolute
    floor (>= 25 ug/m3) is comfortably exceeded.
    """
    n1 = NodeSpec("N1", phase_min=0.0, noise_sigma=1.5,
                  plumes=[Plume(start=60.0, peak=55.0, duration=1080.0,
                                shape="ramp", ramp_min=300.0)])
    n2 = NodeSpec("N2", phase_min=3.0, noise_sigma=1.5,
                  plumes=[Plume(start=90.0, peak=22.0, duration=900.0,
                                shape="ramp", ramp_min=300.0)])
    n3 = NodeSpec("N3", phase_min=6.0, noise_sigma=1.5)
    n4 = NodeSpec("N4", phase_min=9.0, noise_sigma=1.5)
    return synth({
        "name": "burn_pile_slow_ramp",
        "duration_min": 1140.0,
        "nodes": [n1, n2, n3, n4],
        "truth": {
            "expected_watch": False,
            "first_rise_at": 60.0,
            "primary_node": "N1",
            "peak_ugm3": 55.0,
            "rise_rate_ugm3_per_packet": 2.0,
            "review_note": 2,
        },
        "notes": "Slow linear incursion, N1 only materially affected; the "
                 "baseline-chasing case of Review Note #2.",
    }, seed)


def scenario_dust_gust(seed: int = 1002) -> Trace:
    """Scenario 2 -- Road / agricultural dust gust: short, high amplitude,
    single node, poorly correlated.

    Exactly one packet of high PM2.5 (~110 ug/m3) at N2, nothing at the others.
    A.2's two-consecutive-packet rule should reject it: no confirmed rise, no
    Watch, zero false positives.
    """
    n1 = NodeSpec("N1", phase_min=0.0, noise_sigma=1.5)
    n2 = NodeSpec("N2", phase_min=3.0, noise_sigma=1.5,
                  plumes=[Plume(start=240.0, peak=104.0, duration=8.0, shape="step")])
    n3 = NodeSpec("N3", phase_min=6.0, noise_sigma=1.5)
    n4 = NodeSpec("N4", phase_min=9.0, noise_sigma=1.5)
    return synth({
        "name": "dust_gust_single_node",
        "duration_min": 600.0,
        "nodes": [n1, n2, n3, n4],
        "truth": {
            "expected_watch": False,
            "first_rise_at": 240.0,
            "primary_node": "N2",
            "peak_ugm3": 110.0,
            "review_note": 2,
        },
        "notes": "One-packet excursion: the two-packet confirmation filter is "
                 "the thing under test.",
    }, seed)


def scenario_bbq(seed: int = 1003) -> Trace:
    """Scenario 3 -- BBQ / cooking plume directly under ONE node, sustained.

    N3 is downwind of a lit grill for 2 h: PM2.5 holds ~85 ug/m3, far above both
    the absolute floor and the relative delta, so N3's Node Rise Event *is*
    confirmed -- and per Review Note #5 nothing network-level happens, because
    A.3 needs a second neighbour and no neighbour sees anything.
    Expected outcome: **NO WATCH**. Asserted, not merely observed.
    """
    n1 = NodeSpec("N1", phase_min=0.0, noise_sigma=1.5)
    n2 = NodeSpec("N2", phase_min=3.0, noise_sigma=1.5)
    n3 = NodeSpec("N3", phase_min=6.0, noise_sigma=1.5,
                  plumes=[Plume(start=120.0, peak=79.0, duration=120.0, shape="step")])
    n4 = NodeSpec("N4", phase_min=9.0, noise_sigma=1.5)
    return synth({
        "name": "bbq_single_node_extreme",
        "duration_min": 480.0,
        "nodes": [n1, n2, n3, n4],
        "truth": {
            "expected_watch": False,
            "first_rise_at": 120.0,
            "primary_node": "N3",
            "peak_ugm3": 85.0,
            "expect_confirmed_single_node": True,
            "review_note": 5,
        },
        "notes": "Sustained point source under one node; Review Note #5 says "
                 "silence is the designed outcome.",
    }, seed)


def scenario_wildfire_plume(seed: int = 1004) -> Trace:
    """Scenario 4 -- Wildfire smoke plume crossing the property: multi-node,
    sustained rise.

    N1 sees the plume first (step at t=72), N2 one packet later, N3 the packet
    after that. All three are neighbours, so a Watch is expected and then
    escalates on the third node (A.3). This is the "nominal" detection path.
    """
    n1 = NodeSpec("N1", phase_min=0.0, noise_sigma=1.5,
                  plumes=[Plume(start=72.0, peak=48.0, duration=900.0, shape="step")])
    n2 = NodeSpec("N2", phase_min=3.0, noise_sigma=1.5,
                  plumes=[Plume(start=84.0, peak=44.0, duration=888.0, shape="step")])
    n3 = NodeSpec("N3", phase_min=6.0, noise_sigma=1.5,
                  plumes=[Plume(start=96.0, peak=40.0, duration=864.0, shape="step")])
    n4 = NodeSpec("N4", phase_min=9.0, noise_sigma=1.5)
    return synth({
        "name": "wildfire_plume_multi_node",
        "duration_min": 1080.0,
        "nodes": [n1, n2, n3, n4],
        "truth": {
            "expected_watch": True,
            "first_rise_at": 72.0,
            "primary_node": "N1",
            "peak_ugm3": 48.0,
            "expect_escalation": True,
            "review_note": 1,
        },
        "notes": "Nominal multi-node plume; third node should escalate Watch.",
    }, seed)


def scenario_background_only(seed: int = 1005) -> Trace:
    """Scenario 5 -- Diurnal background + sensor noise only. Expected: zero
    Watches, zero confirmed rises (the false-positive floor)."""
    nodes = [NodeSpec("N%d" % i, phase_min=3.0 * (i - 1), noise_sigma=1.5,
                      background_phase_min=17.0 * i)
             for i in range(1, 5)]
    return synth({
        "name": "diurnal_background_noise",
        "duration_min": 1440.0,
        "nodes": nodes,
        "truth": {
            "expected_watch": False,
            "first_rise_at": None,
            "peak_ugm3": 0.0,
            "review_note": None,
        },
        "notes": "24 h of clean air; any Watch here is a false positive.",
    }, seed)


def scenario_staggered_worst_case(seed: int = 1006) -> Trace:
    """Section C measurement 1 / Review Note #1 -- the TRUE worst-case latency.

    Node A's packets land on the interval grid (phase 0), node B's are offset by
    8 minutes. The plume reaches A one minute after A's t=0 packet and B at
    t=21 (i.e. one minute after B's t=20 packet). Then:

      A shows the rise at its t=12 packet, confirms at t=24.
      B shows the rise at its t=32 packet, confirms at t=44.
      |44 - 24| = 20 min <= the 20-minute consensus window -> Watch at t=44.

    Measured from the first ground-truth rise (t=1) that is ~43 min, against the
    spec's claimed <= 25 min (A.1/A.5). Review Note #1's arithmetic predicts
    ~45 min; the difference is the 1-minute phase offset chosen here.
    """
    a = NodeSpec("A", phase_min=0.0, noise_sigma=1.5,
                 plumes=[Plume(start=1.0, peak=50.0, duration=600.0, shape="step")])
    b = NodeSpec("B", phase_min=8.0, noise_sigma=1.5,
                 plumes=[Plume(start=21.0, peak=46.0, duration=580.0, shape="step")])
    return synth({
        "name": "staggered_worst_case",
        "duration_min": 480.0,
        "interval_min": 12.0,
        "nodes": [a, b],
        "truth": {
            "expected_watch": True,
            "first_rise_at": 1.0,
            "primary_node": "A",
            "expected_watch_at": 44.0,
            "peak_ugm3": 50.0,
            "review_note": 1,
        },
        "notes": "Phase-staggered twin nodes chosen to realise Note #1's "
                 "+24/+44 walk-through.",
    }, seed)


def scenario_flap(seed: int = 1007) -> Trace:
    """Section C measurement 3 / Review Note #4 -- flap / re-arm after auto-clear.

    A and B both go high (step to ~44 ug/m3) so a Watch is raised; then both
    drop to background for 42 min, which satisfies A.4's 30-minute below-threshold
    auto-clear; then a second plume arrives and must raise a *new* Watch, because
    A.4's 2-hour suppression applies only to manual acknowledge, not auto-clear.

    Expected: 1 auto-clear and 1 re-arm (2 Watch events). Anything more is a flap.
    """
    a = NodeSpec("A", phase_min=0.0, noise_sigma=1.5, plumes=[
        Plume(start=24.0, peak=38.0, duration=156.0, shape="step"),
        Plume(start=240.0, peak=38.0, duration=444.0, shape="step"),
    ])
    b = NodeSpec("B", phase_min=3.0, noise_sigma=1.5, plumes=[
        Plume(start=36.0, peak=38.0, duration=144.0, shape="step"),
        Plume(start=252.0, peak=38.0, duration=432.0, shape="step"),
    ])
    return synth({
        "name": "flap_clear_rearm",
        "duration_min": 840.0,
        "nodes": [a, b],
        "truth": {
            "expected_watch": True,
            "first_rise_at": 24.0,
            "primary_node": "A",
            "expected_watches": 2,
            "expected_autoclears": 1,
            "peak_ugm3": 44.0,
            "review_note": 4,
        },
        "notes": "Plume, 42-min lull (auto-clear), plume again -- measures the "
                 "re-arm behaviour of Review Note #4.",
    }, seed)


def scenario_fresh_node(seed: int = 1008) -> Trace:
    """Review Note #6 (documented limitation, not a v0.1 defect assertion).

    Node A has been reporting since long before the event (its history starts at
    t=-1440). Node C is deployed at t=0 **into already-elevated air** -- a fire
    that started the same hour. A.2's baseline is the rolling median of the last
    8 readings, so C's baseline becomes the smoke itself on its very first
    stored reading: `current - baseline` is ~0 for every subsequent packet and C
    never confirms a rise for the whole event. A confirms normally.

    Measured consequence, and the reason this is a *sample-path* scenario rather
    than the "~2 hours" of Note #6: the blind window is not a fixed 2 h. A node
    deployed into clean air has a usable baseline after its **second** reading
    (A.2's "or all readings in the last 2 hours if fewer than 8 exist" fallback),
    i.e. ~24 min -- but a node deployed into smoke is blind for the whole event
    plus the ~96 min it takes the 8-reading window to flush the smoke out.
    """
    a = NodeSpec("A", phase_min=-1440.0, noise_sigma=1.5,
                 plumes=[Plume(start=0.0, peak=40.0, duration=300.0, shape="step")])
    c = NodeSpec("C", phase_min=3.0, noise_sigma=1.5,
                 plumes=[Plume(start=0.0, peak=40.0, duration=300.0, shape="step")])
    return synth({
        "name": "fresh_node_blind_window",
        "duration_min": 600.0,
        "nodes": [a, c],
        "truth": {
            "expected_watch": False,
            "first_rise_at": 0.0,
            "primary_node": "A",
            "peak_ugm3": 40.0,
            "review_note": 6,
            "deployed_at": {"A": -1440.0, "C": 0.0},
        },
        "notes": "C deployed into smoke at t=0; only A can confirm, so A.3's "
                 "two-neighbour requirement can never be met.",
    }, seed)


def scenario_three_node_escalation(seed: int = 1010) -> Trace:
    """A.3 escalation path, cleanly: three phase-ALIGNED neighbours.

    With all three nodes on the same packet phase, the whole property confirms on
    the same packet (t=36), the window holds three distinct nodes, and the Watch
    escalates on the "third neighbor node" branch rather than the hard threshold.
    Contrast with `wildfire_plume_multi_node`, where the nodes are staggered by
    12 min and A.3's 20-min window cannot hold three confirmations that are 24+
    minutes apart -- escalation there falls back to the >= 55 ug/m3 branch (or,
    when no node reaches 55, does not happen at all).
    """
    nodes = [NodeSpec("P%d" % i, phase_min=0.0, noise_sigma=1.5,
                      plumes=[Plume(start=24.0, peak=30.0, duration=360.0, shape="step")])
             for i in range(1, 4)]
    return synth({
        "name": "three_node_escalation",
        "duration_min": 480.0,
        "nodes": nodes,
        "truth": {
            "expected_watch": True,
            "first_rise_at": 24.0,
            "primary_node": "P1",
            "peak_ugm3": 36.0,
            "expect_escalation": "third node",
            "review_note": 1,
        },
        "notes": "Phase-aligned triple: isolates the third-node escalation branch.",
    }, seed)


def scenario_ack_suppression(seed: int = 1009) -> Trace:
    """A.4 manual-acknowledge path -- exercises the 2-hour suppress window.

    Two plumes 60 min apart. The first raises a Watch, which is acknowledged at
    the Watch time; the second arrives inside the 120-minute suppress window with
    the same node set, so A.4 forbids a new Watch. The harness asserts 1 Watch,
    not 2 -- this is what proves the ack path is wired, not merely implemented.
    """
    a = NodeSpec("A", phase_min=0.0, noise_sigma=1.5,
                 plumes=[Plume(start=24.0, peak=38.0, duration=60.0, shape="step"),
                         Plume(start=84.0, peak=38.0, duration=240.0, shape="step")])
    b = NodeSpec("B", phase_min=3.0, noise_sigma=1.5,
                 plumes=[Plume(start=36.0, peak=38.0, duration=48.0, shape="step"),
                         Plume(start=96.0, peak=38.0, duration=228.0, shape="step")])
    return synth({
        "name": "ack_suppression",
        "duration_min": 420.0,
        "nodes": [a, b],
        "truth": {
            "expected_watch": True,
            "first_rise_at": 24.0,
            "primary_node": "A",
            "expected_watches": 1,
            "ack_at": 36.0,
            "ack_on_first_watch": True,
            "peak_ugm3": 44.0,
            "review_note": 4,
        },
        "notes": "Manual ack at the first Watch; second identical plume must be "
                 "suppressed for 2 h (A.4).",
    }, seed)


ALL_SCENARIOS = [
    scenario_burn_pile,
    scenario_dust_gust,
    scenario_bbq,
    scenario_wildfire_plume,
    scenario_background_only,
    scenario_staggered_worst_case,
    scenario_flap,
    scenario_fresh_node,
    scenario_ack_suppression,
    scenario_three_node_escalation,
]


def build_all(seed: int = 1000) -> List[Trace]:
    return [fn(seed=seed + i) for i, fn in enumerate(ALL_SCENARIOS)]


if __name__ == "__main__":  # pragma: no cover - manual inspection only
    for tr in build_all():
        print("%-28s packets=%4d duration=%6.0f min  truth=%s"
              % (tr.name, len(tr.packets), tr.duration_min, tr.truth))
