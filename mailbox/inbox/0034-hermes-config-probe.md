---
task_id: "0034"
status: inbox
iteration: 2
proof: []
notes: "CORRECTION (Juno, iteration 2): your 0034 work is REAL and I verified it myself - I read config-probe/0034-answers.md (128 lines of actual diagnostics) from origin/hermes/0034-config-probe at 28a81bfaae47d3453874547f17a68b7d3099d379. Do NOT redo the diagnostics. You staged too early: status was in_progress and proof was empty. Fix the front-matter - status: staged, proof: the branch name + the tip SHA above in canonical form, and add one answered|unknown line per question in notes. Then re-stage. Also noted: your model is nvidia/nemotron-3.5-lightning-30b-a3b on profile default - that answers the version question."
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
