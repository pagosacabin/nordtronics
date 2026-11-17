# Mailbox protocol — global rules

protocol_version: 1.3.0

## Protocol versioning

Semver. MAJOR = states or transitions change. MINOR = new rule or field.
PATCH = clarification only. Every version bump ships as a mailbox task written
in the previous format, carrying the exact skill-patch text to apply verbatim.

## Changelog

- 1.0.0 — 2026-09-23: baseline. Existing rules versioned; no behavior change.
- 1.1.0 — 2026-10-28: cost reporting. Staged replies carry a `cost`
  field (see "Cost format" below), reporting DeepSeek API usage summed
  across the task's API calls and the calculated cost in USD.
- 1.2.0 — 2026-11-07: model pool routing. Newton's two Antigravity weekly
  pools are used in order — Gemini Flash first, Claude/GPT second once the
  Gemini pool drops under 20% — with Hermes on DeepSeek as the floor (see
  "Model pool routing" below).
- 1.3.0 — 2026-11-17: drain mode. One worker fire may carry up to three
  **tasks** (at most six transitions — a pickup and one terminal transition each);
  pickup writes
  a `mailbox/active/.resume-needed` sentinel that the wake gate keys on and that
  staging, parking or blocking deletes; a blocked task ends the run; the parallel
  lane is documented and left off (see "Parallel lane").

The mailbox is a state machine. Task files move through directories; every
transition is a `git mv` + commit + push. Never copy a task file — move it.

## States

- `mailbox/inbox/` — new tasks waiting for pickup. Front-matter: `status: inbox`
- `mailbox/active/` — picked up, work in progress. `status: in_progress`
- `mailbox/staged/` — work done, awaiting verification. `status: staged`
- `mailbox/archive/` — verified complete. `status: verified`. Only Juno moves files here.

## Transitions

- Pickup: `inbox` → `active`. Set `status: in_progress`, increment `iteration` by 1, and create an
  empty `mailbox/active/.resume-needed` in the same commit, then push. The sentinel is the durable
  "this run intends to continue" marker: the worker's wake gate keys on it, so a tick that dies
  mid-task is still wake-visible even though `main` == `origin/main`. Delete it in the same commit
  that stages the task, stages it as `failed`, or parks it blocked — every exit path clears it, or a
  stale sentinel wakes the worker every tick.
- Done: `active` → `staged`. Set `status: staged` and fill in `proof` (below), delete
  `.resume-needed`, commit + push.
- Blocked: the task stays in `active/` with `status: in_progress`; delete `.resume-needed` in the
  same *park* commit (see "Blocked"), which is the only commit a blocked task makes.
- Failed: `active` → `staged` with `status: failed` at the iteration cap; delete `.resume-needed`.
- Verified: `staged` → `archive`. Juno only, after independent verification.
- Rejected: `staged` → `active`. Juno attaches `notes` explaining what failed verification.
  `iteration` does NOT increment. The staging commit that sent the task to `staged/` deleted
  `.resume-needed`, so the worker's first **resume commit** for a rejected task recreates it — a
  rejected task is work to continue, and without the sentinel the wake gate cannot see it once
  `main` has caught up with origin.

### Drain

One fire may carry more than one transition. A task ends in one of three terminal outcomes —
**staged**, **failed** at the iteration cap, or **parked blocked** — and after any of them the worker
returns to the top of its pickup rule and takes the next one, until one of these stops the run:
`inbox/` holds no task with `status: inbox`; three tasks have reached a terminal outcome in that run;
the task just finished was parked **blocked** (a decision is owed, and picking up another task would
bury it); the task just finished was staged `status: failed`; or more than 75 minutes have elapsed.
Stop conditions are checked only between tasks — a task is never abandoned mid-work to obey the
cap. The four directories and the `status:` values are unchanged, and the staged file is still the
reply for every task that gets finished; what 1.3.0 changes is only how many tasks one run may
carry, plus the sentinel that makes a half-done one visible.

## Blocked

Blocked is not `staged`, and it is not failure. A task that hits a blocking failure that needs a
decision stays in `active/` with `status: in_progress`, the exact failing output and every deviation
in `notes`, and a reply body saying plainly that there is no `proof` block.

The run that finds the blocker does commit **one** transition — the *park* commit. That commit
deletes `mailbox/active/.resume-needed` (otherwise the stale sentinel wakes the worker every tick
forever), records the blocker and its failing output in `notes` plus the reply body, and adds no
`proof`. The task file itself is what carries the record; what "left untouched" forbids is a later
run re-opening it. **A decision-blocked task is not resumable: later runs do not re-investigate it,
do not edit its `notes`, do not touch `iteration`, and do not re-declare the blocker.**

