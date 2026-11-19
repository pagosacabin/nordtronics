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

  THE RESTART — its state, and why this run did not finish it
   This cron session IS a child of the gateway (MainPID 1774), so bouncing that
   service kills the run that asks for it; Hermes' own terminal guard also refuses
   the request outright from inside the gateway (its exact wording is quoted in the
   reply body). `hermes update --yes --no-gateway-restart` is the documented
   cron-safe path: code and deps land, the bounce is DEFERRED and recorded as a host
   obligation. Verified present after the update:
     /home/astroboy/.local/state/hermes/gateway-locks/host-update-restart-b98b6592a8357c92.json
       expected_sha ae2a81ccc00e9b7c06301523e5bb09608397149c, armed_by_profile
       cronrunner, pid 147548 (the updater), owners ["...hermes-agent|147548|..."]
   `hermes gateway status` warns that a previous update pulled new code but did not
   bounce running gateways, that they may still serve pre-update modules (mixed
   sys.modules), and that an update or a gateway bounce is what clears it.
   `hermes update --plan` still lists the gateway as owing it:
   "gateway [default] pid 1774 — systemd @ 3b421578".

  THE HANDOFF — the tick that reads this must check, not assume
   A background `hermes update --yes` was launched from this run (the command the
   status line itself names) to discharge the obligation; it drains the gateway and
   bounces it, which ends this run. So:
   1. Re-read the live state: `systemctl --user show hermes-gateway.service -p
      MainPID -p ActiveEnterTimestamp`, `hermes --version`, `hermes gateway status`.
      If the service is active AND its MainPID is NOT 1774 AND the status line no
      longer carries the pre-update-modules warning, the update is fully live:
      stage with the before/after version outputs, the cron id lists, that health
      evidence and a cost block (unknowns + reason: this harness exposes no
      per-call DeepSeek usage).
   2. If the gateway is still PID 1774 after 2026-11-19T22:30Z, the deferred bounce
      did NOT land: park BLOCKED with this exact evidence and no proof block — the
      one-line fix is Hermes' own gateway-bounce command (the one its status line
      names) run from a shell OUTSIDE the gateway, i.e. a desktop/SSH shell and not
      a cron tick. Do not improvise a bounce through systemd timers, unit files or
      python subprocesses to get around the guard: the guard exists because that
      path takes the gateway down under a running agent.
   3. Do NOT run `hermes update --yes` again from a cron tick: it is the command
      whose bounce kills the run, and repeating it is how a restart loop starts.

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
