---
task_id: "0248"
title: "Goal-mode live proof — broken scratch job, gate must fail then pass"
status: in_progress
iteration: 1
expect-reply-within: 24h
run_as_goal: true
goal:
  max_turns: 8
  contract: |
    Outcome: make the failing unittest suite pass in
    /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob
    (package pkg/topic.py, tests tests/test_topic.py).

    Verification: `cd /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob && /usr/bin/python3 -m unittest discover -s tests`
    exits 0 with 2 tests passing.

    Constraints: edit only files under <goaljob>/pkg/. Do not edit, delete, skip or
    weaken anything under <goaljob>/tests/ — the expected topic strings stay exactly
    as written. Do not create, move or delete files outside <goaljob>/. No network,
    no package installs, no flashing, no hardware.

    Boundaries: host-only scratch exercise. The nordtronics repo, the mailbox and the
    live worker jobs are off-limits. Report the final gate output when done.
  gates:
    - "cd /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob && /usr/bin/python3 -m unittest discover -s tests"
proof: []
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-23T19:11:42+00:00"
lease_until: "2026-11-23T20:41:42+00:00"
notes: |
  Filed by the worker for 0247 step 4. Host-only scratch proof of the protocol 1.4.0
  run_as_goal standard: the initial state FAILS its gate (2 tests, 2 failures), and the
  task is staged only after a full green gate pass. Tests/sha256 of the test file at
  filing: 3516a476be357e2cee6c4e1512aa0a34a021bd96470ba241caeec4460081b472.
---

## Context

0247 makes `run_as_goal` a standard (protocol 1.4.0). This task is the live proof: a real
inbox task, carrying `run_as_goal: true`, claimed by the normal worker cycle and executed as
a goal — no hand-running. The subject is the two-file scratch job from 0246 section 4's shape,
seeded deliberately broken.

- Job: `/home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob/`
  (`pkg/topic.py`, `tests/test_topic.py`)
- Initial state: `telemetry_topic()` returns `wildfire/<node>/<kind>`, missing the
  `nordtronics/` root, so both tests fail.
- Gate: `cd <goaljob> && /usr/bin/python3 -m unittest discover -s tests`

## Task

Let the standard cycle claim this task, create the goal card, run it, and gate it. The
worker's obligations are exactly those of protocol 1.4.0: create the card, wait, re-run the
whole gate list itself, and stage only on a full green pass with the gate log committed.

No hand-fixing of the scratch job: the fix must come from the goal card's run.

## Success criteria

- The card was created and actually ran (a spawn, not a guard).
- The gate failed before the run's fix and passed after it, with both runs logged.
- `mailbox/gate-logs/0248-t<n>-1-*.log` committed (the failing pass and the green pass) and the green one cited in `proof` as a `gate_log` entry.
- The test file's sha256 is unchanged from filing.

## Constraints

- Host-only scratch. No hardware, no flashing, no repo changes beyond the mailbox files and
  this task's gate log.
- Do not edit the test file or weaken the expected strings.
- Secrets never in the task file or the reply.

## Proof

- `mailbox/gate-logs/0248-t<n>-1-<slug>.log` (committed gate runs, fail then pass).
- The kanban card id and its run outcome.

## Reply format

Status line first. Then: card id, dispatcher evidence (spawn vs guard), the failing gate run,
the continuation (if any), the final green gate run with its log path, and the test-file hash.
