// test_main.cpp -- host-side proof for the v0.2 consensus engine.
//
// Runs on the `native` PlatformIO env (pio test -e native) and in CI. Three
// kinds of test live here:
//
//   1. the 0091 scenario set (scenarios_v02.h, generated from traces.py) with a
//      recorded ground-truth expectation, each paired with a CONTROL that
//      re-runs the same trace with the v0.2 fix disabled so the assertion cannot
//      be vacuous ("nothing happened" must be shown to be a consequence of the
//      rule, not of the trace);
//   2. hand-built packet sequences for the two rules no 0091 trace exercises
//      (post-clear re-arm cooldown, manual-acknowledge suppression);
//   3. a cross-scenario invariant sweep.
//
// Every test prints a one-line summary; the harness prints the aggregate.

#include <unity.h>

#include <cstdio>
#include <string>
#include <vector>

#include "consensus_v02.h"
#include "scenarios_v02.h"

using wf::ConsensusConfig;
using wf::ConsensusEngine;
using wf::Event;
using wf::EventKind;
using wf::Level;
using wf::test::kScenarioCount;
using wf::test::kScenarios;
using wf::test::Pkt;
using wf::test::Scenario;

static int g_failures = 0;

// --------------------------------------------------------------------------
// helpers
// --------------------------------------------------------------------------
static std::vector<std::string> nodes_of(const Scenario& s) {
  std::vector<std::string> v;
  for (int i = 0; i < s.node_count; ++i) v.push_back(s.nodes[i]);
  return v;
}

static const Scenario* find_scenario(const char* name) {
  for (int i = 0; i < kScenarioCount; ++i) {
    if (std::string(kScenarios[i].name) == name) return &kScenarios[i];
  }
  return nullptr;
}

// Feed a whole trace. `ack_on_first_raise` models an operator acknowledging the
// first Watch/Alert the moment it is raised.
static ConsensusEngine run_scenario(const Scenario& s, const ConsensusConfig& cc,
                                    bool ack_on_first_raise = false) {
  const std::vector<std::string> nodes = nodes_of(s);
  ConsensusEngine e(cc, nodes);
  bool acked = false;
  for (int i = 0; i < s.pkt_count; ++i) {
    const Pkt& p = s.pkts[i];
    const std::vector<Event> evs = e.feed(p.t, nodes[p.node], true, p.pm25);
    if (ack_on_first_raise && !acked) {
      for (const Event& ev : evs) {
        if (ev.kind == EventKind::Watch || ev.kind == EventKind::Alert) {
          e.acknowledge(p.t);
          acked = true;
          break;
        }
      }
    }
  }
  return e;
}

// Hand-built sequence: pack(t, node, pm25) triples.
struct Seq {
  const std::vector<std::string> nodes;
  const std::vector<Pkt> pkts;
};

static ConsensusEngine run_seq(const Seq& s, const ConsensusConfig& cc, float ack_at = -1.0f) {
  ConsensusEngine e(cc, s.nodes);
  bool acked = false;
  for (size_t i = 0; i < s.pkts.size(); ++i) {
    const Pkt& p = s.pkts[i];
    e.feed(p.t, s.nodes[p.node], true, p.pm25);
    // Acknowledge once, after every packet whose time is at or before ack_at --
    // i.e. after the whole time slice, so a multi-node event is complete first.
    const bool last_of_slice = (i + 1 == s.pkts.size()) ||
                               (s.pkts[i + 1].t > ack_at + 1e-6f);
    if (ack_at >= 0.0f && !acked && last_of_slice && p.t <= ack_at + 1e-6f) {
      e.acknowledge(p.t);
      acked = true;
    }
  }
  return e;
}

static bool has_raise_at(const ConsensusEngine& e, float t) {
  for (const Event& ev : e.raises()) {
    if (ev.t == t) return true;
  }
  return false;
}

