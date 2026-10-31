// ---------------------------------------------------------------------------
// 0165 — MKS SERVO42C: can the driver's SETTINGS be read over UART?
//
// Read-only probe. The firmware sends only single-function queries from the
// read family and prints every raw reply byte. It never sends a set/restore
// command, never enables the motor and never moves the shaft.
//
// Why each candidate is on the list:
//   documented reads (MKS wiki, 42C V1.1):
//     0x30 encoder u16 | 0x33 received pulses i32 | 0x36 shaft angle i32
//     0x39 following error i16 | 0x3A En pin status u8 | 0x3E shaft status u8
//   third-party claims to test (the "menu-equivalence" table in 0165):
//     0x31 (claimed multi-turn position)   -> is there a reply at all?
//     0x32 (claimed microstep read-back)   -> is there a reply at all?
//     0x33 (claimed "active step mode 05") -> payload SIZE discriminates:
//            a 1-byte mode reply is 3 bytes, an i32 counter reply is 6.
//     0x3A (claimed "mA e.g. 1789")        -> needs 2 payload bytes to hold a
//            current; a 1-byte status reply cannot.
//
//   NOT SENT, deliberately: 0x3F. The lead table calls it "firmware version",
//   but the manufacturer documents `e0 3f tCHK` as **Restore default
//   parameters** ("Same as the Restore option on screen", and after it the
//   serial baud must be re-set). Sending it would wipe the driver's settings,
//   which this task forbids. A hardware reply cannot be obtained for it without
//   violating read-only, so its framing is settled from the manufacturer's own
//   page instead (see PROTOCOL.md / bench-0165/README.md).
//
// Frame: <addr> <func> <chk>, chk = low byte of the additive byte sum.
// Reply: <addr> <payload...> <rchk>. The driver echoes the request first at
// 9600 baud (0162), so the reply is extracted as the LAST valid frame.
// ---------------------------------------------------------------------------

#include <Arduino.h>

static const char    *BUILD_TAG   = "0165-C (read-only settings probe, 3 attempts/code, controls)";
static const uint32_t DRIVER_BAUD = 9600;    // OLED UartBaud (0162)
static const uint8_t  ADDR        = 0xE0;    // UartAddr default
static const int      PIN_RX2     = 16;
static const int      PIN_TX2     = 17;

static bool withChk = true;                  // V1.1 framing (detected below)

// ------------------------------- helpers -----------------------------------
static uint8_t checksum(const uint8_t *buf, size_t n) {
  uint8_t s = 0;
  for (size_t i = 0; i < n; i++) s += buf[i];
  return s;
}

static void uartFlush() { while (Serial2.available()) (void)Serial2.read(); }

static int readReply(uint8_t *buf, size_t cap, uint32_t timeoutMs) {
  size_t n = 0;
  uint32_t t0 = millis(), last = 0;
  while (millis() - t0 < timeoutMs) {
    if (Serial2.available()) {
      uint8_t b = (uint8_t)Serial2.read();
      if (n < cap) buf[n++] = b;
      last = millis();
    } else if (n > 0 && millis() - last > 15) {
      break;
    }
    delay(1);
  }
  return (int)n;
}

static void printHex(const char *tag, const uint8_t *b, size_t n) {
  Serial.print(tag);
  for (size_t i = 0; i < n; i++) {
    Serial.print(' ');
    if (b[i] < 0x10) Serial.print('0');
    Serial.print(b[i], HEX);
  }
}

// Find the driver reply: the last <addr> ... whose trailing check byte matches
// the additive sum of everything from <addr> up to it. Returns payload length
// and fills payload.
static int findReply(const uint8_t *raw, int n, uint8_t *payload, int *startIdx) {
  if (n < 2) return -1;
  for (int i = n - 2; i >= 0; i--) {
    if (raw[i] != ADDR) continue;
    if (checksum(raw + i, (size_t)(n - i - 1)) != raw[n - 1]) continue;
    const int plen = n - i - 2;
    if (plen < 0 || plen > 16) continue;
    if (plen) memcpy(payload, raw + i + 1, (size_t)plen);
    if (startIdx) *startIdx = i;
    return plen;
  }
  return -1;
}

