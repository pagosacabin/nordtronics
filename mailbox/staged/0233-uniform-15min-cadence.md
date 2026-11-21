---
task_id: "0233"
status: staged
iteration: 1
proof: []
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
notes: |
  DONE 2026-11-20 19:25 MST (02:25 UTC, Saturday UTC -> off-peak) — nothing staged but this file;
  this is a host-only task, so proof is the cron listing below, not a branch.

  WHAT CHANGED (4 worker jobs, schedule only): every job that runs the mailbox pickup worker now
  fires every 15 minutes on its own day(s), seven days a week:
    c0be50a686c6 "Mailbox worker (DeepSeek) Mon-Thu"  */15 4-17,21,22 * * 1-4  ->  */15 * * * 1-4
    7aff6948c2c1 "Mailbox worker (DeepSeek) Fri"      15 3-23 * * 5           ->  */15 * * * 5
    8b1c9e1323c5 "Mailbox worker (DeepSeek) Sat"      15 * * * 6              ->  */15 * * * 6
    5c1532977f15 "Mailbox worker (DeepSeek) Sun"      15 0-17,21,22 * * 0     ->  */15 * * * 0
  I kept the four day-scoped jobs instead of consolidating them into one */15 job: consolidation
  would have meant pausing three jobs, and this task forbids touching pause/resume behavior. Each
  day is still covered by exactly one job, so the change adds no duplicate fires.

  WHY A UNIFORM */15 AND NOT AN OFF-PEAK-ONLY EXPRESSION: peak is defined in UTC (01:00-04:00 and
  06:00-10:00 UTC Mon-Fri) while cron here fires in local Mountain time, so no fixed local hour
  window is off-peak in both MST and MDT. Off-peak in MST is 03:00-18:00 and 21:00-23:00 local; in
  MDT it is 04:00-19:00 and 22:00-24:00 local. A uniform */15 with the unchanged peak guard
  vetoing peak ticks is the DST-robust form, and is the form the spec named as preferred. The
  cost of that choice: peak ticks still fire and are vetoed by the guard (a short agent turn per
  wake), and a tick whose mailbox is unchanged is skipped by the pre-run gate at $0 anyway.

  VERIFICATION (all performed this run, nothing assumed):
  1. Before/after diff of the full `hermes -p cronrunner cron list` (job id + Name + Schedule +
     Next run): exactly 8 changed lines, all of them the four worker Schedule/Next-run pairs above.
     Nothing else moved: 892982733bba "Upwork lead sweep (read-only) 5AM MDT" is still
     `0 5 * * *` (next 2026-11-21T05:00), and 0b32bc199a4e "Mailbox poll (Nvidia)" is still
     `[paused]` with `*/15 * * * *` and its stale 2026-09-24 18:45 next-run, untouched.
  2. next_run_at re-anchored correctly (the stale-past-next-run trap): Fri job next is
     2026-11-20T19:30-07:00, Sat 2026-11-21T00:00, Sun 2026-11-22T00:00, Mon-Thu 2026-11-23T00:00.
     None is in the past.
  3. Schedule simulated over a full week from the four expressions (minute-by-minute, cron
     dow 0=Sunday mapped to Python weekday): 672 fires/week, every day 96 fires at :00/:15/:30/:45,
     exactly one job per fire (no duplicates). Off-peak fires per local day in an MST week:
     Fri 84, Sat 96, Sun 80, Mon-Thu 68 each; a July MDT week gives the same totals (Fri 80,
     Sat 96, Sun 84, Mon-Thu 68), which is the DST robustness being claimed. Gaps longer than
     15 minutes occur only where a UTC peak block vetoes ticks (e.g. the 255-minute weekday
     gap across 00:00-04:00 UTC) — intended, not a cadence hole.
  4. Peak guard unchanged and still discriminating: neither script was edited this run
     (`mailbox-changed.sh` mtime 2026-11-17, `mailbox-protocol-check.py` mtime 2026-09-24;
     sha256 of mailbox-changed.sh = b063f463f7771861844a91cce0ae4595a1ff8ced0c3078c0475b31a044cdfa88).
     I loaded the real `peak_line()` from the checker and patched only its `datetime` to probe the
     rule: 2026-11-24T02:00Z (weekday) -> "PEAK: PEAK ... skipping"; 2026-11-24T12:00Z -> OFF-PEAK;
     2026-11-21T02:00Z (Saturday) -> OFF-PEAK; 2026-11-20T19:30Z (Friday) -> OFF-PEAK. So the guard
     still vetoes weekday UTC peak and still allows weekend hours in that window, i.e. the check is
     non-vacuous and the uniforms schedule relies on a live rule.
  5. This run's own pre-run digest agrees: "PEAK: OFF-PEAK 02:15 UTC -- safe to work." Note the
     local-Friday / UTC-Saturday split is exactly why the guard, not the schedule, is the authority.

  DEVIATIONS: none. No change to the worker's task-selection logic, drain behavior, pause/resume
  state, or the mailbox-changed gate; no other job's schedule touched; the Nvidia poller stays
  paused. The four jobs were kept rather than merged (see above). `cost` is `unknown` because this
  harness does not expose per-call DeepSeek usage to the agent — the figures are not estimated.

  RUN FACTS: PEAK OFF-PEAK 02:15 UTC, PROTOCOL MATCH 1.3.0; pickup commit ce1521f (iteration 1);
  active/ held 0213, 0216, 0230, all decision-blocked (nothing re-opened this run); inbox/ held
  0176 (superseded), 0229 (interactive per Stephen — not queued work), 0233 (this task).
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

