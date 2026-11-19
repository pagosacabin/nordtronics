---
task_id: "0220"
protocol_version: 1.3.0
status: staged
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-19T22:16:18+00:00"
lease_until: "2026-11-19T23:46:18+00:00"
expect-reply-within: 24h
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
proof: []
notes: |
  STAGED — host-only, read-only inspection. `proof: []` (no branch, no CI run;
  nothing was changed on the host). Run 2026-11-19 15:16-15:30 MST (22:16-22:30
  UTC), off-peak (PEAK: OFF-PEAK 22:16 UTC), PROTOCOL MATCH 1.3.0. Cost fields
  are `unknown` because this harness exposes no per-call DeepSeek usage to the
  cron session (not estimated).

  TASK CHOICE. 0219 was staged first in this same run (commit 3a48d0c), which is
  this task's version guard; `hermes --version` then read v0.23.2, so 0220
  proceeded. Active/ below 0220 untouched: 0142 and 0187 `superseded`; 0159,
  0209, 0213, 0214, 0216 decision-blocked.

  WHAT WAS READ (all read-only; no job edited, no run interrupted, nothing
  restarted). The cron `runs` surface is `hermes -p cronrunner cron runs
  [job_id] [--limit]`, which reads the durable `executions` table in
  ~/.hermes/profiles/cronrunner/cron/executions.db. `hermes cron incidents`
  reads `cron_incidents` in the same file. Neither was opened for writing; the
  DB reads used `mode=ro`.

  VERDICT IN ONE LINE. Admission recovery IS observable from the CLI on this
  install — but through the execution ledger and the incident journal, not
  through any admission-specific command (there is no `hermes cron admissions`
  subcommand; the per-fire admission journals in `cron/admissions/` are
  filesystem-only).

  THE FOUR OBSERVABLE SURFACES
   1. `cron runs` — one row per attempt, with a terminal state and an error
      string. States actually present in this profile's ledger (1001 rows):
      completed 940, failed 57, missed 3, running 1. The schema also allows
      claimed/unknown/cancelled; none have occurred here.
   2. `cron runs` detail lines — 'interrupted' (process died mid-run),
      'missed ×N (collapsed into one catch-up run)', 'missed ×1 (skipped,
      catch-up off; next run …)'.
   3. `cron incidents` — durable incidents with lifecycle state (resolved),
      failure type (agent/auth/delivery/unknown), first/last seen, error and
      output path; 33 rows in this profile, all `resolved`.
   4. `cron status` / `cron doctor` — scheduler host + PID, ticker heartbeat,
      and late-fire/catch-up reporting.

  THE RECONCILIATION FACTS, PER JOB
   - Mailbox worker c0be50a686c6: 507 completed, 54 failed, 1 missed, 1 running.
     Three of the failures are the exit-crash class and each is booked exactly
     once, never re-run for the same fire: 2026-10-30T12:15 (interrupted at
     12:18:52, job 7aff6948c2c1), 2026-11-16T17:15 (interrupted 17:29:09),
     2026-11-18T13:00 (interrupted 13:33:50, progress heartbeat 13:14:08). In
     each case the next ledger row is a later scheduled slot, not a replay of
     the interrupted instant — so no second admission was created for a fire
     that had one.
   - The 2026-11-16 machine-down episode is the clearest reconciliation
     evidence: the scheduler wrote a `source=scheduler` row per affected job at
     07:04:30 (the moment the gateway came back) recording the decision instead
     of dropping it silently — `missed ×2 (collapsed into one catch-up run)` for
     the worker, `missed ×1 (collapsed into one catch-up run)` for the Sunday
     worker, and `missed ×1 (skipped, catch-up off; next run …)` for the Upwork
     sweep, whose catch-up is off. Each is paired with exactly one builtin row
     for the same `scheduled_instant`: one catch-up execution, not one per
     missed slot.
   - Upwork sweep 892982733bba: 40 completed, 2 failed (both the 2026-10-15
     HTTP 401 auth episode, incident 892982_4c544b742261 resolved), 1 missed as
     above. No interrupted run in its history.
   - The fire identity is on disk: `cron/admissions/<sha256(job_id,request_id)>.json`
     holds `params.request_id` (the same id `cron runs` prints) and a
     `receipt {session_id, admission_id}`, and the ledger carries
     `handoff_pending` (0 on every row here), `delivery_outcome` ('suppressed'
     997, null 4) and `progress_at` (heartbeat, present on 117 completed, 31
     failed, 1 running).

  HONEST LIMITS
   - The mailbox worker is mid-fire as this is written: row
     219ab6e29fc64c7aa679a4b80ca002a7 status=running at 15:15:01, which is this
     run. It is evidence that the ledger tracks live fires, not evidence of
     recovery.
   - Two `scheduled_instant` values appear twice (c0be50a686c6 at 2026-11-16T11:15Z,
     5c1532977f15 at 2026-11-16T04:15Z). Both are the intended missed+catch-up
     pair above, not a duplicate admission; no other instant is duplicated.
   - Slots 13:15 and 13:30 on 2026-11-18 have no ledger row at all while the
     13:00 run was still held open; the ledger records the interruption once, at
     the instant that was admitted. I am reporting that as observed rather than
     as a defect — deciding whether a held-open slot should also be booked as
     `missed` is not this task's call.
   - Operational note, not a deviation: today's 09:00-14:30 MST failures (31
     rows, all 'Hermes can't reach the model provider') are a provider/network
     outage, tracked as incident c0be50_eea137d5c0a5 (first seen 07:50:10,
     last seen 14:35:12, now resolved). Recovery after it is what allowed this
     run to happen at all.