static int32_t be32(const uint8_t *p) {
  return ((int32_t)p[0] << 24) | ((int32_t)p[1] << 16) |
         ((int32_t)p[2] << 8) | (int32_t)p[3];
}

// Send one query (addr, func, chk) and report the raw traffic + payload length.
// The 9600 link drops/corrupts the odd reply (0162/0163), so every code gets up
// to three attempts; every attempt's raw bytes are printed, and the first valid
// checked frame decides the result. A code that answers nothing still reports
// "none" after all three.
static int probeFunc(uint8_t func, uint8_t *payload, const char *label) {
  uint8_t frame[4], raw[64];
  size_t k = 0;
  frame[k++] = ADDR;
  frame[k++] = func;
  if (withChk) frame[k++] = checksum(frame, 2);

  Serial.println();
  Serial.print("PROBE func=0x");
  if (func < 0x10) Serial.print('0');
  Serial.print(func, HEX);
  Serial.print("  ");
  Serial.print(label);
  Serial.println();
  printHex("  TX:", frame, k);
  Serial.println();

  int result = -1;
  for (int attempt = 1; attempt <= 3; attempt++) {
    uartFlush();
    Serial2.write(frame, k);
    Serial2.flush();

    int n = readReply(raw, sizeof(raw), 400);
    Serial.print("  RX[");
    Serial.print(attempt);
    Serial.print("]:");
    if (n <= 0) {
      Serial.println(" (no reply, 400 ms)");
      continue;
    }
    printHex("", raw, (size_t)n);
    Serial.print("  (raw_len=");
    Serial.print(n);
    Serial.println(")");

    int start = -1;
    const int plen = findReply(raw, n, payload, &start);
    if (plen < 0) {
      Serial.println("    -> no valid checked frame (damaged reply)");
      continue;
    }
    // The driver echoes the request at 9600 baud (0162). If the only valid frame
    // in the capture IS that echo, no reply was sent.
    if (start == 0 && (size_t)n == k) {
      bool echoOnly = true;
      for (size_t i = 0; i < k; i++) if (raw[i] != frame[i]) { echoOnly = false; break; }
      if (echoOnly) {
        Serial.println("    -> echo only, no reply");
        continue;
      }
    }
    Serial.print("    -> reply_frame_start=");
    Serial.print(start);
    Serial.print("  payload_len=");
    Serial.println(plen);
    result = plen;
    break;
  }
  if (result < 0) Serial.println("  payload_len: none");
  return result;
}

