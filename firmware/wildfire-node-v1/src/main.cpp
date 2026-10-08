// Wildfire Node v1 -- unified firmware, task 0094.
//
// ONE binary, TWO roles. The role is decided at boot (src/role_detect.h) by
// probing for the node sensors before any radio or network init, is logged to
// serial on every boot with its source, and every code path below branches on
// it (task 0104 item 3 replaced the 0094 GPIO7 jumper with the probe):
//
//   node  -- BME680 + PMS5003 sampling, routine CHECKIN every 12 min, ALARM on
//            its own hard threshold, deep sleep between check-ins (task 0112,
//            bench phase: the deep sleep and the check-in period are gated --
//            see kDeepSleepEnabled in src/firmware_config.h; the field values
//            are 720 s + sleep, restored at the final pre-deployment gate).
//   base  -- SX1262 receive loop, v0.2 consensus engine (src/consensus_v02.cpp),
//            MQTT uplink, OLED status, captive portal. NEVER deep-sleeps.
//
// The base is another Heltec LoRa 32 V4, USB-powered, no sensors: it bridges
// LoRa -> WiFi/MQTT -> VPS and Home Assistant.
//
// The consensus engine and the wire protocol are portable translation units
// (consensus_v02.cpp, radio_protocol.cpp, role_detect.cpp) so the host-side test
// in test/ exercises exactly the code compiled here.

#include <Arduino.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <RadioLib.h>
#include <U8g2lib.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <time.h>

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "consensus_v02.h"
#include "device_name.h"
#include "firmware_config.h"
#include "mqtt_ca.h"
#include "mqtt_payload.h"
#include "mqtt_topic.h"
#include "radio_protocol.h"
#include "role_detect.h"

#if __has_include(<Adafruit_BME680.h>)
#include <Adafruit_BME680.h>
#define WF_HAS_BME680 1
#else
#define WF_HAS_BME680 0
#endif

// ---------------------------------------------------------------------------
// hardware objects
// ---------------------------------------------------------------------------
static SX1262 g_radio = new Module(wf::kLoraNssPin, wf::kLoraDio1Pin,
                                   wf::kLoraRstPin, wf::kLoraBusyPin);
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C g_oled(U8G2_R0, U8X8_PIN_NONE);
static WebServer g_server(80);
// TLS transport (task 0099 item 7b): the deployed broker listens ONLY on
// 8883/TLS -- there is no 1883 listener -- so a plaintext WiFiClient can never
// reach it. The trust anchor is pinned (src/mqtt_ca.h), never setInsecure().
static WiFiClientSecure g_wifi_client;
static PubSubClient g_mqtt(g_wifi_client);
static Preferences g_prefs;
static HardwareSerial g_pms(1);
#if WF_HAS_BME680
static Adafruit_BME680 g_bme;
#endif

// The panel probe result. NOTHING may drive the SSD1306 until the probe has
// answered: a bulk 1024-byte sendBuffer() to an unproven panel hangs the I2C
// transaction and starves loop() -- which is exactly what stopped the captive
// portal from ever replying on the bench (task 0099 item 4).
static bool g_oled_ready = false;

// ---------------------------------------------------------------------------
// runtime configuration: firmware default (firmware_config.h) + NVS value
// ---------------------------------------------------------------------------
struct RuntimeConfig {
  int role_override = -1;  // -1 = unset (the sensor probe decides)
  uint16_t node_id = 0;
  String prov_ids;
  String wifi_ssid, wifi_pass;
  String mqtt_host, mqtt_user, mqtt_pass, mqtt_root;
  uint16_t mqtt_port = 8883;  // TLS only: the broker has no 1883 listener
  String offl_policy = "buffer";
  int offl_cap = 180;
  uint32_t checkin_s = (uint32_t)wf::kCheckinSecondsDefault;  // bench phase (field: 720)
  float lora_mhz = 915.0f;
  float lora_bw = 125.0f;
  uint8_t lora_sf = 7;
  uint8_t lora_cr = 5;
  uint8_t lora_sync = 0x12;
  int8_t lora_dbm = 20;
  uint16_t ack_to_s = 5;
  uint8_t ack_tries = 3;
  int corr_min = 20;
  float abs_floor = 25.0f;
  float rel_delta = 25.0f;
  int clear_min = 30;
  int rearm_min = 30;
  int baseline_n = 8;
  int live_min = 30;
  uint16_t batt_low_mv = 3400;
  uint32_t pms_baud = 9600;
};

static RuntimeConfig g_cfg;
static wf::Role g_role = wf::Role::Node;
static wf::RoleDecision g_role_decision;
static wf::ConsensusEngine g_engine;

// ---------------------------------------------------------------------------
// RTC-persistent node state (firmware release 0120, task 0125 item 4).
//
// RTC_DATA_ATTR keeps these through deep sleep, which is what makes the wire
// sequence counter monotonic across wakes and lets the node-fast-alarm latch
// survive a sleep. A plain static is re-initialised on every wake, so a node
// that alarmed and slept would come back on the next boot reporting "no alarm"
// -- exactly the state the RTC attribute exists to prevent.
// ---------------------------------------------------------------------------
RTC_DATA_ATTR uint16_t g_tx_seq = 0;        // monotonic across deep-sleep wakes
RTC_DATA_ATTR bool g_alarm_active = false;  // LAYER 1 latch, persists across sleep
RTC_DATA_ATTR uint16_t g_alarm_events = 0;  // LAYER 1 alarm episodes raised
RTC_DATA_ATTR uint32_t g_cycles = 0;        // node power-state cycles completed

// base-side node table, for the OLED and the MQTT "offline" state
struct NodeRecord {
  std::string id;
  float last_seen_min = -1.0f;
  float pm25 = 0.0f;
  uint16_t batt_mv = 0;
  bool online = false;
};
static std::vector<NodeRecord> g_nodes;
static std::string g_last_event = "none";
static float g_boot_min = 0.0f;

// ---------------------------------------------------------------------------
// small helpers
// ---------------------------------------------------------------------------
static void logf(const char* fmt, ...) {
  char buf[256];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  Serial.println(buf);
}

static float now_min() { return (float)(millis() / 60000UL); }

// ---------------------------------------------------------------------------
// Node power-state machine (firmware release 0120, task 0125 item 1)
// ---------------------------------------------------------------------------
// One cycle, in this order, every check-in period:
//
//   WAKE         a wake from deep sleep or a cold boot. The wake cause and the
//                RTC cycle counter are logged here.
//   SENSOR_POWER Vext asserted, PMS boost EN (GPIO16) HIGH, the PMS UART
//                attached, then the Plantower warm-up is honoured in full.
//   SAMPLE       one PMS5003 frame, one BME680 reading, one battery-divider
//                sample -- all taken with the sensors powered.
//   SENSOR_OFF   the PMS UART detached and its MCU TX pin floated BEFORE the
//                boost EN goes LOW, so the idle TX line cannot back-power the
//                unpowered sensor (item 2).
//   LORA_TX      the frame is encoded and transmitted; a failed transmit
//                re-initialises the radio once and retries.
//   ALARM_ACK    node-fast-alarm frames only: the ACK window and its retries.
//   DEEP_SLEEP   the field build sleeps here. The bench build (the
//                kDeepSleepEnabled gate) logs the gate and idles instead --
//                see AWAKE_IDLE.
//   AWAKE_IDLE   bench phase only: awake with USB-serial live, the 1 Hz live
//                PMS poll and the 1 Hz OLED render (tasks 0117/0118) running,
//                until the next cycle begins.
enum class NodeState : uint8_t {
  Wake,
  SensorPower,
  Sample,
  SensorOff,
  LoraTx,
  AlarmAck,
  DeepSleep,
  AwakeIdle,
};

static NodeState g_node_state = NodeState::Wake;

// Short names: they go on the 128 px OLED (21 columns at the 6x10 font) as well
// as in the serial log, so they stay compact.
static const char* node_state_name(NodeState s) {
  switch (s) {
    case NodeState::Wake:        return "WAKE";
    case NodeState::SensorPower: return "SENSOR_PWR";
    case NodeState::Sample:      return "SAMPLE";
    case NodeState::SensorOff:   return "SENSOR_OFF";
    case NodeState::LoraTx:      return "LORA_TX";
    case NodeState::AlarmAck:    return "ALARM_ACK";
    case NodeState::DeepSleep:   return "SLEEP";
    case NodeState::AwakeIdle:   return "IDLE";
  }
  return "?";
}

static void node_enter(NodeState s) {
  g_node_state = s;
  logf("power: %s", node_state_name(s));
}

// Defined with the OLED helpers further down. The sensor warm-up repaints the
// panel while it waits and the SAMPLE step paints the fresh reading, so the
// cycle needs the declaration here rather than below the OLED block.
static void oled_render_node();
static void pms_live_poll();

