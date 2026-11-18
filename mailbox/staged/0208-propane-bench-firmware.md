---
task_id: "0208"
protocol_version: 1.3.0
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-17T23:45:20+00:00"
lease_until: "2026-11-18T01:15:20+00:00"
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
proof:
  branch: hermes/0208-propane-bench-firmware
  sha: ccece52f89c7fabbfd00d0c0116c7d5839982442
  run: none
  files:
    - firmware/propane-sensor/propane-bench.yaml
    - firmware/propane-sensor/README.md
    - firmware/propane-sensor/bench-log-2026-11-17-serial.txt
  run_none_reason: "no workflow covers this path or branch — see notes"
  delivered: "ESPHome 2026.9.1 config, `esphome config` + `esphome compile` clean, flashed to the bench board, 120 s of both channels logged"
status: staged
notes: |
  STAGED. Run 2026-11-17 16:45-17:45 MST (23:45-00:45 UTC), off-peak
  (PEAK: OFF-PEAK 23:45 UTC), PROTOCOL MATCH 1.3.0.

  BRANCH TIP ccece52f89c7fabbfd00d0c0116c7d5839982442 verified by
  `git ls-remote --heads origin hermes/0208-propane-bench-firmware`; both commits on the
  branch are mine. `run: none` is earned, not assumed: `ls .github/workflows/` is
  android-build.yml (push branches [android-toolchain-setup]), android-companion-v0.yml
  (branch allow-list + android/companion-v0 paths), backend.yml (backend/**),
  detection-sim.yml (python/detection-sim/**), platformio.yml (firmware/tank-monitor/**,
  firmware/node-v1/**, firmware/wildfire-node-v1/**, python/detection-sim/**) and
  website-check.yml (branch allow-list). None matches firmware/propane-sensor/** nor any
  hermes/NNNN-* branch, and `gh run list --branch hermes/0208-propane-bench-firmware`
  returns empty. platformio.yml was NOT extended to cover this path: it builds PlatformIO
  projects and this is an ESPHome YAML, so a path entry would start a job that cannot build
  it. Hence no ntfy receipt either — there is no CI run and no artifact to point at; the
  ESPHome build output lives in the worker's scratch dir only.

  WHAT CHANGED. New directory firmware/propane-sensor/ with the ESPHome config, a README
  (wiring, secrets, entity map, bench findings) and the raw serial evidence. `esphome
  config` and `esphome compile` are clean on 2026.9.1 (config_hash 0xc078324e, 806 KB app,
  32% RAM, 44% flash). The board enumerated as an ESP32-C3 Super Mini (MAC
  9c:cc:01:d0:78:80, ESP32-C3 QFN32 rev v0.4) and was flashed twice with the deliverable
  image; the final image on the board is byte-identical to the committed YAML
  (sha256 70082cd1... before the review fixes, and the post-fix file is what was flashed
  last at 17:31).

  DEVIATION 1 — THE BRIEF'S I2C PINS ARE TRANSPOSED ON THE RIG AS WIRED. The task says
  "Hardware as wired (do not re-derive)" and gives SDA = GPIO6, SCL = GPIO7. On the live
  board that pair finds NOTHING: a probe build with the brief's pins logs
  `Recovery: bus successfully recovered` / `Results from bus scan:` / `Found no devices`
  and `ADS1115 ... Communication failed`. The same board, same power, with the pair
  transposed (sda: GPIO7, scl: GPIO6) logs `Found device at address 0x48` and reads both
  channels. I implemented the orientation that works and declared it at the top of the YAML
  and in the README, because a config that cannot talk to the ADC meets none of the
  criteria. If the harness is rewired to match the brief instead, two lines swap back.
  Everything else in the wiring table was taken as given and not re-derived.

  DEVIATION 2 — ADS1115 VDD AT 5 V PULLS SDA/SCL TO 5 V (bench hazard, not firmware).
  Raised by Copilot's review and confirmed as an open item by its second pass. The C3 is
  not 5 V tolerant; if the ADS1115 module pulls SDA/SCL up to VDD, GPIO7/6 see 5 V. The rig
  works today, but before this rung is trusted it wants 3.3 V pull-ups or an I2C level
  shifter (VDD must STAY at 5 V — the sensor VRL outputs are 0-5 V and A0/A1 max out at
  VDD+0.3 V, so lowering VDD is the wrong repair). This is a bench decision for Stephen;
  I changed no wiring.

  REVIEW (Copilot pane t17 via herdr-core, per the standing routing in the assignment).
  Three findings; I fixed two in the code and left the third as the hardware item above.
  1. MAJOR — 5 V I2C pull-ups (open, see DEVIATION 2).
  2. BLOCKER — an unreadable channel read as clean air, so a gas rise on a dead channel was
     undetectable, and at boot with no readings the alarm stayed off. FIXED: a channel that
     is nan now publishes a `Sensor fault` binary sensor and, after a 30 s grace window,
     latches the gas alarm ON; a rise on whichever channel is still valid trips
     immediately. While fixing it I found a second defect of my own: the grace timer could
     never expire, because a dead channel publishes nothing and nothing else re-ran the
     evaluation — so `evaluate_alarm` now also runs on the 10 s interval tick. Proven on
     hardware with a probe build pointing at 0x49 (bench log section 4: grace line, then
     `FAULT ... alarm forced ON` at +30 s, alarm stays ON).
  3. NIT — a 0.1 V threshold floor with 0.1 V hysteresis puts the release level at 0 V, so a
     tripped alarm could never clear. FIXED: floor raised to 0.15 V (release >= 0.05 V).
  Second pass verdict from the pane: findings 2 and 3 RESOLVED, finding 1 NOT RESOLVED
  (hardware), overall VERDICT: FIX on that one open item. That verdict is why this is
  staged with the hardware item declared rather than claimed closed.

  VERIFIED ON HARDWARE, WITH THE ARTEFACT READ (not assumed):
   - both channels report every second: MQ-6 ~1.47 V, Figaro ~1.52 V in still air, drifting
     slowly down as the heaters condition (spec allows uncalibrated values);
   - alarm TRIP and RELEASE both execute: a probe build that ramps the threshold shows
     `ALARM` at 1.0 V then `CLEAR` at 2.5 V with the buzzer/LED driven by the same script;
   - fault path latches ON after 30 s of no data (probe at 0x49);
   - boot log shows the ADS1115 configured at 0x48, gain 6.144 V, 16-bit, 860 SPS, and no
     communication errors with the shipping pins.
  Raw device output for all four is in bench-log-2026-11-17-serial.txt; nothing in it is
  hand-written except the headers and elision marks (WiFi-retry lines removed).

  WHAT THE BENCH SAYS ABOUT THE SPEC'S 1.0 V DEFAULT. Clean air on this rig sits at
  1.5-ish V on both channels, so the brief's 1.0 V starting threshold alarms immediately —
  the delivered image did exactly that on first boot. I kept 1.0 V because criterion 3
  specifies it, and said so in the README; ~2.0 V is the sensible first move once the parts
  have conditioned, and the shootout data should set the final number. Related honesty
  note: `restore_value: true` means the stored value wins over `initial_value`, and the
  board currently holds 2.5 V in NVS from the transition probe, so it is sitting at
  `alarm=off`. Nothing in the repo depends on that.

  NOT DONE / NOT PROVEN. No gas was applied to either sensor — no response curve, no
  calibration, and the buzzer type (active vs passive) is still unconfirmed, so it is
  driven as a plain switch as the brief allows. No WiFi: the only secrets.yaml on this
  laptop is a template with `YOUR_WIFI`, so the flashed unit has no credentials and loops
  on WiFi retries; the `!secret` refs are left in place exactly as instructed, with an
  api_encryption_key generated into a scratch secrets file (mode 600, never committed,
  never echoed here). The keys the deployed device will need are listed in the README.
  The alarm's *audible* behaviour is unverified beyond the GPIO switch state.

  COST. The harness exposes no per-call DeepSeek usage to this cron session, so the token
  and cost fields are `unknown` rather than estimated.
---
# 0208 — Propane bench firmware: STAGED

Deliverable: `firmware/propane-sensor/propane-bench.yaml` on
`hermes/0208-propane-bench-firmware` @ `ccece52`, plus a README and the raw serial log in
the same directory.

## What it does

One ESPHome config for the ESP32-C3 Super Mini + ADS1115 rig. Alarm logic is on the device
and needs no network: trip when either channel rises above a tunable threshold, clear only
when both fall below `threshold − 0.1 V`, and treat an unreadable channel as a FAULT
(30 s grace, then alarm ON) rather than as clean air. WiFi/API/HA are reporting only.

## Criteria

| Criterion | Result |
|---|---|
| `esphome config` validates clean | PASS — "Configuration is valid!" (2026.9.1) |
| `esphome compile` succeeds | PASS — config_hash 0xc078324e, 806 KB app, 32% RAM / 44% flash |
| YAML committed on `hermes/0208-propane-bench-firmware` | PASS — tip `ccece52` |
| If flashed: ≥30 s of logs with both channels reporting | PASS — flashed; 120 s, 13 status lines, both channels ~1.47 V / ~1.52 V |

## Two things the bench corrected

1. **The brief's I2C pins are transposed on the rig as wired.** SDA6/SCL7 finds *no device*
   (`Found no devices`, ADS1115 marked FAILED); SDA7/SCL6 finds `0x48` and reads. The YAML
   uses the working pair and says so at the top; swap two lines if the harness is rewired.
2. **The 1.0 V default threshold is below this rig's clean-air baseline** (~1.5 V), so a
   freshly flashed unit alarms immediately. Kept at 1.0 V per criterion 3; ~2.0 V is the
   first thing to try once the sensors have conditioned.

## Peer review

Copilot (pane t17 via herdr-core) found three issues. Two are fixed and proven on
hardware — the fail-safe fault path (a dead channel used to read as safe) and the
threshold floor vs hysteresis. The third is a **hardware** item, not resolvable in
firmware: ADS1115 VDD is 5 V, so its SDA/SCL pull-ups put 5 V on the C3's GPIO7/6.
It wants 3.3 V pull-ups or a level shifter on the I2C pair (VDD must stay 5 V for the
0–5 V sensor outputs). Its second pass says RESOLVED/RESOLVED/NOT RESOLVED on exactly
that basis.

## Evidence

`firmware/propane-sensor/bench-log-2026-11-17-serial.txt` — bus-orientation probe, the
deliverable's boot + 120 s of both channels, the alarm trip/release probe, and the
sensor-fault probe.

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
