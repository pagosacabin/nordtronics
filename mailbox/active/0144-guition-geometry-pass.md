---
task_id: "0144"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
---

# 0144 — Guition LCARS: geometry/layout pass (colors deferred)

## Context
Stephen's call: the BMS page geometry needs work before colors. The color
pipeline investigation (0143: orange→lime shift, fits no simple model) is
parked — this task is layout only. Two references:
- Current on-panel layout: `mailbox/files/0143-panel-photo.jpg` (the 0141
  build, 480x272).
- Target LCARS composition:
  https://i.pinimg.com/originals/6a/1f/d7/6a1fd710ea257bebbb95a410806cb242.png

Known geometry issues, from the photo:
1. The left-side pill ("PACK SOC %") is jammed against the frame's top-left
   elbow — it needs clear space on all sides, not just a nudge.
2. The stat label pills (PACK V / CHARGE / TEMP) are dwarfed by their values
   and misaligned with them — label and value don't read as a unit.
3. A wide bar across the bottom isn't earning its space; the segmented bars
   below it read as decoration without meaning.
4. The composition still reads as "dashboard with bars taped on" rather than
   LCARS: the elbow frame should structure the page (top sweep + left rail),
   with content blocks sitting inside it.

## Task
Rework the BMS page display lambda's geometry only:
- Give the SOC block room: reposition/resize so the label and the big readout
  read as one unit, clear of the frame.
- Make each stat (V, A, temp) a coherent block: label pill + value aligned and
  proportioned as a unit. LCARS convention: the pill carries the label in black
  text, the value sits adjacent in a consistent position.
- Either give the bottom bars a job (e.g. proportional indicators) or remove
  the dead space and let the content breathe.
- Push the composition toward the reference: the elbow frame structures the
  page; content blocks sit inside it with consistent margins.
- Keep all seven data points legible at 480x272: PACK SOC %, PACK V, CHARGE
  current, TEMP, panel battery pill, clock, date.

## Constraints
- GEOMETRY ONLY. Do not touch the palette declarations or any color values —
  the color pipeline is under separate investigation and the palette must stay
  byte-identical so renders remain comparable across revisions.
- Preserve: "--" for NAN values, the low-battery page-select logic, inactivity
  dimming, the once-a-minute repaint.
- Display lambda + layout tooling only. Do NOT touch sensors, wifi, api, ota,
  ble_client, the watchdog, or the Home Assistant imports.
- 480x272: every element fully on-screen, no clipping or overlap — extend
  tools/audit_layout.py if its checks don't cover the new arrangement.
- Keep model/provider cost economical; no premium tier needed.
- Push via SSH to pagosacabin/cabin-play as with 0140/0141 (the gh.py token is
  nordtronics-scoped and 403s on cabin-play writes).

## Success criteria
1. `jc4827w543.yaml` compiles under ESPHome 2026.9.1 with no errors or new
   warnings.
2. No overlapping or clipped elements, verified by audit_layout.py (extended
   if needed).
3. All seven data points present and legible.
4. Palette declarations byte-identical to the 0141 revision — prove with a
   diff of the palette block.
5. Behaviors preserved (NAN→"--", page-select, dimming) — verify by reading
   the lambda.
6. Commit SHA on cabin-play origin; render PNG committed; panel OTA-flashed
   with the new build timestamp reported.

## Proof
- Commit SHA on cabin-play origin.
- Render PNG of the revised page.
- ESPHome compile output, zero errors.
- New "compiled on" timestamp read back from the panel (Stephen photographs
  the glass — he's the eyes).

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the geometry
changes made, the palette-diff proof, the new build timestamp, and notes on
anything unexpected.
