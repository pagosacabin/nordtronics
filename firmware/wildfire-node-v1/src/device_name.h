// device_name.h -- the DHCP hostname a board asks its router for (task 0104
// item 2, Stephen's ask).
//
// `wildfire-<role>-<nn>`: `role` is `base` or `node`, `nn` is the node id
// zero-padded to two digits, and `01` stands in when the board is still
// unprovisioned (`node_id` 0). That makes a board identifiable in any router's
// client list -- no OLED and no serial console needed.
//
// The name is derived from the role decision plus the node id, so it adds NO
// portal field and NO NVS key: an operator who renames a board would be adding
// a credential-shaped string nobody reads back, and the two inputs already
// exist.
#pragma once

#include <cstdio>
#include <string>

namespace wf {

inline std::string device_hostname(const char* role, unsigned node_id) {
  char buf[32];
  snprintf(buf, sizeof(buf), "wildfire-%s-%02u", (role != nullptr && *role != '\0') ? role : "node",
           node_id == 0 ? 1u : node_id);
  return std::string(buf);
}

}  // namespace wf
