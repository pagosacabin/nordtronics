# Wildfire Node v1 — unified firmware (node + base station)

Task 0094. **One binary, two roles.** The board decides at boot whether it is a
node or a base station and branches all behaviour on that; there is no
role-specific image to flash onto the wrong board.

The base station is another Heltec LoRa 32 V4 (USB-powered, no sensors): it
receives LoRa, runs the v0.2 consensus engine, publishes to MQTT and shows status
on the OLED. The node samples BME680 + PMS5003, checks in every 12 minutes and
deep-sleeps in between.

Protocol: `docs/wildfire/radio-protocol-v1.md` (`protocol_version: 1`).

## Role detection (first thing that runs, before any radio/network init)

Probed, not jumpered (task 0104 item 3): the board asks what is actually fitted.
`resolve_role()` in `src/role_detect.h` applies, in order:

| source | when | serial line |
|---|---|---|
| `NVS` | a `role` value is set in NVS / the captive portal | `ROLE: base (source=NVS, sensors=present, nvs_role=1)` |
| `PROBE` | a BME680 (0x77) / BME688 (0x76) ACKs on I2C, or a valid PMS5003 frame arrives on the UART | `ROLE: node (source=PROBE, sensors=present, nvs_role=unset)` |
| `DEFAULT` | nothing answered the probe — a bare board | `ROLE: base (source=DEFAULT, sensors=absent, nvs_role=unset)` |

The 0094 GPIO7 solder jumper is gone, so **no hardware change is needed on a Rev C
board** and the portal `role` field (`blank = auto-detect`) is the only override.

**Probe pins are frozen nets.** The probe drives GPIO17/18 (BME680/688 I2C) and
GPIO5/6 (PMS5003 UART), all of which are on the 0079 Rev C frozen net list
(`hardware/wildfire-node-v1`, branch
`hermes/0079-wildfire-node-rev-c-interface-fixes` @ `62e64ad`): pins 1–14 are
`BAT, GND, SOLAR, 3V3, GPIO36, GPIO17, GPIO18, GPIO4, GPIO5, GPIO6, GPIO33,
GPIO47, GPIO48, GPIO34`, and 15–42 carry no-connects. `test_role_and_portal`
asserts mechanically that every probe pin is on that list and is not one of the
ESP32-S3 special functions — GPIO0/3/45/46 (boot select), GPIO19/20 (native USB),
GPIO43/44 (UART0 log), GPIO8–14 (SX1262 SPI) — so the claim cannot rot.

**HARD RULE: the base never sleeps.** `enter_deep_sleep()` is the only sleep path;
it asserts `role == node`, logs `FATAL: deep sleep requested while role=base …`
and returns instead of sleeping.

## Node power-state machine (release 0120, task 0125)

One cycle per check-in period, in this order, every state logged as
`power: <STATE>` on the serial console and shown on the panel's `seq:` row:

| state | what happens |
|---|---|
| `WAKE` | wake cause and the RTC cycle / sequence counters are logged |
| `SENSOR_PWR` | Vext asserted, PMS boost EN (GPIO16) HIGH, PMS UART attached, then the full Plantower warm-up |
| `SAMPLE` | one PMS5003 frame + one BME680 reading + the battery-divider sample |
| `SENSOR_OFF` | the UART is detached and the MCU TX pin floated, **then** the boost EN goes LOW |
| `LORA_TX` | the frame is encoded and transmitted; a failed transmit re-inits the radio once and retries |
| `ALARM_ACK` | node-fast-alarm only: the ACK window and its retries |
| `SLEEP` | the field build deep-sleeps here; the bench build logs the gate and idles (`IDLE`) |

**The 5 V sensor rail is switched.** The PMS5003 hangs off the MiniBoost, gated by
`kPmsBoostEnablePin`. It is the dominant load in the power budget (~97 % of the
daily energy, `docs/wildfire/node-power-budget-v1.md`), so it is powered for the
sensor window only. Two consequences are handled in code:

- **No UART back-power.** `SENSOR_OFF` detaches the UART and floats the MCU's TX
  pin *before* the boost drops, so nothing on those lines can source current into
  the unpowered module.
