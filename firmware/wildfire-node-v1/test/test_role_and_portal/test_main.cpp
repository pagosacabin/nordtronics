// test_main.cpp -- host-side proof for the two non-sensor rules:
//
//   1. role detection (task 0104 item 3): the sensor-probe / NVS-override /
//      default truth table AND the exact serial line each case prints ("both
//      roles demonstrated from the SAME binary" is a property of this table,
//      not of a bench photo);
//   2. the portal field table: every field has a key, a label and a default, and
//      the pins the boot probe drives provably come from the 0079 frozen nets.
//
// Task 0104 item 3 replaced 0094's GPIO7 role jumper with the tank-monitor
// probe, so the truth table below is the probe's: a board with node sensors on
// the bus is the node, a board with none is the base station, a valid NVS value
// overrides both ways, and an unusable NVS value falls through to the probe.

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
// role detection (task 0104 item 3: probe, not jumper)
// --------------------------------------------------------------------------
static void test_role_truth_table() {
  // sensors found on the bus -> node, source PROBE
  RoleDecision d = wf::resolve_role(true, false, -1);
  CHECK(d.role == Role::Node && d.source == RoleSource::Probe,
        "sensors present must select node from PROBE");

  // nothing answered -> the base station
  d = wf::resolve_role(false, false, -1);
  CHECK(d.role == Role::Base && d.source == RoleSource::Default,
        "no sensors must fall through to base");

  // a valid NVS override wins over the probe, both directions
  d = wf::resolve_role(false, true, 0);
  CHECK(d.role == Role::Node && d.source == RoleSource::Nvs,
        "NVS role=0 must force node on a board with no sensors");
  d = wf::resolve_role(true, true, 1);
  CHECK(d.role == Role::Base && d.source == RoleSource::Nvs,
        "NVS role=1 must force base on a board with sensors");

  // an NVS value that is present but unusable falls through to the probe
  d = wf::resolve_role(true, true, -1);
  CHECK(d.role == Role::Node && d.source == RoleSource::Probe,
        "an unusable NVS value must not mask the probe");
  d = wf::resolve_role(false, true, 7);
  CHECK(d.role == Role::Base && d.source == RoleSource::Default,
        "an out-of-range NVS value must fall through to the probe");

  printf("[role] probe->node / none->base / NVS->base / NVS->node / "
         "NVS-invalid-falls-through: all 6 cases hold\n");
}

static void test_role_serial_lines() {
  // The exact strings the device prints on every boot. These are what a bench
  // capture of "the same binary in both roles" has to show.
  const std::string probe_node = wf::role_log_line(wf::resolve_role(true, false, -1));
  const std::string no_sensors = wf::role_log_line(wf::resolve_role(false, false, -1));
  const std::string base_nvs = wf::role_log_line(wf::resolve_role(true, true, 1));

  printf("[role-log] %s\n", probe_node.c_str());
  printf("[role-log] %s\n", no_sensors.c_str());
  printf("[role-log] %s\n", base_nvs.c_str());

  CHECK(probe_node == "ROLE: node (source=PROBE, sensors=present, nvs_role=unset)",
        "probe case must print the documented line");
  CHECK(no_sensors == "ROLE: base (source=DEFAULT, sensors=absent, nvs_role=unset)",
        "base case must print the documented line");
  CHECK(base_nvs == "ROLE: base (source=NVS, sensors=present, nvs_role=1)",
        "NVS case must print the documented line");
}

// --------------------------------------------------------------------------
// the pins the boot probe drives, against the 0079 frozen nets
// --------------------------------------------------------------------------
// 0094 asserted that its role pin avoided the frozen nets; that pin is gone
// (item 3) and the probe drives the sensor buses instead, which are ON the
// frozen list by design. Assert exactly that, so a later change cannot quietly
// move a boot probe onto a special-function or unfrozen pin.
static void test_boot_probe_pins_are_frozen_sensor_nets() {
  const int probe_pins[] = {wf::kI2cSdaPin, wf::kI2cSclPin,
                            wf::kPmsUartTxPin, wf::kPmsUartRxPin};
  printf("[probe] i2c=%d/%d pms=%d/%d frozen_nets=%zu\n", wf::kI2cSdaPin,
         wf::kI2cSclPin, wf::kPmsUartTxPin, wf::kPmsUartRxPin, wf::kFrozenNetCount);
  CHECK(wf::kFrozenNetCount == 14, "the 0079 capture froze 14 connected U1 pins");
  CHECK(wf::kFrozenGpioCount == 10, "10 of the 14 frozen nets are GPIOs");
  for (const int p : probe_pins) {
    CHECK(in_set(kFrozenGpioPins, kFrozenGpioCount, p),
          "a boot-probe pin must be on the 0079 frozen net list");
    CHECK(!in_set(wf::kS3BootSelectPins, 4, p),
          "a boot-probe pin must not be an ESP32-S3 boot-select pin");
    CHECK(!in_set(wf::kS3NativeUsbPins, 2, p),
          "a boot-probe pin must not be a native USB pin");
    CHECK(!in_set(wf::kS3Uart0Pins, 2, p), "a boot-probe pin must not be UART0");
    CHECK(!in_set(wf::kLoraSpiPins, 7, p),
          "a boot-probe pin must not be in the SX1262 SPI block");
  }
}

// --------------------------------------------------------------------------
// the Vext power gate driven at boot (task 0108)
// --------------------------------------------------------------------------
// setup() drives the switched sensor rail (GPIO36, active LOW) before any I2C
// init, because an unpowered chip on the bus clamps SDA. Assert the pin facts
// the change relies on: it is the documented GPIO36, it is on the 0079 frozen
// net list (so driving it adds no net to the freeze), and it is none of the
// ESP32-S3 special-function pins the header enumerates.
static void test_vext_gate_pin() {
  printf("[vext] pin=%d (kOledVextPin) frozen_gpios=%zu\n", wf::kOledVextPin,
         wf::kFrozenGpioCount);
  CHECK(wf::kOledVextPin == 36, "the Vext gate is GPIO36 on the Heltec V4");
  CHECK(in_set(kFrozenGpioPins, kFrozenGpioCount, wf::kOledVextPin),
        "the Vext gate must be a 0079 frozen net (U1 pin 5)");
  CHECK(!in_set(wf::kS3BootSelectPins, 4, wf::kOledVextPin),
        "the Vext gate must not be an ESP32-S3 boot-select pin");
  CHECK(!in_set(wf::kS3NativeUsbPins, 2, wf::kOledVextPin),
        "the Vext gate must not be a native USB pin");
  CHECK(!in_set(wf::kS3Uart0Pins, 2, wf::kOledVextPin),
        "the Vext gate must not be UART0");
  CHECK(!in_set(wf::kLoraSpiPins, 7, wf::kOledVextPin),
        "the Vext gate must not be in the SX1262 SPI block");
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
         "intentional: role = auto-detect, ssid/credentials = site-specific)\n",
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
  RUN_TEST(test_boot_probe_pins_are_frozen_sensor_nets);
  RUN_TEST(test_vext_gate_pin);
  RUN_TEST(test_every_portal_field_has_a_key_default_and_namespace);
  RUN_TEST(test_defaults_match_the_frozen_rule_constants);
  const int rc = UNITY_END();
  printf("ROLE-PORTAL-TEST SUMMARY: checks-failed=%d\n", g_failures);
  printf("ROLE-PORTAL-TEST: %s\n", (rc == 0 && g_failures == 0) ? "PASS" : "FAIL");
  return (rc == 0 && g_failures == 0) ? 0 : 1;
}
