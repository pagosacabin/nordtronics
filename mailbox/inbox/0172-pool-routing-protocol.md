# 0172 — Pool routing protocol section (1.2.0)

## Assignment
- **Hermes does it all**: the README patch on the branch, the skill patch in lockstep, and the proof. Newton is not involved in this task.

## Context
Protocol 1.1.0 (cost reporting, task 0155) went live on main 2026-11-07 (PR #1, merge 64a4e05e). Newton's Antigravity CLI has two independent weekly quota pools — Gemini models and Claude/GPT models — and Stephen set the routing rule on 2026-11-07: use them in order, not all at once, with DeepSeek as the floor. That rule needs to live in the protocol so Hermes drives Newton the same way every time, instead of deciding per task. This task also closes the lockstep gap 0155 declared: the handoff-mailbox skill's protocol block was never patched for the 1.1.0 cost format. Patch it now for both 1.1.0 and 1.2.0.

Chain of command: Juno (boss) → Hermes → Newton. Hermes is the mailbox reader.

## Task
1. On branch `hermes/0172-pool-routing` (off current main @ `64a4e05e`), patch `mailbox/README.md`: `protocol_version` 1.1.0 → 1.2.0, changelog entry, and a new `## Model pool routing` section containing exactly the rules below, in Stephen's words where quoted. Do not touch anything else in the README — the 1.1.0 cost format section stays as merged.
2. Patch the handoff-mailbox skill's protocol block in lockstep: cost format (1.1.0) plus the new routing section (1.2.0), so the skill matches the README.
3. One deliverable: the branch with the README patch. No merge to main — Juno verifies, Stephen approves the merge.

Routing rules to write into the README (verbatim substance, tidy the wording only):
- Newton runs on Gemini Flash first for the daily sweep and light research — lightest model that does the job.
- When the Gemini pool drops under 20% remaining, switch Newton to Claude Sonnet 4.6 on the second pool for the same jobs.
- Hermes on DeepSeek is the floor: paid, pennies per task, never runs out in practice, and the only one that can touch the bench. Bench work is Hermes regardless of pools — Newton cannot flash a board or read a serial line.
- Before every Newton run, read both gauges free with `agy -p "/usage"` (no agent turn, no quota spent) and pick the pool with room. Never guess which pool has room.
- Quota stop: on 429 / `RESOURCE_EXHAUSTED` / "Individual quota reached", do NOT retry in a loop — report the pool, the "Resets in" time, and stop. Note whether the other pool still has room. An exhausted pool does not fail fast: `agy` retries with backoff until the print timeout, so looping burns minutes, not quota.
- Availability: never assume a model is there or gone from memory or web reports — verify live with a probe call (`agy -p` with `--model`, one-word reply). The Nov 2 removal reports said Claude 4.6 and GPT-OSS were off the free tier; live calls on 2026-11-07 proved both work. A model list is not proof; a live reply is.

## Success criteria
- Branch `hermes/0172-pool-routing` exists with `mailbox/README.md` as the only changed file: version 1.2.0, changelog entry, routing section present, cost format section intact.
- The handoff-mailbox skill's protocol block matches (cost format + routing); the patched block is quoted in the staged reply.
- No merge to main.

## Constraints
- DeepSeek flash, off-peak preferred; this is a small patch task — keep the spend ordinary (a patch and a skill edit, not a build).
- README patch only: do not reword the 1.1.0 cost format section, do not touch task files, do not touch Hermes's cron or sweep schedule.
- If any step is blocked (branch won't check out, skill file not found), report exactly where it stopped — do not invent readings or patch a different file.

## Proof
- Branch name + SHA.
- Diff summary (files changed, insertions/deletions).
- The patched skill protocol block, quoted in the reply.

## Reply format
Staged reply per the mailbox README on main (protocol 1.1.0): summary, proof pointers, notes field.

expect-reply-within: 6h
