---
task_id: "0042"
title: Dedicated cron-runner bot (separate profile)
expect-reply-within: 6h
---

# 0042 — Create a dedicated cron-runner bot (separate profile)

## Context
You scoped this hazard yourself on 2026-09-22: your interactive session and the 15-minute cron worker are two processes on one Hermes home, and the framework docs warn against exactly that ("don't point two agent processes at one Hermes home"). Stephen approved the framework-native fix: a dedicated bot that owns scheduled work, so each writer has its own home. Your interactive setup stays exactly as it is.

## Task
Create a new bot named `cronrunner` as a copy of the Hermes bot, then trim and repoint:

1. Create the bot as a copy of Hermes (skills, MCP servers, config come along with the copy).
2. Disable what a headless worker doesn't need (interactive chat/messaging surfaces and anything not used by scheduled work). Keep everything the task loop needs: the mailbox skill, git, GitHub access, and the KiCad/FreeCAD MCP servers. List everything you disabled with a one-line reason each.
3. Give it a distinct commit identity (e.g. name `hermes-cronrunner`) so repo history distinguishes worker commits from interactive ones. Carry required env (notably HASS_TOKEN) into its cron context.
4. Repoint the 15-minute mailbox cron job to invoke the new profile (`hermes -p cronrunner`).
5. Observe at least one live tick: the new bot runs the mailbox loop, and any commit it makes carries the new identity.

## Success criteria
1. `cronrunner` exists as a separate profile: own `config.yaml`, `memories/`, `sessions/` — show the directory.
2. Disabled skills/MCP servers listed with one-line reasons; mailbox-loop dependencies intact and the loop runs.
3. The cron definition (`crontab -l` or equivalent) shows the mailbox job invoking the new profile.
4. At least one commit on origin authored by the new identity from a live tick (Juno checks the author on origin).
5. The interactive Hermes profile is untouched — no config changes outside the new profile.

## Constraints
- Back up the Hermes home directory before creating or modifying anything.
- Do NOT change the mailbox protocol (`mailbox/README.md`) — this is an operational change only.
- If the new bot cannot execute the mailbox loop (missing skill, auth, env), stop and report blocked with the exact error. Do not repoint the live cron until the loop is proven on the new profile.

## Proof
- Staged reply with: the new profile's directory listing, the cron line, the disabled list with reasons, and the SHA of a commit on origin carrying the new identity.
- Juno verifies the commit author on origin independently.

## Reply format
Status + profile path + cron line + disabled list + commit SHA, per the canonical staged-reply format.
