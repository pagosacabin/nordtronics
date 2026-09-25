# Task 0060 — worker cron moved to true DeepSeek off-peak hours (MDT)

Evidence branch for `mailbox/0060-worker-cron-mdt` (picked up 2026-09-25, iteration 1).
Captured on the worker host, local tz `America/Denver` (MDT, UTC-6).

## Where the worker schedule actually lives

Not the user crontab. The user crontab holds exactly one line — the Hermes **cron ticker**
(`*/15 * * * * hermes -p cronrunner cron tick --accept-hooks`), which drives every Hermes
cron job. The DeepSeek worker is a **Hermes cron job** in the `cronrunner` profile store:

```
~/.hermes/profiles/cronrunner/cron/jobs.json
managed with:  hermes -p cronrunner cron list | edit <id> | create <expr> <prompt>
```

So the four requested lines were applied there (the task's own constraint: "If the worker
schedule lives somewhere other than your user crontab (a cron-runner bot ...), update it
there and say where").

## Before → after

| | Schedule | Fires |
|---|---|---|
| BEFORE | `c0be50a686c6` "Mailbox worker (DeepSeek)" — `15 4-18,22,23 * * *` | 17/day × 7 = 119/wk |
| AFTER | `c0be50a686c6` "… Mon-Thu" — `15 4-18,22,23 * * 1-4` | 17 × 4 = 68/wk |
| AFTER | `5c1532977f15` "… Sun" — `15 0-18,22,23 * * 0` | 21/wk |
| AFTER | `7aff6948c2c1` "… Fri" — `15 4-23 * * 5` | 20/wk |
| AFTER | `8b1c9e1323c5` "… Sat" — `15 * * * 6` | 24/wk |
| | | 133/wk total (was 119) |

Raw `hermes cron list` output: `cron-list-before.txt`, `cron-list-after.txt`.
Store field dumps: `job-schedules-before.txt`, `job-schedules-after.txt`.

One cron expression per Hermes job, so the four lines are four jobs — the four schedule
strings appear verbatim, one per job, with the pre-existing job `c0be50a686c6` taking the
Mon-Thu line. Everything else on the new jobs is byte-identical to the original worker:
prompt (sha256 `3fa1226182b74252…`), `skills=[github, kicad-mcp-server, handoff-mailbox]`,
`model=deepseek-flash`, `provider=deepseek`, `script=mailbox-protocol-check.py`,
`workdir=/home/astroboy/nordtronics`, `deliver=local`, `failure_deliver=local`.

## Verification actually performed

1. `verify-0060.py` (run with `~/.hermes/hermes-agent/venv/bin/python`, the scheduler's own
   venv) — 14-day sweep of every `:15` slot from 2026-09-26 to 2026-10-10, peak rule taken
   from the job's own pre-run script (`scripts/mailbox-protocol-check.py::peak_line`:
   01:00-04:00 and 06:00-10:00 UTC, Mon-Fri):

   ```
   :15 slots swept          : 336
   of those, peak-window    : 70  (worker fires there: 0)
   off-peak slots           : 266
   uncovered off-peak slots : 0
   double-covered off-peak  : 0
   PASS — four requested lines live, zero peak fires, every off-peak :15 slot
          covered exactly once, poller/ticker untouched
   ```

   (`verify-0060-output.txt`; script included as `verify-0060.py`.)
2. `scheduler-parse-check.txt` — the scheduler's own `cron.jobs.parse_schedule` +
   `compute_next_run` re-run over the four live expressions; each parses and yields exactly
   the `next_run_at` stored in the job, so the scheduler will fire them.
   Next runs as stored: Fri 2026-09-25 08:15, Sat 2026-09-26 00:15, Sun 2026-09-27 00:15,
   Mon 2026-09-28 04:15 (all local MDT).
3. Poller untouched — both halves:
   - job `0b32bc199a4e` "Mailbox poll (Nvidia)" still `*/15 * * * *`, still paused, unchanged;
   - user crontab `crontab-before.txt` vs `crontab-after.txt` are byte-identical (`diff` clean).

## Limits of this evidence (stated plainly)

- **The new jobs were not fired for this proof.** Firing one would run a second mailbox
  worker over the same `mailbox/` file this run is holding, so the proof is store state plus
  the scheduler's own parse/next-run computation — not an observed fire. The first observed
  fire will be the Fri line at 08:15 MDT (job `7aff6948c2c1`).
- `hermes cron list` prints `⚠ Scheduler is not ready: no gateway or no fresh profile
  heartbeat`. That is pre-existing and unrelated: this host has no Hermes gateway; the
  crontab ticker line drives the scheduler (its heartbeat files were fresh at capture time).
- No CI run exists for this branch and none is cited: nothing was built and no repo code
  changed. `evidence/0060-worker-cron-mdt/**` matches no workflow path filter.
- The change is host state, not a repo artifact; a rollback is `hermes -p cronrunner cron
  edit 7aff6948c2c1/8b1c9e1323c5/5c1532977f15 --paused` plus restoring
  `c0be50a686c6` to `15 4-18,22,23 * * *`. A pre-change copy of the store is in
  `job-schedules-before.txt` and at
  `~/.hermes/profiles/cronrunner/cache/scratch/ev0060/jobs.json.bak-0060-20260925-071847`.

## Commands run (transcripts included)

`edit-mon-thu.txt`, `create-sun.txt`, `create-fri.txt`, `create-sat.txt`, i.e.

```bash
hermes -p cronrunner cron edit c0be50a686c6 --schedule '15 4-18,22,23 * * 1-4' \
  --name 'Mailbox worker (DeepSeek) Mon-Thu'
hermes -p cronrunner cron create '15 0-18,22,23 * * 0' "$(cat .../mailbox-worker-prompt.txt)" \
  --name 'Mailbox worker (DeepSeek) Sun' --deliver local --failure-deliver local \
  --skill github --skill kicad-mcp-server --skill handoff-mailbox \
  --script mailbox-protocol-check.py --workdir /home/astroboy/nordtronics \
  --model deepseek-flash --provider deepseek
# same for '15 4-23 * * 5' (Fri) and '15 * * * 6' (Sat)
```
