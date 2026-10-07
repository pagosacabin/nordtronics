// 0122 — LiDAR LDS-006 bring-up on NodeMCU-32S.
//
// VERBATIM from the task's "Sketch" block: not one line added, removed or
// reformatted, so the bench observation is of the sketch Juno specified and not
// of a version this run tuned. If the bring-up needs a change (e.g. swapping
// green/blue, or a different PWM duty), that is the next task's scope.

#define LIDAR_RX 16
#define MOTOR_PWM 17

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, LIDAR_RX, -1);
  ledcSetup(0, 5000, 8);
  ledcAttachPin(MOTOR_PWM, 0);
  ledcWrite(0, 128);
  Serial.println("Motor PWM on, listening...");
}

void loop() {
  while (Serial2.available()) {
    Serial.printf("%02X ", Serial2.read());
  }
}
