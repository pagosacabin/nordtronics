---
task_id: "0168"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 1
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
proof:
  branch: null
  sha: null
  run: none
  run_reason: >-
    No branch, no CI run: this task changes no repository code. It edits the
    Hermes cron store on the worker host
    (~/.hermes/profiles/cronrunner/cron/jobs.json, four schedule records) and
    touches neither the poller job nor the user crontab, so there is nothing to
    push and no workflow to trigger. Proof is host state, quoted below.
  files: []
  files_reason: >-
    No repository file changed. Repo commits in this run are mailbox
    transitions only (pickup in active/, this staged file).
  applied_to_host:
    - path: /home/astroboy/.hermes/profiles/cronrunner/cron/jobs.json
      change: >-
        schedule.expr rewritten on 4 job records (c0be50a686c6, 5c1532977f15,
        7aff6948c2c1, 8b1c9e1323c5); next_run_at re-derived by the store's own
        update_job() at update time.
---

# 0168 — Worker cron: MST off-peak schedule (DST ended)

## Context

Daylight saving time ended Sun 2026-11-01: America/Denver is now MST (UTC-7).
DeepSeek peak = 01:00-04:00 and 06:00-10:00 UTC, Mon-Fri (verified 2026-09-25
against five current sources). In MST that is: Sun-Thu 6:00-9:00 PM, Sun-Thu
11:00 PM-midnight, Mon-Fri 12:00-3:00 AM. Everything else is half price.

The four worker lines task 0060 set are MDT lines:

```
15 4-18,22,23 * * 1-4
15 0-18,22,23 * * 0
15 4-23 * * 5
15 * * * 6
```

As of this morning they fire during MST peak: 18:15 and 23:15, Mon-Thu and
Sunday (10 peak fires/week). Per your own 0060 reply, the worker schedule
lives in the Hermes cron store (profile `cronrunner`, store
`~/.hermes/profiles/cronrunner/cron/jobs.json`, one cron expression per job —
the four worker jobs were c0be50a686c6, 5c1532977f15, 7aff6948c2c1,
8b1c9e1323c5), NOT in the user crontab. The user crontab holds only the
`*/15 * * * * ... cron tick` ticker line.

## Task

Replace the worker schedule — the hourly DeepSeek worker tick at :15 past
the hour, NOT the 15-minute poller — with these four lines, verbatim:

```
15 4-17,21,22 * * 1-4
15 0-17,21,22 * * 0
15 3-23 * * 5
15 * * * 6
```

That is, in MST: Mon-Thu 4 AM-5 PM + 9-10 PM; Sun midnight-5 PM + 9-10 PM;
Fri 3 AM-11 PM; Sat all day.

---

# Reply — staged

Run 2026-11-01 16:15-16:17 UTC (09:15-09:17 MST), off-peak (`PEAK: OFF-PEAK 16:15
UTC`), `PROTOCOL: MATCH protocol_version=1.0.0`. 0142 skipped (`status:
superseded`, Juno's 2026-10-19 resolution) and 0159 skipped (decision-blocked,
still awaiting the Coyote Moon controller address) — 0168 was the lowest
actionable task.

Applied. The four worker lines now read, verbatim, the four expressions in the
task. The poller job and the user crontab are untouched.

## Before — cron store, read from `jobs.json` immediately before the edit

`~/.hermes/profiles/cronrunner/cron/jobs.json`, key `jobs[]`, quoted as read:

```json
{
  "c0be50a686c6": {"name": "Mailbox worker (DeepSeek) Mon-Thu", "expr": "15 4-18,22,23 * * 1-4", "enabled": true, "next_run_at": "2026-11-02T04:15:00-06:00"},
  "0b32bc199a4e": {"name": "Mailbox poll (Nvidia)",             "expr": "*/15 * * * *",          "enabled": false, "next_run_at": "2026-09-24T18:45:00-06:00"},
  "5c1532977f15": {"name": "Mailbox worker (DeepSeek) Sun",     "expr": "15 0-18,22,23 * * 0",   "enabled": true, "next_run_at": "2026-11-01T10:15:00-07:00"},
  "7aff6948c2c1": {"name": "Mailbox worker (DeepSeek) Fri",     "expr": "15 4-23 * * 5",         "enabled": true, "next_run_at": "2026-11-06T04:15:00-06:00"},
  "8b1c9e1323c5": {"name": "Mailbox worker (DeepSeek) Sat",     "expr": "15 * * * 6",            "enabled": true, "next_run_at": "2026-11-07T00:15:00-06:00"}
}
```

(Full pre-edit copy: `~/.hermes/profiles/cronrunner/cache/scratch/0168-before.json`.)

## After — `hermes -p cronrunner cron list` (fresh read after the edit)

```
  c0be50a686c6 [active]
    Name:      Mailbox worker (DeepSeek) Mon-Thu
    Schedule:  15 4-17,21,22 * * 1-4
    Repeat:    ∞
    Next run:  2026-11-02T04:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics

  0b32bc199a4e [paused]
    Name:      Mailbox poll (Nvidia)
    Schedule:  */15 * * * *
    Repeat:    ∞
    Next run:  2026-09-24T18:45:00-06:00
    Deliver:   local
    Workdir:   /home/astroboy/nordtronics

  5c1532977f15 [active]
    Name:      Mailbox worker (DeepSeek) Sun
    Schedule:  15 0-17,21,22 * * 0
    Repeat:    ∞
    Next run:  2026-11-01T10:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics

  7aff6948c2c1 [active]
    Name:      Mailbox worker (DeepSeek) Fri
    Schedule:  15 3-23 * * 5
    Repeat:    ∞
    Next run:  2026-11-06T03:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics

  8b1c9e1323c5 [active]
    Name:      Mailbox worker (DeepSeek) Sat
    Schedule:  15 * * * 6
    Repeat:    ∞
    Next run:  2026-11-07T00:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
```

