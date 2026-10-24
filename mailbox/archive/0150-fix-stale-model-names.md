---
task_id: "0150"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  branch: none
  sha: none
  run: none
  files: []
  reason: "host-only Hermes config task: no repository file changed (0147 precedent)"
live_models:
  deepseek: ["deepseek-flash", "deepseek-v4-pro"]
  nvidia_nemotron_check: ["nvidia/nemotron-3.5-lightning-30b-a3b"]
before:
  default: deepseek-v4-flash
  cronrunner: nvidia/nemotron-3.5-lightning-30b-a3b
  jobsearch: deepseek-flash
changed:
  - profile: default
    old: deepseek-v4-flash
    new: deepseek-flash
backups:
  - /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/config.yaml.pre-0150
proof_calls:
  - profile: default
    session: 20261023_201617_70f228
    prompt: "Reply with exactly one word: OK"
    output: "OK"
    recorded_model: deepseek-flash
gateway_status: |
  Active: active (running) since Thu 2026-10-15 08:18:33 MDT; 1 week 1 day ago
  Main PID: 1001131 (hermes)
  ✓ User gateway service is running
  ✓ Systemd linger is enabled (service survives logout)
attestation: "only model.default changed; comment count 46 -> 46; diff is one line; API key never printed"
notes: |
  JUNO VERIFIED 2026-10-23 20:47 MDT: all criteria met — live model list quoted (deepseek-flash, deepseek-v4-pro); default deepseek-v4-flash → deepseek-flash with one-line diff, backup path recorded, minimal live proof call returned OK with recorded_model deepseek-flash; cronrunner correctly NOT rewritten (provider: nvidia, name live on NVIDIA list — changing only model.default would have broken routing); jobsearch already live, unchanged; gateway healthy; key never printed. Intent-over-wording catch on cronrunner credited.

  See "Staged reply" below. Short form: one stale name (default profile
  deepseek-v4-flash) corrected to deepseek-flash; jobsearch already live and
  unchanged; cronrunner is a different provider (NVIDIA) and its model is
  present in NVIDIA's live list, so it was NOT rewritten to deepseek-flash
  (doing so while leaving provider: nvidia would have broken it). A fourth
  profile, `tim`, exists outside the task's three and is also on a live NVIDIA
  name; untouched.
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

## Staged reply (0150)

### live_models

Fetched once this run from `https://api.deepseek.com/v1/models` (key read from
`~/.hermes/.env`, never printed). The list returned **2** names:

- `deepseek-flash`
- `deepseek-v4-pro`

`deepseek-v4-flash` is **absent** — confirming Juno's 2026-10-23 corroboration and
the drift 0149 flagged. The live fleet has not changed since 0149; no discrepancy
with the task's Context.

### before

| profile | `model.default` BEFORE | provider |
|---|---|---|
| default | `deepseek-v4-flash` | deepseek |
| cronrunner | `nvidia/nemotron-3.5-lightning-30b-a3b` | nvidia |
| jobsearch | `deepseek-flash` | deepseek |

### changed

- **default**: `deepseek-v4-flash` -> `deepseek-flash`

Nothing else changed in any config. `jobsearch` was already on a live name
(`deepseek-flash`) — reported unchanged.

