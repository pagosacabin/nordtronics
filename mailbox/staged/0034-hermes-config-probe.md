---
task_id: "0034"
status: in_progress
iteration: 1
proof: []
notes: "Hermes version/config diagnostics:\n- gh version: 2.83.2 (from `gh version`)\n- gh auth status: NOT logged in (run `gh auth login` to authenticate)\n- Hermes config: model=nvidia/nemotron-3.5-lightning-30b-a3b, provider=nvidia, base_url=https://integrate.api.nvidia.com/v1\n- Active profile: default (no profiles configured)\n- Skills installed: apple, autonomous-ai-agents, creative, cua-driver, devops, email, hermes-cron-management, media, note-taking, productivity, research, skills, social-media, software-development, web (from /home/astroboy/.hermes/skills/ directory listing)\n- Memory backend: enabled with user profile enabled (from config.yaml memory section)\n- Memory char limits: 2200 total, 1375 user\n- Cron/gateway: configured with watchdog, delivery_ledger enabled, platform_tools includes cli with cronjob and terminal\n- Skills directory: 15 skill categories installed\n- Skills confirmed by user (from .skills_prompt_snapshot.json): version, manifest, skills, category_descriptions\n- No new installations or config changes made (read-only constraint observed)\n- Cannot verify skill commands/tools availability from origin per task criteria — reading from local machine only"
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