---

# 0220 — Verify cron admission recovery on v0.23.2

## Assignment
- Hermes: every step below — read-only inspection of your own cron system.
- Juno: verifies the staged reply against the docs description of admission recovery.

expect-reply-within: 24h

## Context
Juno's 2026-11-17 docs learning (official Cron docs, "Admission recovery"): on the current release, an interrupted cron run is reconciled, not re-run or silently lost — agent-backed runs keep their fire identity until output and a durable delivery handoff are recorded; the scheduler reconciles unfinished receipts on every tick, including after a gateway/laptop restart; a run whose terminal result is already known is delivered without running the agent again; an interrupted execution stays paused for operator review; recovery never creates a second admission for the same fire. This matters because your unattended fleet — the mailbox worker and the 5 AM Upwork sweep — runs on a laptop that sleeps, reboots, and loses the gateway.

## Task
1. Version guard: confirm `hermes --version` reports v0.23.2. If it does not, stop and report — 0219 lands first.
2. Inspect what `hermes cron runs` (and any admission/ledger view the CLI exposes) shows for the mailbox-worker and Upwork-sweep jobs: are fire receipts / reconciliation states visible? Does the history show any run that was interrupted and reconciled rather than re-run?
3. Report per job: what the runs ledger shows, and whether admission recovery is observable from the CLI on this install.

## Success criteria
- A per-job statement of what the ledger shows, with the raw CLI output that supports it.
- A plain verdict: admission recovery observable / not observable from the CLI, per job or overall.

## Constraints
- Read-only. Do NOT interrupt a live run, kill the gateway, or reboot anything to manufacture a test case — evidence from the existing history and the CLI's own reporting is the deliverable.
- Model: your normal DeepSeek flash tier.
- If the CLI exposes no surface for this, say so plainly in the reply — that is a valid result, not a failure.

## Proof
Host-only task: `proof: []` with the CLI outputs in the reply body.

## Reply format
Status line first. Then per job: ledger findings and raw output. Then the overall verdict. Deviations, if any.

---

## Reply — 0220 staged

**Status: DONE — admission recovery is observable from the CLI on v0.23.2**, through the durable execution ledger (`hermes cron runs`) and the incident journal (`hermes cron incidents`). There is no admission-specific command; the per-fire journals are on disk only.

### 1. Version guard
```
$ hermes --version
Hermes Agent v0.23.2 (2026.9.24) · stable · local ae2a81ccc00e
Install directory: /home/astroboy/.hermes/hermes-agent
Up to date
```
0219 landed in this same run (commit `3a48d0c`), gateway PID 152866 on revision ae2a81cc. Guard passed.

### 2. Mailbox worker — `c0be50a686c6`

Ledger totals in `~/.hermes/profiles/cronrunner/cron/executions.db` (1001 rows, all jobs): completed 940, failed 57, missed 3, running 1.

```
$ hermes -p cronrunner cron runs c0be50a686c6 --limit 25
219ab6e29fc64c7aa679a4b80ca002a7  running    job=c0be50a686c6  source=builtin  2026-11-19T15:15:01.737036-07:00
c58ec9c5256c46aa843f0d479d7e9c07  completed  job=c0be50a686c6  source=builtin  2026-11-19T14:45:01.210837-07:00
ef3c2a1867cb40b3a5663efc45bf80cd  failed     job=c0be50a686c6  source=builtin  2026-11-19T14:30:01.968275-07:00
    RuntimeError: Hermes can't reach the model provider. You may be offline. …
```

