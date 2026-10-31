// ---------------------------------------------------------------------------
// 0160 — MKS SERVO42C UART bench self-test (classic ESP32 devkit, Arduino)
//
// Driver: MKS SERVO42C closed-loop stepper (Makerbase), firmware V1.1.x.
// Protocol reference: firmware/servo42c-uart-test/PROTOCOL.md (cited sources).
//
//   UART2 @ 9600 8N1, address 0xE0 (UartAddr default; baud set to 9600 to
//           match the driver's OLED UartBaud — see 0162)
//   frame  : <addr> <func> [data...] <chk>      chk = low byte of the byte sum
//            (firmware V1.0 has NO checksum byte — this sketch probes both)
//   reply  : <addr> [data...] <chk>             high byte first, 0x01 = ok
//
// Wiring (JST-SH 1.0mm 4-pin header on the driver: 3V3 / G / TX / RX):
//   ESP32 GPIO16 (UART2 RX)  <-  driver TX
//   ESP32 GPIO17 (UART2 TX)  ->  driver RX
//   ESP32 GND                --  driver G  (common ground is mandatory)
//   driver V+/Gnd            <-  12-24 V bench supply (do NOT power the motor
//                                stage from the ESP32's 3V3 pin)
//   The header's 3V3 pin is "floating" per the MKS manual — leave it open.
//   All logic is 3.3 V TTL; nothing 5 V may touch the UART lines.
//
// Bench self-test on boot (PASS/FAIL line per step, USB serial @115200):
//   1 framing/protocol (checksum vs plain, V1.1 vs V1.0 firmware)
//   2 shaft angle read (0x36)
//   3 following error read (0x39)
//   4 motor enable (0xF3) + En status verify (0x3A)
//   5 commanded +90 deg move accepted (0xFD)
//   6 measured move is +90 deg (+/- 5 deg)
//   7 commanded return move accepted (0xFD, reverse)
//   8 measured return lands back on the start angle (+/- 5 deg)
// ---------------------------------------------------------------------------

#include <Arduino.h>

// ----------------------------- configuration -------------------------------
static const uint32_t DRIVER_BAUD = 9600;    // OLED UartBaud (0162: driver set to 9600)
static const uint8_t  ADDR        = 0xE0;    // UartAddr default (MKS manual)
static const int      PIN_RX2     = 16;      // ESP32 UART2 RX  <- driver TX
static const int      PIN_TX2     = 17;      // ESP32 UART2 TX  -> driver RX

// Pulses per revolution = STEP_SUBDIVISION * motor full steps.
// The protocol has no "read MStep" command, so this MUST match the driver.
// MKS ships 16; check the "MStep" value on the driver's OLED and change it here
// if yours differs, otherwise the commanded move will not be 90 degrees.
static const uint16_t STEP_SUBDIVISION = 16;   // MStep on the OLED
static const uint16_t MOTOR_FULL_STEPS = 200;  // 1.8 deg motor (MotType default)

static const uint8_t  SPEED_GEAR      = 16;    // 0xF6/0xFD speed gear (low 7 bits)
static const uint32_t SETTLE_TIMEOUT_MS = 5000;
static const float    ANGLE_TOL_DEG   = 5.0f;

// ------------------------------ protocol -----------------------------------
// Function codes (MKS SERVO42C firmware V1.1, see PROTOCOL.md for sources)
static const uint8_t CMD_READ_ENCODER = 0x30;  // uint16 encoder value
static const uint8_t CMD_READ_PULSES  = 0x33;  // int32 received pulses
static const uint8_t CMD_READ_ANGLE   = 0x36;  // int32 shaft angle, 0..65535/rev
static const uint8_t CMD_READ_ERROR   = 0x39;  // int16 following error
static const uint8_t CMD_READ_EN      = 0x3A;  // uint8 En pin status 01/02/00
static const uint8_t CMD_READ_SHAFT   = 0x3E;  // uint8 blocked/unblocked
static const uint8_t CMD_ENABLE       = 0xF3;  // 01 enable, 00 disable
static const uint8_t CMD_STOP         = 0xF7;  // stop motion
static const uint8_t CMD_MOVE_PULSES  = 0xFD;  // speed/dir + int32 pulse count

enum Protocol { PROTO_UNKNOWN = 0, PROTO_CHECKSUM, PROTO_PLAIN };
static Protocol proto = PROTO_UNKNOWN;

// ------------------------------ helpers ------------------------------------
static void uartFlush() {
  while (Serial2.available()) (void)Serial2.read();
}

