// test_main.cpp -- host-side proof for the two non-sensor rules:
//
//   1. role detection: the strap / NVS-override / default truth table AND the
//      exact serial line each case prints ("both roles demonstrated from the
//      SAME binary" is a property of this table, not of a bench photo);
//   2. the portal field table: every field has a key, a label and a default, and
//      the strap pin provably does not collide with any 0079 frozen net.

#include <unity.h>

#include <cstdio>
#include <cstring>
#include <set>
#include <string>

#include "consensus_v02.h"
#include "firmware_config.h"
#include "role_detect.h"

using wf::kFrozenGpioPins;
using wf::kFrozenGpioCount;
using wf::kRoleStrapPin;
using wf::Role;
using wf::RoleDecision;
using wf::RoleSource;

static int g_failures = 0;

#define CHECK(cond, msg)                                       \
  do {                                                         \
    if (!(cond)) {                                             \
      g_failures++;                                            \
      printf("    FAIL: %s\n", (msg));                         \
      TEST_ASSERT_TRUE_MESSAGE((cond), (msg));                 \
    }                                                          \
  } while (0)

static bool in_set(const int* xs, size_t n, int v) {
  for (size_t i = 0; i < n; ++i) {
    if (xs[i] == v) return true;
  }
  return false;
}

// --------------------------------------------------------------------------
// role detection
// --------------------------------------------------------------------------
static void test_role_truth_table() {
  // strap LOW (jumper to GND) -> base, source STRAP
  RoleDecision d = wf::resolve_role(wf::kStrapLevelBase, false, -1);
  CHECK(d.role == Role::Base && d.source == RoleSource::Strap,
        "strap low must select base from STRAP");

  // strap HIGH (open, internal pull-up) -> the bench default, node
  d = wf::resolve_role(wf::kStrapLevelNode, false, -1);
  CHECK(d.role == Role::Node && d.source == RoleSource::Default,
        "unstrapped board must default to node");

  // NVS override wins over the strap, both directions
  d = wf::resolve_role(wf::kStrapLevelNode, true, 1);
  CHECK(d.role == Role::Base && d.source == RoleSource::Nvs,
        "NVS role=1 must force base even with no jumper");
  d = wf::resolve_role(wf::kStrapLevelBase, true, 0);
  CHECK(d.role == Role::Node && d.source == RoleSource::Nvs,
        "NVS role=0 must force node even with the jumper fitted");

  // an NVS value that is present but out of range falls through to the strap
  d = wf::resolve_role(wf::kStrapLevelBase, true, -1);
  CHECK(d.role == Role::Base && d.source == RoleSource::Strap,
        "an unusable NVS value must not mask the strap");
  d = wf::resolve_role(wf::kStrapLevelBase, true, 7);
  CHECK(d.role == Role::Base && d.source == RoleSource::Strap,
        "an out-of-range NVS value must not mask the strap");

  printf("[role] STRAP=%s / STRAP2 / NVS->base / NVS->node / NVS-invalid-falls-through: all 5 cases hold\n",
         wf::role_name(wf::resolve_role(0, false, -1).role));
}

static void test_role_serial_lines() {
  // The exact strings the device prints on every boot. These are what a bench
  // capture of "the same binary in both roles" has to show.
  const std::string base_strap =
      wf::role_log_line(wf::resolve_role(wf::kStrapLevelBase, false, -1), kRoleStrapPin);
  const std::string node_default =
      wf::role_log_line(wf::resolve_role(wf::kStrapLevelNode, false, -1), kRoleStrapPin);
  const std::string base_nvs =
      wf::role_log_line(wf::resolve_role(wf::kStrapLevelNode, true, 1), kRoleStrapPin);

  printf("[role-log] %s\n", base_strap.c_str());
  printf("[role-log] %s\n", node_default.c_str());
  printf("[role-log] %s\n", base_nvs.c_str());

  CHECK(base_strap == "ROLE: base (source=STRAP, strap_pin=GPIO7 level=0, nvs_role=unset)",
        "strap case must print the documented line");
  CHECK(node_default == "ROLE: node (source=DEFAULT, strap_pin=GPIO7 level=1, nvs_role=unset)",
        "default case must print the documented line");
  CHECK(base_nvs == "ROLE: base (source=NVS, strap_pin=GPIO7 level=1, nvs_role=1)",
        "NVS case must print the documented line");
}

// --------------------------------------------------------------------------
// strap pin against the 0079 frozen nets
// --------------------------------------------------------------------------
static void test_strap_pin_does_not_collide() {
  printf("[strap] pin=GPIO%d frozen_nets=%zu\n", kRoleStrapPin, wf::kFrozenNetCount);
  CHECK(wf::kFrozenNetCount == 14, "the 0079 capture froze 14 connected U1 pins");
  CHECK(wf::kFrozenGpioCount == 10, "10 of the 14 frozen nets are GPIOs");
  CHECK(!in_set(kFrozenGpioPins, kFrozenGpioCount, kRoleStrapPin),
        "the strap pin must not be one of the 14 frozen nets");
  CHECK(!in_set(wf::kS3BootStrappingPins, 4, kRoleStrapPin),
        "the strap pin must not be an ESP32-S3 boot strapping pin");
  CHECK(!in_set(wf::kS3NativeUsbPins, 2, kRoleStrapPin),
        "the strap pin must not be a native USB pin");
  CHECK(!in_set(wf::kS3Uart0Pins, 2, kRoleStrapPin),
        "the strap pin must not be UART0");
  CHECK(!in_set(wf::kLoraSpiPins, 7, kRoleStrapPin),
        "the strap pin must not be in the SX1262 SPI block");
}

