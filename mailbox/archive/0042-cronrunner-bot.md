---
task_id: "0042"
title: Dedicated cron-runner bot (separate profile)
status: staged
iteration: 1
proof:
  - branch: hermes/0042-cronrunner-bot
    sha: 17709b1322379e3202980db039313916f146ad7a
notes: |
  Status: done. `cronrunner` exists as a separate Hermes profile (Bot Mode's
  primitive — a Bot *is* a profile) with its own config.yaml, memories/,
  sessions/ and cron/ store. It was created with `hermes profile create
  cronrunner --clone`, trimmed to a headless worker, given a distinct git
  identity, and the 15-minute mailbox job now fires from system crond against
  `hermes -p cronrunner`. The interactive default profile's copy of the job
  (2148c3e14463) is paused and no longer the live one; no other change was made
  outside the new profile.

  Live tick: crond fired the job at 2026-09-22T18:15:01-06:00 (execution
  72ba689e3c3d415298095b4e5f53bca4, job c0be50a686c6) and the worker ran the
  mailbox loop on the new profile — it loaded handoff-mailbox/github/
  kicad-mcp-server, picked up task 0043, created a worktree (wt-0043, branch
  hermes/0043-tl431-swap) and worked the TL431 swap on hardware/solar-gate-v1.

  Worker-identity commit on origin: 25993dc3562b15ccb43572ebed643e50f1473140
  (branch hermes/0043-tl431-swap, author hermes-cronrunner
  <hermes-cronrunner@nordtronics.local>). The tick was still running when this
  file was written, so more worker commits may follow.

  One caveat, reported rather than papered over: the job prompt says to pick "the highest-numbered task file with `status:
      inbox`", but your newer task files (0041, 0042, 0043) carry no `status`
      field at all — the literal selector matches nothing. The worker found the
      tasks by filename anyway. Worth adding `status: inbox` to new task files or
      relaxing the selector in the prompt.

  Proof is branch + tip SHA only, no Actions run pointer: both workflows are
  path- or branch-filtered (android-build.yml is pinned to
  android-toolchain-setup, platformio.yml is path-filtered to firmware/**), so
  nothing matches docs/** or mailbox/**. Same convention as 0039. (gh CLI is not
  authenticated in this environment, so the workflow triggers were read from
  .github/workflows/*.yml rather than the Actions API.)

  Full record: docs/ops/cronrunner-profile.md on the proof branch.
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

---

## Reply

Full operational record: `docs/ops/cronrunner-profile.md` on branch
`hermes/0042-cronrunner-bot`.

### 1. Profile

```
/home/astroboy/.hermes/profiles/cronrunner/
  config.yaml        own config (trimmed; see §3)
  .env               copied — provider keys, HASS_TOKEN, DIGIKEY_* (27 KB)
  SOUL.md            copied
  memories/          copied (MEMORY.md, USER.md)
  sessions/          own (empty; created by the clone)
  cron/              own job store — jobs.json holds job c0be50a686c6
  skills/            9 skills (see §3)
  logs/              cron-tick.log, agent.log, errors.log
  workspace/ plans/ skins/ home/ hooks/ cache/ state.db  own
  .no-bundled-skills opt-out marker, so `hermes update` won't re-seed removed skills