// Read whatever the driver sends, stopping after a 15 ms idle gap.
static int readReply(uint8_t *buf, size_t cap, uint32_t timeoutMs) {
  size_t n = 0;
  uint32_t t0 = millis();
  uint32_t last = 0;
  while (millis() - t0 < timeoutMs) {
    if (Serial2.available()) {
      uint8_t b = (uint8_t)Serial2.read();
      if (n < cap) buf[n++] = b;
      last = millis();
    } else if (n > 0 && millis() - last > 15) {
      break;                       // frame finished (no trailing bytes)
    }
    delay(1);
  }
  return (int)n;
}

static uint8_t checksum(const uint8_t *buf, size_t n) {
  uint8_t s = 0;
  for (size_t i = 0; i < n; i++) s += buf[i];
  return s;
}

static void printFrame(const char *tag, const uint8_t *buf, size_t n) {
  Serial.print("    ");
  Serial.print(tag);
  for (size_t i = 0; i < n; i++) {
    Serial.print(' ');
    if (buf[i] < 0x10) Serial.print('0');
    Serial.print(buf[i], HEX);
  }
  Serial.println();
}

// Send one frame and collect the reply. withChk selects the framing generation.
static int xfer(uint8_t func, const uint8_t *data, uint8_t nData,
                uint8_t *reply, size_t cap, bool withChk) {
  uint8_t frame[12];
  size_t k = 0;
  frame[k++] = ADDR;
  frame[k++] = func;
  for (uint8_t i = 0; i < nData; i++) frame[k++] = data[i];
  if (withChk) {
    const uint8_t chk = checksum(frame, k);   // sum BEFORE the check byte
    frame[k++] = chk;
  }

  uartFlush();
  Serial2.write(frame, k);
  Serial2.flush();
  printFrame("TX:", frame, k);

  int n = readReply(reply, cap, 300);
  if (n > 0) printFrame("RX:", reply, (size_t)n);
  else       Serial.println("    RX: (no reply)");
  return n;
}

// Expected reply length: addr + nData + (1 if this generation uses a checksum).
static bool replyOk(const uint8_t *reply, int n, uint8_t nData, bool withChk) {
  int want = 1 + nData + (withChk ? 1 : 0);
  if (n != want) return false;
  if (reply[0] != ADDR) return false;
  if (withChk && checksum(reply, (size_t)want - 1) != reply[want - 1]) return false;
  return true;
}

// Query that returns nData payload bytes in out[].
static bool query(uint8_t func, uint8_t nData, uint8_t *out) {
  uint8_t reply[10];
  bool withChk = (proto == PROTO_CHECKSUM);
  int n = xfer(func, nullptr, 0, reply, sizeof(reply), withChk);
  if (!replyOk(reply, n, nData, withChk)) return false;
  memcpy(out, reply + 1, nData);
  return true;
}

// Command taking one payload byte; verifies result==0x01.
static bool command1(uint8_t func, uint8_t arg) {
  uint8_t reply[10];
  bool withChk = (proto == PROTO_CHECKSUM);
  int n = xfer(func, &arg, 1, reply, sizeof(reply), withChk);
  return replyOk(reply, n, 1, withChk) && reply[1] == 0x01;
}

static int32_t be32(const uint8_t *p) {
  return ((int32_t)p[0] << 24) | ((int32_t)p[1] << 16) |
         ((int32_t)p[2] << 8) | (int32_t)p[3];
}

static bool readAngleRaw(int32_t *raw) {
  uint8_t d[4];
  if (!query(CMD_READ_ANGLE, 4, d)) return false;
  *raw = be32(d) & 0xFFFF;   // one revolution = 0..65535
  return true;
}

static float rawToDeg(int32_t raw) { return raw * 360.0f / 65536.0f; }

// Shortest signed difference between two 16-bit angle positions.
static int32_t angleDelta(int32_t from, int32_t to) {
  int32_t d = (to - from + 32768) & 0xFFFF;
  if (d < 0) d += 65536;
  return d - 32768;
}

// ------------------------ protocol detection -------------------------------
// V1.1 firmware answers with a trailing 8-bit sum; V1.0 has no check byte.
static Protocol detectProtocol() {
  uint8_t reply[10];

  // Try the checksummed frame first: a V1.1 driver answers 6 bytes for 0x36.
  int n = xfer(CMD_READ_ANGLE, nullptr, 0, reply, sizeof(reply), true);
  if (replyOk(reply, n, 4, true)) return PROTO_CHECKSUM;

  // Plain frame: a V1.0 driver answers 5 bytes for 0x36.
  n = xfer(CMD_READ_ANGLE, nullptr, 0, reply, sizeof(reply), false);
  if (replyOk(reply, n, 4, false)) return PROTO_PLAIN;

  return PROTO_UNKNOWN;
}

