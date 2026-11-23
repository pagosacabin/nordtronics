---
task_id: "0248"
title: "Goal-mode live proof — broken scratch job, gate must fail then pass"
status: verified
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

    Constraints: edit only files under /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob/pkg/. Do not edit, delete, skip or
    weaken anything under /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob/tests/ — the expected topic strings stay exactly
    as written. Do not create, move or delete files outside /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob/. No network,
    no package installs, no flashing, no hardware.

    Boundaries: host-only scratch exercise. The nordtronics repo, the mailbox and the
    live worker jobs are off-limits. Report the final gate output when done.
  gates:
    - "cd /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0248/goaljob && /usr/bin/python3 -m unittest discover -s tests"
proof:
  - gate_log: mailbox/gate-logs/0248-t2-1-unittest.log
    turn: 2
    exit: 0
  - gate_log: mailbox/gate-logs/0248-t1-1-unittest.log
    turn: 1
    exit: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-23T19:11:42+00:00"
lease_until: "2026-11-23T20:41:42+00:00"
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
  STAGED — goal-mode live proof complete. Card t_4b8e59ff (goal_mode=1, goal_max_turns=8)
  was created with --goal, spawned by the gateway dispatcher (pid 918374 -> worker 918416),
  ran 27 s, exit 0, and fixed pkg/topic.py. The gate failed before the run (rc=1, 2 failures)
  and passed after (rc=0, 2 tests OK), both logged under mailbox/gate-logs/. Test file sha256
  unchanged (3516a476be357e2cee6c4e1512aa0a34a021bd96470ba241caeec4460081b472), so the fix is
  in the subject, not in weakened tests. Deviations: gate logs are committed at 697498a,
  immediately before the staging commit, because the transition helper commits only the task
  file and the sentinel; and no continuation card was needed, so the gate-failure ->
  continuation branch is not exercised live. Cost unknown: the cron harness exposes no
  per-call usage objects, stated rather than estimated.
  RECOVERY DEVIATION: the first copies of both gate logs (and a commit naming the goaljob path
  absolutely in the contract) were discarded by the transition helper's fetch-reset, which resets
  main to origin/main and drops unpushed local commits. Both logs were regenerated at 19:40Z: t2 by
  re-running the same gate on the same fixed tree, t1 by reproducing the pre-fix state in a copy
  whose restored pkg/topic.py hashes to f8b4405599eefbfb2601b1b03c56d724369313fb8528c6d1360a9aab155d4266,
  byte-identical to the seeded defect. They are committed in the commit immediately AFTER the
  staging commit; the gate_log proof pointers resolve there and at main tip.
Juno verification (2026-11-23 12:2x MST): VERIFIED. Gate logs read back from origin main tip
(5ae454fa): t1 exit 1 with the exact two AssertionErrors claimed (missing 'nordtronics/'
prefix, bench-01 and bench-02), t2 exit 0, 'Ran 2 tests ... OK'. Lifecycle narrative is
internally consistent (filed ca03fb2, claimed aff3a97 by cronrunner, card t_4b8e59ff spawned
for real per board-DB events, run 27 s exit 0). Noted deviation, declared twice (reply notes
and the log files themselves): both gate logs are RECONSTRUCTIONS — the transition helper's
fetch-reset dropped the originals; t2 re-ran on the same fixed tree, t1 reproduced from the
defect file restored to hash f8b4405599... (stated byte-identical to the seeded defect). The
reconstruction is honest and the hash chain is recorded, but the logs are not the raw run
output. Still unproven live, as declared: the gate-failure -> continuation-card branch and
goal.github_contract. Proof task closed; 0247 (the standard itself) remains in active.

---

## Reply

**Status: staged.** The run_as_goal standard ran end to end through the untouched cycle: the
task was claimed by the normal worker helper, the goal card was created and really spawned,
and the gate failed before the fix and passed after it.

### Lifecycle, in order

1. **Filed** at `ca03fb2` with `run_as_goal: true` and `goal.max_turns: 8`.
2. **Claimed** by the standard path at 19:11:42Z — `mailbox-transition.py --mode claim`
   moved it to `active/`, `status: in_progress`, `iteration: 1`, sentinel created (commit
   `aff3a97`).
3. **Turn-1 gate, run by the worker itself before any card existed:** exit 1, 2 failures
   (`AssertionError: 'wildfire/bench-01/telemetry' != 'nordtronics/wildfire/bench-01/telemetry'`).
   Log: `mailbox/gate-logs/0248-t1-1-unittest.log`.
4. **Card created:** `t_4b8e59ff`, `--goal --goal-max-turns 8`. Read back from the board DB:
   `goal_mode=1`, `goal_max_turns=8`, completion contract `local-only` (host-only work names no
   `github_contract`).
5. **Dispatched for real.** Events: `created 12:12` → `claimed {lock: home:152866}` →
   `spawned {pid: 918374}` → `worker_bound {pid: 918416}`. This is a spawn, not a guard — the
   contrast is the stale 0246 probe card `t_3c3bc243`, which is still
   `respawn_guarded {'reason': 'blocker_auth'}` every 60 s.
6. **Run completed:** run #2, 27 s, `exit_code: 0`, summary "Fixed the scratch unittest suite
   by adding the missing `nordtronics/` prefix to the topic string in `pkg/topic.py`".
7. **Turn-2 gate, run by the worker:** exit 0, `Ran 2 tests ... OK`.
   Log: `mailbox/gate-logs/0248-t2-1-unittest.log`.
8. **The fix came from the card, not from weakening the test.** `tests/test_topic.py` sha256 is
   `3516a476be357e2cee6c4e1512aa0a34a021bd96470ba241caeec4460081b472` — identical to filing —
   while `pkg/topic.py` changed exactly one line, the return value.

### Deviations declared

- The gate logs are committed at `697498a`, immediately **before** the staging commit rather
  than inside it: `mailbox-transition.py` stages only the task file and `.resume-needed`.
  Recorded rather than hand-assembling a staging commit the helper would not produce.
- No continuation card was needed: the first run fixed the defect, so `max_turns: 8` was never
  consumed. The continuation path is therefore **not** exercised by this proof — the standard's
  gate-failure branch remains unproven live.

### Not proved here

- The gate-failure → continuation-card → re-run branch (the card succeeded on turn 1).
- `goal.github_contract` / `--completion-contract` on a goal card (host-only task, no CI gate).
