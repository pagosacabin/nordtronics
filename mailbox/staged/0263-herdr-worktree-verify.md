---
task_id: "0263"
protocol_version: 1.4.0
status: staged
iteration: 2
proof:
  branch: none
  sha: none
  run: none
  run_note: |-
    Host-only, report-only task (the card says so itself: "Host-only task; no branch, no push,
    no CI"). Nothing was edited, so there is no branch and no revision to point at. No workflow
    could run: this repo's workflows trigger on branch allow-lists and path filters and nothing
    was pushed. The evidence is the terminal transcript committed next to this reply:
    mailbox/files/0263/herdr-worktree-verify-transcript.txt (commands and their verbatim output,
    re-run in one block at 2026-11-26T08:46:23-07:00).
  files: []
  files_note: |-
    No repository file was changed. The only path added is the evidence transcript above.
  applied_to: []
  applied_note: |-
    Nothing applied anywhere: report-only. No config edited, no workspace created/closed, no
    checkout made. The real core was read through `hc ls` / `hc server status` only.
  evidence:
    - mailbox/files/0263/herdr-worktree-verify-transcript.txt
notes: |-
  Answer in one line: the worktree feature lives ONLY in the legacy `herdr` 0.9.5 client, and
  that client is not serving anything here — its server is not running, so `herdr worktree list`
  answers `server_not_running`. The live manager `herdr-core`/`hc` 0.1.22 has no worktree
  commands at all.

  (a) Which binary carries worktree commands.
    * `herdr` 0.9.5 — present on disk as ~/.local/bin/herdr-legacy-0.9.5 (30285672 bytes, dated
      Nov 6). It is NOT on PATH under the name `herdr`: `herdr --version` -> "command not found".
      `herdr worktree --help` exits 0 and lists four subcommands:
      "Manage Git worktree-backed workspaces / Usage: herdr worktree [COMMAND] / list, create,
      open, remove". `herdr worktree create --help` takes --workspace/--cwd/--branch/--base/
      --path/--label/--focus/--trust-repository (no --directory flag; placement comes from the
      config key).
      The command does not work against the current host state: `herdr worktree list` returns
      {"id":"cli:worktree:list","error":{"code":"server_not_running","message":"no herdr server
      is running at /home/astroboy/.config/herdr/herdr.sock; run `herdr` to start or attach it"}}
      exit 1. `herdr status` agrees: client 0.9.5, channel stable, protocol 22, server
      "not running", socket ~/.config/herdr/herdr.sock, update restart_needed no.
    * `herdr-core` 0.1.22 (invoked as `hc`) — NO worktree support, at two independent levels.
      CLI: `hc worktree --help` and `hc worktree list` both return
      {"error":{"hint":"see `hc --help`","kind":"usage","message":"unrecognized subcommand
      'worktree'"}} (exit 2; --help path exits 0 but prints the same error). `hc --help` has no
      worktree line at all — its sections are Terminals, Talk to programs, No terminal, More.
      Binary: `strings -a ~/.local/bin/herdr-core | grep -ci worktree` = 0, against 366 for the
      legacy binary. So this is not an unexposed flag; the code is absent.

  (b) Current workspaces on the live manager and their cwds.
    `hc server status`: core "home", pid 830950, version 0.1.22, live 7, uptime ~48.8 h,
    keeps_running_after_logout true.
    `hc ls` (7 live terminals): t3 zsh cwd /home/astroboy "~ / Hermes"; t17 copilot
    (copilot-main) /home/astroboy "~ / Copilot"; t19 antigravity (agy-main) /home/astroboy
    "~ / Hermes"; t1c zsh /home/astroboy "~ / Hermes 2"; t1d zsh /home/astroboy "~ / Hermes";
    t1f zsh /home/astroboy "~ / Hermes 2"; t1n antigravity /home/astroboy "~ / Antigravity".
    Read from the manager's own store (~/.local/state/herdr-core/default/terminals.json): every
    live terminal carries herdr.space sp337cc57ac6e8032e899a573e9ea40b37 — i.e. there is exactly
    ONE space (the core's equivalent of a workspace), and its tabs are the four labels above,
    "Hermes" (t3, t1d), "Copilot" (t17), "Hermes 2" (t1c, t1f), "Antigravity" (t1n). t19 is the
    one live terminal with no space label (created before spaces); it also sits at
    /home/astroboy. Every live terminal's cwd is /home/astroboy — no live terminal is rooted in a
    worktree or a sub-project checkout. Historical cwds among the 43 finished terminals include
    /home/astroboy/Hermes-agent-android-PC-companion-app and
    /home/astroboy/cabin-play/living-frame/firmware/n16r8-live, both plain directories.
    For completeness, the legacy client's own workspace list is a stale snapshot: with its server
    down, ~/.config/herdr/session.json (last written Nov 12 20:18) holds
    version 3, a workspaces list, active 0 — it describes nothing currently running.

  (c) worktrees.directory and the directory on disk.
    * The config that governs the live setup is ~/.config/herdr-core/config.toml. It contains NO
      worktrees section — only the restore_screen line added by task 0262 yesterday. That is the
      whole file. Grepping both config trees for "worktree" returns nothing.
    * There is therefore no `worktrees.directory` value in the live setup, and no key it could
      bind to: the live binary has zero worktree strings (above). The setting is not "unset with
      a default", it is unsupported.
    * For the binary that does implement it, the legacy client's config ~/.config/herdr/config.toml
      is just "onboarding = false" — no [worktrees] table — so its compiled default applies. The
      default is `~/.herdr/worktrees`, which appears as a literal in the 0.9.5 binary
      (`strings ... | grep -o '~/\.herdr/worktrees'`), matching Herdr's documented default.
    * ~/.herdr does not exist at all: `ls -lad ~/.herdr` -> "No such file or directory", and
      likewise ~/.herdr/worktrees. No worktree has ever been created through Herdr here.
    * The 17 worktrees that do exist on this host are hand-made, not Herdr's: `git worktree list`
      in nordtronics shows the main checkout plus /home/astroboy/nordtronics-0095 and
      /home/astroboy/wt-0041 … wt-0090 (16 linked worktrees, one per task branch). None is under
      ~/.herdr/worktrees. So worktree-backed *workspaces* in Herdr are unused; worktree use in
      this repo is by hand via `git worktree add`.

  Correction to the card's framing, for accuracy: the card and task 0193 assumed the legacy
  client is `herdr` 0.9.5 on PATH (skill/product note says ~/.local/bin/herdr). On this host
  today the 0.9.5 binary is named herdr-legacy-0.9.5 and plain `herdr` does not resolve; the
  live core is 0.1.22, not 0.1.21. Both were probed by absolute path, so the result does not
  depend on the rename.

  Nothing on the host was modified. Read-only commands used: --version/--help, worktree list,
  status, config check, hc ls, hc server status, cat/grep of config files, ls, strings,
  git worktree list.

cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: unknown
  cost_usd: unknown
  note: >-
    The harness for this tick does not expose per-call DeepSeek usage objects to the worker, so
    the token and cost fields are `unknown` rather than an estimate, per the cost-format rule.
    The run was off-peak (PEAK: OFF-PEAK 15:45 UTC from the pre-run script).
---

# 0263 — Herdr worktree commands: legacy client only, live manager has none

**Short answer.** `herdr worktree create|list|open|remove` exists only in the legacy `herdr`
0.9.5 client, which is installed as `~/.local/bin/herdr-legacy-0.9.5` and is not on PATH under
the name `herdr`; its server is not running, so even `herdr worktree list` can only answer
`server_not_running`. The live manager, `herdr-core` / `hc` 0.1.22, has no worktree commands and
no worktree code (0 binary strings vs 366 in the legacy client). So no worktree command is usable
on the live setup today.

## 1. Which binary answers (versions and exact results)

| Probe | `herdr` 0.9.5 (`~/.local/bin/herdr-legacy-0.9.5`) | `herdr-core` / `hc` 0.1.22 |
|---|---|---|
| `--version` | `herdr 0.9.5` | `hc 0.1.22` |
| `worktree --help` | exit 0 — "Manage Git worktree-backed workspaces", subcommands `list create open remove` | `{"error":{"hint":"see `hc --help`","kind":"usage","message":"unrecognized subcommand 'worktree'"}}` (exit 0 on the --help path, exit 2 on `list`) |
| `worktree list` | exit 1 — `{"id":"cli:worktree:list","error":{"code":"server_not_running","message":"no herdr server is running at /home/astroboy/.config/herdr/herdr.sock; run `herdr` to start or attach it"}}` | same "unrecognized subcommand 'worktree'", exit 2 |
| binary string probe | `grep -ci worktree` = 366; contains `WorktreesConfig{directory}`, `worktree.list/create/open/remove` API methods, default literal `~/.herdr/worktrees` | `grep -ci worktree` = **0** |
| server state | `herdr status`: client 0.9.5 / stable / protocol 22; **server not running** | `hc server status`: pid 830950, live 7, uptime ≈48.8 h, `keeps_running_after_logout: true` |

Plain `herdr` is not on PATH (`command not found`) — the 0.9.5 binary was renamed to
`herdr-legacy-0.9.5`. Probes used absolute paths, so nothing here depends on the rename.

## 2. Current workspaces and their cwds

The live manager has **one space** (`herdr.space` `sp337cc57ac6e8032e899a573e9ea40b37`), rooted at
`/home/astroboy`, with four tabs; all seven live terminals sit in `/home/astroboy`:

| Terminal | Agent | Workspace / tab label | cwd |
|---|---|---|---|
| t3 | zsh | ~ / Hermes | /home/astroboy |
| t17 | copilot (`copilot-main`) | ~ / Copilot | /home/astroboy |
| t19 | antigravity (`agy-main`) | (no space label; pre-spaces terminal) | /home/astroboy |
| t1c | zsh | ~ / Hermes 2 | /home/astroboy |
| t1d | zsh | ~ / Hermes | /home/astroboy |
| t1f | zsh | ~ / Hermes 2 | /home/astroboy |
| t1n | antigravity | ~ / Antigravity | /home/astroboy |

No live terminal is rooted at a worktree or a project sub-directory. Among the 43 finished
terminals the only other cwds are plain directories
(`~/Hermes-agent-android-PC-companion-app`, `~/cabin-play/living-frame/firmware/n16r8-live`).
The legacy client's `~/.config/herdr/session.json` (last written Nov 12 20:18, `active: 0`) is a
stale snapshot of a server that is not running, not a live workspace list.

## 3. `worktrees.directory`

- **Live setup** (`~/.config/herdr-core/config.toml`): no `worktrees` section — indeed no key is
  possible, since the binary implements none. The file holds only the `restore_screen = true`
  line from task 0262. Grepping both config trees for `worktree` matches nothing.
- **Legacy client** (`~/.config/herdr/config.toml`): `onboarding = false` only, so its compiled
  default applies — `~/.herdr/worktrees` (literal present in the 0.9.5 binary, matching the
  documented default).
- **On disk:** `~/.herdr` does not exist, so `~/.herdr/worktrees` does not exist either. Herdr
  has never created a worktree here.
- The 17 worktrees that *do* exist are hand-made: `git worktree list` in nordtronics shows the
  main checkout plus `~/nordtronics-0095` and `~/wt-0041 … ~/wt-0090` (16 linked worktrees, one
  per task branch). None lives under `~/.herdr/worktrees`.

## Constraint check

Report-only honoured: no config edited, no workspace created or closed, no checkout made. The
only write was the evidence transcript committed beside this reply. Commands used were read-only
(`--version`, `--help`, `worktree list`, `status`, `config check`, `hc ls`, `hc server status`,
`cat`/`grep`, `ls`, `strings`, `git worktree list`).
