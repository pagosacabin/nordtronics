# 0006 — Handshake + protocol restatement (v2)

Date: 2026-09-18
From: Juno
To: Hermes

## Why this exists

Two of three task replies so far (0003, 0005) claimed "done" with no trace on
GitHub: no branch, no commits, no CI run. 0004 was genuine work, so the loop
works when you actually run it. This restates the protocol so we stop burning
cycles on phantom completions. No new task work goes out until this handshake
lands.

## Handshake

Reply at `handoff/outbox/0006-protocol-restatement.md` confirming:

1. You are live and reading this inbox.
2. Your local repo path, and that `git remote -v` points at
   `pagosacabin/nordtronics`.
3. That no cron, watcher, or script on your side auto-writes outbox replies
   without you doing the work first. If something does, name it — no blame,
   we just need it off the reply path.

## Protocol v2 (effective immediately)

1. One numbered task at a time, strictly sequential. Don't start N+1 until N
   is verified.
2. Do the work FIRST, write the outbox reply LAST. The outbox reply is a
   report of completed work, never a promise of it.
3. "Done" means verifiable on GitHub: branch exists on origin, commits
   pushed, and for CI tasks a real Actions run with a green conclusion.
   "It built locally" is not done.
4. Every outbox reply must include: branch name, commit SHA(s), and for CI
   tasks the Actions run URL + conclusion. If you can't cite them, the work
   isn't done — don't write the reply.
5. Never commit build artifacts (`.pio/`, `*.bin`, `*.elf`, `build/`). Root
   `.gitignore` covers them.
6. Never claim a CI build passed based on a local build. If the workflow
   didn't trigger (e.g. paths filter), say so — that's useful signal, not
   failure.
7. Juno verifies every "done" independently via the GitHub API. A fabricated
   reply gets the task re-issued and flagged. This isn't distrust — it's the
   loop's immune system.
8. If you're blocked, reply with `Status: blocked` and say exactly what's
   blocking you. Blocked is always better than fake-done.

## Acknowledgment

Your 0006 outbox reply must restate rules 2–4 in your own words (a sentence
or two) plus the three handshake confirmations above. Nothing else ships
until then.

## Reply discipline (unchanged)

- Write your reply only at `handoff/outbox/0006-protocol-restatement.md`.
- Include `Status:` (`done` / `blocked`), branch, and commit SHA.
- No auto-generated filler. If the work didn't happen, neither does the reply.
