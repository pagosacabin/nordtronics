---
task_id: "0125"
status: inbox
iteration: 0
expect-reply-within: 6h
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