// True only while the PMS UART is attached, i.e. inside the sensor window. The
// 1 Hz live poll (task 0117) reads bytes only when this is set, so it can never
// drain a detached port.
static bool g_pms_uart_active = false;

// Set when the operator asked for provisioning mode (3 s BOOT hold at boot, item
// 6). A NODE in field mode never brings WiFi/AP/portal up; a BASE always does.
static bool g_provisioning = false;

// The portal renders stored values back into HTML input attributes. Without
// escaping, any value containing a quote, an angle bracket or an ampersand is
// truncated or mangled on the round trip -- the bench-observed corruption of a
// stored network name came from exactly this (task 0099 item 7a), and a password
// containing a `"` or `'` would be silently corrupted the same way. Escape
// `& < > " '` by walking the bytes: String::replace has no (char, const char*)
// overload, so the naive one-by-one replace() chain does not compile.
static String html_escape(const String& in) {
  String out;
  out.reserve(in.length() + 8);
  for (size_t i = 0; i < in.length(); ++i) {
    switch (in[i]) {
      case '&':  out += "&amp;";  break;
      case '<':  out += "&lt;";   break;
      case '>':  out += "&gt;";   break;
      case '"':  out += "&quot;"; break;
      case '\'': out += "&#39;";  break;
      default:   out += in[i];    break;
    }
  }
  return out;
}

// ---------------------------------------------------------------------------
// configuration load / save (NVS; one key per portal field)
// ---------------------------------------------------------------------------
static int portal_index(const char* key) {
  for (size_t i = 0; i < wf::kPortalFieldCount; ++i) {
    if (strcmp(wf::kPortalFields[i].key, key) == 0) return (int)i;
  }
  return -1;
}

static String cfg_get_str(const char* key) {
  const int i = portal_index(key);
  const char* def = (i >= 0) ? wf::kPortalFields[i].default_text : "";
  if (!g_prefs.isKey(key)) return String(def);
  return g_prefs.getString(key, def);
}

static int cfg_get_int(const char* key) {
  const int i = portal_index(key);
  const char* def = (i >= 0) ? wf::kPortalFields[i].default_text : "0";
  return (int)g_prefs.getInt(key, atoi(def));
}

static float cfg_get_float(const char* key) {
  const int i = portal_index(key);
  const char* def = (i >= 0) ? wf::kPortalFields[i].default_text : "0";
  return g_prefs.getFloat(key, (float)atof(def));
}

// The portal writes every field through here, so a value that arrives over HTTP
// is persisted to NVS before it is used -- there is no in-RAM-only field.
static void portal_store(const String& key, const String& value) {
  const int i = portal_index(key.c_str());
  if (i < 0) return;
  switch (wf::kPortalFields[i].kind) {
    case wf::FieldKind::Int:
      g_prefs.putInt(key.c_str(), value.toInt());
      break;
    case wf::FieldKind::Bool:
      g_prefs.putBool(key.c_str(), value == "1" || value == "true");
      break;
    case wf::FieldKind::Str:
    default:
      g_prefs.putString(key.c_str(), value);
      break;
  }
}

static void load_config() {
  const String role_s = cfg_get_str("role");
  if (role_s.length() == 1 && (role_s == "0" || role_s == "1")) {
    g_cfg.role_override = role_s.toInt();
  } else {
    g_cfg.role_override = -1;
  }
  g_cfg.node_id = (uint16_t)cfg_get_int("node_id");
  g_cfg.prov_ids = cfg_get_str("prov_ids");
  g_cfg.wifi_ssid = cfg_get_str("wifi_ssid");
  g_cfg.wifi_pass = cfg_get_str("wifi_pass");
  g_cfg.mqtt_host = cfg_get_str("mqtt_host");
  g_cfg.mqtt_port = (uint16_t)cfg_get_int("mqtt_port");
  g_cfg.mqtt_user = cfg_get_str("mqtt_user");
  g_cfg.mqtt_pass = cfg_get_str("mqtt_pass");
  g_cfg.mqtt_root = cfg_get_str("mqtt_root");
  g_cfg.offl_policy = cfg_get_str("offl_policy");
  g_cfg.offl_cap = cfg_get_int("offl_cap");
  g_cfg.checkin_s = (uint32_t)cfg_get_int("chk_s");
  g_cfg.lora_mhz = cfg_get_float("lora_mhz");
  g_cfg.lora_bw = cfg_get_float("lora_bw");
  g_cfg.lora_sf = (uint8_t)cfg_get_int("lora_sf");
  g_cfg.lora_cr = (uint8_t)cfg_get_int("lora_cr");
  g_cfg.lora_sync = (uint8_t)strtol(cfg_get_str("lora_sync").c_str(), nullptr, 0);
  g_cfg.lora_dbm = (int8_t)cfg_get_int("lora_dbm");
  g_cfg.ack_to_s = (uint16_t)cfg_get_int("ack_to_s");
  g_cfg.ack_tries = (uint8_t)cfg_get_int("ack_tries");
  g_cfg.corr_min = cfg_get_int("corr_min");
  if (g_cfg.corr_min < 10) g_cfg.corr_min = 10;   // portal range 10..60
  if (g_cfg.corr_min > 60) g_cfg.corr_min = 60;
  g_cfg.abs_floor = cfg_get_float("abs_floor");
  g_cfg.rel_delta = cfg_get_float("rel_delta");
  g_cfg.clear_min = cfg_get_int("clear_min");
  g_cfg.rearm_min = cfg_get_int("rearm_min");
  g_cfg.baseline_n = cfg_get_int("baseline_n");
  g_cfg.live_min = cfg_get_int("live_min");
  g_cfg.batt_low_mv = (uint16_t)cfg_get_int("batt_low_mv");
  g_cfg.pms_baud = (uint32_t)cfg_get_int("pms_baud");
}

static wf::ConsensusConfig consensus_config_from(const RuntimeConfig& c) {
  wf::ConsensusConfig cc;
  cc.abs_floor_ugm3 = c.abs_floor;
  cc.rel_delta_ugm3 = c.rel_delta;
  cc.correlation_window_min = (float)c.corr_min;
  cc.autoclear_below_min = (float)c.clear_min;
  cc.rearm_cooldown_min = (float)c.rearm_min;
  cc.baseline_window = (uint8_t)c.baseline_n;
  cc.live_age_min = (float)c.live_min;
  return cc;
}

// ---------------------------------------------------------------------------
// deep sleep -- the ONLY sleep path, and it refuses to run as base
// ---------------------------------------------------------------------------
static void enter_deep_sleep(uint32_t seconds) {
  if (g_role != wf::Role::Node) {
    // HARD RULE (task 0094 item 0): a base station that sleeps goes deaf and
    // takes the whole property's detection with it. Fail loudly and stay awake.
    logf("FATAL: deep sleep requested while role=%s -- refusing (base must never sleep)",
         wf::role_name(g_role));
    return;
  }
  logf("sleep: %u s (role=node)", (unsigned)seconds);
  // Park the sensor rail, then HOLD the boost enable LOW across the sleep: the
  // MiniBoost carries a 100 kOhm EN->VIN pull-up, so a pin merely released when
  // the RTC domain stops driving it would ENABLE the 5 V rail for the whole
  // sleep and drain the pack. rtc_gpio_hold_en latches the LOW level through
  // deep sleep; setup() releases the hold on the next wake.
  digitalWrite(wf::kPmsBoostEnablePin, !wf::kPmsBoostOnLevel);
  rtc_gpio_hold_en((gpio_num_t)wf::kPmsBoostEnablePin);
  logf("sleep: PMS boost EN gpio%d held LOW across deep sleep (RTC hold)",
       wf::kPmsBoostEnablePin);
  Serial.flush();
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
  esp_deep_sleep_start();
}

// ---------------------------------------------------------------------------
// node role
// ---------------------------------------------------------------------------
static float read_pms25(uint16_t* pm1, uint16_t* pm10, bool* ok) {
  *ok = false;
  *pm1 = 0;
  *pm10 = 0;
  uint8_t frame[32];
  const uint32_t deadline = millis() + 1500;
  int idx = 0;
  while (millis() < deadline) {
    if (!g_pms.available()) continue;
    const uint8_t b = (uint8_t)g_pms.read();
    if (idx == 0 && b != 0x42) continue;
    if (idx == 1 && b != 0x4D) {
      idx = 0;
      continue;
    }
    frame[idx++] = b;
    if (idx == 32) break;
  }
  if (idx != 32) return -1.0f;
  uint16_t sum = 0;
  for (int i = 0; i < 30; ++i) sum += frame[i];
  const uint16_t wire = ((uint16_t)frame[30] << 8) | frame[31];
  if (sum != wire) {
    logf("pms: checksum mismatch (calc=%u wire=%u) -- frame dropped", sum, wire);
    return -1.0f;
  }
  *pm1 = ((uint16_t)frame[10] << 8) | frame[11];
  const uint16_t pm25 = ((uint16_t)frame[12] << 8) | frame[13];
  *pm10 = ((uint16_t)frame[14] << 8) | frame[15];
  *ok = true;
  return (float)pm25;
}

