---
task_id: "0169"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  BLOCKED — PARTIAL, NO PROOF BLOCK. Run 2026-11-06 19:15-19:45 UTC (12:15-12:45 MST), PEAK: OFF-PEAK
    19:15 UTC, PROTOCOL MATCH 1.0.0. 0142 (status: superseded) and 0159 (decision-blocked,
    controller address owed) were left untouched and are not resumable; 0169 was the lowest
    actionable task. Front-matter was absent on arrival — added at pickup (protocol rule 16),
    `iteration` set to 1.

    DONE, AND PROVEN ON THE HARDWARE:
     1. The C3 is on USB. `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_9C:CC:01:D0:64:6C-if00`
        (`/dev/ttyACM1`). Identified before flashing, not guessed: `esptool flash_id` answered
        MAC 9c:cc:01:d0:64:6c (matches the by-id serial), ESP32-C3, 4 MB flash, and a `cat` of the
        port showed the ROM banner `ESP-ROM:esp32c3-api1-20210207` with a flash of `0xffffffff`
        (blank). The other Espressif device on the host (44:BD:8D:7A:CC:CC, ttyACM0) is the bench
        panel and was not touched.
     2. Firmware compiles and flashes clean. `esphome 2026.9.1`, "Hash of data verified",
        `config_hash=0x7cdd1f8b`, factory bin sha256
        256b2d4e61cbab40ea5a9a41c408b98b2ebc1913f847424334efd0d126211b3f.
     3. The C4002 UART is up with no UART errors: 77 notification frames in 75 s and 36 in 35 s
        (~1.00 Hz), and zero lines matching auth|error pack|timed out|discarding|WARN in either
        window. That is the electrical proof of VIN/GND and the TX->GPIO6 path: the sensor is
        powered and talking on the pins the spec names.

    WHERE IT STOPPED, EXACTLY: the walk test. Criterion 3 (presence ON -> ~30 s hold -> OFF),
    criterion 4 (OUT pin vs UART agreement) and criterion 5 (live distance/speed/energy with a
    target) all need a person to step in front of the sensor. There was nobody at the bench during
    this run — an unattended cron tick cannot supply one — and the task forbids inventing
    walk-test results. No presence ON, no hold, no OFF, and no target values are claimed.

    THE SPEC'S YAML DOES NOT COMPILE — four separate defects, each reproduced with `esphome config`:
     (a) `dfrobot_c4002` is not bundled: "Unable to import component dfrobot_c4002: No module named
         'esphome.components.dfrobot_c4002'". It is an external component; DFRobot's own wiki
         sources it from https://github.com/cdjq/esphome.git ref dev.
     (b) No `binary_sensor` platform exists in cdjq's repo at all: "Platform not found:
         'binary_sensor.dfrobot_c4002'".
     (c) Sensor keys wrong: "`[distance]` is an invalid option for `[sensor.dfrobot_c4002]`. Did you
         mean `[movement_distance]`, `[existing_distance]`?" — same for `speed` and `energy`; and
         "`c4002_id` is a required option".
     (d) A bare `ota:` is rejected by 2026.9.1: "At least one platform must be specified for 'ota';
         add 'platform: esphome'". Unreached in the spec file only because (a) fires first.

    DEVIATIONS I MADE, ALL DECLARED (per the task's own "quote it if you changed anything"):
     - Added `external_components:` -> OwlBawl/DFRobot_C4002_SEN0691 `ref: main`, `path: .`. Picked
       over cdjq's `dev` because only it has the `presence:` binary_sensor the task asks for, and it
       carries `existing_energy`. cdjq's `dev` does not build at all — 16 errors, its .cpp
       referencing members its .h never declares (`min_range_number_` … `show_gates_energy_switch_`).
     - Added `- platform: esphome` under `ota:`.
     - Renamed sensors to the component's real schema: distance->existing_distance,
       speed->movement_speed, energy->existing_energy.
     - Added one `number:`, one `switch:` and one `select:` block. Not cosmetic: the component's .cpp
       uses those members unguarded while the .h declares them behind #ifdef USE_NUMBER /
       USE_SWITCH / USE_SELECT, so with no such entity the macros are undefined and the build fails
       with "'min_range_number_' was not declared in this scope" (16 errors). Any config for this
       component needs at least one of each, even unused.
     - Added `uart: id`, parity/stop_bits and `uart_id`. PINS, BOARD AND FRAMEWORK ARE THE SPEC'S,
       UNCHANGED: TX GPIO7, RX GPIO6, OUT GPIO4, esp32-c3-devkitm-1, arduino.
     I did NOT change wiring, and I did not touch Stephen's Home Assistant.

    NETWORKING, for whoever mounts it: the board associates and advertises as c4002-presence.local
    (mac=9ccc01d0646c, config_hash=7cdd1f8b) but its advertised address 192.168.1.155 answered
    neither ARP nor port 6053 during the run, so no values could be read over the API either.
    (192.168.1.72:6053 on this LAN is the bench panel jc4827w543, not this board.)

    NO PROOF BLOCK: there is no branch and no CI run for this task — the deliverable is a board —
    and the walk-test criteria are unattained, so staging would be a false claim. The durable
    artifacts are committed on main at bf55732 under firmware/c4002-presence/
    (c4002-presence.yaml = the exact flashed config, bench-log-2026-11-06-serial.txt = both serial
    windows, README.md = status and deviations).

    DECISION / ACTION OWED: someone needs to be at the bench for the walk test. That is the only
    thing standing between this and a staged reply; the flashed firmware is ready for it.
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



---

## Run report — 2026-11-06 (blocked at the walk test)

**Short version:** the C3 is wired and alive, the firmware on it runs, and the C4002 is talking to
it at ~1 Hz with no UART errors. The walk test has not been run, because nobody was at the bench.
There is no proof block, because three of the five success criteria depend on that walk test.

### What was verified

| Step | Evidence |
|---|---|
| C3 on USB, identified before flashing | by-id `..._9C:CC:01:D0:64:6C-if00` -> `ttyACM1`; `esptool flash_id`: MAC `9c:cc:01:d0:64:6c`, ESP32-C3, 4 MB; ROM banner `ESP-ROM:esp32c3-api1-20210207`, flash read `0xffffffff` (blank) |
| Compiles and flashes clean | `esphome 2026.9.1`, `Hash of data verified`, `config_hash=0x7cdd1f8b`, factory bin sha256 `256b2d4e...11b3f` |
| UART component up, no UART errors | 77 notification frames / 75 s and 36 / 35 s (~1.00 Hz); 0 lines matching `auth|error pack|timed out|discarding|WARN` |

### What was not

The walk test: presence ON, the ~30 s hold, presence OFF, the OUT-pin comparison, and the
distance/speed/energy readings while a target is present. All five need a person in front of the
sensor. Nothing about them is claimed here.

### The spec's YAML is not valid ESPHome — four defects

1. `dfrobot_c4002` is not bundled in 2026.9.1: `Unable to import component dfrobot_c4002`.
   DFRobot's own wiki sources it as an external component from `cdjq/esphome` ref `dev`.
2. No `binary_sensor` platform exists in that repo: `Platform not found: 'binary_sensor.dfrobot_c4002'`.
3. `distance` / `speed` / `energy` are not the component's keys: `[distance] is an invalid option …
   Did you mean [movement_distance], [existing_distance]?`; `c4002_id` is required.
4. A bare `ota:` is invalid in 2026.9.1: `At least one platform must be specified for 'ota'; add
   'platform: esphome'`.

### The config that was flashed, and every difference from the spec

Committed at `bf55732` as `firmware/c4002-presence/c4002-presence.yaml`; the rationale is in the
README beside it. In summary: external component switched to `OwlBawl/DFRobot_C4002_SEN0691`
(only fork with the `presence:` binary_sensor the spec asks for, and with an `energy` sensor;
cdjq's `dev` does not compile at all), `ota:` given a platform, sensor keys renamed to the real
schema, and one `number`/`switch`/`select` entity added because the component's `.cpp` uses those
members unguarded while its `.h` hides them behind `#ifdef USE_*`. Board, framework and pins are
the spec's, unchanged: **TX GPIO7, RX GPIO6, OUT GPIO4**.

One thing worth knowing before mounting: the board advertises as `c4002-presence.local` but its
address did not answer ARP or port 6053 during the run, so Home Assistant may not see it on the
network even though the sensor path works.
