---
task_id: "0235"
protocol_version: 1.3.0
status: pending
iteration: 1
proof: []
notes: |
  Filed 2026-11-21 07:15 MST on Stephen's word ("Yes go ahead") to the daily
  learning finding: Hermes's framework has Persistent Goals (/goal) — a judge
  loop with a written completion contract and deterministic quality gates.
  This task is the trial: one small goal-mode run, honestly reported.
  The v0.23.3 update stays PARKED (Stephen's call, 2026-11-20) — if goal mode
  is not available in the installed v0.23.2, that is the finding; do not
  self-update to get it.
---

expect-reply-within: 24h

# 0235 — Goal-mode trial: extend the SD probe under a contract + `pio run` gate

## Assignment

Hermes does this task. Copilot peer-reviews the committed code change, routed by Hermes via Herdr; Copilot is a reviewer, not the proof — Juno verifies independently on top. Newton is not needed for this one.

## Context

- Juno's daily Hermes-framework learning (2026-11-21, docs: https://hermes-agent.nousresearch.com/docs/user-guide/features/goals) found Persistent Goals: a Ralph-style judge loop where a written completion contract plus deterministic quality gates decide "done" — a shell exit code, not a sentence. Stephen approved a trial the same morning.
- Why it matters here: the mailbox worker runs one turn per 15-minute tick, so multi-rung jobs stall between ticks. If goal mode works as documented, a rung chain can iterate to a verified end state in one session.
- The trial job is real prep for task 0234 (the SD retest, drafted, filed when Stephen seats a new card): extend the 0229 SD probe firmware with the two measurements 0234 needs. Host-only — no flashing, the frame board is not touched.
- Base: branch `hermes/0229-sd-diagnosis` @ 82a76ce in pagosacabin/cabin-play; the probe project is `living-frame/firmware/n16r8-sdprobe/` (PlatformIO). Push via SSH (gh.py writes to cabin-play return 403).

## Task

1. **Check availability first.** Determine whether goal mode exists in the installed Hermes (v0.23.2): the `/goal` command in an interactive/gateway session, and/or the kanban card `--goal` route the docs describe. Prefer the kanban route if it works unattended. If neither exists in this version, stop and report that as the finding, with the exact evidence (command output, menu text, or error). Do NOT run `hermes update` — the v0.23.3 update is parked by Stephen's decision.
2. **Write the contract.** Create one goal whose completion contract names: (a) the deliverable — the probe extension in step 3, committed and pushed on branch `hermes/0235-goal-trial`; (b) the quality gate — `pio run` exits 0 in `living-frame/firmware/n16r8-sdprobe/`; (c) the constraint — no flashing, no board contact.
3. **The job the goal executes.** Extend the probe firmware (extend, don't rewrite) with:
   - a stock-driver attempt: `sdmmc_card_init` + mount attempt, with the CMD5-poison replay control from 0229 run first so a driver failure is attributed, not just reported;
   - a first-pass counter: per-reset record of whether the first init pass succeeded, accumulated in the report partition across at least 10 resets' worth of slots (the counter logic only — the resets themselves happen in 0234, not here);
   - both results written through the existing report-partition path, in the same format style as the current logs.
4. **Let the loop run.** No hand-driving between judge iterations. If the gate rejects a completion claim, let the loop iterate; note each rejection and what was wrong.
5. **Copilot review** of the committed code, per the standing rule.

## Success criteria

- The reply states which route was used (interactive `/goal` or kanban `--goal`), the contract text verbatim, the number of judge iterations, whether the gate ever rejected a completion claim (and what it caught), total wall time, and where the documented behaviour differed from the real behaviour.
- `pio run` exits 0 at the pushed branch tip; branch SHA on origin.
- If goal mode is unavailable in v0.23.2, the reply is that finding with evidence — that is a successful trial outcome, not a failure.

## Constraints

- No flashing, no board contact, no changes to the frame board's current build.
- No `hermes update`, no version change — v0.23.3 stays parked.
- One goal, one small job. Do not expand the probe extension beyond step 3.
- Cost: standard tier; report turns/cost if the harness exposes them, `unknown` if not.

## Proof

- Branch `hermes/0235-goal-trial` SHA on origin (cabin-play).
- The goal contract text and the `pio run` success output excerpt in the reply.
- Copilot review pointer; if the pane has signed out, say so rather than skipping silently.

## Reply format

Standard mailbox reply format, leading with the goal-mode verdict: available or not, route used, iterations, gate behaviour, deviations from the docs — then the job's own results (what was built, compile evidence, branch pointer).
