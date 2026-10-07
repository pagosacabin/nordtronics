---
task_id: "0122"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  branch: "hermes/0122-lidar-nodemcu"
  sha: "20b9bada1e7fe7097521389c264137eb5d0db788"   # == git ls-remote --heads origin hermes/0122-lidar-nodemcu
  files:
    - firmware/lidar-lds006-bringup/platformio.ini
    - firmware/lidar-lds006-bringup/src/main.ino
  board_setup: "PlatformIO Core 6.1.18, platform espressif32 @ 6.12.0, which ships arduino-esp32 core 2.0.17 (PlatformIO package version string 3.20017.241212+sha.dcc1105b). board = nodemcu-32s (ESP-32S v1.1, classic ESP32; esptool identified the silicon as ESP32-D0WD-V3 revision v3.1). Core 2.x is required, not cosmetic: the task's sketch calls ledcSetup()/ledcAttachPin()/ledcWrite(), the 2.x LEDC API that core 3.x replaced with ledcAttach(). USB port pinned by-id, never by name: /dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0 -> /dev/ttyUSB0 (the ESP32-S3 node is ttyACM0 and the base ttyACM2 on this host, so a bare ttyUSB0 reference was avoided on purpose)."
  flash: "pio run -t upload, repeated 3x, identical every time. esptool wrote 0x1000 17536 B (bootloader), 0x8000 3072 B (partition table), 0xe000 8192 B (boot_app0), 0x10000 273824 B (application) -- each followed by 'Hash of data verified.' -- then 'Hard resetting via RTS pin...' and SUCCESS (7.12 s / 7.51 s / 7.36 s). Chip MAC 00:70:07:e6:42:70. Built firmware.bin 273824 bytes, sha256 0e2b730c3127ff745dac3647f7776297c122396e7a07ce161e813c2c92bab628. No CI run exists for this branch's project and none is offered: .github/workflows/platformio.yml builds only firmware/tank-monitor, firmware/node-v1 and firmware/wildfire-node-v1, so a new firmware/ project is deliberately not wired into CI by this task."
  sketch: "VERBATIM from the task's Sketch block -- not one character added or reformatted -- committed as src/main.ino. The .ino extension is a declared deviation from a literal paste into a .cpp: as src/main.cpp the sketch does not compile (PlatformIO does not inject <Arduino.h> for .cpp files, so Serial, Serial2, ledcSetup, ledcAttachPin and ledcWrite are all 'not declared in this scope' -- that exact error was produced and is quoted in notes). As .ino, PlatformIO applies the Arduino prelude and it builds unchanged."
  serial: "Captured over the by-id port at 115200 8N1, port held open by one process that pulses EN via RTS (DTR held False so GPIO0 stays high = normal boot) so setup()'s line cannot be lost to the open() race. Three windows: 62 s, 30 s, 62 s. setup_line: 'Motor PWM on, listening...' present in all three. Hex bytes decoded: 96933 (62 s), 45947 (30 s), 94965 (62 s) -- i.e. ~1.5 kB/s. FA frame headers (the LDS-006 packet header): 0, 0, 0."
  histogram: "run 1 (62 s, 96933 bytes): 00 = 76562 (78.98%), 04 = 20168 (20.81%), then 40 x67, 02 x25, 44 x23, 08 x18, 20 x17, 80 x12, 01 x12, 10 x9, 24 x7, 05 x4, 06 x3, 84 x2, 12 x2, 0C x2 -- 16 distinct values. run 2 (30 s, 45947 bytes): 00 = 34503 (75.09%), 04 = 11299 (24.59%), then 40 x42, 44 x35, 08 x11, 20 x10, 02 x10, 80 x9, 05 x8, 10 x6, 01 x5, 24 x4, FF x2, 18 x1, 84 x1, 30 x1 -- 16 distinct. run 3 (62 s, 94965 bytes): 00 = 74098 (78.03%), 04 = 20501 (21.59%), then 40 x109, 44 x58, 20 x35, 80 x25, 24 x24, 02 x23, 10 x22, 08 x20, 01 x19, 06 x8, 0C x7, 05 x5, 14 x5, 84 x3, 48 x2, 12 x1 -- 18 distinct. Across all three runs the byte rate is 1532-1563 B/s, 74-79% of it is 0x00, 21-25% is 0x04, and everything else is a single-bit value (01/02/08/10/20/40/80) or a two-bit one (44/24/84/0C/06/14/12/30/18/48), each in single or double digits. Nothing in the 0x0A-0x3F band, no ASCII text, and no FA."
  observations: "MOTOR: did not spin -- Stephen at the bench confirms the turret stayed still for the whole run, and that the bench wiring was NOT changed between any of the three runs. DATA: no LDS-006 frames, and no valid UART byte stream at all -- see notes item 3 for why the 00/04 flood is not data. Neither leg of the task's fork (motor spins / data arrives) came true."
  sample_hex: "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 04 00 00 00 04 04 00 04 04 00 00 00 04 00 00 00 04 00 00 00 00 00 00 00 00 (first 40 of 94965 tokens)"
