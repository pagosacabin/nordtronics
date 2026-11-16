---
task_id: "0198"
protocol_version: 1.2.0
status: staged
iteration: 1
expect-reply-within: 12h
proof:
  host_local: true
  branch: "none — design task: the deliverable is this reply; every proof artefact was a throwaway local clone (never a branch of the real repo, never pushed to origin)"
  sha: "none — no branch was created. The only nordtronics commits are this file's own transitions (pickup fb2c63f on main, and the staging commit that carries this file)."
  run: "none — no CI run is possible or applicable: no branch, no PR, no repository file changed. Enumerated: .github/workflows/ gates on branch allow-lists and path filters, and a main-only mailbox commit would run only the mailbox check."
  files: []
  files_reason: "no repository file is part of the deliverable — this is a design + local scratch proof. All proof work ran in ~/.hermes/profiles/cronrunner/cache/scratch/0198/ (a self-contained bare 'origin' plus three worker clones), which has been deleted."
  artifacts:
    - "scratch rig: ~/.hermes/profiles/cronrunner/cache/scratch/0198/ (origin.git + worker-1..3), DELETED after the runs; transcript quoted below"
    - "4 consecutive proof runs, 14/14 assertions each: repeat-1.log .. repeat-3.log + proof-run.log (deleted with the rig; lines quoted verbatim below)"
    - "gate proof against the production mailbox-changed.sh via its MAILBOX_REPO/MAILBOX_GATE_LOG overrides: gate-proof.log (deleted with the rig; quoted verbatim)"
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
  notes: "Harness exposes no per-call DeepSeek usage object to this cron session, so the token and cost fields are unknown rather than estimated. The job's own per-fire usage IS recorded on this host (cron/usage_audit.jsonl) but only on run completion, so this run's own numbers were not visible while composing it. The design's cost comparison below uses those recorded figures for the JOB's history (not for this run) and says so."
notes: |
  STAGED 2026-11-16T19:22Z (12:22 MST), off-peak (PEAK: OFF-PEAK 19:15 UTC), PROTOCOL MATCH 1.2.0.

  Design + scratch proof only. Nothing was implemented: the live worker job, its prompt, its
  schedule, the protocol README and live mailbox state are untouched. 0195 and 0196 are archived,
  so the dependency note resolves — 0196 landed, and `.worktrees/` is gitignored in both repos.

  Pickup note (rule 16): the task arrived with no front-matter; the protocol fields were added at
  pickup (commit fb2c63f, 8 insertions, body unchanged).

  Headline finding that is NOT a design opinion: the wake gate has a live hole that a drain would
  make routine, proven with the production script (section "Gate hole" below). A task whose
  pickup commit has landed is invisible to `mailbox-changed.sh` — `main == origin/main`, mailbox
  clean, and the ticket is no longer in inbox/ — so a run that dies after pickup, or a drain that
  stops mid-task, is never woken again until something unrelated changes. Measured on the real
  script, plus a 5-line patch that flips the verdict.

  Also measured, from this host rather than assumed: 459 completed fires of job c0be50a686c6
  between 2026-09-28 and 2026-11-16, of which 346 (75%) were gate skips and 56 were real task
  runs (median 10.2 min, mean 15.9 min, p90 40 min); per-fire usage from usage_audit.jsonl on
  109 real-work runs: median 2.05M prompt tokens / 20.2k completion tokens, i.e. $0.08–$0.32 per
  task run depending on cache-hit rate. Those numbers drive the latency model and the verdict
  that the fire cadence, not parallelism, is the largest single lever.

  Self-caught defect, declared: the first scratch race run let one worker claim all three tasks
  because my driver had no per-session cap (`--stop-after` was unset) and the "one task" scenario
  still had a three-task queue. Both are fixed in proof.py; the transcript below is from the
  corrected rig, and the failure is why scenario S1 now asserts on claim-commit COUNT rather than
  on per-worker log text.
---

# 0198 — reply: cutting mailbox queue latency

## Recommendation (one paragraph)

