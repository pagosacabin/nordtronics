---
task_id: "0263"
status: filed
iteration: 1
proof: []
notes: ""
expect-reply-within: 6h
---

# 0263 — Verify which Herdr binary carries worktree commands (report only)

## Context

Task 0193 (Nov 16) probed the laptop and found Herdr's worktree feature in
the `herdr` 0.9.5 client (server not running), NOT in the live herdr-core
`hc` 0.1.21 manager. Herdr's docs describe `herdr worktree create/list/open/
remove` and a `worktrees.directory` setting (default `~/.herdr/worktrees`),
but the docs cover preview builds too, so the on-machine truth needs
checking before anyone relies on it. Stephen asked for this verification.

## Task

Report-only. Change nothing: no settings edited, no workspaces
created/closed, no checkouts made.

1. Run the worktree commands against BOTH binaries and report which answers:
   `herdr worktree list` (and `--help`) and `hc worktree list` (and
   `--help`). Record each binary's version and the exact result (works /
   unknown subcommand / error text).
2. List the current workspaces on the live manager (`hc ls` and/or the
   workspace list command) and report each workspace's working directory —
   what exists now and what each is rooted at.
3. Report the `worktrees.directory` setting value from the config file that
   actually governs the live setup, and whether `~/.herdr/worktrees`
   exists on disk.

## Success criteria

- A report naming: (a) which binary (if any) carries working worktree
  commands, with versions and exact outputs; (b) the current workspace
  list with each workspace's cwd; (c) the `worktrees.directory` value and
  whether the directory exists.

## Constraints

- Report-only: do not edit any config, do not create/close workspaces, do
  not create/remove checkouts.
- If neither binary carries worktree commands, say so plainly — that is a
  valid result, not a failure to report around.

## Proof

- Terminal output of the version checks, the worktree list/help attempts
  on both binaries, and the workspace listing. Host-only task; no branch,
  no push, no CI.

## Reply format

Short report: which binary has worktree commands (or neither); current
workspaces and their cwds; `worktrees.directory` value and dir existence.

## Assignment

Hermes does all steps himself on the laptop. No Newton (no build), no
Copilot (not GitHub-bound, no repo change).
