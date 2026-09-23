# Mailbox protocol — global rules

protocol_version: 1.0.0

## Protocol versioning

Semver. MAJOR = states or transitions change. MINOR = new rule or field.
PATCH = clarification only. Every version bump ships as a mailbox task written
in the previous format, carrying the exact skill-patch text to apply verbatim.

## Changelog

- 1.0.0 — 2026-09-23: baseline. Existing rules versioned; no behavior change.

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

- `task_id`: e.g. `"0031"` — quote it. Unquoted `0031` parses as octal in YAML 1.1.
- `status`: inbox | in_progress | staged | verified | failed
- `iteration`: integer, starts at 0
- `proof`: pointers to durable evidence (branch tip SHA, Actions run URL). Required on `staged`.
- `notes`: freeform. Juno's verification feedback goes here on rejection.

The body stays freeform markdown — specs, context, reply format.

## Proof format

Write `proof` as a list so the verifier can check each pointer. Canonical form:

```yaml
proof:
  - branch: android-toolchain-setup
    sha: 92a5b2b44bf30a0fde9eb4dfc72ca25c8692d299
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35583228091
```

- `branch` + `sha`: the branch must exist on origin and its tip must equal `sha`.
- `run`: a real Actions run URL. The verifier checks its status/conclusion and
  that its head SHA matches the claimed `sha` and the current branch tip.
- A green run with matching pointers verifies the task. A red run, a wrong SHA,
  or a run whose head no longer matches the branch tip fails it.

Build tasks (the deliverable is a CI-built artifact — APK, firmware, image):
proof additionally requires a build-green ntfy notification. Once the run is
green, publish to the ntfy build topic named in the task spec
(`https://ntfy.sh`) and include the publish receipt (topic, timestamp) in the
reply. A green build Stephen never hears about is a failed handoff.
(Clarified 2026-09-23: the 0045 companion-app build went green and staged with
full proof, but no ntfy notification went out because the spec never asked for one.)

The verifier is `~/workspace/hermes-tools/verify-staged.py` on Juno's side —
it checks every pointer against the live GitHub API. Pasted terminal output
is not proof and is ignored.

## Rules

1. Iteration cap: 5. `iteration` increments only on pickup (`inbox` → `active`). If a task reaches `iteration: 5` without being verified, move it to `staged` with `status: failed` and a failure summary in `notes`. Hard stop — no further automatic retries.
2. Proof bar: `staged` requires `proof` pointers, not pasted output. Pasted terminal text is a claim; Juno verifies the pointers independently (branch SHAs, CI run state, artifacts). A fabricated or placeholder pointer fails the task.
3. Replies live in the task file. There is no separate outbox — moving the file to `staged/` with `status: staged` IS the reply.

## Task spec format (how Juno writes tasks)

Every task body uses these sections, in this order:

1. **Context** — what exists and what has already been verified. Front-load exact
   branch names, file paths, and prior results.
2. **Task** — one concrete deliverable. One task = one artifact.
3. **Success criteria** — falsifiable conditions that must be true when done.
4. **Constraints** — security, branch, hardware, and scope boundaries. State
   positively ("include X", "push only to branch Y") instead of long negative lists.
5. **Proof** — the exact pointers Juno will verify (branch + SHA, run URL).
   Pasted terminal output is a claim, not proof.
6. **Reply format** — the exact structured fields Hermes fills in when staging.

From the Hermes Agent framework research (2026-09-21): positive instructions beat
negative lists, structured sections beat walls of prose, exact names beat
descriptions, one deliverable per task. Keep tasks small enough to interrupt drift
early.

Never assume Hermes's local installation has any framework feature configured
(skills, memory, profiles, gateways) — only what he has proven on his own machine.
