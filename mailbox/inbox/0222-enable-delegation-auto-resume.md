---
task_id: "0222"
status: inbox
iteration: 0
proof: []
notes: ""
---

# 0222 — Enable delegation.auto_resume for subagents

## Assignment
- Hermes: every step below — your own config on your own machine.
- Juno: verifies the staged reply (config read-back and effective limits).

expect-reply-within: 24h

## Context
Juno's 2026-11-19 docs learning (official Subagent Delegation docs): every `delegate_task` child keeps a durable record in the profile's `state.db`. A child cut off by a gateway shutdown, crash, hard kill, or provider limit is resumable; with opt-in `delegation.auto_resume: true` (default false) the gateway continues AUTO_AT (rate limit with a reset time) and AUTO_ON_BOOT (shutdown/crash) children itself — at most 2 automatic attempts per child, a 360-minute deadline (`delegation.auto_resume_deadline_minutes`), and the child's worktree pinned up to `delegation.resumable_ttl_hours` (default 48). Stephen directed enabling it on 2026-11-19, accepting the docs' cost caveat: a resumed child spends tokens with nobody watching. Today a Newton run cut off by a DeepSeek rate limit or a gateway restart is lost work you re-dispatch from scratch.

## Task
1. Version guard: confirm `hermes --version` reports v0.23.2. If it does not, stop and report — 0219 lands first.
2. Set `delegation.auto_resume: true` in your config, using the documented config path/CLI for this install.
3. Read the setting back from the config and report the effective values alongside it: `auto_resume` itself, `delegation.auto_resume_deadline_minutes`, the max automatic attempts, and `delegation.resumable_ttl_hours` — as the machine reports them, defaults stated as defaults.
4. State whether the setting takes effect immediately or on the next gateway start, per the docs/config behavior on this install. Restart the gateway only if that is the documented requirement for the setting to take effect; if you restart it, confirm it comes back healthy.

## Success criteria
- Config read-back showing `delegation.auto_resume: true`.
- The effective limits reported (deadline, max attempts, worktree TTL).
- A plain statement of when the setting takes effect, and gateway health confirmed if a restart was needed.

## Constraints
- Model: your normal DeepSeek flash tier.
- Change only the `delegation.auto_resume` setting (plus reading its sibling values). No other delegation or config changes in this task.
- Do not manufacture a test child failure to prove resumption — the config change plus read-back is the deliverable; the first real rate-limit or restart will exercise it in production.

## Proof
Host-only task: `proof: []` with the config read-back in the reply body.

## Reply format
Status line first. Then: the config change as read back, the effective limits, when it takes effect, gateway health if restarted. Deviations, if any.