static float first_raise_after(const ConsensusEngine& e, float t) {
  for (const Event& ev : e.raises()) {
    if (ev.t > t + 1e-6f) return ev.t;
  }
  return -1.0f;
}

static int confirmations(const ConsensusEngine& e, const std::string& node) {
  int n = 0;
  for (const Event& ev : e.events()) {
    if (ev.kind == EventKind::NodeRiseConfirmed && ev.node == node) n++;
  }
  return n;
}

#define CHECK(cond, msg)                                                     \
  do {                                                                       \
    if (!(cond)) {                                                           \
      g_failures++;                                                          \
      printf("    FAIL: %s\n", (msg));                                       \
      TEST_ASSERT_TRUE_MESSAGE((cond), (msg));                               \
    }                                                                        \
  } while (0)

// --------------------------------------------------------------------------
// 1. the 0091 scenario set
// --------------------------------------------------------------------------
// Commit 1: Review Note #2, the baseline-chasing case. v0.1 never confirmed
// here; v0.2 must not miss it. N1 alone is affected (N2's 22 ug/m3 peak never
// clears the frozen-baseline delta), so the correct v0.2 outcome is a WATCH --
// one node, no escalation.
static void test_burn_pile_slow_ramp_detected() {
  const Scenario* s = find_scenario("burn_pile_slow_ramp");
  ConsensusConfig cc;
  const ConsensusEngine on = run_scenario(*s, cc);
  ConsensusConfig off = cc;
  off.freeze_on_suspected_rise = false;  // reproduces v0.1 baseline chasing
  const ConsensusEngine ctrl = run_scenario(*s, off);

  printf("[burn_pile_slow_ramp] v0.2 freeze=on watch=%d alert=%d confirmations(N1)=%d "
         "firstRaise=%.1f (latency %.0f min from the t=60 rise) | control freeze=off "
         "watch=%d alert=%d confirmations(N1)=%d\n",
         on.watch_count(), on.alert_count(), confirmations(on, "N1"),
         on.raises().empty() ? -1.0f : on.raises()[0].t,
         (on.raises().empty() ? -1.0f : on.raises()[0].t) - s->first_rise_at,
         ctrl.watch_count(), ctrl.alert_count(), confirmations(ctrl, "N1"));

  CHECK(on.watch_count() >= 1, "burn pile: v0.2 must raise a Watch (it must not be missed)");
  CHECK(confirmations(on, "N1") == 1, "burn pile: only the frozen baseline lets N1 confirm at all");
  CHECK(on.alert_count() == 0, "burn pile: only one node sees it -- no Alert");
  // The detection must land before the ramp peaks at t=360, i.e. it is the
  // frozen baseline firing, not an eventual absolute-threshold crossing.
  CHECK(!on.raises().empty() && on.raises()[0].t < 360.0f,
        "burn pile: the Watch must be raised before the plume peaks");
  // Control: with the freeze disabled the trace produces nothing at all, which
  // is exactly the v0.1 behaviour 0091 measured. If this control ever raises an
  // event the test above stops proving anything.
  CHECK(ctrl.watch_count() == 0 && ctrl.alert_count() == 0 &&
            confirmations(ctrl, "N1") == 0,
        "control: freeze disabled (v0.1 chasing) must miss the slow ramp entirely");
}

// Commit 2: the two-packet confirmation rule. One 110 ug/m3 packet is a gust,
// not smoke.
static void test_dust_gust_two_packet_filter() {
  const Scenario* s = find_scenario("dust_gust_single_node");
  ConsensusConfig cc;
  const ConsensusEngine on = run_scenario(*s, cc);
  ConsensusConfig one = cc;
  one.confirm_packets = 1;  // control: the filter removed
  const ConsensusEngine ctrl = run_scenario(*s, one);

  printf("[dust_gust_single_node] v0.2 confirm=2 watch=%d alert=%d | control confirm=1 watch=%d alert=%d\n",
         on.watch_count(), on.alert_count(), ctrl.watch_count(), ctrl.alert_count());

  CHECK(on.watch_count() == 0 && on.alert_count() == 0 && on.clear_count() == 0,
        "dust gust: a single high packet must produce no event");
  CHECK(ctrl.watch_count() >= 1, "control: with confirm=1 the same packet must raise a Watch");
}

