---
task_id: "0246"
title: Goal mode through the standard pickup — find the mechanism, prove it host-only
status: pending
iteration: 1
expect-reply-within: 24h
proof: []
notes: Filed by Juno 2026-11-23 at Stephen's direction. Research-shaped like 0235, hence the 24h window. Companion to 0245, which runs as a hand-driven interactive goal in the meantime.
---

## Assignment

- **Hermes:** all of it — the research on his own install, the host-only proof if a mechanism exists, the staged reply.
- **Copilot:** not assigned — no GitHub-bound code change is expected. If the work turns into one, stop and say so.
- **Newton:** not assigned.
- **Juno:** verifies the reply against the claims that can be checked from the mailbox side (the proof task's lifecycle, quoted outputs) and puts the proposed standard to Stephen.

## Context

Task 0235 (Nov 21) established the split: the interactive `/goal` route in Hermes v0.23.2 runs a written contract, a judge loop, and a real gate — while the kanban `--goal` route the mailbox worker would use is **judge-only, with no gate hook**.

This morning's 0244 run showed the cost of that split: in interactive mode every leg (review, fix pass, second pass) waited for Stephen to say "go," because nothing chained them. 0245 is running as a hand-driven interactive goal to cover today's bench work. Stephen's direction now: **find out how a goal-mode run happens through the standard pickup** — a task filed in the inbox, claimed by the normal worker cycle, executed as a goal with its contract and gate — so "run as a goal" can become a standard marking in task specs instead of an interactive detour.

Related open thread: the Nov 23 daily learning found Hermes has shell hooks that can block tool calls fail-closed and inject context, documented as of v0.23.3, with whether they fire inside cron worker runs unstated. The v0.23.3 update itself is **parked** at Stephen's word and stays parked.

## Task

1. **Map the standard pickup as it actually runs on this machine.** The worker jobs, the cronrunner profile, the kanban commands the cycle invokes, and where a goal contract and gate would have to attach. Evidence from the machine — help output, the profile skill, config files — not documentation alone.
2. **Pin down the kanban `--goal` route in this install.** What it accepts, where the judge gets its criteria, whether a contract can be supplied, and whether the "no gate hook" finding from 0235 is the whole story or whether a config/flag on this version adds one. The installed version's own help and, if needed, its source on this machine are the authority.
3. **Check the hook systems in this install.** Can a shell hook serve as the gate on the worker route (verify or block between turns)? If hooks are not present or do not fire in worker runs on v0.23.2, say so plainly. "Impossible without the parked update" is a valid finding — report it, do not install anything.
4. **Proof, host-only, if a mechanism exists.** One small multi-step host-only job (for example a two-file edit with a build or test gate), filed as a normal inbox task carrying the proposed run-as-goal marking, picked up by the standard worker cycle **untouched by hand**, executed as a goal, with the gate demonstrably firing — a deliberately failing intermediate state caught, or the gate's pass logged. If no mechanism exists, skip the proof and say why.
5. **Recommend the standard.** The exact task-file marking (front-matter field name and values), any worker-side change it needs, and what Juno writes in future specs to invoke it. If the recommendation changes the mailbox protocol, draft the README wording in the reply but do not edit the README — protocol changes are Stephen's call.

## Success criteria

- The staged reply states, with machine-level evidence quoted, whether standard-pickup goal execution is possible on this install today.
- If possible: the host-only proof ran through the untouched standard cycle, and the reply quotes the gate firing.
- If not possible: the reply names the precise blocker (route limitation, version, missing hook) and the smallest change that would unblock it.
- Either way: a concrete proposed standard — the field, where it lives, what the worker does with it.

## Constraints

- Host-only proof. No hardware, no flashing.
- Do not install the v0.23.3 update, or any update.
- Do not change the live worker jobs' standing behaviour beyond the single proof pickup. Structural changes are recommendations for Stephen, not changes.
- Do not edit `mailbox/README.md`.
- Secrets never in the task file, the proof task, or the reply.
- Model cost: flash tier is fine; if a step genuinely needs a different tier, say so in the reply rather than switching silently.

## Proof

- The staged reply with quoted help/config/source evidence for each claim.
- The proof task's own mailbox lifecycle (filed, claimed by the standard cycle, staged) visible to Juno's checks, if the proof runs.

## Reply format

Status line first. Then the findings in task order, each with its evidence. Then the proposed standard. Blockers and deviations declared plainly.
