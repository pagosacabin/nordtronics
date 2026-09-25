---
id: "0054"
title: Two-tier model routing — Nvidia polls, DeepSeek works
status: in_progress
iteration: 2
expect-reply-within: 6h
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