// Commit 3: Review Note #5. A sustained extreme under ONE node is a Watch, and
// it must never escalate to an Alert by itself.
static void test_bbq_single_node_never_alerts() {
  const Scenario* s = find_scenario("bbq_single_node_extreme");
  ConsensusConfig cc;
  const ConsensusEngine on = run_scenario(*s, cc);
  ConsensusConfig one = cc;
  one.require_two_nodes = false;  // control: the second-node requirement removed
  const ConsensusEngine ctrl = run_scenario(*s, one);

  printf("[bbq_single_node_extreme] v0.2 watch=%d alert=%d | control need=1 watch=%d alert=%d\n",
         on.watch_count(), on.alert_count(), ctrl.watch_count(), ctrl.alert_count());

  CHECK(on.watch_count() >= 1, "bbq: the single elevated node must raise a Watch");
  CHECK(on.alert_count() == 0, "bbq: a single node must NEVER escalate to an Alert");
  CHECK(ctrl.alert_count() >= 1, "control: with the two-node rule relaxed the same node must Alert");
}

// The nominal multi-node plume: Watch on the first node, Alert when the second
// confirms, inside the correlation window.
static void test_wildfire_plume_multi_node_alerts() {
  const Scenario* s = find_scenario("wildfire_plume_multi_node");
  ConsensusConfig cc;
  const ConsensusEngine e = run_scenario(*s, cc);

  printf("[wildfire_plume_multi_node] watch=%d alert=%d firstRaise=%.1f\n",
         e.watch_count(), e.alert_count(), e.raises().empty() ? -1.0f : e.raises()[0].t);

  CHECK(e.alert_count() >= 1, "multi-node plume: a second confirming node must Alert");
  CHECK(s->expected_watch, "multi-node plume: the 0091 ground truth expects a detection");
}

// The false-positive floor: 24 h of clean air must produce nothing.
static void test_background_only_zero_false_positives() {
  const Scenario* s = find_scenario("diurnal_background_noise");
  ConsensusConfig cc;
  const ConsensusEngine e = run_scenario(*s, cc);
  int confirms = 0;
  for (const Event& ev : e.events()) {
    if (ev.kind == EventKind::NodeRiseConfirmed) confirms++;
  }
  printf("[diurnal_background_noise] watch=%d alert=%d confirmations=%d (4 nodes x 24 h)\n",
         e.watch_count(), e.alert_count(), confirms);
  CHECK(e.watch_count() == 0 && e.alert_count() == 0 && confirms == 0,
        "clean air: 24 h of background must produce no event");
}

// Review Note #1 / section C: the true worst-case latency with phase-staggered
// nodes. The 0091 harness measured ~43 min against the spec's claimed <= 25.
static void test_staggered_worst_case_latency() {
  const Scenario* s = find_scenario("staggered_worst_case");
  ConsensusConfig cc;
  const ConsensusEngine e = run_scenario(*s, cc);
  const float t_alert = (e.alert_count() > 0) ? e.raises().back().t : -1.0f;

  printf("[staggered_worst_case] watch=%d alert=%d alertAt=%.1f firstRise=%.1f latency=%.1f min\n",
         e.watch_count(), e.alert_count(), t_alert, s->first_rise_at,
         t_alert - s->first_rise_at);

  CHECK(e.alert_count() == 1, "staggered: exactly one Alert (the two nodes are the same event)");
  CHECK(t_alert >= 40.0f && t_alert <= 50.0f,
        "staggered: the Alert lands at ~44 min (the 0091 walk-through), not <= 25");
  CHECK(t_alert - s->first_rise_at > 25.0f,
        "staggered: measured latency must contradict the <= 25 min claim");
}

