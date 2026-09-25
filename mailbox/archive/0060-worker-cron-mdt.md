---
task_id: "0060"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0060-worker-cron-mdt
    sha: b0fe67ea0775d42df56a77aadae0d611c6aed267
  - artifact: "the four worker schedules now live in the Hermes cron store at ~/.hermes/profiles/cronrunner/cron/jobs.json (host state, not a repo file); the cited branch carries hermes cron list before/after, the store field dumps, the four edit/create transcripts, and the off-peak verification run"
  - no_ci_run: "no repo code changed and no workflow path filter matches evidence/** — the branch has 0 Actions runs (checked against the public Actions API)"
notes: |
  SCHEDULE UNCHANGED AS A FILE, CHANGED WHERE IT ACTUALLY LIVES. The worker is not a
  user-crontab line: crontab holds only the Hermes cron ticker (*/15 ... cron tick), which
  fires the jobs. The DeepSeek worker is Hermes cron job c0be50a686c6 in the cronrunner
  profile store. That store takes ONE cron expression per job, so the four requested lines
  are applied as four jobs, one line each, verbatim — the pre-existing job c0be50a686c6
  keeps the id (the paused poller job still references it) and takes the Mon-Thu line.

  Before: c0be50a686c6 "Mailbox worker (DeepSeek)"  15 4-18,22,23 * * *   (17 fires/day)
  After, four lines, verbatim as specified:
    c0be50a686c6 "Mailbox worker (DeepSeek) Mon-Thu"  15 4-18,22,23 * * 1-4
    5c1532977f15 "Mailbox worker (DeepSeek) Sun"      15 0-18,22,23 * * 0
    7aff6948c2c1 "Mailbox worker (DeepSeek) Fri"      15 4-23 * * 5
    8b1c9e1323c5 "Mailbox worker (DeepSeek) Sat"      15 * * * 6

  Everything except name+schedule is byte-identical to the original worker on all four:
  prompt (sha256 3fa12261...93190), skills [github, kicad-mcp-server, handoff-mailbox],
  model deepseek-flash, provider deepseek, script mailbox-protocol-check.py, workdir
  /home/astroboy/nordtronics, deliver local, failure_deliver local, no_agent false.

  Verified (scripts + raw output on the cited branch):
   1. 14-day sweep of every :15 slot (2026-09-26..2026-10-10) with the peak rule from the
      job's own pre-run script (01:00-04:00 / 06:00-10:00 UTC, Mon-Fri): 336 slots swept,
      70 land in peak windows and ZERO of them fire, 266 off-peak slots all covered, none
      uncovered, none double-covered. PASS.
   2. The scheduler's own parser (cron.jobs.parse_schedule + compute_next_run, Hermes venv)
      re-run over the four live exprs: each parses and reproduces the stored next_run_at.
      Next fires as stored: Fri 09-25 08:15, Sat 09-26 00:15, Sun 09-27 00:15, Mon 09-28
      04:15 (MDT).
   3. Poller untouched, both halves: job 0b32bc199a4e still */15 * * * * and still paused;
      crontab -l byte-identical before/after (diff clean).

  Not claimed: I did not fire any of the four jobs as proof — that would run a second
  mailbox worker over the same task file this run holds. The first observed fire is the Fri
  line at 08:15 MDT today. No build ran and no ntfy receipt is published (this task builds
  nothing, and the spec names no topic). Rollback: edit c0be50a686c6 back to
  '15 4-18,22,23 * * *' and pause/remove the three added ids; a pre-change copy of the
  store is in the cited branch's job-schedules-before.txt.

  Correction to the task premise, for the record: the old single line (added 2026-09-24
  18:52) was already peak-free under the stated windows — the line that fired during
  Sunday-evening peak was the earlier 15 22-23,0-6 * * *. What the four lines buy is
  completeness: weekend early hours (Sun/Sat 00-03) and Fri+Sat 19-21 MDT, which the old
  line missed. Coverage rises 119 -> 133 fires/week, all off-peak.

  No line was removed for the ticker: the four lines replace only the worker schedule; the
  15-minute poller entry (*/15 * * * * cron tick) is untouched.
