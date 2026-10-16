---
task_id: "0140"
protocol_version: 1.0.0
status: filed
iteration: 1
expect-reply-within: 6h
---

# 0140 — Guition panel LCARS restyle (Star Trek TNG feel)

## Context
The Guition JC4827W543 panel (480x272, ESPHome) shows a BMS page fed by four Home
Assistant imports (`bms_soc`, `bms_v`, `bms_i`, `bms_t`) plus local panel state
(`batt_pct`, `batt_state` "CHG"/"BATT", `ha_time`). Stephen wants the page restyled
with a Star Trek TNG LCARS feel. Current look: black background, pastel pill labels,
a circular SOC gauge, three stat pills (PACK V / CHARGE / TEMP), segmented color
bars at the bottom, top bar with panel-battery pill + clock + date.

The file is `guition-jc4827w543/jc4827w543.yaml` in the `pagosacabin/cabin-play`
repo — NOT nordtronics. Play projects live in cabin-play; push via SSH from your
laptop as with 0136 (the gh.py token is nordtronics-scoped and 403s on cabin-play
writes).

## Task
Restyle the BMS page display lambda in LCARS fashion:
- LCARS palette: orange #FF9900 / #FFCC99, salmon #CC6666, purple #CC99CC,
  periwinkle #9999FF / #9999CC, pale yellow #FFFFCC, on black #000000.
- Signature LCARS elbow bars (L-shaped rounded frames) framing the sections.
- Keep the big SOC readout prominent — keep the dial or convert it to an LCARS
  bar, your call.
- Keep all data on the page: PACK SOC %, PACK V, CHARGE current, TEMP, panel
  battery pill (e.g. PANEL CHG), clock, date.
- Preserve existing behaviors: "--" for NAN values, the low-battery page-select
  logic (`batt_pct` < `low_batt_pct` selects the status page), inactivity dimming,
  and the once-a-minute repaint trigger.

## Success criteria
1. `jc4827w543.yaml` compiles under ESPHome 2026.9.1 with no errors or new
   warnings.
2. The BMS page shows all seven data points (SOC, V, A, temp, panel state, time,
   date) in LCARS styling.
3. NAN still renders "--"; page-select and dimming behaviors unchanged — verify
   by reading the lambda, not just by compiling.
4. Photo of the physical panel displaying the new LCARS BMS page.
5. Committed and pushed to cabin-play; the staged reply states the commit SHA.

## Constraints
- Touch the display section only (plus color/font definitions as needed). Do NOT
  touch sensors, wifi, api, ota, ble_client, esp32_ble_tracker, the watchdog
  (`globals` / `interval` / `on_boot` / `on_value`), or the Home Assistant imports.
- 480x272: keep the primary numbers legible — no LCARS micro-text for SOC/V/A/temp.
- Keep model/provider cost economical; no premium tier needed for this task.
- Do not disturb the living-frame project files in the same repo.

## Proof
- Commit SHA on cabin-play origin containing the restyled lambda.
- Photo of the panel showing the LCARS BMS page (physical render proof, not just
  a compile log).
- ESPHome compile output showing zero errors.

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the photo, the
compile result, and notes on any behavior intentionally changed.
