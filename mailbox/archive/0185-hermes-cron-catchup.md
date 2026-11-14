---
task_id: "0185"
protocol_version: 1.2.0
status: verified
iteration: 1
expect-reply-within: 12h
proof:
  branch: none
  sha: none
  run: none
  files: []
  reason: >-
    Host-local cron configuration task: no repository file changed, so there is no branch,
    no SHA and no CI run in any outcome. The task's own Proof clause says the same. The
    durable record is the raw terminal output below plus the jobs.json backup
    (profiles/cronrunner/cron/jobs.json.bak-0185) taken before the edit.
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
  COMPLETED with two corrections to the task premise. Run 2026-11-14 15:15-15:2x UTC
  (08:15 MDT), off-peak (PEAK: OFF-PEAK 15:15 UTC), PROTOCOL MATCH 1.2.0.

  1. CATCH-UP APPLIED ONLY TO THE UPWORK SWEEP; THERE IS NO BACKUP JOB TO SET. The task
     assumes a sensor-DB backup cron job feeds the morning pipeline. It does not exist in
     Hermes cron on this host: cronrunner holds 6 jobs (4 active mailbox workers, the 5 AM
     Upwork sweep, 1 paused 15-min poll), the default profile holds 2 jobs (both paused:
     "Process Juno handoff inbox", "Weekly review"), and profiles `tim` and `jobsearch` hold
     none. The "Daily sensor DB backup" commits are authored by `snordlund
     <125768193+pagosacabin@users.noreply.github.com>` and pushed from another host — this
     host's git identity is `astroboy`/`hermes-cronrunner`, and no job, script, systemd user
     timer or user crontab entry produces them. So the 720-minute class has zero members here
     and nothing was invented to fill it. If Juno wants that job's catch-up widened, it has to
     be changed where the job actually runs (it is not this machine).

  2. THE DOCS' "0 = ALWAYS SKIP" IS NOT WHAT THE INSTALLED BUILD DOES. `--grace-minutes 0` is
     floored at 120 s in this build (`cron/jobs.py:947-960`,
     `return max(override, _MIN_GRACE_SECONDS)`), so 0 means "skip anything more than two
     minutes late", not "never catch up". Measured on the installed build:
     `_compute_grace_seconds('0 5 * * *', 0) -> 120`. I followed the installed build and report
     the difference rather than restating the docs.

  ALSO WORTH KNOWING (verified, not assumed): for a DAILY job the computed window is already
  half the period clamped to 2 h, i.e. exactly 7200 s — so `--grace-minutes 120` on the sweep
  makes the intent explicit and durable but changes no effective behaviour today
  (`_compute_grace_seconds('0 5 * * *', None) -> 7200`). The hourly mailbox workers already
  compute 1800 s (30 min) and the paused */15 poll computes 450 s; I left those alone.

  WHAT WAS CHANGED, AND NOTHING ELSE: one field on one job. A key-by-key diff of the cron
  store before vs after shows 6 jobs before, 6 after, same id set, and exactly one differing
  field across the whole file:
    `892982733bba -> {'grace_minutes': (None, 120)}`  (5 jobs byte-identical)
  Schedules, commands/prompts, models, providers, skills, scripts, workdirs, deliver targets
  and enable/disable state are untouched. No live run was cancelled. This tick's own job
  (8b1c9e1323c5) was not edited.

  PROFILE SCOPE (declared): this session exports HERMES_HOME to the cronrunner profile, so a
  bare `hermes cron list` prints "profile: cronrunner" — which is where every live job is and
  where the edit was applied. The default profile was read explicitly with
  `HERMES_HOME=/home/astroboy/.hermes hermes cron list` and left untouched (both its jobs are
  paused, so a grace setting there would be inert). `hermes -p <profile>` was used for every
  edit.

  MISSED-FIRE CHECK: `hermes cron doctor` reported "no issues" and quoted no warnings. The
  durable run ledger (`hermes cron runs --limit 500`, spanning 2026-10-19 -> 2026-11-14) holds
  zero `missed` rows and zero `cancelled` rows; the sweep has 26 consecutive daily
  `completed` fires, every one at 05:00:0x local. The single `failed` row in the ledger is the
  Fri mailbox worker on 2026-10-30 12:15 ("interrupted"), already recovered.
