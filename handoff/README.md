# Juno ⇄ Hermes handoff

Async mailbox so Juno (cloud, design/management) and Hermes (local, KiCad + VS Code
via ACP) can work together on Nordtronics hardware and firmware. Stephen is the human
in the loop; nothing here ships to production without his say.

## How it works

- **Juno → Hermes:** Juno writes numbered specs to `handoff/inbox/`, e.g.
  `handoff/inbox/0002-wildfire-node-power.md`. Hermes pulls the repo, does the work
  locally, and replies in `handoff/outbox/` with a matching number, e.g.
  `handoff/outbox/0002-wildfire-node-power.md`.
- **Hermes → Juno:** results, exported schematics (PDF), netlists, build logs, and
  questions go in `handoff/outbox/`. Juno reads them via the GitHub API, reviews,
  and follows up with a new inbox file if changes are needed.
- **Status:** each outbox file starts with a `Status:` line — one of
  `in-progress`, `done`, `blocked`, `question`. Hermes updates it as work proceeds.
- **Numbering:** never reuse a number. Inbox and outbox numbers match 1:1 per task.
- **Keep it small:** binaries stay out of git. Schematics go in as PDF exports;
  KiCad project files live under the normal `hardware/` tree, referenced by path.

## Reply windows

- Each inbox task may carry an `expect-reply-within:` line (e.g.
  `expect-reply-within: 12h`). When absent, the default is **6h**.
- The window starts when the inbox file lands on `main` (commit time).
- If the window expires with no matching outbox reply, the task is **stale**.
  Juno's side flags it to Stephen and sends exactly one automatic nudge task.
  A task that stays stale after a nudge is Stephen's call — no nudge cascades.
- Hermes: if the window is slipping, post a short status note to the outbox
  before it expires. "In progress, need 4 more hours" beats silence.

## Templates

- `templates/build-spec-template.md` — Juno's spec format.
- `templates/review-template.md` — Juno's review format.

## Ground rules

1. Specs are binding; if a spec is ambiguous, ask in the outbox file (status:
   `question`) instead of guessing on anything that affects safety, cost, or pinout.
2. Power and high-voltage sections get a Juno review pass before boards are ordered.
3. Stephen approves all spending and all pushes to `main` that aren't handoff files.
