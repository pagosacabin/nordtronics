---
task_id: "0219"
status: in_progress
iteration: 1
proof: []
notes: ""
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-19T21:49:15+00:00"
lease_until: "2026-11-19T23:19:15+00:00"
---

# 0219 — Self-update to Hermes v0.23.2

## Assignment
- Hermes: every step below — this is your own harness on your own machine.
- Juno: verifies the staged reply (version strings, gateway health, cron list) and then releases 0220–0222.

expect-reply-within: 12h

## Context
Stephen directed this on 2026-11-19: Hermes self-updates (he has had you do it before), and three follow-on tasks depend on the update landing — 0220 (cron admission recovery), 0221 (fallback provider chain), 0222 (delegation.auto_resume). Your install is on v0.23.1; v0.23.2 has been the newest stable since 2026-11-16.

## Task
1. Record the current version (`hermes --version`).
2. Run the documented self-update (`hermes update`) to v0.23.2.
3. Verify after the update: `hermes --version` reports 0.23.2, the gateway is back up and responsive (your normal health check), and your cron jobs are still listed intact — at minimum the mailbox worker and the 5 AM Upwork sweep. List only; change nothing about them in this task.

## Success criteria
- Version output before and after, with the after output showing v0.23.2.
- Gateway healthy after the update.
- Cron job list present and unchanged.

## Constraints
- Model: your normal DeepSeek flash tier; no provider changes in this task.
- If the update fails, or the gateway does not come back healthy, park this task blocked with the exact failing output in `notes` — do not improvise a recovery beyond the documented update path. 0220–0222 wait on this task, so a clean block here is worth more than a forced fix.

## Proof
Host-only task: no branch, no CI run. `proof: []` with the version outputs and health check results in the reply body is the honest form for this one.

## Reply format
Status line first. Then: version before, version after, gateway health result, cron list confirmation, any deviations.
