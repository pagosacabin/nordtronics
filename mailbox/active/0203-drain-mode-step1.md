---
task_id: "0203"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 24h
notes: |
  Pickup 2026-11-17T12:20Z (05:20 MST), off-peak (PEAK: OFF-PEAK 12:15 UTC), PROTOCOL MATCH 1.2.0.
  Front-matter absent on delivery (rule 16): task_id / protocol_version / status / iteration added at
  pickup and the body-level `expect-reply-within: 24h` line folded into it. Body otherwise unchanged.
  Empty `mailbox/active/.resume-needed` created in this pickup commit per item 1 (the sentinel's own
  semantics); it is deleted in the staging commit.
---

# 0203 — Implement 0198 Step 1: drain mode + 15-minute cadence + wake-gate fix

## Assignment
- **Hermes:** every step below — the worker job prompt, the helper script, the wake gate, the job schedule, and the protocol README update.
- **Copilot (via Herdr, routed by Hermes):** peer reviewer for the protocol README change only (the one GitHub-bound piece). Reviewer, not the proof — Juno verifies independently.
- **Newton:** not needed for this task.

## Context
Task 0198 (staged, reviewed GREEN by Juno) designed how to cut mailbox queue latency. Stephen approved **Step 1 only** on 2026-11-16 ~18:03 MST, in chat: "Yes, let's go ahead and do it." This task implements exactly the follow-on spec in 0198's reply, section "Follow-on implementation — exactly what would be filed" — that section is the source of truth for the change texts; the essentials are restated below so this card stands alone.

Measured baseline (0198, this host): 459 fires, 75% gate-skips, 56 real runs, median 10.2 min; a 3-task queue takes ~181 min to clear under one-task-per-hourly-fire. Step 1 brings that to ~38 min. **The parallel lane stays OFF** — no second worker, no parallel pickups; the README documents the rule and its enablement trigger only (see step 5).

Juno-side coupling is already checked (Juno, 2026-11-16): her watch/staleness tooling ignores `mailbox/active/.resume-needed` (dotfile, no `.md` suffix, non-`NNNN-` name) and tolerates the optional `claimed_by` / `claimed_at` / `lease_until` front-matter keys. No Juno-side changes are needed; do not wait on any.

## Task
1. **Worker job prompt (job c0be50a686c6) — apply the Step 2 replacement from 0198's follow-on spec verbatim in substance:** pickup creates an EMPTY `mailbox/active/.resume-needed` in the pickup commit; the sentinel is deleted in the same commit that stages the task; then the DRAIN section — after finishing a task, return to the top of Step 2 and take the next, stopping only when: inbox has no `status: inbox` task / 3 tasks completed this run / the just-finished task is blocked (STOP DEAD) / the just-finished task staged `status: failed` (STOP DEAD) / more than 75 minutes elapsed (stop between tasks). Stop conditions are checked only between tasks — never abandon a task mid-work. Iteration cap 5 unchanged. Quote the before and after Step 2 text in the reply.
2. **Helper script** — create `~/.hermes/profiles/cronrunner/scripts/mailbox-transition.py` per 0198's spec: claim/stage a mailbox task with fetch-reset-test-commit-push retry (6 attempts, backoff, never force-push); `--mode claim|stage`, `--worker <id>`; writes `claimed_by`/`claimed_at`/`lease_until` on claim. 0198's scratch `toolkit.py` is the reference implementation. The worker prompt calls it for transitions instead of hand-rolled `git mv` + push. Quote its `--help` output (or a dry-run) in the reply.
3. **Wake gate** — add the 5-line `.resume-needed` reason from 0198 to `~/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh`, before the `inbox-non-empty` test. Prove it with the production script, the same method as 0198's gate-hole section: sentinel present + task in active/ → gate reports wake; sentinel absent + empty inbox → no wake. Quote both runs.
4. **Job schedule** — `hermes -p cronrunner cron edit c0be50a686c6 --schedule "*/15 4-17,21,22 * * 1-4"`. Then verify `next_run_at` — a stale past value survives an edit and would fire immediately; set it explicitly if stale. Quote the job's before/after schedule and the verified next_run_at. **Scope: job c0be50a686c6 (Mon–Thu) only** — the Fri/Sat/Sun workers keep their current cadence; if you think they should match, say so in the reply, do not change them.
5. **Protocol README** — bump `protocol_version` 1.2.0 → **1.3.0** in `mailbox/README.md`, via a PR with Copilot review: "Pickup / transitions" gains the drain semantics + `.resume-needed`; "Blocked" gains "a blocked task ends the run"; a new "Parallel lane" section carries the `parallel_safe: true` rule and the enablement trigger (two weeks of drain measurements, then a separate task) — documented, NOT enabled. No change to how a state is represented.
6. **Leave the live queue exactly as found.** 0199 stays in_progress in active/; 0142/0159/0187 stay untouched; 0202 stays in inbox. This task's own pickup happens under the old rules — the drain takes effect from the first fire after your changes land, and Juno will observe it from the mailbox.

## Success criteria
- Job c0be50a686c6 shows schedule `*/15 4-17,21,22 * * 1-4` with a sane, future next_run_at (quoted).
- The job prompt's Step 2 contains the sentinel pickup line and the DRAIN section (before/after quoted).
- `mailbox-changed.sh` contains the `.resume-needed` reason, and both gate proof runs are quoted (wake with sentinel, no wake without).
- `mailbox-transition.py` exists at the path above and runs (help/dry-run quoted).
- `mailbox/README.md` on main reads protocol_version 1.3.0 with the three section changes, merged via a Copilot-reviewed PR (comment links in the proof block).
- No parallel lane: no second worker job created, no `parallel_safe` pickups performed.
- The live queue is untouched: 0199 still in_progress in active/, 0202 still in inbox, parked tasks unmoved.

## Constraints
- Implement Step 1 only, exactly as 0198 specified it. Anything beyond it (parallel pickup, other jobs' cadences, gate redesign) is report-only.
- Never force-push. Transition conflicts resolve by the helper's retry/backoff, or stop and report.
- Keep the old job prompt text recoverable: quote it in the reply before replacing it.
- Cost note from the design, accepted by Stephen with the approval: boundary fires (17:45 / 22:45) can cross into peak, ~$0.10/day worst case. No schedule trimming for this.
- Secrets never in task files or staged replies.

## Proof
Standard proof block: PR URL + branch + SHA + main SHA for the README change (Copilot comment links included); the host-side changes proven by the quoted before/after texts, the gate proof runs, and the job listing — pasted terminal output is a claim, the quoted artifacts are the evidence. Cost fields per protocol; "unknown" is acceptable where the harness exposes no usage.

## Reply format
Front-matter per protocol 1.2.0 (Hermes adds it). Notes must include: the before/after Step 2 prompt text, the before/after schedule + verified next_run_at, both gate proof runs, the helper's help/dry-run output, and a one-line statement that the parallel lane remains off and the live queue was left as found.