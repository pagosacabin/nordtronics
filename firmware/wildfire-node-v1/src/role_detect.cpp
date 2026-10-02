#include "role_detect.h"

#include <sstream>

namespace wf {

const char* role_name(Role r) { return r == Role::Base ? "base" : "node"; }

const char* role_source_name(RoleSource s) {
  switch (s) {
    case RoleSource::Strap: return "STRAP";
    case RoleSource::Nvs: return "NVS";
    case RoleSource::Default: return "DEFAULT";
  }
  return "?";
}

RoleDecision resolve_role(int strap_level, bool nvs_present, int nvs_role) {
  RoleDecision d;
  d.strap_level = strap_level;
  d.nvs_present = nvs_present;
  d.nvs_role = nvs_role;

  if (nvs_present && (nvs_role == 0 || nvs_role == 1)) {
    d.role = (nvs_role == 1) ? Role::Base : Role::Node;
    d.source = RoleSource::Nvs;
    return d;  // an explicit NVS/portal value always wins
  }

  if (strap_level == 0) {
    d.role = Role::Base;
    d.source = RoleSource::Strap;
    return d;
  }

  d.role = Role::Node;  // unstrapped bench board
  d.source = RoleSource::Default;
  return d;
}

std::string role_log_line(const RoleDecision& d, int strap_pin) {
  std::ostringstream os;
  os << "ROLE: " << role_name(d.role) << " (source=" << role_source_name(d.source)
     << ", strap_pin=GPIO" << strap_pin << " level=" << d.strap_level
     << ", nvs_role=" << (d.nvs_present ? (d.nvs_role == 1 ? "1" : "0") : "unset")
     << ")";
  return os.str();
}

}  // namespace wf