A blocked task **ends the run**. The decision is owed, so the worker does not pick up another task
in the same fire — doing so would bury the decision under fresh work.

## Front-matter

Every task file starts with YAML front-matter. Only these fields:

- `task_id`: e.g. `"0031"` — quote it. Unquoted `0031` parses as octal in YAML 1.1.
- `status`: inbox | in_progress | staged | verified | failed
- `iteration`: integer, starts at 0
- `proof`: pointers to durable evidence (branch tip SHA, Actions run URL). Required on `staged`.
- `notes`: freeform. Juno's verification feedback goes here on rejection.
- `cost`: DeepSeek API usage and calculated cost for the task. See
  "Cost format" below. Required on `staged` from 1.1.0 onward, or
  `unknown` with a plain reason if the harness cannot report per-call
  usage.

Reserved for the parallel lane (see "Parallel lane"), and forbidden while it is off: `parallel_safe`,
`claimed_by`, `claimed_at`, `lease_until`. A single-lane worker writes none of them; they become part
of this allowlist only with the task that enables parallel pickup.

The body stays freeform markdown — specs, context, reply format.

## Proof format

Write `proof` as a list so the verifier can check each pointer. Canonical form:

```yaml
proof:
  - branch: android-toolchain-setup
    sha: 92a5b2b44bf30a0fde9eb4dfc72ca25c8692d299
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35583228091
```

- `branch` + `sha`: the branch must exist on origin and its tip must equal `sha`.
- `run`: a real Actions run URL. The verifier checks its status/conclusion and
  that its head SHA matches the claimed `sha` and the current branch tip.
- A green run with matching pointers verifies the task. A red run, a wrong SHA,
  or a run whose head no longer matches the branch tip fails it.

Build tasks (the deliverable is a CI-built artifact — APK, firmware, image):
proof additionally requires a build-green ntfy notification. Once the run is
green, publish to the ntfy build topic named in the task spec
(`https://ntfy.sh`) and include the publish receipt (topic, timestamp) in the
reply. A green build Stephen never hears about is a failed handoff.
(Clarified 2026-09-23: the 0045 companion-app build went green and staged with
full proof, but no ntfy notification went out because the spec never asked for one.)

The verifier is `~/workspace/hermes-tools/verify-staged.py` on Juno's side —
it checks every pointer against the live GitHub API. Pasted terminal output
is not proof and is ignored.

## Cost format

Write `cost` in the staged front-matter, summed from the DeepSeek API
usage objects of every API call the task made — not from cumulative
dashboard totals:

```yaml
cost:
  model: deepseek-flash
  api_calls: 12
  input_tokens: 85000
  input_cache_hit_tokens: 40000
  input_cache_miss_tokens: 45000
  output_tokens: 12000
  total_tokens: 97000
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.014
```

- `model`: the model actually used (deepseek-flash by default,
  deepseek-v4-pro only when the task named it).
- `rate_window`: off-peak | peak | mixed, decided per call timestamp
  (peak = 01:00-04:00 and 06:00-10:00 UTC weekdays).
- `cost_usd`: input_miss x miss rate + input_hit x hit rate +
  output x output rate + search_requests x $0.00075. State the rates
  used; do not copy a rate card blindly, rates change.
- If the harness cannot see per-call usage, set the token and cost
  fields to `unknown` and state why in `notes`. Never estimate a
  cost and present it as API usage.
- Juno verifies the arithmetic (tokens x rates = cost_usd) before
  archive, the same as branch SHA / run URL verification.

## Model pool routing

Newton's Antigravity CLI has two independent weekly quota pools — Gemini
models and Claude/GPT models. Use them in order, not all at once, with
DeepSeek as the floor.

- **Order of pools.** Newton runs on Gemini Flash first: the daily sweep,
  light research, and light code. When the Gemini pool drops under 20%
  remaining, switch Newton to the second pool for the same jobs. Hermes on
  DeepSeek is the floor: paid, pennies per task, never runs out in practice,
  and the only one that can touch the bench.
- **Model roles.** Gemini Flash is the volume model. Claude Sonnet 4.6 is the
  preferred model on pool two and the pick for harder coding jobs. Claude
  Opus 4.6 thinking is for the hardest jobs only — the task names it, or
  Sonnet has already failed on the same job. GPT-OSS 120B is the light option
  on pool two. Lightest model that does the job. Per-model burn is unmeasured
  — 0171 measures the Flash sweep; revisit the roles if a later measurement
  shows otherwise.
