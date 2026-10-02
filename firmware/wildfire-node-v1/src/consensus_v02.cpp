#include "consensus_v02.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace wf {
namespace {
constexpr float kEps = 1e-9f;

float median_of(std::vector<float> xs) {
  std::sort(xs.begin(), xs.end());
  const size_t n = xs.size();
  if (n == 0) return 0.0f;
  if (n % 2) return xs[n / 2];
  return 0.5f * (xs[n / 2 - 1] + xs[n / 2]);
}

std::string join(const std::vector<std::string>& v) {
  std::string s;
  for (size_t i = 0; i < v.size(); ++i) {
    if (i) s += ",";
    s += v[i];
  }
  return s;
}
}  // namespace

const char* level_name(Level l) {
  switch (l) {
    case Level::None: return "NONE";
    case Level::Watch: return "WATCH";
    case Level::Alert: return "ALERT";
  }
  return "?";
}

const char* event_kind_name(EventKind k) {
  switch (k) {
    case EventKind::NodeRiseConfirmed: return "NODE_RISE_CONFIRMED";
    case EventKind::Watch: return "WATCH";
    case EventKind::Alert: return "ALERT";
    case EventKind::Cleared: return "CLEARED";
  }
  return "?";
}

ConsensusEngine::ConsensusEngine(const ConsensusConfig& cfg,
                                 const std::vector<std::string>& nodes)
    : cfg_(cfg) {
  for (const auto& n : nodes) nodes_[n] = Node();
}

Event* ConsensusEngine::push(Event ev) {
  events_.push_back(ev);
  if (ev.kind == EventKind::Watch || ev.kind == EventKind::Alert) {
    raises_.push_back(ev);
    // point the caller at the copy that lives in events_ (raises_ may reallocate)
    return &events_.back();
  }
  if (ev.kind == EventKind::Cleared) clears_.push_back(ev);
  return &events_.back();
}

bool ConsensusEngine::rolling_baseline(const Node& nd, float t, float* out) const {
  if (nd.readings.empty()) return false;
  std::vector<float> vals;
  for (const auto& r : nd.readings) {
    if (t - r.first <= cfg_.baseline_max_age_min + kEps) vals.push_back(r.second);
  }
  if (vals.empty()) {
    // Rules are silent on a node silent for > 2 h; fall back to the full history
    // so the node is not permanently baseline-less (same fallback as v0.1).
    for (const auto& r : nd.readings) vals.push_back(r.second);
  }
  const size_t keep = std::min<size_t>(vals.size(), cfg_.baseline_window);
  std::vector<float> tail(vals.end() - keep, vals.end());
  *out = median_of(tail);
  return true;
}

std::vector<Event> ConsensusEngine::feed(float t, const std::string& node_id, bool valid,
                                         float pm25) {
  const size_t before = events_.size();
  auto it = nodes_.find(node_id);
  if (it == nodes_.end()) {
    // Unknown / unprovisioned node ID: the base ignores it (radio contract).
    Node fresh;
    it = nodes_.emplace(node_id, fresh).first;
  }
  Node& nd = it->second;

  if (valid) {
    float base = 0.0f;
    bool have_base = false;
    if (cfg_.freeze_on_suspected_rise && nd.frozen) {
      base = nd.frozen_base;  // v0.2: the baseline stays put while the rise runs
      have_base = true;
    } else {
      have_base = rolling_baseline(nd, t, &base);
    }

    // v0.2 "first suspected rise": the earliest point at which an incursion is
    // even possible is the packet that reaches the absolute floor. Freezing HERE
    // (rather than on the relative rise) is what makes the fix work -- a
    // relative-rise trigger cannot fire on a slow ramp, because the rolling
    // baseline has already been dragged up by the ramp itself. Measured on the
    // 0091 burn-pile trace: freezing on the relative rise never fires at all,
    // freezing on the floor crossing raises a Watch at t=264 min.
    const bool at_or_above_floor = pm25 >= cfg_.abs_floor_ugm3 - kEps;
    if (at_or_above_floor) nd.last_at_floor_at = t;
    if (cfg_.freeze_on_suspected_rise && !nd.frozen && at_or_above_floor && have_base) {
      nd.frozen = true;
      nd.frozen_base = base;
      nd.frozen_at = t;
      base = nd.frozen_base;
    }

    const bool rise = have_base && at_or_above_floor &&
                      (pm25 - base) >= cfg_.rel_delta_ugm3 - kEps;

    nd.last_packet_at = t;
    nd.last_pm25 = pm25;
    nd.above = rise;

    if (rise) {
      if (nd.streak == 0) nd.first_rise_at = t;
      nd.streak += 1;
      nd.last_rise_true_at = t;
      if (nd.streak >= static_cast<int>(cfg_.confirm_packets) &&
          !nd.confirmation_emitted) {
        nd.confirmation_emitted = true;
        nd.confirmed_at = t;
        latest_confirmed_[node_id] = t;
        Event ev;
        ev.t = t;
        ev.kind = EventKind::NodeRiseConfirmed;
        ev.node = node_id;
        std::ostringstream os;
        os.setf(std::ios::fixed);
        os.precision(1);
        os << "pm25=" << pm25 << " baseline=" << base
           << (nd.frozen ? " (frozen)" : "");
        ev.detail = os.str();
        push(ev);
      }
    } else {
      if (nd.streak > 0) {
        nd.streak = 0;
        nd.first_rise_at = -1.0f;
        nd.confirmation_emitted = false;
      }
    }

    // Release the freeze only once the node has been back below the absolute
    // floor long enough that the excursion is over; the rolling baseline then
    // re-converges on the new background.
    if (nd.frozen && nd.last_at_floor_at >= 0.0f &&
        (t - nd.last_at_floor_at) >= cfg_.autoclear_below_min - kEps) {
      nd.frozen = false;
      nd.frozen_base = 0.0f;
    }

    nd.readings.emplace_back(t, pm25);
    if (nd.readings.size() > 64) nd.readings.erase(nd.readings.begin());
  }

  evaluate(t);
  return std::vector<Event>(events_.begin() + before, events_.end());
}