// --------------------------- move + settle ---------------------------------
static bool movePulses(bool forward, uint32_t pulses) {
  uint8_t arg = (uint8_t)((forward ? 0x00 : 0x80) | (SPEED_GEAR & 0x7F));
  // The pulse field grew from int16 (V1.0) to int32 (V1.1): the two firmware
  // generations are NOT frame-compatible for 0xFD. Build the right one.
  uint8_t data[5];
  uint8_t nData;
  if (proto == PROTO_CHECKSUM) {          // V1.1: dir/speed + int32 pulses
    if (pulses > 0x7FFFFFFFUL) pulses = 0x7FFFFFFFUL;
    data[0] = arg;
    data[1] = (uint8_t)(pulses >> 24);
    data[2] = (uint8_t)(pulses >> 16);
    data[3] = (uint8_t)(pulses >> 8);
    data[4] = (uint8_t)(pulses);
    nData = 5;
  } else {                                // V1.0: dir/speed + int16 pulses
    if (pulses > 0xFFFFUL) pulses = 0xFFFFUL;
    data[0] = arg;
    data[1] = (uint8_t)(pulses >> 8);
    data[2] = (uint8_t)(pulses);
    nData = 3;
  }
  uint8_t reply[10];
  bool withChk = (proto == PROTO_CHECKSUM);
  int n = xfer(CMD_MOVE_PULSES, data, nData, reply, sizeof(reply), withChk);
  return replyOk(reply, n, 1, withChk) && reply[1] == 0x01;
}

// Poll the shaft angle until it stops changing (or the timeout expires).
static bool waitForSettle(int32_t *finalRaw) {
  int32_t prev = 0, cur = 0;
  uint8_t stable = 0;
  uint32_t t0 = millis();

  if (!readAngleRaw(&prev)) return false;
  while (millis() - t0 < SETTLE_TIMEOUT_MS) {
    delay(100);
    if (!readAngleRaw(&cur)) continue;
    if (labs((long)angleDelta(prev, cur)) < 30) {   // < 0.17 deg per 100 ms
      if (++stable >= 5) { *finalRaw = cur; return true; }
    } else {
      stable = 0;
    }
    prev = cur;
  }
  *finalRaw = cur;
  return false;
}

// ------------------------------- test --------------------------------------
static int failures = 0;

