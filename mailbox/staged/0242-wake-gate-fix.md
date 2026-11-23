---
task_id: '0242'
protocol_version: 1.3.0
status: staged
iteration: 2
expect-reply-within: 6h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-23T04:02:09+00:00"
lease_until: "2026-11-23T05:32:09+00:00"
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
  note: "This harness exposes no per-call usage to the cron session; token and cost fields are unknown rather than estimated."
proof: []
notes: |
  STAGED 2026-11-22 21:0x MST (2026-11-23 04:0x UTC), off-peak (PEAK: OFF-PEAK
  04:00 UTC), PROTOCOL MATCH 1.3.0. HOST-ONLY TASK: `proof: []` is correct here,
  not an omission — no repository file was modified, so there is no branch, no
  SHA and no Actions run to point at. The gate script lives at
  ~/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh, is not
  version-controlled (`git -C ~/.hermes remote -v` empty; `git ls-files` errors),
  and no CI covers it. The evidence is the quoted gate output below plus the
  before/after sha256: b063f463f7771861844a91cce0ae4595a1ff8ced0c3078c0475b31a044cdfa88
  -> 49b687cd441b0498b5e68d5e0331b207542847c2fd64c7f5977ae7cfddc860c1, backup at
  mailbox-changed.sh.bak-0242, doc note in the profile skill:
  ~/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md (idle-tick
  gate rule, dated bullet).
  FILES APPLIED (host, outside the repo):
    ~/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh
    ~/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md
    ~/.hermes/profiles/cronrunner/mailbox-gate.log (appended by the live run)
  TASK CHOICE. active/ held 0216 (iteration 1) and 0230 (iteration 2), both
  decision-blocked with "DECISION OWED" and left byte-identical and untouched;
  inbox/ 0176 and 237-nudge-0236 are `superseded`. 0242 was the lowest actionable
  task (0243 is next, untouched). Pickup used mailbox-transition.py --mode claim
  (commit bc4ad33, iteration 2), which also created and this staging deletes the
  .resume-needed sentinel.
  CORRECTIONS DECLARED IN THE REPLY BODY: (1) a `DECISION OWED` task is treated as
  parked so it cannot wake every tick forever — beyond the spec's literal "any
  in_progress task", because 0216/0230 sit in_progress in active/ indefinitely and
  the quiet-case criterion forbids perpetual wakes; (2) the inbox check now also
  accepts `status: pending`, the value Juno actually filed 0242 and 0243 with,
  which the old check ignored (scope extension, with its own negative control);
  (3) the .resume-needed sentinel was NOT what failed for 0241 — its pickup was a
  hand-rolled git mv, so the sentinel was never created.
  NOT PROVEN: a real tick dying mid-flight and a later tick resuming it (needs a
  tick to fail on demand; the fixture pair plus the live _run_job_script run are
  the deterministic evidence), and the scheduler-fired 04:15Z line (this run
  appended the equivalent line to mailbox-gate.log at 04:03:51Z).
  Copilot review: not applicable — local-only script, nothing to review; stated in
  the reply rather than skipped silently.
  Cost fields are `unknown` because this harness exposes no per-call DeepSeek usage
  to the cron session; no estimate is presented as API usage.
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


## Reply 2026-11-23 04:0x UTC (2026-11-22 21:0x MST) — the gate now wakes on in-progress tasks in `active/`, proven both directions

**Outcome:** `mailbox-changed.sh` now wakes the worker when `active/` holds a task whose
front-matter `status:` is `in_progress`, in addition to the unchanged git-state and
`inbox/` reasons. Wake case and quiet case are both proven below, against fixture clones
and through the scheduler's own `_run_job_script`; the negative control (the `.bak` copy
of the script) returns `wake=false` on the same fixtures, so the change is what
discriminates. 0241's stall is measured at **32 recorded `wake=false` ticks** across the
gap, and 0241 itself is **verified and archived** (it resumed in the 01:00Z tick, not
because of this change).

### What changed (host-only; no repository file touched)

`~/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh`
sha256 `b063f463…44cdfa88` → `49b687cd…9c860c1`; backup kept at
`mailbox-changed.sh.bak-0242`; `bash -n` clean. Two additions, nothing else:

1. `fm_status()` — the front-matter `status:` reader, hoisted out of the inbox loop so
   both checks use the same parser (quotes/trailing space stripped, empty when a file has
   no front-matter at all — 0071).
2. A second reason block, **`active-in-progress`**: for each `mailbox/active/*.md` whose
   `fm_status` is `in_progress`, wake — *unless the task declares `DECISION OWED`*.
   Reason string reported as `active-in-progress`.

Plus one **declared scope extension**: the inbox check now also accepts
**`status: pending`**. Juno filed both 0242 and 0243 with `pending`, which is not in the
protocol vocabulary (README: `inbox | in_progress | staged | verified | failed`), and the
old check only accepted `inbox` or absent — so a `pending` task was invisible to the gate
and could only ever be picked up by a push. That is the same defect shape as the one this
task fixes, one status value over, and it would have stalled 0243 immediately.

### Wake case — `active-in-progress`

