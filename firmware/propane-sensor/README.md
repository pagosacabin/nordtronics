# Propane bench firmware (task 0208)

Indoor propane monitor, v1 bench rung. One ESPHome config for an ESP32-C3 Super Mini
(TENSTAR) reading two MOS propane sensors through a shared ADS1115, so the
MQ-6 vs Figaro TGS2610-D00 shootout can run on live data.

| | |
|---|---|
| Config | `propane-bench.yaml` (`esphome config` / `esphome compile` clean on ESPHome 2026.9.1) |
| Board | ESP32-C3 Super Mini, MAC `9c:cc:01:d0:78:80`, ESP32-C3 (QFN32) rev v0.4 |
| Console | USB Serial/JTAG (`logger.hardware_uart: USB_SERIAL_JTAG`) — the Super Mini's USB-C is wired to the chip's USB peripheral, not UART0, so this is required for logs |
| Evidence | `bench-log-2026-11-17-serial.txt` (bus probe, 100 s of both channels, alarm trip + release) |

## Division of labour

Alarm logic runs **on the device**; WiFi/Home Assistant are reporting only. The alarm
script reads the two ADS1115 channels directly, so it trips and clears with no network,
no HA automation and no cloud dependency. This rung reports to HA via the native API;
the nordtronics backend/MQTT is a later rung.

## Wiring

As built and meter-verified by Stephen, 2026-11-17:

| Signal | Pin |
|---|---|
| I2C SDA | **GPIO7** (see the correction below) |
| I2C SCL | **GPIO6** (see the correction below) |
| Buzzer | GPIO10 (plain switch output; type still unconfirmed — active vs passive) |
| Alert LED | GPIO5, active HIGH, 470 Ω series |
| Untouched | GPIO8 (onboard LED), GPIO9 (BOOT) — strapping pins; GPIO20/21 stay free |

ADS1115 at `0x48` (ADDR → GND), **VDD = 5 V**, ALRT unconnected. Because VDD is 5 V there
are **no input dividers**, so A0/A1 see the sensors' true VRL and the config uses the
±6.144 V range so a heater transient cannot clip the reading.

- A0 = MQ-6 VRL — one sense pair → +5 V, other pair → RL 10 kΩ → GND, output at that junction
- A1 = Figaro TGS2610-D00 VRL — pin 3 → +5 V, pin 2 → RL 10 kΩ → GND, output at that junction (pins 1 & 4 are the heater)

### Correction: the I2C pair is transposed relative to the task brief

The brief specified SDA = GPIO6 / SCL = GPIO7. On the rig as built that pair finds
**no device at all**: with `sda: GPIO6, scl: GPIO7` the bus scan reports
`Found no devices` and the ADS1115 is marked FAILED (`Communication failed`); with the
pair transposed (`sda: GPIO7, scl: GPIO6`) the same board, same power, logs
`Found device at address 0x48` and both channels read. Both probe builds are quoted in
`bench-log-2026-11-17-serial.txt` section 1. The shipping config therefore uses
GPIO7/GPIO6 so the sensor actually reads.

If the harness is rewired to match the brief instead, swap those two lines back — the
firmware follows the wire, and the wire currently answers on 7/6.

## Secrets

All credentials are `!secret` references; no values are in this repo. `secrets.yaml` must provide:

| Key | Note |
|---|---|
| `wifi_ssid` | |
| `wifi_password` | |
| `api_encryption_key` | 32-byte base64. Generate with `openssl rand -base64 32`. The bench key generated for this task lives only in the build machine's scratch `secrets.yaml` (mode 600) — it is not committed and not echoed in the task reply. |

The only `secrets.yaml` on the build laptop was a **template with placeholder WiFi values**
(`YOUR_WIFI`), so the flashed unit has no network credentials and runs its WiFi retry loop
forever. That is expected here and does not affect the alarm: everything the alarm needs is
on the device.

## Entities

| Entity | Type | Meaning |
|---|---|---|
| MQ-6 VRL | sensor, V | ADS1115 A0, 1 s updates |
| Figaro VRL | sensor, V | ADS1115 A1, 1 s updates |
| Alarm threshold | number, 0.1–5.0 V, step 0.05, start 1.0 V | trip level; `restore_value` keeps a tuned value across reboots |
| Gas alarm | binary_sensor (gas) | latched alarm state |
| Buzzer / Alert LED | switches (GPIO10 / GPIO5) | driven only by the alarm script |

Hysteresis is 0.1 V (`substitutions.hysteresis`): trip when **either** channel rises above
the threshold, clear only when **both** channels fall below `threshold − 0.1 V`. A channel
that is `nan` (sensor unplugged) is never treated as clean air — a trip can happen on one
valid channel, a release requires both.

## Bench observation: the 1.0 V starting threshold is below this rig's clean-air baseline

Measured in still air after warm-up: **MQ-6 ≈ 1.53 V, Figaro ≈ 1.68 V** (both drifting
slowly downward as the heaters condition). The brief's 1.0 V starting threshold is below
both, so a freshly flashed unit alarms immediately — which is what the delivered image does,
and it is the spec'd value rather than a defect. Raise the threshold from Home Assistant
(or set `initial_value`) once the parts have conditioned; a first guess for this rig is
~2.0 V, and the shootout data is what should set the final number.

Note that `restore_value: true` means the stored value wins over `initial_value` after the
first change: the unit on the bench currently holds 2.5 V in NVS from the alarm-transition
probe, which is why it reads `alarm=off` at 1.5 V.

## Flashing and logs

```bash
esphome config  propane-bench.yaml
esphome compile propane-bench.yaml
esphome run     propane-bench.yaml --device /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_9C:CC:01:D0:78:80-if00
```

`esphome upload` alone does **not** rebuild: it re-flashes the last compiled binary. Use
`compile` + `upload`, or `run`, after any YAML edit.

Two bench traps seen on this board: opening the serial port with `pyserial`/`stty` drives
DTR/RTS and puts the C3's USB Serial/JTAG into ROM download mode (`waiting for download`) —
use `esphome logs`, which attaches without resetting. And after an esptool reset the port
re-enumerates (`ttyACM0` → `ttyACM1`), so address it by `/dev/serial/by-id/...`.