// Review Note #4: clear -> re-arm, with no flap. The same trace shows whether
// the cooldown is load-bearing by varying it.
static void test_flap_clear_and_rearm() {
  const Scenario* s = find_scenario("flap_clear_rearm");
  ConsensusConfig cc;
  const ConsensusEngine e = run_scenario(*s, cc);
  ConsensusConfig long_cd = cc;
  long_cd.rearm_cooldown_min = 200.0f;
  const ConsensusEngine suppressed = run_scenario(*s, long_cd);

  printf("[flap_clear_rearm] v0.2 alert=%d clear=%d | control cooldown=200 alert=%d clear=%d\n",
         e.alert_count(), e.clear_count(), suppressed.alert_count(), suppressed.clear_count());

  CHECK(e.alert_count() == 2, "flap: the second plume must raise a second Alert");
  CHECK(e.clear_count() == 2, "flap: each plume ends in one auto-clear");
  CHECK(suppressed.alert_count() == 1,
        "control: a cooldown longer than the lull must suppress the re-arm");
}

// Review Note #6, carried as a documented limitation rather than a defect: a
// node deployed INTO smoke is blind for the whole event, so the second-node
// requirement can never be met and no Alert is possible.
static void test_fresh_node_blind_window_limitation() {
  const Scenario* s = find_scenario("fresh_node_blind_window");
  ConsensusConfig cc;
  const ConsensusEngine e = run_scenario(*s, cc);

  printf("[fresh_node_blind_window] watch=%d alert=%d confirmations(A)=%d confirmations(C)=%d\n",
         e.watch_count(), e.alert_count(), confirmations(e, "A"), confirmations(e, "C"));

  CHECK(e.alert_count() == 0, "fresh node: no Alert is possible (only one node can confirm)");
  CHECK(confirmations(e, "C") == 0, "fresh node: the node deployed into smoke stays blind");
  CHECK(confirmations(e, "A") >= 1, "fresh node: the long-running node still confirms");
}

// A.3's third-node branch, phase-aligned so all three confirm on one packet.
static void test_three_node_escalation() {
  const Scenario* s = find_scenario("three_node_escalation");
  ConsensusConfig cc;
  const ConsensusEngine e = run_scenario(*s, cc);
  printf("[three_node_escalation] watch=%d alert=%d\n", e.watch_count(), e.alert_count());
  CHECK(e.alert_count() >= 1, "three-node: phase-aligned triple must Alert");
}

// Manual acknowledge on the 0091 ack trace (kept for parity with 0091; the
// rule's real coverage is the hand-built test below).
static void test_ack_suppression_trace() {
  const Scenario* s = find_scenario("ack_suppression");
  ConsensusConfig cc;
  const ConsensusEngine e = run_scenario(*s, cc, /*ack_on_first_raise=*/true);
  printf("[ack_suppression] watch=%d alert=%d clear=%d (ack at first raise)\n",
         e.watch_count(), e.alert_count(), e.clear_count());
  CHECK(e.alert_count() >= 1, "ack trace: the first plume must still Alert");
}