notes: |
  STAGED (iteration 1). Model tier: DeepSeek Flash (this session). Bench work done
  interactively at Stephen's request and at his word on the two observations only
  he can make; the four cron worker jobs were already paused (since the 0119 bench
  block) and remain paused, so no tick could resume this task mid-flight. The
  headline: the board and sketch are proven good, and the LiDAR gave nothing --
  no spin, no frames, and not even a plausible UART byte stream on the wire.

  1. WHAT WAS SET UP (success criterion 1 MET). Toolchain, recorded in
     firmware/lidar-lds006-bringup/platformio.ini so it is reproducible: PlatformIO
     Core 6.1.18 + espressif32 @ 6.12.0, arduino-esp32 core 2.0.17, board
     nodemcu-32s. The port is pinned by-id (CP2102 bridge -> ttyUSB0) rather than by
     name, because this host has three serial devices attached and two of them
     matter (the S3 node is ttyACM0, the base ttyACM2). Silicon read back as
     ESP32-D0WD-V3 rev v3.1, MAC 00:70:07:e6:42:70.

  2. FLASHED AND OBSERVED (criterion 2 MET). Three identical flash+observe cycles.
     Every esptool region reported 'Hash of data verified.' and the app is 273824
     bytes at 0x10000. The sketch printed its setup line in all three windows, so
     the board, the port, the monitor and the PWM setup all work. Nothing was
     rewired: the bench wiring is Stephen's, exactly as the Constraint requires,
     and he confirms it was untouched across all three runs.

  3. THE 290 kB OF "DATA" IS NOT DATA (the part worth reading twice). The task asks
     for the first ~10 lines of hex if any appear. Hex does appear -- ~1.5 kB/s of
     it -- but it is not a byte stream from the LiDAR, and calling it "data" would
     send the next run down the wrong path. Evidence: 0 occurrences of 0xFA in
     237845 decoded bytes across three runs (0xFA is the LDS-006 packet header, so
     a working link would show it constantly), and a value distribution of 74-79%
     0x00 / 21-25% 0x04, with the remaining <1% made up of single-bit values
     (01/02/08/10/20/40/80) and two-bit ones (44/24/84/0C/06/14/12/30/18/48), no
     ASCII text at all, and nothing in the 0x0A-0x3F band.
     An idle UART TX line sitting high yields NO bytes at all; a line held LOW into
     an RX pin yields a continuous stream of 0x00 with occasional low-order bits
     (0x04 = exactly one bit high mid-frame) from noise, crosstalk or the line
     being driven at the wrong level. That is the signature observed. INFERENCE,
     labelled as such and not as measurement: GPIO16 is most likely sitting on a
     line that is low/undriven at the ESP32 rather than on a live TX -- consistent
     with green/blue being swapped relative to the LiDAR's actual TX, and with an
     LDS-006 that streams no data while its motor is not turning. The task already
     names the green/blue swap as the next bench step; this evidence supports it
     but does not prove it, and I did not rewire to test it (Constraint: do not).

     A second, smaller inference: because ~79% of the sampled bytes are 0x00, the
     divider tap is sitting near 0 V while the sketch runs, not at the ~2.8 V the
     10k/23k divider would give for the ~4 V Stephen measured on the wire idle.
     That is consistent with the LiDAR holding the line low (or the wire in
     question not being an idle-high TX at all) and is worth one meter reading at
     the bench before the swap.

  4. FORMAT NOTE ON "LINES" OF HEX. The sketch prints each byte with
     printf("%02X ") and no newline, so its output is one continuous hex stream,
     not lines -- there are no line breaks to quote. The first 40 tokens are in
     proof.sample_hex. Nothing was changed in the sketch to fix this (verbatim
     requirement); if Juno wants line-wrapped hex for the next run, that is a
     one-line change to the NEXT task's sketch.

  5. THE TWO FAILURES, STATED PLAINLY, AS THE TASK ASKS. (a) The turret did NOT
     spin, at any point, in any of the three runs, with the sketch driving 5 kHz
     PWM at duty 128 on GPIO17 (blue) for the whole window. (b) No data appeared on
     GPIO16 (green via the 10k/23k divider) beyond the framing garbage in item 3.
     The task's fork assumed one of the two wires would work; neither did, which is
     itself the finding: the wire pair is not doing TX/PWM-in in the order assumed,
     and the swap is the cheapest next test.

  6. CONSTRAINTS HONOURED. No rewiring (bench confirmed unchanged). Sketch verbatim
     (the single deviation is the .ino extension, declared in proof.sketch and
     explained: as a .cpp it does not compile for want of the Arduino prelude, and
     the fix was the file name, not the sketch text). Nothing on the ESP32-S3 node
     or the base station was touched by this task -- their ports were simply left
     alone, and the by-id pinning exists to keep it that way. No configuration,
     no NVS and no network path was involved: this sketch only talks to Serial and
     Serial2.

  7. OUTSTANDING / NOT DONE. (a) Motor spin and any wiring change remain bench
     actions for Stephen; nothing here was guessed. (b) The bring-up project is on
     a branch and deliberately not in CI (stated under proof.flash) -- say the word
     if it should be, and that is a workflow change, not a code change. (c) The
     node's own 0119 bench half is still outstanding and unrelated to this task.

    - KNOCK-ON FROM A BENCH FAULT (recorded 2026-10-07, later the same day): the 1.6k
      series resistor in the wire going to P17 (TX) was found DISCONNECTED and was
      reconnected by Stephen, so this task's 5 kHz PWM on blue never reached the LiDAR --
      "the PWM did not spin the motor" is a true bench observation but not evidence about
      the unit. (This sketch never drove that wire anyway: it passed -1 as the Serial2 TX
      argument, which 0122 already reported.) Full write-up and the re-run with the wire
      intact are in the staged 0124 addendum.
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
