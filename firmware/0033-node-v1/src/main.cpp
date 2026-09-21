#include <Arduino.h>

const char NODE_FW_VERSION[] = "0.1.0-scaffold";

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.print("Firmware version: ");
  Serial.println(NODE_FW_VERSION);
}

void loop() {
  Serial.println("scaffold alive");
  delay(1000);
}
