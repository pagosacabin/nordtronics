// test_main.cpp -- host-side proof that the firmware implements EXACTLY the wire
// format documented in docs/wildfire/radio-protocol-v1.md.
//
// The point of this test is the "no undocumented fields on the wire" criterion:
// the offsets, the payload lengths, the frame sizes and the CRC are asserted
// against hand-built byte arrays, and the golden frames in the doc are encoded
// here and compared byte for byte. A field added to the struct without a doc
// update, or a doc entry with no field, changes one of these numbers.

#include <unity.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "radio_protocol.h"

using wf::DecodeResult;
using wf::Frame;

static int g_failures = 0;

#define CHECK(cond, msg)                                       \
  do {                                                         \
    if (!(cond)) {                                             \
      g_failures++;                                            \
      printf("    FAIL: %s\n", (msg));                         \
      TEST_ASSERT_TRUE_MESSAGE((cond), (msg));                 \
    }                                                          \
  } while (0)

static std::string hex(const uint8_t* p, size_t n) {
  static const char* d = "0123456789abcdef";
  std::string s;
  for (size_t i = 0; i < n; ++i) {
    s.push_back(d[p[i] >> 4]);
    s.push_back(d[p[i] & 0xF]);
  }
  return s;
}

// The canonical CHECKIN frame: version 1, node 0x1234, seq 7, flags 0x02.
static Frame golden_checkin() {
  Frame f;
  f.version = 1;
  f.type = wf::kMsgCheckin;
  f.node_id = 0x1234;
  f.seq = 7;
  f.flags = 0x02;
  f.pm1_x10 = 12;
  f.pm25_x10 = 137;
  f.pm10_x10 = 421;
  f.temp_c_x100 = 2234;
  f.rh_x100 = 4512;
  f.press_pa = 101325;
  f.batt_mv = 3990;
  f.status = 0x07;
  return f;
}

