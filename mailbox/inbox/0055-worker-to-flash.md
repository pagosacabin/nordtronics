---
id: "0055"
title: Worker model to DeepSeek Flash (cost)
status: inbox
iteration: 1
expect-reply-within: 6h
---

## Context

0054 (two-tier routing) is verified and archived. Cost decision made by Stephen:
the worker moves from `deepseek-v4-pro` to `deepseek-flash` (native id, already
confirmed from the account's own GET /v1/models during 0054). Everything else
stays exactly as 0054 left it.

## Task

1. In the cronrunner profile config, change the mailbox worker job's model from
   `deepseek-v4-pro` to `deepseek-flash` (provider `deepseek`). Nothing else
   changes — same job, same schedule, same key, same poller.
2. Run one worker probe under the cronrunner profile and confirm it completes.

## Success criteria

- [ ] Worker job shows model `deepseek-flash`, provider `deepseek`.
- [ ] One probe completes with output `PROBE-OK`.
- [ ] DeepSeek balance delta for the probe is under $0.10.

## Constraints

- One-line change. Do not touch the poller, the schedule, or any other job.
- Do not unpause any paused jobs except for the single probe run; re-pause
  immediately after, as before.

## Proof

- The changed config line (file path + model id).
- Probe output line and the DeepSeek balance before/after.

## Reply format

Stage your reply as mailbox/staged/0055-worker-to-flash.md with the proof
pointers above.