---

# 0185 — Hermes cron hygiene: inventory, missed-fire check, catch-up windows

## Commands run (verbatim, in order)

```bash
hermes cron list                                    # resolves to profile cronrunner (HERMES_HOME exported)
hermes -p cronrunner cron list
HERMES_HOME=/home/astroboy/.hermes hermes cron list  # default profile, read explicitly
hermes -p tim cron list
hermes -p jobsearch cron list
hermes -p cronrunner cron doctor
hermes -p cronrunner cron cancel --help
hermes -p cronrunner cron edit --help
cp /home/astroboy/.hermes/profiles/cronrunner/cron/jobs.json \
   /home/astroboy/.hermes/profiles/cronrunner/cron/jobs.json.bak-0185
hermes -p cronrunner cron edit 892982733bba --grace-minutes 120
hermes -p cronrunner cron list                      # after
hermes -p cronrunner cron doctor                    # re-run, unchanged verdict
hermes -p cronrunner cron runs --limit 500          # missed/failed row audit
```

Supporting enumerations (all run this tick, all read-only):

```bash
systemctl --user list-timers --all      # 4 timers: tmpfiles-clean, drkonqi x2, grub-boot-success — no backup
crontab -l                              # cron tick, ha-alerts, herdr-agents, @reboot herdr — no backup
grep -rln "sensor DB backup|sensor_db_backup|db_backup" .   # no hits in the repo
ls .github/workflows/                   # 6 workflows, none does a DB backup
git log -1 --format='%an <%ae>%ad%n%s' c82a165              # author snordlund, not this host
```

## Before: full `hermes cron list`

(pre-edit state, 08:16 MDT. The post-edit output further down differs only by the added
`Grace: 120 min` line on the sweep — 80 lines vs 79, verified by diff.)

```text
┌─────────────────────────────────────────────────────────────────────────┐
│                         Scheduled Jobs (profile: cronrunner)            │
└─────────────────────────────────────────────────────────────────────────┘

  c0be50a686c6 [active]
    Name:      Mailbox worker (DeepSeek) Mon-Thu
    Schedule:  15 4-17,21,22 * * 1-4
    Repeat:    ∞
    Next run:  2026-11-16T04:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-12T22:15:39.618477-07:00  ok
    Dispatch:  on time (scheduled 2026-11-12T22:15:00-07:00)
    Execution: completed  74564f5199544990a9d6f71d5ff643e6
    ⚠ Last failure at 2026-10-15T08:15:11.701120-06:00: RuntimeError: HTTP 401: Authentication Fails, Your api key: ****d53d is invalid (request_id: 362e6022-6e1e-4d74-93f1-10… — recovered since

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
    Schedule:  15 0-17,21,22 * * 0
    Repeat:    ∞
    Next run:  2026-11-15T00:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-08T22:15:06.832940-07:00  ok
    Dispatch:  on time (scheduled 2026-11-08T22:15:00-07:00)
    Execution: completed  58818cbcddff4dbe882525778241ff8b

  7aff6948c2c1 [active]
    Name:      Mailbox worker (DeepSeek) Fri
    Schedule:  15 3-23 * * 5
    Repeat:    ∞
    Next run:  2026-11-20T03:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-13T23:15:31.958411-07:00  ok
    Dispatch:  on time (scheduled 2026-11-13T23:15:00-07:00)
    Execution: completed  b8ab7814223f42c0886bf4611b2649db
    ⚠ Last failure at 2026-10-30T12:18:52.351269-06:00: interrupted — recovered since

  8b1c9e1323c5 [active]
    Name:      Mailbox worker (DeepSeek) Sat
    Schedule:  15 * * * 6
    Repeat:    ∞
    Next run:  2026-11-14T09:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-14T07:15:27.432834-07:00  ok
    Dispatch:  on time (scheduled 2026-11-14T08:15:00-07:00)
    Execution: running  130dd3683d2c4dd69e45905b74eed799

  892982733bba [active]
    Name:      Upwork lead sweep (read-only) 5AM MDT
    Schedule:  0 5 * * *
    Repeat:    ∞
    Next run:  2026-11-15T05:00:00-07:00
    Deliver:   local
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-14T05:03:41.829520-07:00  ok
    Dispatch:  on time (scheduled 2026-11-14T05:00:00-07:00)
    Execution: completed  9fb3384da4fc4875913ab94b86e52113
    ⚠ Last failure at 2026-10-15T05:00:08.564484-06:00: RuntimeError: HTTP 401: Authentication Fails, Your api key: ****d53d is invalid (request_id: aecf6cec-a457-4379-9cdf-5b… — recovered since
```