// ---------------------------------------------------------------------------
// node role -- 0117 live poll (bench diagnostic)
// ---------------------------------------------------------------------------
// The 60 s check-in cadence cannot show the sensor reacting to a stimulus (a
// breath puff at the inlet), so this prints one line per successfully parsed
// PMS5003 frame on a ~1 Hz schedule, independently of node_checkin().
//
// NON-BLOCKING by construction: it only drains bytes that are ALREADY in the
// UART RX buffer when it runs, and exits immediately when the buffer is empty
// (no busy-wait on the 1 s period). The 5 ms deadline is a hard ceiling on the
// time it can steal from loop(); a frame is 32 bytes = ~33 ms on the wire at
// 9600 baud, so a straddling frame is carried in the static state below and
// completed by the next poll. That is also why it must not call read_pms25():
// that function's 1500 ms scan is for the check-in path only.
//
// It deliberately emits NO log on a checksum mismatch: a dropped frame is
// normal on this sensor and must not turn a 1 Hz diagnostic into log spam.
// The 0115 per-check-in `pms:` line is untouched and still fires.
//
// Task 0118: the same frame's three values are latched into file scope so the
// node OLED can show them. Latched only on a checksum-valid frame, so a poll
// with no complete frame leaves the OLED on the last good values instead of
// falling back to 0 -- 0 on this panel reads as clean air, which it is not.
static uint16_t g_pms_live_pm1 = 0;
static uint16_t g_pms_live_pm25 = 0;
static uint16_t g_pms_live_pm10 = 0;
static bool g_pms_live_valid = false;

static void pms_live_poll() {
  static uint8_t buf[32];
  static int idx = 0;
  static uint32_t last_ms = 0;
  // The sensor window owns the port (task 0125): outside it the UART is
  // detached and there is nothing to read. A half-received frame cannot survive
  // the window boundary, so the parser state is reset when the window closes.
  if (!g_pms_uart_active) {
    idx = 0;
    return;
  }
  if ((uint32_t)(millis() - last_ms) < 1000UL) return;
  last_ms = millis();

  const uint32_t deadline = millis() + 5;  // << the 1 s period
  while (millis() < deadline && g_pms.available()) {
    const uint8_t b = (uint8_t)g_pms.read();
    if (idx == 0 && b != 0x42) continue;
    if (idx == 1 && b != 0x4D) {
      idx = 0;
      continue;
    }
    buf[idx++] = b;
    if (idx < 32) continue;
    idx = 0;
    uint16_t sum = 0;
    for (int i = 0; i < 30; ++i) sum += buf[i];
    const uint16_t wire = ((uint16_t)buf[30] << 8) | buf[31];
    if (sum != wire) continue;
    const uint16_t pm1 = ((uint16_t)buf[10] << 8) | buf[11];
    const uint16_t pm25 = ((uint16_t)buf[12] << 8) | buf[13];
    const uint16_t pm10 = ((uint16_t)buf[14] << 8) | buf[15];
    // Task 0118: latch for the OLED. The values are the ones decoded right here,
    // so the panel and the `pms-live:` line can never disagree.
    g_pms_live_pm1 = pm1;
    g_pms_live_pm25 = pm25;
    g_pms_live_pm10 = pm10;
    g_pms_live_valid = true;
    logf("pms-live: pm1=%u pm25=%.1f pm10=%u", pm1, (float)pm25, pm10);
  }
}

// ---------------------------------------------------------------------------
// node role -- battery voltage (task 0119)
// ---------------------------------------------------------------------------
// The Heltec HTIT-WB32LAF V4.2 carries a SWITCHED divider on the battery input
// (no external hardware): ADC_Ctrl (GPIO37) HIGH connects VBAT through a
// 390k/100k string to ADC1_CH0, so the tap sees 100/(100+390) = 0.2041 of the
// pack -- 4.2 V reads ~0.86 V. GPIO37 LOW disconnects the divider, so it MUST
// be returned LOW on every exit path: a stuck-HIGH GPIO37 drains the pack
// continuously through the 490k string.
//
// read_battery_mv() has NO early return, and the single return is preceded by
// the GPIO37 LOW, so the guarantee is structural -- there is no branch today
// that can escape with the divider live.
static uint16_t g_batt_mv = 0;  // latched at check-in (the 0119 OLED/telemetry value)

static uint16_t read_battery_mv() {
  pinMode(wf::kBattAdcCtrlPin, OUTPUT);
  digitalWrite(wf::kBattAdcCtrlPin, HIGH);  // divider connected
  delay(10);                                // settle before the first sample

  // 0 dB attenuation: ~1.1 V full scale at the pin, which covers the 0.86 V a
  // 4.2 V pack presents through the divider. An 11 dB range would spend most of
  // the code space on voltages this tap can never produce.
  analogSetPinAttenuation(wf::kBattAdcPin, ADC_0db);

  const int kSamples = 16;
  uint32_t raw_sum = 0;
  uint32_t mv_sum = 0;
  for (int i = 0; i < kSamples; ++i) {
    raw_sum += (uint32_t)analogRead(wf::kBattAdcPin);
    mv_sum += (uint32_t)analogReadMilliVolts(wf::kBattAdcPin);
  }
  const uint32_t raw_avg = raw_sum / (uint32_t)kSamples;
  const uint32_t adc_mv = mv_sum / (uint32_t)kSamples;

  // analogReadMilliVolts() applies the eFuse calibration, so adc_mv is the
  // pin voltage and the pack is that divided by the divider ratio.
  const uint16_t vbatt_mv = (uint16_t)((float)adc_mv / wf::kBattDividerRatio + 0.5f);

  digitalWrite(wf::kBattAdcCtrlPin, LOW);  // divider disconnected -- EVERY exit path
  logf("batt: pin=%d raw=%u adc_mv=%u vbatt=%u mV", wf::kBattAdcPin,
       (unsigned)raw_avg, (unsigned)adc_mv, (unsigned)vbatt_mv);
  return vbatt_mv;
}

// Bench diagnostic (task 0119): the task text names GPIO2 as the tap while the
// V4.2 datasheet names ADC1_CH0 = GPIO1, so both candidates are sampled once at
// node boot with the divider live and the result is in the serial log -- the
// pin choice is evidence, not an assertion.
static void batt_adc_pin_probe() {
  pinMode(wf::kBattAdcCtrlPin, OUTPUT);
  digitalWrite(wf::kBattAdcCtrlPin, HIGH);
  delay(10);
  const int pins[] = {1, 2};  // ADC1_CH0 (datasheet) vs ADC1_CH1 (task text)
  for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i) {
    const int p = pins[i];
    analogSetPinAttenuation(p, ADC_0db);
    uint32_t raw = 0, mv = 0;
    for (int i2 = 0; i2 < 16; ++i2) {
      raw += (uint32_t)analogRead(p);
      mv += (uint32_t)analogReadMilliVolts(p);
    }
    logf("batt-probe: gpio%d raw=%u adc_mv=%u", p, (unsigned)(raw / 16),
         (unsigned)(mv / 16));
  }
  digitalWrite(wf::kBattAdcCtrlPin, LOW);
}

// ---------------------------------------------------------------------------
// node role -- sensor rail control (firmware release 0120, task 0125 item 2)
// ---------------------------------------------------------------------------
// The PMS5003 hangs off the MiniBoost's switched 5 V rail, gated by
// kPmsBoostEnablePin. It is the dominant load in the power budget (~97 % of the
// daily energy, docs/wildfire/node-power-budget-v1.md), so the state machine
// powers it for the sensor window only.
//
// Vext (the switched 3.3 V rail) stays asserted across SENSOR OFF on purpose:
// on this board family the SSD1306 panel shares that rail (task 0108) and the
// bench phase requires a live panel, while the load the acceptance gate
// measures -- and the back-power path -- is the 5 V boost and the UART. A
// Vext-per-cycle policy is a one-line change here if the panel is ever
// confirmed to sit on 3V3.
static void sensor_power_on() {
  digitalWrite(wf::kOledVextPin, LOW);  // Vext asserted (active LOW); idempotent
  digitalWrite(wf::kPmsBoostEnablePin, wf::kPmsBoostOnLevel);
  // The UART is attached only while the sensor is powered: with the port open
  // but the module unpowered, the MCU's idle-high TX line feeds the sensor's RX
  // protection diode and back-powers it (task 0125 item 2).
  g_pms.begin(g_cfg.pms_baud, SERIAL_8N1, wf::kPmsUartTxPin, wf::kPmsUartRxPin);
  g_pms_uart_active = true;
  logf("power: sensor rail up -- Vext on, PMS boost EN gpio%d %s, UART %lu baud attached",
       wf::kPmsBoostEnablePin, wf::kPmsBoostOnLevel == HIGH ? "HIGH" : "LOW",
       (unsigned long)g_cfg.pms_baud);
}

