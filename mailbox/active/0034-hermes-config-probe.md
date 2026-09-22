--- 
task_id: "0034"
status: inbox
iteration: 3
proof:
  - branch: hermes/0034-config-probe
    sha: 28a81bfaae47d3453874547f17a68b7d3099d379
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35583228091
notes: "CORRECTION (Juno, iteration 3): the diagnostics work is DONE and accepted - do not touch the branch again. Two pointer errors in your proof, both fixable in the front-matter only: (1) the SHA you cited (28a81bfa...) is STALE - you pushed a new commit after I gave it to you, so the live tip is now 410efd7c2b1dad843d8bdeabd2b21748114c2e08. Lesson: run git rev-parse origin/hermes/0034-config-probe immediately before staging and cite exactly what it prints. (2) DELETE the run citation entirely - task 0034 has no CI run and never asked for one. The URL you cited is a FAILED Android build on a different branch; a wrong pointer is worse than no pointer. Re-stage with status: staged and proof listing ONLY the branch name and the live tip SHA above. That is the whole remaining task."
---"

# 0034 — Prove your local configuration: version, skills, memory

## Context

You run locally on your own machine as an agent. I have read the public
documentation for the Hermes Agent framework, but I do not know which of its
features your installation actually has configured. Answer from your own machine
— not from the docs.

## Task

Prove your local configuration. Run the diagnostics, commit the raw command
output to a new branch on origin, then stage this task pointing at it.

## Success criteria

1. I can read every answer from files on origin — not from pasted text.
2. Every claim names the exact command or file path it came from.

## Questions to answer

- Hermes version or commit (show the exact command you ran to get it)
- Active profile name, if profiles are in use
- Skills: Stephen confirmed skills are enabled on your installation as of 2026-09-21, with the GitHub skills added. Confirm from your side — which skill commands/tools are available (show actual command output), your skill directories and precedence (exact paths), and whether saving a new skill needs approval
- Memory backend in use, and whether memory writes are enabled
- Cron or gateway configuration relevant to how you poll this mailbox

## Constraints

- Read-only on your machine: do not change any configuration, do not install anything.
- Push to a new branch named `hermes/0034-config-probe`. Do not touch `main` or `android-toolchain-setup`.
- Put the raw output in `config-probe/0034-answers.md` on that branch. Paste full command output — do not summarize it away.

## Proof

`proof` lists the branch and tip SHA in the canonical form from the mailbox README.
I will read `config-probe/0034-answers.md` from origin myself.

## Reply format

- Status: done | blocked
- Branch: hermes/0034-config-probe
- Branch tip SHA: exact output of `git rev-parse origin/hermes/0034-config-probe`
- Per question: answered | unknown (if unknown, the exact command or path you checked)

expect-reply-within: 6h
