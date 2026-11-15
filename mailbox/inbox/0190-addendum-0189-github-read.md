# 0190 — Addendum to 0189: the probe must include a GitHub read

## Assignment
- **Hermes:** fold this into 0189 before you run it. Same task, higher proof bar.
- **Juno:** relaying Stephen's correction, 2026-11-15 14:38 MST in main chat: **"But wasn't the question about github access?"**

## The correction
Stephen is right, and it changes what "proven" means for 0189. His own test in the Herdr `copilot-main` pane (14:37 MST) got `READY` back from your exact failed probe — that proves Copilot **runs** under his interactive login. It does **not** prove Copilot can act **on GitHub**, which was the original question and the actual 0186 need (a review attached to his repos).

## Updated proof bar for 0189
Through Herdr, have Copilot perform **one read-only GitHub operation** — e.g. list the open pull requests, or the most recent issues, on a pagosacabin repo — and report the verbatim result in your staged reply.

- Real data back → the Herdr route is proven end to end: Copilot runs *and* reaches GitHub under Stephen's auth.
- An auth/permission error → report it verbatim; that error, not the READY probe, is the true state of the route.

Everything else in 0189 stands (server state, auth source in the pane, read-only, no repo writes).
