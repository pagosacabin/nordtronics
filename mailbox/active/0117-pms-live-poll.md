---
task_id: "0117"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-06 17:15 UTC
  (11:15 MDT). Off-peak (PEAK: OFF-PEAK 17:15 UTC). The filed front-matter had
  no `iteration` field (protocol field); it was added at pickup and the pickup
  increment applied literally, so this task reads iteration: 1.
  Predecessors 0097/0098/0106/0107/0109/0110 remain decision-blocked/parked in
  active/ and were NOT touched (handoff-mailbox rule 6; 0111 explicitly says
  "Do NOT touch 0110").
---

# 0117 — PMS 1-second live poll (node only, bench diagnostic)

## Context

0115 (archived) proved the PMS5003 streams valid frames with all-zero fields
(rx_avail_before=256 saturated, ok=1, pm1/pm25/pm10 = 0/0.0/0) and added a
per-checkin diagnostic line. Stephen wants live numbers on the serial — the
60 s checkin cadence can't show the sensor reacting to a stimulus (e.g. a
breath puff at the inlet). This task adds a 1-second live poll. Base branch:
`hermes/0115-pms-uart-diag` (keeps the per-checkin line); new branch
`hermes/0117-pms-live-poll`.

## Task

In the node `loop()` (not in `node_checkin()`), poll the PMS5003 once per
second and print one line per successfully parsed frame:

`pms-live: pm1=<u> pm25=<f> pm10=<u>`

## Success criteria

1. Branch `hermes/0117-pms-live-poll` builds green in CI from the 0115 tree.
2. The NODE (MAC `B0:A6:04:C5:75:4C`, by-id path) — and only the node — is
   flashed; bench discipline holds (partition-table read first, sha256 on the
   read-back matches the built image).
3. Serial shows approximately one `pms-live:` line per second with live values,
   AND the 60 s checkin still fires on schedule (backend keeps receiving node
   "0" readings) — the live poll must not break or delay checkins.

## Constraints

- NODE ONLY. Do not touch the base.
- NON-BLOCKING: the live poll must not stall `loop()`. Cap the per-poll read
  window short (well under the 1 s period — e.g. assemble from already-buffered
  bytes, bail fast if no header appears). The existing 1500 ms blocking scan in
  `read_pms25()` is for the checkin path; do not call that blindly every
  second.
- Keep the 0115 per-checkin `pms:` line as is — do not remove it.
- Diagnostic only: no deep-sleep changes, no config changes, no merge, no tag.
- Keep model cost on DeepSeek Flash. State the tier in the reply.

## Proof

- Branch SHA on origin + CI run URL + artifact.
- Flash read-back sha256 matching the built image.
- Pasted serial excerpt showing at least five consecutive `pms-live:` lines
  (~1 s apart) plus one checkin `tx:` line, proving both paths coexist.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, first line of notes states the live poll is running at ~1 Hz
and checkins are unaffected, `proof` block with the pointers and pasted serial.
