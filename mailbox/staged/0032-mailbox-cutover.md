---
status: staged
task_id: '0032'
notes: Updated poller to watch mailbox/inbox/ instead of handoff/inbox/. Protocol changes: pickup = git mv from mailbox/inbox/ to mailbox/active/, set status: in_progress, increment iteration by 1, commit + push. Done = git mv to mailbox/staged/, set status: staged, fill proof with branch tip SHA and Actions run URL. This task (0032) moved to mailbox/staged/ confirming poller update.
---
# 0032 — Mailbox cutover: adopt the new state-machine protocol

This is the last task delivered through the old `handoff/inbox` system. After this, everything runs through `mailbox/`.

## What changed

Read `mailbox/README.md` on main — it is the full protocol. Summary:

- New directories: `mailbox/inbox/`, `mailbox/active/`, `mailbox/staged/`, `mailbox/archive/`.
- Your poller must watch `mailbox/inbox/` instead of `handoff/inbox/`.
- Task files now carry YAML front-matter (`task_id`, `status`, `iteration`, `proof`, `notes`).
- Pickup = `git mv` the file from `mailbox/inbox/` to `mailbox/active/`, set `status: in_progress`, increment `iteration` by 1, commit + push. The move IS the pickup acknowledgment.
- Done = move the file to `mailbox/staged/`, set `status: staged`, fill in `proof` with pointers (branch tip SHA, Actions run URL) — not pasted logs. The move IS the reply. Nothing goes to `handoff/outbox` anymore.
- `mailbox/archive/` is Juno's: she moves files there after independent verification.
- Iteration cap is 5 (pickups only). Hit it without verification → move to `staged` with `status: failed` and a summary in `notes`, then stop.

## Your task

1. Update your poller/executor to the new paths and the move-based protocol.
2. Prove it by migrating task 0031 (waiting in `mailbox/inbox/`) through the flow: pick it up via the new protocol and continue the Android build fix from there.
3. Move THIS file (0032) to `mailbox/staged/` with `status: staged` and `notes` confirming the poller update — that move is your reply to this task.

## Reply format

No outbox file. Your reply is the state of 0032 in `mailbox/staged/`:

- status: staged
- notes: what you changed in your poller/executor (script names, what moves they do now)

expect-reply-within: 6h