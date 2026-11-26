---
task_id: "0262"
status: in_progress
iteration: 2
blocked: true
proof: []
notes: |
  BLOCKED — decision owed before the reboot; NO reboot was performed and there is no proof block.

  Step 1 (done): herdr-core's config is ~/.config/herdr-core/config.toml (located from the
  binary's own help text, not guessed). It did NOT exist: the dir held only templates/.
  Step 2 (done): created it with `restore_screen = true` at 2026-11-26 08:17 MST. So the option
  was ABSENT before this task (documented default false) and this task added it. Version on the
  host is herdr-core 0.1.22 (the context says 0.1.21); `restore_screen` is one of its parsed
  config keys and help text confirms the semantics. Evidence:
  mailbox/files/0262/restore-screen-config-before-after.txt.

  Mechanism proved without touching Stephen's panes: a second, isolated herdr-core
  (own XDG_CONFIG_HOME/XDG_STATE_HOME/HC_SOCKET) ran a kept terminal, lost its program via
  `hc server stop`, and recovered it with the old screen above "-- the program above ended;
  this is a new shell --". Negative control with restore_screen=false in the same harness
  recovered to a blank shell. Real core untouched throughout (pid 830950, 7 live terminals
  before and after). Evidence:
  mailbox/files/0262/restore-screen-mechanism-proof.txt.

  Step 3 (reboot proof) NOT DONE — deliberately, not for lack of trying:
  the autostart chain this task asks me to verify cannot survive a reboot unattended on this
  machine. /etc/sddm.conf has [Autologin] with User= and Session= commented out, /etc/sddm.conf.d/
  is empty, and the boot record agrees: boots -3 and -1 produced NO user session at all, and
  this boot sat at the greeter for 11 min 37 s before Stephen logged in. herdr-core has no boot
  unit (it runs from the transient hc-serve-9ce03d48.service) and its launchers are XDG autostart
  entries (herdr-app.desktop, herdr-agents.desktop) inside a graphical session, so after a reboot
  herdr/Copilot/Newton stay down until someone logs in; only hermes-gateway.service returns
  (enabled + linger). Rebooting now would take 7 live terminals (t3 Stephen's Hermes,
  t17 copilot-main turn 308, t19 agy-main, t1n antigravity, three shells) to a greeter with
  herdr down, while Stephen is away for the day — i.e. it would knowingly produce the failure the
  task says to report as blocked, at the cost of his whole session set. Evidence:
  mailbox/files/0262/restore-screen-autostart-chain.txt (all read-only).

  DECISION OWED (any one unblocks the reboot proof):
   (a) enable SDDM autologin — root-owned, the worker has no sudo password:
       /etc/sddm.conf.d/10-autologin.conf with [Autologin] User=astroboy Session=plasma
       (and Relogin=true), then the next tick can reboot and verify criteria 2 and 3 unattended;
   (b) or say "reboot it" and the worker reboots — the desktop will NOT come back by itself,
       hermes-gateway will (linger), and the worker would have to start herdr-core by hand
       afterwards to read the restored panes, which is a hand-started chain, not autostart;
   (c) or accept a core-restart proof instead of a reboot: `hc server stop/start` reproduces the
       same recovery path, but it kills the live programs in Stephen's panes (Copilot, Newton,
       his interactive Hermes) — needs his explicit go-ahead, not the worker's.

  Host changes made by this task: created ~/.config/herdr-core/config.toml (434 B, mode 644);
  a temporary isolated core under the worker scratch dir, stopped and killed afterwards. No repo
  changes, no reboot, no changes to any pane, no Newton, no Copilot, no branch/CI (host-only task).

  0264 DECISION RECEIVED — the blocker above is resolved. Stephen chose the reboot
  (option b) and Juno filed 0264 to carry it. The worker is rebooting the laptop NOW
  under 0264; no SDDM autologin was enabled and nothing else was changed. The "before"
  half of the reboot proof is captured and committed at
  ../files/0262/pre-reboot-pane-capture.txt (7 live panes, chosen proof pane t17
  copilot-main, distinctive strings 'Verdict: SHIP' / 'Session: 119.51 AIC used' /
  'fix-shell-env'). After the reboot the machine sits at the greeter until Stephen logs
  in and restarts Herdr by hand; criteria 2 and 3 (kept pane shows its pre-reboot
  screen above the new shell; Herdr/Hermes/Copilot/Newton back) are verified after
  that, and this file is staged only then — not before.

  One redaction, declared: the capture as first taken contained one OpenRouter API
  key that pane t1d's scrollback held (a `cat ~/.hermes/openrouter-api-key.txt`
  from an earlier session). GitHub push protection blocked the push on it; the key
  is replaced by a marker in the committed file, and the key itself appears nowhere
  in this repo or in the reply. Stephen should rotate it — it was echoed in plain
  pane scrollback and is likely also in ~/.zsh_history.
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