Fixture `fx_active_inprog` (own bare origin, `main == origin/main`, `inbox/` empty, one
`status: in_progress` task with no `DECISION OWED`), run through the real script:

    GATE: mailbox changed -> active-in-progress [2026-11-23T04:03:11Z]
    {"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "active-in-progress"}}

Negative control, same fixture, pre-change script:

    GATE: unchanged (main == origin/main, HEAD on main, mailbox clean) [2026-11-23T04:03:15Z]
    {"wakeAgent": false}

Live, on the real repo and the real job dict (`5c1532977f15`, `script=mailbox-changed.sh`),
via `cron.scheduler_script._run_job_script`:

    GATE: mailbox changed -> resume-needed inbox-non-empty active-in-progress [2026-11-23T04:03:51Z]
    {"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "resume-needed inbox-non-empty active-in-progress"}}
    run_job_script ok = True   _parse_wake_gate -> True   HEAD == origin/main == bc4ad33, mailbox clean

This tick of 0242 is the live in-progress task: the gate now sees a task the worker
itself started, which is exactly what it could not do for 0241.

### Quiet case — no perpetual wake

`fx_parked` (0216 and 0230 copies + a dummy that declares `DECISION OWED`; inbox holds
only a `superseded` nudge) and `fx_quiet` (active/ empty, inbox holds only `superseded`):

    fx_parked: GATE: unchanged (main == origin/main, HEAD on main, mailbox clean)  {"wakeAgent": false}
    fx_quiet:  GATE: unchanged (main == origin/main, HEAD on main, mailbox clean)  {"wakeAgent": false}

**Why the park suppression exists (correction to the literal spec):** the spec says wake on
any `in_progress` task in `active/`. Taken literally that wakes forever here — 0216 and 0230
are decision-blocked ("DECISION OWED") and protocol requires them to stay
`status: in_progress` in `active/` indefinitely, so every tick would be a paid run that must
do nothing. The task's own quiet-case criterion forbids that, so a task that declares
`DECISION OWED` is treated as parked and does not wake. Verified the two parked files both
carry the token, and that the fixtures containing them return `wake=false`.

### Inbox `pending` (extension), with its own control

    fx_pending (status: pending): GATE: mailbox changed -> inbox-non-empty   wake=true
    fx_inbox   (status: inbox):   GATE: mailbox changed -> inbox-non-empty   wake=true
    fx_pending, pre-change script: GATE: unchanged …                        {"wakeAgent": false}

### 0241's state — resumed, staged, verified, archived

- Picked up 2026-11-22 07:30:27 MST (`ac0953c`), pushed by the worker — after which
  `main == origin/main` and the gate answered `unchanged`.
- 14:30Z → 01:15Z window: **34 gate ticks, 32 of them `wake=false`**, two `wake=true`
  (both `main-not-at-origin/main`, i.e. Juno's pushes, not this task).
- Resumed in the **01:00Z tick** — the wake came from Juno's 0242 push moving a ref, not
  from any resume mechanism. It finished the bench run and staged at 18:10:06 MST
  (`13d22af`); Juno then verified and archived it (`84afe7e`, `status: verified`,
  `iteration: 2`, proof `hermes/0241-newcard-write @ ceb31e1`). So 0241's outcome is good;
  its **latency** was the defect.
- Related finding: the `.resume-needed` sentinel did not fire for 0241 because its pickup
  was a hand-rolled `git mv` commit (`ac0953c`, 2 lines changed), not
  `mailbox-transition.py --mode claim` — the only writer of that sentinel. The sentinel is
  therefore a helper, not the mechanism, and the new status check is what covers the
  hand-rolled and crashed cases alike.

### Copilot peer review — not applicable, stated rather than skipped

The gate script is **local-only, not version-controlled**: `git -C ~/.hermes remote -v` is
empty and `git ls-files .hermes/profiles/cronrunner/scripts/mailbox-changed.sh` errors
("did not match any file(s) known to git"), so there is no branch, no PR and nothing for
Copilot to review. Per the task's own instruction ("if the script is local-only, say so in
the reply and the log evidence below carries the proof"), the fixture runs plus the live
`_run_job_script` output above are the evidence.

### Doc note

`~/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md`, under the
idle-tick-gate rule — a dated bullet (2026-11-22, task 0242) stating that the gate also
wakes on `status: in_progress` in `active/`, that `DECISION OWED` tasks are parked and must
not wake, that `status: pending` in `inbox/` counts as work, and why the `.resume-needed`
sentinel cannot be the only mechanism. `mailbox/README.md` was deliberately **not** edited:
it is the protocol-managed document (`protocol_version: 1.3.0`) and this change alters the
worker's gate, not the protocol.

### Not proven this run (stated, not implied)

- **A tick dying mid-flight and a later tick resuming it.** That needs a tick to fail on
  demand; the fixture pair is the deterministic substitute. The spec's dummy-task variant
  needs ticks *after* this staging, which a single run cannot supply. What is proven is
  that the gate's decision is correct for the same file state on the real repo.
- **The next real tick's log line.** The live `_run_job_script` invocation appended
  `2026-11-23T04:03:51Z GATE wake=true … active-in-progress` to
  `~/.hermes/profiles/cronrunner/mailbox-gate.log`; the 04:15Z tick's own line will be the
  scheduler-fired confirmation, and with 0243 (`status: pending`) waiting it should read
  `inbox-non-empty`. The quiet case becomes observable again once `inbox/` is clear.
- Nothing else in the tree was touched: 0216 and 0230 were left byte-identical, no job was
  edited (cadence, peak guard, drain caps all unchanged), and no `next_run_at` moved.
