# c4002-presence — ESP32-C3 Super Mini + DFRobot C4002 (SEN0691)

Bench verification of Stephen's presence detector (mailbox task 0169, 2026-11-06).

## Status

| Step | Result |
|---|---|
| C3 identified on USB | **yes** — `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_9C:CC:01:D0:64:6C-if00` (`ttyACM1`, MAC `9c:cc:01:d0:64:6c`, ESP32-C3, 4 MB flash, flash was blank `0xffffffff`) |
| Firmware compiled + flashed | **yes** — `esphome 2026.9.1`, hash of written data verified, `config_hash=0x7cdd1f8b` |
| C4002 UART up, no UART errors | **yes** — ~1.00 Hz notification frames over two windows (77 frames / 75 s, 36 / 35 s), 0 error/auth/timeout lines |
| Walk test (presence ON / 30 s hold / OFF) | **yes** — 2026-11-06 14:36–14:38 MST, driven by spoken cues: OFF 33.3 s after "step out", ON 6.0 s after "walk in", OFF 31.1 s after "step away", then held OFF for a further 60 s |
| OUT-pin vs UART agreement | **NO — disagreement, quoted exactly.** `Room Presence OUT pin` (GPIO4) read `True` in all 106 samples taken across four captures spanning 14:32–14:45 MST and never transitioned once, including while `Room Presence` was `False` (14:42:40.784 and 14:44:07.928–14:45:31, 83 s) |
| Distance / speed / energy values with a target | **yes** — live values over a 20 s sample with a person at ~1.4 m: distance 1.37–1.54 m (mean 1.435), speed −0.02…+0.08 m/s, energy 99.0; over the whole session distance spanned 0.68–8.73 m and energy 0–99 |

So the wiring and the firmware path are proven electrically, and the walk test now exists. Two
behaviours are recorded exactly as measured because they contradict the spec's intent: the GPIO4
OUT-pin entity never changes state, and the presence entity emits sub-second `False` blips while a
target is present.

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

## Walk test — 2026-11-06, method and evidence

The board came up on the LAN after the first run, so the walk test was driven over the ESPHome
native API (`:6053`) rather than the serial console: `api-walk-test-controller.py` subscribes to
state and speaks each step as audio on the host's speakers, so the person at the bench needs no
screen. Raw timelines are the four `walk-test-2026-11-06-*.log` files; the summarised result is
`walk-test-2026-11-06-result.json`.

| Step | Cue | Entity event | Latency from cue |
|---|---|---|---|
| A | "step out of the sensor's view" | `Room Presence` True → False at 14:37:27.442 | 33.3 s |
| B | "walk into view, move 20 s" | False → True at 14:37:38.808 | 6.0 s |
| B | 20 s sample with a target at ~1.4 m | distance 1.37–1.54 m (mean 1.435), speed −0.02…+0.08 m/s, energy 99.0 | — |
| C | "step out and stay out" | True → False at 14:38:34.206 | 31.1 s |
| C | stayed out | still False after a further 60 s | — |

The ~30 s hold the spec asks for is therefore real and reproducible (33.3 s and 31.1 s measured
from the spoken cue, which itself includes human reaction time).

### Finding 1 — the GPIO4 OUT-pin entity never changes state

`Room Presence OUT pin` read `True` on every one of 106 samples across four captures
(14:32:33 → 14:45:31 MST) and produced **zero** transitions, while `Room Presence` toggled
True/False repeatedly. Two clean disagreement windows:

- `14:42:40.784` — initial publish: `Room Presence` = `False`, `Room Presence OUT pin` = `True`,
  with distance `0.0`, energy `0.0` (no target).
- `14:44:07.928` → `14:45:31.181` (83 s) — `Room Presence` = `False` the whole time;
  `out_pin` = `True` the whole time.