## Result (2026-11-26 08:30 MST) — BLOCKED, decision owed; no reboot performed

**Was `restore_screen` already set?** No. `~/.config/herdr-core/config.toml` did not exist at all
(the directory held only `templates/`), so the documented default (false) was in force.
The path was located from the binary's own help text
(`Set restore_screen = true in ~/.config/herdr-core/config.toml ... It defaults to false`),
and `strings herdr-core` lists `restore_screen` among the config keys. Installed version is
**0.1.22**, not the 0.1.21 named in the context.

**What changed.** I created that file with `restore_screen = true` (434 B, mode 644). The running
core was not restarted; the option is read at recovery, and a reboot is by definition a fresh core.

**Mechanism proven (not the reboot).** A second, isolated herdr-core — its own
`XDG_CONFIG_HOME`/`XDG_STATE_HOME`/`HC_SOCKET`, so it could not reach Stephen's core — ran a kept
terminal holding `RESTORE-PROOF-0262-LINE-A … LINE-B`, lost the program via `hc server stop`, and
recovered it showing that screen above:

```
RESTORE-PROOF-0262-LINE-B
-- the program above ended; this is a new shell --
[astroboy@home]…/work%
```

Negative control, same harness with `restore_screen = false`: recovery came back **blank** (only the
prompt). So the behaviour tracks the option, not just the recovery. The real core (pid 830950, 7 live
terminals) was untouched before and after.

**Reboot proof: not done, and I do not think it should be done unattended.** The chain this task asks
me to verify cannot survive a reboot on this laptop without a human:

- `/etc/sddm.conf`'s `[Autologin]` has `User=` and `Session=` commented out and `/etc/sddm.conf.d/`
  is empty — no autologin.
- The boot record agrees: boots `-3` and `-1` produced **no user session at all**, and this boot sat
  at the greeter for 11 min 37 s before Stephen logged in.
- `herdr-core` has no boot unit (transient `hc-serve-…` only); its launchers are XDG autostart entries
  that run inside a graphical session. Only `hermes-gateway.service` returns on its own (enabled +
  linger).

So a reboot right now would take Stephen's 7 live terminals (his interactive Hermes in t3,
`copilot-main` at turn 308, `agy-main`, `antigravity`, three shells) to a greeter with Herdr, Copilot
and Newton down and nothing able to bring them back until he logs in — while he is away for the day.
That is the failure branch the task says to record as blocked, and it is knowable **before** spending
his environment on it. Hence: parked, not staged, no `proof` block.

**Decision owed — any one of these unblocks it:**

1. **Enable SDDM autologin** (root-owned; the worker has no sudo password): 
   `/etc/sddm.conf.d/10-autologin.conf` with `[Autologin] User=astroboy Session=plasma Relogin=true`.
   Then the next tick can reboot and verify criteria 2 and 3 unattended.
2. **Say "reboot it".** It will go through, but the desktop will not come back by itself: Hermes will
   (linger), Herdr will not — the worker would start the core by hand afterwards, which is a
   hand-started chain, not the autostart chain the criterion names.
3. **Accept a core-restart proof instead** (`hc server stop` / `start` hits the same recovery path):
   it would prove restore_screen on the real panes, but it kills the programs running in them —
   Copilot, Newton and Stephen's interactive Hermes — so it needs his go-ahead, not mine.

Evidence (committed with this file):

- `../files/0262/restore-screen-config-before-after.txt` — config absent → created, verbatim.
- `../files/0262/restore-screen-mechanism-proof.txt` — true/positive and false/negative runs.
- `../files/0262/restore-screen-autostart-chain.txt` — the read-only chain evidence above.

Host changes: created `~/.config/herdr-core/config.toml`; a temporary isolated core (stopped and killed).
No reboot, no pane touched, no repo change, no branch/CI (host-only task).
