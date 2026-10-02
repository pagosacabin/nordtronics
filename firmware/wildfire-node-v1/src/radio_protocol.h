// radio_protocol.h -- Wildfire radio protocol v1, versioned packed binary.
//
// The wire format lives here and ONLY here: the spec text in
// docs/wildfire/radio-protocol-v1.md is generated from (and checked against) this
// file by test_radio_protocol, which asserts the exact byte offsets and the
// golden frame hex. Portable C++17 -- compiled for the ESP32 and for the host
// test.
//
// Frame = HEADER (8 B, little endian) || PAYLOAD (type-dependent) || CRC16 (2 B).
// The version byte is byte 0, so a future v2 receiver can reject a v1 frame (and
// vice versa) before parsing anything else.
#pragma once

#include <cstddef>
#include <cstdint>

namespace wf {

constexpr uint8_t kProtoVersion = 1;

// Wire sizes. kHeaderBytes + payload + kCrcBytes must equal the encoded length.
constexpr size_t kHeaderBytes = 8;
constexpr size_t kCrcBytes = 2;
constexpr size_t kTelemetryPayloadBytes = 17;  // CHECKIN and ALARM
constexpr size_t kAckPayloadBytes = 3;
constexpr size_t kJoinReqPayloadBytes = 4;
constexpr size_t kJoinAckPayloadBytes = 4;

constexpr size_t kCheckinFrameBytes = kHeaderBytes + kTelemetryPayloadBytes + kCrcBytes;  // 27
constexpr size_t kAlarmFrameBytes = kCheckinFrameBytes;                                   // 27
constexpr size_t kAckFrameBytes = kHeaderBytes + kAckPayloadBytes + kCrcBytes;            // 13
constexpr size_t kJoinReqFrameBytes = kHeaderBytes + kJoinReqPayloadBytes + kCrcBytes;    // 14
constexpr size_t kJoinAckFrameBytes = kHeaderBytes + kJoinAckPayloadBytes + kCrcBytes;    // 14

// uint8_t so far.
enum MsgType : uint8_t {
  kMsgCheckin = 1,  // 12-minute routine check-in
  kMsgAlarm = 2,    // Watch/Alert raised: needs an ACK (retried)
  kMsgAck = 3,      // base -> node, acknowledges a specific sequence number
  kMsgJoinReq = 4,  // node announces itself; base replies only for provisioned IDs
  kMsgJoinAck = 5,  // base -> node, confirms provisioning + the ID it was given
};

// Header `flags` bits. Byte 7 of the frame.
enum HeaderFlag : uint8_t {
  kFlagAlarm = 1u << 0,      // payload is an alarm record (mirrors type == kMsgAlarm)
  kFlagAckRequired = 1u << 1,
  kFlagLowBatt = 1u << 2,    // battery mV below the portal threshold
  kFlagJoinPending = 1u << 3,
};

// Telemetry `status` bits (last payload byte of CHECKIN/ALARM). Sensor presence
// and health for the base's node table; nothing here changes the wire length.
enum StatusFlag : uint8_t {
  kStatusBmePresent = 1u << 0,
  kStatusPmsPresent = 1u << 1,
  kStatusPmsOk = 1u << 2,
  kStatusAs3935Present = 1u << 3,
  kStatusLightning = 1u << 4,
  kStatusSensorFault = 1u << 5,
};

// ACK codes carried in MsgAck.
enum AckCode : uint8_t {
  kAckOk = 0,
  kAckUnknownNode = 1,
  kAckBadFrame = 2,
  kAckNotPermitted = 3,
};

struct Frame {
  uint8_t version = kProtoVersion;
  uint8_t type = kMsgCheckin;
  uint8_t flags = 0;
  uint16_t node_id = 0;   // 0 = unprovisioned / not yours
  uint16_t seq = 0;       // wraps at 65535; per-node, incremented per TX

  // Telemetry payload (CHECKIN / ALARM)
  uint16_t pm1_x10 = 0;    // ug/m3 * 10
  uint16_t pm25_x10 = 0;   // ug/m3 * 10
  uint16_t pm10_x10 = 0;   // ug/m3 * 10
  int16_t temp_c_x100 = 0; // degC * 100, signed
  uint16_t rh_x100 = 0;    // %RH * 100
  uint32_t press_pa = 0;   // Pa, unsigned
  uint16_t batt_mv = 0;    // mV
  uint8_t status = 0;      // StatusFlag bits

  // ACK payload
  uint16_t acked_seq = 0;
  uint8_t ack_code = 0;

  // JOIN payloads
  uint32_t hw_hash = 0;     // JOIN_REQ: hash of the module MAC
  uint16_t assigned_id = 0; // JOIN_ACK: the ID the base provisioned
  uint16_t base_seq = 0;    // JOIN_ACK: base frame counter at provisioning
};

enum class DecodeResult : uint8_t {
  Ok = 0,
  TooShort = 1,
  BadVersion = 2,
  UnknownType = 3,
  BadLength = 4,
  BadCrc = 5,
};

// CRC16-CCITT (poly 0x1021, init 0xFFFF, no reflection, no final xor).
uint16_t crc16_ccitt(const uint8_t* data, size_t len);

// Encode into `out` (capacity `cap`). Returns the number of bytes written, or 0
// if the buffer is too small or the frame cannot be encoded.
size_t encode_frame(const Frame& f, uint8_t* out, size_t cap);

// Decode `len` bytes. On Ok, `out` carries every field. Any failure leaves the
// receiver to drop the frame and count it (the base never acts on a bad frame).
DecodeResult decode_frame(const uint8_t* in, size_t len, Frame* out);

// Plausibility check the base applies after decode: the node ID must be inside
// the provisioned range and a telemetry frame must not carry impossible values.
bool frame_plausible(const Frame& f);

const char* decode_result_name(DecodeResult r);

}  // namespace wf