Default profile (`HERMES_HOME=/home/astroboy/.hermes`), both jobs **paused**, unchanged:

```text
  2148c3e14463 [paused]
    Name:      Process Juno handoff inbox
    Schedule:  */15 * * * *
    Repeat:    ∞
    Next run:  2026-09-22T19:15:00-06:00
    Deliver:   local
    Skills:    github, kicad-mcp-server
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-09-22T19:11:43.574215-06:00  ok
    Dispatch:  on time (scheduled 2026-09-22T18:00:00-06:00)
    Execution: completed  57d50c5a85fe4b119dec9263b6a2a3c9

  a6c49a025f6f [paused]
    Name:      Weekly review
    Schedule:  0 18 * * 0
    Repeat:    ∞
    Next run:  2026-09-27T18:00:00-06:00
    Deliver:   bot-chat:cronrunner
    Skills:    weekly-review-planning
```

`hermes -p tim cron list` → "No scheduled jobs in profile 'tim'."
`hermes -p jobsearch cron list` → "No scheduled jobs in profile 'jobsearch'."

## Doctor findings: full `hermes cron doctor`

```text
✓ Cron doctor found no issues
  Checked 5 active job(s).
```

**Every missed-fire warning quoted verbatim: there were none.** Doctor emitted no warnings, and
the durable ledger agrees — `hermes cron runs --limit 500` (500 rows, 2026-10-19 → 2026-11-14)
contains **0 `missed` rows, 0 `cancelled` rows**, 498 `completed`, 1 `failed`, 1 `running` (this
tick). The sweep's own rows: 26 consecutive `completed`, each dispatched at `05:00:0x` local, the
latest `9fb3384da4fc4875913ab94b86e52113  completed  job=892982733bba  2026-11-14T05:00:01.665827-07:00`.
The one failure in the window is not the sweep:

```text
364c6c590b5646beb1643e7f6c799af4  failed  job=7aff6948c2c1  source=builtin  2026-10-30T12:15:06.062295-06:00
```

That is the Fri mailbox worker, marked "interrupted — recovered since" in `cron list`; the job
has run green since (last run 2026-11-13T23:15:31 ok).

## Changes applied

| Job | ID | Class | Before | After |
|---|---|---|---|---|
| Upwork lead sweep (read-only) 5AM MDT | `892982733bba` | sweep | no override → computed 7200 s (120 min) | **`--grace-minutes 120`** (7200 s) |

Command:

```bash
hermes -p cronrunner cron edit 892982733bba --grace-minutes 120
# Updated job: 892982733bba
#   Name: Upwork lead sweep (read-only) 5AM MDT
#   Schedule: 0 5 * * *
#   Skills: none
#   Workdir: /home/astroboy/nordtronics
```

Nothing else changed. Key-by-key diff of `profiles/cronrunner/cron/jobs.json` against the
pre-edit backup:

```text
jobs before/after: 6 6 | same id set: True
892982733bba Upwork lead sweep (read-only) 5AM MDT -> {'grace_minutes': (None, 120)}
untouched count: 5
```

## After: full `hermes cron list`

