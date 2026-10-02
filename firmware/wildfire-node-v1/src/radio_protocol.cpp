#include "radio_protocol.h"

#include <cstring>

namespace wf {
namespace {

inline void put_u16(uint8_t* p, uint16_t v) {
  p[0] = static_cast<uint8_t>(v & 0xFF);
  p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
}

inline uint16_t get_u16(const uint8_t* p) {
  return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << 8));
}

inline void put_u32(uint8_t* p, uint32_t v) {
  p[0] = static_cast<uint8_t>(v & 0xFF);
  p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
  p[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
  p[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}

inline uint32_t get_u32(const uint8_t* p) {
  return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
         (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

size_t payload_len_for(uint8_t type) {
  switch (type) {
    case kMsgCheckin:
    case kMsgAlarm:
      return kTelemetryPayloadBytes;
    case kMsgAck:
      return kAckPayloadBytes;
    case kMsgJoinReq:
      return kJoinReqPayloadBytes;
    case kMsgJoinAck:
      return kJoinAckPayloadBytes;
    default:
      return 0;
  }
}

}  // namespace

uint16_t crc16_ccitt(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (int b = 0; b < 8; ++b) {
      crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021)
                           : static_cast<uint16_t>(crc << 1);
    }
  }
  return crc;
}

size_t encode_frame(const Frame& f, uint8_t* out, size_t cap) {
  const size_t plen = payload_len_for(f.type);
  if (plen == 0) return 0;
  const size_t total = kHeaderBytes + plen + kCrcBytes;
  if (cap < total) return 0;

  out[0] = f.version;
  out[1] = f.type;
  put_u16(out + 2, f.node_id);
  put_u16(out + 4, f.seq);
  out[6] = static_cast<uint8_t>(plen);
  out[7] = f.flags;

  uint8_t* p = out + kHeaderBytes;
  switch (f.type) {
    case kMsgCheckin:
    case kMsgAlarm:
      put_u16(p + 0, f.pm1_x10);
      put_u16(p + 2, f.pm25_x10);
      put_u16(p + 4, f.pm10_x10);
      put_u16(p + 6, static_cast<uint16_t>(f.temp_c_x100));
      put_u16(p + 8, f.rh_x100);
      put_u32(p + 10, f.press_pa);
      put_u16(p + 14, f.batt_mv);
      p[16] = f.status;
      break;
    case kMsgAck:
      put_u16(p + 0, f.acked_seq);
      p[2] = f.ack_code;
      break;
    case kMsgJoinReq:
      put_u32(p + 0, f.hw_hash);
      break;
    case kMsgJoinAck:
      put_u16(p + 0, f.assigned_id);
      put_u16(p + 2, f.base_seq);
      break;
    default:
      return 0;
  }

  const uint16_t crc = crc16_ccitt(out, kHeaderBytes + plen);
  put_u16(out + kHeaderBytes + plen, crc);
  return total;
}

DecodeResult decode_frame(const uint8_t* in, size_t len, Frame* out) {
  if (in == nullptr || len < kHeaderBytes + kCrcBytes) return DecodeResult::TooShort;
  if (in[0] != kProtoVersion) return DecodeResult::BadVersion;

  const uint8_t type = in[1];
  const size_t plen = payload_len_for(type);
  if (plen == 0) return DecodeResult::UnknownType;
  if (in[6] != plen) return DecodeResult::BadLength;
  if (len != kHeaderBytes + plen + kCrcBytes) return DecodeResult::BadLength;

  const uint16_t crc_wire = get_u16(in + kHeaderBytes + plen);
  if (crc_wire != crc16_ccitt(in, kHeaderBytes + plen)) return DecodeResult::BadCrc;

  Frame f;
  f.version = in[0];
  f.type = type;
  f.node_id = get_u16(in + 2);
  f.seq = get_u16(in + 4);
  f.flags = in[7];

  const uint8_t* p = in + kHeaderBytes;
  switch (type) {
    case kMsgCheckin:
    case kMsgAlarm:
      f.pm1_x10 = get_u16(p + 0);
      f.pm25_x10 = get_u16(p + 2);
      f.pm10_x10 = get_u16(p + 4);
      f.temp_c_x100 = static_cast<int16_t>(get_u16(p + 6));
      f.rh_x100 = get_u16(p + 8);
      f.press_pa = get_u32(p + 10);
      f.batt_mv = get_u16(p + 14);
      f.status = p[16];
      break;
    case kMsgAck:
      f.acked_seq = get_u16(p + 0);
      f.ack_code = p[2];
      break;
    case kMsgJoinReq:
      f.hw_hash = get_u32(p + 0);
      break;
    case kMsgJoinAck:
      f.assigned_id = get_u16(p + 0);
      f.base_seq = get_u16(p + 2);
      break;
    default:
      return DecodeResult::UnknownType;
  }

  *out = f;
  return DecodeResult::Ok;
}

bool frame_plausible(const Frame& f) {
  if (f.node_id == 0 || f.node_id == 0xFFFF) return false;
  if (f.type == kMsgCheckin || f.type == kMsgAlarm) {
    if (f.pm25_x10 > 30000) return false;  // > 3000 ug/m3 is not a real reading
    if (f.rh_x100 > 10000) return false;
    if (f.temp_c_x100 < -6000 || f.temp_c_x100 > 9000) return false;
    if (f.press_pa != 0 && (f.press_pa < 30000 || f.press_pa > 120000)) return false;
    if (f.batt_mv != 0 && f.batt_mv > 6000) return false;
  }
  return true;
}

const char* decode_result_name(DecodeResult r) {
  switch (r) {
    case DecodeResult::Ok: return "OK";
    case DecodeResult::TooShort: return "TOO_SHORT";
    case DecodeResult::BadVersion: return "BAD_VERSION";
    case DecodeResult::UnknownType: return "UNKNOWN_TYPE";
    case DecodeResult::BadLength: return "BAD_LENGTH";
    case DecodeResult::BadCrc: return "BAD_CRC";
  }
  return "?";
}

}  // namespace wf
