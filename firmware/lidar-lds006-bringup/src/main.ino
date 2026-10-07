// 0123 — LiDAR LDS-006: try UART start commands.
//
// Follow-up to 0122 (same project, same board, same bench wiring: green -> P16
// via the 10k/23k divider, blue -> P17 via 1.6k). Three changes per the task:
//   1. Serial2 is opened with BOTH roles: RX = GPIO16, TX = GPIO17. In 0122 the
//      TX argument was -1, so the ESP32 never drove the wire at all.
//   2. The LEDC PWM block is removed -- 5 kHz at duty 128 on blue did not spin
//      the motor (0122), and it must not fight the UART TX now sharing P17.
//   3. The three start sequences are sent with their gaps, and every byte is
//      echoed to Serial so the capture PROVES what went out on the wire.
//
// Nothing here guesses beyond the three sequences the task lists. The task also
// forbids swapping the software roles (P17 taken directly to 4 V is not safe
// without the divider), so the RX/TX assignment is exactly as specified.

#define LIDAR_RX 16
#define LIDAR_TX 17

static unsigned rxCount = 0;

static void sendCommand(const char* label, const uint8_t* bytes, size_t n) {
  Serial.printf("cmd: %s ->", label);
  for (size_t i = 0; i < n; ++i) {
    Serial2.write(bytes[i]);
    Serial.printf(" %02X", bytes[i]);
  }
  Serial.println();
  Serial2.flush();
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, LIDAR_RX, LIDAR_TX);
  Serial.println("LDS-006 UART start-command probe: RX=16 TX=17 @115200, listening...");

  const uint8_t kYdlStart[2] = {0xA5, 0x60};
  const uint8_t kRplStart[2] = {0xA5, 0x20};
  const uint8_t kYdlStop[2] = {0xA5, 0x65};

  delay(2000);                                        // let the LiDAR settle first
  sendCommand("1 YDLidar start (A5 60)", kYdlStart, 2);
  delay(3000);
  sendCommand("2 RPLidar start (A5 20)", kRplStart, 2);
  delay(3000);
  sendCommand("3a YDLidar stop (A5 65)", kYdlStop, 2);
  delay(1000);
  sendCommand("3b YDLidar start again (A5 60)", kYdlStart, 2);
  Serial.println("all three sequences sent -- now dumping anything received");
}

void loop() {
  while (Serial2.available()) {
    Serial.printf("%02X ", Serial2.read());
    if (++rxCount % 16 == 0) {
      Serial.println();
    }
  }
}
