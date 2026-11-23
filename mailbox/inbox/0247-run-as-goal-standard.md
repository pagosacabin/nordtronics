---
task_id: "0247"
title: Make run_as_goal a standard — protocol bump, worker behaviour, live host-only proof
status: pending
iteration: 1
expect-reply-within: 24h
proof: []
notes: Filed by Juno 2026-11-23. Stephen approved the protocol-level standard from 0246 (option A) in chat 2026-11-23 11:54 MST ("Yes"). The 24h window spans the proof task's lifecycle across worker ticks.
---

## Assignment

- **Hermes:** all of it — the README edit, the worker-skill change, the Copilot step, filing and reporting the proof task.
- **Copilot:** peer review of the README diff (GitHub-bound), driven by Hermes via Herdr. Reviewer, not the proof.
- **Newton:** not assigned.
- **Juno:** verifies independently — reads the README back on origin, checks 0248's lifecycle in the mailbox, and closes 0247 on the staged reply.

## Context

0246 (archived, verified) established that goal execution through the standard pickup is not available natively on Hermes v0.23.2: the mailbox worker is a cron job with no goal mode, and the kanban judge loop is unreachable from the pickup and carries no per-turn gate. The gap is a protocol field plus worker wording — not a version update. 0246's reply (section 5) drafted the standard and the README wording; the README was deliberately not edited there, because protocol changes are Stephen's call.

**Stephen has now made that call: option A is approved** (chat, 2026-11-23 11:54 MST). Option B (engine-level gates inside `run_kanban_goal_loop`) is NOT approved and stays a recorded alternative.

## Task

1. **README protocol bump.** Add the `run_as_goal` field to `mailbox/README.md`'s front-matter section, using the wording drafted in 0246's reply, adjusted only for fit with the surrounding text. Bump the protocol version per the README's own convention. Stephen's approval is the authority for this edit — cite it in the commit message.
2. **Worker behaviour.** Update the cronrunner handoff-mailbox skill (and the worker prompt wording, if that is where it lives) so that on claiming a task with `run_as_goal: true` the worker:
   - creates a kanban card with `--goal` and `--goal-max-turns` from `goal.max_turns`, adding `--completion-contract <goal.github_contract>` when that field is set, with `goal.contract` as the card body;
   - waits for the run to finish;
   - runs every command in `goal.gates` itself; on a non-zero exit, dispatches a continuation card quoting the gate's output tail, up to `max_turns`; on budget exhaustion, parks the task in `active/` with the last gate output;
   - stages the mailbox reply only when the card completed AND every gate exits 0.
   Quote the changed skill lines in the reply.
3. **Copilot peer review** of the README diff; address findings; then push. `git fetch` and read origin before any push.
4. **Live proof.** After steps 1–3 land, file task **0248** in the inbox as a normal task carrying `run_as_goal: true`: a small host-only scratch job whose initial state FAILS its gate — the 0246 section-4 shape (a two-file scratch job, `python3 -m unittest discover` as the gate, the subject or test deliberately broken), with a contract stating the outcome (tests pass) and the boundary (scratch only). Let the standard cycle claim it untouched by hand. Report 0248's full lifecycle in this task's reply: claimed by the tick, goal card run, the gate's failure fed back, the fix, the gate passing, the staged reply. If any link in that chain does not happen, report exactly where it stopped — a failed proof is a finding, not a licence to hand-run it and call it proved.

## Success criteria

- `mailbox/README.md` on origin carries the `run_as_goal` field and the behaviour paragraph (Juno reads it back).
- The cronrunner skill carries the claim-time behaviour, quoted in the reply.
- 0248's lifecycle ran through the untouched standard cycle with gate evidence quoted (fail, then pass) — or the reply names the exact link that failed.
- Copilot's review of the README diff is dispositioned in the reply.

## Constraints

- No engine changes. No v0.23.3 update, or any update. Option B stays parked.
- Tasks WITHOUT `run_as_goal` follow exactly the path they follow today — the default behaviour does not change.
- The proof job is host-only scratch: no hardware, no flashing, nothing changed outside the scratch directory, the mailbox, and the files named above.
- Secrets never in the task files, the proof task, or the reply.
- Model cost: flash tier is fine for this work.

## Proof

- The README as read back from origin after the push.
- The quoted skill lines implementing the claim-time behaviour.
- 0248's mailbox lifecycle (inbox → active → staged) plus the quoted gate outputs.

## Reply format

Status line first. Then per step: what changed, with pointers. Then the proof lifecycle in order. Deviations and failed links declared plainly.
