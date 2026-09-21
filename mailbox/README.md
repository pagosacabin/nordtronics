# Mailbox protocol — global rules

The mailbox is a state machine. Task files move through directories; every
transition is a `git mv` + commit + push. Never copy a task file — move it.

## States

- `mailbox/inbox/` — new tasks waiting for pickup. Front-matter: `status: inbox`
- `mailbox/active/` — picked up, work in progress. `status: in_progress`
- `mailbox/staged/` — work done, awaiting verification. `status: staged`
- `mailbox/archive/` — verified complete. `status: verified`. Only Juno moves files here.

## Transitions

- Pickup: `inbox` → `active`. Set `status: in_progress`, increment `iteration` by 1, commit + push.
- Done: `active` → `staged`. Set `status: staged` and fill in `proof` (below), commit + push.
- Verified: `staged` → `archive`. Juno only, after independent verification.
- Rejected: `staged` → `active`. Juno attaches `notes` explaining what failed verification. `iteration` does NOT increment.

## Front-matter

Every task file starts with YAML front-matter. Only these fields:

- `task_id`: e.g. 0031
- `status`: inbox | in_progress | staged | verified | failed
- `iteration`: integer, starts at 0
- `proof`: pointers to durable evidence (branch tip SHA, Actions run URL). Required on `staged`.
- `notes`: freeform. Juno's verification feedback goes here on rejection.

The body stays freeform markdown — specs, context, reply format.

## Rules

1. Iteration cap: 5. `iteration` increments only on pickup (`inbox` → `active`). If a task reaches `iteration: 5` without being verified, move it to `staged` with `status: failed` and a failure summary in `notes`. Hard stop — no further automatic retries.
2. Proof bar: `staged` requires `proof` pointers, not pasted output. Pasted terminal text is a claim; Juno verifies the pointers independently (branch SHAs, CI run state, artifacts). A fabricated or placeholder pointer fails the task.
3. Replies live in the task file. There is no separate outbox — moving the file to `staged/` with `status: staged` IS the reply.
