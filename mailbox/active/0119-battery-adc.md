---
task_id: "0119"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-06 21:15 UTC.
  Off-peak (PEAK: OFF-PEAK 21:15 UTC). The six tasks in active/
  (0097/0098/0106/0107/0109/0110) are all decision-blocked and were left
  completely untouched.
---

# 0119 — Node battery voltage telemetry (node only)

## Context

The node currently hardcodes `batt_mv = 0`: the OLED shows `vbat:0mV` and the
backend/app show 0.00 V. Stephen connected a 3.7 V li-ion to the node today.
The Heltec LoRa 32 V4.2 has an on-board switched battery divider — no external
hardware needed. Base branch: `hermes/0118-pms-oled`; new branch
`hermes/0119-battery-adc`.

## Task

Implement `read_battery_mv()` on the node using the V4.2 on-board divider:

1. Drive GPIO37 HIGH to enable the divider, wait ~10 ms for settle.
2. Sample GPIO2 (ADC1) 16 times, average the raw readings.
3. Convert: divider is 390 kΩ / 100 kΩ, so the ADC sees VBAT × 0.2041.
   `vbatt_mv = adc_mv / 0.2041`. Set ADC attenuation to cover ~0–1 V at the
   pin (4.2 V battery → ~0.86 V at ADC).
4. Drive GPIO37 LOW before returning — on EVERY exit path, including ADC
   errors. A stuck-high GPIO37 drains the battery through the divider.

Wire the result into the existing 60 s checkin: replace the hardcoded
`batt_mv = 0` with the measured value so it flows through the LoRa payload,
backend ingest, API, app, and the OLED `vbat:` line (latched at checkin
cadence is fine — battery doesn't move at 1 Hz).

## Success criteria

1. Branch `hermes/0119-battery-adc` builds green in CI from the 0118 tip.
2. The NODE (MAC `B0:A6:04:C5:75:4C`, by-id path) — and only the node — is
   flashed; bench discipline holds (partition-table read first, sha256 on the
   read-back matches the built image, NVS preserved).
3. The node OLED `vbat:` line shows a plausible li-ion voltage (not 0 mV),
   and the backend API for node `"0"` reports non-zero `battery_v` after a
   checkin.
4. Stephen's step (stated in the reply, not Hermes's): compare the OLED/API
   voltage against his meter on the battery terminals — agreement within
   ~100 mV closes the task.

## Constraints

- NODE ONLY. Do not touch the base.
- GPIO37 LOW is guaranteed on every return path — grep the function and prove
  it in the reply.
- No changes to the PMS poll, 1 Hz OLED cadence, 60 s checkin timing,
  deep-sleep gating, or config. No merge, no tag.
- Keep model cost on DeepSeek Flash. State the tier in the reply.

## Proof

- Branch SHA on origin + CI run URL + artifact.
- Flash read-back sha256 matching the built image.
- API query showing node `"0"` with non-zero `battery_v`.
- Quoted `read_battery_mv()` showing the GPIO37 LOW guarantee.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, first line of notes confirms the OLED/API voltage and that
Stephen's meter check is his step, `proof` block with the pointers above.
