---
id: "0054"
title: Two-tier model routing — Nvidia polls, DeepSeek works
status: verified
iteration: 2
expect-reply-within: 6h
proof:
  - config: /home/astroboy/.hermes/profiles/cronrunner/cron/jobs.json — poller 0b32bc199a4e (nvidia/nemotron-3.5-lightning-30b-a3b, */15) and worker c0be50a686c6 (deepseek-v4-pro via provider deepseek, 0 9 * * *)
  - poll_run: ~/.hermes/profiles/cronrunner/cron/output/0b32bc199a4e/2026-09-24_18-37-10.md -> "POLL ok inbox=0 active=1 staged=0 hold=4 lowest=0054"
  - poll_model_receipt: ~/.hermes/profiles/cronrunner/cron/usage_audit.jsonl -> {"ts":"2026-09-25T00:37:09.989Z","job_id":"0b32bc199a4e","model":"nvidia/nemotron-3.5-lightning-30b-a3b"}
  - worker_probe: provider deepseek, model deepseek-v4-pro -> output "PROBE-OK"; balance 19.85 -> 19.85 USD (delta < 0.01)
  - key_scope: DEEPSEEK_API_KEY provisioned into /home/astroboy/.hermes/profiles/cronrunner/.env (mode 600) — value never committed, never pasted
  - ntfy: topic nordtronics-build-ed05a663, id kREEHfiCsTKN, published 2026-09-24 18:38 MDT (epoch 1790296689)
  - note: local-configuration deliverable — no repo branch and no CI run applies (same shape as 0044)
notes: |
  Replied 2026-09-24 ~18:40 MDT. Both tiers configured and both verified live.
  Poller = Nvidia, worker = DeepSeek, poll prompt carries no protocol digest.
  Model ids taken from the account's own GET /v1/models, not guessed: deepseek-flash
  and deepseek-v4-pro. Honest caveats are in the Reply section: the poll cycle needed
  a brief resume (cron run refuses a paused job) and was re-paused immediately; and
  this task cannot produce README-format remote proof pointers because the deliverable
  is local configuration, not an artifact.
---
---

## Context

Hermes's model providers are partially down: OpenRouter and the built-in Nous
Portal models are at credit limits. Nvidia NIM is working. Decision: split
model usage into two tiers so the free/working pipe carries the dumb,
high-frequency job and paid budget is spent only on real work.

- Poller (mailbox check every 15 min): Nvidia model. Dumb job, ~96 runs/day.
- Worker (real tasks): DeepSeek. Smart job, rare.
- DeepSeek peak pricing is 01:00–04:00 and 06:00–10:00 UTC on weekdays
  (7:00–10:00 PM and 12:00–4:00 AM MDT during daylight saving). Schedule heavy
  tasks for weekday daytime or weekends for the 50% off-peak rate.

## Task

1. Report your current model configuration: the config file path(s), the env
   vars involved, and the output of `hermes model` (or equivalent).
2. Check for a DeepSeek API key in the laptop environment (`DEEPSEEK_API_KEY`).
   If it is missing: STOP. Do not invent one. Reply asking Stephen to create a
   key at https://platform.deepseek.com and export it in the laptop
   environment where the cron worker runs. Never ask him to paste it into chat
   or into any file you will commit.
3. Configure two profiles: the mailbox poller uses the Nvidia model, task work
   uses DeepSeek. Confirm the exact DeepSeek model id from the DeepSeek
   account — do not guess it.
4. Keep the poll prompt lean: the poller checks for new/changed mailbox files
   and reports. It must NOT carry the protocol digest — protocol knowledge
   lives with the worker.
5. Verify live: run one poll cycle and one small worker probe (e.g. re-read
   mailbox/README.md from origin/main and report the protocol version).

## Success criteria

- [ ] Poller runs on the Nvidia model; worker runs on DeepSeek.
- [ ] One poll cycle completed with the Nvidia model id visible in the log.
- [ ] One worker probe completed on DeepSeek with a small billed amount
       (under $0.10) visible on the DeepSeek usage page.
- [ ] Poll prompt contains no protocol digest text.
- [ ] Exact DeepSeek model id confirmed from the account, not guessed.

## Constraints

- One deliverable: the routing configuration. Do not touch anything under
  mailbox/hold/ — those tasks are parked and will be restored after this.
