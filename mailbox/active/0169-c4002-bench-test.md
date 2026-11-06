---
task_id: "0169"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  Picked up 2026-11-06 19:15 UTC (12:15 MST), PEAK: OFF-PEAK, PROTOCOL MATCH 1.0.0.
  Front-matter was absent on arrival (protocol rule 16); added here.
---

# 0169 — Bench-test the ESP32-C3 Super Mini + DFRobot C4002 presence detector

## Context
Stephen's bench project: a simple Home Assistant presence detector from a DFRobot Fermion C4002 (SEN0691) 24 GHz mmWave sensor driven by an ESP32-C3 Super Mini, via ESPHome. Both boards are in hand on the bench. The wiring is five wires, already decided (Stephen wires it, you verify it electrically):

- C4002 VIN → C3 5V pin (sensor wants 3.6–5 V, ~90 mA — NEVER 3V3)
- C4002 GND → C3 G (GND)
- C4002 OUT → C3 GPIO4
- C4002 TX → C3 GPIO6
- C4002 RX → C3 GPIO7

UART runs at 115200. The C3's silkscreen numbers ARE the GPIO numbers (pin marked 4 = GPIO4, etc.). Purpose: prove the wiring and the ESPHome config on the bench before Stephen mounts it in a printed case.

## Task
1. Confirm the C3 Super Mini is visible on your host's USB (by-id path). If no C3 appears, STOP and report blocked — do not guess at ports or flash anything else.
2. Flash this exact ESPHome config to the C3 with your ESPHome tooling (esphome CLI preferred). If you have no working ESPHome route, STOP and report that instead of improvising different firmware.

```yaml
esphome:
  name: c4002-presence
  friendly_name: C4002 Presence

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: arduino

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

api:
ota:
logger:

uart:
  tx_pin: GPIO7
  rx_pin: GPIO6
  baud_rate: 115200

dfrobot_c4002:

binary_sensor:
  - platform: dfrobot_c4002
    presence:
      name: "Room Presence"
      device_class: presence
      filters:
        - delayed_off: 30s
  - platform: gpio
    pin:
      number: GPIO4
      mode: INPUT
    name: "Room Presence OUT pin"
    device_class: presence

sensor:
  - platform: dfrobot_c4002
    distance:
      name: "Target Distance"
    speed:
      name: "Target Speed"
    energy:
      name: "Target Energy"
```

3. Watch the ESPHome log. Run the walk test: have a person step in front of the sensor, then step away. Capture the log lines for presence ON, the ~30 s hold after leaving, and presence OFF.
4. Compare the two presence entities during the walk test: the UART `Room Presence` and the GPIO4 `Room Presence OUT pin`. Note any disagreement.
5. While a target is present, record what the distance / speed / energy sensors report (values, not just "they exist").

## Success criteria
- Firmware compiles and flashes clean to the attached C3.
- ESPHome log shows the C4002 UART component up (no UART errors).
- Walk test: UART presence turns ON with a person in front, holds ~30 s after they leave, then clears.
- OUT-pin entity agrees with UART presence through the walk test (or the disagreement is quoted exactly).
- Distance / speed / energy sensors report live values while a target is present.

## Constraints
- Default model tier (keep the spend ordinary).
- C4002 VIN on the 5V pin only — never 3V3.
- Identify the C3 by its USB by-id path before flashing. Do not flash any other board.
- Read-only toward Stephen's Home Assistant config — no changes there; this task is bench verification only.
- If the walk test can't run (no person available at the bench, sensor not responding), report exactly where it stopped — do not invent walk-test results.

## Proof
- Staged reply quoting the ESPHome log lines for the walk test (presence ON / hold / OFF with timestamps) and the live distance/speed/energy values.
- The exact YAML flashed (quote it if you changed anything from the spec above, with the reason).

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