Not diagnosed. Two candidates, in order of likely cheapness to check: the pin is configured
`mode: INPUT` with **no pull** (`c4002-presence.yaml`), so a non-driving OUT floats and reads
high; or the sensor's OUT genuinely does not switch in `OUT Mode = Motion OR Presence`. A meter
on OUT-to-GND during presence ON/OFF settles it in one minute, and `mode: INPUT_PULLDOWN` (or
`INPUT_PULLUP`, depending on the module's polarity) is the config-side test. **Until this is
resolved the OUT pin is not usable as a second presence signal**, which is the whole point of
wiring it.

### Finding 2 — sub-second `False` blips on `Room Presence`

Three momentary drops while a target was demonstrably present (distance and energy non-zero):

| When | Duration |
|---|---|
| 14:36:54.232 → .243 | 11 ms |
| 14:40:57.054 → .668 | 614 ms |
| 14:43:30.135 → .584 | 449 ms |

The entity is configured `filters: - delayed_off: 30s`, which should make any OFF shorter than
30 s impossible to observe. Either the filter is not being applied to this external component's
`binary_sensor`, or something republishes state around it. Not resolved here; it is worth a task
of its own because an 0.5 s presence dropout will trigger automations that a 30 s hold was
supposed to prevent.

### Also observed

- Presence clear latency after the target is lost was not uniformly 30 s: one episode cleared
  18 s after the last target frame (last seen 14:43:49, `False` at 14:44:07), another was still
  `True` 30 s after distance had gone to 8.73 m with energy 0 and cleared somewhere in the
  following minute. Quoted rather than averaged.
- Energy sat at 99.0 almost always, dipping to 74.0 and 73.0 twice; distance read `0.6800000071525574`
  repeatedly, which looks like the component's minimum-report clamp rather than a real target at 68 cm.

## Networking note (superseded)

The first run recorded that `192.168.1.155` answered neither ARP nor `:6053`. That was transient:
from 14:28 MST the address answered both (`ip neigh` → `9c:cc:01:d0:64:6c` REACHABLE; API
connected in 0.38–0.65 s), and every run above is API-based. `192.168.1.72:6053` is still the
bench panel (`jc4827w543`) — do not confuse the two.

`secrets.yaml` is **not** in this directory, so `esphome logs <this yaml>` fails with
`Error reading file .../secrets.yaml`. Recreate it with `wifi_ssid` / `wifi_password` before
re-flashing; the API path above needs no secrets.

## Next step

The board is on the network, so nothing here needs the serial console. Owed before mounting:
resolve Finding 1 (OUT pin) and decide whether Finding 2 needs fixing in the component fork.

## Multi-target: the part has two channels, not N (probed 2026-11-06)

The component's structured output is one **moving** target and one **stationary** target
(`moveTargetDist/Speed/Energy/Direction`, `existTargetDist/Energy`) plus a `targetStatus` of
NO_BODY / EXIST / MOVE. There is no list of bodies, so two people and a dog cannot be resolved
into three contacts by this sensor: at best it is one moving + one still, and a dog is usually
below the energy threshold.

The gate occupancy mask was the obvious candidate for real multi-target data, so it was enabled
and measured rather than assumed. `switch_show_gates_energy` on and an `active_gates_summary`
text sensor added — both now in the config in this directory, flashed as
`config_hash=0xd8e93f96`. Sampled straight from the board's API every 2 s for 60 s while the
reported distance swung 0.68 → 2.93 m and presence dropped and returned:

```
-1 2 3 4 5 6--------(1.0m - 5.0m)    30 / 30 samples identical
```

A second probe bypassed that switch entirely: two template sensors (`Gate mask lo/hi`) read the
component's public `get_exist_dist_index()` straight and split it into 16-bit halves (a template
sensor is a float32, and a 25-gate mask exceeds float's exact-integer range). Flashed as
`config_hash=0x00458931`, sampled every 2 s for 40 s: the mask held at `0x00000e` (gates 1-3, one
contiguous run) through 20 samples while the reported distance swung 0.68 -> 2.14 m. Between runs
it settles on a different constant (`0x7e` earlier), so it is slowly-varying, but it never
follows the targets. It does not track occupancy — it behaves like a configuration bitmask of enabled gates rather
than a detection result. The parser does rewrite `existDistIndex` from every notification frame
(`data[3..6]`), so the field refreshes; what it carries is just not a per-body map. Conclusion:
**this part cannot feed a three-contact display.** A tracking radar that can (HLK-LD2450 reports
up to 3 targets with X/Y and speed) would be the upgrade, keeping this board as the
presence/occupancy trigger.

## The device, as of the 2026-11-06 clean-up

Six entities, three of them knobs. Flashed as `config_hash=0x84ce06ca`; HA re-read the device and
pruned the old set on its own.

| entity | kind | why |
|---|---|---|
| `binary_sensor.c4002_presence_presence` | signal, `device_class: presence` | the one reliable output; `delayed_off: 30s` is the hold automations should use |
| `sensor.c4002_presence_range` | signal, m | single-target range, named for what it is |
| `sensor.c4002_presence_detected` | diagnostic, 0/1/2 | no body / stationary / moving, as one number instead of three columns |
| `number.c4002_presence_disappear_delay` | config | sensor-side hold |
| `select.c4002_presence_out_mode` | config | OUT-pin behaviour |
| `switch.c4002_presence_run_led` | config | LED |

Dropped on purpose: `movement_speed`, `existing_energy`, the GPIO OUT-pin binary sensor (stuck
high, cause still open), and every gate-mask entity. Each of those either implied multi-target
tracking the part cannot do or pointed at a pin that never changed state. The page this feeds on
the bench panel now says `SINGLE TARGET RANGE / 2+ BODIES = PRESENCE ONLY` rather than dressing a
presence detector up as a tracker.