- Do not commit any API key or secret to the repo, ever.
- If the DeepSeek key is missing, the reply asking for it IS the deliverable
  for this iteration.

## Proof

- Config diff or `hermes model` output showing both profiles and their models.
- Poll log line showing the Nvidia model id.
- DeepSeek usage/billing line for the probe run (amount and model id).

## Reply format

Stage your reply as mailbox/staged/0054-two-tier-model-routing.md with the
proof pointers above. If the DeepSeek key is missing, stage the reply with
status: staged and the exact ask for Stephen instead.


## Reply

### 1. Current model configuration (as it stood when this task opened)

Config files:
- `/home/astroboy/.hermes/config.yaml` — default (interactive/desktop) profile
- `/home/astroboy/.hermes/profiles/cronrunner/config.yaml` — the profile the cron worker runs under

Secrets files (secrets only, mode 600):
- `/home/astroboy/.hermes/.env`
- `/home/astroboy/.hermes/profiles/cronrunner/.env`

Profile-level model settings (cronrunner/config.yaml):

```
model:
  default: nvidia/nemotron-3.5-lightning-30b-a3b
  provider: nvidia
  base_url: https://integrate.api.nvidia.com/v1
fallback_providers:
  - provider: openrouter
    model: deepseek/deepseek-v4.1-flash
  - provider: nous
    model: deepseek/deepseek-v4.1-flash
```

`hermes model` cannot be run for evidence: it is interactive-only and refuses a pipe
("`hermes model` requires an interactive terminal"). Equivalent evidence used instead:
the parsed config above, the job-level overrides below, and the credential pool.

Credential pool (`auth.json`, per profile) at task start — note the gap:
- default: `nous, openrouter, nvidia, deepseek`
- cronrunner: `openrouter, nous, nvidia` — **no deepseek slot**

This is also the answer to why the worker went down at 11:15 and again at 16:00/16:15:
the primary was rate-limited (HTTP 429 on nvidia) and BOTH fallback entries are
`deepseek-v4.1-flash` via openrouter and nous — the pair that fails with "requires
available credits". A primary hiccup therefore had nothing healthy to fail over to.

### 2. DeepSeek key check

Present, in the laptop environment:
- `/home/astroboy/.hermes/.env` line 554 — length 35, `sk-` prefix, suffix `…d53d`
- interactive shell env — a different value, suffix `…3c89`

Both authenticate: `GET https://api.deepseek.com/v1/models` returns HTTP 200 for both.

Missing, where it actually matters: the key was **not** in the cronrunner profile —
`cronrunner/.env` had no DEEPSEEK entry and its `auth.json` credential pool had no
deepseek slot. Since profiles are independent islands, a key imported via the CLI into
the default profile does not reach the cron worker.

Because the key existed, I did not take the "key is missing, here is the ask" path.
I provisioned it into the profile scope: appended to
`/home/astroboy/.hermes/profiles/cronrunner/.env`, mode 600, read from the existing
default-profile value — the value never entered chat and was never written to the repo.
Constraint honoured: no secret committed anywhere.

### 3. Two-tier routing configured

Model ids confirmed from the DeepSeek account itself (`GET /v1/models`), not guessed:

```
deepseek-flash
deepseek-v4-pro
```

These are the native DeepSeek ids. They are NOT the aggregator ids for the same family
(`deepseek-ai/deepseek-v4.1-flash` on nvidia, `deepseek/deepseek-v4.1-flash` on
openrouter/nous) — worth recording, because guessing the aggregator id here would have
failed.

Tier 1 — poller (dumb job, high frequency, free pipe):

```
job     0b32bc199a4e  "Mailbox poll (Nvidia)"
model   nvidia/nemotron-3.5-lightning-30b-a3b   provider nvidia
sched   */15 * * * *     skills: none     prompt: 1124 bytes
```

Tier 2 — worker (real tasks, paid, rare):

```
job     c0be50a686c6  "Mailbox worker (DeepSeek)"
model   deepseek-v4-pro   provider deepseek
sched   0 9 * * *     skills: github, kicad-mcp-server, handoff-mailbox
script  mailbox-protocol-check.py (pre-run, every tick)
```