```text
┌─────────────────────────────────────────────────────────────────────────┐
│                         Scheduled Jobs (profile: cronrunner)            │
└─────────────────────────────────────────────────────────────────────────┘

  c0be50a686c6 [active]
    Name:      Mailbox worker (DeepSeek) Mon-Thu
    Schedule:  15 4-17,21,22 * * 1-4
    Repeat:    ∞
    Next run:  2026-11-16T04:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-12T22:15:39.618477-07:00  ok
    Dispatch:  on time (scheduled 2026-11-12T22:15:00-07:00)
    Execution: completed  74564f5199544990a9d6f71d5ff643e6
    ⚠ Last failure at 2026-10-15T08:15:11.701120-06:00: RuntimeError: HTTP 401: Authentication Fails, Your api key: ****d53d is invalid (request_id: 362e6022-6e1e-4d74-93f1-10… — recovered since

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
    Schedule:  15 0-17,21,22 * * 0
    Repeat:    ∞
    Next run:  2026-11-15T00:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-08T22:15:06.832940-07:00  ok
    Dispatch:  on time (scheduled 2026-11-08T22:15:00-07:00)
    Execution: completed  58818cbcddff4dbe882525778241ff8b

  7aff6948c2c1 [active]
    Name:      Mailbox worker (DeepSeek) Fri
    Schedule:  15 3-23 * * 5
    Repeat:    ∞
    Next run:  2026-11-20T03:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-13T23:15:31.958411-07:00  ok
    Dispatch:  on time (scheduled 2026-11-13T23:15:00-07:00)
    Execution: completed  b8ab7814223f42c0886bf4611b2649db
    ⚠ Last failure at 2026-10-30T12:18:52.351269-06:00: interrupted — recovered since

  8b1c9e1323c5 [active]
    Name:      Mailbox worker (DeepSeek) Sat
    Schedule:  15 * * * 6
    Repeat:    ∞
    Next run:  2026-11-14T09:15:00-07:00
    Deliver:   local
    Skills:    github, kicad-mcp-server, handoff-mailbox
    Script:    mailbox-changed.sh
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-14T07:15:27.432834-07:00  ok
    Dispatch:  on time (scheduled 2026-11-14T08:15:00-07:00)
    Execution: running  130dd3683d2c4dd69e45905b74eed799

  892982733bba [active]
    Name:      Upwork lead sweep (read-only) 5AM MDT
    Schedule:  0 5 * * *
    Repeat:    ∞
    Next run:  2026-11-15T05:00:00-07:00
    Deliver:   local
    Grace:     120 min
    Workdir:   /home/astroboy/nordtronics
    Last run:  2026-11-14T05:03:41.829520-07:00  ok
    Dispatch:  on time (scheduled 2026-11-14T05:00:00-07:00)
    Execution: completed  9fb3384da4fc4875913ab94b86e52113
    ⚠ Last failure at 2026-10-15T05:00:08.564484-06:00: RuntimeError: HTTP 401: Authentication Fails, Your api key: ****d53d is invalid (request_id: aecf6cec-a457-4379-9cdf-5b… — recovered since
```

