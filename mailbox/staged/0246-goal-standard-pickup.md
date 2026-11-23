---
task_id: "0246"
title: Goal mode through the standard pickup — find the mechanism, prove it host-only
status: staged
protocol_version: 1.3.0
iteration: 2
expect-reply-within: 24h
completed_at: "2026-11-23T18:52:00+00:00"
proof: []
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
  DONE — research reply; host-only, no repository file changed, so `proof: []` (the task's
  own Assignment says no GitHub-bound change is expected, and its success criteria ask for
  evidence and a proposed standard, not branch pointers).

  VERDICT: standard-pickup goal execution is NOT available natively on v0.23.2. The worker
  is a cron job; cron carries no goal mode (`grep -rni goal cron/` = 0 hits, no goal flag on
  `hermes cron create`, one `agent.run_conversation` per fire at cron/scheduler.py:1984).
  The only judge loop is `run_kanban_goal_loop`, reachable only by a kanban worker, and it
  takes no gate.

  CORRECTION to 0235 #4, measured not read: the kanban route is not entirely gate-free.
  `--completion-contract` is a fail-closed completion gate inside `complete_task`
  (kanban_db.py:2667). Scratch-board probe: contract `pagosacabin/nordtronics` -> complete
  REFUSED (`classification: auth`, card stayed `ready`, `pr_acceptance {'ok': False ...}`);
  contract `local-only` -> `Completed`, `status: done`, no acceptance event. So the gate is
  real but only GitHub CI can judge it — host-only work gets `local-only` and no gate.

  HOOKS, corrected against the briefs: shell hooks DO reach cron workers in this build —
  `cron/scheduler.py:3897-3901` registers the owning profile's `hooks:` block via
  `GatewayStartupMixin._register_config_hooks` (#131764; test
  tests/cron/test_external_worker_config_hooks.py). None is configured on either profile
  (`hermes hooks list` = "No shell hooks or outbound webhooks configured"), and a hook is a
  per-tool policy block, not a done-gate: no hook event judges completion or continues a goal
  except `pre_verify`, which fires only after file edits and defaults OFF (`agent.verify_on_stop`
  unset).

  GATE PROOF (host-only, no model): two-file scratch job, gate `python3 -m unittest discover`
  through `hermes_cli.goals.run_gate` — broken state `ok=False rc=1`, fixed state `ok=True rc=0`.
  Non-vacuous, so a future "gate passed" claim means something. Artifacts under
  ~/.hermes/profiles/cronrunner/cache/scratch/0246/ (gate-demo/, kh/ kanban probe board).

  RECOMMENDED STANDARD: front-matter `run_as_goal: true` + `goal: {max_turns, contract, gates,
  github_contract}`; worker creates the kanban card with `--goal`, waits, runs `gates` itself
  before staging and refuses to stage on failure. Zero engine change; needs a README protocol
  bump (front-matter allowlist is closed) — README wording drafted in the reply, NOT edited,
  per the task. Durable alternative: thread `gates` into `run_kanban_goal_loop`.

  NOT PROVED, declared: no live goal run through the pickup (needs the README field, which the
  task forbids me to add, plus a later tick); no hook fired at runtime (none configured); no
  branch/CI run (no repo file changed; the six workflows on main all trigger on push/paths).
  Cost is `unknown`: the cron worker's harness exposes no per-call usage objects to the agent —
  stated rather than estimated, per the protocol.
---

## Assignment