static void sensor_power_off() {
  // ORDER MATTERS (item 2): detach the UART and float the MCU's TX pin BEFORE
  // the boost goes down, so nothing on the UART lines can source current into
  // the unpowered sensor. kPmsUartRxPin is the MCU's TX (GPIO6, "PMS5003 RX <-
  // GPIO6" on the Rev C; HardwareSerial::begin takes rx first, tx second).
  if (g_pms_uart_active) {
    g_pms.end();
    g_pms_uart_active = false;
  }
  pinMode(wf::kPmsUartRxPin, INPUT);  // MCU TX -> high-Z, no back-power path
  digitalWrite(wf::kPmsBoostEnablePin, !wf::kPmsBoostOnLevel);
  logf("power: sensor rail down -- UART detached, MCU TX gpio%d high-Z, PMS boost EN gpio%d LOW",
       wf::kPmsUartRxPin, wf::kPmsBoostEnablePin);
  // The 0118 PM values are deliberately NOT cleared here: they are the last
  // checksum-valid frame and the panel keeps showing them between windows
  // rather than falling back to 0, which on this panel reads as clean air.
}

// ---------------------------------------------------------------------------
// role probe -- task 0104 item 3 (the tank-monitor pattern: probe, do not jump)
// ---------------------------------------------------------------------------
// Asked before anything else, because it decides which half of this file runs.
// Two independent answers and either one is enough to call the board a node:
//   * a BME680 (0x77) or BME688 (0x76) ACKs on the sensor I2C bus, or
//   * a valid 32-byte PMS5003 frame arrives on the sensor UART.
// The bus is brought up here on the sensor pins; oled_probe_and_begin() re-runs
// Wire.begin() with the same pins later. The SSD1306 at 0x3C is deliberately
// NOT part of the probe: both roles carry the panel.
static bool probe_node_sensors() {
  Wire.begin(wf::kI2cSdaPin, wf::kI2cSclPin);
  const uint8_t addrs[] = {wf::kBme680Addr, wf::kBme688Addr};
  for (const uint8_t a : addrs) {
    Wire.beginTransmission(a);
    const uint8_t ack = Wire.endTransmission();
    logf("probe: i2c 0x%02X -> %s", a, ack == 0 ? "ACK" : "no ACK");
    if (ack == 0) return true;
  }

  // PMS: the module sits behind the MiniBoost's switched 5 V rail, which the
  // state machine keeps OFF (0125 item 2), so the rail has to be brought up for
  // this test to mean anything -- an unpowered PMS can never emit a frame and
  // the board would fall through to the base default. Both the rail and the
  // UART are handed back down before returning.
  digitalWrite(wf::kPmsBoostEnablePin, wf::kPmsBoostOnLevel);
  delay(700);  // the module's own MCU needs to boot before it streams
  g_pms.begin(g_cfg.pms_baud, SERIAL_8N1, wf::kPmsUartTxPin, wf::kPmsUartRxPin);
  uint16_t pm1 = 0, pm10 = 0;
  bool pms_ok = false;
  (void)read_pms25(&pm1, &pm10, &pms_ok);
  logf("probe: pms5003 -> %s", pms_ok ? "frame valid" : "no frame");
  g_pms.end();
  pinMode(wf::kPmsUartRxPin, INPUT);  // no back-power path while the rail is down
  digitalWrite(wf::kPmsBoostEnablePin, !wf::kPmsBoostOnLevel);
  return pms_ok;
}

static void radio_reinit() {
  const int rs = g_radio.begin(g_cfg.lora_mhz, g_cfg.lora_bw, g_cfg.lora_sf,
                               g_cfg.lora_cr, g_cfg.lora_sync, g_cfg.lora_dbm, 8,
                               1.8f, false);
  logf("radio: re-init -> %s", rs == RADIOLIB_ERR_NONE ? "ok" : "FAILED");
}

static void node_send(wf::Frame& f) {
  uint8_t buf[64];
  f.version = wf::kProtoVersion;
  f.node_id = g_cfg.node_id;
  f.seq = g_tx_seq++;
  const size_t n = wf::encode_frame(f, buf, sizeof(buf));
  if (n == 0) {
    logf("tx: encode failed (type=%u)", f.type);
    return;
  }
  int st = g_radio.transmit(buf, n);
  if (st != RADIOLIB_ERR_NONE) {
    // Radio re-init robustness (0125 context: the LoRa TX path wedged ~21 min
    // after the USB unplug on the discharge run, and only a reboot restored
    // it). The state machine re-initialises the radio once and retransmits, so
    // a wedged transmit becomes a logged, recoverable event instead of a
    // silent loss until the next wake.
    logf("tx: transmit FAILED (err=%d) -- re-initialising the radio and retrying", st);
    radio_reinit();
    st = g_radio.transmit(buf, n);
  }
  logf("tx: type=%u node=%u seq=%u len=%u -> %s", f.type, f.node_id, f.seq,
       (unsigned)n, st == RADIOLIB_ERR_NONE ? "sent" : "FAILED");
}

static bool wait_for_ack(uint16_t want_seq, wf::Frame* ack_out) {
  uint8_t buf[64];
  const uint32_t deadline = millis() + (uint32_t)g_cfg.ack_to_s * 1000UL;
  while (millis() < deadline) {
    const int st = g_radio.receive(buf, sizeof(buf));
    if (st != RADIOLIB_ERR_NONE) continue;
    wf::Frame f;
    if (wf::decode_frame(buf, g_radio.getPacketLength(), &f) != wf::DecodeResult::Ok) {
      continue;  // never act on a bad frame
    }
    if (f.type == wf::kMsgAck && f.acked_seq == want_seq) {
      *ack_out = f;
      return true;
    }
  }
  return false;
}

// ---------------------------------------------------------------------------
// node role -- SAMPLE / TX / ACK steps (firmware release 0120, task 0125)
// ---------------------------------------------------------------------------
struct NodeSample {
  bool pms_ok = false;
  uint16_t pm1 = 0;
  uint16_t pm10 = 0;
  float pm25 = 0.0f;
  bool bme_ok = false;
  float temp_c = 0.0f;
  float rh = 0.0f;
  uint32_t press_pa = 0;
  uint16_t batt_mv = 0;
  bool alarm = false;
};

static NodeSample node_sample() {
  NodeSample s;
  const int pms_avail_before = g_pms.available();
  s.pm25 = read_pms25(&s.pm1, &s.pm10, &s.pms_ok);
  logf("pms: rx_avail_before=%d ok=%d pm1=%u pm25=%.1f pm10=%u",
       pms_avail_before, s.pms_ok ? 1 : 0, s.pm1, s.pm25, s.pm10);
#if WF_HAS_BME680
  if (g_bme.performReading()) {
    s.bme_ok = true;
    s.temp_c = g_bme.temperature;
    s.rh = g_bme.humidity;
    s.press_pa = (uint32_t)g_bme.pressure;
  } else {
    logf("bme680: performReading failed at 0x%02X", wf::kBme680Addr);
  }
#endif
  // Task 0119: the measured pack voltage from the module's switched divider,
  // latched into g_batt_mv so the OLED row and this payload cannot disagree.
  g_batt_mv = read_battery_mv();
  s.batt_mv = g_batt_mv;

  // LAYER 1 -- node-fast-alarm (task 0125 item 5). NODE firmware only; the
  // base's LAYER 2 consensus is never evaluated here. The latch is RTC-backed
  // (item 4), so an alarm still standing when the node sleeps is still standing
  // when it wakes: only a valid frame BELOW the threshold clears it, and a
  // failed frame leaves the last known state untouched rather than inventing a
  // clear.
  if (s.pms_ok) {
    if (s.pm25 >= wf::kNodeFastAlarmPm25) {
      if (!g_alarm_active) {
        g_alarm_active = true;
        ++g_alarm_events;
        logf("alarm: LAYER 1 node-fast-alarm raised (pm25=%.1f >= %.1f)",
             s.pm25, wf::kNodeFastAlarmPm25);
      }
    } else if (g_alarm_active) {
      g_alarm_active = false;
      logf("alarm: LAYER 1 node-fast-alarm cleared (pm25=%.1f < %.1f)",
           s.pm25, wf::kNodeFastAlarmPm25);
    }
  }
  s.alarm = g_alarm_active;
  return s;
}