## Reply — 2026-11-20 19:25 MST (task 0233)

**Status: staged.** All four mailbox-worker jobs now fire every 15 minutes, seven days a week,
during off-peak hours; the peak guard is untouched and remains the veto on peak ticks.

### Verbatim cron listing after the change (`hermes -p cronrunner cron list`)

```
  c0be50a686c6 [active]
    Name:      Mailbox worker (DeepSeek) Mon-Thu
    Schedule:  */15 * * * 1-4
    Next run:  2026-11-23T00:00:00-07:00

  5c1532977f15 [active]
    Name:      Mailbox worker (DeepSeek) Sun
    Schedule:  */15 * * * 0
    Next run:  2026-11-22T00:00:00-07:00

  7aff6948c2c1 [active]
    Name:      Mailbox worker (DeepSeek) Fri
    Schedule:  */15 * * * 5
    Next run:  2026-11-20T19:30:00-07:00

  8b1c9e1323c5 [active]
    Name:      Mailbox worker (DeepSeek) Sat
    Schedule:  */15 * * * 6
    Next run:  2026-11-21T00:00:00-07:00
```

Previous (for contrast): Mon-Thu `*/15 4-17,21,22 * * 1-4`, Fri `15 3-23 * * 5`,
Sat `15 * * * 6`, Sun `15 0-17,21,22 * * 0`.

### Peak guard and other jobs — unchanged

- `mailbox-changed.sh` and `mailbox-protocol-check.py` were not edited this run (mtimes 2026-11-17
  and 2026-09-24; sha256 of the gate script `b063f463…44cdfa88`). The guard's rule still vetoes
  weekday 01:00-04:00 / 06:00-10:00 UTC and still allows weekend hours in that window — probed this
  run by loading the real `peak_line()` and patching only its clock (four instants, verdicts as
  expected above).
- No other job's schedule changed: `892982733bba` Upwork lead sweep is still `0 5 * * *`
  (next 2026-11-21T05:00:00-07:00); `0b32bc199a4e` Mailbox poll (Nvidia) is still `[paused]`
  at `*/15 * * * *` with its old 2026-09-24 next-run.
- A full before/after diff of the listing shows exactly the four worker Schedule/Next-run pairs and
  nothing else.

### Why `*/15 ... * * <day>` rather than an off-peak-only hour window

Peak is defined in UTC; cron fires in local Mountain time. No single local hour window is off-peak
in both MST (off-peak 03:00-18:00, 21:00-23:00) and MDT (off-peak 04:00-19:00, 22:00-24:00), which
is the DST hazard this task's own note anticipates. A uniform `*/15` with the unchanged guard
vetoing peak ticks is the DST-robust form and the one the spec named as preferred; unchanged-mailbox
ticks are skipped by the pre-run gate at $0, so the extra peak wakes cost only the guard's veto turn.

### Measured outcome

Simulating the four expressions over a full week: 96 fires per local day at :00/:15/:30/:45, one job
per fire (no duplicates), and every off-peak slot covered — off-peak fires in an MST week:
Fri 84, Sat 96, Sun 80, Mon-Thu 68; in a July MDT week the same totals with Fri/Sun swapped
(Fri 80, Sun 84). Gaps longer than 15 minutes occur only across vetoed UTC peak blocks.

### Deviations

None. The four day-scoped jobs were kept rather than merged into one job (merging would have
required pausing three, which the task forbids); no change to task-selection logic, drain behavior,
pause/resume state, or the gate. `cost` is `unknown` — this harness does not expose per-call
DeepSeek usage; nothing was estimated.
