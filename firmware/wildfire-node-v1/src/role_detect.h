// role_detect.h -- boot-time role detection, in portable C++ so the truth table
// (strap / NVS override / default) and the exact serial line are unit-testable
// without hardware.
//
// Order of authority, per task 0094 item 0:
//   1. NVS "role" value, when set  -> source NVS
//   2. the strap pin, when it is pulled low -> source STRAP
//   3. otherwise the compile-time default (node) -> source DEFAULT
//
// The strap is a solder jumper on Rev C (SHORT to GND = base). An unstrapped
// board reads HIGH through the internal pull-up, which is indistinguishable from
// "no jumper" -- that is deliberate: an unstrapped bench board must come up as a
// NODE, and the NVS/portal value is the only way to force base on a board with no
// jumper fitted.
#pragma once

#include <cstdint>
#include <string>

namespace wf {

enum class Role : uint8_t { Node = 0, Base = 1 };

enum class RoleSource : uint8_t {
  Strap = 0,    // the strap pin decided
  Nvs = 1,      // an NVS/portal "role" value overrode the strap
  Default = 2,  // nothing set it; the bench default (node) applied
};

struct RoleDecision {
  Role role = Role::Node;
  RoleSource source = RoleSource::Default;
  int strap_level = 1;      // 1 = HIGH (open), 0 = LOW (jumper to GND)
  bool nvs_present = false; // an NVS/portal role value exists
  int nvs_role = -1;        // 0 = node, 1 = base, -1 = unset
};

// Pure decision. `strap_level`: 0 = LOW/jumper fitted, 1 = HIGH/open.
// `nvs_present` + `nvs_role`: the persisted override, if any.
RoleDecision resolve_role(int strap_level, bool nvs_present, int nvs_role);

// The exact line printed to serial on every boot, e.g.
//   ROLE: base (source=NVS, strap_pin=GPIO7 level=1, nvs_role=1)
std::string role_log_line(const RoleDecision& d, int strap_pin);

const char* role_name(Role r);
const char* role_source_name(RoleSource s);

}  // namespace wf
