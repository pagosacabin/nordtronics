// role_detect.h -- boot-time role detection, in portable C++ so the truth table
// (sensor probe / NVS override / default) and the exact serial line are
// unit-testable without hardware.
//
// Order of authority, per task 0104 item 3 (Stephen's call -- no hardware mods,
// the GPIO7 solder jumper from 0094 is gone):
//   1. NVS "role" value, when set                      -> source NVS
//   2. the node sensor probe, when it found sensors    -> source PROBE (node)
//   3. nothing answered on the buses                   -> source DEFAULT (base)
//
// The role is PROBED, not jumpered: the tank-monitor pattern. A board carrying
// a BME680/BME688 on the I2C bus (0x77/0x76) or a PMS5003 streaming on the UART
// is a node; a bare board is the base station. NVS/portal still wins in both
// directions, which is what a bench board with no sensors fitted needs in order
// to be forced into either role.
#pragma once

#include <cstdint>
#include <string>

namespace wf {

enum class Role : uint8_t { Node = 0, Base = 1 };

enum class RoleSource : uint8_t {
  Nvs = 0,      // an NVS/portal "role" value decided
  Probe = 1,    // the boot probe found node sensors
  Default = 2,  // nothing answered the probe; the base default applied
};

struct RoleDecision {
  Role role = Role::Base;
  RoleSource source = RoleSource::Default;
  bool sensors_present = false;  // the boot probe found a node sensor
  bool nvs_present = false;      // an NVS/portal role value exists
  int nvs_role = -1;             // 0 = node, 1 = base, -1 = unset
};

// Pure decision. `sensors_present`: the boot probe answered (a BME680/BME688 on
// I2C or a valid PMS5003 frame on the UART). `nvs_present` + `nvs_role`: the
// persisted override, if any.
RoleDecision resolve_role(bool sensors_present, bool nvs_present, int nvs_role);

// The exact line printed to serial on every boot, e.g.
//   ROLE: base (source=DEFAULT, sensors=absent, nvs_role=unset)
std::string role_log_line(const RoleDecision& d);

const char* role_name(Role r);
const char* role_source_name(RoleSource s);

}  // namespace wf
