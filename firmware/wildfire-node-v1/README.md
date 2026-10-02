# Wildfire Node v1 — unified firmware (node + base station)

Task 0094. **One binary, two roles.** The board decides at boot whether it is a
node or a base station and branches all behaviour on that; there is no
role-specific image to flash onto the wrong board.

The base station is another Heltec LoRa 32 V4 (USB-powered, no sensors): it
receives LoRa, runs the v0.2 consensus engine, publishes to MQTT and shows status
on the OLED. The node samples BME680 + PMS5003, checks in every 12 minutes and
deep-sleeps in between.

Protocol: `docs/wildfire/radio-protocol-v1.md` (`protocol_version: 1`).

## Role detection (first thing that runs, before any radio/sensor/power init)

| source | when | serial line |
|---|---|---|
| `NVS` | a `role` value is set in NVS / the captive portal | `ROLE: base (source=NVS, strap_pin=GPIO7 level=1, nvs_role=1)` |
| `STRAP` | GPIO7 pulled LOW (jumper to GND) | `ROLE: base (source=STRAP, strap_pin=GPIO7 level=0, nvs_role=unset)` |
| `DEFAULT` | nothing set — every unstrapped bench board | `ROLE: node (source=DEFAULT, strap_pin=GPIO7 level=1, nvs_role=unset)` |

**Strap pin = GPIO7.** It is not one of the 14 frozen nets of the 0079 Rev C
capture (`hardware/wildfire-node-v1`, branch
`hermes/0079-wildfire-node-rev-c-interface-fixes` @ `62e64ad`): pins 1–14 are
`BAT, GND, SOLAR, 3V3, GPIO36, GPIO17, GPIO18, GPIO4, GPIO5, GPIO6, GPIO33,
GPIO47, GPIO48, GPIO34`, and 15–42 carry no-connects. GPIO7 is also clear of the
ESP32-S3 functions that make a bad input strap — GPIO0/3/45/46 (boot strapping),
GPIO19/20 (native USB), GPIO43/44 (UART0 log), GPIO8–14 (SX1262 SPI) and
GPIO17/18 (I2C). `test_role_and_portal` asserts all of that mechanically, so the
claim cannot rot. No PCB change is needed for bench work: the NVS/portal override
covers an unstrapped board, and Rev C gets a solder jumper.

**HARD RULE: the base never sleeps.** `enter_deep_sleep()` is the only sleep path;
it asserts `role == node`, logs `FATAL: deep sleep requested while role=base …`
and returns instead of sleeping.

## Fixed wiring (Rev C — do not flip)

| signal | pin | note |
|---|---|---|
| I2C SDA | GPIO17 | BME680 SCK/SDA, bench-verified 2026-10-02; SSD1306 OLED 0x3C shares the bus |
| I2C SCL | GPIO18 | BME680 SDI/SCL, bench-verified 2026-10-02 |
| PMS5003 TX → MCU | GPIO5 | through the 1 kΩ series resistor, bench-verified |
| SX1262 NSS / DIO1 / RST / BUSY | 8 / 14 / 12 / 13 | Heltec V4.2 module wiring |

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
| `test_role_and_portal` | the strap/NVS/default truth table, the exact serial lines, the strap-pin collision check, the portal field table |

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
| `test/*` | host-side unity tests |

## Known state / bench verification left

- The `heltec_wifi_lora_32_V3` board definition is used for the V4.2 target: the
  silicon, SX1262 US915 radio and Arduino core are the same, and the pin map comes
  from `src/firmware_config.h`, not the board JSON. TCXO voltage is set to 1.8 V
  in `radio.begin(...)`; confirm on the bench.
- `batt_mv` is transmitted as 0: Rev C has no fuel gauge, the battery is a bench
  measurement at TP1. The low-battery flag therefore never sets yet.
- GPIO7's availability on the V4.2 header is unverified — carry it as a bench item
  alongside the existing 0079 open items (GPIO47/GPIO48 header reach).
- AS3935 lightning is present as a status bit only; the byte is reserved so the
  driver can be added without a wire-format change.
- `setDio2AsRfSwitch` is not called; Heltec V4.2 RF switch control needs a bench
  confirmation before it can be asserted in code.
