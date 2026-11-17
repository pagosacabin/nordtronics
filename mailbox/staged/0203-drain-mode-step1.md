---
task_id: "0203"
protocol_version: 1.3.0
status: staged
iteration: 1
expect-reply-within: 24h
proof:
  host_local: true
  pr: https://github.com/pagosacabin/nordtronics/pull/6
  branch: hermes/0203-protocol-1.3.0
  sha: d5d68038bd37d7e42d66b41f95959f105fed0014
  main_sha: 68e1f59700d77e689c11fdadab55beffcef09fb7
  run: https://github.com/pagosacabin/nordtronics/actions/runs/44082854335
  files:
    - mailbox/README.md
  files_reason: >-
    mailbox/README.md is the only repository file changed. The rest of this task is
    host-local (the cron job prompt + schedule, two scripts under
    ~/.hermes/profiles/cronrunner/scripts/, and the worker state file); those are proven
    in the reply by quoted before/after artifacts, not by a branch.
  reviews:
    - https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947876260
    - https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947920898
    - https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947946671
    - https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947978957
  applied_to_host:
    - ~/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh (wake gate, +5 lines)
    - ~/.hermes/profiles/cronrunner/scripts/mailbox-transition.py (new helper)
    - ~/.hermes/profiles/cronrunner/cron/jobs store (job c0be50a686c6 prompt + schedule)
    - ~/.hermes/profiles/cronrunner/mailbox-worker-state.json (last_seen_protocol_version=1.3.0)
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
  notes: "The harness exposes no per-call DeepSeek usage object to this cron session (cron/usage_audit.jsonl is written at run completion), so the token and cost fields are unknown rather than estimated. Every call in this run was off-peak (PEAK: OFF-PEAK 12:15 UTC at start, 12:26 UTC at the protocol record)."

