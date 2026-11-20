---
task_id: "0233"
status: pending
iteration: 0
proof: []
notes: "Filed by Juno 2026-11-20 on Stephen's direct instruction: the mailbox worker runs a 15-minute cadence during off-peak hours, period — all seven days. Today the cadence is Mon-Thu only; Fri/Sat/Sun are hourly at :15 (per Hermes's own schedule table, 2026-11-20). This task removes the day-of-week distinction."
---

# 0233 — Uniform 15-minute worker cadence during off-peak hours, all days

## Assignment
- Hermes: every step below — your own cron config on your own machine.
- Stephen: no input; this is his instruction, relayed by Juno.
- Juno: verifies the staged reply against the reported schedule.

expect-reply-within: 24h

## Context
Your current worker schedule (your own table, 2026-11-20): Mon–Thu every 15 minutes in off-peak hours; Fri, Sat, Sun hourly at :15. Stephen's ruling today: the 15-minute pickup exists for throughput and applies during off-peak hours **period** — the day of the week does not matter. The DeepSeek peak guard stays exactly as it is: it remains the authority that vetoes peak ticks (peak 19:00–22:00 and 00:00–04:00 MT), so a uniform schedule plus the unchanged guard produces a 15-minute cadence in off-peak hours and silence during peak.

## Task
1. Change the schedule of every cron job that runs the mailbox pickup worker so the cadence is every 15 minutes across all off-peak hours, seven days a week. How the expression achieves that on your cron system is your call (a uniform */15 with the peak guard vetoing peak ticks is acceptable and preferred for DST robustness) — the outcome is what is specified, not the expression.
2. Do not change the peak guard, its windows, or its behavior.
3. Do not change any other job's schedule (the paused-by-design Nvidia poller stays paused; the Upwork sweep and daily-learning jobs keep their timing).
4. Report the full resulting schedule for the worker job(s), verbatim from your cron listing, plus a one-line confirmation that no other job's schedule changed.

## Success criteria
- The worker job(s) tick every 15 minutes during off-peak hours on all seven days.
- The peak guard is unchanged.
- The reply contains the verbatim post-change schedule listing for the worker job(s).

## Constraints
- Schedule only: no changes to the worker's task-selection logic, drain behavior, pause/resume behavior, or the mailbox-changed gate.
- Model for this task: your normal DeepSeek flash tier.

## Proof
Host-only task: `proof: []` with the cron listing in the reply body.

## Reply format
Status line first. Then: the verbatim schedule listing for the worker job(s), the confirmation that the peak guard and all other jobs are untouched, and any deviations.
