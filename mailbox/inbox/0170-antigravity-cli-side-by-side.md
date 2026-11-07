# 0170 — Install Antigravity CLI on the bench + side-by-side daily sweep

## Context
Stephen wants real data on whether Google Antigravity's free tier holds up under his actual workload, versus Hermes on DeepSeek. Google publishes no token number for the free weekly quota, so the only honest test is empirical: run the same work both ways and measure.

Hermes's current workload: the daily 5:00 AM Upwork sweep (mailbox task 0121 definition, output `handoff/upwork-leads/YYYY-MM-DD.md`) plus occasional mailbox bench tasks. Reference point: task 0155 measured 1,240,094 tokens / $0.012726 on deepseek-flash off-peak for one full task.

Antigravity CLI only — NOT the desktop IDE. The laptop is x86_64 Linux; the CLI ships a linux x64 build (`agy`). Run it in a Herdr pane alongside Hermes. Hermes's own sweep keeps running as normal; this is an additional side-by-side run, not a replacement.

The question to answer: does the free weekly quota cover a daily sweep at this pace, and what does one sweep cost in quota versus Hermes's DeepSeek cost?

## Task
1. Install the Antigravity CLI on the bench laptop (linux x64 build, from the official source). Record `agy --version` output. Do NOT install the desktop IDE.
2. Authenticate using the CLI's own login flow. If it needs a browser sign-in Stephen must do himself, STOP at that step and report the exact step/URL for him — do not invent credentials, do not enter any password or code yourself, do not store tokens in task files or git.
3. Free tier only: do NOT subscribe to any Google AI plan, do NOT enter payment details. If the CLI offers a paid upgrade, decline it.
4. Record the Antigravity quota meter BEFORE the test run (CLI output or screenshot, verbatim).
5. Run the daily sweep as the test workload: the same Upwork sweep Hermes's cronrunner runs (task 0121 definition — same sources, same filters). Read-only: no bids, no applications, no messages, no account creation.
6. Record the quota meter AFTER the run, wall-clock time, and any quota/rate-limit messages verbatim.
7. Run Hermes's own sweep for the same day as usual and record its session token usage + cost from `session_model_usage` (the same source 0155 used).
8. Deliver the comparison: one table — Antigravity quota used, Hermes tokens + DeepSeek cost, wall time each, errors on either side — plus a one-line verdict: does the free tier cover a daily sweep at this pace?

## Success criteria
- `agy --version` runs; CLI authenticated on the free tier (or the exact auth step for Stephen is documented).
- One full daily sweep completed via the Antigravity CLI, with its results.
- Quota meter before AND after, quoted verbatim.
- Hermes's same-day sweep tokens + cost quoted from `session_model_usage`.
- Comparison table + verdict in the staged reply.

## Constraints
- CLI only — no desktop IDE, no paid plan, no payment details.
- DeepSeek flash, off-peak preferred; keep the spend ordinary (this is a measurement task, not a big build).
- Read-only toward Upwork / Freelancer / AuraOne — no bids, applications, or messages.
- Do not touch Hermes's own config, cron, or sweep schedule. Do not replace the 5 AM sweep.
- Run the Antigravity side in a Herdr pane; keep it separate from Hermes's panes.
- If any step is blocked (auth needs Stephen, CLI won't install, quota meter unreadable), report exactly where it stopped — do not invent readings.

## Proof
- `agy --version` output + install source.
- Quota meter before/after (verbatim CLI output or screenshot).
- Sweep output (file path or pasted summary) + wall time.
- Hermes `session_model_usage` rows for the same-day sweep.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

expect-reply-within: 6h
