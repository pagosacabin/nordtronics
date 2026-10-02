// consensus_v02.h -- Wildfire v0.2 network consensus engine.
//
// Portable C++17: no Arduino, no clock, no I/O, no randomness. The identical
// translation unit is compiled into the ESP32 firmware (base-station role) and
// into the host-side unity test (test/test_consensus_native), so the engine the
// bench runs is the engine the test proves.
//
// v0.1 (0091, archived) chased the ramp: the baseline was a rolling median over
// the recent history, so a slow incursion pulled the baseline up with it and the
// relative-rise term never fired. v0.2 changes exactly three things (see
// docs/wildfire/radio-protocol-v1.md and the 0091 sim results):
//   1. freeze each node's baseline on the first suspected rise,
//   2. a post-clear re-arm cooldown,
//   3. one codebase for both roles -- but this file is base-only.
// The threshold constants stay at the values the task fixes: the absolute floor
// is 25 ug/m3 and the rise above the (now frozen) baseline must also be
// >= 25 ug/m3, confirmed on >= 2 consecutive packets.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace wf {

enum class Level : uint8_t { None = 0, Watch = 1, Alert = 2 };

enum class EventKind : uint8_t {
  NodeRiseConfirmed = 0,
  Watch = 1,       // one node confirmed -- local notice, never self-escalates
  Alert = 2,       // two or more nodes confirmed inside the correlation window
  Cleared = 3,     // all contributing nodes below threshold for the clear time
};

struct Event {
  float t = 0.0f;                       // minutes since trace/boot start
  EventKind kind = EventKind::NodeRiseConfirmed;
  std::string node;                     // set for NodeRiseConfirmed
  std::vector<std::string> nodes;       // participating set, sorted
  std::string detail;
};

// Every value below is a captive-portal field on the device (firmware default +
// portal field + NVS). The defaults are the ones this task fixes; the correlation
// window is the one open parameter (0091 validated 20 min and recommended
// widening with no number attached -- default 20, portal range 10..60).
struct ConsensusConfig {
  float abs_floor_ugm3 = 25.0f;         // A.2.1 absolute floor, unchanged
  float rel_delta_ugm3 = 25.0f;         // rise above the FROZEN baseline
  uint8_t confirm_packets = 2;          // consecutive packets required
  float correlation_window_min = 20.0f; // portal 10..60
  float autoclear_below_min = 30.0f;    // below-threshold time before a clear
  float rearm_cooldown_min = 30.0f;     // v0.2: no re-arm this soon after a clear
  uint8_t baseline_window = 8;          // readings in the rolling median
  float baseline_max_age_min = 120.0f;  // staleness fallback, as v0.1 A.2
  float live_age_min = 30.0f;           // node counts as live if it reported lately
  float ack_suppress_min = 120.0f;      // manual acknowledge suppression window
  bool freeze_on_suspected_rise = true; // set false to reproduce v0.1 chasing
  bool require_two_nodes = true;        // set false for the single-node control
  bool ack_suppress_enabled = true;
};

class ConsensusEngine {
 public:
  ConsensusEngine() = default;
  ConsensusEngine(const ConsensusConfig& cfg, const std::vector<std::string>& nodes);

  // Feed one received packet in time order. `valid=false` models a corrupt or
  // rejected frame: the packet updates liveness but can never confirm a rise.
  // Returns the events this packet caused (possibly none).
  std::vector<Event> feed(float t_min, const std::string& node_id, bool valid, float pm25);

  // Operator acknowledge (app / portal). Clears the active level and suppresses
  // re-alerting for the same participating set for ack_suppress_min.
  void acknowledge(float t_min);

  Level level() const { return level_; }
  const std::vector<std::string>& members() const { return members_; }

  // Counters and the event log, for assertions and for the OLED/MQTT status.
  int watch_count() const { return watch_count_; }
  int alert_count() const { return alert_count_; }
  int clear_count() const { return clear_count_; }
  const std::vector<Event>& events() const { return events_; }
  const std::vector<Event>& raises() const { return raises_; }   // Watch|Alert
  const std::vector<Event>& clears() const { return clears_; }

  bool confirmed(const std::string& node_id) const;
  float confirmed_at(const std::string& node_id) const;

 private:
  struct Node {
    std::vector<std::pair<float, float>> readings;  // (t, pm25) valid packets only
    float last_packet_at = -1.0f;
    float last_pm25 = -1.0f;
    int streak = 0;
    bool above = false;              // the latest packet met the rise condition
    float first_rise_at = -1.0f;
    float last_rise_true_at = -1.0f;
    float last_at_floor_at = -1.0f;  // last packet at/above the absolute floor
    float confirmed_at = -1.0f;
    bool confirmation_emitted = false;
    bool frozen = false;             // v0.2 baseline freeze is engaged
    float frozen_base = 0.0f;
    float frozen_at = -1.0f;
  };

  bool rolling_baseline(const Node& nd, float t, float* out) const;
  void evaluate(float t);
  void raise_watch(float t, const std::vector<std::string>& live);
  void escalate(float t, const std::vector<std::string>& live);
  void clear_level(float t);
  bool all_below_for(float t, float minutes) const;
  bool node_live(const std::string& n, float t) const;
  bool suppressed(float t, const std::vector<std::string>& live) const;
  bool armed(float t) const { return t + 1e-9f >= rearm_ready_at_; }
  Event* push(Event ev);

  ConsensusConfig cfg_;
  std::map<std::string, Node> nodes_;
  std::map<std::string, float> latest_confirmed_;

  Level level_ = Level::None;
  std::vector<std::string> members_;
  float rearm_ready_at_ = -1e9f;

  std::vector<std::string> ack_nodes_;
  float ack_until_ = -1.0f;

  int watch_count_ = 0;
  int alert_count_ = 0;
  int clear_count_ = 0;
  std::vector<Event> events_;
  std::vector<Event> raises_;
  std::vector<Event> clears_;
};

// Human-readable helpers (used by the test summary and the serial log).
const char* level_name(Level l);
const char* event_kind_name(EventKind k);

}  // namespace wf