notes: |
  STAGED 2026-11-17T12:30Z (05:30 MST), off-peak (PEAK: OFF-PEAK 12:15 UTC at start),
  PROTOCOL MATCH 1.2.0 at pickup -> 1.3.0 on main at the end of this run.

  All six numbered items are implemented. The parallel lane is OFF (no second worker job, no
  `parallel_safe` pickup, nothing reads the key), and this task's own pickup ran under the old
  one-task-per-run rules exactly as item 6 requires, so the drain takes effect from the next fire
  (05:30 MST), not from this one.

  ----------------------------------------------------------------------------------------------
  1. JOB PROMPT (c0be50a686c6) — Step 2 replaced, before/after quoted verbatim below. The prompt
     also now tells the worker to claim/stage through the helper, and adds the blocked-park case to
     the sentinel lifecycle (0198 specified claim/stage only; parking a blocked task would otherwise
     leave a permanent wake-forever sentinel — a defect in the design text, not in the intent).
     Verified after the write: prompt len 6569 -> 8189, equals the composed file byte-for-byte,
     contains "### DRAIN", ".resume-needed", "mailbox-transition.py", and no longer contains
     "Work ONE task per run".

  2. HELPER (mailbox-transition.py) — new, 12,170 bytes, sha256 2a8c7c50fc4134cf6db89ac94f51e4c76bab67b07808e024cbd6de7279fe9b75.
     --mode claim|stage with fetch-reset-test-commit-push, 6 attempts, backoff 2..32 s, never
     force-pushing; claim writes claimed_by/claimed_at/lease_until + the sentinel; stage deletes the
     sentinel in the same commit. Proven on a self-contained scratch origin (claim 1fa351c, stage
     93f505d, head == origin/main, dirty worktree refused). --help and --dry-run quoted in the body.

  3. WAKE GATE (mailbox-changed.sh) — +5 lines, the 0198 text verbatim, inserted before the
     inbox-non-empty test. Three production-script runs via MAILBOX_REPO/MAILBOX_GATE_LOG: A
     (half-done task, no sentinel) -> wakeAgent false = the hole reproduced; B (sentinel) -> reasons
     "resume-needed"; C (sentinel gone, inbox non-empty) -> "inbox-non-empty" still fires. The live
     repo gate reports "resume-needed inbox-non-empty".

  4. SCHEDULE — 15 4-17,21,22 * * 1-4 -> */15 4-17,21,22 * * 1-4, read back from the job store.
     next_run_at 2026-11-17T05:30:00-07:00 (future, sane; re-read after the prompt write). The HOURS
     already read 4-17,21,22 before this task (a DST correction, not mine); the cadence is what
     changed. Other three workers unchanged — recommendation only, no edit.

  5. PROTOCOL README — 1.3.0 merged via PR #6 after four Copilot passes (BLOCK, BLOCK, BLOCK, SHIP);
     all findings fixed in text, none rebutted. Section changes: Transitions (pickup sentinel +
     Blocked/Failed entries), Drain, Blocked, Parallel lane, plus the two one-line exceptions Rules 3
     and Front-matter needed. State representation unchanged.

  6. QUEUE — left exactly as found, but the card's premise is stale: 0199/0202/0204 were ALREADY
     staged by earlier fires and 0199 is not in active/; 0202 is not in inbox/. active/ =
     0142(superseded)/0159(decision-blocked)/0187(superseded), inbox/ = 0176(superseded, do not
     pick up)/0205. Nothing in those files was touched; 0205 is next.

  CORRECTIONS/DECLARATIONS: (a) the PR used the keyring OAuth login, not gh-env.sh — the PAT answers
  403 "Resource not accessible by personal access token" on pulls POST while the keyring gho_ token
  answers 422 (permission present); probed with an invalid head ref so nothing was created. (b) The
  sentinel was created at THIS task's pickup though item 6 says old rules (inert until the gate
  landed; deleted at staging). (c) --mode release is deliberately NOT in the protocol: as a new
  transition it would make 1.3.0 a MAJOR bump, so it stays a helper-only aborted-pickup rollback.
  (d) 0198's design text had no sentinel rule for a task parked blocked (permanent wake-forever);
  prompt and README clear it on every exit path. (e) The handoff-mailbox skill's PROTOCOL-MANAGED
  block was not edited — the card carries no delimited patch text, and paraphrase is where drift
  enters; the README is canonical. (f) mailbox-protocol-check.py --record was run after the merge so
  the next fire reads MATCH 1.3.0 instead of hunting a non-existent protocol-update task.

  PARALLEL LANE: still off. No second worker, no parallel_safe pickup, nothing reads the key.
---

# 0203 — STAGED: drain mode, 15-minute cadence, wake-gate sentinel — protocol 1.3.0 merged