```

Created with `hermes profile create cronrunner --clone`. `--clone` (not
`--clone-all`) deliberately: `--clone-all` would also carry `auth.json`,
`browser-profile/`, caches and `state.db` — runtime junk a worker does not need,
and its cron-exclusion behaviour is the same. Copying `.env` is what carries
`HASS_TOKEN` into the worker's cron context.

### 2. Cron definition

`crontab -l` (the mailbox job now invokes `-p cronrunner`):

```
SHELL=/bin/bash
# Juno ⇄ Hermes mailbox loop — runs on the dedicated `cronrunner` Hermes profile
# (mailbox task 0042) instead of inside the interactive `default` profile, so the
# worker and the interactive session never share one Hermes home.
# The GIT_* identity makes worker commits distinguishable from interactive ones
# (the nordtronics repo-local user.name is `astroboy`).
# PATH must include /usr/sbin (gh) and ~/.local/bin (node, hermes).
PATH=/home/astroboy/.local/bin:/usr/sbin:/usr/bin:/bin
*/15 * * * * GIT_AUTHOR_NAME=hermes-cronrunner GIT_AUTHOR_EMAIL=hermes-cronrunner@nordtronics.local GIT_COMMITTER_NAME=hermes-cronrunner GIT_COMMITTER_EMAIL=hermes-cronrunner@nordtronics.local /home/astroboy/.local/bin/hermes -p cronrunner cron tick --accept-hooks >> /home/astroboy/.hermes/profiles/cronrunner/logs/cron-tick.log 2>&1
```

The job itself lives in the profile's own store. Cron is per-profile by design
(`cron/jobs.py` anchors at `get_hermes_home()`), so a job in `default` can never
run under `cronrunner` — the job was *recreated* there via `hermes -p cronrunner
cron create '*/15 * * * *' …` (id `c0be50a686c6`) rather than copied, because
`--clone-all` knowingly excludes `cron/` to prevent double-firing.

`PATH` is explicit because crond's default is `/usr/bin:/bin` while `gh` lives at
`/usr/sbin/gh` and `node` (the KiCad MCP runtime) at `~/.local/bin/node`. The
mechanism was proven in a cron-like minimal env (`env -i HOME=… SHELL=/bin/bash
PATH=…`) *before* the job was resumed.

Old writer: the default profile's job `2148c3e14463` is paused. That is the
repoint — one live writer, not two.

### 3. Disabled — one line of reasoning each

**Toolsets** (`agent.disabled_toolsets`, inherited by every cron run and *not*
widenable by a per-job `enabled_toolsets` list —
`cron/scheduler.py::_resolve_cron_disabled_toolsets`):

| Toolset | Why it is off |
|---|---|
| `a2a` | agent-to-agent chat surface — interactive only |
| `browser` | browser-use cloud session; git/gh/KiCad need no browser |
| `computer_use` | drives a desktop GUI; nothing to drive headless |
| `connections` | desktop multi-gateway connection surface |
| `homeassistant` | home control; unrelated to the mailbox loop |
| `image_gen` | no image generation in scheduled work |
| `kanban` | this worker does not dispatch board tasks |
| `session_search` | one job per tick; no cross-session recall to do |
| `tts` | no voice output on a headless tick |
| `vision` | no image analysis in the loop |

(The framework additionally force-disables `messaging`, `clarify` and `cronjob`
for every cron-spawned agent.) Also `stt.enabled: false` (no microphone) and
`kanban.review_dispatch: false` (not an orchestrator).

**Toolset allowlist** (`platform_toolsets.cron` — what cron actually resolves):
`code_execution, delegation, file, memory, skills, terminal, todo, web`, plus the
`kicad` and `freecad` MCP toolsets merged in automatically. Verified by resolving
the profile's own config through `_resolve_cron_enabled_toolsets`:

```
enabled:  code_execution, delegation, file, freecad, kicad, memory, skills, terminal, todo, web
disabled: a2a, browser, clarify, computer_use, connections, cronjob, homeassistant,
          image_gen, kanban, messaging, session_search, tts, vision
```

**Skills** — 75 inherited, 66 removed, 9 kept: `handoff-mailbox`,
`async-inbox-task-loop`, `hermes-agent`, `hermes-cron-management`, `github`,
`kicad-mcp-server`, `freecad-mcp-headless`, `systematic-debugging`,
`requesting-code-review`. One shared reason for the removals: the loop is a
git/gh/EDA worker, so the creative, media, note-taking, email, social,
productivity, Apple and research trees (plus the non-cron autonomous-agent
skills) have no scheduled-work consumer. Any of them restores with a copy from
`~/.hermes/skills/`.

**MCP servers** — none disabled. Both configured servers (`kicad`, `freecad`) are
used by scheduled work and stay enabled.

**Messaging surfaces** — nothing to disable: no `platforms:` block exists in this
home and the copied `.env` holds no messaging tokens. No gateway is installed for
this profile; the ticker is crond.

### 4. Commit identity

`GIT_AUTHOR_*`/`GIT_COMMITTER_*` are exported by the crontab line; they beat the
repo-local `user.name` (`astroboy`). Worker commits are attributable as
`hermes-cronrunner <hermes-cronrunner@nordtronics.local>`.

First commit on origin with that identity:

    25993dc3562b15ccb43572ebed643e50f1473140 (branch hermes/0043-tl431-swap)
    hermes-cronrunner <hermes-cronrunner@nordtronics.local>
    0043: swap D1 TLV431 -> TL431 (SOT-23 DBZ pinout, 2.5 V)

### 5. Live tick

Fired by crond at `2026-09-22T18:15:01-06:00` — execution
`72ba689e3c3d415298095b4e5f53bca4`, job `c0be50a686c6`, `source=builtin`. The
worker ran the loop on the new profile: `handoff-mailbox` + `github` +
`kicad-mcp-server` loaded, task 0043 picked up, worktree `/home/astroboy/wt-0043`
on branch `hermes/0043-tl431-swap`, TL431 swap worked in
`hardware/solar-gate-v1/` (schematic, symbol, netlist edits; `mcp__kicad__*` tool
calls). ~50 API calls and still running at staging time.

### Success criteria

1. ✅ Profile exists with own `config.yaml`, `memories/`, `sessions/`, `cron/`.
2. ✅ Disabled list above, one line each; loop dependencies intact — proven live
   (the tick loaded the mailbox skill and worked a task with kicad/freecad MCP.
3. ✅ `crontab -l` shows the job invoking `hermes -p cronrunner cron tick`.
4. ✅ Worker-identity commit on origin `25993dc` (branch `hermes/0043-tl431-swap`) —
   author `hermes-cronrunner <hermes-cronrunner@nordtronics.local>`.
5. ✅ Interactive profile untouched — the only change outside `cronrunner` was
   pausing its duplicate copy of this job, which the repoint requires.

### Rollback

```bash
crontab -r                                  # stop the worker ticker
hermes profile delete cronrunner            # remove the profile
hermes cron resume 2148c3e14463             # restore the default-profile job
```