- **Hermes:** all of it — the research on his own install, the host-only proof if a mechanism exists, the staged reply.
- **Copilot:** not assigned — no GitHub-bound code change is expected. If the work turns into one, stop and say so.
- **Newton:** not assigned.
- **Juno:** verifies the reply against the claims that can be checked from the mailbox side (the proof task's lifecycle, quoted outputs) and puts the proposed standard to Stephen.

## Context

Task 0235 (Nov 21) established the split: the interactive `/goal` route in Hermes v0.23.2 runs a written contract, a judge loop, and a real gate — while the kanban `--goal` route the mailbox worker would use is **judge-only, with no gate hook**.

This morning's 0244 run showed the cost of that split: in interactive mode every leg (review, fix pass, second pass) waited for Stephen to say "go," because nothing chained them. 0245 is running as a hand-driven interactive goal to cover today's bench work. Stephen's direction now: **find out how a goal-mode run happens through the standard pickup** — a task filed in the inbox, claimed by the normal worker cycle, executed as a goal with its contract and gate — so "run as a goal" can become a standard marking in task specs instead of an interactive detour.

Related open thread: the Nov 23 daily learning found Hermes has shell hooks that can block tool calls fail-closed and inject context, documented as of v0.23.3, with whether they fire inside cron worker runs unstated. The v0.23.3 update itself is **parked** at Stephen's word and stays parked.

## Task

1. **Map the standard pickup as it actually runs on this machine.** The worker jobs, the cronrunner profile, the kanban commands the cycle invokes, and where a goal contract and gate would have to attach. Evidence from the machine — help output, the profile skill, config files — not documentation alone.
2. **Pin down the kanban `--goal` route in this install.** What it accepts, where the judge gets its criteria, whether a contract can be supplied, and whether the "no gate hook" finding from 0235 is the whole story or whether a config/flag on this version adds one. The installed version's own help and, if needed, its source on this machine are the authority.
3. **Check the hook systems in this install.** Can a shell hook serve as the gate on the worker route (verify or block between turns)? If hooks are not present or do not fire in worker runs on v0.23.2, say so plainly. "Impossible without the parked update" is a valid finding — report it, do not install anything.
4. **Proof, host-only, if a mechanism exists.** One small multi-step host-only job (for example a two-file edit with a build or test gate), filed as a normal inbox task carrying the proposed run-as-goal marking, picked up by the standard worker cycle **untouched by hand**, executed as a goal, with the gate demonstrably firing — a deliberately failing intermediate state caught, or the gate's pass logged. If no mechanism exists, skip the proof and say why.
5. **Recommend the standard.** The exact task-file marking (front-matter field name and values), any worker-side change it needs, and what Juno writes in future specs to invoke it. If the recommendation changes the mailbox protocol, draft the README wording in the reply but do not edit the README — protocol changes are Stephen's call.

## Success criteria

- The staged reply states, with machine-level evidence quoted, whether standard-pickup goal execution is possible on this install today.
- If possible: the host-only proof ran through the untouched standard cycle, and the reply quotes the gate firing.
- If not possible: the reply names the precise blocker (route limitation, version, missing hook) and the smallest change that would unblock it.
- Either way: a concrete proposed standard — the field, where it lives, what the worker does with it.

## Constraints

- Host-only proof. No hardware, no flashing.
- Do not install the v0.23.3 update, or any update.
- Do not change the live worker jobs' standing behaviour beyond the single proof pickup. Structural changes are recommendations for Stephen, not changes.

---

## Reply

### Verdict

**Standard-pickup goal execution is not available natively on this install** (v0.23.2,
local `ae2a81ccc00e`). The mailbox worker is a cron job, and cron has no goal mode at all:
zero occurrences of "goal" in `cron/` (`grep -rni goal cron/ --include=*.py` → no output),
no goal flag on `hermes cron create --help`, and the job body runs exactly one
`agent.run_conversation(prompt)`:

```
$ sed -n '1984p' cron/scheduler.py
        _cron_context.run, agent.run_conversation, prompt, task_id=task_id)
```

Everything a tick does — claim, work, stage — happens inside that one conversation. The
"loop" is the agent's own tool-call loop, bounded by `max_iterations`; there is no turn
boundary afterwards for a judge or a gate to sit on.

The pieces for a goal run do all exist, and one of them is a gate 0235 did not see. The gap
is a protocol field plus a paragraph of worker wording — **not** a version update.

### 1. The standard pickup as it actually runs here

- Job `c0be50a686c6` "Mailbox worker (DeepSeek) Mon-Thu", `*/15 * * * 1-4`, model
  `deepseek-flash`, skills `github, kicad-mcp-server, handoff-mailbox`, script
  `mailbox-changed.sh`, workdir `/home/astroboy/nordtronics`.
- The fire is handed to an **external worker process**, not run in-process. This very tick is
  pid 905093:

  ```
  /home/astroboy/.hermes/tools/python-3.14.9+20261021-linux-x64/bin/python3 -m cron.scheduler \
    --external-worker-file .../profiles/cronrunner/cron/external-workers/6fa644397b4b4837be46af5ebc0c8c3c.json \
    --ack-file .../6fa644397b4b4837be46af5ebc0c8c3c.ready
  ```

  (`cron/scheduler.py:3599` `_launch_external_cron_worker`: "outside a managed systemd gateway
  … in-process path. In managed topology the job always goes to an external worker".)
- That worker's bootstrap is: profile-home override → `discover_plugins()` → hydrate secret
  sources → register the owning profile's `hooks:` block → `run_one_job(job)`
  (`cron/scheduler.py:3880-3935`).
- Attach points for a contract, therefore: the **prompt** (already free-form markdown) and
  the **staging step** (the worker itself decides when the task becomes `status: staged`).
  Nothing sits between them.

### 2. The goal routes that exist, and the gate each carries

| Route | Loop | Deterministic gate | Reachable from the standard pickup |
|---|---|---|---|
| Interactive `/goal` (CLI/gateway session) | judge after each turn | **yes** — `/goal gate add <cmd>`, exit code checked before DONE | no — `GoalManager` is per live session ("The CLI and gateway each hold one per live session", `hermes_cli/goals.py:1090`); never instantiated in `cron/` |
| `hermes kanban create --goal` | `run_kanban_goal_loop` judges the card's title+body after each turn | **none per-turn** — signature is exactly `task_id, goal_text, run_turn, task_status_fn, block_fn, max_turns, first_response, log` (inspected) | no — only a kanban worker gets `HERMES_KANBAN_GOAL_MODE=1` (`gateway/session_kanban.py:171`) |
| kanban `--completion-contract OWNER/REPO` or a PR URL | — | **yes, at completion** — `complete_task` refuses unless exact-head CI is green | only through a kanban card |
| cron worker (this route) | none | none | — |

**Correction to 0235 finding #4.** The kanban route is *not* entirely gate-free.
`--completion-contract` is a fail-closed completion gate wired into `complete_task`
(`hermes_cli/kanban_db.py:2667` → `prepare_acceptance` / `record_acceptance`). Proven on a
scratch board, no model call, two cards:

```
card t_3c3bc243  --completion-contract pagosacabin/nordtronics  ->  complete:
  cannot complete t_3c3bc243 (unknown id or terminal state)
  card status: ready
  event: pr_acceptance {'ok': False, 'classification': 'auth', ... 'detail':
         "GitHub refused the acceptance read (assignee profile 'cronrunner' cannot be resolved)"}

card t_4e6ccf0f  --completion-contract local-only  ->  complete:
  Completed t_4e6ccf0f        card status: done      (no pr_acceptance event)
```

Read the shape, not the reason: any contract other than `local-only` **blocks completion**
until acceptance passes; `local-only` — every host-only task — returns `None` from
`prepare_acceptance` (`kanban_pr_acceptance_store.py:18`) and closes with **no gate at all**.
The kanban gate exists, but only GitHub CI can judge it.

### 3. Hooks: they do reach worker runs, and they are not a loop

The brief said hooks may not fire in cron worker runs. **On this build they do.** The
external worker registers the owning profile's `hooks:` block explicitly:

```
cron/scheduler.py:3896-3901
        # This process never ran gateway startup, so the OWNING profile's ``hooks:`` block (shell
        # hooks + ``hooks.outbound``) was never registered and cron sessions silently lost it
        # (#131764). Same helper the gateway uses per profile ...
        GatewayStartupMixin._register_config_hooks(
            "Cron external worker: config hook registration failed", level=logging.WARNING)
```

with a shipped regression test (`tests/cron/test_external_worker_config_hooks.py`, whose
docstring describes exactly this bug and fix). A `pre_tool_call` shell hook can block
fail-closed (exit 2 or a JSON action — `agent/shell_hooks.py:44,642-689`), and `pre_verify`
can hold a turn open after file edits.

But on **this host none is configured**: `hermes hooks list` → "No shell hooks or outbound
webhooks configured in ~/.hermes/config.yaml", and neither `~/.hermes/config.yaml` nor
`~/.hermes/profiles/cronrunner/config.yaml` has a `hooks:` key. And a hook is the wrong
shape for what the card asks: it is a **per-tool policy** block, not a **done** gate. No hook
event judges completion; `pre_verify` only nudges after edits, and `agent.verify_on_stop`
(the one config that would demand fresh verification evidence before finishing) is unset and
defaults OFF. A hook could have stopped 0235's goal worker from flashing a board — that is
worth having — but it cannot decide "this task is complete".

Two adjacent mechanisms already in the install, both unused here: `hermes cron create
--continuity` (injects the job's previous output, i.e. memory, not a judge) and
`agent.verify_on_stop` (nudge, not verdict).

### 4. Host-only gate test — the discriminating half

Two-file job in scratch (`pkg/topic.py`, `tests/test_topic.py`), gate command
`cd <dir> && python3 -m unittest discover -s tests`, executed through
`hermes_cli.goals.run_gate` — the same function the interactive gate path uses:

```
BROKEN STATE -> gate ok=False rc=1   tail: ['Ran 1 test in 0.001s', '', 'FAILED (failures=1)']
FIXED STATE  -> gate ok=True  rc=0   tail: ['-'*70, 'Ran 1 test in 0.000s', '', 'OK']
```

The gate returns opposite verdicts on the two states, so "the gate passed" is a claim with
content. Artifacts: `~/.hermes/profiles/cronrunner/cache/scratch/0246/gate-demo/` (plus the
scratch kanban board `.../0246/kh/` used for section 2).

### 5. Smallest change, and the proposed standard

**A. Protocol-level — no engine change, no update, no live-job change until Stephen says so.**

Front-matter marking, read by the worker at claim:

```yaml
run_as_goal: true
goal:
  max_turns: 12
  contract: |
    Outcome: ...
    Verification: ...
    Constraints: ...
    Boundaries: ...
  gates:
    - "cd living-frame/firmware/x && pio run"
  github_contract: pagosacabin/nordtronics    # optional; adds the CI completion gate
```

Worker behaviour on claim of a `run_as_goal` task:

1. `hermes kanban create "<title>" --body-file <composed body> --assignee cronrunner --goal
   --goal-max-turns <max_turns> [--completion-contract <github_contract>]`
2. wait for the run (`hermes kanban runs <id>`; the gateway dispatcher ticks every 60 s)
3. run every `goal.gates` command **itself**; non-zero → re-dispatch a continuation card
   quoting the gate's output tail, up to `max_turns`, then park the task in `active/` with
   that output
4. stage the mailbox reply only when the card completed **and** every gate exits 0

Step 3 is the gate: the worker owns the staging boundary, so a failing gate cannot reach a
`staged` reply. Cost: one paragraph in the worker prompt plus the field in the skill. No
update, no engine patch.

**B. Engine-level — the durable version.** Give the kanban goal loop its own gate:
add a `gates` column beside `goal_max_turns` (`hermes_cli/kanban_db.py:640,1270`), expose
`--gate` on `kanban create` (`hermes_cli/kanban_parser.py:208-215`), and call `run_gate`
before `judge_goal` inside `run_kanban_goal_loop`, reusing the gate-failed continuation
prompt that already exists (`CONTINUATION_PROMPT_GATE_FAILED_TEMPLATE`,
`hermes_cli/goals.py:104-125`, with `_GATE_OUTPUT_TAIL_CHARS` = 3000). That gives host-only
cards the same deterministic gate the interactive route has, and removes the need for the
worker to supervise.

**Draft README wording (protocol change — Stephen's call; README NOT edited):**

> `run_as_goal` (optional, boolean, default false): the worker executes this task as a goal.
> It creates a kanban card with `--goal` and `--goal-max-turns` taken from `goal.max_turns`,
> dispatches it, and stages the mailbox reply only when the card completes **and** every
> command in `goal.gates` exits 0. A failing gate's output is fed back to a continuation
> card, up to `max_turns`; exhausting the budget parks the task in `active/` with the last
> gate output. `goal.contract` is passed verbatim as the card body. With
> `goal.github_contract`, the card additionally carries `--completion-contract` so exact-head
> CI must be green before it can complete.

The README's front-matter section is a closed allowlist ("Only these fields"), so this is a
protocol version bump rather than an informal addition — hence the draft, not an edit.

### 6. What was not proved, stated plainly

- **No live goal run through the pickup.** Criterion 4 wants an inbox task claimed by the
  untouched cycle. That needs (a) the front-matter field above, which the task forbids me to
  add to the protocol, and (b) a following tick — neither exists inside one fire. I did not
  improvise a marking the protocol does not carry, and I did not spin a live kanban goal card
  on the shared board for a demo: it would spend a paid worker run and leave board state that
  is not this task's deliverable.
- The **gate half is proved** (section 4); the **pickup half is the proposal**.
- **No hook fired at runtime** — none is configured; the hook pathway is evidenced by source
  and the shipped test, not by an event on this host. (Deliberately not configured as a
  probe: adding one would change the live worker's standing behaviour, which the task
  forbids.)
- **No branch, no CI run.** No repository file changed, so `proof: []`. Northtronics main
  carries six workflows — `android-build.yml`, `android-companion-v0.yml`, `backend.yml`,
  `detection-sim.yml`, `platformio.yml`, `website-check.yml` — all triggered by push/paths;
  a host-only task can start none of them.
