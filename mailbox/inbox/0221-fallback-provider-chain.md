---
task_id: "0221"
status: superseded
iteration: 0
proof: []
notes: "HELD by Juno 2026-11-19 at Stephen's direction: he does not currently have an OpenRouter API key and is re-evaluating whether he keeps using OpenRouter at all, so the conditional add in step 4 cannot run until he decides. Do not pick up. Juno will un-hold or re-file on his decision; if he drops OpenRouter, the backup route is re-chosen in a successor spec."
---

# 0221 — HELD (pending Stephen's OpenRouter decision) — Fallback provider chain: report, pin check, and conditional add

## Assignment
- Hermes: every step below — your own config on your own machine.
- Stephen: the only input is the optional OpenRouter API key in step 3, staged by him at the path named there. The task never asks him directly; it reads the file if present.
- Juno: verifies the staged reply, including the `hermes fallback check` result.

expect-reply-within: 24h

## Context
Juno's 2026-11-18 docs learning (official Fallback Providers docs): Hermes has a three-layer provider-resilience system. Layer 2 is a primary-model fallback chain — an ordered `fallback_providers:` list that swaps to a different provider:model mid-session on 429/500/502/503 (after retries) or immediately on 401/403/404, preserving conversation and context. `hermes fallback check` resolves each entry exactly as a switch would, without starting a session; `--live` sends one minimal completion per ready entry; the command exits 1 if any entry is not ready. Two facts that shape this task: fallback resets the prompt cache (one full-price re-read per switch), and unpinned cron jobs inherit the configured chain while a job with its own provider/model pinned never falls back to it. Today you run on DeepSeek alone — a DeepSeek outage, rate limit, or auth failure kills the mailbox worker and the Upwork sweep outright.

## Task
1. Version guard: confirm `hermes --version` reports v0.23.2. If it does not, stop and report — 0219 lands first.
2. Run `hermes fallback list` and report the current chain state exactly as shown.
3. Check whether the mailbox-worker and Upwork-sweep cron jobs are pinned to a provider/model, or unpinned. Report per job. Do not change any pinning in this task.
4. Conditional add — only if a file exists at `/home/astroboy/.hermes/openrouter-api-key.txt` (Stephen stages it there; read it on this machine only): add one OpenRouter free-tier model as the fallback entry via `hermes fallback add`, choosing a current free model suited to the worker's text tasks, then verify with `hermes fallback check --live`. Name the model you chose in the reply. If the file is not there, stop after steps 2–3 and report that the chain awaits the key.

## Success criteria
- `hermes fallback list` output reported verbatim.
- Per-job pinned/unpinned verdict for the mailbox worker and the Upwork sweep.
- If the key was staged: fallback entry added, and `hermes fallback check --live` exits 0 with the entry `ready`. If it does not verify, report the check output as-is — do not swap models repeatedly hunting for a pass; one chosen model, one check, report the result.

## Constraints
- The API key never appears in the reply, in any commit, or in any file you write — it is read from the staged path on this machine only.
- Model for this task itself: your normal DeepSeek flash tier.
- No other config changes: no pinning changes, no cron edits, no provider changes beyond the single conditional fallback add.

## Proof
Host-only task: `proof: []` with the CLI outputs in the reply body.

## Reply format
Status line first. Then: fallback list output, per-job pin verdicts, and — if the key was staged — the model chosen and the `fallback check --live` result. Deviations, if any.