Two judgement calls, stated so you can overrule them:
- The worker's schedule moved from `*/15` to `0 9 * * *`. Keeping it at `*/15` would
  have run the smart paid model 96x/day, which defeats the point of the split. 09:00 MDT
  is 15:00 UTC — outside DeepSeek's weekday peak window (01:00-04:00 and 06:00-10:00 UTC).
- Poller model is the cheap Nvidia tier-1 model rather than the 550B ultra. "Nvidia
  polls" did not name a model; if you want the ultra on the poller, it is a one-line edit.

Wiring: the poller queues the worker with
`hermes -p cronrunner cron run c0be50a686c6` when `inbox/` or `active/` holds work.
Verified that `hermes cron run` exists and refuses a paused job ("Job is paused/disabled;
resume it before running") — so the trigger only works while the worker is enabled.

Both jobs are **paused**, matching the cluster state Stephen set at ~16:2x. Nothing
fires until you resume.

### 4. Poll prompt carries no protocol digest

Checked by string search over the stored prompt (1124 bytes):

```
protocol_version  absent      PROTOCOL-MANAGED  absent
proof block       absent      iteration         absent
ntfy              absent      handoff-mailbox   absent
```

The only occurrences of the word "staged" are the mailbox directory name. The poller has
**no skills attached at all**, so it cannot pull the protocol in via the handoff-mailbox
skill either. Protocol knowledge lives only in the worker.

### 5. Live verification

Poll cycle (Nvidia):

```
run id   3adc7f4cb65143f3a587f362d1b13f8a   source=direct   completed 18:36:35 MDT
job last_status ok at 18:37:10 MDT
output   cron/output/0b32bc199a4e/2026-09-24_18-37-10.md
response: POLL ok inbox=0 active=1 staged=0 hold=4 lowest=0054
model receipt: cron/usage_audit.jsonl
  {"ts":"2026-09-25T00:37:09.989Z","job_id":"0b32bc199a4e",
   "model":"nvidia/nemotron-3.5-lightning-30b-a3b"}
```

The poll's counts match the repo exactly (0054 in active, 0044/0050/0053 + README in hold).

Worker probe (DeepSeek, run under the cronrunner profile):

```
hermes -p cronrunner -z "Reply with exactly the token PROBE-OK and nothing else." \
  -m deepseek-v4-pro --provider deepseek
-> PROBE-OK   (exit 0)
balance before: 19.85 USD     balance after: 19.85 USD
```

The balance delta is below the 2-decimal display precision, so it reports as 0.00 — i.e.
under $0.10 as required, but I want to be precise: I can evidence *that* the probe billed
under a cent, not an exact figure. The per-model line on the DeepSeek usage page is yours
to read; `GET /user/balance` is the only usage surface I can query.

### Caveats, stated rather than smoothed over

1. **The poll cycle needed a temporary resume.** `hermes cron run` refuses a paused job,
   so I resumed the poller, ran one tick, and re-paused it immediately. All four jobs
   across both profiles (default 2148c3e14463 + a6c49a025f6f, cronrunner c0be50a686c6 +
   0b32bc199a4e) are paused right now.
2. **No README-format proof pointers.** The deliverable is local configuration, so there
   is no branch and no CI run to point at. The proof block therefore carries local
   artifact paths plus the exact commands to reproduce — the same shape 0044 used, which
   is still unresolved in hold. If you want real remote pointers, this needs the
   deliverable committed into the repo (e.g. a redacted job-config snapshot and the poller
   prompt under a path CI watches). I did not invent a workflow or add a branch to fake it.
3. **No `model=` line in agent.log for these runs.** One-shot and direct cron runs record
   the model in `usage_audit.jsonl` rather than the conversation log; that JSONL row is
   the durable model-id receipt.
4. **A stale pointer in my own trail, flagged rather than buried.** My first pickup attempt
   committed `e89aa57` — which moved the wrong file (0053) because the worktree was 8
   commits stale, under a commit message claiming 0054. I caught it before pushing,
   discarded it (`reset --hard origin/main`), and redid the pickup cleanly. It never
   reached origin. The visible history is `c05ff2d` (rename) then `1453ed6` (front-matter),
   because the first commit captured only the rename.
5. **`mailbox/hold/` untouched**, per the constraint.
6. **Fallback chain is still broken** and I did not change it, because the task scoped me
   to the two tiers. The openrouter/nous fallbacks will keep failing on credits. Worth its
   own task; a healthy fallback would be another Nvidia model rather than a paid provider.