static void step(int n, const char *name, bool ok, const char *fmt, ...) {
  char detail[160];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(detail, sizeof(detail), fmt, ap);
  va_end(ap);
  Serial.printf("STEP %d: %-38s %s  (%s)\n", n, name, ok ? "PASS" : "FAIL", detail);
  if (!ok) failures++;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== MKS SERVO42C UART bench self-test (build 0162) ===");
  Serial.printf("UART2 9600 8N1  addr 0x%02X  RX=GPIO%d TX=GPIO%d\n",
                ADDR, PIN_RX2, PIN_TX2);
  Serial.printf("Assume MStep=%u, motor %u full steps -> %lu pulses/rev, "
                "90 deg = %lu pulses\n",
                STEP_SUBDIVISION, MOTOR_FULL_STEPS,
                (unsigned long)STEP_SUBDIVISION * MOTOR_FULL_STEPS,
                (unsigned long)(STEP_SUBDIVISION * MOTOR_FULL_STEPS / 4));

  Serial2.begin(DRIVER_BAUD, SERIAL_8N1, PIN_RX2, PIN_TX2);
  delay(50);
  uartFlush();

  // ---- step 1: framing / protocol generation
  Serial.println("-- probing driver framing (0x36 both generations) --");
  proto = detectProtocol();
  step(1, "uart framing / protocol", proto != PROTO_UNKNOWN,
       proto == PROTO_CHECKSUM ? "V1.1 firmware, checksummed frames"
       : proto == PROTO_PLAIN  ? "V1.0 firmware, no checksum byte"
                               : "no valid reply on either framing");
  if (proto == PROTO_UNKNOWN) {
    Serial.println("HINTS: check TX/RX swap, common GND, UartAddr=0xE0, "
                   "UartBaud=9600, CtrMode=CR_UART on the OLED.");
    return;   // nothing else is meaningful without a link
  }

  // ---- step 2: shaft angle
  int32_t startRaw = 0;
  bool ok = readAngleRaw(&startRaw);
  step(2, "read shaft angle (0x36)", ok,
       ok ? "raw %ld = %.2f deg" : "no valid reply",
       (long)startRaw, rawToDeg(startRaw));
  if (!ok) { Serial.println("ABORT: no angle feedback, later steps meaningless."); return; }

  // ---- step 3: following error
  uint8_t eb[2];
  bool errOk = query(CMD_READ_ERROR, 2, eb);
  int16_t ferr = errOk ? (int16_t)((eb[0] << 8) | eb[1]) : 0;
  step(3, "read following error (0x39)", errOk,
       errOk ? "raw %d = %.2f deg" : "no valid reply",
       (int)ferr, ferr * 360.0f / 65536.0f);

  // ---- step 4: enable + confirm
  uint8_t enb[1];
  bool enAck = command1(CMD_ENABLE, 0x01);
  bool enRead = query(CMD_READ_EN, 1, enb);
  step(4, "motor enable (0xF3=01, 0x3A)", enAck && enRead && enb[0] == 0x01,
       "ack=%d status=0x%02X", (int)enAck, enRead ? enb[0] : 0xFF);
  delay(200);

  // ---- step 5/6: +90 deg relative move (0xFD; protocol has no absolute move)
  const uint32_t quarterTurn =
      (uint32_t)STEP_SUBDIVISION * MOTOR_FULL_STEPS / 4;
  bool ack = movePulses(true, quarterTurn);
  step(5, "command +90 deg move (0xFD)", ack,
       ack ? "%lu pulses fwd @ gear %u" : "driver rejected the frame",
       (unsigned long)quarterTurn, SPEED_GEAR);
  if (!ack)
    Serial.println("HINT: a rejected 0xFD usually means CtrMode is not "
                   "CR_UART on the driver's OLED menu.");

  int32_t afterRaw = startRaw;
  bool settled = waitForSettle(&afterRaw);
  int32_t d1 = angleDelta(startRaw, afterRaw);
  float movedDeg = d1 * 360.0f / 65536.0f;
  step(6, "measured move is +90 deg", settled && ack &&
       fabsf(fabsf(movedDeg) - 90.0f) <= ANGLE_TOL_DEG,
       "moved %.2f deg in %s", movedDeg, settled ? "time" : "TIMEOUT");
  if (ack && settled && fabsf(fabsf(movedDeg) - 90.0f) > ANGLE_TOL_DEG)
    Serial.printf("HINT: %.2f deg measured for %lu pulses -> set "
                  "STEP_SUBDIVISION to the MStep shown on the OLED "
                  "(expected %u).\n", movedDeg, (unsigned long)quarterTurn,
                  STEP_SUBDIVISION);

  // ---- step 7/8: return to the start angle with a reverse relative move
  bool back = movePulses(false, quarterTurn);
  step(7, "command return move (0xFD rev)", back,
       back ? "%lu pulses rev @ gear %u" : "driver rejected the frame",
       (unsigned long)quarterTurn, SPEED_GEAR);

  int32_t homeRaw = afterRaw;
  bool settled2 = waitForSettle(&homeRaw);
  int32_t d2 = angleDelta(startRaw, homeRaw);
  float residDeg = d2 * 360.0f / 65536.0f;
  step(8, "returned to start angle", settled2 && back &&
       fabsf(residDeg) <= ANGLE_TOL_DEG,
       "residual %.2f deg from start", residDeg);

  // ---- teardown / diagnostics (informational, not graded)
  // 0xF7 carries no payload byte — only its 1-byte result comes back.
  uint8_t stopRes[1];
  if (!query(CMD_STOP, 1, stopRes) || stopRes[0] != 0x01)
    Serial.println("INFO: stop (0xF7) did not acknowledge — motor may still be enabled.");
  uint8_t sh[1];
  if (query(CMD_READ_SHAFT, 1, sh))
    Serial.printf("INFO: shaft status 0x%02X (%s)\n", sh[0],
                  sh[0] == 0x01 ? "blocked" : sh[0] == 0x02 ? "unblocked" : "error");
  // 0x33 answers int16 on V1.0 firmware and int32 on V1.1.
  if (proto == PROTO_CHECKSUM) {
    uint8_t pb[4];
    if (query(CMD_READ_PULSES, 4, pb))
      Serial.printf("INFO: received pulses %ld\n", (long)be32(pb));
  } else {
    uint8_t pb[2];
    if (query(CMD_READ_PULSES, 2, pb))
      Serial.printf("INFO: received pulses %d\n", (int16_t)((pb[0] << 8) | pb[1]));
  }

  Serial.println();
  Serial.printf("SELF-TEST: %s (%d of 8 steps failed)\n",
                failures == 0 ? "PASS" : "FAIL", failures);
  Serial.println("=== self-test complete ===");
}

void loop() {
  delay(1000);   // one-shot bench test; nothing runs continuously
}
