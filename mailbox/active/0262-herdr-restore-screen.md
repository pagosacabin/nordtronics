---
task_id: "0262"
status: in_progress
iteration: 2
proof: []
notes: ""
expect-reply-within: 6h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-26T15:17:37+00:00"
lease_until: "2026-11-26T16:47:37+00:00"
---

# 0262 — herdr-core restore_screen: verify, enable if missing, prove across a reboot

## Context

Daily learning (2026-11-26, verified against the official herdr-core CHANGELOG.md):
herdr-core 0.1.21 — the version already installed on the laptop — adds a
`restore_screen` option. Set `restore_screen = true` in herdr-core's
config.toml and kept terminals show their program's last screen above the new
shell after a reboot, instead of coming back blank.

Stephen believes this is already implemented — he has done this before — but
asked to run the check anyway. Herdr autostarts Copilot and Newton
(Antigravity) when it starts up, and it also starts up Hermes, so the reboot
proof doubles as a check of that whole startup chain.

## Task

1. Find herdr-core's config.toml on the laptop (locate it, don't guess the
   path) and read it. Report whether `restore_screen` is present and its
   value.
2. If `restore_screen` is absent or false, set `restore_screen = true`.
   If it is already set to true, change nothing — say so plainly.
3. Proof across a reboot: pick (or keep) one pane with distinct,
   identifiable content, reboot the laptop, and after the autostart chain
   brings Herdr and Hermes back, verify the kept pane shows its pre-reboot
   last screen above the new shell. Capture the evidence (screenshot and/or
   captured pane output).

## Success criteria

- config.toml contains `restore_screen = true`, with a clear statement of
  whether it was already set or added by this task.
- After a reboot, at least one kept pane displays its pre-reboot last screen
  above the new shell (evidence attached: screenshot or captured terminal
  output).
- The autostart chain is verified after the reboot: Herdr up, Hermes
  running, Copilot and Newton panes back.

## Constraints

- Holiday morning; Stephen is away from the laptop. Do the reboot as part of
  this task only — no other reboots, no other changes.
- If the autostart chain fails to bring Hermes back after the reboot, do NOT
  stage a success. Record the failure in this task file's notes so Juno's
  watch surfaces it as blocked.
- Host-only config task: no repo changes, no branch, no PR, no merge, no
  Copilot review (not GitHub-bound).
- If `restore_screen` is already set, still run the reboot proof — Stephen
  explicitly asked for it anyway.

## Proof

- Terminal output of the config.toml read (showing the `restore_screen`
  line or its absence) and of the edit, if any.
- Post-reboot evidence: screenshot or captured output of the kept pane
  showing its last screen above the new shell, plus a note that Hermes,
  Copilot, and Newton are back.

## Reply format

Short report: was `restore_screen` already set (yes/no, and the config path
found); what changed, if anything; reboot proof result (pass/fail with
evidence path); autostart chain status after the reboot.

## Assignment

Hermes does all steps himself on the laptop. No Newton (no software build),
no Copilot (not GitHub-bound).
