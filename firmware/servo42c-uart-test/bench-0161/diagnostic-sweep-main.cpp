
// 0161 diagnostic (read-only): is the SERVO42C answering at all?
// Sends ONLY read commands (0x36 shaft angle, 0x30 encoder). No writes.
#include <Arduino.h>
static const int RPIN = 16, TPIN = 17;
static const uint8_t ADDRS[] = {0xE0, 0x00, 0x01, 0x02};
static const uint32_t BAUDS[] = {38400, 9600, 19200, 57600, 115200, 4800};

static void drain() { while (Serial2.available()) (void)Serial2.read(); }

static int listenRaw(uint32_t ms, uint8_t *buf, int cap) {
  int n = 0; uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    if (Serial2.available()) { uint8_t b = Serial2.read(); if (n < cap) buf[n++] = b; }
    else delay(1);
  }
  return n;
}

static void hex(const char *tag, const uint8_t *b, int n) {
  Serial.print("      "); Serial.print(tag);
  if (n == 0) { Serial.println(" (nothing)"); return; }
  for (int i = 0; i < n; i++) { Serial.print(' '); if (b[i] < 0x10) Serial.print('0'); Serial.print(b[i], HEX); }
  Serial.println();
}

void setup() {
  Serial.begin(115200); delay(400);
  Serial.println();
  Serial.println("=== 0161 UART diagnostic sweep (read-only 0x36/0x30) ===");

  for (unsigned bi = 0; bi < sizeof(BAUDS)/sizeof(BAUDS[0]); bi++) {
    Serial2.begin(BAUDS[bi], SERIAL_8N1, RPIN, TPIN);
    delay(50); drain();
    uint8_t idle[32];
    int nIdle = listenRaw(300, idle, sizeof(idle));
    Serial.printf("  baud %lu : idle bytes in 300ms = %d\n", (unsigned long)BAUDS[bi], nIdle);
    if (nIdle) hex("idle:", idle, nIdle > 16 ? 16 : nIdle);

    for (unsigned ai = 0; ai < sizeof(ADDRS)/sizeof(ADDRS[0]); ai++) {
      uint8_t f[3] = {ADDRS[ai], 0x36, 0}; f[2] = (uint8_t)(f[0] + f[1]);
      drain(); Serial2.write(f, 3); Serial2.flush();
      uint8_t r[16]; int n = listenRaw(150, r, sizeof(r));
      Serial.printf("    addr 0x%02X chk  -> %d bytes\n", ADDRS[ai], n);
      if (n) { hex("rx:", r, n > 8 ? 8 : n); }
    }
    // one plain frame (no checksum) at 0xE0
    uint8_t p[2] = {0xE0, 0x36}; drain(); Serial2.write(p, 2); Serial2.flush();
    uint8_t r2[16]; int n2 = listenRaw(150, r2, sizeof(r2));
    Serial.printf("    addr 0xE0 plain -> %d bytes\n", n2);
    if (n2) hex("rx:", r2, n2 > 8 ? 8 : n2);
    Serial2.end();
  }
  Serial.println("=== sweep complete ===");
}
void loop() { delay(1000); }