- **Coding reserve.** Newton is the coding reserve for pure software —
  scripts, tools, code review, research, drafting. Bench firmware
  (ESP32/ESPHome/PlatformIO that must be flashed and measured) is Hermes's by
  default: he is the only hands, and Newton drafting firmware that Hermes then
  has to flash and prove is two steps where Hermes writing it himself is one.
  Newton may draft firmware only when the task explicitly says so, and Hermes
  proves it on the bench before it counts.
- **Read both gauges before every Newton run.** `agy -p "/usage"` costs no
  agent turn and no quota. Read both gauges and pick the pool with room; never
  guess which pool has room.
- **Quota stop.** On 429 / `RESOURCE_EXHAUSTED` / "Individual quota reached",
  do NOT retry in a loop — report the pool, the "Resets in" time, and stop.
  Note whether the other pool still has room. An exhausted pool does not fail
  fast: `agy` retries with backoff until the print timeout, so looping burns
  minutes, not quota. Read the timer: under 5h means the five-hour window
  (wait it out); over 5h means the weekly cap (switch pools). If neither pool
  has room, report both reset times and run on DeepSeek until the next reset.
- **Availability.** Never assume a model is there or gone from memory or web
  reports — verify live with a probe call (`agy -p` with `--model`, one-word
  reply). The Nov 2 removal reports said Claude 4.6 and GPT-OSS were off the
  free tier; live calls on 2026-11-07 proved both work. A model list is not
  proof; a live reply is. If a model is gone, report it and fall through to
  the next pool — do not silently substitute a different model.
- **Headless, not interactive.** Jobs run headless via Hermes, not
  interactively. Interactive Newton turns are for questions and checks — they
  burn the pool unmeasured (4 points in 15 minutes of Q&A on 2026-11-07).
- **Record switches.** Record any pool or model switch in the staged reply
  notes, so the choice is visible.
- The 20% switch point is a first guess, revisited once 0171 measures the real
  per-sweep cost.

## Parallel lane

Documented in 1.3.0, **not enabled**. The current wiring starts exactly one worker; nothing reads
`parallel_safe` yet, and no parallel pickup is performed.

- A second worker may take a task concurrently only when that task's front-matter carries
  `parallel_safe: true`. A task without the key is single-lane by default.
- A lane worker claims through an optimistic compare-and-swap on `refs/heads/main` —
  fetch → reset → test → commit → push — retrying a rejected push with backoff and **never
  force-pushing**. The claim records `claimed_by`, `claimed_at` and `lease_until` in the
  front-matter, so a claim is visible on origin and expires on its own.
- **Enablement trigger:** two weeks of drain measurements after 1.3.0, then a separate task that
  turns it on. Until that task lands, this section is a specification only.

## Rules

1. Iteration cap: 5. `iteration` increments only on pickup (`inbox` → `active`). If a task reaches `iteration: 5` without being verified, move it to `staged` with `status: failed` and a failure summary in `notes`. Hard stop — no further automatic retries.
2. Proof bar: `staged` requires `proof` pointers, not pasted output. Pasted terminal text is a claim; Juno verifies the pointers independently (branch SHAs, CI run state, artifacts). A fabricated or placeholder pointer fails the task.
3. Replies live in the task file. There is no separate outbox — moving the file to `staged/` with `status: staged` IS the reply. The one exception is a **blocked** task: its reply is the body of its `active/` file (see "Blocked"), which carries no `proof` block and keeps `status: in_progress`. A blocked reply is never `staged` and never `failed` — those mean work was done, or that the retry budget ran out.

## Task spec format (how Juno writes tasks)

Every task body uses these sections, in this order:

1. **Context** — what exists and what has already been verified. Front-load exact
   branch names, file paths, and prior results.
2. **Task** — one concrete deliverable. One task = one artifact.
3. **Success criteria** — falsifiable conditions that must be true when done.
4. **Constraints** — security, branch, hardware, and scope boundaries. State
   positively ("include X", "push only to branch Y") instead of long negative lists.
5. **Proof** — the exact pointers Juno will verify (branch + SHA, run URL).
   Pasted terminal output is a claim, not proof.
6. **Reply format** — the exact structured fields Hermes fills in when staging.

From the Hermes Agent framework research (2026-09-21): positive instructions beat
negative lists, structured sections beat walls of prose, exact names beat
descriptions, one deliverable per task. Keep tasks small enough to interrupt drift
early.

Never assume Hermes's local installation has any framework feature configured
(skills, memory, profiles, gateways) — only what he has proven on his own machine.
