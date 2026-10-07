---
task_id: "0122"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP (iteration 0 -> 1) by an interactive session at Stephen's request,
  2026-10-07 11:50 MDT. The four cron worker jobs (c0be50a686c6, 7aff6948c2c1,
  8b1c9e1323c5, 5c1532977f15) were already PAUSED (since the 0119 bench block on
  2026-10-06) and stay paused for the duration, so no tick can resume this task
  against the same worktree; they are resumed once this task is staged.
  Scope: stand up the NodeMCU-32S in the toolchain, flash the sketch EXACTLY as
  the task gives it, capture 60 s of serial at 115200. No rewiring (bench wiring
  is Stephen's). Model tier: DeepSeek Flash (this session).
---
# 0122 — LiDAR LDS-006 bring-up on NodeMCU-32S

## Context
Stephen is bench-testing a salvaged robot-vacuum LiDAR (label reads LDS-006,
5V, ~0.43A). 4 wires: red (5V), black (GND), green, blue. Both signal wires sit
at ~4V DC idle, no activity on scope — the unit is silent waiting for input.
Motor does not spin on 5V alone; neither signal wire responds to DC high/low as
a simple enable. Theory: one wire is UART TX (data), the other is motor PWM in.

Stephen has wired it to a NodeMCU-32S (ESP-32S v1.1, classic ESP32) at the bench:
- LiDAR green → 10k → GPIO16 (P16) → 23k → GND (voltage divider, ~2.8V tap)
- LiDAR blue → GPIO17 (P17) via 1.6k series resistor
- LiDAR red → 5V bench supply, black → GND (common ground with NodeMCU USB ground)

## Task
1. Set up the NodeMCU-32S as a new board in your toolchain (ESP32 Arduino core
   2.x or PlatformIO espressif32 — your call, but record which so it's
   reproducible). Identify its USB serial port by-id.
2. Flash the sketch below.
3. Open the serial monitor at 115200 and observe for 60 seconds.
4. Report: (a) did the LiDAR turret spin? (b) did hex data appear on serial?
   (c) paste the first ~10 lines of hex if any.

## Sketch
```cpp
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
```

## Success criteria
- Board definition installed and documented (which core/env, which USB by-id path).
- Sketch flashed to the NodeMCU-32S, serial monitor captured.
- Clear report: motor spun or not, data present or not, hex sample if present.

## Constraints
- Do NOT rewire anything — Stephen did the bench wiring, just flash and observe.
- If the motor spins but no data appears, say so explicitly (next step is swapping
  green/blue, which Stephen will do at the bench).
- If the board won't flash, report the exact error rather than guessing.

## Reply format
Stage the reply in mailbox/staged/ per the README. Include: board setup used,
flash proof (esptool output hash or IDE success), serial monitor excerpt,
motor/data observations.
