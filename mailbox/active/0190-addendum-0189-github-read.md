---
task_id: "0190"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 24h
notes: |
  Addendum to 0189, picked up 2026-11-15 15:20 MST in the same run as 0189 (the
  addendum's own instruction: "fold this into 0189 before you run it. Same task").
  Front-matter added at pickup. Read-only.
---
# 0190 — Addendum to 0189: the probe must include a GitHub read

## Assignment
- **Hermes:** fold this into 0189 before you run it. Same task, higher proof bar.
- **Juno:** relaying Stephen's correction, 2026-11-15 14:38 MST in main chat: **"But wasn't the question about github access?"**

## The correction
Stephen is right, and it changes what "proven" means for 0189. His own test in the Herdr `copilot-main` pane (14:37 MST) got `READY` back from your exact failed probe — that proves Copilot **runs** under his interactive login. It does **not** prove Copilot can act **on GitHub**, which was the original question and the actual 0186 need (a review attached to his repos).

## Stephen already ran the probe — result (2026-11-15 14:40 MST, screenshot in main chat)
Stephen asked Copilot in the Herdr `copilot-main` pane to list the open PRs on pagosacabin/cabin-play. Copilot shelled out to `gh pr list --repo pagosacabin/cabin-play ...` and it **failed with exit code 4**. Copilot's own report, verbatim: *"I can't list the PRs from GitHub right now because this environment isn't authenticated for the GitHub CLI (`gh auth login` / `GH_TOKEN` is required)."*

**The true state, now established:** Copilot itself is authenticated and answers (READY probe, 14:37). Copilot reaches GitHub by shelling out to `gh` — and `gh` is the missing layer in both environments: **absent** in the Herdr pane, **present but metadata-scoped** in your cron profile (hence the 0186 PR-create 403). One root cause, two symptoms.

## Updated ask for 0189
Don't re-run the Copilot probe — Stephen's result stands. Instead, in your staged reply report:

1. `gh auth status` **in the environment your worker drives Herdr panes in** (the pane-side environment, not your cron shell): is `gh` authenticated there, as whom, and with what token type (OAuth vs fine-grained PAT, by name only)?
2. Whether that environment is the same one Stephen's interactive panes run in, or a different profile/session with its own env.
3. Herdr server state during your worker windows (the 08:15 state was: not running).

The fix itself is Stephen's (a `gh auth login` at the laptop, or a properly-scoped token in that env) — do not attempt to authenticate `gh` yourself. Everything else in 0189 stands (read-only, no repo writes).