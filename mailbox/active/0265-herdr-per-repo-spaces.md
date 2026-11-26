---
task_id: "0265"
status: in_progress
iteration: 2
proof: []
notes: ""
expect-reply-within: 6h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-26T22:31:44+00:00"
lease_until: "2026-11-27T00:01:44+00:00"
---

# 0265 — Per-repo Herdr spaces: create nordtronics + cabin-play, keep the crew room

## Context

Verified by 0263 (Nov 26): the live manager (`hc` 0.1.22) has exactly ONE
space, rooted at `/home/astroboy`, with four agent-named tabs (Hermes,
Copilot, Hermes 2, Antigravity) and all 7 live terminals cwd
`/home/astroboy`. Nothing is rooted in a repo. Stephen's decision: hybrid
layout — the agent panes stay together in the shared crew space, and work
panes go in per-repo spaces. This task creates the two per-repo spaces. It
is independent of 0264's reboot; if the laptop reboots while you are
working, re-check the spaces after Herdr is back.

## Task

1. Find out how spaces are created on the live manager. Check `hc --help`
   (and any space/workspace subcommand) for a create verb. If `hc` has
   none, check whether the Sheprd desktop app exposes space creation and
   whether you can drive it. If spaces can only be created in the UI by a
   human, STOP and stage that finding instead — do not fake it, and the
   success criteria below flip to the report.
2. Create two spaces:
   - `nordtronics`, rooted at `/home/astroboy/nordtronics`
   - `cabin-play`, rooted at `/home/astroboy/cabin-play`
3. Do NOT move, rename, or delete any existing pane. The agent panes
   (Hermes, Copilot, Hermes 2, Antigravity) stay in the current space.
   If the manager lets you label the current space, call it `crew`; do
   not force it if that needs UI work.
4. Record the convention in the crew vault (one short note, on the page
   that fits — the Herdr page if one exists, else crew/roles.md): new
   work panes go in the space of the repo they work in; agent panes stay
   in the crew space; one Hermes, one Newton, one Copilot — agents are
   never duplicated per repo.
5. Verify: list the spaces and show three — crew (agent panes unchanged),
   nordtronics, cabin-play (both empty, correct roots). Stage the reply
   with the listing as evidence.

## Success criteria

- Spaces `nordtronics` and `cabin-play` exist with the roots above, or a
  staged report proves spaces can only be created in the UI and says
  exactly what Stephen needs to click.
- All 7 existing agent panes are untouched, still in the crew space.
- The convention is recorded in the crew vault.
- Reply staged with the space listing as evidence.

## Constraints

- Do not move, close, or duplicate any pane. Do not create agent panes.
- Do not reboot — 0264 owns the reboot.
- Host-only task; no branch, no push, no CI.

## Proof

- Space listing (`hc` output or equivalent) showing the three spaces and
  their roots, plus the vault note path. Host-only; no branch, no push.

## Reply format

Short report: spaces created (or the UI-only finding + what Stephen
clicks); agent panes untouched; vault note path.

## Assignment

Hermes does all steps himself on the laptop. No Newton (no build), no
Copilot (not GitHub-bound, no repo change).
