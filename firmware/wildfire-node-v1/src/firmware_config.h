// firmware_config.h -- the captive-portal field table (tank-monitor pattern:
// firmware default + portal field + NVS value) and the strap-pin choice.
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
// The 0079 frozen net list, encoded so the strap-pin rule is machine-checked
// (test/test_role_and_portal) rather than asserted in prose.
//
// Branch hermes/0079-wildfire-node-rev-c-interface-fixes @ 62e64ad: U1 (Heltec
// HTIT-WB32LAF V4.2) has 42 pins; pins 1..14 are connected and every one of
// those nets is frozen, pins 15..42 carry a no-connect flag.
// ---------------------------------------------------------------------------
inline constexpr const char* kFrozenNets[] = {
    "BAT", "GND", "SOLAR", "3V3",  // U1 pins 1-4 (power / ground / panel)
    "GPIO36", "GPIO17", "GPIO18", "GPIO4", "GPIO5", "GPIO6",
    "GPIO33", "GPIO47", "GPIO48", "GPIO34",
};
constexpr size_t kFrozenNetCount = sizeof(kFrozenNets) / sizeof(kFrozenNets[0]);

// The GPIO numbers among the frozen nets: the strap pin must not be one of them.
inline constexpr int kFrozenGpioPins[] = {36, 17, 18, 4, 5, 6, 33, 47, 48, 34};
constexpr size_t kFrozenGpioCount = sizeof(kFrozenGpioPins) / sizeof(kFrozenGpioPins[0]);

// ESP32-S3 pins that are unusable as a boot strap for other reasons.
inline constexpr int kS3BootStrappingPins[] = {0, 3, 45, 46};  // boot / VDD_SPI / JTAG sel
inline constexpr int kS3NativeUsbPins[] = {19, 20};            // USB D- / D+
inline constexpr int kS3Uart0Pins[] = {43, 44};                // the serial boot log
inline constexpr int kLoraSpiPins[] = {8, 9, 10, 11, 12, 13, 14};

// ---------------------------------------------------------------------------
// Strap pin.
//
// Chosen against the 0079 frozen net list (branch
// hermes/0079-wildfire-node-rev-c-interface-fixes @ 62e64ad): the 14 connected
// Heltec pins are 1 BAT, 2 GND, 3 SOLAR, 4 3V3, 5 GPIO36, 6 GPIO17, 7 GPIO18,
// 8 GPIO4, 9 GPIO5, 10 GPIO6, 11 GPIO33, 12 GPIO47, 13 GPIO48, 14 GPIO34.
//
// GPIO7 is NOT among them (and is not one of the 28 no-connect pins either), so
// adding the strap changes no frozen net. It is also free of the ESP32-S3
// special functions that would make an input strap a bad idea: GPIO0/GPIO3/
// GPIO45/GPIO46 are boot strapping pins, GPIO19/GPIO20 are native USB D-/D+,
// GPIO43/GPIO44 are UART0 (the serial log), GPIO8..GPIO14 are the SX1262 SPI
// block, and GPIO17/GPIO18 are the I2C bus (BME680 @0x77 + SSD1306 @0x3C).
// GPIO7 needs only an internal pull-up (open = node) and a jumper to GND (base).
// Rev C carries the jumper as a documented "solder jumper" future change; for
// bench work the NVS/portal override covers an unstrapped board, as the task
// allows. GPIO7's availability on the V4.2 header is listed as a bench
// verification item in firmware/wildfire-node-v1/README.md.
// ---------------------------------------------------------------------------
constexpr int kRoleStrapPin = 7;
constexpr int kStrapLevelBase = 0;  // jumper to GND == base
constexpr int kStrapLevelNode = 1;  // internal pull-up, open == node

// ---------------------------------------------------------------------------
// Fixed wiring (Rev C, frozen -- do not flip; bench-verified 2026-10-02).
// ---------------------------------------------------------------------------
constexpr int kI2cSdaPin = 17;   // BME680 SCK/SDA  (bench-verified)
constexpr int kI2cSclPin = 18;   // BME680 SDI/SCL  (bench-verified)
constexpr uint8_t kBme680Addr = 0x77;
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

inline constexpr PortalField kPortalFields[] = {
    {"role",        "Radio role (0 node / 1 base; blank = strap)", FieldKind::Str,  "", "wildfire"},
    {"node_id",     "Node ID (1..65534; 0 = unprovisioned)",       FieldKind::Int,  "0", "wildfire"},
    {"prov_ids",    "Provisioned node IDs (csv, base allowlist)",   FieldKind::Str,  "", "wildfire"},
    {"wifi_ssid",   "WiFi SSID",                                   FieldKind::Str,  "", "wildfire"},
    {"wifi_pass",   "WiFi password",                               FieldKind::Str,  "", "wildfire"},
    {"mqtt_host",   "MQTT broker host",                            FieldKind::Str,  "api.nordtronics.io", "wildfire"},
    {"mqtt_port",   "MQTT broker port",                            FieldKind::Int,  "1883", "wildfire"},
    {"mqtt_user",   "MQTT username",                               FieldKind::Str,  "", "wildfire"},
    {"mqtt_pass",   "MQTT password",                               FieldKind::Str,  "", "wildfire"},
    {"mqtt_root",   "MQTT topic root",                             FieldKind::Str,  "nordtronics/wildfire", "wildfire"},
    {"offl_policy", "Offline policy (buffer|drop)",                FieldKind::Str,  "buffer", "wildfire"},
    {"offl_cap",    "Offline buffer cap (records)",                FieldKind::Int,  "180", "wildfire"},
    {"chk_s",       "Routine check-in period (s)",                 FieldKind::Int,  "720", "wildfire"},
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

}  // namespace wf