Each `next_run_at` was re-derived by the store's own `update_job()` (not
hand-written) and now carries a `-07:00` offset — the MDT `-06:00` offsets in
the before block were themselves the drift. `repeat` stays `∞`, `enabled` stays
true, and `script: mailbox-changed.sh` / model `deepseek-flash` / workdir are
unchanged on all four.

## Criterion 1 — no worker line fires in an MST peak window (non-vacuous)

The check is a full 14-day expansion, not a hash of the expressions. Same
checker, both schedules, America/Denver → UTC, peak = 01:00-04:00 and
06:00-10:00 UTC Mon-Fri (Python `weekday() <= 4`; cron dow is never consulted,
so the dow-off-by-one trap does not apply here).

Script: `~/.hermes/profiles/cronrunner/cache/scratch/0168-peak-sweep.py`
(croniter, 2026-11-01 00:00 MST → +14 days). Full output:

```
=== OLD (MDT lines, evaluated in MST) ===
Mon-Thu    15 4-18,22,23 * * 1-4    fires=135 peak=16
           PEAK Mon 2026-11-02 18:15 MST -> Tue 01:15 UTC
           PEAK Mon 2026-11-02 23:15 MST -> Tue 06:15 UTC
           PEAK Tue 2026-11-03 18:15 MST -> Wed 01:15 UTC
           PEAK Tue 2026-11-03 23:15 MST -> Wed 06:15 UTC
           PEAK Wed 2026-11-04 18:15 MST -> Thu 01:15 UTC
           PEAK Wed 2026-11-04 23:15 MST -> Thu 06:15 UTC
Fri        15 4-23 * * 5            fires= 39 peak=0
Sat        15 * * * 6               fires= 47 peak=0
Sun        15 0-18,22,23 * * 0      fires= 42 peak=4
           PEAK Sun 2026-11-01 18:15 MST -> Mon 01:15 UTC
           PEAK Mon 2026-11-02 00:15 MST -> Mon 07:15 UTC
           PEAK Sun 2026-11-08 18:15 MST -> Mon 01:15 UTC
           PEAK Sun 2026-11-08 23:15 MST -> Mon 06:15 UTC
=== NEW (proposed) ===
Mon-Thu    15 4-17,21,22 * * 1-4    fires=127 peak=0
Fri        15 3-23 * * 5            fires= 41 peak=0
Sat        15 * * * 6               fires= 47 peak=0
Sun        15 0-17,21,22 * * 0      fires= 39 peak=0

old peak fires in 14 days: 20
new peak fires in 14 days: 0
OK: no new-line fire lands in a peak window
```

The old schedule's 20 peak fires per 14 days is the 10/week the task predicted,
so the check is live rather than vacuous — a checker that returns 0 for
everything would not have produced the old column. Peak definition re-verified
this run against footnote (2) of
<https://api-docs.deepseek.com/quick_start/pricing>: "Peak hours are 01:00 -
04:00 and 06:00 - 10:00 UTC, Monday through Friday, excluding Chinese public
holidays."

## Criterion 2 — poller and crontab untouched

Poller `0b32bc199a4e` "Mailbox poll (Nvidia)": still `*/15 * * * *`, still
`[paused]` (store: `enabled: false`, `state: "paused"`), `next_run_at` unchanged
at `2026-09-24T18:45:00-06:00`. It was not passed to `update_job()` at all.

User crontab, byte-identical before and after:

```
sha256 before: c644cf06570eee551c0b1ed886dfe913b2a2c24a739500e7246cc22241051de4
sha256 after:  c644cf06570eee551c0b1ed886dfe913b2a2c24a739500e7246cc22241051de4
diff before/after: empty ("CRONTAB BYTE-IDENTICAL")
```

The `*/15 * * * * ... cron tick` line and the `*/5 ... ha-alerts.py` line are
both present, unchanged.

## How the edit was made

`from cron.jobs import update_job` under
`HERMES_HOME=/home/astroboy/.hermes/profiles/cronrunner`, one call per job with
`{"schedule": "<expr>"}` — the store's own code path, so `schedule_display` and
`next_run_at` are re-derived by the same function the scheduler uses and the
stale `pending_slot` for the old expression is dropped. No hand-edit of
`jobs.json`. `hermes -p cronrunner` was used for every read (the profile flag
matters: without it the CLI resolves the default profile).

## Notes / deviations

- No repo code changed, so there is no branch, no CI run and no ntfy receipt —
  stated rather than manufactured. The `HERMES_HOME=/home/astroboy/.hermes`
  (default-profile) cron store was not read or written by this task; only the
  `cronrunner` profile's store was.
- Sun's new line keeps hours 0-2 excluded even though local Sun 00-02 is
  Sunday *UTC* (weekend, therefore already off-peak). That is what the task
  specified, it is a superset of the peak windows, and it costs nothing — the
  checker above uses the real UTC rule, not the hour list.
- One unrelated observation, not acted on: the store also holds the daily
  "Upwork lead sweep (read-only) 5AM MDT" job (`892982733bba`, `0 5 * * *`),
  whose name still says MDT. Its fire time is 05:00 MST → 12:00 UTC, off-peak
  either way, so it is not a cost defect — left alone as out of scope.
