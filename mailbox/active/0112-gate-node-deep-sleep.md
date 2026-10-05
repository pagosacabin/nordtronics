---
task_id: "0112"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 12h
proof: []
notes: |
  Filed by Juno, 2026-10-05 ~10:55 MDT. Stephen's decision: disable the
  node's deep sleep for the bench phase. Re-enabling it is the FINAL GATE
  before field deployment -- the sleep code stays intact, gated, not ripped
  out. 0111 proved the node healthy (720 s sleep is designed); the sleep
  makes bench verification painful (12-min cycles, USB drops between them).
---

# 0112 — Gate the node deep sleep off for the bench phase

## Context

0111 (verified, archived) proved the wildfire node board healthy: it boots
the 0108 firmware, finds the BME680 (`ROLE: node`), sends one LoRa packet,
then deep-sleeps 720 s, which drops it off USB. That sleep is correct for the
field power budget but makes bench verification miserable — every observation
waits on a 12-minute cycle and the USB serial vanishes between them.

Stephen's call: turn the sleep OFF for now so the node stays awake with USB
live. Turning it back ON is the final gate before the system is declared
field-ready. The base role never sleeps already (HARD RULE refusal in
enter_deep_sleep); this change is node-effective but lives in the shared
unified firmware.

## Task

1. Add a deep-sleep gate to the unified firmware (`firmware/wildfire-node-v1`):
   a boolean such as `deep_sleep_enabled`, default FALSE for the bench phase.
   When false, the node skips `enter_deep_sleep()` and instead waits
   `checkin_s` between sample/report cycles, staying awake with USB-serial
   live. When true, today's behavior is unchanged.
2. Do NOT delete the sleep code or the base's HARD RULE refusal — this is a
   gate, not a removal. Add a comment at the gate naming it as the final
   pre-deployment gate.
3. Bench-phase checkin interval: default `checkin_s` to 60 s (was 720 s).
   Both the interval and the sleep gate return to field values (720 s +
   sleep on) at the final gate. Note the change in the code comment.
4. Build on a branch through CI per the usual recipe; attach the artifact.
5. Flash the NODE only (MAC B0:A6:04:C5:75:4C, by-id path, never ttyACM0/1):
   the node is currently in its 720 s sleep cycle, awake ~4 s per cycle.
   Poll `/dev/serial/by-id/` for the node path and open it the instant it
   appears — opening asserts DTR/RTS, which on this board always enters ROM
   download mode (0111 §1). Flash app-only at 0x10000, then read back and
   verify byte-for-byte against the artifact (0109's recipe). Do NOT touch
   the base board (MAC 80:F1:B2:A7:47:EC) in any way. No NVS writes.

Cost: flash tier, small change task. Off-peak preferred; do not delay past
the reply window.

## Success criteria

- CI green on the branch; artifact attached.
- After flash, the node's by-id path is present CONTINUOUSLY for 10+ minutes
  (no more 720 s disappearances).
- Quoted serial lines showing repeated sample/report cycles with LoRa TX and
  NO `sleep:` line.
- The base board received no write and no reset. No NVS writes anywhere.

## Constraints

- One deliverable: the gated firmware, built and flashed to the node.
- No repo commits outside the task branch. No base flash. No NVS writes.
- If the node cannot be caught in its awake window after reasonable polling,
  stop and report — do not invent a riskier flash method.

## Proof

- Branch SHA on origin, Actions run URL, artifact ID.
- `ls /dev/serial/by-id/` showing the node stable, with timestamps 10+ min
  apart.
- Quoted serial capture of two consecutive report cycles.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