- **Held LOW through sleep.** The MiniBoost carries a 100 kΩ EN→VIN pull-up, so a
  merely released enable line turns the 5 V rail **ON**. `enter_deep_sleep()` drives
  it LOW and latches it with `rtc_gpio_hold_en()`; `setup()` releases the hold on the
  next wake. That latching requirement is why the pin has to be RTC-capable.

**Warm-up.** A power-cycled PMS5003 needs ≥ 30 s before its readings mean anything
(`kPmsWarmupMs`; Plantower manual V2.3 and the Rev C schematic note "allow >= 30 s
warm-up"). `SENSOR_PWR` serves that warm-up in 1 s slices so the 0117 live poll and
the 0118 OLED render keep running through it — the panel is never frozen for 30 s.

**RTC-persistent state (item 4).** The wire sequence counter, the LAYER 1 alarm
latch, the alarm-episode count and the cycle counter are all `RTC_DATA_ATTR`, so
they survive deep sleep: a node that alarmed and slept wakes still reporting the
alarm, and a failed frame never fabricates a clear.

**Provisioning is a deliberate gesture (item 6).** Field mode sheds WiFi/AP/portal
on a node entirely — the node talks LoRa only, so an AP and a portal are pure idle
load. Hold BOOT (GPIO0) through the first 3 s of boot with the panel counting down
(`kProvisionHoldMs`) and the board comes up in provisioning mode: WiFi + captive
portal. Release early and it boots straight into field mode. A **base ignores the
gesture** — it is the uplink and always carries the network.

Press BOOT once the board has already started: GPIO0 held **through** power-on or a
reset selects the ESP32-S3 ROM's download boot, not the application. The gesture is
therefore read ~2 s into `setup()` (after the panel is up, so it can draw), which is
after the bootloader has sampled the strap.

## Alarm layers (task 0125 item 5)

Two independent layers exist, and each code path belongs to exactly one of them:

| layer | where it lives | rule |
|---|---|---|
| **LAYER 1 — node-fast-alarm** | node firmware only, `node_sample()` in `src/main.cpp` | PM2.5 ≥ `kNodeFastAlarmPm25` (55 µg/m³) on the reading in hand. No consensus and no other node: a single node raises it. Sends an ACK-required ALARM frame. |
| **LAYER 2 — base-consensus** | base firmware only, `src/consensus_v02.cpp` | ≥ 2 nodes rising inside one correlation window (`ConsensusConfig::correlation_window_min`). A node never evaluates it, and the base never evaluates LAYER 1. |

## Fixed wiring (Rev C — do not flip)

| signal | pin | note |
|---|---|---|
| I2C SDA | GPIO17 | BME680 SCK/SDA, bench-verified 2026-10-02; SSD1306 OLED 0x3C shares the bus |
| I2C SCL | GPIO18 | BME680 SDI/SCL, bench-verified 2026-10-02 |
| PMS5003 TX → MCU | GPIO5 | through the 1 kΩ series resistor, bench-verified |
| SX1262 NSS / DIO1 / RST / BUSY | 8 / 14 / 12 / 13 | Heltec V4.2 module wiring |
| PMS 5 V boost EN (MiniBoost) | GPIO16 | release 0120 bench wiring (`kPmsBoostEnablePin`); the Rev C interface board wires this to **GPIO4** — open item, see below |
| BOOT / provisioning button | GPIO0 | the Heltec's own BOOT switch; held 3 s at boot on a node = provisioning mode |

The production sensor is the BME688 and the bench part is the BME680; the base
does not care which one a node carries, so nothing branches on it.

## Build and test

```bash
pio run  -d firmware/wildfire-node-v1 -e heltec_v4   # the firmware
pio test -d firmware/wildfire-node-v1 -e native      # host-side tests (no hardware)
```

The `native` env compiles `src/consensus_v02.cpp`, `src/radio_protocol.cpp` and
`src/role_detect.cpp` — the same translation units the ESP32 firmware builds —
together with the unity tests, so the host tests exercise the code the device runs.
CI runs both (`platformio.yml`, jobs `build` and `host-tests`).

| test | what it proves |
|---|---|
| `test_consensus_native` | the v0.2 engine against the 0091 scenario set, plus hand-built cooldown/ack cases |
| `test_radio_protocol` | the byte layout in the doc: offsets, frame sizes, golden hex, CRC rejection |
| `test_role_and_portal` | the probe/NVS/default truth table, the exact serial lines, the boot-probe pin check against the 0079 frozen nets, the power-state pins (boost EN / BOOT / warm-up / LAYER 1 threshold), the portal field table |

`scenarios_v02.h` is generated — regenerate with
`python3 python/detection-sim/emit_v02_scenarios.py` after changing `traces.py`,
and commit the result. The tests never read files at runtime.

## Files

| file | role |
|---|---|
| `src/main.cpp` | Arduino entry: role detection, node/base loops, radio, MQTT, portal, OLED |
| `src/consensus_v02.h/.cpp` | v0.2 engine (portable, no Arduino) |
| `src/radio_protocol.h/.cpp` | wire format (portable) |
| `src/role_detect.h/.cpp` | role decision + serial line (portable) |
| `src/firmware_config.h` | portal field table, pin map, the 0079 frozen-net list |
| `src/mqtt_topic.h` | the telemetry topic builder (the deployed `<root>/<node-id>/telemetry` shape) |
| `src/mqtt_payload.h` | the telemetry/event payload builders + the `<root>/<base-id>/events` topic (the deployed backend contract) |
| `src/device_name.h` | the DHCP hostname builder (`wildfire-<role>-<nn>`) |
| `test/*` | host-side unity tests |

## Known state / bench verification left

- The `heltec_wifi_lora_32_V3` board definition is used for the V4.2 target: the
  silicon, SX1262 US915 radio and Arduino core are the same, and the pin map comes
  from `src/firmware_config.h`, not the board JSON. TCXO voltage is set to 1.8 V
  in `radio.begin(...)`; confirm on the bench.
- `batt_mv` is now measured: the node reads the Heltec V4.2 module's switched
  divider (ADC_Ctrl GPIO37 HIGH → ADC1_CH0 GPIO1 → GPIO37 LOW, task 0119) at every
  check-in and transmits it. TP1 remains the bench cross-check.
- **Open item — MiniBoost EN pin.** Release 0120 drives the boost enable on
  **GPIO16** (task 0125's call, and where Stephen wires the bench line), but the Rev C
  interface board (tasks 0076/0077) wires MiniBoost EN to Heltec **GPIO4** with a
  4.7 kΩ pull-down, and task 0079 lists GPIO16 in the V4.2 pinout as `XTAL_32K_N`.
  The constant is `kPmsBoostEnablePin` (one line) and `test_role_and_portal` pins the
  current value, so either pin is a one-line change once this is decided; do not bench
  the SENSOR_OFF current until it is.
- **Open item — Vext policy.** `SENSOR_OFF` leaves Vext (GPIO36) asserted, because on
  this board family the SSD1306 panel shares that rail (task 0108) and the bench phase
  needs a live panel; only the 5 V boost is switched. If the panel is confirmed to sit
  on 3V3, Vext can be switched per cycle instead (`sensor_power_off()`).
- **Bench item — the RTC hold on the boost enable.** `rtc_gpio_hold_en()` is the
  documented way to latch an output through deep sleep, but whether the MiniBoost's EN
  pull-up is actually held off can only be confirmed with a meter on `5V2` while the
  node is asleep. Deep sleep is gated off in this build, so nothing exercises it yet.
- The 0094 role jumper on GPIO7 is retired (task 0104 item 3): the role is probed
  from the sensors, so GPIO7's header availability is no longer a bench item.
- AS3935 lightning is present as a status bit only; the byte is reserved so the
  driver can be added without a wire-format change.
- `setDio2AsRfSwitch` is not called; Heltec V4.2 RF switch control needs a bench
  confirmation before it can be asserted in code.