`cronrunner` is **not** stale and was deliberately left alone: its
`model.default` names an **NVIDIA** model while its `provider` is `nvidia`, so the
DeepSeek `/v1/models` list is the wrong oracle for it. Checked against the right
one — `https://integrate.api.nvidia.com/v1/models` (85 names returned) — and
`nvidia/nemotron-3.5-lightning-30b-a3b` is present. Rewriting it to
`deepseek-flash` while leaving `provider: nvidia` and
`base_url: https://integrate.api.nvidia.com/v1` in place (the task's "change
`model.default` only" constraint) would have pointed an NVIDIA endpoint at a
DeepSeek-only model ID and broken the profile. Recorded here rather than done;
flagging as the one place I read the task's wording against its intent.

### backups

- `/home/astroboy/.hermes/profiles/cronrunner/cache/scratch/config.yaml.pre-0150`
  (sha256 `627785de184891f076e9b140ab6214fefc456fc198568ce6ac5eb5119590f417`,
  identical to the pre-write `~/.hermes/config.yaml`)
- also `/home/astroboy/.hermes/profiles/cronrunner/cache/scratch/config.default.yaml.20261024-baseline`

Config diff for the changed profile — the `model.default` line only:

```
--- config.yaml.pre-0150
+++ ~/.hermes/config.yaml
@@ -1,5 +1,5 @@
 model:
-  default: deepseek-v4-flash
+  default: deepseek-flash
   provider: deepseek
   base_url: https://api.deepseek.com/v1
   api_mode: chat_completions
```

Comment-preservation evidence (per the mailbox-conventions rule): comment lines
`46 -> 46`, and the file was written by plain `hermes config set` under
`HERMES_HOME=/home/astroboy/.hermes` (the worker's own `HERMES_HOME` is the
cronrunner profile, so the profile was pinned explicitly).

Read back:

```
$ HERMES_HOME=/home/astroboy/.hermes hermes config get model.default
deepseek-flash
```

### proof_calls

One minimal live call, through the **default** profile after the change:

```
$ HERMES_HOME=/home/astroboy/.hermes hermes -z "Reply with exactly one word: OK"
OK
```

Routing corroborated from the default profile's own session store, which records
the model that actually served it — not merely the configured value:

```
session 20261023_201617_70f228  profile=default  source=oneshot  model=deepseek-flash
session_model_usage: ('20261023_201617_70f228', 'deepseek-flash', 'deepseek', api_call_count=1)
```

(`~/.hermes/state.db`, read-only query against `sessions` and
`session_model_usage`.) No profile besides `default` was changed, so no other
proof call was made.

### gateway_status

```
$ HERMES_HOME=/home/astroboy/.hermes hermes gateway status
Active: active (running) since Thu 2026-10-15 08:18:33 MDT; 1 week 1 day ago
 Main PID: 1001131 (hermes)
✓ User gateway service is running
✓ Systemd linger is enabled (service survives logout)
```

Residual, stated rather than glossed: the gateway process predates the edit
(started 2026-10-15) and was **not** restarted — the task asked to confirm health,
not to bounce it. Its recurring log line is the pre-existing
`MCP OAuth setup failed for 'gitlab'` warning (unrelated to model config, present
before this run). The one-shot proof call above loads config per process, so it
proves the on-disk value routes; if the long-lived gateway caches `model.default`
at startup, a `hermes gateway restart` would be what applies it to gateway-served
sessions. Not done unasked.

### attestation

Only `model.default` was changed, in one profile. The API key value was never
printed: it was read into a Python variable from `~/.hermes/.env` and used only as
an `Authorization: Bearer` header.

### notes

1. **Provider-aware reading of "not in the live list"** — see `changed` above.
   `cronrunner` was validated against NVIDIA's `/v1/models`, not DeepSeek's, and
   left unchanged because its name is live there. If Juno intended all three
   profiles to be forced onto `deepseek-flash` regardless of provider, that is a
   provider/base_url migration, not a `model.default` edit, and this file should
   be re-filed with that decision.
2. **A fourth profile exists that the task does not name**: `~/.hermes/profiles/tim`
   (`model.default: nvidia/nemotron-3.5-lightning-30b-a3b`, also live on NVIDIA).
   Untouched, not in scope, reported for completeness.
3. **No discrepancy with Context.** Live DeepSeek fleet is exactly
   `deepseek-flash` + `deepseek-v4-pro`; no legacy alias appears in the list.
4. No branch, no CI run, no ntfy receipt: this is a host-only config task
   (`proof: branch none / run none / files []`), per the 0147 precedent. The
   evidence is the outputs quoted above.
