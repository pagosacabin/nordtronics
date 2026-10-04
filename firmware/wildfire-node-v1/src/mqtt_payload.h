// mqtt_payload.h -- the MQTT payload + event-topic shapes the deployed backend
// actually accepts (task 0105).
//
// 0104 fixed the telemetry TOPIC (`<root>/<node-id>/telemetry`) but left the
// payload and the event leg in shapes the backend rejects, so no reading and no
// event would ever be stored. The authority is the deployed backend, read on
// main before this header was written:
//
//   backend/ingest/ingest/validation.py
//     REQUIRED_FIELDS = pm25, temperature_c, humidity_pct, battery_v
//       with range checks (battery_v is 0..30 VOLTS, so millivolts fail)
//     node_id must be a STRING (the worker checks it against the topic)
//     a timestamp is read from observed_utc (or ts / timestamp)
//     unknown keys are ignored, never fatal -- but the required keys are exact
//   backend/ingest/ingest/events.py
//     EVENT_KINDS = watch_raised, watch_cleared, alert_raised, alert_cleared
//     required: `event` (one of the four) and `observed_utc`
//     optional: node_id (defaults to "network"), pm25, baseline, nodes, window_min
//     topic: nordtronics/wildfire/+/events
//
// Portable C++17 with no Arduino dependency, so a host harness compiles the
// same code the firmware runs and can print its exact bytes (the tactic 0104
// used for wf::telemetry_topic). There is one builder per shape so the wire
// form is stated in exactly one place and cannot drift per call site.
#pragma once

#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

#include "consensus_v02.h"

namespace wf {

// ---------------------------------------------------------------------------
// tiny helpers
// ---------------------------------------------------------------------------

// ISO-8601 UTC with a trailing Z, exactly `YYYY-MM-DDTHH:MM:SSZ` -- the shape
// validation.py's _iso_z() emits and accepts. The base's clock is set from NTP
// before the uplink (task 0099), so a real instant is always available.
inline std::string iso8601_z(std::time_t t) {
  std::tm tm{};
#if defined(_WIN32)
  gmtime_s(&tm, &t);
#else
  gmtime_r(&t, &tm);
#endif
  char buf[32];
  if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm) == 0) return std::string();
  return std::string(buf);
}

// Minimal JSON string escape. The values here are node ids, an ISO timestamp
// and four snake_case keys -- this exists so a malformed value can never break
// the document, not as a general-purpose encoder.
inline std::string json_escape(const std::string& in) {
  std::string out;
  out.reserve(in.size() + 8);
  for (char c : in) {
    switch (c) {
      case '"':  out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n";  break;
      case '\r': out += "\\r";  break;
      case '\t': out += "\\t";  break;
      default:   out += c;      break;
    }
  }
  return out;
}

inline std::string fixed(double value, const char* fmt) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), fmt, value);
  return std::string(buf);
}

// ---------------------------------------------------------------------------
// telemetry
// ---------------------------------------------------------------------------

// The exact document validation.py stores: the four required measurements, a
// STRING node_id matching the topic, and the sample instant. Nothing else is
// emitted -- extra keys are tolerated by the backend but the contract asks for
// exactly these, and a smaller document cannot smuggle a unit mix-up past the
// range checks.
inline std::string telemetry_json(const std::string& node_id,
                                  double pm25,
                                  double temperature_c,
                                  double humidity_pct,
                                  double battery_v,
                                  const std::string& observed_utc) {
  std::string out;
  out.reserve(192);
  out += "{\"pm25\":";
  out += fixed(pm25, "%.1f");
  out += ",\"temperature_c\":";
  out += fixed(temperature_c, "%.2f");
  out += ",\"humidity_pct\":";
  out += fixed(humidity_pct, "%.2f");
  out += ",\"battery_v\":";
  out += fixed(battery_v, "%.3f");
  out += ",\"node_id\":\"";
  out += json_escape(node_id);
  out += "\",\"observed_utc\":\"";
  out += json_escape(observed_utc);
  out += "\"}";
  return out;
}

// ---------------------------------------------------------------------------
// events
// ---------------------------------------------------------------------------

// `nordtronics/wildfire/<base-id>/events` -- the one-level shape events.py
// subscribes (`EVENT_TOPIC_TEMPLATE`, `+` matches exactly one level). The older
// `<root>/event/alert` form had `event` where the base id belongs and was
// dropped by the broker, exactly as the telemetry topic was (task 0104 item 1).
inline std::string events_topic(const std::string& root, const std::string& base_id) {
  std::string r = root;
  while (!r.empty() && r.back() == '/') r.pop_back();
  return r + "/" + base_id + "/events";
}

// The backend's four kinds. The engine's NodeRiseConfirmed is an internal
// per-node signal with no backend counterpart, so it has no wire name and is
// never published. A Cleared event carries no level of its own -- the caller
// passes whether the level being cleared was an Alert (otherwise it was a
// Watch), which is the only thing that distinguishes the two cleared kinds.
inline const char* wire_event_kind(EventKind kind, bool cleared_was_alert) {
  switch (kind) {
    case EventKind::Watch:   return "watch_raised";
    case EventKind::Alert:   return "alert_raised";
    case EventKind::Cleared: return cleared_was_alert ? "alert_cleared" : "watch_cleared";
    case EventKind::NodeRiseConfirmed: return nullptr;
  }
  return nullptr;
}

// The subject of an event: a single participating node names itself, anything
// broader (a network alert, or an empty set) is `network`, events.py's default.
inline std::string event_subject(const std::vector<std::string>& nodes) {
  return (nodes.size() == 1) ? nodes[0] : std::string("network");
}

// The exact event document events.py stores. `event` is one of the four kinds
// and `observed_utc` is required; node_id and nodes are optional in the
// backend but are what the app's alert card renders, so both are always sent.
inline std::string event_json(const std::string& event,
                              const std::string& observed_utc,
                              const std::string& node_id,
                              const std::vector<std::string>& nodes) {
  std::string out;
  out.reserve(128);
  out += "{\"event\":\"";
  out += json_escape(event);
  out += "\",\"observed_utc\":\"";
  out += json_escape(observed_utc);
  out += "\",\"node_id\":\"";
  out += json_escape(node_id.empty() ? std::string("network") : node_id);
  out += "\",\"nodes\":[";
  for (std::size_t i = 0; i < nodes.size(); ++i) {
    if (i) out += ",";
    out += "\"";
    out += json_escape(nodes[i]);
    out += "\"";
  }
  out += "]}";
  return out;
}

}  // namespace wf
