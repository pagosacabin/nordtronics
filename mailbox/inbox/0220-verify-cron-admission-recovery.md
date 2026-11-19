---
task_id: "0220"
status: inbox
iteration: 0
proof: []
notes: ""
---

# 0220 — Verify cron admission recovery on v0.23.2

## Assignment
- Hermes: every step below — read-only inspection of your own cron system.
- Juno: verifies the staged reply against the docs description of admission recovery.

expect-reply-within: 24h

## Context
Juno's 2026-11-17 docs learning (official Cron docs, "Admission recovery"): on the current release, an interrupted cron run is reconciled, not re-run or silently lost — agent-backed runs keep their fire identity until output and a durable delivery handoff are recorded; the scheduler reconciles unfinished receipts on every tick, including after a gateway/laptop restart; a run whose terminal result is already known is delivered without running the agent again; an interrupted execution stays paused for operator review; recovery never creates a second admission for the same fire. This matters because your unattended fleet — the mailbox worker and the 5 AM Upwork sweep — runs on a laptop that sleeps, reboots, and loses the gateway.

## Task
1. Version guard: confirm `hermes --version` reports v0.23.2. If it does not, stop and report — 0219 lands first.
2. Inspect what `hermes cron runs` (and any admission/ledger view the CLI exposes) shows for the mailbox-worker and Upwork-sweep jobs: are fire receipts / reconciliation states visible? Does the history show any run that was interrupted and reconciled rather than re-run?
3. Report per job: what the runs ledger shows, and whether admission recovery is observable from the CLI on this install.

## Success criteria
- A per-job statement of what the ledger shows, with the raw CLI output that supports it.
- A plain verdict: admission recovery observable / not observable from the CLI, per job or overall.

## Constraints
- Read-only. Do NOT interrupt a live run, kill the gateway, or reboot anything to manufacture a test case — evidence from the existing history and the CLI's own reporting is the deliverable.
- Model: your normal DeepSeek flash tier.
- If the CLI exposes no surface for this, say so plainly in the reply — that is a valid result, not a failure.

## Proof
Host-only task: `proof: []` with the CLI outputs in the reply body.

## Reply format
Status line first. Then per job: ledger findings and raw output. Then the overall verdict. Deviations, if any.
