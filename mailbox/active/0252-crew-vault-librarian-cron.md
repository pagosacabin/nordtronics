---
task_id: "0252"
title: Crew-vault librarian cron — weekly distillation, with a Nov 18 → today catch-up run
status: in_progress
iteration: 2
expect-reply-within: 6h
proof: []
notes: Filed by Juno 2026-11-23 on Stephen's word ("Go ahead with 0252", 21:41 MST). AMENDED 21:45 MST the same evening: Stephen had already asked his interactive Hermes session to do the one-time librarian catch-up pass directly, before 0252 was filed. So the catch-up may be done — or in progress — by the time the worker picks this up. On pickup, check the vault HEAD first: if it has advanced past 6db52d8, treat the interactive pass as the catch-up and scope step 3 to whatever window remains after the latest vault commit (possibly nothing). The durable deliverable of this task is the recurring cron. Context: the fresh Hermes session found the crew vault healthy but asleep — HEAD 6db52d8, last commit 2026-11-18 15:16, ~30 tasks of durable knowledge since never distilled. Stephen approved Juno's proposal: a weekly librarian job on Hermes's side, window = since the last vault commit (self-healing), one-line staged report per run so Juno's outbox watch sees the vault move. The vault remote/backup question is deliberately NOT in this task (repo creation 403s on the narrow PAT; Stephen creating the empty private repo in the GitHub UI is a separate one-minute step).
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-24T04:45:21+00:00"
lease_until: "2026-11-24T06:15:21+00:00"
---

## Assignment

- **Hermes:** all of it — create the cron job, run the catch-up pass, stage the reply and the first librarian report. Host-only work plus local-git commits in ~/crew-vault; no GitHub repo code changes, so no Copilot step.
- **Stephen:** nothing.
- **Juno:** verifies the reply (job definition, catch-up commit, pages touched) and watches for the weekly staged reports thereafter.

## Context

The crew vault at ~/crew-vault (pilot 0214, closed verified 2026-11-20; retest 0231 passed) is the laptop crew's shared curated memory: Hermes is librarian and the only editor of curated pages; every fact dated and sourced; OPEN/unverified markers are the conventions working, not failing. It has not been updated since its pilot content landed on 2026-11-18. Since then the mailbox has moved roughly thirty tasks — the 64-colour VGA port, the photo flow (0244/0245), the SD card chain (0234–0243 and today's undetectable state), the DAC calibration (0250), the goal-mode work (0246–0248) — and none of it reached the vault. Durable knowledge is accumulating in archived task files, which is the exact gap the vault was built to close.

## Task

1. **Create a weekly librarian cron job** in your own scheduler: **Sundays 18:00 America/Denver** (outside the worker peak guard). Name it clearly (e.g. `crew-vault-librarian`). Use the profile's standard model configuration; do not change any job's pinning as part of this task.
2. **The job's standing instructions, each run:**
   - Determine the window: everything the mailbox has closed since the vault's last commit (read the vault's HEAD commit date; source material is mailbox/archive/ and mailbox/staged/ replies newer than that, plus your own session knowledge where a task is still open — cited as such).
   - Distill only durable facts: bench facts (pinouts, measured values, settled failure modes), decisions Stephen made, standing rules. Follow the vault's existing conventions exactly — every fact dated and sourced to its task, OPEN/unverified markers where the source doesn't settle it, no invented facts, no secrets, no task-by-task narrative.
   - File facts where they belong (bench/ pages, decisions/, a log/YYYY-MM-DD.md entry for the run), update INDEX if pages are added, and commit locally in the vault repo with a message naming the window covered.
   - If a window contains nothing durable, write the one-line log entry saying so and commit nothing else.
   - **Stage a one-line report in mailbox/staged/** named `librarian-report-YYYY-MM-DD.md`: date, window covered, pages created/updated, new vault HEAD SHA — or "nothing durable this week". This is how Juno's watch sees the vault move.
3. **First run — check before you distill.** Stephen already commissioned the catch-up pass from his interactive session the same evening this task was filed. On pickup, read the vault's HEAD: if it has advanced past 6db52d8, the catch-up is done or in progress interactively — do NOT redo it; run the job once over whatever window remains after the latest vault commit (which may be empty, in which case stage the report saying so). Only if HEAD is still 6db52d8 do you run the full catch-up yourself: window 2026-11-18 → today. Expected content includes the frame/VGA bench facts and pinout, the photo-flow behaviour, the SD chain's settled findings including today's state (card undetectable at every clock with known-good firmware; camera path displays without storing), the 0250 measured DAC tables (source: the 0250 task record; mark anything not yet formally closed as OPEN), and the goal-mode/protocol-1.4.0 decisions. Judge each candidate fact by the vault's bar, not by this list — the list is a pointer, not a quota.

## Success criteria

- The cron job exists (definition quoted in the reply) on the Sunday 18:00 MT cadence with the standing instructions above.
- The catch-up pass is committed in the vault: reply states the new HEAD SHA, the pages created/updated, and the window covered.
- The first `librarian-report-*.md` is present in mailbox/staged/.
- No vault convention violations in the new pages: spot-checkable dates and task sources on the new facts.

## Constraints

- Librarian edits the vault only. Do not modify mailbox task files (your staged report and this task's reply excepted), do not touch the worker/sweep job definitions, and do not change model pinning.
- No remote creation or push attempts in this task — the vault stays local-git until Stephen creates the repo.
- Secrets never enter the vault or the reports.
- If the scheduler cannot express the cadence or the staged-report step, park blocked with the exact limitation rather than approximating silently.

## Proof

- Host-only task: the reply is the deliverable — cron job definition, catch-up commit SHA, page list, and the staged report file, which Juno reads directly from the repo.

## Reply format

Status line first. Then: the cron job as created (name, cadence, where its instructions live), the catch-up run (window, pages created/updated, new HEAD SHA, anything deliberately left out and why), then deviations or blockers, plainly.
