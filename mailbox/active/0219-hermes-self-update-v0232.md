---
task_id: "0219"
status: in_progress
iteration: 1
proof: []
notes: |
  IN FLIGHT — update applied, gateway restart pending; this run does NOT stage.
  Run 2026-11-19 14:45-15:00 MST (21:45-22:00 UTC), off-peak (PEAK: OFF-PEAK 21:45
  UTC); the pre-run script's PROTOCOL line errored ("Could not resolve hostname
  github.com") and was re-run by hand after DNS recovered: PROTOCOL MATCH 1.3.0.

  DONE AND VERIFIED THIS RUN
   - Before: "Hermes Agent v0.23.1 (2026.9.24) · stable · local 3b4215785882".
   - `hermes update --yes --no-gateway-restart` completed; the checkout is at
     ae2a81ccc00 (stable v0.23.2) and "hermes update --check" reports
     "Up to date with the latest release (stable v0.23.2 ...)".
   - After: "Hermes Agent v0.23.2 (2026.9.24) · stable · local ae2a81ccc00e".
   - Cron unchanged, both profiles, id+name+schedule identical before/after:
     cronrunner c0be50a686c6, 0b32bc199a4e(paused), 5c1532977f15, 7aff6948c2c1,
     8b1c9e1323c5, 892982733bba (Upwork 5AM sweep); default 2148c3e14463,
     a6c49a025f6f (both paused).
   - The update stashed and re-applied an uncommitted local patch to
     hermes-agent tools/tts_tool.py (23+/4-, the Edge-TTS asyncio timeout fix);
     it is still in the worktree, untouched by me.

  THE RESTART IS A DETACHED TIMER, NOT A DONE STEP — READ THIS BEFORE ACTING
   A gateway restart kills the run that requests it (this cron session IS a child
   of the gateway, MainPID 1774, and the unit's ExecStopPost runs
   gateway.cgroup_cleanup). So the restart was armed as a systemd --user transient
   timer, /home/astroboy/.hermes/cache/scratch/0219/post-restart-verify.sh, which
   restarts hermes-gateway.service and then writes
   /home/astroboy/.hermes/cache/scratch/0219/post-restart-health.log
   (is-active, MainPID, NRestarts, ActiveEnterTimestamp, `hermes --version`,
   `hermes gateway status`, cron id lists, journal tail).

  DO NOT RESTART THE GATEWAY AGAIN. The next tick resumes this task:
   1. read post-restart-health.log — confirm is-active=active, the MainPID is a
      NEW pid (not 1774), it started after 2026-11-19T22:00Z, and `hermes
      --version` in it reads v0.23.2;
   2. if the log is missing or shows a dead/inactive service, that is the
      blocked outcome: park it with the exact text, do not improvise;
   3. otherwise stage with the version outputs from the log, the cron id lists and
      this notes block, plus a cost block (unknowns + reason: this harness exposes
      no per-call DeepSeek usage).

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