// --------------------------------------------------------------------------
// 2. hand-built rules
// --------------------------------------------------------------------------
// The post-clear re-arm cooldown, with the guard proven load-bearing: the same
// sequence re-arms at t=108 with the cooldown off and cannot with it on.
static void test_rearm_cooldown_blocks_early_rearm() {
  // clean -> both nodes high -> clear -> both nodes high again 12 min later
  const Seq seq{{"A", "B"},
                {{0.0f, 0, 6.0f},  {0.0f, 1, 6.0f},
                 {12.0f, 0, 6.0f}, {12.0f, 1, 6.0f},
                 {24.0f, 0, 46.0f}, {24.0f, 1, 6.0f},
                 {36.0f, 0, 46.0f}, {36.0f, 1, 46.0f},
                 {48.0f, 0, 46.0f}, {48.0f, 1, 46.0f},
                 {60.0f, 0, 6.0f},  {60.0f, 1, 6.0f},
                 {72.0f, 0, 6.0f},  {72.0f, 1, 6.0f},
                 {84.0f, 0, 6.0f},  {84.0f, 1, 6.0f},
                 {96.0f, 0, 46.0f}, {96.0f, 1, 6.0f},
                 {108.0f, 0, 46.0f}, {108.0f, 1, 46.0f},
                 {120.0f, 0, 46.0f}, {120.0f, 1, 46.0f}}};

  ConsensusConfig cc;
  const ConsensusEngine on = run_seq(seq, cc);
  ConsensusConfig zero = cc;
  zero.rearm_cooldown_min = 0.0f;
  const ConsensusEngine ctrl = run_seq(seq, zero);

  const float clear_at = on.clears().empty() ? -1.0f : on.clears()[0].t;
  const float next_on = first_raise_after(on, clear_at);
  const float next_off = first_raise_after(ctrl, clear_at);

  printf("[rearm_cooldown] clearAt=%.1f cooldown=30 nextRaise=%.1f | control cooldown=0 nextRaise=%.1f\n",
         clear_at, next_on, next_off);

  CHECK(clear_at > 0.0f, "cooldown: the first Watch must auto-clear (precondition)");
  CHECK(!has_raise_at(on, 108.0f),
        "cooldown: 24 min after the clear is inside the 30-min cooldown -- no re-arm");
  CHECK(next_on >= clear_at + cc.rearm_cooldown_min,
        "cooldown: the next raise must be at or after clear + cooldown");
  CHECK(has_raise_at(ctrl, 108.0f) && next_off < clear_at + cc.rearm_cooldown_min,
        "control: with the cooldown off the same sequence re-arms 24 min after the clear");
}

// Manual acknowledge: a same-set confirmation inside the suppress window must
// not re-alert, while a previously uninvolved node still may. A and B rise on the
// same packet so the Alert at t=36 already names both; the ack then covers {A,B}.
static void test_ack_suppresses_same_set_and_lets_new_node_through() {
  const Seq seq{{"A", "B", "C"},
                {{0.0f, 0, 6.0f},  {0.0f, 1, 6.0f},  {0.0f, 2, 6.0f},
                 {12.0f, 0, 6.0f}, {12.0f, 1, 6.0f}, {12.0f, 2, 6.0f},
                 {24.0f, 0, 46.0f}, {24.0f, 1, 46.0f}, {24.0f, 2, 6.0f},
                 {36.0f, 0, 46.0f}, {36.0f, 1, 46.0f}, {36.0f, 2, 6.0f},
                 {48.0f, 0, 46.0f}, {48.0f, 1, 46.0f}, {48.0f, 2, 6.0f},
                 {60.0f, 0, 46.0f}, {60.0f, 1, 46.0f}, {60.0f, 2, 46.0f},
                 {72.0f, 0, 46.0f}, {72.0f, 1, 46.0f}, {72.0f, 2, 46.0f},
                 {84.0f, 0, 46.0f}, {84.0f, 1, 46.0f}, {84.0f, 2, 46.0f}}};

  ConsensusConfig cc;
  const ConsensusEngine e = run_seq(seq, cc, /*ack_at=*/36.0f);
  ConsensusConfig nosup = cc;
  nosup.ack_suppress_enabled = false;
  const ConsensusEngine ctrl = run_seq(seq, nosup, /*ack_at=*/36.0f);

  printf("[ack_suppress] ackedAt=36.0 alert=%d raises=%zu | control suppress=off alert=%d raises=%zu\n",
         e.alert_count(), e.raises().size(), ctrl.alert_count(), ctrl.raises().size());

  CHECK(e.alert_count() >= 1, "ack: the first consensus must Alert");
  CHECK(!has_raise_at(e, 48.0f),
        "ack: the same node set must not re-alert inside the suppress window");
  CHECK(has_raise_at(ctrl, 48.0f),
        "control: without suppression the same packets re-alert 12 min after the ack");
  CHECK(ctrl.alert_count() > e.alert_count(),
        "control: suppression must reduce the alert count on the same packets");
  CHECK(has_raise_at(e, 72.0f),
        "ack: a previously uninvolved node must still be able to raise Watch/Alert");
}