**Do Option C, shipped in two steps, and do not build parallel pickup first.** Step 1 is what
buys the latency: drain mode (one worker session claims and finishes up to 3 tasks back-to-back,
stopping only between tasks), plus a 15-minute fire cadence inside the same off-peak hours, plus
a 5-line fix to the wake gate so a half-done task can never go invisible. On this host's measured
task duration (median 10.2 min) that takes a three-task queue from **~181 minutes to ~38 minutes**
end to end, changes no task-file state shape, and adds no new concurrency to the mailbox. Step 2 —
true parallel pickup — is specified in full below, is proven safe in a scratch rig, and should be
enabled **only** for tasks Juno marks `parallel_safe: true`, at N=2, after the drain has been
measured for two weeks: it buys a further ~18 minutes on a three-task queue (38 → 20), and it is
the only option that adds a claim protocol, a resurrection surface, and a wasted-session cost that
the drain does not have. The honest reading of the numbers is that the fire cadence is the biggest
single lever (112 of the 143 minutes), the drain is the second (120), and parallelism is the small
third (18 beyond both) — so the cheap fix is most of it, and it is worth doing before the
expensive one.

## Measured baseline (this host, not estimates)

```
# fires, from the cron executions store
job c0be50a686c6, 2026-09-28 → 2026-11-16
total completed: 459
gate-skips(<0.5m): 346   mid(0.5-2m): 57   real work(>=2m): 56
real-work minutes: median 10.2 mean 15.9 p90 40.0 max 70.7

# per-fire usage, from cron/usage_audit.jsonl (109 real-work rows with usage)
prompt tokens: median 2,052,026  p90 12,157,934
completion:    median 20,188     p90 83,265
cost/run upper-bound (all prompt tokens billed as misses):
  median $0.32  p90 $1.89  mean $0.67
cost/run at an 80% cache-hit split: median $0.075  p90 $0.48
```

Three facts fall out of that and shape everything below:

1. **75% of fires already cost nothing.** The gate works; idle ticks are free. So *tightening the
   cadence is nearly free* — the marginal fires are script runs, not model runs.
2. **A task run is ~10 minutes of wall clock but ~2M prompt tokens.** The expensive part of a
   worker session is the prompt it carries, not the work it does — which is why a drain (one
   prompt, three tasks, warm cache) is structurally cheaper per task than N fresh sessions, and
   why a parallel worker that wakes and finds nothing to do is not free.
3. **A median task (10 min) is much shorter than the fire interval (60 min).** Latency is
   dominated by waiting for the next fire, not by doing the work. That is the whole argument for
   cadence-first.

