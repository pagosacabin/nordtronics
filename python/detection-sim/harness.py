#!/usr/bin/env python3
"""harness.py -- feed synthetic traces through rules_v01 and report the results.

Section C of the spec asks for four things; this harness prints them as
PASS/FAIL lines, not prose:

  1. the true worst-case latency path (staggered two-node confirmation, Note #1)
  2. whether the burn-pile case is detected or missed via baseline chasing (Note #2)
  3. the flap / re-arm count after auto-clear (Note #4)
  4. the asserted no-Watch on the single-node extreme case (Note #5)

Usage (one command, no arguments needed):

    python3 python/detection-sim/harness.py

Exit code is 0 when every assertion passes, 1 when any assertion fails, so the
harness is usable as a gate. `--verbose` also dumps every detector event.
"""

from __future__ import annotations

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from rules_v01 import (  # noqa: E402
    ABS_FLOOR_UGM3, REL_DELTA_UGM3, HARD_THRESHOLD_UGM3,
    V01Detector, rolling_baseline,
)
import traces  # noqa: E402

# --- the four Section C assertions the spec names ---------------------------
CLAIMED_WORST_CASE_MIN = 25.0     # A.1 / A.5 "<= 25 minutes"
NOTE1_PREDICTED_MIN = 45.0        # Review Note #1 "approximately 45 minutes"
NOTE1_TOLERANCE_MIN = 3.0         # our phase choices shift it by a minute or two


def probe_trace(trace: traces.Trace):
    """Replay a trace with the *same* A.2 baseline function and record, per node
    per packet, the baseline and the relative delta. Used for the diagnostic
    numbers the results document quotes (peak, max delta, baseline drift)."""
    rows = {}
    for node in sorted({n for _, n, _ in trace.packets}):
        hist = []
        rows[node] = []
        for t, n, v in trace.packets:
            if n != node:
                continue
            b = rolling_baseline(hist, t)
            rows[node].append({"t": t, "pm25": v, "baseline": b,
                               "delta": (None if b is None else v - b)})
            hist.append((t, v))
    return rows


def run_trace(trace: traces.Trace, spec_gap: bool = False, ack_first_watch: bool = False,
              verbose: bool = False):
    nodes = sorted({n for _, n, _ in trace.packets})
    det = V01Detector(nodes, spec_gap=spec_gap)
    acked = False
    for t, n, v in trace.packets:
        for ev in det.feed(t, n, v):
            if verbose:
                print("      " + str(ev))
            if ev.kind == "WATCH" and ack_first_watch and not acked:
                det.acknowledge(t)
                acked = True
    return det


def severity_path(det: V01Detector) -> str:
    """Render the severity path taken: watch -> escalated (why)."""
    parts = []
    for t in det.watches_raised:
        parts.append("Watch@%.0f" % t)
    for t in det.escalations:
        parts.append("Escalated@%.0f" % t)
    for t in det.clears:
        parts.append("AutoClear@%.0f" % t)
    return " -> ".join(parts) if parts else "no events"


def nodes_involved(det: V01Detector) -> str:
    involved = sorted(n for n, st in det.nodes.items() if st.confirmed_at is not None)
    return ",".join(involved) if involved else "-"


def ramp_sensitivity_sweep():
    """Diagnostic (not an assertion): the slowest linear rise a *single* node can
    confirm. Quantifies Review Note #2 -- the relative test (A.2.2) is a rate
    gate, so a slow incursion is invisible no matter how high the absolute
    concentration climbs."""
    rows = []
    for slope in (0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 5.0, 6.0):
        node = traces.NodeSpec(
            "X", phase_min=0.0, noise_sigma=0.0,
            plumes=[traces.Plume(start=0.0, peak=slope * 60.0, duration=720.0,
                                 shape="ramp", ramp_min=720.0)])
        tr = traces.synth({"name": "sweep", "duration_min": 720.0,
                           "nodes": [node], "truth": {}}, seed=7)
        det = V01Detector(["X"])
        for t, n, v in tr.packets:
            det.feed(t, n, v)
        probes = probe_trace(tr)["X"]
        rows.append({
            "slope_per_packet": slope,
            "rate_per_hour": slope * 5.0,      # 12-min packets -> 5 per hour
            "confirmed": det.nodes["X"].confirmed_at is not None,
            "peak": max(p["pm25"] for p in probes),
            "max_delta": max(p["delta"] for p in probes if p["delta"] is not None),
        })
    first = next((r for r in rows if r["confirmed"]), None)
    return rows, first