static wf::Frame node_frame_from(const NodeSample& s) {
  wf::Frame f;
  f.pm1_x10 = s.pm1 * 10;
  f.pm25_x10 = s.pms_ok ? (uint16_t)(s.pm25 * 10.0f) : 0;
  f.pm10_x10 = s.pm10 * 10;
#if WF_HAS_BME680
  if (s.bme_ok) {
    f.temp_c_x100 = (int16_t)(s.temp_c * 100.0f);
    f.rh_x100 = (uint16_t)(s.rh * 100.0f);
    f.press_pa = s.press_pa;
    f.status |= wf::kStatusBmePresent;
  } else {
    f.status |= wf::kStatusSensorFault;
  }
#endif
  f.status |= s.pms_ok ? (wf::kStatusPmsPresent | wf::kStatusPmsOk) : wf::kStatusPmsPresent;
  f.batt_mv = s.batt_mv;
  f.flags = 0;
  // The LAYER 1 latch turns a routine check-in into an ALARM, which is the only
  // packet type that requires an ACK and is therefore retried.
  f.type = s.alarm ? wf::kMsgAlarm : wf::kMsgCheckin;
  if (s.alarm) f.flags |= wf::kFlagAlarm | wf::kFlagAckRequired;
  return f;
}

static void node_alarm_ack_window(wf::Frame& f) {
  wf::Frame ack;
  bool got = false;
  for (uint8_t attempt = 0; attempt < g_cfg.ack_tries && !got; ++attempt) {
    got = wait_for_ack(f.seq, &ack);
    if (!got) {
      logf("alarm: no ACK within %u s (attempt %u/%u) -- retransmitting",
           g_cfg.ack_to_s, attempt + 1, g_cfg.ack_tries);
      node_send(f);
    }
  }
  logf("alarm seq=%u %s", f.seq, got ? "ACKed" : "UNACKED (retries exhausted)");
}

// The Plantower warm-up is served here rather than as one long blocking delay:
// each second the 0117 live poll runs (the module streams frames within ~1 s of
// power-on; only the VALUES need the 30 s) and the 0118 OLED render repaints, so
// a bench operator watches the window open instead of staring at a frozen
// panel. delay() yields to the idle task, so the task WDT stays fed.
static void sensor_warmup() {
  const uint32_t t0 = millis();
  while ((uint32_t)(millis() - t0) < wf::kPmsWarmupMs) {
    delay(1000);
    pms_live_poll();
    oled_render_node();
  }
  logf("power: PMS warm-up complete (%u ms) -- readings are now trustworthy",
       (unsigned)(millis() - t0));
}

// The whole node cycle, one check-in period apart. See the NodeState comment
// block at the top of this file for the sequence and which task each step
// serves.
static void node_run_cycle() {
  ++g_cycles;
  node_enter(NodeState::Wake);
  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  logf("power: wake cause=%d (%s), RTC cycle=%lu seq=%u alarm=%d events=%u",
       (int)cause, cause == ESP_SLEEP_WAKEUP_TIMER ? "timer" : "power-on/other",
       (unsigned long)g_cycles, g_tx_seq, g_alarm_active ? 1 : 0,
       (unsigned)g_alarm_events);

  node_enter(NodeState::SensorPower);
  sensor_power_on();
  sensor_warmup();

  node_enter(NodeState::Sample);
  const NodeSample s = node_sample();
  oled_render_node();  // the fresh sample is on the panel before radio/sleep

  node_enter(NodeState::SensorOff);
  sensor_power_off();

  node_enter(NodeState::LoraTx);
  wf::Frame f = node_frame_from(s);
  node_send(f);

  if (s.alarm) {
    node_enter(NodeState::AlarmAck);
    node_alarm_ack_window(f);
  }

  node_enter(NodeState::DeepSleep);
  if (wf::kDeepSleepEnabled) {
    enter_deep_sleep(g_cfg.checkin_s);
    // Reached only if the sleep was refused (i.e. we are not a node after all).
  } else {
    node_enter(NodeState::AwakeIdle);
    logf("bench gate: deep sleep DISABLED -- staying awake, next cycle in %u s",
         (unsigned)g_cfg.checkin_s);
  }
}

// ---------------------------------------------------------------------------
// base role
// ---------------------------------------------------------------------------
static NodeRecord* node_record(const std::string& id) {
  for (auto& r : g_nodes) {
    if (r.id == id) return &r;
  }
  NodeRecord r;
  r.id = id;
  g_nodes.push_back(r);
  return &g_nodes.back();
}

static bool is_provisioned(uint16_t id) {
  if (g_cfg.prov_ids.length() == 0) return true;  // empty allowlist = bench open
  int start = 0;
  while (start < (int)g_cfg.prov_ids.length()) {
    int comma = g_cfg.prov_ids.indexOf(',', start);
    if (comma < 0) comma = g_cfg.prov_ids.length();
    const String tok = g_cfg.prov_ids.substring(start, comma);
    if (tok.length() && (uint16_t)tok.toInt() == id) return true;
    start = comma + 1;
  }
  return false;
}

// ---------------------------------------------------------------------------
// MQTT publish, with the offline policy honoured (task 0104 item 4)
// ---------------------------------------------------------------------------
// 0099's uplink early-returned while the session was down, so a dropped TLS
// session silently discarded telemetry even though `offl_policy` is `buffer`,
// and nothing in the log said so. Records now go into a bounded buffer and are
// replayed in order on reconnect, drop-oldest per wf::kOfflineBufferDropOldest
// -- the rule docs/wildfire/radio-protocol-v1.md sec. 8 states for the buffer.
struct OfflineRecord {
  std::string topic;
  std::string payload;
};
static std::vector<OfflineRecord> g_offline;

static void offline_push(const std::string& topic, const std::string& payload) {
  const size_t cap = (g_cfg.offl_cap > 0) ? (size_t)g_cfg.offl_cap : 0;
  if (cap == 0) return;
  if (g_offline.size() >= cap) {
    if (!wf::kOfflineBufferDropOldest) return;
    g_offline.erase(g_offline.begin());  // evict the OLDEST record
  }
  g_offline.push_back(OfflineRecord{topic, payload});
}

// The sample instant for both payload shapes. The base's clock is set from NTP
// before the uplink (task 0099), so this is a real UTC instant; the backend
// requires it on telemetry and on every event.
static std::string now_iso_z() { return wf::iso8601_z(std::time(nullptr)); }

// Publish one record. `topic` is the FULL topic, root included.
static void mqtt_publish(const std::string& topic, const String& payload) {
  if (!g_mqtt.connected()) {
    if (g_cfg.offl_policy == "buffer") {
      offline_push(topic, payload.c_str());
      logf("mqtt: session down -- buffered %u/%d [%s]", (unsigned)g_offline.size(),
           g_cfg.offl_cap, topic.c_str());
    } else {
      logf("mqtt: session down -- dropped (offl_policy=%s) [%s]",
           g_cfg.offl_policy.c_str(), topic.c_str());
    }
    return;
  }
  g_mqtt.publish(topic.c_str(), payload.c_str());
}

// Replay the buffer in order once the session is up. Stops at the first record
// the client refuses, so a still-buffered record is never silently lost.
static void offline_flush() {
  if (!g_mqtt.connected()) return;
  while (!g_offline.empty()) {
    const OfflineRecord rec = g_offline.front();
    if (!g_mqtt.publish(rec.topic.c_str(), rec.payload.c_str())) break;
    g_offline.erase(g_offline.begin());
  }
  static uint32_t last_log = 0;
  if (!g_offline.empty() && millis() - last_log > 30000UL) {
    last_log = millis();
    logf("mqtt: replay pending -- %u records still buffered", (unsigned)g_offline.size());
  }
}

static void base_handle_frame(const wf::Frame& f) {
  NodeRecord* rec = node_record(std::to_string(f.node_id));
  rec->last_seen_min = now_min();
  rec->online = true;
  rec->batt_mv = f.batt_mv;
  rec->pm25 = (float)f.pm25_x10 / 10.0f;

  const bool valid = wf::frame_plausible(f);
  // Captured BEFORE feed(): a Cleared event carries no level of its own, and
  // feed() has already reset the engine's level by the time the event is
  // returned, so the pre-feed level is the only record of which kind of raise
  // (watch or alert) is being cleared.
  const wf::Level level_before = g_engine.level();
  const std::vector<wf::Event> evs =
      g_engine.feed(now_min(), std::to_string(f.node_id), valid, rec->pm25);

  // Telemetry payload + topic: exactly what the deployed ingest accepts
  // (src/mqtt_payload.h states the contract). 0104 fixed the topic; the payload
  // still carried `node`/`temp_c`/`rh`/`batt_mv` and no `observed_utc`, any one
  // of which makes validation.py reject the reading outright, so nothing was
  // ever stored (task 0105 item 1).
  const std::string node_id = std::to_string((unsigned)f.node_id);
  mqtt_publish(wf::telemetry_topic(g_cfg.mqtt_root.c_str(), node_id),
               wf::telemetry_json(node_id, f.pm25_x10 / 10.0,
                                  f.temp_c_x100 / 100.0, f.rh_x100 / 100.0,
                                  f.batt_mv / 1000.0, now_iso_z()).c_str());

  for (const auto& e : evs) {
    g_last_event = std::string(wf::event_kind_name(e.kind)) + " @" +
                   std::to_string((int)e.t) + "min";
    logf("EVENT t=%.1f %s %s %s", e.t, wf::event_kind_name(e.kind),
         e.nodes.empty() ? e.node.c_str() : "", e.detail.c_str());
    const char* wire = wf::wire_event_kind(e.kind, level_before == wf::Level::Alert);
    if (wire == nullptr) continue;  // NodeRiseConfirmed: internal, no backend kind
    // Events go to `<root>/<base-id>/events` -- the shape events.py subscribes.
    // The old `<root>/event/alert` form put `event` where the base id belongs
    // and was dropped by the broker before ingest saw it (task 0105 item 2).
    mqtt_publish(wf::events_topic(g_cfg.mqtt_root.c_str(),
                                  std::to_string((unsigned)g_cfg.node_id)),
                 wf::event_json(wire, now_iso_z(), wf::event_subject(e.nodes),
                                e.nodes).c_str());
  }
}