// --------------------------------------------------------------------------
// portal table
// --------------------------------------------------------------------------
static void test_every_portal_field_has_a_key_default_and_namespace() {
  std::set<std::string> keys;
  int with_nonempty_default = 0;
  for (size_t i = 0; i < wf::kPortalFieldCount; ++i) {
    const wf::PortalField& f = wf::kPortalFields[i];
    CHECK(f.key != nullptr && f.key[0] != '\0', "every portal field needs an NVS key");
    CHECK(f.label != nullptr && f.label[0] != '\0', "every portal field needs a portal label");
    CHECK(f.default_text != nullptr, "every portal field needs a firmware default");
    CHECK(f.nvs_namespace != nullptr && strcmp(f.nvs_namespace, wf::kPrefsNamespace) == 0,
          "every portal field must persist into the single wildfire namespace");
    CHECK(strlen(f.key) <= 15, "NVS keys must fit the ESP32 Preferences 15-char limit");
    CHECK(keys.insert(std::string(f.key)).second, "portal keys must be unique");
    if (f.default_text[0] != '\0') with_nonempty_default++;
  }
  printf("[portal] fields=%zu unique_keys=%zu non-empty_defaults=%d (blank defaults are "
         "intentional: role = follow the strap, ssid/credentials = site-specific)\n",
         wf::kPortalFieldCount, keys.size(), with_nonempty_default);
  CHECK(wf::kPortalFieldCount >= 25, "the portal exposes every tunable in the protocol doc");
  CHECK(with_nonempty_default >= 20, "all but the site-specific fields carry a firmware default");

  // the fields the task and the protocol doc name explicitly
  const char* required[] = {"role", "node_id", "prov_ids", "mqtt_host", "mqtt_port",
                            "mqtt_user", "mqtt_pass", "mqtt_root", "offl_policy",
                            "offl_cap", "chk_s", "lora_mhz", "lora_bw", "lora_sf",
                            "lora_cr", "lora_sync", "lora_dbm", "ack_to_s", "ack_tries",
                            "corr_min", "abs_floor", "rel_delta", "clear_min",
                            "rearm_min", "baseline_n", "live_min"};
  for (const char* r : required) {
    CHECK(keys.count(r) == 1, "a required portal field is missing");
  }
}

static void test_defaults_match_the_frozen_rule_constants() {
  // The portal default for every detection parameter must equal the constant the
  // engine was tested with, or the device quietly runs different rules.
  wf::ConsensusConfig cc;  // consensus_v02.h defaults == the tested values
  int corr = -1, chk = -1, cap = -1;
  float floor_v = -1.0f, delta = -1.0f;
  for (size_t i = 0; i < wf::kPortalFieldCount; ++i) {
    const wf::PortalField& f = wf::kPortalFields[i];
    if (!strcmp(f.key, "corr_min")) corr = atoi(f.default_text);
    if (!strcmp(f.key, "chk_s")) chk = atoi(f.default_text);
    if (!strcmp(f.key, "offl_cap")) cap = atoi(f.default_text);
    if (!strcmp(f.key, "abs_floor")) floor_v = (float)atof(f.default_text);
    if (!strcmp(f.key, "rel_delta")) delta = (float)atof(f.default_text);
  }
  printf("[defaults] corr_min=%d abs_floor=%.1f rel_delta=%.1f chk_s=%d offl_cap=%d\n",
         corr, floor_v, delta, chk, cap);
  CHECK(corr == (int)cc.correlation_window_min, "corr_min default must equal the tested window");
  CHECK(corr >= 10 && corr <= 60, "corr_min default must sit inside the portal range 10..60");
  CHECK(floor_v == cc.abs_floor_ugm3, "abs_floor default must equal the tested floor");
  CHECK(delta == cc.rel_delta_ugm3, "rel_delta default must equal the tested delta");
  CHECK(chk == 720, "the routine check-in default must be 12 minutes (720 s)");
  CHECK(cap == 180, "the offline buffer default must be 180 records");
  CHECK(wf::kOfflineBufferDropOldest, "the offline policy must be buffer-with-cap, drop-oldest");

  // the offline policy the table advertises must match the flag
  for (size_t i = 0; i < wf::kPortalFieldCount; ++i) {
    if (!strcmp(wf::kPortalFields[i].key, "offl_policy")) {
      CHECK(std::string(wf::kPortalFields[i].default_text) == "buffer",
            "offl_policy default must be 'buffer' (the documented choice)");
    }
  }
}

void setUp() {}
void tearDown() {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_role_truth_table);
  RUN_TEST(test_role_serial_lines);
  RUN_TEST(test_strap_pin_does_not_collide);
  RUN_TEST(test_every_portal_field_has_a_key_default_and_namespace);
  RUN_TEST(test_defaults_match_the_frozen_rule_constants);
  const int rc = UNITY_END();
  printf("ROLE-PORTAL-TEST SUMMARY: checks-failed=%d\n", g_failures);
  printf("ROLE-PORTAL-TEST: %s\n", (rc == 0 && g_failures == 0) ? "PASS" : "FAIL");
  return (rc == 0 && g_failures == 0) ? 0 : 1;
}
