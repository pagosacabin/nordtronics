---
task_id: "0145"
protocol_version: 1.0.0
status: filed
iteration: 1
expect-reply-within: 6h
---

# 0145 — Guition: roll geometry back to the 0141 layout

## Context
Stephen's verdict on the 0144 geometry rework: "I like the other one better."
The 0141 layout is the one he wants on the glass. The color-pipeline artifacts
(blue/lime instead of the canonical palette) persist in the current build —
that investigation stays parked; this task is geometry only.

- 0141 geometry: cabin-play commit 2ee02d829e0f7a7dbe0e735a77b081f8fc3d2c82
  (the layout in mailbox/files/0143-panel-photo.jpg).
- 0144 geometry: cabin-play commit 8e5024b98d71f02e17d237d8462ef46ab26cb478
  (current on-panel build, "compiled on 2026-10-16 17:23:46 -0600").

## Task
Restore the BMS page display lambda's geometry to the 0141 state:
- The display lambda geometry in `guition-jc4827w543/jc4827w543.yaml` should
  match the 0141 revision (2ee02d8). Cleanest path: check out the lambda region
  from 2ee02d8, or reverse-apply 0144's lambda diff — your call, same result.
- Keep 0141's palette declarations, roboto_80 font block, and behaviors
  (NAN→"--", page-select, dimming, repaint) exactly as they are.
- tools/audit_layout.py (added in 0141, extended in 0144) may stay — it's
  tooling, not geometry. The render PNG should be regenerated from the
  restored lambda.

## Constraints
- Geometry revert only. No color/palette changes, no behavior changes.
- The restored lambda must compile clean under ESPHome 2026.9.1 with no new
  warnings, and audit_layout.py must pass on the restored arrangement.
- Push via SSH to pagosacabin/cabin-play as with 0140/0141/0144 (the gh.py
  token is nordtronics-scoped and 403s on cabin-play writes).
- Keep model/provider cost economical; no premium tier needed.

## Success criteria
1. The BMS page geometry matches the 0141 revision (prove with a diff of the
   lambda region against 2ee02d8 — expect no differences).
2. Compiles clean, audit checks pass.
3. Panel OTA-flashed; new "compiled on" timestamp reported.
4. Palette byte-identical to 0141 (unchanged by this task).

## Proof
- Commit SHA on cabin-play origin.
- Lambda-region diff against 2ee02d8 showing no differences.
- Render PNG regenerated from the restored lambda.
- New "compiled on" timestamp read back from the panel (Stephen photographs
  the glass — he's the eyes).

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the diff
result, the new build timestamp, and notes on anything unexpected.
