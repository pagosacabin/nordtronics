---
task_id: "0126"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0126 — Apply the durable wake-gate fix (wake on non-empty inbox)

## Context

On 2026-10-07 your 13:15 tick skipped task 0124: the wake gate only compares git state (local vs origin), so a task filed locally on your machine is invisible to it — 0124 would have sat there forever. You unstuck it by faking the 'Juno push' git state, and offered the durable fix: wake on a non-empty `mailbox/inbox/` instead of the git diff (one condition in `mailbox-changed.sh`). Stephen gave the word to proceed with held-off items on 2026-10-07 ~15:17 MDT — this fix is one of them.

## Task

Apply the durable fix to `mailbox-changed.sh` on your machine: the wake condition becomes "mailbox/inbox/ is non-empty" rather than "git state differs from origin". Keep the rest of the tick flow unchanged.

## Success criteria

- A test task file placed locally in `mailbox/inbox/` (not pushed anywhere) is picked up by your next tick with no git-state faking. Remove the test file after the pickup proves the fix.
- After the fix, `mailbox-protocol-check.py` still reports PROTOCOL: MATCH (protocol_version 1.0.0).

## Constraints

- Local worker/skill change only. No repo branches, no CI runs, no commits to the nordtronics repo for this task's own work.
- Cost: use your cheapest model tier for this task. Do not use premium/pro models.

## Proof

This task type carries no branch or CI run (same as 0044/0053). Proof is durable local evidence: the changed condition in `mailbox-changed.sh` (file path + sha256 before/after), the tick transcript showing the locally-filed test task picked up without git faking, and the PROTOCOL: MATCH checker output.

## Reply format

Stage with front-matter proof listing the local artifacts above. Body: the exact condition changed, the test-task demonstration transcript, and confirmation the test file was removed afterward.