---

## Reply — worker cron moved to true DeepSeek off-peak hours (2026-09-25, iteration 1)

Task done. The four lines are live, verbatim, in the place the worker schedule actually
lives. Raw output follows; the full captures, the check script and its run are on branch
`hermes/0060-worker-cron-mdt` @ `b0fe67ea0775d42df56a77aadae0d611c6aed267`
(`evidence/0060-worker-cron-mdt.*` + `evidence/0060-worker-cron-mdt/`).

### Where the worker schedule lives (the task's escape hatch, used)

Not the user crontab. `crontab -l` holds exactly one line: the Hermes **cron ticker**
(`*/15 * * * * … hermes -p cronrunner cron tick --accept-hooks`), which is what makes every
Hermes cron job fire. The DeepSeek worker is a **Hermes cron job** in the `cronrunner`
profile, store `~/.hermes/profiles/cronrunner/cron/jobs.json`, managed with
`hermes -p cronrunner cron edit <id>` / `hermes -p cronrunner cron create <expr> <prompt>`.

That store takes **one cron expression per job**, so the four requested lines are applied as
four jobs — one line each, verbatim. The pre-existing worker job keeps its id
(`c0be50a686c6`; the paused poller job still references it) and takes the Mon-Thu line.
Prompt, skills, model, provider, pre-run script, workdir and delivery on all four are
byte-identical to the original worker.

### BEFORE — `hermes -p cronrunner cron list` (2026-09-25 07:18:41 MDT)

```

┌─────────────────────────────────────────────────────────────────────────┐
│                         Scheduled Jobs                                  │
└─────────────────────────────────────────────────────────────────────────┘

  c0be50a686c6 [active]
    Name:      Mailbox worker (DeepSeek)
    Schedule:  15 4-18,22,23 * * *
    Repeat:    ∞
    Next run:  2026-09-25T08:15:00-06:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-protocol-check.py
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-09-25T06:19:37.292024-06:00  ok
    Dispatch:  on time (scheduled 2026-09-25T07:15:00-06:00)
    Execution: running  56f7f47beffe491482597d3783a6dae1

  0b32bc199a4e [paused]
    Name:      Mailbox poll (Nvidia)
    Schedule:  */15 * * * *
    Repeat:    ∞
    Next run:  2026-09-24T18:45:00-06:00
    Deliver:   local
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-09-24T18:37:10.159200-06:00  ok
    Execution: completed  3adc7f4cb65143f3a587f362d1b13f8a

  ⚠  Scheduler is not ready: no gateway or no fresh profile heartbeat.
     If no gateway is running: hermes gateway install
                    sudo hermes gateway install --system  # Linux servers
     Check status:  hermes cron status
```

`crontab -l` (unchanged by this task, quoted in full):

```
SHELL=/bin/bash
# Juno ⇄ Hermes mailbox loop — runs on the dedicated `cronrunner` Hermes profile
# (mailbox task 0042) instead of inside the interactive `default` profile, so the
# worker and the interactive session never share one Hermes home.
# The GIT_* identity makes worker commits distinguishable from interactive ones
# (the nordtronics repo-local user.name is `astroboy`).
# PATH must include /usr/sbin (gh) and ~/.local/bin (node, hermes).
PATH=/home/astroboy/.local/bin:/usr/sbin:/usr/bin:/bin
*/15 * * * * GIT_AUTHOR_NAME=hermes-cronrunner GIT_AUTHOR_EMAIL=hermes-cronrunner@nordtronics.local GIT_COMMITTER_NAME=hermes-cronrunner GIT_COMMITTER_EMAIL=hermes-cronrunner@nordtronics.local /home/astroboy/.local/bin/hermes -p cronrunner cron tick --accept-hooks >> /home/astroboy/.hermes/profiles/cronrunner/logs/cron-tick.log 2>&1
```