// The receive window, in milliseconds. 0094 called receive() with the library
// default, and RadioLib's default is 5 x the time-on-air of the WHOLE 128-byte
// buffer (SX126x.cpp: `timeoutInternal = (getTimeOnAir(maxLen) * 5) / 1000;`).
// At the configured SF7/125 kHz that is 198 payload symbols = 215 ms of air, so
// the default window is ~1.1 s; at SF12 it is ~25 s -- longer than the 22 s
// mosquitto allows a client to miss its keepalive. Every one of those seconds is
// a second PubSubClient is not serviced. A short bounded window keeps the loop
// responsive: a node frame lasts ~215 ms here and check-ins repeat, so polling
// in slices loses nothing (item 4).
static constexpr uint32_t kRadioPollMs = 500;

static void base_radio_poll() {
  uint8_t buf[128];
  const int st = g_radio.receive(buf, sizeof(buf), kRadioPollMs);
  if (st != RADIOLIB_ERR_NONE) return;
  wf::Frame f;
  const wf::DecodeResult r = wf::decode_frame(buf, g_radio.getPacketLength(), &f);
  if (r != wf::DecodeResult::Ok) {
    logf("rx: dropped frame (%s)", wf::decode_result_name(r));
    return;
  }
  if (!is_provisioned(f.node_id)) {
    logf("rx: node %u is not provisioned -- ignored", f.node_id);
    return;
  }
  base_handle_frame(f);
  if (f.type == wf::kMsgAlarm || (f.flags & wf::kFlagAckRequired)) {
    wf::Frame ack;
    ack.type = wf::kMsgAck;
    ack.node_id = f.node_id;
    ack.seq = g_tx_seq++;
    ack.acked_seq = f.seq;
    ack.ack_code = wf::kAckOk;
    uint8_t out[32];
    const size_t n = wf::encode_frame(ack, out, sizeof(out));
    if (n) {
      g_radio.transmit(out, n);
      logf("tx: ACK seq=%u for node %u", f.seq, f.node_id);
    }
  }
}

// ---------------------------------------------------------------------------
// OLED (SSD1306 128x64, I2C 0x3C) -- task 0099 items 3 and 4
// ---------------------------------------------------------------------------
// The panel's reset line is GPIO21 and it is NOT tied to the ESP32's own reset,
// so without a real pulse the SSD1306 never answers, U8g2's begin() spins on an
// absent device, and every later bulk write wedges the I2C bus. Bring the panel
// up in this order: drive RST, then Wire.begin(), then PROVE it answers on 0x3C.
static bool oled_probe_and_begin() {
  pinMode(wf::kOledRstPin, OUTPUT);
  digitalWrite(wf::kOledRstPin, LOW);
  delay(20);
  digitalWrite(wf::kOledRstPin, HIGH);
  delay(20);

  Wire.begin(wf::kI2cSdaPin, wf::kI2cSclPin);
  Wire.beginTransmission(wf::kSsd1306Addr);
  const uint8_t ack = Wire.endTransmission();
  logf("oled: probe 0x%02X -> %s", wf::kSsd1306Addr, ack == 0 ? "ACK" : "no ACK");
  if (ack != 0) {
    logf("oled: panel did not answer -- rendering disabled, bus left idle");
    return false;
  }
  g_oled.setI2CAddress(wf::kSsd1306Addr << 1);
  g_oled.begin();
  return true;
}

// The ONLY path to sendBuffer(): gated on the probe. A 1024-byte page write to
// an unproven panel hangs loop() and starves g_server.handleClient(), which is
// what stopped the captive portal from replying on the bench.
static void oled_flush() {
  if (!g_oled_ready) return;
  g_oled.sendBuffer();
}

static void oled_render_base() {
  if (!g_oled_ready) return;
  g_oled.clearBuffer();
  g_oled.setFont(u8g2_font_6x10_tf);
  g_oled.drawStr(0, 10, "WILDFIRE BASE");
  char line[40];
  snprintf(line, sizeof(line), "level:%s  nodes:%d", wf::level_name(g_engine.level()),
           (int)g_nodes.size());
  g_oled.drawStr(0, 20, line);
  snprintf(line, sizeof(line), "mqtt:%s", g_mqtt.connected() ? "up" : "down");
  g_oled.drawStr(0, 30, line);
  // last-seen for the first two nodes in the table
  int y = 42;
  int shown = 0;
  for (const auto& n : g_nodes) {
    if (shown >= 2) break;
    snprintf(line, sizeof(line), "%s last-seen %.0f min", n.id.c_str(), n.last_seen_min);
    g_oled.drawStr(0, y, line);
    y += 10;
    shown++;
  }
  // last event, truncated to the 128 px panel
  std::string ev = g_last_event;
  if (ev.size() > 20) ev = ev.substr(0, 20);
  snprintf(line, sizeof(line), "last: %s", ev.c_str());
  g_oled.drawStr(0, 62, line);
  oled_flush();
}

static void oled_render_node() {
  if (!g_oled_ready) return;
  g_oled.clearBuffer();
  g_oled.setFont(u8g2_font_6x10_tf);
  g_oled.drawStr(0, 10, "WILDFIRE NODE");
  // Task 0118: six rows no longer fit on the 12 px cadence, so this block uses
  // the 10 px cadence oled_render_base() already proves on this panel (its
  // last-seen rows and its `last:` row sit 10-12 px apart on the same 64 px
  // height). Every pre-existing line is kept; the PM row is added last.
  char line[32];
  snprintf(line, sizeof(line), "id:%u", g_cfg.node_id);
  g_oled.drawStr(0, 20, line);
  snprintf(line, sizeof(line), "chk:%lus", (unsigned long)g_cfg.checkin_s);
  g_oled.drawStr(0, 30, line);
  // Task 0125: the power-state row. It carries the RTC-persistent sequence
  // counter (item 4) and the state the machine is in, so a bench operator can
  // read the cycle off the panel without a serial console.
  snprintf(line, sizeof(line), "seq:%u %s", g_tx_seq, node_state_name(g_node_state));
  g_oled.drawStr(0, 40, line);
  // Task 0119: the value latched at the last check-in (see g_batt_mv), never a
  // hardcoded 0 -- the row and the LoRa payload carry the same number.
  snprintf(line, sizeof(line), "vbat:%umV", g_batt_mv);
  g_oled.drawStr(0, 50, line);
  // Live PM from the 1 Hz poll (task 0118) -- the same frame the `pms-live:`
  // serial line prints, refreshing at the same ~1 Hz. `--` until the first
  // checksum-valid frame arrives, never a fake 0.
  if (g_pms_live_valid) {
    snprintf(line, sizeof(line), "PM %u/%u/%u", g_pms_live_pm1, g_pms_live_pm25, g_pms_live_pm10);
  } else {
    snprintf(line, sizeof(line), "PM --/--/--");
  }
  g_oled.drawStr(0, 60, line);
  oled_flush();
}

// ---------------------------------------------------------------------------
// network -- task 0099 items 5, 6 and the NTP precondition for TLS
// ---------------------------------------------------------------------------
// The portal runs on the setup AP, so the AP must be up BEFORE portal_setup()
// and must never depend on the STA join: with a stored SSID that fails to join
// the board used to end up on neither network, and 192.168.4.1 -- the only way
// back in -- was gone.
// The DNS name the board asks DHCP for (task 0104 item 2, Stephen's ask):
// `wildfire-<role>-<nn>`, so a board is identifiable in any router's client
// list with no OLED and no serial console. Derived from the role decision plus
// the node id, so it adds no portal field and no NVS key, and it must be set
// BEFORE the STA starts or the whole session keeps the default `espressif`
// name. The AP keeps the fixed kPortalApName: that one is what the operator
// joins to reach the portal, not a site value.
static void wifi_set_device_name() {
  const std::string hostname = wf::device_hostname(wf::role_name(g_role), g_cfg.node_id);
  WiFi.setHostname(hostname.c_str());
  logf("wifi: dhcp hostname [%s]", hostname.c_str());
}

