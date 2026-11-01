---
task_id: "0168"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
proof: []
---

# 0168 — Worker cron: MST off-peak schedule (DST ended)

## Context

Daylight saving time ended Sun 2026-11-01: America/Denver is now MST (UTC-7).
DeepSeek peak = 01:00-04:00 and 06:00-10:00 UTC, Mon-Fri (verified 2026-09-25
against five current sources). In MST that is: Sun-Thu 6:00-9:00 PM, Sun-Thu
11:00 PM-midnight, Mon-Fri 12:00-3:00 AM. Everything else is half price.

The four worker lines task 0060 set are MDT lines:

```
15 4-18,22,23 * * 1-4
15 0-18,22,23 * * 0
15 4-23 * * 5
15 * * * 6
```

As of this morning they fire during MST peak: 18:15 and 23:15, Mon-Thu and
Sunday (10 peak fires/week). Per your own 0060 reply, the worker schedule
lives in the Hermes cron store (profile `cronrunner`, store
`~/.hermes/profiles/cronrunner/cron/jobs.json`, one cron expression per job —
the four worker jobs were c0be50a686c6, 5c1532977f15, 7aff6948c2c1,
8b1c9e1323c5), NOT in the user crontab. The user crontab holds only the
`*/15 * * * * ... cron tick` ticker line.

## Task

Replace the worker schedule — the hourly DeepSeek worker tick at :15 past
the hour, NOT the 15-minute poller — with these four lines, verbatim:

```
15 4-17,21,22 * * 1-4
15 0-17,21,22 * * 0
15 3-23 * * 5
15 * * * 6
```

That is, in MST: Mon-Thu 4 AM-5 PM + 9-10 PM; Sun midnight-5 PM + 9-10 PM;
Fri 3 AM-11 PM; Sat all day.

## Success criteria

1. The worker schedule shows the four new lines verbatim (in the cron store
   jobs, per your 0060 reply — or in crontab if that is where yours lives)
   and no remaining worker line fires during an MST peak window
   (Sun-Thu hours 18-20, Sun-Thu hour 23, Mon-Fri hours 0-2).
2. The 15-minute poller is untouched: job `0b32bc199a4e` "Mailbox poll
   (Nvidia)" still reads `*/15 * * * *` and is still paused, and the
   user-crontab `cron tick` line is byte-identical before/after.

## Constraints

- Small task: deepseek-flash.
- Worker jobs only — do not touch the poller job or the crontab ticker line.
- Quote the before and after schedule output in full in the reply.

## Proof

The staged reply quoting the before/after schedule output (host state; no
repo code changes, so no CI run is expected).

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: before block, after block, and one line confirming the poller is
untouched.