// ------------------------ framing detection --------------------------------
// A checksummed 0x36 answers with 6 bytes (addr + 4 + chk); V1.0 firmware
// answers the plain frame with 5 bytes. Detect, don't assume.
static void detectFraming() {
  uint8_t raw[64];
  for (int attempt = 0; attempt < 3; attempt++) {
    uint8_t frame[3] = {ADDR, 0x36, 0};
    frame[2] = checksum(frame, 2);
    uartFlush();
    Serial2.write(frame, 3);
    Serial2.flush();
    int n = readReply(raw, sizeof(raw), 400);
    uint8_t payload[16];
    if (n > 0 && findReply(raw, n, payload, nullptr) == 4) { withChk = true; return; }
    frame[1] = 0x36;                       // plain (no check byte)
    uartFlush();
    Serial2.write(frame, 2);
    Serial2.flush();
    n = readReply(raw, sizeof(raw), 400);
    if (n >= 5) { withChk = false; return; }
    delay(50);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial2.begin(DRIVER_BAUD, SERIAL_8N1, PIN_RX2, PIN_TX2);
  delay(100);

  Serial.println();
  Serial.println("==========================================================");
  Serial.print("SERVO42C settings-read probe — ");
  Serial.println(BUILD_TAG);
  Serial.print("link: UART2 ");
  Serial.print(DRIVER_BAUD);
  Serial.print(" 8N1, addr 0x");
  Serial.println(ADDR, HEX);
  Serial.println("READ-ONLY: queries only, no set/restore/enable/move frames");
  Serial.println("==========================================================");

  detectFraming();
  Serial.print("framing: ");
  Serial.println(withChk ? "V1.1 (checksum, 6-byte 0x36 reply)" : "V1.0 (plain, 5-byte 0x36 reply)");

  uint8_t p[16];

  // --- documented reads -----------------------------------------------------
  int n30 = probeFunc(0x30, p, "documented: encoder u16");
  if (n30 == 2) {
    Serial.print("  decode: encoder=");
    Serial.println((uint16_t)((p[0] << 8) | p[1]));
  }

  int n33 = probeFunc(0x33, p, "documented: received pulses i32 | CLAIMED: work mode");
  if (n33 == 4) {
    Serial.print("  decode: pulses=");
    Serial.println(be32(p));
  }

  int n36 = probeFunc(0x36, p, "documented: shaft angle i32");
  if (n36 == 4) {
    Serial.print("  decode: angle_raw=");
    Serial.print(be32(p) & 0xFFFF);
    Serial.print("  deg=");
    Serial.println((be32(p) & 0xFFFF) * 360.0f / 65536.0f, 2);
  }

  int n39 = probeFunc(0x39, p, "documented: following error i16");
  if (n39 == 2) {
    Serial.print("  decode: error=");
    Serial.println((int16_t)((p[0] << 8) | p[1]));
  }

  int n3a = probeFunc(0x3A, p, "documented: En pin status u8 | CLAIMED: motor current mA");
  if (n3a == 1) {
    Serial.print("  decode: en_status=0x");
    Serial.println(p[0], HEX);
  }

  int n3e = probeFunc(0x3E, p, "documented: shaft status u8");
  if (n3e == 1) {
    Serial.print("  decode: shaft_status=0x");
    Serial.println(p[0], HEX);
  }

  // --- third-party claims ---------------------------------------------------
  int n31 = probeFunc(0x31, p, "CLAIMED: multi-turn position (not in MKS table)");
  if (n31 >= 0) {
    Serial.print("  decode: raw_payload=");
    for (int i = 0; i < n31; i++) { Serial.print(' '); if (p[i] < 0x10) Serial.print('0'); Serial.print(p[i], HEX); }
    Serial.println();
  }

  int n32 = probeFunc(0x32, p, "CLAIMED: microstep read-back (not in MKS table)");
  if (n32 >= 0) {
    Serial.print("  decode: raw_payload=");
    for (int i = 0; i < n32; i++) { Serial.print(' '); if (p[i] < 0x10) Serial.print('0'); Serial.print(p[i], HEX); }
    Serial.println();
  }

  // Re-read 0x32 once more: silence must be repeatable, not a timing artefact.
  (void)probeFunc(0x32, p, "CLAIMED 0x32, second attempt (repeatability)");

  // --- negative control ------------------------------------------------------
  // 0x37 and 0x77 are not in the MKS command table. If an unsupported code
  // answers with the SAME 1-byte 0x00 frame as 0x31/0x32, then that frame is the
  // generic "not supported / failed" reply, not data of any kind.
  (void)probeFunc(0x37, p, "CONTROL: undocumented code 0x37");
  (void)probeFunc(0x77, p, "CONTROL: undocumented code 0x77");

  Serial.println();
  Serial.println("----------------------------------------------------------");
  Serial.println("SUMMARY");
  Serial.print("  0x30 payload_len="); Serial.println(n30);
  Serial.print("  0x31 payload_len="); Serial.println(n31);
  Serial.print("  0x32 payload_len="); Serial.println(n32);
  Serial.print("  0x33 payload_len="); Serial.println(n33);
  Serial.print("  0x36 payload_len="); Serial.println(n36);
  Serial.print("  0x39 payload_len="); Serial.println(n39);
  Serial.print("  0x3A payload_len="); Serial.println(n3a);
  Serial.print("  0x3E payload_len="); Serial.println(n3e);
  Serial.println("  0x3F NOT SENT: MKS documents it as RESTORE DEFAULTS (write).");
  Serial.println("PROBE-COMPLETE");
}

void loop() {
  // One shot per boot. Re-run by resetting the board.
  delay(1000);
}