Latency model (arrivals uniform against the fire schedule, so expected wait = half a cadence;
d = median 10.2 min; 3-task queue; last task's completion measured from arrival):

| Mode | Last of 3 tasks done | Notes |
|---|---:|---|
| Serial, hourly (today) | **181 min** (198 at the mean) | matches Stephen's "3–4 hours" |
| Serial, 15-min cadence | 68 min | cadence alone removes 112 min |
| Drain cap 3, hourly | 61 min | drain alone removes 120 min |
| **Drain cap 3 + 15-min cadence (recommended)** | **38 min** | 55 min at the mean task |
| Parallel N=3, hourly | 42 min | |
| Parallel N=3 + 15-min cadence | 20 min | the remaining 18 min costs a claim protocol |

## Option A — drain mode (serial, no hourly gaps)

**What changes in the selector.** The whole of Step 2's item 2 and the "Work ONE task per run"
sentence becomes a bounded loop. The loop's stop conditions are checked **before** a new claim,
never mid-task, so a task is never abandoned half-done by its own worker:

- inbox empty (normal end);
- `max_tasks_per_run` (propose **3**) reached;
- a task ends `blocked`/decision-owed (stop dead — see below);
- a task ends failed at the iteration cap (stop dead, do not start another);
- a wall-clock soft cap (`elapsed > 75 min`) — checked only between tasks.

**Scheduler interaction.** None needed. The session is one fire, so `_inflight_key`/`_submit_with_guard`
(which refuses a second concurrent fire *of the same job*) is satisfied by construction, and the
next :15 fire is skipped while this one runs — the "eaten slot" behaviour that already exists. At a
median 10 min/task a 3-task drain is ~30 min and eats no slot at all; the slot is only eaten when
a single task already runs long, which is today's behaviour too. Missed fires are still recovered
by the scheduler's catch-up path (`last_dispatch.kind: catch-up` is recorded today).

**Hang / iteration cap.** A task that hangs is bounded by the model loop, not by the drain; the
drain must not add a second timer that kills work in flight. If a task reaches `iteration: 5`
unverified it is staged `status: failed` as the protocol already says, and the drain stops — a
failure means a decision, not another task.

**Decision-blocked task.** The drain stops dead and reports, exactly as rule 6 requires: no
staging, no iteration bump, no re-investigation. Proven in S5.

**Gate hole (must be fixed with A).** Today's `mailbox-changed.sh` wakes on: `main != origin/main`,
HEAD off main, tracked edits under `mailbox/`, unpushed commits, or a non-empty `mailbox/inbox/`.
A task that has been *picked up* satisfies none of those — so the tick after a crash (or after a
drain that stopped mid-task) sees "unchanged" and skips forever. That is live today and it would
become routine under drain. The fix is a durable resume marker written into the pickup commit and
deleted by the staging commit: `mailbox/active/.resume-needed` (an empty file, so it has no
front-matter shape Juno's tooling can trip over), plus 5 lines in the gate:

```bash
  # drain mode: a pickup leaves a durable resume marker so a half-done task is
  # wake-visible even though main == origin/main (task 0198).
  if [ -f "$REPO/mailbox/active/.resume-needed" ]; then
    reasons="$reasons resume-needed"
  fi
```

Deliberately *not* "wake on any `status: in_progress` in active/": the three parked files there
now (0142, 0159, 0187) are decision-blocked/superseded, and a naive active/ test would wake the
worker every hour forever. The marker is written only when a run intends to continue.

**Juno's side.** No state-shape change: the same four directories, the same `status:` values, the
same staged-file-is-the-reply rule. What changes is only the *rate* of transitions — several
files can move inbox→staged within one fire, and mtimes cluster. Her staleness tool already keys
on `status:`/`iteration`, both of which keep their meaning.

## Option B — true parallel pickup (N ≥ 2)

**(a) Claim.** The claim *is* the transition push — an optimistic CAS on `refs/heads/main`, which
is the only primitive the mailbox already has and which works across hosts:

```
loop (≤6 attempts):
  git fetch origin main && git reset --hard origin/main     # sync
  if task not in origin/main:mailbox/inbox/  -> CLAIM LOST, exit
  git mv inbox/NNNN-a.md active/NNNN-a.md; front-matter:
      status: in_progress, iteration+1,
      claimed_by: "<worker-id>", claimed_at: <iso>, lease_until: <iso+180m>
  commit; git push origin HEAD:main
  push rejected (non-fast-forward) -> backoff, retry (never force-push)
```

Concurrent workers cannot double-take a file: the loser's push is rejected, it re-syncs, and its
precondition (file still in inbox) is now false. Proven in S1 (one task, three sessions, exactly
one winner) and S2 (three tasks, three sessions, each claimed exactly once). Recovery from a
worker dying while holding a claim: the file sits in `active/` as `in_progress` with a
`claimed_by` that is not the resuming worker, and the existing selector already outranks it —
"S resume an in_progress task in active/ first" — so the next session resumes it with no lease
expiry logic at all. `lease_until` exists so a *future* sweep can distinguish a 5-minute-old claim
from a 3-day-old one without inventing new state. Proven in S4.

**(b) Transition serialization** — mapped against 0193's failure modes:

- *non-fast-forward*: the push is the CAS; a rejection is the normal path (S1 winner's third
  attempt, S2's 3-attempt winner). No force-push, ever.
- *double-move*: impossible for a claim (the second `git mv` cannot find the file after re-sync),
  and the same guard protects staging (S2, all three staged exactly once).
- *resurrection*: the 2026-09-24 class. The retry loop's `git reset --hard origin/main` means a
  worker never rebases a stale local `git mv` onto a newer main, which is exactly what resurrected
  0054. S4 asserts the staged file count for the resumed task is 1 and no inbox copy returns.
- *eaten slot*: parallel workers are N distinct jobs (`_inflight_key` is per job id), so N=3 jobs
  at the same :15 all dispatch — `cron.max_parallel_jobs` is unbounded by default on this host
  (`_resolve_max_parallel_workers` → None), so nothing needs configuring to allow it.

**(c) Work isolation.** Per-task worktrees as proven in 0193 (`.worktrees/NNNN`, `workdir` per
task); transitions stay in the one main checkout behind the CAS, so the two never interleave. What
must still never run concurrently:

- **the Copilot pane (t17) and the Newton pane (t19)** — one instance each, verified with
  `hc ls`; two tasks needing review or a build serialize on them regardless of how many workers
  exist;
- **two builds of the same repo** (PlatformIO/CMake caches are shared);
- **Newton/Gemini work** — parallel builds burn the pool ~2× per wall-clock hour, same total per
  task, so it is a *scheduling* consequence (a pool can hit 429 mid-batch and stall unrelated
  tasks), not a correctness one. Routing consequence: keep Newton to one task at a time, i.e. the
  Gemini pool's burn rate doubles only if two build tasks are allowed to run concurrently — which
  is why the proposal below caps N at 2 and excludes builds from the parallel lane.

**(d) Concurrency cap: N = 2.** Reasons, in order: (i) this host has 7 GB RAM with ~1 GB available
and 16 cores — two agent sessions plus their MCP children fit, three or more risk the OOM killer
taking a task with it; (ii) the single Copilot pane and single Newton pane serialize review/build
anyway, so N>2 mostly converts model time into queue time; (iii) each concurrently-woken session
that finds an empty queue pays a full ~2M-token prompt for nothing, and that waste scales with N.

## Option C — hybrid (recommended)

Drain as the default for everything, plus a parallel lane that exists but is **off** until a task
explicitly asks for it. The classification rule, mechanically applicable with no runtime judgement:

> **A task may run in the parallel lane iff its front-matter carries `parallel_safe: true`, set by
> Juno at filing time; every other task drains serially.**

That is deliberately a *filing-time declaration* rather than a worker-side heuristic: a worker
cannot reliably tell whether a task will contend for the Copilot pane, the bench, or a build
cache, and the failure mode of guessing wrong is two agents in one checkout. Juno, who writes the
task, can answer the question in one word. Nothing else in the protocol changes: a
`parallel_safe: true` task is claimed by the same CAS, in its own worktree, by a worker that
knows its lane.

Enablement trigger for the lane (put it in the README so it is not a vibe): **enable when, over
two weeks, (a) the drain's cap-stop fires at least once a day, and (b) no drain run exceeds 75
min.** Until then the parallel lane stays designed-but-off.

## Comparison

| | Latency for a 3-task queue | Complexity added | New failure modes | Quota / cost effect |
|---|---|---|---|---|
| **A — drain + 15-min cadence** | **38 min** (from 181) | ~40 lines of prompt, 5 lines of gate, one cron schedule edit | a long task still eats the next slot (unchanged); a crashed drain needs the resume marker (fixed in A) | ~neutral: 4× script runs, all $0 when idle (75% → ~97% of ticks skip); marginal tasks in a warm session are *cheaper* per task than fresh sessions |
| **B — parallel N=2** | 42 min hourly / 20 min at 15-min cadence | claim protocol, `claimed_by`/`lease_until`, N jobs, per-task worktrees, a resumed-claim path | claim race (proven safe), resurrection on stale rebase (guarded), two agents in one checkout (worktrees), OOM at N>2, wasted sessions on an empty queue | same total tokens per task; +1 full ~2M-token prompt for each worker that wakes and finds nothing; Newton/Gemini pool burns 2× per wall-clock hour during concurrent builds |
| **C — drain now, parallel lane gated** | 38 min today, 20 if the lane is enabled later | A's cost now; B's cost only when enabled | A's only, until the trigger fires | A's |

Recommendation as stated at the top: **C, step 1 = A**. The evidence for stopping there: the
cadence removes 112 of the 143 minutes and the drain another 120 (overlapping), so B's marginal
18 minutes is the most expensive minute-per-complexity in the table — and it is the only change
that puts a second writer on `main`.

## Gate hole — proven with the production script

Run against the scratch rig via the script's own `MAILBOX_REPO` / `MAILBOX_GATE_LOG` overrides, so
this is `~/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh` unmodified:

```
### A. before pickup: task in inbox, one task queued
GATE: mailbox changed -> inbox-non-empty [2026-11-16T19:18:49Z]
{"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "inbox-non-empty"}}
### B. after the pickup commit (main == origin/main, task in_progress in active/)
('claimed', 'mailbox/inbox/9001-synthetic.md', 1)
## main...origin/main
mailbox/active/.gitkeep
mailbox/active/9001-synthetic.md
-- gate output for the half-done task:
GATE: unchanged (main == origin/main, HEAD on main, mailbox clean) [2026-11-16T19:18:49Z]
{"wakeAgent": false}
### C. same state + the proposed .resume-needed sentinel, gate patched in scratch
patched gate: +5 lines before the inbox test
GATE: mailbox changed -> resume-needed [2026-11-16T19:18:49Z]
{"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "resume-needed"}}
```

B is the hole (`wakeAgent: false` while a task is in progress), C is the fix. The patch was applied
to a **copy** of the gate in scratch; the installed script was not modified.

## Scratch proof — commands and verbatim output

Rig (self-contained, `origin` is a local bare repo; the real remote is never referenced):

```
$ cat setup-rig.sh            # git init --bare origin.git; seed repo with mailbox/{inbox,active,staged,archive}
                              # and tasks 9001-9003; 3 clones worker-1..3 whose only remote is origin.git
$ bash setup-rig.sh 3
worker-1: origin  /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0198/origin.git (fetch)
worker-2: origin  /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0198/origin.git (fetch)
worker-3: origin  /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0198/origin.git (fetch)
```

Mechanism under test — `toolkit.py` (the shape the proposed `mailbox-transition.py` helper takes):
`fetch` → `reset --hard origin/main` → test precondition → `git mv` + front-matter edit → commit →
`git push`; a rejected push means re-sync and retry, max 6 attempts, no force-push. `session.py`
drives either a race (`--mode race`, N concurrent sessions) or a drain (`--mode drain --cap N`).

```
$ python3 proof.py
=== S1 claim race: ONE task, 3 concurrent sessions ===
--- worker-1 ---
[12:18:15] w1: claim -> empty  (attempts=2)
--- worker-2 ---
[12:18:15] w2: claim -> empty  (attempts=2)
--- worker-3 ---
[12:18:14] w3: claim -> claimed mailbox/inbox/9001-synthetic.md (attempts=1)
[12:18:15] w3: work 9001-synthetic.md on hermes/9001-synthetic @ 505d214f pushed=True
[12:18:15] w3: stage 9001-synthetic.md -> staged (attempts=1)
claim commits for 9001: ['9001']
PASS  S1 exactly one winner for 9001  [count=1]
PASS  S1 exactly one session claimed  [[3]]
PASS  S1 losers exited without a claim
```

S2 — three tasks, three sessions launched together (note the contention visible in `attempts`):

```
=== S2 claim race: THREE tasks, 3 concurrent sessions ===
--- worker-1 ---
[12:18:24] w1: claim -> claimed mailbox/inbox/9003-synthetic.md (attempts=3)
[12:18:25] w1: work 9003-synthetic.md on hermes/9003-synthetic @ aed07308 pushed=True
[12:18:25] w1: stage 9003-synthetic.md -> staged (attempts=1)
--- worker-2 ---
[12:18:24] w2: claim -> claimed mailbox/inbox/9002-synthetic.md (attempts=2)
[12:18:25] w2: work 9002-synthetic.md on hermes/9002-synthetic @ 61fc4b9f pushed=True
[12:18:25] w2: stage 9002-synthetic.md -> staged (attempts=1)
--- worker-3 ---
[12:18:24] w3: claim -> claimed mailbox/inbox/9001-synthetic.md (attempts=1)
[12:18:24] w3: work 9001-synthetic.md on hermes/9001-synthetic @ 2b599646 pushed=True
[12:18:25] w3: stage 9001-synthetic.md -> staged (attempts=1)
claim commits: ['9001', '9002', '9003']
PASS  S2 each task claimed exactly once
PASS  S2 all three staged
PASS  S2 inbox and active empty of tasks
```

S3 — drain, cap 2 over a 3-task queue:

```
=== S3 drain: cap 2 over a 3-task queue ===
[12:18:00] w1: claim -> claimed 9001-synthetic.md (attempts=1)
[12:18:00] w1: work 9001-synthetic.md on hermes/9001-synthetic @ 8f91bdf2 pushed=True
[12:18:00] w1: stage 9001-synthetic.md -> staged (attempts=1)
[12:18:00] w1: claim -> claimed 9002-synthetic.md (attempts=1)
[12:18:01] w1: work 9002-synthetic.md on hermes/9002-synthetic @ 7cf16f22 pushed=True
[12:18:01] w1: stage 9002-synthetic.md -> staged (attempts=1)
[12:18:01] w1: cap reached (2 tasks in this session) -- stop between tasks
RESULT: cap-stop
PASS  S3 drained exactly 2 tasks
PASS  S3 clean stop at cap with 1 left in inbox  [['mailbox/inbox/9003-synthetic.md']]
PASS  S3 no task left half-moved in active/
```

S4 — a worker dies after its claim commit; the next session resumes it, once:

```
=== S4 dead claim: worker dies after its claim commit ===
after the dead claim: ['mailbox/README.md', 'mailbox/active/9001-synthetic.md', 'mailbox/inbox/9002-synthetic.md', 'mailbox/inbox/9003-synthetic.md']
[12:18:01] w2: resume 9001-synthetic.md (claimed_by="w1")
[12:18:01] w2: work 9001-synthetic.md on hermes/9001-synthetic @ ... pushed=True
[12:18:01] w2: stage 9001-synthetic.md -> staged (attempts=1)
...
PASS  S4 the orphaned task was resumed and staged exactly once
PASS  S4 no resurrection of the archived/inbox copy
PASS  S4 claim commit count for 9001 is 1
```

S5 — drain meets a decision-blocked task (stops dead, stages nothing):

```
=== S5 drain meets a decision-blocked task ===
[12:18:02] w1: STOP: 9002-synthetic.md is decision-blocked; not resumable
RESULT: blocked-stop
final: ['mailbox/README.md', 'mailbox/active/9002-synthetic.md']
PASS  S5 blocked task left in active/, not staged
PASS  S5 drain reported a blocked stop
```

Stability — the whole suite re-run four times, races are probabilistic:

```
run 1: 14 pass, 0 fail -> ALL PASS
run 2: 14 pass, 0 fail -> ALL PASS
run 3: 14 pass, 0 fail -> ALL PASS
claim commits for 9001 (S1): ['9001'] ['9001'] ['9001'] ['9001']
claim commits (S2):          ['9001','9002','9003'] x4
```

**The scratch clone and the whole rig directory were deleted after these runs** (`rm -rf
~/.hermes/profiles/cronrunner/cache/scratch/0198`). Nothing was pushed anywhere: the rig's only
remote was `origin.git` inside the scratch directory, and no commit from it ever entered
`pagosacabin/nordtronics` (verified: `git ls-remote --heads origin | grep -i 0198` returns nothing,
and no `hermes/0198` branch was ever created).

## Follow-on implementation — exactly what would be filed

**1. Worker job prompt — Step 2 diff (replaces item 2 and the "Work ONE task per run" sentence).**
Current text:

```
2. Else pick the lowest-numbered file in `mailbox/inbox/` with `status: inbox`.
   Pickup: `git mv mailbox/inbox/NNNN-*.md mailbox/active/NNNN-*.md`, set
   `status: in_progress`, `iteration: iteration + 1`, commit + push. Then do the
   work on a branch (never main), push, and confirm CI green at the branch tip.
...
Work ONE task per run. Iteration cap 5: at `iteration: 5` unverified, move to
`staged/` with `status: failed` and a failure summary in `notes` (hard stop).
```

Proposed replacement:

```
2. Else pick the lowest-numbered file in `mailbox/inbox/` with `status: inbox`.
   Pickup: `git mv mailbox/inbox/NNNN-*.md mailbox/active/NNNN-*.md`, set
   `status: in_progress`, `iteration: iteration + 1`, create an EMPTY file
   `mailbox/active/.resume-needed`, commit + push. Then do the work on a branch
   (never main), push, and confirm CI green at the branch tip. Delete
   `.resume-needed` in the same commit that stages the task.

### DRAIN — work up to 3 tasks per run

After finishing a task (staged, or failed at the iteration cap) return to the
top of Step 2 and take the next one, until ONE of these stops the run:
  * `mailbox/inbox/` has no task with `status: inbox`  -> normal end
  * 3 tasks have been completed in this run           -> cap end
  * the task just finished is decision-blocked/blocked -> STOP DEAD
  * the task just finished was staged `status: failed` -> STOP DEAD
  * more than 75 minutes have elapsed                   -> stop BETWEEN tasks
Stop conditions are checked only BETWEEN tasks. Never abandon a task mid-work
to obey the cap: finish or park it, then decide. A blocked or failed task ends
the run — a decision is owed, and picking up another task would bury it.

Iteration cap 5 unchanged: at `iteration: 5` unverified, move to `staged/` with
`status: failed` and a failure summary in `notes`, then end the run.
```

**2. New helper script** — `~/.hermes/profiles/cronrunner/scripts/mailbox-transition.py`
(one line: "claim/stage a mailbox task with fetch-reset-test-commit-push retry, never
force-pushing"). Shape proven in the scratch run: `--mode claim|stage`, `--worker <id>`,
6 attempts with backoff, `claimed_by`/`claimed_at`/`lease_until` written on claim. The prompt calls
it instead of hand-rolling `git mv` + push, which is what makes the CAS reliable under a future
parallel lane. (~120 lines; the scratch `toolkit.py` is the reference implementation.)

**3. Wake gate** — the 5-line `.resume-needed` reason quoted in Option A, added to
`~/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh` before the `inbox-non-empty` test.

**4. Job schedule** — `hermes -p cronrunner cron edit c0be50a686c6 --schedule "*/15 4-17,21,22 * * 1-4"`
(same off-peak hours, 4× cadence). Then **verify `next_run_at`** — a past `next_run_at` survives
an edit and would fire immediately — and set it explicitly if stale. Cost consequence to accept:
the last fire before each peak boundary (17:45, 22:45) can run into the boundary; at a ~30% chance
of a run crossing and half its calls landing in peak, that is ~$0.10/day on a ~$0.35/day
upper-bound budget. If that matters, drop the `17` and `22` hours' final slots instead.

**5. Protocol README** — bump `protocol_version` 1.2.0 → **1.3.0**, sections touched: "Pickup /
transitions" (drain semantics + `.resume-needed`), "Blocked" (a blocked task ends the run), and a
new "Parallel lane" section carrying the `parallel_safe: true` rule and the enablement trigger.
No change to how a state is represented — only to how many transitions one run may perform.

**6. Juno's side.** Her watch/staleness tooling keeps working unchanged if it keys on `status:` and
`iteration:`; two things to confirm on her side, both additive: (a) it ignores
`mailbox/active/.resume-needed` (a dotfile, no `.md` suffix), and (b) it tolerates the optional
`claimed_by` / `claimed_at` / `lease_until` front-matter keys that appear only when a task is in
progress. If her parser is strict about unknown front-matter keys, that is the one real coupling
in this design — and it only affects the parallel lane, not the drain.

**Not implemented in this task**, per its constraints: no job edit, no prompt edit, no README
bump, no gate edit was made. The live worker still runs one task per fire.
