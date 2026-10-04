#include "role_detect.h"

#include <sstream>

namespace wf {

const char* role_name(Role r) { return r == Role::Base ? "base" : "node"; }

const char* role_source_name(RoleSource s) {
  switch (s) {
    case RoleSource::Nvs: return "NVS";
    case RoleSource::Probe: return "PROBE";
    case RoleSource::Default: return "DEFAULT";
  }
  return "?";
}

RoleDecision resolve_role(bool sensors_present, bool nvs_present, int nvs_role) {
  RoleDecision d;
  d.sensors_present = sensors_present;
  d.nvs_present = nvs_present;
  d.nvs_role = nvs_role;

  if (nvs_present && (nvs_role == 0 || nvs_role == 1)) {
    d.role = (nvs_role == 1) ? Role::Base : Role::Node;
    d.source = RoleSource::Nvs;
    return d;  // an explicit NVS/portal value always wins
  }

  if (sensors_present) {
    d.role = Role::Node;  // a board with node sensors on it is the node
    d.source = RoleSource::Probe;
    return d;
  }

  d.role = Role::Base;  // nothing answered the probe: this is the base station
  d.source = RoleSource::Default;
  return d;
}

std::string role_log_line(const RoleDecision& d) {
  std::ostringstream os;
  os << "ROLE: " << role_name(d.role) << " (source=" << role_source_name(d.source)
     << ", sensors=" << (d.sensors_present ? "present" : "absent")
     << ", nvs_role=" << (d.nvs_present ? (d.nvs_role == 1 ? "1" : "0") : "unset")
     << ")";
  return os.str();
}

}  // namespace wf