static void test_crc16_ccitt_known_vector() {
  const uint8_t v[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  const uint16_t crc = wf::crc16_ccitt(v, sizeof(v));
  printf("[crc] crc16_ccitt(\"123456789\") = 0x%04X (expect 0x29B1)\n", crc);
  CHECK(crc == 0x29B1, "CRC-16/CCITT-FALSE check value must be 0x29B1");
}

static void test_frame_sizes_match_the_documented_layout() {
  printf("[sizes] checkin=%zu alarm=%zu ack=%zu joinreq=%zu joinack=%zu (header=%zu crc=%zu)\n",
         wf::kCheckinFrameBytes, wf::kAlarmFrameBytes, wf::kAckFrameBytes,
         wf::kJoinReqFrameBytes, wf::kJoinAckFrameBytes, wf::kHeaderBytes, wf::kCrcBytes);
  CHECK(wf::kHeaderBytes == 8, "header must be 8 bytes");
  CHECK(wf::kCrcBytes == 2, "CRC must be 2 bytes");
  CHECK(wf::kTelemetryPayloadBytes == 17, "telemetry payload must be 17 bytes");
  CHECK(wf::kCheckinFrameBytes == 27, "CHECKIN frame must be 27 bytes");
  CHECK(wf::kAckFrameBytes == 13, "ACK frame must be 13 bytes");
  CHECK(wf::kJoinReqFrameBytes == 14, "JOIN_REQ frame must be 14 bytes");
}

static void test_golden_checkin_frame_hex() {
  uint8_t out[64];
  const size_t n = wf::encode_frame(golden_checkin(), out, sizeof(out));
  const std::string got = hex(out, n);
  const std::string want = "01013412070011020c008900a501ba08a011cd8b0100960f07e844";
  printf("[golden] checkin len=%zu hex=%s\n", n, got.c_str());
  CHECK(n == 27, "golden CHECKIN must encode to 27 bytes");
  CHECK(got == want, "golden CHECKIN hex must match the documented frame");
}

static void test_field_offsets_are_exactly_as_documented() {
  // Hand-built byte array with ONE distinctive value per field, then decoded.
  // If any offset moves, one of these assertions fails.
  uint8_t b[wf::kCheckinFrameBytes] = {0};
  b[0] = 1;                 // version
  b[1] = 1;                 // type = CHECKIN
  b[2] = 0x34; b[3] = 0x12; // node_id LE = 0x1234
  b[4] = 0x07; b[5] = 0x00; // seq LE = 7
  b[6] = 17;                // payload length
  b[7] = 0x05;              // flags
  b[8] = 0x0C;  b[9] = 0x00;   // pm1  = 12
  b[10] = 0x89; b[11] = 0x00;  // pm25 = 137
  b[12] = 0xA5; b[13] = 0x01;  // pm10 = 421
  b[14] = 0xBA; b[15] = 0x08;  // temp = 2234
  b[16] = 0xA0; b[17] = 0x11;  // rh   = 4512
  b[18] = 0xCD; b[19] = 0x8B; b[20] = 0x01; b[21] = 0x00;  // press = 101325
  b[22] = 0x96; b[23] = 0x0F;  // batt = 3990
  b[24] = 0x07;                // status
  const uint16_t crc = wf::crc16_ccitt(b, 25);
  b[25] = (uint8_t)(crc & 0xFF);
  b[26] = (uint8_t)(crc >> 8);

  Frame f;
  const DecodeResult r = wf::decode_frame(b, sizeof(b), &f);
  printf("[offsets] decode=%s version=%u type=%u node=0x%04X seq=%u plen=%u flags=0x%02X "
         "pm1=%u pm25=%u pm10=%u temp=%d rh=%u press=%lu batt=%u status=0x%02X\n",
         wf::decode_result_name(r), f.version, f.type, f.node_id, f.seq, b[6], f.flags,
         f.pm1_x10, f.pm25_x10, f.pm10_x10, f.temp_c_x100, f.rh_x100,
         (unsigned long)f.press_pa, f.batt_mv, f.status);
  CHECK(r == DecodeResult::Ok, "hand-built frame must decode");
  CHECK(f.version == 1, "byte 0 = version");
  CHECK(f.type == wf::kMsgCheckin, "byte 1 = type");
  CHECK(f.node_id == 0x1234, "bytes 2-3 = node_id, little endian");
  CHECK(f.seq == 7, "bytes 4-5 = seq, little endian");
  CHECK(f.flags == 0x05, "byte 7 = flags");
  CHECK(f.pm1_x10 == 12, "bytes 8-9 = PM1.0 x10");
  CHECK(f.pm25_x10 == 137, "bytes 10-11 = PM2.5 x10");
  CHECK(f.pm10_x10 == 421, "bytes 12-13 = PM10 x10");
  CHECK(f.temp_c_x100 == 2234, "bytes 14-15 = temperature x100, signed");
  CHECK(f.rh_x100 == 4512, "bytes 16-17 = humidity x100");
  CHECK(f.press_pa == 101325, "bytes 18-21 = pressure in Pa, unsigned 32-bit");
  CHECK(f.batt_mv == 3990, "bytes 22-23 = battery mV");
  CHECK(f.status == 0x07, "byte 24 = status flags");
}

static void test_all_frame_types_round_trip() {
  uint8_t out[64];
  // CHECKIN
  const Frame in = golden_checkin();
  const size_t n = wf::encode_frame(in, out, sizeof(out));
  Frame back;
  CHECK(n == 27 && wf::decode_frame(out, n, &back) == DecodeResult::Ok,
        "CHECKIN round trip");
  CHECK(back.pm25_x10 == 137 && back.press_pa == 101325 && back.temp_c_x100 == 2234,
        "CHECKIN fields survive the round trip");

  // ALARM shares the telemetry payload
  Frame alarm = in;
  alarm.type = wf::kMsgAlarm;
  alarm.flags = 0x03;
  const size_t na = wf::encode_frame(alarm, out, sizeof(out));
  CHECK(na == 27 && wf::decode_frame(out, na, &back) == DecodeResult::Ok,
        "ALARM round trip");
  CHECK(back.type == wf::kMsgAlarm, "ALARM type survives");

  // ACK
  Frame ack;
  ack.type = wf::kMsgAck;
  ack.node_id = 7;
  ack.seq = 9;
  ack.acked_seq = 0x0123;
  ack.ack_code = wf::kAckOk;
  const size_t nk = wf::encode_frame(ack, out, sizeof(out));
  const std::string ack_hex = hex(out, nk);
  CHECK(nk == 13 && wf::decode_frame(out, nk, &back) == DecodeResult::Ok,
        "ACK round trip");
  CHECK(back.acked_seq == 0x0123, "ACK carries the acknowledged sequence number");
  CHECK(ack_hex == "01030700090003002301009b25", "golden ACK hex");

  // JOIN_REQ
  Frame req;
  req.type = wf::kMsgJoinReq;
  req.node_id = 7;
  req.seq = 9;
  req.hw_hash = 0xDEADBEEF;
  const size_t nr = wf::encode_frame(req, out, sizeof(out));
  CHECK(nr == 14 && wf::decode_frame(out, nr, &back) == DecodeResult::Ok,
        "JOIN_REQ round trip");
  CHECK(back.hw_hash == 0xDEADBEEF, "JOIN_REQ carries the hardware hash");
  CHECK(hex(out, nr) == "0104070009000400efbeaddef261", "golden JOIN_REQ hex");

  // JOIN_ACK
  Frame jack;
  jack.type = wf::kMsgJoinAck;
  jack.node_id = 7;
  jack.seq = 9;
  jack.assigned_id = 0x0042;
  jack.base_seq = 0x0011;
  const size_t nj = wf::encode_frame(jack, out, sizeof(out));
  CHECK(nj == 14 && wf::decode_frame(out, nj, &back) == DecodeResult::Ok,
        "JOIN_ACK round trip");
  CHECK(back.assigned_id == 0x0042 && back.base_seq == 0x0011,
        "JOIN_ACK carries the provisioned ID");
  CHECK(hex(out, nj) == "01050700090004004200110066fe", "golden JOIN_ACK hex");
  printf("[round-trip] checkin/alarm/ack/join_req/join_ack all encode+decode; golden hex matches\n");
}

static void test_bad_frames_are_rejected() {
  uint8_t out[64];
  Frame f = golden_checkin();
  const size_t n = wf::encode_frame(f, out, sizeof(out));
  Frame back;

  // 1. corrupt payload byte -> CRC failure
  uint8_t bad[64];
  memcpy(bad, out, n);
  bad[10] ^= 0x40;
  CHECK(wf::decode_frame(bad, n, &back) == DecodeResult::BadCrc,
        "a flipped payload bit must fail the CRC");

  // 2. corrupt CRC -> CRC failure
  memcpy(bad, out, n);
  bad[n - 1] ^= 0xFF;
  CHECK(wf::decode_frame(bad, n, &back) == DecodeResult::BadCrc,
        "a corrupted CRC must be rejected");

  // 3. a v2 sender must be rejected before parsing
  memcpy(bad, out, n);
  bad[0] = 2;
  CHECK(wf::decode_frame(bad, n, &back) == DecodeResult::BadVersion,
        "a future protocol version must be rejected");

  // 4. unknown message type
  memcpy(bad, out, n);
  bad[1] = 99;
  CHECK(wf::decode_frame(bad, n, &back) == DecodeResult::UnknownType,
        "an unknown message type must be rejected");

  // 5. declared payload length that disagrees with the type
  memcpy(bad, out, n);
  bad[6] = 20;
  CHECK(wf::decode_frame(bad, n, &back) == DecodeResult::BadLength,
        "a payload length that disagrees with the type must be rejected");

  // 6. truncated frame
  CHECK(wf::decode_frame(out, n - 1, &back) == DecodeResult::BadLength,
        "a truncated frame must be rejected");
  CHECK(wf::decode_frame(out, 4, &back) == DecodeResult::TooShort,
        "a header-only fragment must be rejected");

  // 7. an oversized frame of the same type
  CHECK(wf::decode_frame(out, n + 1, &back) == DecodeResult::BadLength,
        "a frame longer than its type allows must be rejected");

  printf("[reject] crc/payload-crc/version/type/length/truncated/oversized all rejected\n");
}

static void test_plausibility_gate() {
  Frame f = golden_checkin();
  f.node_id = 0x1234;
  f.pm25_x10 = 137;
  f.rh_x100 = 4512;
  f.temp_c_x100 = 2234;
  f.press_pa = 101325;
  f.batt_mv = 3990;
  CHECK(wf::frame_plausible(f), "a normal frame must be plausible");

  f.node_id = 0;
  CHECK(!wf::frame_plausible(f), "node_id 0 (unprovisioned) must be implausible");
  f.node_id = 0xFFFF;
  CHECK(!wf::frame_plausible(f), "node_id 0xFFFF (broadcast) must be implausible");
  f.node_id = 1;

  f.pm25_x10 = 30001;  // > 3000 ug/m3
  CHECK(!wf::frame_plausible(f), "an impossible PM2.5 must be implausible");
  f.pm25_x10 = 137;

  f.rh_x100 = 10001;
  CHECK(!wf::frame_plausible(f), "humidity above 100% must be implausible");
  f.rh_x100 = 4512;

  f.temp_c_x100 = 9001;
  CHECK(!wf::frame_plausible(f), "temperature above 90 C must be implausible");
  f.temp_c_x100 = 2234;

  f.press_pa = 20000;
  CHECK(!wf::frame_plausible(f), "pressure below 300 hPa must be implausible");
  f.press_pa = 101325;

  // An ACK is not subject to the telemetry plausibility rules beyond the node ID.
  Frame ack;
  ack.type = wf::kMsgAck;
  ack.node_id = 7;
  ack.acked_seq = 1;
  CHECK(wf::frame_plausible(ack), "a well-formed ACK must be plausible");
  printf("[plausibility] telemetry ranges + node-id gate enforced on every decoded frame\n");
}

static void test_encode_refuses_a_short_buffer() {
  uint8_t small[4];
  const size_t n = wf::encode_frame(golden_checkin(), small, sizeof(small));
  CHECK(n == 0, "encode must refuse a buffer smaller than the frame");
  printf("[buffers] short encode buffers refused, no partial frames\n");
}

void setUp() {}
void tearDown() {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_crc16_ccitt_known_vector);
  RUN_TEST(test_frame_sizes_match_the_documented_layout);
  RUN_TEST(test_golden_checkin_frame_hex);
  RUN_TEST(test_field_offsets_are_exactly_as_documented);
  RUN_TEST(test_all_frame_types_round_trip);
  RUN_TEST(test_bad_frames_are_rejected);
  RUN_TEST(test_plausibility_gate);
  RUN_TEST(test_encode_refuses_a_short_buffer);
  const int rc = UNITY_END();
  printf("RADIO-PROTOCOL-TEST SUMMARY: checks-failed=%d\n", g_failures);
  printf("RADIO-PROTOCOL-TEST: %s\n", (rc == 0 && g_failures == 0) ? "PASS" : "FAIL");
  return (rc == 0 && g_failures == 0) ? 0 : 1;
}
