# 0185 — Hermes cron hygiene: inventory, missed-fire check, set catch-up windows

## Assignment
- **Hermes** (host-local CLI work on your own machine): all steps.
- **Newton:** not used — no code.
- **Copilot:** not used — no GitHub deliverable; this is local cron configuration.

## Context
Juno's daily learning for 2026-11-14, verified against the official Hermes docs (https://hermes-agent.nousresearch.com/docs/user-guide/features/cron), found two v0.23.0 behaviors your installed v0.23.1 should carry:

- `hermes cron cancel` stops a run in flight without touching the schedule. Script jobs get SIGTERM escalating to SIGKILL on the whole process group; agent jobs interrupt at the next checkpoint. The run books a `cancelled` row — no failure notice, and it does not count toward the failure streak.
- Missed runs follow an explicit catch-up policy: a missed occurrence catches up once inside a grace window of half the job's period (clamped 2 min–2 h). Per-job override `--grace-minutes 0–1440` (0 = always skip); global off via `cron.catch_up_missed: false`. Passed-over slots book as `missed ×N` ledger rows.

Stephen approved implementing this on 2026-11-14. Your machine sleeps and restarts, and two of your cron products feed Juno's morning pipeline: the 05:00 Upwork leads sweep (its handoff file is read by the 06:00 briefing) and the sensor-DB backup.

## Task
1. Run `hermes cron list` and `hermes cron doctor`. Capture the full raw output of both.
2. Run `hermes cron cancel --help` (read-only) and capture enough of the help text to prove the subcommand exists in the installed build.
3. Set catch-up grace by job class, matching jobs by the names in your own `cron list` output:
   - The Upwork leads sweep job: `--grace-minutes 120`.
   - Each backup job (name contains "backup", or its command writes backup files): `--grace-minutes 720`.
   - Each reminder / one-shot-style job: `--grace-minutes 0`.
   - Any job you cannot classify confidently: leave it unchanged and flag it in the reply with its name and current settings.
   Use the edit mechanism your installed build documents (`hermes cron --help` / `hermes cron edit --help` govern the exact form). Include every command you ran in the reply.
4. Re-run `hermes cron list` and capture the after state.

## Success criteria
- Before and after `hermes cron list` outputs in the reply, showing the sweep at 120 minutes grace and each backup job at 720 (or a flagged explanation for each exception).
- Doctor output in the reply, with every missed-fire warning quoted verbatim — or an explicit statement that doctor reported none.
- `hermes cron cancel` availability confirmed or refuted from the installed build's own help text.

## Constraints
- Change ONLY catch-up / grace settings. Include in your "do not touch" list: schedules, commands, models, and enable/disable state.
- Do not cancel any live run.
- If the installed CLI's edit path differs from the docs page, follow the installed build's help and say so in the reply.
- Cost: one short run on the standard tier. This is CLI output capture and applying settings, not reasoning work — no model escalation.

## Proof
Host-local task: no branch, no CI run. The staged reply carries the raw terminal outputs (before/after list, doctor, cancel help excerpt) as the record; Juno verifies against the docs behavior quoted in Context.

## Reply format
One staged reply with these sections:
- Commands run (verbatim, in order)
- Before: full `hermes cron list` output
- Doctor findings: full `hermes cron doctor` output, missed-fire warnings called out
- Changes applied: job name → grace value set, one line each
- After: full `hermes cron list` output
- Flagged: any unclassified jobs left unchanged, with their current settings
- Cancel: available / not available, with the help excerpt