// --------------------------------------------------------------------------
// 3. cross-scenario invariants
// --------------------------------------------------------------------------
static void test_invariants_over_the_whole_scenario_set() {
  ConsensusConfig cc;
  int checked = 0;
  for (int i = 0; i < kScenarioCount; ++i) {
    const ConsensusEngine e = run_scenario(kScenarios[i], cc);
    // (a) every clear is followed by a raise at or after clear + cooldown
    for (const Event& c : e.clears()) {
      const float next = first_raise_after(e, c.t);
      if (next > 0.0f) {
        CHECK(next >= c.t + cc.rearm_cooldown_min - 1e-6f,
              "invariant: no re-arm inside the post-clear cooldown");
      }
      checked++;
    }
    // (b) an Alert is always preceded by a Watch or by a second confirmation:
    //     every Alert carries >= 2 distinct nodes, and every Watch carries 1-2.
    for (const Event& ev : e.raises()) {
      if (ev.kind == EventKind::Alert) {
        CHECK(ev.nodes.size() >= 2, "invariant: an Alert always names >= 2 nodes");
      } else {
        CHECK(ev.nodes.size() >= 1, "invariant: a Watch always names >= 1 node");
      }
    }
    // (c) the level is never left dangling: an active level must have members
    CHECK(e.level() == Level::None || !e.members().empty(),
          "invariant: an active level always has a member set");
    // (d) determinism: the same trace twice gives identical counters
    const ConsensusEngine again = run_scenario(kScenarios[i], cc);
    CHECK(again.watch_count() == e.watch_count() &&
              again.alert_count() == e.alert_count() &&
              again.clear_count() == e.clear_count(),
          "invariant: the engine is deterministic");
  }
  printf("[invariants] clears=%d scenarios=%d -- cooldown / node-count / determinism invariants hold\n",
         checked, kScenarioCount);
}

// --------------------------------------------------------------------------
// runner
// --------------------------------------------------------------------------
void setUp() {}
void tearDown() {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_burn_pile_slow_ramp_detected);
  RUN_TEST(test_dust_gust_two_packet_filter);
  RUN_TEST(test_bbq_single_node_never_alerts);
  RUN_TEST(test_wildfire_plume_multi_node_alerts);
  RUN_TEST(test_background_only_zero_false_positives);
  RUN_TEST(test_staggered_worst_case_latency);
  RUN_TEST(test_flap_clear_and_rearm);
  RUN_TEST(test_fresh_node_blind_window_limitation);
  RUN_TEST(test_three_node_escalation);
  RUN_TEST(test_ack_suppression_trace);
  RUN_TEST(test_rearm_cooldown_blocks_early_rearm);
  RUN_TEST(test_ack_suppresses_same_set_and_lets_new_node_through);
  RUN_TEST(test_invariants_over_the_whole_scenario_set);
  const int rc = UNITY_END();
  printf("CONSENSUS-TEST SUMMARY: scenarios=%d checks-failed=%d\n", kScenarioCount, g_failures);
  if (rc == 0 && g_failures == 0) {
    printf("CONSENSUS-TEST: PASS\n");
  } else {
    printf("CONSENSUS-TEST: FAIL\n");
  }
  return rc == 0 && g_failures == 0 ? 0 : 1;
}