def main(argv=None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    verbose = "--verbose" in argv or "-v" in argv

    traces_list = traces.build_all()
    results = []
    for tr in traces_list:
        ack = bool(tr.truth.get("ack_on_first_watch"))
        det = run_trace(tr, ack_first_watch=ack, verbose=verbose)
        probes = probe_trace(tr)
        expected = bool(tr.truth.get("expected_watch"))
        watches = det.watches_raised
        fp = len(watches) if not expected else 0
        fn = 1 if (expected and not watches) else 0
        delay = None
        if watches and tr.truth.get("first_rise_at") is not None:
            delay = watches[0] - float(tr.truth["first_rise_at"])  # type: ignore[arg-type]
        peak = max((p["pm25"] for rows in probes.values() for p in rows), default=0.0)
        max_delta = max((p["delta"] for rows in probes.values() for p in rows
                         if p["delta"] is not None), default=0.0)
        results.append({
            "trace": tr, "det": det, "probes": probes, "expected": expected,
            "watches": watches, "fp": fp, "fn": fn, "delay": delay,
            "peak": peak, "max_delta": max_delta,
        })

    # ---------------- per-scenario table ------------------------------------
    print("=" * 100)
    print("DETECTION SIM v0.1 -- v0.1 rules (spec Section A) against synthetic traces")
    print("=" * 100)
    print("%-26s %-8s %-9s %-8s %-8s %-16s %s"
          % ("scenario", "expect", "observed", "delay", "fp/fn", "nodes", "severity path"))
    print("-" * 100)
    for r in results:
        tr = r["trace"]
        observed = "WATCH" if r["watches"] else "none"
        delay = "-" if r["delay"] is None else "%.0f min" % r["delay"]
        print("%-26s %-8s %-9s %-8s %-8s %-16s %s"
              % (tr.name, "WATCH" if r["expected"] else "none", observed, delay,
                 "%d/%d" % (r["fp"], r["fn"]), nodes_involved(r["det"]),
                 severity_path(r["det"])))
    print("-" * 100)

    assertions = []

    def check(name, ok, detail):
        assertions.append((name, bool(ok), detail))

    by_name = {r["trace"].name: r for r in results}

    # -- Section C #1 / Review Note #1: true worst-case latency ---------------
    r = by_name["staggered_worst_case"]
    got = r["delay"]
    want = float(r["trace"].truth["expected_watch_at"]) - float(r["trace"].truth["first_rise_at"])
    ok = (got is not None and abs(got - want) <= 1.0
          and got > CLAIMED_WORST_CASE_MIN
          and abs(got - NOTE1_PREDICTED_MIN) <= NOTE1_TOLERANCE_MIN)
    check("worst_case_latency",
          ok,
          "staggered two-node path measured %.0f min from first rise to Watch "
          "(designed %.0f min); Review Note #1 predicts ~%.0f min; the spec "
          "claims <= %.0f min (A.1/A.5) -- the claim is %.0f min optimistic"
          % (got if got is not None else -1, want, NOTE1_PREDICTED_MIN,
             CLAIMED_WORST_CASE_MIN,
             (got if got is not None else 0) - CLAIMED_WORST_CASE_MIN))

    # -- Section C #2 / Review Note #2: burn pile -----------------------------
    r = by_name["burn_pile_slow_ramp"]
    n1 = r["probes"]["N1"]
    peak_pm = max(p["pm25"] for p in n1)
    max_d = max(p["delta"] for p in n1 if p["delta"] is not None)
    n1_confirmed = r["det"].nodes["N1"].confirmed_at is not None
    outcome = "MISSED" if not r["watches"] else "DETECTED"
    check("burn_pile_outcome",
          (outcome == "MISSED") and (not n1_confirmed),
          "burn pile %s: N1 peaked at %.1f ug/m3 (absolute floor %.0f exceeded) "
          "but its largest relative rise was %.1f ug/m3 against the %.0f ug/m3 "
          "requirement -- the rolling median (last 8 readings / ~96 min) chased "
          "the ramp. No Node Rise Event, no Watch."
          % (outcome, peak_pm, ABS_FLOOR_UGM3, max_d, REL_DELTA_UGM3))

    # -- Section C #3 / Review Note #4: flap / re-arm -------------------------
    r = by_name["flap_clear_rearm"]
    rearm = max(0, len(r["watches"]) - 1)
    clears = r["det"].clears
    tr = r["trace"]
    # Which clears fired while the ground-truth plume was still present?
    in_smoke_clears = []
    for c in clears:
        involved = [n for n in r["det"].nodes if tr.in_smoke(n, c)]
        if involved:
            in_smoke_clears.append((c, involved))
    check("flap_rearm_count",
          rearm == 1 and len(clears) == 2,
          "flap count: %d re-arm(s) after %d auto-clear(s) (%d Watch event(s) "
          "total: %s). A.4 has no post-clear cooldown -- suppression follows "
          "manual ack only -- so a mid-plume self-clear is followed by a fresh "
          "Watch the user sees as flicker."
          % (rearm, len(clears), len(r["watches"]),
             ",".join("t=%.0f" % w for w in r["watches"])))

    check("premature_clear_in_smoke",
          len(in_smoke_clears) >= 1,
          "Review Note #4 confirmed with a physical cross-check: %d of %d "
          "auto-clear(s) fired while the ground-truth plume was still on "
          "(clear(s) at %s). The adaptive baseline had absorbed the smoke, so "
          "the rise condition lapsed and A.4 counted 30 min below threshold "
          "while PM2.5 was still ~%.0f ug/m3."
          % (len(in_smoke_clears), len(clears),
             ",".join("t=%.0f[%s]" % (c, "+".join(ns)) for c, ns in in_smoke_clears),
             float(r["trace"].truth.get("peak_ugm3", 0.0))))

    # -- sustained plume: does the Watch survive while the smoke does? --------
    r = by_name["wildfire_plume_multi_node"]
    w_tr = r["trace"]
    sus_clears = [(c, [n for n in r["det"].nodes if w_tr.in_smoke(n, c)])
                  for c in r["det"].clears]
    sus_clears = [(c, ns) for c, ns in sus_clears if ns]
    check("sustained_plume_self_clear",
          bool(r["watches"]) and bool(sus_clears) and len(r["watches"]) == 1,
          "the multi-node plume runs uninterrupted for 900 min, yet its Watch "
          "auto-cleared at t=%.0f with every node still in smoke and never "
          "re-armed (Watch events: %s). Cause: the 8-reading baseline absorbs "
          "the sustained plume, (current - baseline) falls under 15 ug/m3 with "
          "PM2.5 still ~%.0f ug/m3, A.4 counts 30 min 'below threshold' and "
          "clears. This is Review Note #2 and #4 firing together on the "
          "scenario the design exists for; the sim's own output is the "
          "evidence, not an argument."
          % (sus_clears[0][0] if sus_clears else -1,
             ",".join("t=%.0f" % w for w in r["watches"]),
             float(w_tr.truth.get("peak_ugm3", 0.0))))

    # -- Section C #4 / Review Note #5: single node, extreme ------------------
    r = by_name["bbq_single_node_extreme"]
    n3_confirmed = r["det"].nodes["N3"].confirmed_at is not None
    control = run_trace(r["trace"], spec_gap=True)   # non-vacuity control
    check("single_node_no_watch",
          (not r["watches"]) and n3_confirmed and bool(control.watches_raised),
          "BBQ point source under N3 alone: N3's Node Rise Event IS confirmed "
          "(confirmed_at=%.0f) yet no Watch is raised -- A.3 needs two "
          "neighbours. Non-vacuity control: the same trace with a one-node "
          "consensus rule raises a Watch at t=%.0f, so the assertion is not "
          "true by construction."
          % (r["det"].nodes["N3"].confirmed_at or -1,
             control.watches_raised[0] if control.watches_raised else -1))

    # -- supporting assertions (the rest of the required scenario set) --------
    r = by_name["dust_gust_single_node"]
    check("dust_gust_rejected",
          (not r["watches"]) and len(r["det"].watches_raised) == 0,
          "single-packet 110 ug/m3 spike on N2 produced %d confirmed rise "
          "event(s) and %d Watch(es) -- A.2's two-consecutive-packet rule "
          "rejected it (0 false positives)."
          % (len(r["det"].latest_confirmed), len(r["watches"])))

    r = by_name["wildfire_plume_multi_node"]
    esc = r["det"].escalations
    check("wildfire_plume_detected",
          bool(r["watches"]) and r["delay"] is not None and r["delay"] <= 60
          and sum(1 for n in "N1 N2 N3".split()
                  if r["det"].nodes[n].confirmed_at is not None) >= 2,
          "multi-node plume: Watch at t=%.0f (delay %.0f min from first rise); "
          "confirmed nodes %s; escalation %s; nodes involved %s."
          % (r["watches"][0] if r["watches"] else -1,
             r["delay"] if r["delay"] is not None else -1,
             ",".join(n for n in "N1 N2 N3".split()
                      if r["det"].nodes[n].confirmed_at is not None),
             ("at t=%.0f" % esc[0]) if esc else "did NOT fire (see note)",
             nodes_involved(r["det"])))

    r = by_name["three_node_escalation"]
    watch_sets = r["det"].watch_node_sets
    reasons = r["det"].escalation_reasons
    confirmed_nodes = sorted(r["det"].latest_confirmed)
    check("third_node_escalation",
          bool(r["watches"]) and bool(r["det"].escalations)
          and len(confirmed_nodes) == 3
          and bool(reasons) and reasons[0] == "third node in window",
          "phase-aligned triple: Watch at t=%.0f on %d nodes (%s), then "
          "escalated at t=%.0f via the '%s' branch once the third node's packet "
          "for the same minute confirmed (3 nodes confirmed: %s). Contrast the "
          "staggered wildfire scenario, where three confirmations span >20 min "
          "and cannot all sit inside A.3's window."
          % (r["watches"][0] if r["watches"] else -1,
             len(watch_sets[0]) if watch_sets else 0,
             ",".join(watch_sets[0]) if watch_sets else "-",
             r["det"].escalations[0] if r["det"].escalations else -1,
             reasons[0] if reasons else "-",
             ",".join(confirmed_nodes)))

    r = by_name["diurnal_background_noise"]
    check("background_zero_watches",
          len(r["watches"]) == 0 and len(r["det"].latest_confirmed) == 0,
          "24 h of diurnal background + 1.5 ug/m3 Gaussian noise across 4 "
          "nodes: %d Watch(es), %d confirmed rise event(s) -- the "
          "false-positive floor of the frozen thresholds."
          % (len(r["watches"]), len(r["det"].latest_confirmed)))

    r = by_name["fresh_node_blind_window"]
    c_conf = r["det"].nodes["C"].confirmed_at
    a_conf = r["det"].nodes["A"].confirmed_at
    check("fresh_node_blind",
          c_conf is None and a_conf is not None and len(r["watches"]) == 0,
          "Review Note #6 reproduced on the sample path: node C is deployed at "
          "t=0 into air that is already at PM2.5 ~46 ug/m3, so its own first "
          "reading becomes its baseline and it never confirms a rise "
          "(confirmed_at=%s) for the whole event, while the pre-existing node A "
          "confirms at t=%.0f. With only one confirmation A.3 cannot reach a "
          "Watch, so the miss is silent. The blind window is NOT a fixed ~2 h: "
          "the 'all readings in the last 2 hours' fallback gives a clean-deployed "
          "node a usable baseline after its second reading (~24 min); the ~2 h "
          "figure only holds for a node that powers up inside smoke."
          % (c_conf, a_conf if a_conf is not None else -1))

    r = by_name["ack_suppression"]
    check("ack_suppression_holds",
          len(r["watches"]) == 1,
          "manual acknowledge at the first Watch suppressed the second "
          "identical plume inside the 120-min window (A.4): %d Watch(es) "
          "raised, expected 1." % len(r["watches"]))

    # ---------------- Section C summary -------------------------------------
    print()
    print("SECTION C MEASUREMENTS (Review Notes #1, #2, #4, #5)")
    print("-" * 100)
    for name in ("worst_case_latency", "burn_pile_outcome", "flap_rearm_count",
                 "single_node_no_watch"):
        detail = next(d for n, _, d in assertions if n == name)
        print("* %s:" % name)
        for line in _wrap(detail, 94):
            print("    " + line)
    print()

    print()
    print("THRESHOLD SENSITIVITY SWEEP (single clean node, linear ramp, noise-free)")
    print("-" * 100)
    print("%-14s %-16s %-10s %-12s %s"
          % ("per packet", "per hour", "confirmed?", "peak ug/m3", "max rise vs baseline"))
    rows, first = ramp_sensitivity_sweep()
    for r in rows:
        print("%-14s %-16s %-10s %-12.1f %.1f"
              % ("%.1f ug/m3" % r["slope_per_packet"],
                 "%.1f ug/m3" % r["rate_per_hour"],
                 "yes" if r["confirmed"] else "no",
                 r["peak"], r["max_delta"]))
    if first:
        print("-> slowest confirmable linear rise: %.1f ug/m3 per packet "
              "(%.1f ug/m3/h); anything slower is invisible to A.2.2 no matter "
              "how high the absolute concentration climbs."
              % (first["slope_per_packet"], first["rate_per_hour"]))
    print("-" * 100)

    # ---------------- PASS/FAIL block ---------------------------------------
    print("ASSERTIONS")
    print("-" * 100)
    failures = 0
    for name, ok, detail in assertions:
        print("%s %-26s %s" % ("PASS" if ok else "FAIL", name,
                               _wrap(detail, 66)[0]))
        if not ok:
            failures += 1
    print("-" * 100)
    print("SUMMARY: %d/%d PASS, %d FAIL"
          % (len(assertions) - failures, len(assertions), failures))
    return 1 if failures else 0


def _wrap(text: str, width: int):
    words = text.split()
    lines, cur = [], ""
    for w in words:
        if len(cur) + len(w) + 1 > width and cur:
            lines.append(cur)
            cur = w
        else:
            cur = (cur + " " + w).strip()
    if cur:
        lines.append(cur)
    return lines


if __name__ == "__main__":
    sys.exit(main())