**Branch:** `hermes/0203-protocol-1.3.0` @ `d5d68038bd37d7e42d66b41f95959f105fed0014` — on origin,
equal to `git ls-remote --heads origin` at staging time.
**PR:** [#6](https://github.com/pagosacabin/nordtronics/pull/6) — **MERGED** at
`68e1f59700d77e689c11fdadab55beffcef09fb7`, `mergedAt 2026-11-17T12:25:58Z`.
**Main now reads:** `protocol_version: 1.3.0` (`mailbox/README.md` line 3).
**CI:** Website Check [run 44082854335](https://github.com/pagosacabin/nordtronics/actions/runs/44082854335)
= **success at the branch tip** `d5d68038` (it is the only workflow that fires on this PR — the other
five carry path filters or branch allow-lists that `mailbox/**` does not match; `website-check.yml`
triggers on `pull_request -> main` with no path filter). It is a site link check, not a build of the
protocol text; the evidence for the text is the review plus the diff.

**Copilot peer review:** four passes on the PR, all hosted from the Herdr pane `t17` with the keyring
OAuth login (gh-env.sh NOT sourced for this — see "Correction 2" below).

| pass | verdict | what it found |
|---|---|---|
| [1](https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947876260) | **BLOCK** | BLOCKER: the blocked path never deletes `.resume-needed` (wake-forever); the cap unit (transitions vs tasks) was ambiguous; the blocked reply surface contradicted Rule 3; the parallel-lane fields were not in the front-matter allowlist |
| [2](https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947920898) | **BLOCK** | all four RESOLVED; two new SHOULD FIX/NIT: a rejected task has no sentinel, and "blocked" was not named as a terminal outcome |
| [3](https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947946671) | **BLOCK** | #6 RESOLVED; #5 still open — it did not accept "the first resume commit recreates it" without a wake trigger |
| [4](https://github.com/pagosacabin/nordtronics/pull/6#issuecomment-6947978957) | **SHIP** | all six RESOLVED at `d5d68038` |

The review was peer review as the card specifies (reviewer, not the proof). Every finding was fixed in
the text, none rebutted; the fixes are the four branch commits, not a rewrite.

## 1. Worker job prompt — before / after

### BEFORE
```
2. Else pick the lowest-numbered file in `mailbox/inbox/` with `status: inbox`.
   Pickup: `git mv mailbox/inbox/NNNN-*.md mailbox/active/NNNN-*.md`, set
   `status: in_progress`, `iteration: iteration + 1`, commit + push. Then do the
   work on a branch (never main), push, and confirm CI green at the branch tip.
3. If both `active/` and `inbox/` are empty of actionable tasks, stop and report:
   nothing to do.

Work ONE task per run. Iteration cap 5: at `iteration: 5` unverified, move to
`staged/` with `status: failed` and a failure summary in `notes` (hard stop).
```

### AFTER
```
2. Else pick the lowest-numbered file in `mailbox/inbox/` with `status: inbox`.
   Pickup, via the transition helper (it does the fetch-reset-test-commit-push
   with retry/backoff and NEVER force-pushes):

       python3 ~/.hermes/profiles/cronrunner/scripts/mailbox-transition.py \
           --mode claim --task NNNN --worker cronrunner/c0be50a686c6

   It moves `mailbox/inbox/NNNN-*.md` -> `mailbox/active/NNNN-*.md`, sets
   `status: in_progress`, `iteration: iteration + 1`, writes
   `claimed_by`/`claimed_at`/`lease_until`, and creates an EMPTY file
   `mailbox/active/.resume-needed`. Do the same by hand only if the helper
   cannot run (`git mv` + those front-matter fields + `touch
   mailbox/active/.resume-needed`). Then do the work on a branch (never main),
   push, and confirm CI green at the branch tip. Stage with
   `--mode stage --status staged|failed [--body-file <reply>]` so the sentinel is
   deleted in the SAME commit that stages the task. A task parked decision-blocked
   is not staged — delete `.resume-needed` in that parking commit too (use
   `--mode release` to put it back in inbox), or a stale sentinel wakes the worker
   every tick.

3. If both `active/` and `inbox/` are empty of actionable tasks, stop and report:
   nothing to do.

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

## 2. Helper script — `~/.hermes/profiles/cronrunner/scripts/mailbox-transition.py`

New, 12,170 bytes, sha256 `2a8c7c50fc4134cf6db89ac94f51e4c76bab67b07808e024cbd6de7279fe9b75`.
`--mode claim|stage` as specified, plus `--mode release` (an aborted-pickup rollback: `active` ->
`inbox`, `iteration` restored, sentinel cleared) and `--dry-run`. fetch -> reset -> test -> commit ->
push, 6 attempts with exponential backoff (2..32 s), **never** `--force`; a non-fast-forward push is
treated as losing the CAS and re-fetched. It refuses to run on a dirty tracked worktree (rule 17
debris) unless `--allow-dirty` is passed explicitly.

```
usage: mailbox-transition.py [-h] --mode {claim,stage,release} --task TASK
                             [--worker WORKER] [--status {staged,failed}]
                             [--body-file BODY_FILE] [--repo REPO]
                             [--attempts ATTEMPTS] [--dry-run] [--allow-dirty]

Claim/stage/release a mailbox task with fetch-reset-test-commit-push retry
(never force-pushing). Designed for the drain-mode worker (task 0198/0203).
```

Dry-run against a scratch origin (never the real repo):

```
$ python3 .../mailbox-transition.py --repo $RIG/w --mode claim --task 9003 --worker cronrunner/c0be50a686c6 --dry-run
claim 9003: mailbox/inbox/9003-synthetic.md -> mailbox/active/9003-synthetic.md
  status=in_progress iteration=1 worker=cronrunner/c0be50a686c6
  claimed_at=2026-11-17T12:17:09+00:00 lease_until=2026-11-17T13:47:09+00:00
  sentinel: mailbox/active/.resume-needed
```

Proven end-to-end on that scratch rig (self-contained bare `origin.git` + clone, the real remote never
referenced): claim wrote `status: in_progress`, `iteration: 1`,
`claimed_by/claimed_at/lease_until` and the sentinel in one commit (`1fa351c`); a second claim was a
no-op ("already claimed"); stage with `--body-file` moved it to `staged/` with `status: staged` and
deleted the sentinel **in the same commit** (`93f505d`, `mailbox/active/.resume-needed | 0` in the
deletion list), and `git rev-parse HEAD origin/main` matched; a dirty worktree was refused rather than
committed.

## 3. Wake gate — `mailbox-changed.sh`, +5 lines before the `inbox-non-empty` test

```bash
  # drain mode: a pickup leaves a durable resume marker so a half-done task is
  # wake-visible even though main == origin/main (task 0198).
  if [ -f "$REPO/mailbox/active/.resume-needed" ]; then
    reasons="$reasons resume-needed"
  fi
```

Proof runs, all three against the **production script** via its `MAILBOX_REPO`/`MAILBOX_GATE_LOG`
overrides (scratch rig: bare origin + clone, one half-done task in `active/`):

```
### A. half-done task in active/, NO sentinel (the pre-existing hole)
GATE: unchanged (main == origin/main, HEAD on main, mailbox clean) [2026-11-17T12:16:44Z]
{"wakeAgent": false}            <- the 0198 gate hole, reproduced

### B. same state + .resume-needed sentinel
GATE: mailbox changed -> resume-needed [2026-11-17T12:16:44Z]
{"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "resume-needed"}}

### C. sentinel gone, inbox non-empty (existing reason still fires)
GATE: mailbox changed -> inbox-non-empty [2026-11-17T12:16:44Z]
{"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "inbox-non-empty"}}
```

And on the **live repo**, with this task's own pickup sentinel in place:

```
GATE: mailbox changed -> resume-needed inbox-non-empty [2026-11-17T12:17:51Z]
```

B and C are the non-vacuity pair: B shows the sentinel alone wakes a task that A proves is otherwise
invisible, C shows the reason I inserted next to still fires.

## 4. Job schedule — before / after, and `next_run_at`

```
before:  15 4-17,21,22 * * 1-4      (hourly at :15)
after:   */15 4-17,21,22 * * 1-4    (verified, read back from the job store)
next_run_at: 2026-11-17T06:15:00-07:00  -> re-read after the prompt write: 2026-11-17T05:30:00-07:00
```

`next_run_at` is in the future and sane; no stale past value survived the edit (the trap in 0198).
Note the **hours** already read `4-17,21,22` before this task — that was a DST correction, not my
change; what this task changed is the cadence (`15` -> `*/15`). Scope kept to the Mon-Thu job
(c0be50a686c6); the Fri/Sat/Sun workers keep their current cadence. **My recommendation:** let them
match once a week of drain measurements confirms the Mon-Thu numbers — the same 75-minute cap and
3-task cap apply to all four, so the only reason they differ today is conservatism. I did not change
them.

## 5. Protocol README — 1.2.0 -> 1.3.0

Merged via the Copilot-reviewed PR above. Section changes, exactly as the card scoped them:

- **Transitions** — pickup bullet now creates `.resume-needed` and every exit path clears it; new
  **Blocked** and **Failed** transition entries.
- **Drain** (new subsection under Transitions) — three terminal outcomes (staged / failed / parked
  blocked), three tasks per fire, 75-minute stop, stop conditions checked only between tasks.
- **Blocked** (new section) — a blocked task stays in `active/`, its reply is its own body with no
  `proof`, the *park* commit records it and clears the sentinel, later runs leave it untouched, and
  **a blocked task ends the run**.
- **Parallel lane** (new section) — `parallel_safe: true`, CAS claim on `refs/heads/main`,
  `claimed_by`/`claimed_at`/`lease_until`, enablement trigger = two weeks of drain measurements and
  then a separate task. **Documented only; nothing is enabled.**
- **Rules 3** and **Front-matter** — the one-line exceptions those two sections needed once the above
  landed (blocked reply surface; the four lane fields reserved and forbidden while the lane is off).

No change to how a state is represented: same four directories, same `status:` values.

## Corrections and deviations (all declared, none silent)

1. **The card's item 6 premise is stale.** It says "0199 stays in_progress in active/; 0202 stays in
   inbox". By this run 0199 and 0202 (and 0204) were already **staged** by earlier fires; `active/`
   held 0142/0159/0187 (superseded / decision-blocked / superseded) and `inbox/` held 0176
   (superseded, "do not pick up") and 0205. I touched none of them: the queue is exactly as found,
   and 0205 is the next task for the next fire.
2. **The PR was made with the keyring OAuth login, not `gh-env.sh`.** In this profile the PAT cannot
   create PRs: probed with a deliberately invalid head ref, `gh api -X POST .../pulls` answered
   `403 Resource not accessible by personal access token` for the PAT, while the keyring `gho_` token
   answered `422 Validation Failed (head invalid)` — i.e. permission present, proving the endpoint
   works and the token choice is what matters. `x-accepted-github-permissions: metadata=read` on the
   repo endpoint corroborates the PAT's narrow grant. This is the same gap 0187 recorded; it is not
   the *credential* that is broken, it is the PAT's permission set.
3. **`.resume-needed` was created at this task's own pickup** even though item 6 says this pickup runs
   under the old rules. The file is inert until the gate change landed minutes later, and the staging
   commit deletes it. The next tick therefore starts with no sentinel and 0205 in the inbox.
4. **`--mode release` is not documented in the protocol.** It is the helper's aborted-pickup rollback
   (`active` -> `inbox` with `iteration` restored). I left it out of the README deliberately: as a
   *new transition* it would make 1.3.0 a MAJOR bump under the README's own semver rule, and 0203's
   scope is Step 1 only. It is documented in the helper's docstring; if Stephen wants it formal, it
   belongs in the version after 1.3.0.
5. **A defect in the design text, fixed in the implementation.** 0198's prompt replacement creates the
   sentinel at pickup and deletes it at staging — it has no rule for a task that is *parked blocked*,
   which would leave a permanent sentinel and wake the worker every tick. The prompt and the README
   now clear it on every exit path (staged, failed, parked); Copilot's pass 1 found the same hole
   independently, which is why the text reads as it does.
6. **The handoff-mailbox skill's PROTOCOL-MANAGED block was not edited.** The card assigns a protocol
   README change, not a skill patch, and carries no delimited patch text to apply verbatim (the
   protocol-update mechanic requires that text; paraphrasing it is where drift enters). The README is
   the canonical source, and the checker was re-recorded so the worker reads MATCH — see below.
7. **`mailbox-protocol-check.py --record` was run after the merge**
   (`PROTOCOL: RECORDED last_seen_protocol_version=1.3.0 in .../mailbox-worker-state.json`), and the
   checker now prints `PROTOCOL: MATCH protocol_version=1.3.0`. Without it the next fire would read a
   MISMATCH and hunt for a protocol-update task that does not exist.

## Parallel lane — still off

No second worker job exists, no `parallel_safe` pickup was performed, and nothing in the helper or the
gate reads that key. `hermes -p cronrunner cron list` shows the same four worker jobs as before, plus
the paused Nvidia poller; the only job edited is c0be50a686c6.

## Live queue — exactly as found

`active/`: 0142 (superseded), 0159 (decision-blocked), 0187 (superseded) — untouched, byte-identical.
`inbox/`: 0176 (superseded, do not pick up), 0205 (next up) — untouched.
`staged/`: unchanged apart from this task's own arrival; 0199/0202/0204 were already there.

## Next fire

The job now fires at :00/:15/:30/:45 inside 04:15-17:45 and 21:15-22:45 local, and its Step 2 carries
the drain. Expected first effect: 0205 (C1 move + silkscreen + GND pour on the 0199 branch) is picked
up at 05:30 MST without waiting for 06:15.
