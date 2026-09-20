#include <Arduino.h>

RTC_DATA_ATTR int boot_count = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  boot_count++;
  Serial.printf("boot #%d\n", boot_count);
  Serial.println("sleeping 10s");
  esp_sleep_enable_timer_wakeup(10 * 1000000);
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {
  // Should never reach here
}