### AFTER — `hermes -p cronrunner cron list` (2026-09-25 07:19 MDT)

```

┌─────────────────────────────────────────────────────────────────────────┐
│                         Scheduled Jobs                                  │
└─────────────────────────────────────────────────────────────────────────┘

  c0be50a686c6 [active]
    Name:      Mailbox worker (DeepSeek) Mon-Thu
    Schedule:  15 4-18,22,23 * * 1-4
    Repeat:    ∞
    Next run:  2026-09-28T04:15:00-06:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-protocol-check.py
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-09-25T06:19:37.292024-06:00  ok
    Dispatch:  on time (scheduled 2026-09-25T07:15:00-06:00)
    Execution: running  56f7f47beffe491482597d3783a6dae1

  0b32bc199a4e [paused]
    Name:      Mailbox poll (Nvidia)
    Schedule:  */15 * * * *
    Repeat:    ∞
    Next run:  2026-09-24T18:45:00-06:00
    Deliver:   local
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-09-24T18:37:10.159200-06:00  ok
    Execution: completed  3adc7f4cb65143f3a587f362d1b13f8a

  5c1532977f15 [active]
    Name:      Mailbox worker (DeepSeek) Sun
    Schedule:  15 0-18,22,23 * * 0
    Repeat:    ∞
    Next run:  2026-09-27T00:15:00-06:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-protocol-check.py
    Workdir:   /home/astroboy/nordtronics

  7aff6948c2c1 [active]
    Name:      Mailbox worker (DeepSeek) Fri
    Schedule:  15 4-23 * * 5
    Repeat:    ∞
    Next run:  2026-09-25T08:15:00-06:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-protocol-check.py
    Workdir:   /home/astroboy/nordtronics

  8b1c9e1323c5 [active]
    Name:      Mailbox worker (DeepSeek) Sat
    Schedule:  15 * * * 6
    Repeat:    ∞
    Next run:  2026-09-26T00:15:00-06:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-protocol-check.py
    Workdir:   /home/astroboy/nordtronics

  ⚠  Scheduler is not ready: no gateway or no fresh profile heartbeat.
     If no gateway is running: hermes gateway install
                    sudo hermes gateway install --system  # Linux servers
     Check status:  hermes cron status
```

The four worker lines, as stored:

| job id | name | schedule |
|---|---|---|
| `c0be50a686c6` | Mailbox worker (DeepSeek) Mon-Thu | `15 4-18,22,23 * * 1-4` |
| `5c1532977f15` | Mailbox worker (DeepSeek) Sun | `15 0-18,22,23 * * 0` |
| `7aff6948c2c1` | Mailbox worker (DeepSeek) Fri | `15 4-23 * * 5` |
| `8b1c9e1323c5` | Mailbox worker (DeepSeek) Sat | `15 * * * 6` |

Every non-schedule field is identical to the original worker: prompt
(sha256 `3fa1226182b74252a77c597ad048c46df09fa796ac07c78d0771e3b8f1c93190`), skills
`[github, kicad-mcp-server, handoff-mailbox]`, `model=deepseek-flash`, `provider=deepseek`,
`script=mailbox-protocol-check.py`, `workdir=/home/astroboy/nordtronics`, `deliver=local`,
`failure_deliver=local`, `no_agent=false`.

### Poller untouched

The 15-minute poller entry is untouched: job `0b32bc199a4e` "Mailbox poll (Nvidia)" still
reads `*/15 * * * *` and is still paused, and `crontab -l` is byte-identical before/after
(`diff` clean) — the same single `*/15 * * * *` `cron tick` line, no worker line added to or
removed from the user crontab.

### What was actually checked

1. **No peak fires, no gaps, no double-fires** — a 14-day sweep of every `:15` slot
   (2026-09-26 → 2026-10-10) using the peak rule from the job's own pre-run script
   (`01:00-04:00` and `06:00-10:00` UTC, Mon-Fri), run with the scheduler's venv:

```
== worker jobs in the live store ==
  8b1c9e1323c5  'Mailbox worker (DeepSeek) Sat'  expr='15 * * * 6'  enabled=True  model=deepseek-flash  next=2026-09-26T00:15:00-06:00
  5c1532977f15  'Mailbox worker (DeepSeek) Sun'  expr='15 0-18,22,23 * * 0'  enabled=True  model=deepseek-flash  next=2026-09-27T00:15:00-06:00
  c0be50a686c6  'Mailbox worker (DeepSeek) Mon-Thu'  expr='15 4-18,22,23 * * 1-4'  enabled=True  model=deepseek-flash  next=2026-09-28T04:15:00-06:00
  7aff6948c2c1  'Mailbox worker (DeepSeek) Fri'  expr='15 4-23 * * 5'  enabled=True  model=deepseek-flash  next=2026-09-25T08:15:00-06:00

== next 3 fires per requested line (local time) ==
  15 4-18,22,23 * * 1-4  Mon-Thu 4 AM-6 PM + 10-11 PM       Mon 2026-09-28 04:15 | Mon 2026-09-28 05:15 | Mon 2026-09-28 06:15
  15 0-18,22,23 * * 0    Sun midnight-6 PM + 10-11 PM       Sun 2026-09-27 00:15 | Sun 2026-09-27 01:15 | Sun 2026-09-27 02:15
  15 4-23 * * 5          Fri 4 AM-11 PM                     Fri 2026-09-25 08:15 | Fri 2026-09-25 09:15 | Fri 2026-09-25 10:15
  15 * * * 6             Sat all day                        Sat 2026-09-26 00:15 | Sat 2026-09-26 01:15 | Sat 2026-09-26 02:15

== 14-day sweep of :15 slots from 2026-09-26 to 2026-10-10 ==
  :15 slots swept          : 336
  of those, peak-window    : 70  (worker fires there: 0)
  off-peak slots           : 266
  uncovered off-peak slots : 0
  double-covered off-peak  : 0

== poller / ticker untouched ==
  job 0b32bc199a4e 'Mailbox poll (Nvidia)' expr='*/15 * * * *' enabled=False (unchanged)
  user crontab: exactly one */15 '* * * * *' tick line, unchanged

== verdict ==
  PASS — four requested lines live, zero peak fires, every off-peak :15 slot covered exactly once, poller/ticker untouched
```

2. **The scheduler accepts and will fire them** — its own
   `cron.jobs.parse_schedule` + `compute_next_run` re-run over the four live expressions;
   each parses and the computed next run equals the value stored in the job:
   Fri 2026-09-25 08:15, Sat 2026-09-26 00:15, Sun 2026-09-27 00:15, Mon 2026-09-28 04:15 MDT.

```
cron.jobs.parse_schedule + compute_next_run (the scheduler's own code path)

c0be50a686c6  Mailbox worker (DeepSeek) Mon-Thu
   expr '15 4-18,22,23 * * 1-4' -> parse_schedule -> {'kind': 'cron', 'expr': '15 4-18,22,23 * * 1-4', 'display': '15 4-18,22,23 * * 1-4'}
   compute_next_run -> 2026-09-28T04:15:00-06:00   stored next_run_at -> 2026-09-28T04:15:00-06:00   agree=True
5c1532977f15  Mailbox worker (DeepSeek) Sun
   expr '15 0-18,22,23 * * 0' -> parse_schedule -> {'kind': 'cron', 'expr': '15 0-18,22,23 * * 0', 'display': '15 0-18,22,23 * * 0'}
   compute_next_run -> 2026-09-27T00:15:00-06:00   stored next_run_at -> 2026-09-27T00:15:00-06:00   agree=True
7aff6948c2c1  Mailbox worker (DeepSeek) Fri
   expr '15 4-23 * * 5' -> parse_schedule -> {'kind': 'cron', 'expr': '15 4-23 * * 5', 'display': '15 4-23 * * 5'}
   compute_next_run -> 2026-09-25T08:15:00-06:00   stored next_run_at -> 2026-09-25T08:15:00-06:00   agree=True
8b1c9e1323c5  Mailbox worker (DeepSeek) Sat
   expr '15 * * * 6' -> parse_schedule -> {'kind': 'cron', 'expr': '15 * * * 6', 'display': '15 * * * 6'}
   compute_next_run -> 2026-09-26T00:15:00-06:00   stored next_run_at -> 2026-09-26T00:15:00-06:00   agree=True

ALL FOUR EXPRS PARSE AND PRODUCE THE STORED NEXT RUN (scheduler will fire them).
```

