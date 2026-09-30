# 0082 — Apply config.yaml cost optimizations

expect-reply-within: 6h

## Context

On 2026-09-30 Juno completed framework-level research into Hermes config.yaml
cost optimizations (Stephen's request from 2026-09-25). All keys below were
verified against the official Hermes configuration reference and cron docs —
no invented keys. Baseline from the 2026-09-25 state.db analysis: ~$1.05
DeepSeek spend on a heavy build day; the cronrunner worker fleet is $0.64/day
(~60%, ~$0.03–0.05 per run); output tokens are 45% of the bill from under 1%
of tokens. Already decided earlier: background_review stays disabled, Saturday
worker schedule trimmed from 24 to 17 fires.

## Task

Apply the six verified cost optimizations below to the Hermes installation on
your laptop (config.yaml + cron jobs), install and wire up the wakeAgent
idle-tick gate script, and validate everything. One change at a time, verifying
as you go.

Optimization 1 — idle-tick gate (biggest win): attach `script="mailbox-changed.sh"`
to the cronrunner worker job(s). Scripts live in `~/.hermes/scripts/`. The
script runs first at $0 cost; when its LAST stdout line is exactly
`{"wakeAgent": false}` the agent never wakes for that tick. The script must
detect "mailbox unchanged since last run" (e.g. compare mailbox mtimes against
a last-run timestamp file) and print `{"wakeAgent": false}` in that case,
otherwise print `{"wakeAgent": true}` (optionally with a `context` object).
This takes $0.03–0.05 per empty tick to $0.

Optimization 2 — pin reasoning effort down: run
`hermes cron edit <worker-job> --reasoning-effort minimal` for each worker
cron job, or set `agent.reasoning_effort: minimal` globally in config.yaml.
There is NO `max_tokens` key in config.yaml (verified absent) — do not hunt
for one.

Optimization 3 — trim per-job toolsets: give the worker cron jobs
`enabled_toolsets=["file", "terminal"]`, and leave `workdir` unset so jobs run
repo-detached with no AGENTS.md loaded. Carrying browser/delegation schemas
into tiny jobs bloats the prompt on every run.

Optimization 4 — fleet model default: `hermes config set cron.model
<deepseek-flash-slug>` using the Flash slug already configured on this machine
(or per-job `hermes cron edit <job> --provider deepseek --model <slug>`).

Optimization 5 — background_review: confirm it remains disabled; do not
re-enable it. (Middle paths exist if Stephen ever asks: max_input_tokens or a
cheap-model route — out of scope for this task.)

Optimization 6 — cache hygiene: no knob exists (always-on). Keep prompts
stable and avoid model switches; note that editing compression.* or
model.context_length rebuilds the cached agent and resets the prefix cache —
avoid touching those keys in this task.

Also commit the gate script to the repo at `hermes/scripts/mailbox-changed.sh`
on the branch below (same content as installed to `~/.hermes/scripts/`), so
Juno can read and verify it.

## Success criteria

1. `hermes config check` passes with zero errors after all changes (run it
   after EACH change, not just at the end).
2. The gate script is installed at `~/.hermes/scripts/mailbox-changed.sh`,
   wired into the worker cron job(s) via `script=`, and a test run against an
   unchanged mailbox prints `{"wakeAgent": false}` as its last stdout line.
3. Worker cron job(s) show reasoning effort `minimal` (per-job pin or the
   global `agent.reasoning_effort` key) in their effective config.
4. Worker cron job(s) carry `enabled_toolsets=["file", "terminal"]` (list any
   job where you deliberately did not apply it, with the reason).
5. `cron.model` resolves to the DeepSeek Flash slug (state the slug).
6. background_review confirmed still disabled.
7. Branch `hermes/0082-config-cost-optimizations` pushed to origin containing
   `hermes/scripts/mailbox-changed.sh`.

## Constraints

- Apply one optimization at a time; re-run `hermes config check` after each.
- config.yaml never leaves your laptop unredacted: the staged reply carries a
  REDACTED old→new diff per key only — no API keys, tokens, or secrets anywhere
  in the reply or the repo branch.
- Push only to branch `hermes/0082-config-cost-optimizations`.
- Use only the config keys listed in this task; do not invent keys. If a key
  does not exist on your installation, report it as blocked, do not substitute.
- Run on the already-configured worker tier (Flash); do not upgrade any model.
- Prefer off-peak windows (7–10 PM, 12–4 AM MDT weekdays) for any non-trivial
  model calls this task requires.
- A misconfigured job fails to `blocked_config` with one alert and no LLM call
  — if `hermes config check` flags anything, fix it before proceeding.

## Proof

- Branch `hermes/0082-config-cost-optimizations` on origin, with
  `hermes/scripts/mailbox-changed.sh` readable by Juno (she will read the
  script and check the gate logic).
- Staged reply includes: redacted old→new diff for every changed key, the full
  `hermes config check` output from the final state, the gate-script test
  output showing `{"wakeAgent": false}` on an unchanged mailbox, and the
  effective config (reasoning effort, toolsets, model) of each worker cron job
  touched. Raw terminal output, not summaries.

## Reply format

- status: done | blocked (with the blocking key or step named)
- summary: one paragraph on what changed
- branch: `hermes/0082-config-cost-optimizations` + SHA
- config_changes: old→new per key (redacted)
- validation: `hermes config check` final output; gate-script test output
- jobs_touched: each cron job edited and its effective reasoning effort,
  toolsets, and model
- notes: anything not applied and why; estimated savings where measurable
