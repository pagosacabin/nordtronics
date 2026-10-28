---
task_id: "0155"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0155 — Cost reporting in staged replies (protocol 1.1.0)

## Context

Stephen wants a per-task cost number for Hermes work — he was curious
what demos like 0153 cost, and what his side bench lab work costs.
Right now staged replies carry `proof` and `notes`, but no cost field,
so a per-task cost has to be reconstructed from outside the reply.

The mailbox protocol (mailbox/README.md, protocol_version 1.0.0) says:
MINOR = new rule or field. A version bump ships as a mailbox task
written in the previous format, carrying the exact patch text to apply
verbatim. This task is that bump: 1.0.0 -> 1.1.0, adding a `cost` field.

Source of truth for the numbers is the DeepSeek API usage objects,
summed across every API call the task made — NOT cumulative dashboard
totals. Reference rates checked 2026-10-28 (state the rates actually
used on the day, do not copy these blindly — rates change):
- Flash off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output
- Flash peak: $0.30 / $0.006 / $1.20 (double; peak = 01:00-04:00 and
  06:00-10:00 UTC weekdays)
- Pro off-peak: $0.66 / $0.022 / $1.98; peak double
- Search/fetch: $0.75 per 1,000 successful requests (separate from tokens)

## Task

Apply the patch text in the "Patch (apply verbatim)" section below to
mailbox/README.md on a branch named `hermes/0155-cost-reporting`,
push that branch to origin, and stage this task with proof. One
deliverable: the protocol 1.1.0 README change, nothing else. Do not
touch any other file, do not merge to main, do not edit task files.

In your staged notes, also state plainly whether your harness can
actually see per-call DeepSeek API usage objects (field names you
read, e.g. prompt_tokens / completion_tokens / cache split), or
whether it cannot — if it cannot, say so and say what it can see
instead. Do not fabricate usage numbers for this task itself; this
task's own cost field may read `unknown: harness cannot report
per-call usage` if that is the honest answer.

## Success criteria

1. Branch `hermes/0155-cost-reporting` exists on origin.
2. Its diff against main touches ONLY mailbox/README.md, and matches
   the patch text below exactly (version line, changelog entry, `cost`
   in the front-matter field list, and the new "Cost format" section).
3. No other file changed, no merge to main.
4. Staged notes state the harness capability answer from the Task
   section above.

## Constraints

- Default model tier for this task (flash). Do not use pro.
- Apply the patch verbatim. If a hunk does not apply cleanly, stop
  and stage with `status: staged` and a notes entry describing exactly
  which hunk failed and why — do not improvise a different wording.
- No secrets in the branch, the staged file, or commit messages.

## Proof

- branch: hermes/0155-cost-reporting
- sha: <tip SHA of that branch>
- A diff summary in notes confirming only mailbox/README.md changed.

## Reply format

Front-matter on stage, as usual, plus:

```yaml
cost:
  model: <model used for this task>
  api_calls: <count, or unknown>
  input_tokens: <sum, or unknown>
  output_tokens: <sum, or unknown>
  total_tokens: <sum, or unknown>
  cost_usd: <calculated, or unknown>
```

Notes must include: (a) the harness capability answer, (b) the diff
summary, (c) the rates used if a cost was calculated.

## Patch (apply verbatim)

Replace the line:

```
protocol_version: 1.0.0
```

with:

```
protocol_version: 1.1.0
```

In the Changelog section, after the 1.0.0 entry, add:

```
- 1.1.0 — 2026-10-28: cost reporting. Staged replies carry a `cost`
  field (see "Cost format" below), reporting DeepSeek API usage summed
  across the task's API calls and the calculated cost in USD.
```

In the Front-matter section, after the `notes` bullet, add:

```
- `cost`: DeepSeek API usage and calculated cost for the task. See
  "Cost format" below. Required on `staged` from 1.1.0 onward, or
  `unknown` with a plain reason if the harness cannot report per-call
  usage.
```

After the "Proof format" section, add a new section:

```
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
```