Reconciliation rows for this job, from the full ledger:
```
2588ff5187234a0f9b71ac0e9e754a54  failed  job=c0be50a686c6  2026-11-18T13:00:02-07:00
    interrupted
81fc6f65862d42c38c1fac3726f85619  failed  job=c0be50a686c6  2026-11-16T17:15:01-07:00
    interrupted
cdb6f445ff7e4d97bedb9c84f4eaf75d  missed  job=c0be50a686c6  source=scheduler  2026-11-16T07:04:30-07:00
    missed ×2 (collapsed into one catch-up run)
```
Each `interrupted` fire is booked once, with the time it actually died (`finished_at` 13:33:50 for the 13:00 fire, progress heartbeat recorded at 13:14:08), and the next row for that job is the next scheduled slot (13:45) — not a replay of the interrupted instant. The missed episode is booked as a `source=scheduler` row that states the decision, paired with exactly one `builtin` catch-up row carrying the same `scheduled_instant`.

### 3. Upwork sweep — `892982733bba`

```
$ hermes -p cronrunner cron runs 892982733bba --limit 8
c60c70c8e6b44e038e7424289ca01ed8  completed  job=892982733bba  source=builtin  2026-11-19T05:00:01-07:00
8d281d1880cb403297ccea6c0366f918  completed  job=892982733bba  source=builtin  2026-11-18T05:00:02-07:00
504805a5bd07428cb59ca581338e65b3  completed  job=892982733bba  source=builtin  2026-11-17T05:00:02-07:00
21474a5927c14ab69ac6a42e2be4396e  missed     job=892982733bba  source=scheduler  2026-11-16T07:04:30-07:00
    missed ×1 (skipped, catch-up off; next run 2026-11-17T05:00:00-07:00)
```
History: 40 completed, 2 failed (the 2026-10-15 HTTP 401 auth episode), 1 missed — the 2026-11-16 machine-down morning, where the scheduler recorded "skipped, catch-up off" instead of running late. No interrupted run in this job's history. This is the cleanest demonstration that a missed fire is reconciled and recorded rather than silently lost or blindly re-run.

### 4. Incident journal

```
$ hermes -p cronrunner cron incidents
  c0be50_eea137d5c0a5  resolved   Job: c0be50a686c6   Type: agent
    First seen: 2026-11-19T07:50:10-07:00   Last seen: 2026-11-19T14:35:12-07:00
    Error: RuntimeError: Hermes can't reach the model provider. …
  c0be50_66dac7c1f4fe  resolved   Job: c0be50a686c6   Type: unknown
    Error: interrupted
  c0be50_77b79caeab04  resolved   Job: c0be50a686c6   Type: delivery
  c0be50_77b79caeab04 … (33 rows total, all `resolved`)
```

### 5. Scheduler-level view
```
$ hermes -p cronrunner cron status
  Scheduler host: the host gateway (PID 152866) serving profiles default, cronrunner, jobsearch, tim
✓ Gateway is running — cron jobs will fire automatically
  PID: 152866
  Ticker heartbeat: 44s ago
  5 active job(s)
  ⚠ 1 job(s) last fired late: 5c1532977f15 … catch-up after missed fire … (9h 49m late)

$ hermes -p cronrunner cron doctor
Cron doctor found 1 issue(s) across 1 job(s): 5c1532977f15 … last fire was catch-up after missed fire …
```
That late-fire report is the post-restart reconciliation of the same 2026-11-16 outage, still visible a day later and due to clear at the next on-time fire.

### 6. Verdict

| Surface | Observable? | What it shows |
|---|---|---|
| `cron runs` | yes | per-attempt ledger: completed / failed / missed / running, `interrupted` and `missed ×N` detail lines, `source=builtin|scheduler` |
| `cron incidents` | yes | durable incidents, state `resolved`, type agent/auth/delivery/unknown |
| `cron status` / `cron doctor` | yes | scheduler host+PID, ticker heartbeat, late-fire catch-up |
| admission journals `cron/admissions/*.json` | no CLI command | on-disk only: `params.request_id` ↔ `receipt {session_id, admission_id}` |

**Overall: admission recovery is observable**, and the CLI's own history contains three genuine reconciliations (two `interrupted` fires on the mailbox worker plus one on the Friday worker, and the 2026-11-16 missed/catch-up episode across three jobs). What is *not* exposed is the admission record itself — no `hermes cron admissions` subcommand — so "an interrupted execution stays paused for operator review" can only be confirmed from the filesystem journal, and no such paused-for-review case exists in this install's history to point at.

### 7. Deviations
None. Read-only throughout: no job edited, nothing restarted, no live run interrupted, no synthetic failure manufactured. The database was opened with `mode=ro` for the schema/inventory queries; every state quoted above also appears in `hermes cron runs` / `cron incidents` output.
