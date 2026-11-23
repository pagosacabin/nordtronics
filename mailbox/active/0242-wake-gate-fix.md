---
task_id: '0242'
status: in_progress
iteration: 2
proof: []
expect-reply-within: 6h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-23T04:02:09+00:00"
lease_until: "2026-11-23T05:32:09+00:00"
notes: |
  Filed 2026-11-22 16:57 MST at Stephen's direction in chat ("file 0242 so
  that we fix this problem from happening in the future"). Diagnosis is
  Hermes's own, from the interactive session at Stephen's laptop
  (2026-11-22 ~16:55, screenshot shared with Juno): the wake gate decides
  from git state alone, and 0241's pickup commit was pushed by the worker
  itself, so every tick since (20+ consecutive, 19:00Z-23:45Z) reported
  wake=false unchanged, ~5 s per run, nothing launched. The resume rule
  never fired because the worker never started.
---

# 0242 — Fix the wake gate: wake on in-progress tasks in active/

## Assignment

- **Hermes:** all of it — the gate change, the proof runs, the doc note.
  (Harness tooling on your machine; nothing here goes to Newton.)
- **Copilot:** peer-review the gate-script diff before it goes live, if
  the script is version-controlled. Reviewer, not proof — Juno verifies
  the staged reply independently. If the script is local-only, say so in
  the reply and the log evidence below carries the proof.
- **Juno:** verifies the reply (proof log lines, 0241's state), archives,
  pings Stephen.

## Context

The worker's wake gate (mailbox-changed.sh) reports "changed" only when
git state moves: local main vs origin/main, unpushed commits, dirty tree.
That misses state the worker itself creates. On 2026-11-22, task 0241 was
picked up at 07:30:27 (commit ac0953c "Pick up 0241") — pushed by the
worker — and from the next tick on, the mailbox has read "unchanged"
permanently: 20+ consecutive ticks with wake=false, each run ~5 seconds,
nothing launched. The protocol rule "resume any in_progress task in
active/" never fired because the worker never started. The same trap
shape contributed to 0236's stall on Nov 21. 0239 and 0240 escaped it only
because their runs completed inside the wake that claimed them.

0241 is still sitting in mailbox/active/ marked in_progress, with no
bench work done and nothing written. It is the live test case for this
fix.

## Task

1. **Change the gate** so it reports changed/wake when EITHER:
   - git state moved (the existing conditions, unchanged), OR
   - `mailbox/active/` contains at least one task whose status is
     in_progress (picked up, not yet staged, blocked, or closed).
   Nothing else changes: the peak guard still applies after the gate
   exactly as now, cadence stays */15, drain caps stay as protocol 1.3.0
   sets them.
2. **Prove both directions from the gate/worker log, after the change:**
   - *Wake case:* a tick with an in_progress task in active/ and no git
     movement attributable to anything else reports wake=true and the
     worker launches and resumes that task. If the wake that claims this
     task (0242) resumes 0241 first, per protocol, that is acceptable —
     report it — and then carry the falsifiable weight with a dummy: a
     small real task placed in active/ as in_progress (e.g. a
     reply-with-one-word task) that a later tick, with no git movement
     since, wakes for and completes end-to-end.
   - *Quiet case:* a tick with active/ clear of in_progress tasks and no
     git movement reports wake=false. The worker must not end up waking
     every tick forever.
3. **0241:** report when it actually resumed and what it staged. Do not
   hand-run it outside the worker as part of this task — its resumption
   through the gate is the demonstration.
4. **Record the fix** where the trap is documented (the mailbox skill /
   README section that warns about this trap): one dated note that the
   gate now also wakes on in_progress tasks in active/, so the knowledge
   survives this session.

## Success criteria

- The gate change is in place (diff shown in the reply, or committed if
  the script is version-controlled).
- Wake case proven with log lines: wake=true with an in_progress active
  task and no other git movement; the task resumed.
- Quiet case proven with log lines: wake=false with active/ clear and no
  git movement, after the change.
- 0241's resumption and outcome reported.
- The doc/skill note updated.

## Constraints

- Gate condition only. Do not change cadence, the peak guard, drain-mode
  caps, or protocol 1.3.0.
- Do not hand-run 0241 outside the worker for this task's proof.
- Cost: DeepSeek off-peak rates; this is a small script change plus
  observation ticks.

## Proof

- The gate diff (or its commit pointer).
- The wake-case and quiet-case log lines, quoted in the reply.
- 0241's staged reply existing, or its current state stated plainly.

## Reply format

Lead with the outcome in one sentence: the gate now wakes on in-progress
active tasks, proven both directions, or it does not. Then: the diff in
brief, the wake-case log lines, the quiet-case log lines, 0241's state,
the Copilot pass count and verdict (or the local-only statement), and
where the doc note landed.