bool ConsensusEngine::node_live(const std::string& n, float t) const {
  auto it = nodes_.find(n);
  if (it == nodes_.end() || it->second.last_packet_at < 0.0f) return false;
  return (t - it->second.last_packet_at) <= cfg_.live_age_min + kEps;
}

bool ConsensusEngine::suppressed(float t, const std::vector<std::string>& live) const {
  if (!cfg_.ack_suppress_enabled || level_ != Level::None) return false;
  if (ack_until_ < 0.0f || t > ack_until_ + kEps) return false;
  for (const auto& n : live) {
    if (std::find(ack_nodes_.begin(), ack_nodes_.end(), n) == ack_nodes_.end()) {
      return false;  // a previously uninvolved neighbour may still alert
    }
  }
  return !live.empty();
}

bool ConsensusEngine::all_below_for(float t, float minutes) const {
  if (members_.empty()) return false;
  for (const auto& n : members_) {
    auto it = nodes_.find(n);
    if (it == nodes_.end()) return false;
    const Node& nd = it->second;
    if (nd.above) return false;
    if (nd.last_rise_true_at < 0.0f) return false;
    if ((t - nd.last_rise_true_at) < minutes - kEps) return false;
  }
  return true;
}

void ConsensusEngine::raise_watch(float t, const std::vector<std::string>& live) {
  level_ = Level::Watch;
  members_ = live;
  watch_count_ += 1;
  Event ev;
  ev.t = t;
  ev.kind = EventKind::Watch;
  ev.nodes = live;
  std::ostringstream os;
  os << "single-node elevation (" << join(live) << ") -- watch only, no escalation";
  ev.detail = os.str();
  push(ev);
}

void ConsensusEngine::escalate(float t, const std::vector<std::string>& live) {
  level_ = Level::Alert;
  members_ = live;
  alert_count_ += 1;
  Event ev;
  ev.t = t;
  ev.kind = EventKind::Alert;
  ev.nodes = live;
  std::ostringstream os;
  os << "consensus on " << live.size() << " nodes (" << join(live) << ") in "
     << static_cast<int>(cfg_.correlation_window_min) << "-min window";
  ev.detail = os.str();
  push(ev);
}

void ConsensusEngine::clear_level(float t) {
  level_ = Level::None;
  clear_count_ += 1;
  Event ev;
  ev.t = t;
  ev.kind = EventKind::Cleared;
  ev.nodes = members_;
  std::ostringstream os;
  os << "all contributing nodes below threshold for "
     << static_cast<int>(cfg_.autoclear_below_min) << " min; re-arm at t+"
     << static_cast<int>(cfg_.rearm_cooldown_min);
  ev.detail = os.str();
  push(ev);
  members_.clear();
  rearm_ready_at_ = t + cfg_.rearm_cooldown_min;  // v0.2 post-clear cooldown
}

void ConsensusEngine::evaluate(float t) {
  std::vector<std::string> live;
  for (const auto& kv : latest_confirmed_) {
    if ((t - kv.second) <= cfg_.correlation_window_min + kEps && node_live(kv.first, t)) {
      live.push_back(kv.first);
    }
  }
  std::sort(live.begin(), live.end());
  const int n = static_cast<int>(live.size());

  if (level_ == Level::None) {
    if (!armed(t) || suppressed(t, live)) return;
    const int need = cfg_.require_two_nodes ? 2 : 1;
    if (n >= need) {
      escalate(t, live);
    } else if (n >= 1) {
      // v0.2: one elevated node is a Watch. It never becomes an Alert by itself,
      // however extreme the reading -- only a second node can escalate.
      raise_watch(t, live);
    }
    return;
  }

  if (level_ == Level::Watch && n >= 2) {
    escalate(t, live);
    return;  // escalate() re-took members_; the clear check runs next packet
  }
  if (all_below_for(t, cfg_.autoclear_below_min)) clear_level(t);
}

void ConsensusEngine::acknowledge(float t) {
  if (level_ == Level::None) return;
  ack_nodes_ = members_;
  ack_until_ = t + cfg_.ack_suppress_min;
  level_ = Level::None;
  members_.clear();
}

bool ConsensusEngine::confirmed(const std::string& node_id) const {
  return latest_confirmed_.find(node_id) != latest_confirmed_.end();
}

float ConsensusEngine::confirmed_at(const std::string& node_id) const {
  auto it = latest_confirmed_.find(node_id);
  return it == latest_confirmed_.end() ? -1.0f : it->second;
}

}  // namespace wf