// Connect / drop logging with the pair 0098 asked for: the STA IP on connect and
// the disconnect REASON CODE on drop. The current build printed neither, which
// is why 0103's bench session loop could not be diagnosed from the log.
static void wifi_event_log(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      logf("wifi: connected -- ip=%s gw=%s rssi=%d dBm",
           WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(),
           WiFi.RSSI());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      logf("wifi: disconnected -- reason=%d (%s)", (int)info.wifi_sta_disconnected.reason,
           WiFi.status() == WL_CONNECTED ? "still up" : "sta down");
      break;
    default:
      break;
  }
}

static void wifi_bring_up_ap(const char* why) {
  WiFi.mode(g_cfg.wifi_ssid.length() ? WIFI_AP_STA : WIFI_AP);
  const bool ok = WiFi.softAP(wf::kPortalApName);
  logf("portal AP fallback: %s  http://%s (%s)", wf::kPortalApName,
       WiFi.softAPIP().toString().c_str(), ok ? why : "softAP FAILED");
}

// Merge the AP and the STA bring-up into one step, since the AP has to come up
// first regardless of whether a join is attempted. `waitMs` only bounds the
// join; the AP is available immediately.
static void wifi_begin(uint32_t waitMs) {
  wifi_set_device_name();
  wifi_bring_up_ap("boot");
  if (!g_cfg.wifi_ssid.length()) return;
  WiFi.begin(g_cfg.wifi_ssid.c_str(), g_cfg.wifi_pass.c_str());
  const uint32_t deadline = millis() + waitMs;
  while (WiFi.status() != WL_CONNECTED && millis() < deadline) delay(200);
  logf("wifi: status=%d (%s) ip=%s", (int)WiFi.status(),
       WiFi.status() == WL_CONNECTED ? "connected" : "not connected",
       WiFi.localIP().toString().c_str());
}

// The clock precondition for TLS: mbedTLS validates the pinned ISRG Root X1
// against the system clock (CONFIG_MBEDTLS_HAVE_TIME_DATE), and the ESP32 boots
// at the 1970 epoch, so the handshake cannot succeed before NTP has set it.
// Declared as a scope extension in the 0099 reply -- without it the pinned root
// can never validate and the uplink is dead on arrival.
static void net_time_sync(uint32_t waitMs) {
  if (WiFi.status() != WL_CONNECTED) return;
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  const uint32_t deadline = millis() + waitMs;
  while (time(nullptr) < 1700000000L && millis() < deadline) delay(200);
  const time_t t = time(nullptr);
  if (t < 1700000000L) {
    logf("ntp: no time yet (t=%ld) -- TLS verification will fail until it syncs", (long)t);
  } else {
    logf("ntp: clock set (t=%ld)", (long)t);
  }
}

// A failed join must not lock the operator out. Re-arm the setup AP before each
// rejoin attempt so the portal stays reachable while the STA is down.
static void base_network_tick() {
  if (!g_cfg.wifi_ssid.length()) return;  // no stored SSID: AP-only by design
  if (WiFi.status() == WL_CONNECTED) return;
  static uint32_t last_try = 0;
  if (last_try != 0 && millis() - last_try < 30000UL) return;
  last_try = millis();
  wifi_bring_up_ap("join failed");
  logf("wifi: retrying join to [%s] (status=%d)", g_cfg.wifi_ssid.c_str(),
       (int)WiFi.status());
  WiFi.begin(g_cfg.wifi_ssid.c_str(), g_cfg.wifi_pass.c_str());
}

// ---------------------------------------------------------------------------
// provisioning gesture -- firmware release 0120, task 0125 item 6
// ---------------------------------------------------------------------------
// Field mode sheds WiFi/AP/portal on a NODE: the node talks LoRa only, so
// carrying an AP and a portal is pure idle load and an attack surface. Joining
// the setup AP is therefore a deliberate maintenance act: hold BOOT (GPIO0,
// active low) through the first kProvisionHoldMs of the boot, with the panel
// showing the hold, and provisioning mode comes up. Release early and the board
// boots straight into field mode.
//
// Runs after the panel probe (the countdown is drawn on the panel) and before
// the network bring-up, so the decision is made once, at boot, for both roles.
// A BASE ignores the gesture -- it always needs the network, it is the uplink.
static bool provisioning_gesture() {
  pinMode(wf::kBootButtonPin, INPUT_PULLUP);
  if (digitalRead(wf::kBootButtonPin) != LOW) return false;

  logf("provision: BOOT held at boot -- hold %u ms to enter provisioning mode",
       (unsigned)wf::kProvisionHoldMs);
  const uint32_t t0 = millis();
  while ((uint32_t)(millis() - t0) < wf::kProvisionHoldMs) {
    if (g_oled_ready) {
      const uint32_t left_ms =
          wf::kProvisionHoldMs - (uint32_t)(millis() - t0);
      char line[32];
      g_oled.clearBuffer();
      g_oled.setFont(u8g2_font_6x10_tf);
      g_oled.drawStr(0, 10, "PROVISION?");
      snprintf(line, sizeof(line), "hold %us", (unsigned)((left_ms + 999) / 1000));
      g_oled.drawStr(0, 24, line);
      g_oled.drawStr(0, 38, "release = field");
      g_oled.drawStr(0, 52, "keep held = setup AP");
      oled_flush();
    }
    if (digitalRead(wf::kBootButtonPin) != LOW) {
      logf("provision: BOOT released after %u ms -- field mode (no WiFi)",
           (unsigned)(millis() - t0));
      return false;
    }
    delay(100);
  }
  logf("provision: BOOT held %u ms -- PROVISIONING MODE (WiFi + captive portal)",
       (unsigned)(millis() - t0));
  return true;
}

static void portal_setup() {
  g_server.on("/", HTTP_GET, []() {
    String html = "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>";
    html += "<h2>Wildfire " + html_escape(String(wf::role_name(g_role))) + "</h2>";
    html += "<form method=POST action=/save>";
    for (size_t i = 0; i < wf::kPortalFieldCount; ++i) {
      const wf::PortalField& f = wf::kPortalFields[i];
      // Every value goes through html_escape: the attribute is single-quoted,
      // so an unescaped value containing ', " < or & truncates or corrupts the
      // round trip (task 0099 item 7a).
      html += "<label>" + html_escape(String(f.label)) + "<br><input name='" +
              html_escape(String(f.key)) + "' value='" + html_escape(cfg_get_str(f.key)) +
              "'></label><br>";
    }
    html += "<button>Save</button></form>";
    g_server.send(200, "text/html", html);
  });
  g_server.on("/save", HTTP_POST, []() {
    for (size_t i = 0; i < wf::kPortalFieldCount; ++i) {
      const wf::PortalField& f = wf::kPortalFields[i];
      if (g_server.hasArg(f.key)) portal_store(f.key, g_server.arg(f.key));
    }
    g_server.send(200, "text/plain", "saved; reboot to apply\n");
  });
  g_server.begin();
}

