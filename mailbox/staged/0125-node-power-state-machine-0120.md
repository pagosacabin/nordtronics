---
task_id: "0125"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0125-node-power-state-machine
    sha: aae32107784a60007d4819f03a762ac59a5ba9de   # == git ls-remote --heads origin
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37800832173
  - jobs: "build (green, incl. 'Build firmware (wildfire-node-v1, unified node+base)'
      + the artifact upload) and host-tests (green, '28 test cases: 28 succeeded',
      incl. the new test_power_state_pins [PASSED]). Run conclusion success; headSha
      aae32107784a60007d4819f03a762ac59a5ba9de == the branch tip."
  - artifact: wildfire-node-v1-unified-firmware, id 11561090952, 773665 B zip;
      downloaded -> firmware.bin 1229408 B, sha256
      17a2511f1e68dd3b87818a075563b123dff7351b69bcb03159e565d5e5fe1f1b. NOT flashed
      to any board (this task is code + CI; the bench acceptance gate runs later).
      https://github.com/pagosacabin/nordtronics/actions/runs/37800832173/artifacts
  - ntfy: topic nordtronics-build-ed05a663 on https://ntfy.sh, id gJxlo3Q28U9m,
      published 2026-10-08T15:31:17Z (epoch 1791473477), receipt read back from the
      topic after publishing (json?poll=1&since=10m returns the id and the exact
      Branch/SHA/Workflow/Artifacts/Status body)
  - base: "hermes/0119-battery-adc @ d6fce0ec1710c8e5847a57df1cb40b5f8fcaf59c --
      the revision whose image the NODE is running; this branch is that tip + 1
      commit. main does NOT carry the 0099/0104/0105/0108/0112/0115/0117/0118/0119
      firmware chain (main's firmware/wildfire-node-v1 is the 0094 revision), so a
      diff against main shows the whole chain, not just this task's change."
  - files:
      - firmware/wildfire-node-v1/src/main.cpp
      - firmware/wildfire-node-v1/src/firmware_config.h
      - firmware/wildfire-node-v1/test/test_role_and_portal/test_main.cpp
      - firmware/wildfire-node-v1/README.md
notes: |
  STAGED (iteration 1), 2026-10-08 15:15-15:4x UTC. Off-peak (PEAK: OFF-PEAK
  15:15 UTC). Model tier: DeepSeek Flash -- the cron worker's own cheapest tier,
  as the task requires. NO BOARD WAS TOUCHED: no flash, no serial port opened, no
  NVS write, nothing on USB -- the deliverable is code + CI and the bench gate
  below runs later at Stephen's bench.

  WHAT REGRESSED NOTHING. The PMS5003's 5 V rail is now switched, so the live PM
  diagnostic of tasks 0117/0118 is powered only inside the sensor window and the
  PM row holds its last checksum-valid frame between windows (0118's own latch
  semantics, not a change to them). The 30 s Plantower warm-up is served in 1 s
  slices with the 0117 poll and the 0118 render running through it, so those two
  behaviours are demonstrably still live in the window. Declared because it is a
  real change in sensor duty, not a defect.

  THE ONE THING OWED A DECISION -- the MiniBoost EN pin. 0125 says GPIO16 and
  Stephen wires it at the bench; the Rev C interface board (0076/0077) wires
  MiniBoost EN to Heltec GPIO4 with a 4.7 kOhm pull-down, and 0079 lists GPIO16
  in the V4.2 pinout as XTAL_32K_N. The firmware implements GPIO16 as specified,
  as a single constant (kPmsBoostEnablePin) with a host test pinning the value,
  so either pin is a one-line change. Do not bench the SENSOR_OFF current until
  it is settled. Full account, plus the Vext-policy question, in the reply body.

  SELF-CAUGHT DEFECT (fixed before the push, so invisible unless written down):
  the first cut of firmware_config.h spelled the boost level as `HIGH`, which
  does not exist in the portable header the host test compiles --
  `pio test -e native` failed with "‘HIGH’ was not declared in this scope" on
  test_role_and_portal. Caught by running the host tests locally before pushing;
  fixed to a plain `int 1` with the reason in a comment. The pushed revision is
  the fixed one.

  PROCESS DEVIATION: pickup and staging both landed in this one tick (the pickup
  commit is 6632055, then the staging commit). The work was done first and the
  mailbox transitions followed, so the two-commit history is real but compressed;
  `iteration` moved 0 -> 1 exactly once, at the pickup.

  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-08 15:15 UTC. The
  filed front-matter carried no `protocol_version`; it was added at pickup
  (protocol field, rule 16). The seven tasks in active/ at pickup
  (0097/0098/0106/0107/0109/0110/0119) are all decision-blocked and were left
  completely untouched (rule 6). Task 0125's stated firmware path
  (`firmware/node-v1/src/main.cpp`) is the 16-line 0004 scaffold; the firmware the
  task describes is `firmware/wildfire-node-v1/`, so the work was done there.
---

# 0125 — Firmware release 0120: first real node power-state-machine

Stephen's call (2026-10-06), "proceed" given 2026-10-07 ~15:17 MDT after the LiDAR thread closed.
(Note: this mailbox task is 0125; the firmware release number is 0120 — do not confuse them.)

## Context

- Node (Heltec LoRa 32 V4, 915 MHz) currently runs the recovered 0119 image; base runs 0112. Both stay as-is until this task's bench gate is reached.
- The bench node is ON A DISCHARGE RUN (restarted 2026-10-07 ~10:59 MDT at 3.949V; 3.792V/38% at 15:17 MDT, est. 20–30 h total). Do NOT flash the node or interrupt the run. The MQTT quiet-watcher page (5 min telemetry silence) marks time-of-death; Stephen/Juno decide when the run is over. Your work on this task is code + CI; the bench acceptance below runs at Stephen's bench after the discharge run ends, with his hands on the wiring.
- GPIO16 chosen for the MiniBoost EN line (RTC-capable, non-strapping, verified free: not LoRa, I2C, PMS UART, OLED, battery ADC, USB, or boot). MiniBoost modules are on the bench. Stephen wires EN himself; code must not assume the wire exists before the bench step.
- Battery ADC: Heltec V4.2 on-board switched divider — GPIO37 HIGH → sample GPIO1/ADC1_CH0 → GPIO37 LOW, LOW guaranteed on every path. (You corrected the GPIO2 spec from the V4.2 datasheet — keep it GPIO1.)
- Observed during the discharge run: the node's LoRa TX path wedged ~21 min after the USB unplug (a node reboot restored it; root cause unknown). Note radio re-init robustness anywhere the state machine touches the radio.
- Settings model: `firmware-spec-settings-draft.md` in the goal files. Radio params are captive-portal config items: firmware default + portal field + NVS. The base portal configures WiFi credentials + LoRa/node settings ONLY — the MQTT broker (mqtt.nordtronics.io/8883) is a firmware default, never a portal field (Stephen, 2026-10-03).
- Firmware lives at `firmware/node-v1/src/main.cpp` (PlatformIO, `firmware/node-v1/`).

## Task

Implement firmware release 0120 on the wildfire node:

1. Power-state machine: WAKE → SENSOR POWER (Vext on + PMS5003 boost enable on GPIO16) → SAMPLE → SENSOR OFF → LORA TX → alarm ACK window → DEEP SLEEP.
2. PMS boost enable firmware-controlled on GPIO16; no UART back-power when the boost is off (self-review your diff for this).
3. Battery telemetry via the GPIO37/GPIO1 divider sequence above.
4. RTC_DATA_ATTR sequence counter; alarm state persists across deep sleep.
5. Two documented alarm layers: node-fast-alarm (PM2.5 ≥ 55) in node firmware vs base-consensus (≥2 nodes rising in the same correlation window) in the base — name which layer each code path belongs to.
6. Field mode sheds WiFi/AP/portal. Provisioning is a deliberate maintenance mode: 3 s BOOT-button hold at boot with OLED indication → WiFi + captive portal.
7. Deep sleep stays GATED OFF for the bench phase (node awake, 60 s checkin). Re-enabling deep sleep is the FINAL GATE before field — a separate decision, not this task. Your 0120 build keeps the gate-off behavior until that separate task.
8. Keep the 0118 behavior: live PM line on the node's 1 s OLED render.

## Success criteria

- Firmware builds clean and CI is green on your branch.
- Acceptance gate (runs at Stephen's bench after the discharge run, his hands): measured PMS5003 OFF-state current + UART-pin voltage with the MiniBoost disabled, per rung — wire MiniBoost EN to GPIO16 → PM2.5 at 60 s checkin → verify full PMS shutdown → characterize sleep states.
- Your staged reply includes the step-by-step bench plan for that acceptance gate (pin list, meter points, the pass thresholds you computed) and states explicitly what was NOT done: no node flash during the discharge run; deep sleep still gated off.

## Constraints

- Bench discipline on anything you flash: USB selected by the stable by-id path + MAC check; partition-table read first to prove NVS safety; sha256 on the read-back.
- Do not interrupt or flash the bench node while the discharge run is live.
- Cost: use your cheapest model tier for this task. Do not use premium/pro models.
- No hardware mods — the EN wire is Stephen's bench move, not yours.
- Do not invent new top-level repo directories.

## Proof

Per mailbox/README.md: branch tip SHA on origin + Actions run URL, as pointers (not pasted logs).

## Reply format

Stage with front-matter proof (branch + sha, run URL). Body: what changed, the bench plan for the acceptance gate, what was explicitly not done, and any open questions for the bench step.

---

# Reply — release 0120 implemented; CI green at the branch tip

**Branch** `hermes/0125-node-power-state-machine` @ `aae32107784a60007d4819f03a762ac59a5ba9de`
(base `hermes/0119-battery-adc` @ `d6fce0e` + 1 commit — the revision whose image the
node is running; **main does not carry that chain**, so a diff against main shows the
whole 0099→0119 lineage, not just this change).
**Run** https://github.com/pagosacabin/nordtronics/actions/runs/37800832173 — `build`
green (including the wildfire firmware build + artifact upload), `host-tests` green
(**28 test cases: 28 succeeded**, including the new `test_power_state_pins`), run
conclusion `success`, `headSha` == the branch tip.
**Artifact** `wildfire-node-v1-unified-firmware`, id `11561090952`, 773 665 B zip →
`firmware.bin` 1 229 408 B, sha256
`17a2511f1e68dd3b87818a075563b123dff7351b69bcb03159e565d5e5fe1f1b`. Downloaded and
hashed, **not flashed anywhere**.
**ntfy** `nordtronics-build-ed05a663`, id `gJxlo3Q28U9m`, 2026-10-08T15:31:17Z —
published in the Branch/SHA/Workflow/Artifacts/Status form and read back from the topic.

## 0 — a path correction, declared up front

The task says "Firmware lives at `firmware/node-v1/src/main.cpp`". That file is the
16-line 0004 scaffold (`NODE_FW_VERSION = "0.1.0-scaffold"`); it has no sensors, no
radio and no sleep. The firmware this task *describes* — PMS5003 + BME680 + the 1 s
OLED render + the deep-sleep gate of 0112, "keep the 0118 behavior" — is
`firmware/wildfire-node-v1/`, and the image the bench node runs is the 0119 branch tip.
All the work is there. `firmware/node-v1` was not touched.

## 1 — the power-state machine (item 1)

`NodeState` + `node_enter()` in `src/main.cpp`; every transition logs
`power: <STATE>` and the state rides the OLED's `seq:` row. One cycle per check-in
period:

| state | what it does |
|---|---|
| `WAKE` | logs the wake cause (`esp_sleep_get_wakeup_cause()`) and the RTC cycle / sequence counters |
| `SENSOR_PWR` | asserts Vext, drives PMS boost EN HIGH, attaches the PMS UART, then serves the warm-up |
| `SAMPLE` | one PMS5003 frame, one BME680 reading, one battery-divider sample; repaints the panel |
| `SENSOR_OFF` | detaches the UART and floats the MCU TX pin, **then** drops the boost EN |
| `LORA_TX` | builds and transmits the frame; on a transmit error re-inits the radio once and retransmits |
| `ALARM_ACK` | only for a LAYER 1 alarm: the ACK window with `ack_tries` retransmits |
| `SLEEP` | field: `enter_deep_sleep()`. Bench: logs the gate and moves to `IDLE` |

**Radio re-init robustness** (your context note about the TX path wedging ~21 min after
the USB unplug): a failed `transmit()` now logs the error, re-runs `g_radio.begin(...)`,
and retransmits once before giving up — a wedged radio becomes a logged, recoverable
event rather than a silent loss until the next wake.

## 2 — PMS boost enable + no UART back-power (item 2)

`kPmsBoostEnablePin` = GPIO16, driven **LOW as the very first thing in `setup()`**
before even Vext. That ordering is load-bearing: the MiniBoost carries a 100 kΩ EN→VIN
pull-up, so the previous build (which never drove the pin) had the 5 V rail **on** the
whole time the node was awake — which is also why the role probe's PMS frame test used
to pass without anyone powering anything.

- `sensor_power_on()`: Vext asserted (idempotent), boost EN HIGH, UART attached.
- `sensor_power_off()`: UART detached, **`pinMode(kPmsUartRxPin, INPUT)`** (GPIO6 is
  the MCU's TX — `kPmsUartRxPin` is named from the PMS's side), and only then the boost
  EN LOW. No line is left sourcing into the unpowered module.
- `probe_node_sensors()` now raises the rail for its PMS test and hands it back down,
  because the rail is off by default; without that change every sensor-only node would
  have probed as a base station.
- Warm-up: `kPmsWarmupMs = 30000` (Plantower manual V2.3; the Rev C schematic's own
  "allow >= 30 s warm-up" note). Served in 1 s slices, not one 30 s `delay()`, so the
  0117 poll and the 0118 render keep running — see §7.

## 3 — battery telemetry (item 3)

Unchanged from 0119 and now taken inside the `SAMPLE` step: ADC_Ctrl (GPIO37) HIGH →
16× `analogRead`/`analogReadMilliVolts` on ADC1_CH0 (GPIO1) at 0 dB → GPIO37 LOW on
the single exit path, `vbatt = pin_mv / 0.2041`. `g_batt_mv` is latched so the OLED row
and the LoRa payload cannot disagree. The `batt_adc_pin_probe()` boot evidence stays.

## 4 — RTC-persistent state (item 4)

`RTC_DATA_ATTR` on: `g_tx_seq` (wire sequence, monotonic across wakes), `g_alarm_active`
(the LAYER 1 latch), `g_alarm_events` (episodes raised), `g_cycles`. Semantics: a valid
frame ≥ threshold sets the latch and counts the episode once; a valid frame below the
threshold clears it; **a failed frame changes nothing** — it cannot fabricate a clear.
The old build's plain `static` counter restarted at 0 on every wake, and its alarm was
re-derived from the current frame only.

## 5 — the two alarm layers, named (item 5)

| layer | code path | rule |
|---|---|---|
| **LAYER 1 — node-fast-alarm** | node firmware only, `node_sample()` in `src/main.cpp`; threshold `kNodeFastAlarmPm25 = 55.0f` | PM2.5 ≥ 55 µg/m³ on the reading in hand. No consensus, no other node. Sends an ACK-required ALARM frame. |
| **LAYER 2 — base-consensus** | base firmware only, `src/consensus_v02.cpp` | ≥ 2 nodes rising inside one correlation window. A node never evaluates it. |

Documented in a header block in `src/firmware_config.h` and a README table.

## 6 — field mode sheds the network; provisioning is a gesture (item 6)

- `provisioning_gesture()`: GPIO0 (BOOT) held through the first `kProvisionHoldMs` =
  3 s, with the panel counting down (`PROVISION?` / `hold 3s` / `release = field`).
- `setup()`: `g_provisioning = (role == Base) ? true : provisioning_gesture()`; a node
  that did **not** ask returns from `setup()` before any WiFi/TLS/portal call, and
  `loop()` guards `g_server.handleClient()` on the same flag. A node in field mode
  therefore carries no AP, no STA, no portal and no MQTT config at all.
- A **base always** brings the network up — it is the uplink — so nothing changes for
  the base role.
- Documented caveat: press BOOT *after* the board has started. GPIO0 held **through**
  power-on or reset is the ESP32-S3 ROM's download-boot strap, so holding it before
  power-up boots the ROM, not the app. The gesture is read ~2 s in, after the
  bootloader has already sampled the strap.

## 7 — what is kept, and the one real behaviour change to know about

Deep sleep is **still gated off**: `kDeepSleepEnabled == false` and the `chk_s` portal
default is still `60`; `kCheckinSecondsField == 720` is untouched. Item 7 is satisfied.

The 0118 behaviour is kept *and* its render cadence is preserved inside the warm-up
window. What **is** different, and is declared deliberately: the PMS is now powered only
for the sensor window, so between windows the 1 Hz poll has no bytes to read and the PM
row shows the last checksum-valid frame — which is 0118's own latch rule ("never fall
back to 0"), not a change to it. With the 60 s bench cadence and the 30 s warm-up, the
PM row updates live for roughly half of every cycle. If you would rather the bench keep
the rail up continuously, that is one line in `sensor_power_off()`/`sensor_power_on()`;
say the word and it is a follow-up commit (do **not** ask for it mid-bench — it changes
the OFF-state current the gate below measures).

## 8 — the pin this task names is not the pin the Rev C board wires

Declared before anything is measured, because it changes what a bench reading means:

- **Task 0125 says GPIO16** for MiniBoost EN, "Stephen wires EN himself".
- **Rev C (tasks 0076/0077) wires MiniBoost EN to Heltec GPIO4**, with a 4.7 kΩ
  pull-down to GND alongside the module's own 100 kΩ EN→VIN pull-up:
  *"EN <- Heltec GPIO4. 4.7 kΩ pull-down resistor from EN to GND"* (0076 §2).
- **Task 0079 lists GPIO16 in the V4.2 pinout as `XTAL_32K_N`** (it is RTC-capable for
  exactly that reason), and 0076's interface freeze does not include GPIO4 or GPIO16.
- 0092 already flags this area as "worth a check when firmware is written": *"R2 on the
  Rev C is a 4.7 k pull-down on the MiniBoost EN pin, which only DISABLES the boost —
  the MCU has to actively drive EN high."*

The firmware implements **GPIO16 as the task specifies** (one constant,
`kPmsBoostEnablePin`, plus a host test that pins the value and asserts it is clear of
the 0079 frozen nets and every ESP32-S3 special function). If the bench is being wired
on a Rev C interface board rather than hand-wired, this needs a decision first — either
move the constant to 4, or wire EN to GPIO16 — because on a Rev C board with the
firmware at GPIO16 the rail would never come up (nothing drives GPIO4) and the
`SENSOR_OFF` current measurement would be measuring a rail that was never on.

## 9 — bench plan for the acceptance gate (pin list, meter points, thresholds)

Runs at Stephen's bench after the discharge run. **Nothing here is done yet.**

**Pins**

| pin | role |
|---|---|
| GPIO16 | MiniBoost EN — the bench wire Stephen makes (HIGH = boost on) |
| GPIO6 | MCU TX → PMS RX (floated by `SENSOR_OFF`) |
| GPIO5 | PMS TX → 1 kΩ → MCU RX (the serial evidence line) |
| GPIO36 | Vext, switched 3.3 V rail (stays asserted per cycle in this build — see the open item) |
| GPIO37 | battery ADC_Ctrl (HIGH = divider connected) |
| GPIO1 | battery divider tap (ADC1_CH0) |
| GPIO0 | BOOT — the provisioning gesture |

**Meter points:** TP1/`VBAT_F` (pack), TP2/`5V2` (boost out), TP3 `3V3`, TP4 `Vext`,
TP6 PMS-TX, TP7 GND star. Break the 5V2 feed at the PMS VCC connector for the current
rungs; a µA-capable meter, not a shunt-less clamp.

**Rungs and pass thresholds**

| rung | setup | expected | pass |
|---|---|---|---|
| R1 — PMS OFF, boost disabled | node awake in `IDLE` (between windows), meter in series with the PMS VCC on 5V2 | MiniBoost disabled, PMS unpowered | **< 50 µA** into the PMS (vs the 60–100 mA of R2 — four decades of margin, so the exact leakage does not matter) |
| R2 — PMS ON | node in `SENSOR_PWR`/`SAMPLE` | PMS active | 60–100 mA at 5 V (datasheet ≤ 100 mA); this is the load the release buys back |
| R3 — UART back-power | boost off, meter on **GPIO6** to GND, and on the PMS RX pad | MCU TX must be high-Z | **GPIO6 ≤ 0.1 V** (not 3.3 V) and the PMS RX pad **< 0.3 V** — above that the sensor's input protection is conducting and the rail is being back-powered |
| R4 — full PMS shutdown | boost off, listen at the inlet + watch 5V2 | fan/laser off | 5V2 **0.0 V**, no fan, and the `pms-live:` line stops within one poll period while `power: SENSOR_OFF` is on the console |
| R5 — state sequence | 30 s serial capture over ≥ 2 cycles at `chk_s = 60` | the states in order, once per cycle | `power: WAKE → SENSOR_PWR → SAMPLE → SENSOR_OFF → LORA_TX → (ALARM_ACK) → SLEEP → IDLE`, `bench gate: deep sleep DISABLED` each cycle, no `FATAL`, no reboot (`rst:` absent from the capture) |
| R6 — sleep states | **requires the sleep gate restored** (separate decision): `kDeepSleepEnabled = true` + flash, then remove USB and meter the pack | board asleep, boost held off | pack current ≤ 50 µA (0092's 30 µA board-level assumption, 7 µA floor) and **5V2 = 0 V** — if 5V2 reads ~5 V the RTC hold lost to the module's EN pull-up. Also measure the USB-UART bridge contribution (0092: ~1.4 mA if it is not held in reset — the single highest-value measurement in the budget) |
| R7 — provisioning gesture | hold BOOT ~1 s after power-up for > 3 s; repeat releasing early | panel countdown then `PROVISIONING MODE`; early release → `field mode` | both serial lines present; `wifi: dhcp hostname [...]` only in the provisioned run; a node in field mode shows **no** AP in a scan |

Also worth capturing at the same bench: the boot line
`power: sensor rail up -- ... UART 9600 baud attached` and the `batt:` line, to confirm
the divider sequence still runs inside `SAMPLE`.

## 10 — explicitly NOT done

- **No board touched at all**: no flash, no serial port opened, no esptool call, no NVS
  write, nothing on USB. The discharge-run constraint was respected trivially because no
  bench action was taken — this task is code + CI, as 0128 also states.
- **Deep sleep not re-enabled** and the check-in period not moved off 60 s (item 7).
- **No bench measurement** of the OFF-state current or the UART-pin voltage — R1–R4 are
  blocked on the GPIO16/GPIO4 decision in §8.
- **The settings model was left alone** (see open question 4).

## 11 — open questions for the bench step

1. **GPIO16 or GPIO4?** (§8) — blocking for R1–R4.
2. **Vext policy.** `SENSOR_OFF` leaves Vext asserted: on this board family the SSD1306
   panel shares that rail (0108's note) and the awake bench phase needs a live panel
   (item 8), while the load and the back-power path the gate measures are the 5 V boost
   and the UART. If the panel is in fact on 3V3, Vext should be switched per cycle too —
   one line in `sensor_power_off()`. **Please confirm which rail the panel is on**; if it
   is Vext, switching it would blank the panel between windows (and need a re-`begin()`
   on every cycle), so it was not done speculatively.
3. **Continuous live PM in the bench phase?** §7 — one-line change either way.
4. **The settings-model bullet.** The task's context says the broker
   (`mqtt.nordtronics.io:8883`) must be "a firmware default, never a portal field", while
   `mqtt_host`/`mqtt_port` (and `mqtt_user`/`mqtt_pass`) are portal fields today. That is
   a *Context* bullet, not one of the eight numbered Task items, and the spec it points
   at — `firmware-spec-settings-draft.md` "in the goal files" — is not reachable from
   here: `git ls-tree -r --name-only origin/main | grep -i firmware-spec` → empty;
   `git log --all -- '*firmware-spec-settings*'` → empty;
   `git rev-list --all --objects | grep -i firmware-spec` → empty; a filesystem scan of
   `$HOME` → empty. Per the mailbox rule about unreachable specs I did **not** guess the
   intended field set or delete portal fields on my own authority (removing them changes
   the base's only provisioning surface, and the broker host/port already default to
   exactly the values named). **Say which fields the base portal should expose** and it
   is a small, self-contained follow-up.

## 12 — verification actually performed (what I ran, not what I assume)

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` locally → `SUCCESS`,
  RAM 16.6 % / Flash 18.8 %; only the pre-existing `-Wcpp` warnings, no new ones.
- `pio test -d firmware/wildfire-node-v1 -e native` locally → **28 test cases: 28
  succeeded**, including `test_power_state_pins` (its own stdout:
  `[power] boost_en=gpio16 on_level=1 warmup_ms=30000 boot_btn=gpio0 hold_ms=3000 alarm_pm25=55.0 boost_is_frozen_net=0`).
- CI run `37800832173`: `gh run view --json conclusion,headSha` → `success` /
  `aae3210…` == `git ls-remote --heads origin hermes/0125-node-power-state-machine`;
  both jobs green; the host-tests log carries `28 test cases: 28 succeeded`.
- Artifact downloaded and `sha256sum`'d (value in the proof block).
- ntfy published and **read back** from the topic (`json?poll=1&since=10m`), not just
  assumed from the HTTP response.
- The pin-claim checks are machine-checked by `test_power_state_pins`, not asserted in
  prose: GPIO16 is not on the 0079 frozen net list, is not a boot-select / native-USB /
  UART0 / SX1262-SPI pin, and is not the I2C, OLED, PMS-UART, battery-ADC or BOOT pin.

## 13 — self-caught defect, declared

The first cut of `firmware_config.h` wrote the boost level as `HIGH`. That header is
portable C++17 (the `native` host test compiles it, and it deliberately has no Arduino
dependency), so the test build failed:
`src/firmware_config.h:123:34: error: 'HIGH' was not declared in this scope`. Caught by
running the host tests locally *before* pushing, fixed to a plain `int 1` (comment
explains why), and the pushed revision is the fixed one. Written down because a defect
fixed before its commit is otherwise invisible to you.

## 14 — doc corrections carried in the same commit

- `firmware/wildfire-node-v1/README.md` claimed "`batt_mv` is transmitted as 0: Rev C
  has no fuel gauge" — stale since 0119 added the divider read. Corrected to describe
  the actual measurement (one-line doc fix in a file this task already touches).
- The README gained the power-state / alarm-layer / provisioning sections and the two
  open items above, so the branch documents its own known state.

