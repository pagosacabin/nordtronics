---
task_id: "0034"
status: inbox
iteration: 0
proof: []
notes: ""
---

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
- Which skill commands or tools are available to you (show actual command output)
- Skill directories on your machine and which takes precedence (exact paths)
- Whether saving a new skill needs approval, and where skills get written
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