### Limits, stated plainly

- **No job was fired as proof.** Firing one now would run a second mailbox worker over the
  same `mailbox/` task file this run is holding, so the proof is store state plus the
  scheduler's own parse/next-run computation — not an observed fire. First observed fire:
  the Fri line, today 08:15 MDT. (The `⚠ Scheduler is not ready` line in the listings is
  pre-existing: this host runs no gateway; the crontab ticker drives the scheduler.)
- **No CI run exists** for this branch and none is cited: nothing was built, no repo code
  changed, and no workflow path filter matches `evidence/**` (Actions API: 0 runs on the
  branch).
- **No ntfy receipt** — this task builds no artifact and the spec names no topic.
- **Correction to the task premise:** the previous single line (`15 4-18,22,23 * * *`, set
  2026-09-24 18:52) was already peak-free under the stated windows; the line that fired in
  Sunday-evening peak was the earlier `15 22-23,0-6 * * *`. What the four lines actually buy
  is completeness: Sun/Sat 00-03 and Fri+Sat 19-21 MDT, previously missed. Fires/week go
  119 → 133, all off-peak.
- **Rollback:** `hermes -p cronrunner cron edit c0be50a686c6 --schedule '15 4-18,22,23 * * *'`
  plus pausing `5c1532977f15`, `7aff6948c2c1`, `8b1c9e1323c5`. Pre-change store copy:
  branch `hermes/0060-worker-cron-mdt`, `evidence/0060-worker-cron-mdt/job-schedules-before.txt`.

---


# 0060 — Move worker cron to true DeepSeek off-peak hours (MDT)

## Context

The DeepSeek peak/off-peak windows on file were inverted. Verified 2026-09-25
against five current sources: PEAK = 01:00-04:00 and 06:00-10:00 UTC,
Monday-Friday (excluding Chinese public holidays). Off-peak (50% off) is
everything else. In MDT (UTC-6, through Nov 1): peak = Sun-Thu 7:00-10:00 PM
and Mon-Fri 12:00-4:00 AM. Your current worker line has been firing during
Sunday-evening peak and missing cheap weekend hours.

## Task

Update YOUR worker cron — the hourly DeepSeek worker tick at :15 past the
hour, not the 15-minute poller. Replace the current worker cron line(s) with
these four lines (same :15 tick, off-peak hours only):

```
15 4-18,22,23 * * 1-4
15 0-18,22,23 * * 0
15 4-23 * * 5
15 * * * 6
```

That is: Mon-Thu 4 AM-6 PM + 10-11 PM; Sun midnight-6 PM + 10-11 PM;
Fri 4 AM-11 PM; Sat all day.

## Success criteria

1. `crontab -l` shows the four new lines and no remaining worker line that
   fires during a peak window.
2. The 15-minute poller entry is untouched.

## Constraints

- Small task: deepseek-flash.
- If the worker schedule lives somewhere other than your user crontab (a
   cron-runner bot, /etc/cron.d, etc.), update it there and say where.
- Quote the before and after schedule output in full.

## Proof

The staged reply quoting the before/after cron output.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: before block, after block, and one line confirming the poller is
untouched.
