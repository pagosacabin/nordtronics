"""rules_v01.py -- Detection Logic Spec v0.1, Section A, transcribed verbatim.

THIS FILE IS THE CODE UNDER TEST.

Review Notes #1-#9 (Section B of the spec) are deliberately NOT fixed here. The
harness exists to *measure* them (Section C). If a rule looks wrong while reading
this file, that is expected: it is implemented as written.

Spec: docs/wildfire/detection-logic-spec-v0.1-review-notes.md (Section A).
Every function below quotes the Section A clause it implements, so a reviewer can
check the transcription clause by clause.

Pure Python 3, standard library only. No I/O, no clock, no randomness.
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field
from typing import Dict, Iterable, List, Optional, Tuple

# --- Section A.1 "Operating Assumptions" -------------------------------------
PACKET_INTERVAL_MIN = 12      # A.1 "Nominal packet interval: 12 minutes"
BASELINE_WINDOW = 8           # A.2 "rolling median of the most recent 8 valid readings"
BASELINE_MAX_AGE_MIN = 120    # A.2 "or all readings in the last 2 hours if fewer than 8 exist"

# --- Section A.2 "Node-Level Rise Event" ------------------------------------
ABS_FLOOR_UGM3 = 25.0         # A.2.1 "Absolute floor: current PM2.5 >= 25 ug/m3"
REL_DELTA_UGM3 = 15.0         # A.2.2 "Relative rise: current PM2.5 - baseline >= 15 ug/m3"
CONFIRM_PACKETS = 2           # A.2 "observed on at least two consecutive packets"

# --- Section A.3 "Network Consensus (Watch / Alert)" ------------------------
CONSENSUS_WINDOW_MIN = 20     # A.3 "20-minute sliding window"
LIVE_AGE_MIN = 30             # A.3 "reported at least one packet in the last 30 minutes"
ESCALATE_NODE_COUNT = 3       # A.3 "A third neighbor node also generates a confirmed ... Event"
HARD_THRESHOLD_UGM3 = 55.0    # A.3 "any participating node reports PM2.5 >= 55 ug/m3"

# --- Section A.4 "Clearing" --------------------------------------------------
AUTOCLEAR_BELOW_MIN = 30      # A.4 "remained below their rise thresholds for ... 30 minutes"
ACK_SUPPRESS_MIN = 120        # A.4 "suppresses re-alerting for the same set of nodes for 2 hours"


def _median(xs: Iterable[float]) -> float:
    """Plain median (stdlib statistics.median is equivalent; kept local and tiny)."""
    s = sorted(xs)
    n = len(s)
    if n == 0:
        raise ValueError("median of empty sequence")
    mid = n // 2
    if n % 2:
        return s[mid]
    return (s[mid - 1] + s[mid]) / 2.0


# --- Section A.2: baseline ---------------------------------------------------
def rolling_baseline(readings: List[Tuple[float, float]], now: float,
                     age_limit_min: float = BASELINE_MAX_AGE_MIN,
                     window: int = BASELINE_WINDOW) -> Optional[float]:
    """A.2: "baseline is the rolling median of the most recent 8 valid readings
    (or all readings in the last 2 hours if fewer than 8 exist)".

    `readings` is the node's prior valid readings as (t_minutes, pm25) pairs, in
    order, excluding the packet being evaluated. Returns None when there is no
    prior reading at all (a node with one reading can never show a relative rise
    -- Review Note #6, a known limitation of the frozen rules, not a bug here).

    Implementation note (declared assumption): the current packet is excluded
    from its own baseline, which is what makes "current - baseline" a rise
    against history rather than against itself.
    """
    if not readings:
        return None
    recent = [r for r in readings if now - r[0] <= age_limit_min]
    if not recent:
        # Rules are silent on a node silent for >2 h; fall back to the full
        # history so the node is not permanently baseline-less.
        recent = list(readings)
    window_vals = [v for _, v in recent[-window:]]
    return _median(window_vals)


# --- Section A.2: the rise condition ----------------------------------------
def node_rise_condition(pm25: float, baseline: Optional[float]) -> bool:
    """A.2: a packet generates a (provisional) Node Rise Event when BOTH hold:

    1. Absolute floor: current PM2.5 >= 25 ug/m3          (A.2.1)
    2. Relative rise:  current PM2.5 - baseline >= 15 ug/m3 (A.2.2)

    Provisional only -- A.2 requires two consecutive packets before the event is
    confirmed; see V01Detector.feed().
    """
    if baseline is None:
        return False
    return (pm25 >= ABS_FLOOR_UGM3) and ((pm25 - baseline) >= REL_DELTA_UGM3)


@dataclass
class NodeState:
    node_id: str
    readings: List[Tuple[float, float]] = field(default_factory=list)  # valid only
    last_packet_at: Optional[float] = None
    last_pm25: Optional[float] = None
    rise_streak: int = 0               # consecutive packets meeting A.2
    above: bool = False                # this packet met the rise condition
    first_rise_at: Optional[float] = None   # first packet of the current streak
    last_rise_true_at: Optional[float] = None  # last packet meeting the condition
    confirmed_at: Optional[float] = None    # A.2 second consecutive packet
    confirmation_emitted: bool = False


@dataclass
class Event:
    t: float
    kind: str          # NODE_RISE_CONFIRMED | WATCH | WATCH_ESCALATED | WATCH_CLEARED
    node: Optional[str] = None
    nodes: Tuple[str, ...] = ()
    detail: str = ""

    def __str__(self) -> str:  # pragma: no cover - formatting only
        who = self.node or ",".join(self.nodes)
        return f"t={self.t:8.1f}  {self.kind:<20} {who:<20} {self.detail}"


class V01Detector:
    """Sections A.2-A.4 as one state machine, fed packets in time order.

    Neighbors (A.1: "'Nearby' is initially defined statically by property
    membership and a configurable neighbor list") are supplied by the caller; in
    these scenarios every node in a scenario is a mutual neighbor.
    """

    def __init__(self, nodes: Iterable[str], neighbors: Optional[Dict[str, set]] = None,
                 spec_gap: bool = False):
        self.nodes: Dict[str, NodeState] = {n: NodeState(n) for n in nodes}
        self.neighbors: Dict[str, set] = (
            neighbors if neighbors is not None
            else {n: set(self.nodes) - {n} for n in self.nodes}
        )
        # spec_gap=True raises Watch on a single confirmed node as well; used to
        # show the Note #5 assertion is non-vacuous. Default is the letter of A.3.
        self.spec_gap = spec_gap

        self.watch_active = False
        self.watch_nodes: Tuple[str, ...] = ()
        self.watch_node_sets: List[Tuple[str, ...]] = []   # node set of each Watch
        self.watch_raised_at: Optional[float] = None
        self.watch_escalated = False
        self.watches_raised: List[float] = []
        self.clears: List[float] = []
        self.escalations: List[float] = []
        self.escalation_reasons: List[str] = []

        # Manual acknowledge (A.4): node-set suppression until a wall time.
        self.suppress_nodes: Tuple[str, ...] = ()
        self.suppress_until: Optional[float] = None

        self.latest_confirmed: Dict[str, float] = {}
        self.events: List[Event] = []

    # -- helpers --------------------------------------------------------------
    def _is_neighbor(self, a: str, b: str) -> bool:
        return b in self.neighbors.get(a, set())

    def _window_nodes(self, t: float) -> List[str]:
        """Confirmed events inside the 20-minute sliding window (A.3)."""
        return sorted(
            n for n, ct in self.latest_confirmed.items()
            if (t - ct) <= CONSENSUS_WINDOW_MIN + 1e-9
        )

    def _live(self, n: str, t: float) -> bool:
        """A.3: 'reported at least one packet in the last 30 minutes'."""
        st = self.nodes[n]
        return st.last_packet_at is not None and (t - st.last_packet_at) <= LIVE_AGE_MIN + 1e-9

    def _suppressed(self, participating: Iterable[str], hard_hit: bool) -> bool:
        """A.4: 'suppresses re-alerting for the same set of nodes for 2 hours'.

        'A new Watch can still be raised during the suppress window if a
        previously uninvolved neighbor joins or a hard threshold (>= 55 ug/m3)
        is crossed.' -- so suppression blocks only the same-set, no-hard case.
        """
        if self.suppress_until is None or self.watch_active:
            return False
        if hard_hit:
            return False
        return set(participating) <= set(self.suppress_nodes)

    # -- A.4 manual acknowledge ----------------------------------------------
    def acknowledge(self, t: float) -> None:
        """A.4 manual ack: suppress re-alerting for the current watch's node set."""
        # The spec attaches suppression to "the same set of nodes"; an ack with
        # no watch is recorded against the nodes currently confirmed.
        nodes = self.watch_nodes or tuple(self._window_nodes(t))
        self.suppress_nodes = tuple(nodes)
        self.suppress_until = t + ACK_SUPPRESS_MIN
        self.watch_active = False
        self.watch_escalated = False

    # -- main entry -----------------------------------------------------------
    def feed(self, t: float, node_id: str, pm25: Optional[float]) -> List[Event]:
        """Feed one packet. Returns the events it caused (may be empty)."""
        st = self.nodes[node_id]
        out: List[Event] = []

        valid = pm25 is not None and isinstance(pm25, (int, float)) and math.isfinite(pm25)
        if valid:
            baseline = rolling_baseline(st.readings, t)
            rise = node_rise_condition(float(pm25), baseline)

            st.last_packet_at = t
            st.last_pm25 = float(pm25)
            st.above = rise

            if rise:
                st.rise_streak += 1
                st.last_rise_true_at = t
                if st.rise_streak == 1:
                    st.first_rise_at = t
                # A.2: "must be observed on at least two consecutive packets
                # before the Node Rise Event is considered confirmed."
                if (st.rise_streak >= CONFIRM_PACKETS) and not st.confirmation_emitted:
                    st.confirmed_at = t
                    st.confirmation_emitted = True
                    self.latest_confirmed[node_id] = t
                    ev = Event(t, "NODE_RISE_CONFIRMED", node=node_id,
                               detail=f"pm25={pm25:.1f} baseline={baseline:.1f}")
                    out.append(ev)
            else:
                st.rise_streak = 0
                st.first_rise_at = None
                st.confirmation_emitted = False

            st.readings.append((t, float(pm25)))

        out.extend(self._evaluate(t))
        self.events.extend(out)
        return out

    # -- A.3 / A.4 evaluation -------------------------------------------------
    def _evaluate(self, t: float) -> List[Event]:
        out: List[Event] = []

        window = [n for n in self._window_nodes(t) if self._live(n, t)]
        # A.3 requires "at least two distinct nodes"; spec_gap mode relaxes it to
        # one so the Note #5 assertion has a non-vacuous control.
        need = 1 if self.spec_gap else 2
        participating = [n for n in window if self._is_neighbor_of_any(n, window, need)]

        hard_hit = any(
            (self.nodes[n].last_pm25 or 0.0) >= HARD_THRESHOLD_UGM3 for n in participating
        )

        if len(participating) >= need and not self.watch_active:
            if not self._suppressed(participating, hard_hit):
                self.watch_active = True
                self.watch_nodes = tuple(participating)
                self.watch_node_sets.append(tuple(participating))
                self.watch_raised_at = t
                self.watch_escalated = False
                self.watches_raised.append(t)
                out.append(Event(t, "WATCH", nodes=tuple(participating),
                                 detail="consensus on %d nodes in %d-min window"
                                        % (len(participating), CONSENSUS_WINDOW_MIN)))
                self.suppress_until = None  # a raised watch consumes the ack window

        if self.watch_active:
            # A.3 escalation: third neighbor node, or any participant >= 55 ug/m3.
            escalated = (ESCALATE_NODE_COUNT <= len(participating)) or hard_hit
            if escalated and not self.watch_escalated:
                self.watch_escalated = True
                self.escalations.append(t)
                why = ("third node in window" if len(participating) >= ESCALATE_NODE_COUNT
                       else "hard threshold >= %g ug/m3" % HARD_THRESHOLD_UGM3)
                self.escalation_reasons.append(why)
                out.append(Event(t, "WATCH_ESCALATED", nodes=tuple(participating), detail=why))

            # A.4 auto-clear: every contributing node below its rise threshold
            # continuously for 30 minutes.
            if self._all_below_for(t, self.watch_nodes, AUTOCLEAR_BELOW_MIN):
                self.watch_active = False
                self.watch_escalated = False
                self.clears.append(t)
                out.append(Event(t, "WATCH_CLEARED", nodes=self.watch_nodes,
                                 detail="all contributing nodes below thresholds for %d min"
                                        % AUTOCLEAR_BELOW_MIN))
                self.watch_nodes = ()

        return out

    def _is_neighbor_of_any(self, n: str, window: List[str], need: int) -> bool:
        """A.3: the confirmed nodes must be 'defined as neighbors'. With a fully
        connected scenario every windowed node qualifies; the check is kept so a
        partial neighbor list is honoured."""
        if need <= 1:
            return True
        return any(self._is_neighbor(n, m) for m in window if m != n)

    def _all_below_for(self, t: float, nodes: Iterable[str], minutes: float) -> bool:
        for n in nodes:
            st = self.nodes[n]
            if st.above:
                return False
            if st.last_rise_true_at is None:
                return False
            if (t - st.last_rise_true_at) < minutes - 1e-9:
                return False
        return True

    # -- reporting ------------------------------------------------------------
    def watch_at(self) -> Optional[float]:
        return self.watches_raised[0] if self.watches_raised else None
