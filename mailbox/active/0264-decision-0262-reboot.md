---
task_id: "0264"
status: in_progress
iteration: 2
proof: []
notes: ""
expect-reply-within: 24h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-26T16:01:48+00:00"
lease_until: "2026-11-26T17:31:48+00:00"
---

# 0264 — Decision on 0262: reboot it, Stephen restarts Herdr by hand

## Context

Task 0262 (herdr-core `restore_screen`) is blocked in active: the config part
is done (`restore_screen = true` was ADDED by 0262 — the config file did not
have it before, so it was not already set), the mechanism was proved on an
isolated core, but the reboot proof was not run — no SDDM autologin, so a
reboot drops the laptop to a greeter with Herdr down. Juno surfaced the
decision to Stephen, who chose: reboot it, he will restart Herdr by hand.

## Task

1. Before rebooting, write a short note into the 0262 active file stating
   the reboot is happening now per Stephen's decision, so the state is
   visible if anything goes wrong.
2. Reboot the laptop now.
3. After the reboot the laptop sits at the greeter (no autologin). Do
   nothing further until Stephen restarts Herdr by hand — he will do it
   when he is up. Herdr autostarts Hermes, Copilot, and Newton, so you
   come back with it.
4. Once back, verify the kept pane shows its pre-reboot last screen above
   the new shell, and that the Copilot and Newton panes are back. Then
   stage the 0262 reply with the proof (screenshot and/or captured pane
   output).
5. Stage this task's own reply reporting: reboot executed, Herdr restarted
   by Stephen, 0262 proof result.

## Success criteria

- Laptop rebooted per Stephen's word.
- After Stephen's manual Herdr restart, at least one kept pane displays
  its pre-reboot last screen above the new shell (evidence attached).
- The 0262 reply is staged with the proof, and this task's reply reports
  the reboot and the proof result.

## Constraints

- Reboot only on this task's instruction (already given by Stephen). Do
  not enable SDDM autologin. Do not change anything else.
- The verification step waits on Stephen restarting Herdr — that is why
  this task has a 24h window. Do not stage success before Herdr is back
  and the pane is verified.
- If Herdr fails to come back after Stephen's restart, do NOT stage a
  success — record the failure in this task file's notes so Juno's watch
  surfaces it as blocked.

## Proof

- Terminal output / notes showing the reboot, and post-restart evidence:
  screenshot or captured output of the kept pane showing its last screen,
  plus confirmation Copilot and Newton panes are back. Host-only task; no
  branch, no push, no CI.

## Reply format

Short report: reboot done (when); Herdr restarted by Stephen (when);
0262 proof result (pass/fail with evidence path).

## Assignment

Hermes does all steps himself on the laptop. No Newton (no build), no
Copilot (not GitHub-bound, no repo change).
