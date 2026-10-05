// firmware_config.h -- the captive-portal field table (tank-monitor pattern:
// firmware default + portal field + NVS value) and the boot-time wiring facts.
//
// Portable C++17 with no Arduino dependency so test_portal_fields (host test)
// can assert, mechanically, that every portal field has a key, a default and an
// NVS namespace -- i.e. that "every portal field has a firmware default and
// persists to NVS" is a property of the table rather than a claim in a reply.
//
// NVS keys MUST be <= 15 characters (ESP32 Preferences limit). The table is
// checked for that too.
#pragma once

#include <cstddef>
#include <cstdint>

namespace wf {

// ---------------------------------------------------------------------------
// The 0079 frozen net list, encoded so the boot-time wiring rule is
// machine-checked (test/test_role_and_portal) rather than asserted in prose.
//
// Branch hermes/0079-wildfire-node-rev-c-interface-fixes @ 62e64ad: U1 (Heltec
// HTIT-WB32LAF V4.2) has 42 pins; pins 1..14 are connected and every one of
// those nets is frozen, pins 15..42 carry a no-connect flag.
//
// The boot role probe (task 0104 item 3) touches only pins from that frozen
// list: GPIO17/18 are the BME680/BME688 I2C bus and GPIO5/6 the PMS5003 UART,
// so the probe cannot reach a net the interface freeze did not already allow.
// ---------------------------------------------------------------------------
inline constexpr const char* kFrozenNets[] = {
    "BAT", "GND", "SOLAR", "3V3",  // U1 pins 1-4 (power / ground / panel)
    "GPIO36", "GPIO17", "GPIO18", "GPIO4", "GPIO5", "GPIO6",
    "GPIO33", "GPIO47", "GPIO48", "GPIO34",
};
constexpr size_t kFrozenNetCount = sizeof(kFrozenNets) / sizeof(kFrozenNets[0]);

// The GPIO numbers among the frozen nets.
inline constexpr int kFrozenGpioPins[] = {36, 17, 18, 4, 5, 6, 33, 47, 48, 34};
constexpr size_t kFrozenGpioCount = sizeof(kFrozenGpioPins) / sizeof(kFrozenGpioPins[0]);

// ESP32-S3 pins that carry a fixed function and must never be driven as a
// general-purpose input at boot.
inline constexpr int kS3BootSelectPins[] = {0, 3, 45, 46};  // boot / VDD_SPI / JTAG sel
inline constexpr int kS3NativeUsbPins[] = {19, 20};         // USB D- / D+
inline constexpr int kS3Uart0Pins[] = {43, 44};             // the serial boot log
inline constexpr int kLoraSpiPins[] = {8, 9, 10, 11, 12, 13, 14};

// ---------------------------------------------------------------------------
// Role selection: probed, not jumpered (task 0104 item 3).
//
// 0094 read a GPIO7 solder jumper to pick the role. Stephen's call replaced it
// with the tank-monitor pattern -- probe the node sensors and see what is
// actually fitted -- so there is no role pin any more and no hardware change is
// needed on a Rev C board. The probe (main.cpp, probe_node_sensors) answers
// from, in order:
//   * a BME680 (0x77) or BME688 (0x76) ACKing on the I2C bus, or
//   * a valid 32-byte PMS5003 frame arriving on the sensor UART.
// Sensors found => node; nothing found => base station. The NVS/portal `role`
// value still overrides both directions (src/role_detect.h).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Fixed wiring (Rev C, frozen -- do not flip; bench-verified 2026-10-02).
// ---------------------------------------------------------------------------
constexpr int kI2cSdaPin = 17;   // BME680 SCK/SDA  (bench-verified)
constexpr int kI2cSclPin = 18;   // BME680 SDI/SCL  (bench-verified)
constexpr int kOledRstPin = 21;  // SSD1306 reset -- NOT tied to the ESP32 reset
// Vext is the switched 3.3 V rail the panel and the sensor share on this board
// family. Heltec LoRa 32 V4 gates it on GPIO36, ACTIVE LOW (U1 pin 5 -- already
// a 0079 frozen net, so driving it adds no net to the freeze). The bench
// BME680's VIN sits on Vext; while the rail is off the chip sits at 0 V and
// clamps SDA, which takes the whole bus down and blinds probe_node_sensors().
// It is driven LOW at the top of setup(), before any I2C init (task 0108; the
// legacy bench firmware did the same: node.cpp PIN_VEXT 36, OUTPUT, LOW).
constexpr int kOledVextPin = 36;
constexpr uint8_t kBme680Addr = 0x77;
// The BME688 shares the BME680's register map and answers at 0x76 on the same
// board family, so the boot role probe asks for both addresses.
constexpr uint8_t kBme688Addr = 0x76;
constexpr uint8_t kSsd1306Addr = 0x3C;
constexpr int kPmsUartTxPin = 5; // PMS5003 TX -> 1k -> GPIO5 (bench-verified)
constexpr int kPmsUartRxPin = 6; // PMS5003 RX <- GPIO6 (U1 pin 10)
constexpr int kLoraNssPin = 8;
constexpr int kLoraDio1Pin = 14;
constexpr int kLoraRstPin = 12;
constexpr int kLoraBusyPin = 13;

enum class FieldKind : uint8_t { Str = 0, Int = 1, Bool = 2 };

struct PortalField {
  const char* key;          // NVS key, <= 15 chars
  const char* label;        // captive-portal label
  FieldKind kind;
  const char* default_text; // firmware default, as text
  const char* nvs_namespace; // Preferences namespace
};

// Every portal field, in portal order. The portal renders exactly this list and
// the NVS layer reads/writes exactly these keys -- there is no field that exists
// in one and not the other.
constexpr const char* kPrefsNamespace = "wildfire";

// The captive portal's own AP name. This is a fixed firmware constant, not a
// network credential and not a site value: it is the name the operator joins to
// reach http://192.168.4.1 before any site configuration exists. The site
// network name and the MQTT password are NVS fields written at the bench by the
// portal and live in no repository file (task 0099 constraints).
constexpr const char* kPortalApName = "wildfire-setup";

inline constexpr PortalField kPortalFields[] = {
    {"role",        "Radio role (0 node / 1 base; blank = auto-detect)", FieldKind::Str,  "", "wildfire"},
    {"node_id",     "Node ID (1..65534; 0 = unprovisioned)",       FieldKind::Int,  "0", "wildfire"},
    {"prov_ids",    "Provisioned node IDs (csv, base allowlist)",   FieldKind::Str,  "", "wildfire"},
    {"wifi_ssid",   "WiFi SSID",                                   FieldKind::Str,  "", "wildfire"},
    {"wifi_pass",   "WiFi password",                               FieldKind::Str,  "", "wildfire"},
    // The uplink is TLS-only: the deployed broker listens on 8883 and has no
    // 1883 listener, and the uplink is a WiFiClientSecure with the ISRG Root X1
    // trust anchor pinned (src/mqtt_ca.h). The old defaults here
    // (api.nordtronics.io:1883) were plaintext and pointed at a Cloudflare edge
    // with nothing listening on either port (task 0099 item 7b).
    {"mqtt_host",   "MQTT broker host",                            FieldKind::Str,  "mqtt.nordtronics.io", "wildfire"},
    {"mqtt_port",   "MQTT broker port (TLS)",                      FieldKind::Int,  "8883", "wildfire"},
    {"mqtt_user",   "MQTT username",                               FieldKind::Str,  "", "wildfire"},
    {"mqtt_pass",   "MQTT password",                               FieldKind::Str,  "", "wildfire"},
    {"mqtt_root",   "MQTT topic root",                             FieldKind::Str,  "nordtronics/wildfire", "wildfire"},
    {"offl_policy", "Offline policy (buffer|drop)",                FieldKind::Str,  "buffer", "wildfire"},
    {"offl_cap",    "Offline buffer cap (records)",                FieldKind::Int,  "180", "wildfire"},
    // Bench phase (task 0112): 60 s, so a bench capture sees repeated
    // sample/report cycles. The FINAL PRE-DEPLOYMENT GATE restores "720" (12
    // minutes) together with kDeepSleepEnabled = true in this header.
    {"chk_s",       "Routine check-in period (s)",                 FieldKind::Int,  "60", "wildfire"},
    {"lora_mhz",    "LoRa frequency (MHz, 915 US only)",           FieldKind::Str,  "915.0", "wildfire"},
    {"lora_bw",     "LoRa bandwidth (kHz)",                        FieldKind::Str,  "125.0", "wildfire"},
    {"lora_sf",     "LoRa spreading factor",                       FieldKind::Int,  "7", "wildfire"},
    {"lora_cr",     "LoRa coding rate (5..8)",                     FieldKind::Int,  "5", "wildfire"},
    {"lora_sync",   "LoRa sync word (private, hex)",               FieldKind::Str,  "0x12", "wildfire"},
    {"lora_dbm",    "LoRa TX power (dBm)",                         FieldKind::Int,  "20", "wildfire"},
    {"ack_to_s",    "Alarm ACK timeout (s)",                       FieldKind::Int,  "5", "wildfire"},
    {"ack_tries",   "Alarm retries without ACK",                   FieldKind::Int,  "3", "wildfire"},
    {"corr_min",    "Correlation window (min, 10..60)",            FieldKind::Int,  "20", "wildfire"},
    {"abs_floor",   "Absolute floor (ug/m3)",                      FieldKind::Str,  "25.0", "wildfire"},
    {"rel_delta",   "Rise above frozen baseline (ug/m3)",          FieldKind::Str,  "25.0", "wildfire"},
    {"clear_min",   "Auto-clear below-threshold time (min)",       FieldKind::Int,  "30", "wildfire"},
    {"rearm_min",   "Post-clear re-arm cooldown (min)",            FieldKind::Int,  "30", "wildfire"},
    {"baseline_n",  "Baseline window (readings)",                  FieldKind::Int,  "8", "wildfire"},
    {"live_min",    "Node live age (min)",                         FieldKind::Int,  "30", "wildfire"},
    {"batt_low_mv", "Low-battery threshold (mV)",                  FieldKind::Int,  "3400", "wildfire"},
    {"pms_baud",    "PMS5003 baud",                                FieldKind::Int,  "9600", "wildfire"},
};

constexpr size_t kPortalFieldCount = sizeof(kPortalFields) / sizeof(kPortalFields[0]);

// Offline policy: the base BUFFERS with a cap and drops the OLDEST record when
// the cap is reached (a rolling window of the most recent telemetry), rather
// than dropping new data. Sized at offl_cap = 180 records = 15 node-hours for one
// node = 36 h for a 4-node property at the 12-minute cadence, and 30 kB of the
// ESP32-S3's 2 MB PSRAM. See docs/wildfire/radio-protocol-v1.md section 8.
constexpr bool kOfflineBufferDropOldest = true;

// ---------------------------------------------------------------------------
// FINAL PRE-DEPLOYMENT GATE -- node deep sleep and the routine check-in period.
//
// Bench phase (task 0112): the node stays AWAKE. With kDeepSleepEnabled ==
// false the node skips enter_deep_sleep() and instead idles for one check-in
// period with USB-serial live, so a bench capture sees repeated sample/report
// cycles instead of the node vanishing off the USB bus for 12 minutes at a
// time. The sleep code itself and the base's HARD-RULE refusal are untouched:
// this constant only gates whether the NODE CALLS enter_deep_sleep(). It is a
// gate, not a removal.
//
// THE FINAL GATE, immediately before the system is declared field-ready: set
// kDeepSleepEnabled back to `true` and kCheckinSecondsDefault back to 720 (and
// the portal default for chk_s back to "720" with it) so the field power budget
// -- one wake, one LoRa report, then a 12-minute sleep -- is restored. The two
// values are paired: 60 s is a bench convenience only, never a field value, and
// a field build must never carry the 60 s interval with the sleep gate open.
// ---------------------------------------------------------------------------
constexpr bool kDeepSleepEnabled = false;   // bench phase: node does not sleep
constexpr int kCheckinSecondsDefault = 60;  // bench phase default (field: 720)
constexpr int kCheckinSecondsField = 720;   // the field value the gate restores

}  // namespace wf
