---
task_id: "0060"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0060 — Move worker cron to true DeepSeek off-peak hours (MDT)

## Context

The DeepSeek peak/off-peak windows on file were inverted. Verified 2026-09-25
against five current sources: PEAK = 01:00-04:00 and 06:00-10:00 UTC,
Monday-Friday (excluding Chinese public holidays). Off-peak (50% off) is
everything else. In MDT (UTC-6, through Nov 1): peak = Sun-Thu 7:00-10:00 PM
and Mon-Fri 12:00-4:00 AM. Your current worker line has been firing during
Sunday-evening peak and missing cheap weekend hours.

## Task

Update YOUR worker cron — the hourly DeepSeek worker tick at :15 past the
hour, not the 15-minute poller. Replace the current worker cron line(s) with
these four lines (same :15 tick, off-peak hours only):

```
15 4-18,22,23 * * 1-4
15 0-18,22,23 * * 0
15 4-23 * * 5
15 * * * 6
```

That is: Mon-Thu 4 AM-6 PM + 10-11 PM; Sun midnight-6 PM + 10-11 PM;
Fri 4 AM-11 PM; Sat all day.

## Success criteria

1. `crontab -l` shows the four new lines and no remaining worker line that
   fires during a peak window.
2. The 15-minute poller entry is untouched.

## Constraints

- Small task: deepseek-flash.
- If the worker schedule lives somewhere other than your user crontab (a
   cron-runner bot, /etc/cron.d, etc.), update it there and say where.
- Quote the before and after schedule output in full.

## Proof

The staged reply quoting the before/after cron output.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: before block, after block, and one line confirming the poller is
untouched.
