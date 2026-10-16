---
task_id: "0141"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
---

# 0141 — Guition LCARS revision: canonical TNG palette

## Context
0140 landed and runs (compiles clean, all live data on screen), but the aesthetic
missed the mark. Stephen's verdict from the panel photo: "I don't think Hermes
understands LCARS." Three specific misses to fix:

1. **Wrong palette.** The attempt used lime green + sky blue. Canonical TNG LCARS
   is orange-forward: peach, orange, salmon/red, purple, periwinkle. Lime green
   does not exist in LCARS.
2. **Blob artifacts at the elbow joints.** The frame bars meet with black circular
   blobs. Real LCARS elbows are one smooth continuous rounded sweep.
3. **Old dashboard layout retained.** The revision should push the composition
   toward real LCARS: a big asymmetric elbow frame, section pills carrying black
   text, not just a recolor of the previous pills.

The file is `guition-jc4827w543/jc4827w543.yaml` in `pagosacabin/cabin-play`
(push via SSH from your laptop as with 0136/0140 — the gh.py token is
nordtronics-scoped and 403s on cabin-play writes).

## Canonical palette (use these exact hex codes)
- Peach / tan: `#FFCC99` — large elbow frame pieces
- Orange: `#FF9900` — primary frames, main pills, key labels
- Light orange: `#FFCC66` — secondary pills
- Salmon / red: `#CC6666` — accents, warnings
- Purple: `#CC99CC` — secondary sections
- Lavender / periwinkle: `#9999CC`, `#9999FF` — secondary bars
- Pale yellow: `#FFFF99` — highlights only
- Background: `#000000`. Text on colored bars: black. Text on black: white/pale.
- BANNED: lime/chartreuse greens, sky/cyan blues anywhere on the page.

## Visual reference
Open these and match the look — asymmetric elbow frames, pill labels with black
text, the orange/purple/periwinkle family on black:
- https://i.pinimg.com/originals/6a/1f/d7/6a1fd710ea257bebbb95a410806cb242.png
- https://c.wallhere.com/photos/fe/2f/Star_Trek_LCARS-135996.jpg!s2

## Task
Revise the BMS page display lambda:
- Swap the full palette to the hex codes above.
- Redraw the elbow frame as smooth continuous rounded bars — no dot/blob
  artifacts at the joints.
- Push the composition toward the reference: big asymmetric top elbow sweeping
  the frame, section labels as colored pills with BLACK text (PACK V, CHARGE,
  TEMP, PANEL CHG), data values large and legible in white/pale on black.
- Keep all seven data points: PACK SOC %, PACK V, CHARGE current, TEMP, panel
  battery pill, clock, date.
- Preserve: "--" for NAN values, the low-battery page-select logic, inactivity
  dimming, once-a-minute repaint.

## Success criteria
1. `jc4827w543.yaml` compiles under ESPHome 2026.9.1 with no errors or new
   warnings.
2. No lime green or sky blue anywhere on the page (spot-check the color
   definitions).
3. Elbow joints are smooth — no blob artifacts.
4. All seven data points present and legible at 480x272.
5. NAN still renders "--"; page-select and dimming unchanged.
6. Photo of the physical panel showing the revised page; commit SHA on
   cabin-play origin.

## Constraints
- Display section only (plus color/font definitions). Do NOT touch sensors,
  wifi, api, ota, ble_client, the watchdog, or the Home Assistant imports.
- 480x272: primary numbers stay large and legible.
- Keep model/provider cost economical; no premium tier needed.
- Do not disturb the living-frame project files in the same repo.

## Proof
- Commit SHA on cabin-play origin with the revised lambda.
- Photo of the panel showing the revised LCARS page.
- ESPHome compile output showing zero errors.

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the photo, the
compile result, and notes on any behavior intentionally changed.
