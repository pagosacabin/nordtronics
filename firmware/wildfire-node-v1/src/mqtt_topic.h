// mqtt_topic.h -- the MQTT topic shape the deployed backend actually accepts.
//
// The ingest worker and the Mosquitto ACL on main both match
// `nordtronics/wildfire/+/telemetry`
//   backend/ingest/ingest/validation.py : TOPIC_TEMPLATE
//   backend/ingest/ingest/worker.py     : parse_topic() requires exactly 4
//                                         slash-separated parts
//   backend/mosquitto/acl               : `topic read nordtronics/wildfire/+/telemetry`
//   backend/DEPLOY.md sec. 5            : the per-user write ACL
//
// `+` matches exactly ONE level, so the older `<root>/node/<id>/telemetry` form
// was silently dropped twice over: the broker never matched it, and `parse_topic`
// saw five parts and returned None. WiFi joined, TLS up, and no live data would
// ever flow (task 0104 item 1, from 0100). There is one builder so the shape is
// stated in exactly one place and cannot drift per call site.
#pragma once

#include <string>

namespace wf {

// `("nordtronics/wildfire", "bench-01")` ->
// `"nordtronics/wildfire/bench-01/telemetry"`.
inline std::string telemetry_topic(const std::string& root, const std::string& node_id) {
  std::string r = root;
  while (!r.empty() && r.back() == '/') r.pop_back();
  return r + "/" + node_id + "/telemetry";
}

}  // namespace wf
