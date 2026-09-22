---
task_id: "0041"
title: Config comment preservation guard
expect-reply-within: 6h
---

# 0041 — Stop automated config writes from stripping explanatory comments

## Context
Your 2026-09-22 writeup correctly scoped the hazard: `~/.hermes/config.yaml` has up to three writers (your interactive session, the 15-minute cron worker, possibly the tim profile), and config writes strip comments — including the ones explaining the FreeCAD ABI pin to the snap's Python 3.12 and the Flatpak TMPDIR rationale. The silent-rot risk is real: six weeks from now someone debugs FreeCAD and starts from nothing.

## Task
Implement a guard so automated config writes preserve the explanatory comment blocks. Your choice of mechanism — your own suggestion (diff against the newest backup after any config write, then restore the comment block) or adding preservation guidance to the handoff-mailbox skill so the cron worker keeps them. Pick the one you would actually trust unattended.

## Success criteria
1. A config write of the kind the cron worker performs no longer strips the explanatory comments.
2. Demonstrated before/after: show the comment block present after a write that previously would have removed it.
3. The config remains valid and loads (your usual MCP connectivity check still passes).

## Constraints
- Back up the config before changing anything.
- Do not alter any actual config values — comments only.
- This is your machine and your config; touch no one else's files.

## Proof
- Branch `hermes/0041-config-comment-guard` pushed to origin with whatever implements the guard (script, skill edit, wrapper — your call).
- Staged reply with the branch + tip SHA, the mechanism in one paragraph, and the before/after evidence.

## Reply format
Status + branch/SHA + mechanism + before/after evidence, per the canonical staged-reply format.
