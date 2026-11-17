---
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-17T23:45:20+00:00"
lease_until: "2026-11-18T01:15:20+00:00"
---
# 0208 — Propane bench firmware (ESPHome, ESP32-C3 Super Mini)

## Assignment
- **Hermes:** author, validate, compile — and flash if the board is enumerated on the laptop. This is bench firmware, so it stays with you (not Newton).
- **Copilot:** peer review of the repo-bound deliverable, routed by Hermes via Herdr per standing rule.
- **Juno:** independent verification on the staged reply.

## Context
Nordtronics indoor propane monitor, v1 bench rung. Architecture set with Stephen Nov 17: alarm logic lives on the device; WiFi is reporting only (Home Assistant this rung; the nordtronics backend/MQTT is a later rung). A wall-powered ESP32-C3 Super Mini reads two MOS propane sensors through an ADS1115 so the MQ-6 vs Figaro TGS2610-D00 shootout can run on live data.

Stephen wired the rig on his bench Nov 17 to the agreed pinout and meter-verified it: MQ-6 pin pairs confirmed by resistance (heater ~30 Ω), Figaro heater measured 53 Ω (datasheet ~59 Ω approx — in range; 5 V / 53 Ω ≈ 94 mA cold inrush vs the datasheet's 100 mA max). The board is being plugged into the laptop by USB now.

## Hardware as wired (do not re-derive)
- ESP32-C3 Super Mini (TENSTAR). I2C: SDA = GPIO6, SCL = GPIO7.
- ADS1115 at 0x48: VDD = 5 V, ADDR → GND, ALRT unconnected. **Powered at 5 V, so there are NO input dividers** — A0/A1 see true VRL. Use the ±6.144 V full-scale range.
- A0 = MQ-6 VRL (one sense pair → +5 V; other sense pair → RL 10 kΩ → GND; output at that junction).
- A1 = Figaro TGS2610-D00 VRL (pin 3 → +5 V; pin 2 → RL 10 kΩ → GND; output at that junction; pins 1 & 4 are the heater).
- Buzzer on GPIO10 (type unconfirmed — active or passive; a plain switch output is fine for this rung, tone support welcome if it turns out passive). Alert LED on GPIO5, active HIGH, 470 Ω in series.
- GPIO8/GPIO9 untouched (strapping pins; GPIO8 is the onboard LED, GPIO9 is BOOT). GPIO20/21 stay console.

## Task
Write one ESPHome configuration for this rig and land it in the nordtronics repo at `firmware/propane-sensor/propane-bench.yaml` (if the repo already has a firmware layout convention, follow it and name the path you used in the reply).

The config must:
1. Bring up WiFi and the ESPHome native API for Home Assistant. All credentials via `!secret` references — never commit credential values. If no secrets file is available to you, say so plainly in the reply and leave the `!secret` refs in place; do not invent values.
2. Configure the ADS1115 hub at 0x48 on GPIO6/GPIO7 and two voltage sensors: A0 named for the MQ-6, A1 named for the Figaro, update interval ~1 s, values in volts.
3. Expose a tunable alarm threshold as a number entity (start at 1.0 V, step 0.05 V) with ~0.1 V of hysteresis.
4. On-device automation: when either channel rises above the threshold, turn the buzzer (GPIO10) and the alert LED (GPIO5) ON; when both channels fall below threshold minus hysteresis, turn them OFF. The alarm must not depend on WiFi or Home Assistant being up.
5. If the board enumerates as a serial device on the laptop, flash it and capture at least 30 s of logs showing both channels reporting plausible clean-air voltages with the heaters warm.

## Success criteria
- `esphome config` validates the YAML clean.
- `esphome compile` succeeds.
- The YAML is committed on branch `hermes/0208-propane-bench-firmware`.
- If flashed: the reply's log excerpt shows A0 and A1 both reporting. Values need not be calibrated — both sensors are still conditioning, and drift in the first hours is expected, not a failure.

## Constraints
- ESPHome only, no custom firmware. If ESPHome cannot express something above, stop and report rather than improvising C++.
- Cost: standard tier only; no premium models for a YAML authoring task.
- Any generated keys (e.g. an API encryption key) stay in local files on the laptop — name the file path in the reply; never paste a key into the reply or the repo.
- Do not reassign any pin; the rig is wired exactly as documented above.

## Proof
- Branch name and commit SHA on origin.
- `esphome version`, the `esphome config` result, and the compile outcome.
- Serial log excerpt if flashed (or a plain statement that the board was not enumerated).

## Reply format
Status line, then: repo path used, branch + SHA, validation/compile results, flash result + log excerpt if any, and anything blocked (secrets, board not found) stated plainly.

expect-reply-within: 6h
