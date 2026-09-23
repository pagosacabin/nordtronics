# cronrunner — dedicated Hermes profile for scheduled work

Operational record for mailbox task **0042**. The 15-minute Juno ⇄ Hermes mailbox
loop used to run inside the interactive `default` profile's cron ticker — two
agent processes on one Hermes home, which the framework docs warn against ("don't
point two agent processes at one Hermes home"). The loop now runs on its own
profile, `cronrunner`, so each writer owns a home.

Nothing in `mailbox/README.md` or the mailbox state machine changed. This is an
operational change only.

## 1. Profile

- Path: `~/.hermes/profiles/cronrunner/`
- Created with `hermes profile create cronrunner --clone` (config.yaml, `.env`,
  `SOUL.md`, all installed skills, `memories/MEMORY.md`, `memories/USER.md`).
  `--clone` rather than `--clone-all`: `--clone-all` additionally carries the
  source profile's runtime junk (`auth.json`, `browser-profile/`, caches), and a
  worker does not need any of it. Own `cron/`, `sessions/`, `memories/` are
  created by the clone.
- Gateway: not installed. The ticker runs from system crond (below), so no
  messaging surface exists on this profile at all.
- `hermes -p cronrunner skills opt-out` → `.no-bundled-skills` marker, so
  `hermes update` does not re-seed the bundled skills that were removed.

## 2. Cron definition

`crontab -l`:

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

The job itself lives in the profile's own store
(`~/.hermes/profiles/cronrunner/cron/jobs.json`, id `c0be50a686c6`) — cron is
per-profile by design (`cron/jobs.py` anchors at `get_hermes_home()`), so a job in
the `default` profile can never run under `cronrunner`.

`PATH` is set explicitly: crond's default is `/usr/bin:/bin`, but `gh` is at
`/usr/sbin/gh` and `node` (the KiCad MCP server runtime) is at
`~/.local/bin/node`. The mechanism was proven in a cron-like minimal environment
(`env -i HOME=... SHELL=/bin/bash PATH=...`) before the job was resumed.

## 3. Commit identity

`GIT_AUTHOR_*` / `GIT_COMMITTER_*` are exported by the crontab line. They beat the
repo-local `user.name` (`astroboy`), so scheduled commits are attributable:

```
hermes-cronrunner <hermes-cronrunner@nordtronics.local>
```

## 4. Disabled — one line of reasoning each

### Toolsets (`agent.disabled_toolsets`, inherited by every cron run)

The framework already force-disables `messaging`, `clarify` and `cronjob` for cron
spawned agents (`cron/scheduler.py::_resolve_cron_disabled_toolsets`). Layered on
top, and *not* widenable by a per-job `enabled_toolsets` list:

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

Config additions: `stt.enabled: false` (no microphone) and
`kanban.review_dispatch: false` (not an orchestrator).

### Toolset allowlist for cron (`platform_toolsets.cron`)

Cron runs resolve toolsets from `platform_toolsets.cron`
(`_resolve_cron_enabled_toolsets` → `_get_platform_tools(cfg, "cron")`). Verified
resolved surface with the profile's own config:

```
code_execution, delegation, file, freecad, kicad, memory, skills, terminal, todo, web
```

`kicad` and `freecad` are merged in automatically because both MCP servers are
enabled — the mailbox loop keeps its PCB/EDA capability.

### Skills

75 skills were inherited from `default`; 66 were removed, leaving 9:

`handoff-mailbox`, `async-inbox-task-loop`, `hermes-agent`,
`hermes-cron-management`, `github`, `kicad-mcp-server`, `freecad-mcp-headless`,
`systematic-debugging`, `requesting-code-review`.

Removed with one shared reason: the mailbox loop is a git/gh/EDA worker, so the
creative, media, note-taking, email, social, productivity, Apple and research
skill trees (and the non-cron autonomous-agent skills) have no scheduled-work
consumer. Restoring any of them is a copy from `~/.hermes/skills/`.

### MCP servers

None disabled — both configured servers (`kicad`, `freecad`) are used by scheduled
work and both stay enabled.

### Messaging surfaces

Nothing to disable: no `platforms:` block exists in this home and the copied
`.env` carries no messaging tokens (only provider keys, `HASS_TOKEN`,
`DIGIKEY_*`). No gateway is installed for this profile.

## 5. Live tick

The crontab fired the job at `2026-09-22T18:15:01-06:00` (execution
`72ba689e3c3d415298095b4e5f53bca4`, job `c0be50a686c6`). The worker ran the mailbox
loop on the new profile: loaded the `handoff-mailbox` / `github` /
`kicad-mcp-server` skills, picked up task 0043, created a worktree
(`wt-0043`, branch `hermes/0043-tl431-swap`) and worked the PCB change.

First commit on origin carrying the worker identity:

```
25993dc3562b15ccb43572ebed643e50f1473140
hermes-cronrunner <hermes-cronrunner@nordtronics.local>
0043: swap D1 TLV431 -> TL431 (SOT-23 DBZ pinout, 2.5 V)
branch: hermes/0043-tl431-swap
```

## 6. Rollback

```bash
crontab -r                                   # stop the worker ticker
hermes profile delete cronrunner             # remove the profile
hermes cron resume 2148c3e14463              # (only if the default-profile job is kept paused)
```
