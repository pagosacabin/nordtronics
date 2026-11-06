# c4002-presence — ESP32-C3 Super Mini + DFRobot C4002 (SEN0691)

Bench verification of Stephen's presence detector (mailbox task 0169, 2026-11-06).

## Status

| Step | Result |
|---|---|
| C3 identified on USB | **yes** — `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_9C:CC:01:D0:64:6C-if00` (`ttyACM1`, MAC `9c:cc:01:d0:64:6c`, ESP32-C3, 4 MB flash, flash was blank `0xffffffff`) |
| Firmware compiled + flashed | **yes** — `esphome 2026.9.1`, hash of written data verified, `config_hash=0x7cdd1f8b` |
| C4002 UART up, no UART errors | **yes** — ~1.00 Hz notification frames over two windows (77 frames / 75 s, 36 / 35 s), 0 error/auth/timeout lines |
| Walk test (presence ON / 30 s hold / OFF) | **NOT RUN** — no person at the bench during the run |
| OUT-pin vs UART agreement | **NOT RUN** — depends on the walk test |
| Distance / speed / energy values with a target | **NOT RUN** — depends on the walk test |

So the wiring and the firmware path are proven as far as they can be without a human: the
sensor is powered, and it is talking back on the pins the spec names. Everything downstream of
"a person walks in front of it" is still owed.

## The spec's YAML does not compile — four separate defects

The config in the task is not valid ESPHome. Each of these was observed with `esphome config`:

1. `dfrobot_c4002` is not a bundled component in ESPHome 2026.9.1 —
   `Unable to import component dfrobot_c4002: No module named 'esphome.components.dfrobot_c4002'`.
   It is an **external** component; DFRobot's own wiki sources it from
   `https://github.com/cdjq/esphome.git` ref `dev`.
2. There is no `binary_sensor` platform for it at all in the cdjq repo —
   `Platform not found: 'binary_sensor.dfrobot_c4002'`. (The OwlBawl repo used below does have
   one, with a `presence:` key that matches the task's block verbatim.)
3. The sensor keys `distance` / `speed` / `energy` do not exist. The component's keys are
   `movement_distance`, `existing_distance`, `movement_speed`, `movement_direction`,
   `target_status` (cdjq) plus `movement_energy`, `existing_energy`, `presence_countdown` (OwlBawl).
   `esphome` replies: `[distance] is an invalid option ... Did you mean [movement_distance], [existing_distance]?`
   Also `c4002_id` is required on every platform block.
4. A bare `ota:` is rejected by 2026.9.1:
   `At least one platform must be specified for 'ota'; add 'platform: esphome'`.
   (The spec's YAML never reaches this error because defect 1 fires first.)

## What was flashed, and why it differs

`c4002-presence.yaml` in this directory is the exact flashed config
(sha256 of `firmware.factory.bin`: `256b2d4e61cbab40ea5a9a41c408b98b2ebc1913f847424334efd0d126211b3f`).

Deviations from the task's YAML, each forced:

- **`external_components:` added**, pointing at `OwlBawl/DFRobot_C4002_SEN0691` (`ref: main`,
  `path: .`). Chosen over cdjq's `dev` because only that repo has the `binary_sensor` `presence:`
  platform the task asks for, and because it carries `existing_energy` — the closest thing to the
  task's `energy` sensor. cdjq's `dev` does not build: its `.cpp` references 16 members its `.h`
  does not declare.
- **`ota:` given `- platform: esphome`** (2026.9.1 requirement).
- **Sensor keys renamed** to the component's real schema: `distance` → `existing_distance`,
  `speed` → `movement_speed`, `energy` → `existing_energy`.
- **`number:` / `switch:` / `select:` blocks added.** The component's `.cpp` uses
  `min_range_number_`, `run_led_switch_`, `operating_selector_` … unconditionally while its `.h`
  declares them behind `#ifdef USE_NUMBER` / `USE_SWITCH` / `USE_SELECT`. With no entity of those
  kinds in the config the macros are undefined and the build fails with
  `'min_range_number_' was not declared in this scope` (16 errors). One entity of each kind makes
  it compile. **Any future config for this component needs at least one `number`, one `switch` and
  one `select` block, even if unused.**
- `uart:` gained an explicit `id`, `parity: NONE`, `stop_bits: 1`; `dfrobot_c4002` gained
  `uart_id`. Pins are unchanged from the spec: **TX GPIO7, RX GPIO6, OUT GPIO4**.

Pins, board and framework are the spec's, verbatim.

## Networking note

The device associates and advertises as `c4002-presence.local`
(`mac=9ccc01d0646c`, `config_hash=7cdd1f8b`) but the advertised address `192.168.1.155` did not
answer ARP or port 6053 during the run, so no values could be pulled over the API. Worth checking
before mounting: it may be the 2.4 GHz side of a mesh/AP that does not forward, or client isolation.
`192.168.1.72:6053` on this LAN is the bench panel (`jc4827w543`), not this board — do not confuse them.

## Next step

Power it up with someone at the bench and run the walk test. The serial console is enough:
`/dev/ttyACM1` at 115200, read non-blocking (a `pyserial` open resets the board via DTR/RTS).