// ---------------------------------------------------------------------------
// setup / loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  for (uint32_t t0 = millis(); !Serial && millis() - t0 < 2000;) delay(10);

  // ---- STEP -2: the 5 V sensor rail OFF, before anything else (task 0125) ---
  // The MiniBoost carries a 100 kOhm EN->VIN pull-up, so a floating enable line
  // turns the 5 V rail ON by itself: on the previous build the pin was never
  // driven and the PMS5003 was powered for the whole time the node was awake.
  // Release any RTC hold left by the last deep sleep, then drive the enable to
  // its off level immediately -- the state machine is the only thing that may
  // raise it again.
  rtc_gpio_hold_dis((gpio_num_t)wf::kPmsBoostEnablePin);
  pinMode(wf::kPmsBoostEnablePin, OUTPUT);
  digitalWrite(wf::kPmsBoostEnablePin, !wf::kPmsBoostOnLevel);

  // ---- STEP -1: sensor rail ON, before ANY I2C init (task 0108) ------------
  // GPIO36 gates Vext, the switched 3.3 V rail the BME680/OLED sit on, active
  // LOW. With the rail off the sensor sits at 0 V and clamps SDA, so the role
  // probe below sees nothing on the bus and the board falls through to the
  // base default. Both roles carry the gate (the panel lives on Vext either
  // way) and it is harmless on a sensorless board, so it is driven
  // unconditionally and first -- before probe_node_sensors() and before
  // oled_probe_and_begin(), i.e. before the first Wire.begin() in the sketch.
  pinMode(wf::kOledVextPin, OUTPUT);
  digitalWrite(wf::kOledVextPin, LOW);
  delay(10);  // let the rail settle before the probe's first I2C transaction

  // ---- STEP 0: role detection (task 0104 item 3 -- probe, not jumper) ------
  // The NVS/portal value is read first and the sensor probe second, so the
  // probe runs with the configured PMS baud while the persisted role still
  // outranks it inside resolve_role(). Nothing radio-, network- or power-
  // related is initialised before the role is known, with the single deliberate
  // exception of the Vext gate above: the probe cannot read the sensor bus it
  // powers (task 0108).
  g_prefs.begin(wf::kPrefsNamespace, false);
  load_config();
  const String role_s = g_prefs.getString("role", "");
  const bool nvs_present = (role_s.length() == 1 && (role_s == "0" || role_s == "1"));
  const int nvs_role = nvs_present ? role_s.toInt() : -1;
  const bool sensors_present = probe_node_sensors();

  g_role_decision = wf::resolve_role(sensors_present, nvs_present, nvs_role);
  g_role = g_role_decision.role;
  logf("%s", wf::role_log_line(g_role_decision).c_str());
  logf("firmware: wildfire-unified-v1 proto=%u build=%s %s", wf::kProtoVersion,
       __DATE__, __TIME__);
  if (g_role == wf::Role::Base) {
    logf("base: deep sleep is disabled for this role (asserted on every sleep path)");
  } else {
    // The bench-phase gate is logged on every node boot so a serial capture
    // shows which side of the FINAL PRE-DEPLOYMENT GATE this build is on.
    logf("node: deep sleep gate=%s (bench phase) -- check-in period %u s "
         "(field value %d s + sleep restores at the final gate)",
         wf::kDeepSleepEnabled ? "ENABLED" : "DISABLED",
         (unsigned)g_cfg.checkin_s, wf::kCheckinSecondsField);
  }

  g_boot_min = now_min();
  if (g_role == wf::Role::Base) {
    std::vector<std::string> ids;
    g_engine = wf::ConsensusEngine(consensus_config_from(g_cfg), ids);
  }

  // ---- radio ---------------------------------------------------------------
  const int rs = g_radio.begin(g_cfg.lora_mhz, g_cfg.lora_bw, g_cfg.lora_sf,
                               g_cfg.lora_cr, g_cfg.lora_sync, g_cfg.lora_dbm, 8,
                               1.8f, false);
  logf("radio: begin(%0.1fMHz bw%0.0fk sf%u cr%u sync=0x%02X %ddBm) -> %s",
       g_cfg.lora_mhz, g_cfg.lora_bw, g_cfg.lora_sf, g_cfg.lora_cr,
       g_cfg.lora_sync, g_cfg.lora_dbm,
       rs == RADIOLIB_ERR_NONE ? "ok" : "FAILED");
  if (g_cfg.lora_mhz > 928.0f || g_cfg.lora_mhz < 902.0f) {
    logf("FATAL: %0.1f MHz is outside the 915 MHz US band (902-928) -- "
         "this build is US915 only", g_cfg.lora_mhz);
  }

  // The panel probe drives the GPIO21 reset pulse and calls Wire.begin() itself,
  // so it must run before anything else touches the I2C bus (task 0099 item 3).
  g_oled_ready = oled_probe_and_begin();

  if (g_role == wf::Role::Node) {
    // The PMS UART is NOT opened here any more. The power-state machine owns the
    // port and attaches it only while the sensor rail is up (task 0125 item 2);
    // the boot role probe already handed it back down.
#if WF_HAS_BME680
    if (!g_bme.begin(wf::kBme680Addr)) logf("bme680: not found at 0x%02X", wf::kBme680Addr);
#endif
    // Task 0119: one-shot, node-only pin evidence (see batt_adc_pin_probe).
    batt_adc_pin_probe();
  }

  // ---- provisioning decision ----------------------------------------------
  // Read once, after the panel is up (the gesture draws its countdown on the
  // panel) and before any network call. A BASE always brings the network up --
  // it is the uplink. A NODE brings it up only when the operator held BOOT, so
  // field mode carries no WiFi/AP/portal at all (task 0125 item 6).
  g_provisioning = (g_role == wf::Role::Base) ? true : provisioning_gesture();
  if (!g_provisioning) {
    logf("field mode: WiFi/AP/portal shed (node, no provisioning gesture); "
         "hold BOOT %u ms at boot to provision",
         (unsigned)wf::kProvisionHoldMs);
    return;  // nothing below this point may touch WiFi, TLS or the portal
  }

  // ---- network + portal ----------------------------------------------------
  // The setup AP comes up first and unconditionally, so portal_setup() below is
  // always reachable: with no stored SSID, and equally after a failed join
  // (task 0099 items 5 and 6 -- in that order, not the other way round).
  // Connect / disconnect logging (item 4): registered before the STA starts so
  // the first join is covered, not just later reconnects.
  WiFi.onEvent(wifi_event_log);
  wifi_begin(15000);
  // TLS cannot verify the pinned root at the 1970 boot clock, so the clock has
  // to be right before the first handshake.
  net_time_sync(15000);

  g_wifi_client.setCACert(wf::kIsrgRootX1Pem);  // pin the root; never setInsecure()
  logf("mqtt cfg: host=[%s] port=%u (TLS, pinned ISRG Root X1)",
       g_cfg.mqtt_host.c_str(), g_cfg.mqtt_port);
  g_mqtt.setServer(g_cfg.mqtt_host.c_str(), g_cfg.mqtt_port);
  portal_setup();
}

void loop() {
  if (g_role == wf::Role::Base) {
    // Keep trying to reach the AP. A failed join re-arms the setup AP, so the
    // portal is never lost (task 0099 item 6).
    base_network_tick();
    // The MQTT session is the one deadline in this firmware: mosquitto drops a
    // client that misses 1.5 x its 15 s keepalive, and 0103 saw exactly that
    // every ~22 s. So the client is serviced FIRST and every pass, and every
    // blocking call after it is bounded (base_radio_poll: kRadioPollMs).
    if (!g_mqtt.connected()) {
      const String client_id = String("wf-base-") + String((unsigned)g_cfg.node_id);
      if (g_mqtt.connect(client_id.c_str(), g_cfg.mqtt_user.c_str(),
                         g_cfg.mqtt_pass.c_str())) {
        logf("mqtt: connected to %s:%u", g_cfg.mqtt_host.c_str(), g_cfg.mqtt_port);
        offline_flush();
      } else {
        static uint32_t last_mqtt_err = 0;
        if (millis() - last_mqtt_err > 30000UL) {
          last_mqtt_err = millis();
          logf("mqtt: connect to %s:%u failed (state=%d)",
               g_cfg.mqtt_host.c_str(), g_cfg.mqtt_port, (int)g_mqtt.state());
        }
      }
    }
    g_mqtt.loop();
    offline_flush();
    g_server.handleClient();
    base_radio_poll();

    // offline rule: a node with no packet for 3 check-in periods is offline.
    const float offline_after = 3.0f * (float)g_cfg.checkin_s / 60.0f;
    for (auto& n : g_nodes) {
      const bool was = n.online;
      n.online = (now_min() - n.last_seen_min) <= offline_after;
      if (was && !n.online) {
        logf("node %s offline (no packet for %.0f min)", n.id.c_str(), offline_after);
        // No MQTT publish here (task 0105 item 3): the only topic the ACL
        // grants this device is `+/telemetry` and `+/events`, so the old
        // `<root>/node/<id>/state` notice was denied at the broker and never
        // reached anything. The backend derives staleness itself from the
        // reading's age (`/v1/nodes` carries last_seen_utc / status "stale"),
        // and this board still shows it on the OLED, so the notice is removed
        // rather than re-routed onto a topic whose schema it does not match.
      }
    }
    static uint32_t last_oled = 0;
    if (millis() - last_oled > 1000) {
      last_oled = millis();
      oled_render_base();
    }
    return;  // the base loop never sleeps
  }

  // ---- node: one power-state cycle per check-in period ---------------------
  //
  // Field build (kDeepSleepEnabled == true): node_run_cycle() ends in
  // enter_deep_sleep(), which never returns -- one full cycle per wake.
  //
  // Bench phase (gate open): the identical cycle runs, but its DEEP_SLEEP step
  // only logs the gate. The period is enforced here so the node cycles once per
  // check-in period rather than once per loop() pass, staying awake with
  // USB-serial live in between.
  static uint32_t last_cycle_ms = 0;
  static bool cycle_started = false;
  const uint32_t period_ms = (uint32_t)g_cfg.checkin_s * 1000UL;
  if (!cycle_started || (uint32_t)(millis() - last_cycle_ms) >= period_ms) {
    cycle_started = true;
    last_cycle_ms = millis();
    node_run_cycle();
  }

  // 0117/0118: the 1 Hz live poll and the 1 Hz OLED render run on every pass,
  // OUTSIDE the cycle, so the two cadences stay independent. The poll reads
  // bytes only while the sensor window is open (g_pms_uart_active); between
  // windows the PM row holds the last checksum-valid frame, which is the 0118
  // latch behaviour, not a regression.
  pms_live_poll();
  static uint32_t last_oled_ms = 0;
  if ((uint32_t)(millis() - last_oled_ms) > 1000UL) {
    last_oled_ms = millis();
    oled_render_node();
  }
  // A node carries the portal only in provisioning mode (task 0125 item 6).
  if (g_provisioning) g_server.handleClient();
  delay(100);  // stay responsive and keep the USB-CDC link enumerated
}