Effective catch-up windows on the installed build (`cron.jobs._compute_grace_seconds`, called
against each job's stored schedule and override):

```text
c0be50a686c6  15 4-17,21,22 * * 1-4  override=None  -> 1800 s = 30.0 min
0b32bc199a4e  */15 * * * *            override=None  ->  450 s =  7.5 min   (paused)
5c1532977f15  15 0-17,21,22 * * 0    override=None  -> 1800 s = 30.0 min
7aff6948c2c1  15 3-23 * * 5          override=None  -> 1800 s = 30.0 min
8b1c9e1323c5  15 * * * 6             override=None  -> 1800 s = 30.0 min
892982733bba  0 5 * * *              override=120   -> 7200 s = 120.0 min
```

## Flagged — left unchanged, with current settings

**No backup job exists to set to 720 minutes.** This is a gap in the task premise, not an
omission on my side:

| Where | Result |
|---|---|
| `profiles/cronrunner/cron/jobs.json` | 6 jobs; none named/described as a backup; none writes backup files (4 mailbox workers, 1 sweep, 1 paused poll) |
| default profile `cron/jobs.json` | 2 jobs, both **paused**, neither a backup |
| `profiles/tim`, `profiles/jobsearch` | 0 jobs each |
| `crontab -l` | cron tick, `ha-alerts.py`, `herdr-agents.py`, `@reboot herdr-agents.py` — no backup |
| `systemctl --user list-timers --all` | 4 timers (tmpfiles-clean, drkonqi ×2, grub-boot-success) — no backup |
| repo | no script/workflow matching `sensor DB backup` |
| commit `c82a165 "Daily sensor DB backup 2026-11-14"` | author `snordlund <125768193+pagosacabin@users.noreply.github.com>`, 03:13:13 MST — pushed by another host, not produced by any job on this machine |

**Unclassified jobs, left at the computed default** (the rubric's three classes — sweep / backup /
reminder-or-one-shot — do not cover a recurring interval worker, and inventing a value would be
guessing):

| Job | ID | Schedule | State | `grace_minutes` | Effective |
|---|---|---|---|---|---|
| Mailbox worker (DeepSeek) Mon-Thu | `c0be50a686c6` | `15 4-17,21,22 * * 1-4` | active | absent (default) | 1800 s / 30 min |
| Mailbox worker (DeepSeek) Sun | `5c1532977f15` | `15 0-17,21,22 * * 0` | active | absent (default) | 1800 s / 30 min |
| Mailbox worker (DeepSeek) Fri | `7aff6948c2c1` | `15 3-23 * * 5` | active | absent (default) | 1800 s / 30 min |
| Mailbox worker (DeepSeek) Sat | `8b1c9e1323c5` | `15 * * * 6` | active | absent (default) | 1800 s / 30 min |
| Mailbox poll (Nvidia) | `0b32bc199a4e` | `*/15 * * * *` | **paused** | absent (default) | 450 s / 7.5 min |
| Process Juno handoff inbox | `2148c3e14463` | `*/15 * * * *` | **paused** (default profile) | absent (default) | 450 s / 7.5 min |
| Weekly review | `a6c49a025f6f` | `0 18 * * 0` | **paused** (default profile) | absent (default) | 3600 s / 60 min |

Rationale for leaving the workers: they are idempotent and gated (the pre-run script skips the
agent run when the mailbox is unchanged), so a late catch-up costs nothing while a half-period
window already behaves sensibly. Say the word and I will set them to `0` (skip) or `720`.

**Do-not-touch list, honoured:** schedules, commands/prompts, models, providers, skills, scripts,
workdirs, deliver targets, and enable/disable state — all identical before and after (proved by
the 5 byte-identical jobs in the diff). No live run was cancelled; `hermes cron cancel` was used
only as `--help`.

## Cancel: available

Confirmed from the installed build's own help text (not assumed from the docs):

```text
usage: hermes cron cancel [-h] [--reason REASON] job_id

positional arguments:
  job_id           Job ID whose in-flight run to stop

options:
  -h, --help       show this help message and exit
  --reason REASON  Optional note recorded with the cancelled run
```

## Docs vs installed build

`hermes cron edit --help` on this build exposes `--grace-minutes GRACE_MINUTES` (0–1440,
"Pass an empty string to restore the computed window") and `--catch-up {on,off}`, so the edit
path matches the documented page. Two behavioural notes where the docs are the looser statement:

1. `--grace-minutes 0` is **floored at 120 s** (`_compute_grace_seconds` → `max(override, 120)`),
   so it means "skip anything over two minutes late", not "never catch up".
2. The default window for a daily job is already 7200 s, so the sweep's new explicit 120 minutes
   is a documentation-of-intent change, not a behaviour change today.

Global `cron.catch_up_missed` is unset in both configs, i.e. at its documented default `true`;
I did not change it (it is not a per-job grace setting and the constraint says grace only).
