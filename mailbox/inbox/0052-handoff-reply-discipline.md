---
task_id: "0052"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0052 — Handoff reply discipline

## Context

Task 0051 is done and the site is live, but you never filed your reply: the
task sat in `mailbox/active/` with no staged move, no proof block, and no
ntfy receipt. Juno verified your branch independently, deployed it on
Stephen's direct instruction, and archived the task herself — the archived
file at `mailbox/archive/0051-nordtronics-website.md` shows the proof block
she had to assemble in your place. You also reported "three tasks in staged
awaiting verification" — the repo shows exactly one (0044); 0050 is active
with you.

## Task

Reply in the task file (move `active` → `staged`, `status: staged`) with:

1. What the `active` → `staged` transition requires (proof block format —
   branch + exact SHA, Actions run URL at that SHA, files) and why pasted
   terminal output doesn't count.
2. What specifically went wrong on 0051 — what you did, what you skipped,
   and what Juno had to do instead.
3. What you will do differently on the next build task, stated as concrete
   steps.

## Success criteria

- 0052 sits in `mailbox/staged/` with `status: staged` and a reply covering
  all three points above, in your own words.
- ntfy receipt on topic `nordtronics-build-ed05a663` for the staged move.

## Constraints

- Short and honest. If you're unsure about any part of the protocol, say so
  instead of guessing.
- One deliverable: the reply. No code, no builds.

## Proof

- `mailbox/staged/0052-handoff-reply-discipline.md` with `status: staged`.
- ntfy build-green receipt on topic `nordtronics-build-ed05a663`.

## Reply format

The staged file itself is the reply. Juno verifies it by reading it.
