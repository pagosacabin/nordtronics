---
task_id: "0150"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0150 — Fix stale model.default names across Hermes profiles

## Context

- 0149 notes flagged pre-existing drift, not fixed there (that task forbade
  touching the default profile): the **default** profile's `model.default` is
  `deepseek-v4-flash`, which is absent from the live `/v1/models` list — the
  list returns only `deepseek-flash` and `deepseek-v4-pro` (your own check,
  0149 profile_state).
- Juno corroborated from DeepSeek's published material on 2026-10-23: the
  current API names are `deepseek-flash` (DeepSeek-V4.1-Flash-1017, the model
  Hermes has run since 2026-10-17) and `deepseek-v4-pro` (V4-Pro-0813).
  Third-party trackers note the legacy `deepseek-v4-flash` ID may still route
  to V4.1 Flash, but the `deepseek-chat`/`deepseek-reasoner` aliases were
  retired outright on 2026-07-24 — legacy names die without ceremony, so the
  canonical name is the fix, not reliance on routing.
- Stephen's call (2026-10-23, main chat): the default should be
  `deepseek-flash`. His standing cost rule: flash is the everyday tier;
  `deepseek-v4-pro` (≈4.4× flash on input, ≈3.3× on output) only when a task
  spec names it with an explicit cost bound.
- The `jobsearch` profile was verified in 0149 as pointing at
  `deepseek-flash` already. The `cronrunner` profile's `model.default` is
  unknown to Juno — check it, don't assume.

## Task

One deliverable: every Hermes profile's `model.default` names a model that
exists on the live API, with the stale ones corrected to `deepseek-flash`.

1. Fetch the live model list once (`/v1/models` with the existing key — never
   print the key) and record the names returned.
2. Read `model.default` for all three profiles: default, cronrunner,
   jobsearch.
3. For each profile whose `model.default` is NOT in the live list: back up
   that profile's config (baseline copy, 0147 precedent), then set
   `model.default` to `deepseek-flash` via `hermes config set`, changing
   nothing else.
4. For each changed profile: read the value back, and make ONE minimal live
   model call through that profile (shortest prompt that proves routing —
   e.g. a one-line reply) and record that it returned.
5. Confirm the gateway is healthy afterwards.

## Success criteria

1. Reply quotes the live `/v1/models` names from this run.
2. Reply lists all three profiles with their `model.default` BEFORE the fix.
3. Every profile whose name was stale now reads back `deepseek-flash`; every
   profile already on a live name is reported unchanged (a profile on
   `deepseek-v4-pro` is valid — leave it and say so).
4. One successful minimal live call per changed profile, output quoted.
5. Config backups exist for each changed profile (paths in the reply), and
   the config diff for each is the `model.default` line only.
6. Gateway healthy at the end (status output quoted).

## Constraints

- Change `model.default` only, and only where the current value is absent
  from the live model list. Nothing else in any config.
- `deepseek-flash` is the replacement for every stale name. Do not upgrade
  any profile to `deepseek-v4-pro` — cost rule above.
- Keep live proof calls minimal: one per changed profile, trivial prompt.
  Total task spend should be effectively zero; stay on the flash tier.
- Never print the API key value anywhere.
- If `/v1/models` returns names different from Context (e.g. the fleet
  changed again), trust the live list, fix against it, and flag the
  discrepancy in `notes`.

## Proof

Host-only config task: `branch: none`, `run: none` — honest non-proof per the
0147 precedent. Evidence is the staged reply's exact outputs: live model list,
before/after values per profile, backup paths, the proof-call outputs, and
gateway status.

## Reply format

Fill in exactly these fields when staging:

- `live_models`: the names returned by `/v1/models` this run
- `before`: model.default per profile (default / cronrunner / jobsearch)
- `changed`: which profiles were changed, old value → new value; "none" if
  none needed it
- `backups`: baseline copy path per changed profile
- `proof_calls`: the minimal live call per changed profile + its returned
  output (or "n/a — no profile changed")
- `gateway_status`: verbatim status line(s) after the change
- `attestation`: only model.default lines changed; key never printed
- `notes`: discrepancies with Context, anything unexpected
