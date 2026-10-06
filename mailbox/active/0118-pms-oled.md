---
task_id: "0118"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP (iteration 0 -> 1) by the interactive session at Stephen's request,
  2026-10-06 13:50 MDT. The four cron worker jobs (c0be50a686c6, 7aff6948c2c1,
  8b1c9e1323c5, 5c1532977f15) are PAUSED for the duration so that two sessions
  never hold this repo at once; they are resumed once this task is staged.
  Base: hermes/0117-pms-live-poll @ a37e1e71fe6f6a0bd6fe79466865d26419e9040c --
  the revision this task names as the base. Work branch: hermes/0118-pms-oled.
  Scope: display-only addition in oled_render_node(); no change to the PMS read
  path, the 1 s poll, the 60 s check-in, deep-sleep gating or config. Model tier:
  DeepSeek Flash.
---

# 0118 — Show live PM on the node OLED (node only)

## Context

0117 (archived) added a 1 Hz non-blocking PMS live poll printing
`pms-live: pm1/pm25/pm10` to serial. Stephen wants the same live numbers on
the node's OLED — glance at the screen, breathe on the sensor, watch the
numbers move, no laptop needed. Base branch: `hermes/0117-pms-live-poll`;
new branch `hermes/0118-pms-oled`.

## Task

In the node's 1-second OLED render block, add one line showing the latest live
PMS values from the 1 Hz poll (pm1, pm2.5, pm10 — fit all three compactly,
e.g. `PM 9/9/23`). The line must update at the same ~1 Hz cadence as the
`pms-live:` serial lines, from the same live values.

## Success criteria

1. Branch `hermes/0118-pms-oled` builds green in CI from the 0117 tree.
2. The NODE (MAC `B0:A6:04:C5:75:4C`, by-id path) — and only the node — is
   flashed; bench discipline holds (partition-table read first, sha256 on the
   read-back matches the built image).
3. The node OLED shows live PM values updating ~1/sec. Proof: the code change
   quoted plus Stephen's bench confirmation (he will look at the screen) —
   state in the reply that visual confirmation is Stephen's step.

## Constraints

- NODE ONLY. Do not touch the base.
- Display only: no changes to the PMS read path, the 1 s poll, the 60 s
  checkin, deep-sleep gating, or config. No merge, no tag.
- Keep the existing OLED lines (id/chk/seq/vbat) — add, don't replace.
- Keep model cost on DeepSeek Flash. State the tier in the reply.

## Proof

- Branch SHA on origin + CI run URL + artifact.
- Flash read-back sha256 matching the built image.
- Quoted code diff of the OLED line addition.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, first line of notes confirms the OLED shows live PM at ~1 Hz,
`proof` block with the pointers above.
