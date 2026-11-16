---
task_id: "0193"
protocol_version: 1.2.0
status: staged
iteration: 1
expect-reply-within: 24h
proof:
  host_local: true
  branch: "none - host-local harness configuration task; nothing was pushed to origin (the task forbids pushes, PRs and merges)"
  sha: "none - no branch was created; the nordtronics mailbox commits are 1d114cd (pickup, main) and the staging commit that carries this file"
  run: "none - no CI run is possible: no branch, no PR. Enumerated anyway: pagosacabin/nordtronics workflows gate on branch allow-lists (android-toolchain-setup, hermes/NNNN-* task branches) and path filters; a main-only mailbox commit runs the mailbox check only, and this task changed no code."
  files: []
  files_reason: "no repository file is part of the deliverable. The only repo change in this task is this mailbox file's own state transition (inbox -> active -> staged). All other changes are host-local: ~/.hermes/config.yaml (+1 line) and scratch state in ~/.hermes/profiles/cronrunner/cache/scratch/."
  artifacts:
    - "cabin-play worktree proof: .worktrees/0193-proof on hermes/0193-proof (created and removed in this run; outputs quoted below)"
    - "subagent archive ref: refs/hermes/archive/subagent-sa-0-d7aaa3cf = e8566cc (left in place per the task's constraint)"
    - "config change: /home/astroboy/.hermes/config.yaml delegation.worktree_isolation: true (46 comment lines before and after; baseline copy kept in scratch)"
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
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
  notes: "Harness exposes no per-call DeepSeek usage to this cron session, so token/cost fields are unknown (not estimated). The one delegated child and the two interactive CLI sessions it took to prove the flag report their own token counts; those were not exposed to this session either."
notes: |
  STAGED 2026-11-16T15:21Z (08:21 MST), off-peak (PEAK: OFF-PEAK 15:15 UTC), PROTOCOL MATCH 1.2.0.
  Host-local task: no branch, no push, no CI. Everything below was run on this host in this
  run; the raw outputs are quoted.

  Gate status: all six steps passed. Two things the task did not predict, both declared:
  (1) creating a worktree (either route) DIRTIES the shared checkout — the harness appends
  `.worktrees/` to cabin-play/.gitignore; reverted, so the after state matches the before state
  byte for byte. (2) `hermes config set delegation.worktree_isolation` warns "not a recognized
  config key" — the key is absent from the config schema but IS read at runtime
  (tools/delegate_tool_config.py:168), and a probe in the default profile returned True.

  Pickup note: the task file arrived with no front-matter, so the protocol fields were added at
  pickup (rule 16); the body is unchanged.

  Self-caught slip, declared rather than hidden: the first `git add` for the staging transition
  named both the new staged path and the old active path, so git aborted the whole add on the
  missing pathspec and only the bare `git mv` rename was staged (`git diff --cached --stat`
  showed `| 0`). Re-ran `git add` on the staged path alone; the committed diff shows the
  active-file deletion (84 lines) and the staged insertions (553 lines) together.
---


# 0193 — reply: per-task worktree proof, subagent isolation enabled, worker/agent routing analysis

## Gate failures

None. Every step ran to completion on this host in this run.

Two findings the task did not predict, both declared and both handled:

1. **Creating a worktree dirties the shared checkout.** Both routes (`/worktree new` and the
   delegated child) append `.worktrees/` to `cabin-play/.gitignore` in the *shared* checkout —
   an uncommitted one-line edit that persists after the tree is gone. Reverted with
   `git checkout -- .gitignore`, so the after state is byte-identical to the before state. It is
   worth deciding once whether cabin-play (and nordtronics) should commit that line, because
   otherwise every worktree creation shows up as a stray edit on `main`.
2. **`hermes config set delegation.worktree_isolation` warns "not a recognized config key — it
   was saved anyway, but Hermes may not read it."** It is saved, and it *is* read:
   `tools/delegate_tool_config.py:168` `return bool(_cfg().get("worktree_isolation", False))`.
   A probe in the default profile printed `worktree_isolation = True`, and the live delegated
   run below proves the read path end to end. The warning is a schema-coverage gap, not a defect
   in the setting.

Scope note: the task says "do not change any cron job" — none was touched. `hermes cron list`
was read only; no `cron edit`, no store write.

## Commands run (verbatim, in order)

```
# --- step 1: inventory ---
hermes --version
hermes --help | grep -E 'worktree|-w\b'
hermes worktree --help
grep -rn '"worktree"' --include=*.py . | grep -iE 'slash|command|name='
grep -n -A20 '^delegation:'   ~/.hermes/config.yaml
grep -n -A15 '^checkpoints:'  ~/.hermes/config.yaml
hermes -p cronrunner cron list
python3 -c "... cron.jobs get_job('c0be50a686c6') ..."        # job dict read from jobs.json
cd ~/cabin-play && git status --short && git branch --show-current && git rev-parse HEAD && git worktree list

# --- step 2: live named-worktree proof (fresh interactive CLI session, cabin-play) ---
cd ~/cabin-play && env -u HERMES_HOME TERM=xterm-256color hermes      # fresh session, default profile
/worktree new 0193-proof
git worktree list
git -C .worktrees/0193-proof rev-parse --abbrev-ref HEAD
printf '0193 worktree proof\n' > .worktrees/0193-proof/WORKTREE-PROOF-0193.txt
git -C .worktrees/0193-proof status --short
git -C /home/astroboy/cabin-play status --short
git -C /home/astroboy/cabin-play rev-parse HEAD
ls -l /home/astroboy/cabin-play/WORKTREE-PROOF-0193.txt      # ENOENT by design
rm .worktrees/0193-proof/WORKTREE-PROOF-0193.txt
/exit                                                        # atexit cleanup removes the clean tree
git worktree list && git branch --list 'hermes/0193*' && git status --short && git rev-parse HEAD
git checkout -- .gitignore && rmdir .worktrees

# --- step 3: subagent isolation ---
cp ~/.hermes/config.yaml <scratch>/config.yaml.baseline-0193
env -u HERMES_HOME hermes config set delegation.worktree_isolation true
grep -cE '^[[:space:]]*#' ~/.hermes/config.yaml ; diff -u <baseline> ~/.hermes/config.yaml
env -u HERMES_HOME python3 -c "import sys;sys.path.insert(0,'.');from tools.delegate_tool_config import _get_worktree_isolation;print(_get_worktree_isolation())"
cd ~/cabin-play && env -u HERMES_HOME TERM=xterm-256color hermes   # fresh session, then one delegate_task turn
git for-each-ref refs/hermes/ --format='%(refname) %(objectname:short)'
git log --oneline -1 e8566cc && git show e8566cc:DELEGATE-PROOF-0193.txt
git worktree list && git status --short

# --- steps 4-6: report-only, read-only ---
git -C ~/nordtronics worktree list ; hermes worktree list
hermes worktree --help ; herdr --version ; herdr worktree --help ; herdr worktree list ; herdr status
hc --version ; hc server status ; hc ls ; hc templates ; hc run --help
cat ~/.config/herdr/config.toml ; ls ~/.herdr ; find ~/.config/herdr -name settings.json
strings ~/.local/bin/herdr | grep -iE '\.herdr/worktrees|worktrees\.directory|settings\.json'
```

## Before

**Installed build** — `Hermes Agent v0.23.1 (2026.9.24) · stable · local 3b4215785882`, Python 3.14.9.

`hermes --help` (excerpt):

```
              [--worktree] [--accept-hooks] [--skills SKILLS] [--yolo]
    worktree            Audit and reclaim accumulated git worktrees and merged
  --worktree, -w        Run in an isolated git worktree (for parallel agents)
    hermes -w                     Start in isolated git worktree
```

`hermes worktree --help` (excerpt) — so the installed build has both the flag and the reclaimer:

```
usage: hermes worktree [-h] {list,ls,audit,prune,restore} ...
Audit, reclaim and restore the .worktrees/ trees Hermes sessions accumulate.
Idle trees with uncommitted changes or unpushed commits are archived to
refs/hermes/archive/<tree> before removal (worktrees.archive_dirty); `restore`
brings one back. In-use trees are never touched.
```

`/worktree` as a session command is real in this build, not just in the docs:
`hermes_cli/commands.py:112` `CommandDef("worktree", "Show, list, create, or prune isolated git worktrees", "Session", ...)`,
handler `cli_commands_mixin.py:981` (`/worktree [new [name]|list|prune [--dry-run]]`), which calls
`cli._setup_worktree(repo_root=..., sync_base=..., name=...)` and registers the same keep-if-unpushed
atexit cleanup as `hermes -w`.

**Config state** (`~/.hermes/config.yaml`, default profile) — before:

```yaml
delegation:
  max_iterations: 250
```

`checkpoints:` — **absent entirely** from the file. The effective state is off, from
`hermes_cli/cli_init_mixin.py:329-333`: `self.checkpoints_enabled = checkpoints or cp_cfg.get("enabled", False)`.
Left untouched, as the constraint requires.

**Mailbox worker launch mechanism.** Profile `cronrunner`, job `c0be50a686c6` ("Mailbox worker
(DeepSeek) Mon-Thu"):

```
Schedule:  15 4-17,21,22 * * 1-4        Workdir: /home/astroboy/nordtronics
Script:    mailbox-changed.sh           Skills: github, kicad-mcp-server, handoff-mailbox
model: deepseek-flash / provider: deepseek
enabled_toolsets: [file, terminal, skills, code_execution, web]
```

There is **no `command` field** on the job (`job["command"] is None`): each fire is one
prompt-driven agent session whose cwd is the shared checkout at `/home/astroboy/nordtronics`.
It does **not** branch-switch that checkout for the transition — every state move is `git mv` +
commit + push **to main** — but task *work* has historically been done in the same checkout or in
hand-made trees (see step 4).

**cabin-play before:**

```
$ git status --short          (empty)
$ git branch --show-current   main
$ git rev-parse HEAD          a7d0dabaef19e8c7557995a4627bb579c7dba4fa
$ git worktree list
/home/astroboy/cabin-play a7d0dab [main]
```

## Worktree proof

Created from a fresh interactive CLI session in cabin-play (own process, default profile, not
this cron session):

```
❯ /worktree new 0193-proof
  ✅ Worktree ready: /home/astroboy/cabin-play/.worktrees/0193-proof
  Branch: hermes/0193-proof
  Terminal and file tools now operate in the worktree.
```

During the proof, from the host shell:

```
$ git worktree list
/home/astroboy/cabin-play                       a7d0dab [main]
/home/astroboy/cabin-play/.worktrees/0193-proof a7d0dab [hermes/0193-proof] locked

$ git -C .worktrees/0193-proof rev-parse --abbrev-ref HEAD
hermes/0193-proof
$ git -C .worktrees/0193-proof rev-parse HEAD
a7d0dabaef19e8c7557995a4627bb579c7dba4fa          # based on the fetched tip, as documented

$ printf '0193 worktree proof\n' > .worktrees/0193-proof/WORKTREE-PROOF-0193.txt
$ git -C .worktrees/0193-proof status --short
?? WORKTREE-PROOF-0193.txt

$ git -C /home/astroboy/cabin-play status --short
 M .gitignore                                      # ".worktrees/" appended by the harness
$ git -C /home/astroboy/cabin-play branch --show-current
main
$ ls -l /home/astroboy/cabin-play/WORKTREE-PROOF-0193.txt
ls: cannot access '.../WORKTREE-PROOF-0193.txt': No such file or directory
```

The scratch file existed only inside the worktree; the shared checkout never saw it. The
`.gitignore` edit is the harness's own bookkeeping, not the scratch file — see the gate-failure
note; it was reverted.

## Cleanup

Scratch file removed, then `/exit` — the documented keep-if-unpushed cleanup fired:

```
✓ Worktree cleaned up: /home/astroboy/cabin-play/.worktrees/0193-proof
```

After, in the shared checkout:

```
$ git worktree list
/home/astroboy/cabin-play a7d0dab [main]
$ git branch --list 'hermes/0193*'      (empty — the branch was removed with the tree)
$ git status --short                    (empty)
$ git branch --show-current             main
$ git rev-parse HEAD                    a7d0dabaef19e8c7557995a4627bb579c7dba4fa
```

Identical to the before state: same branch, same HEAD, clean status, no leftover worktree.
(`.gitignore` was reverted and the now-empty `.worktrees/` directory removed, so the match is
exact rather than approximate.)

## Subagent isolation

Flag set: **yes** — `~/.hermes/config.yaml` `delegation.worktree_isolation: true`.

```
$ env -u HERMES_HOME hermes config set delegation.worktree_isolation true
✓ Set delegation.worktree_isolation = True in /home/astroboy/.hermes/config.yaml
⚠ 'delegation.worktree_isolation' is not a recognized config key — it was saved anyway, but
  Hermes may not read it.

$ diff -u <baseline> ~/.hermes/config.yaml
@@ -102,6 +102,7 @@
   nudge_interval: 10
 delegation:
   max_iterations: 250
+  worktree_isolation: true
 skills:
   creation_nudge_interval: 15

$ grep -cE '^[[:space:]]*#' ~/.hermes/config.yaml
46            # 46 before the write as well — comments preserved
$ env -u HERMES_HOME python3 -c "... _get_worktree_isolation() ..."
worktree_isolation = True
```

Proof run: one `delegate_task` turn in a fresh cabin-play session, child told to `pwd` and write
`DELEGATE-PROOF-0193.txt`. Child result, quoted from the parent session:

```
- pwd: /home/astroboy/cabin-play/.worktrees/subagent-sa-0-d7aaa3cf
- Succeeded: yes — status=completed, exit 0, 3 API calls, 7.03s.
  It created DELEGATE-PROOF-0193.txt (60 bytes) in that worktree.
- Verified just now: .worktrees/subagent-sa-0-d7aaa3cf no longer exists ... Nothing was written
  into /home/astroboy/cabin-play.
```

The parent reported no archive line from the tool, so I checked the git side myself rather than
take the child's prediction:

```
$ git for-each-ref refs/hermes/ --format='%(refname) %(objectname:short)'
refs/hermes/archive/subagent-sa-0-d7aaa3cf e8566cc
$ git log --oneline -1 e8566cc
e8566cc hermes worktree archive: subagent-sa-0-d7aaa3cf
$ git show e8566cc:DELEGATE-PROOF-0193.txt
/home/astroboy/cabin-play/.worktrees/subagent-sa-0-d7aaa3cf
```

So the child's tree was **dirty, therefore archived** (not discarded): the archive commit is the
full working state, and the file inside it names its own worktree path — the isolation is proved
by the artifact, not by the tool's prose. Flag left **on**; archive ref left in place, as the
task allows.

Caveat worth knowing before relying on it: the archive commit also carries the child's working
copy of the whole repo, and `.gitignore` again picked up `.worktrees/` in the shared checkout
(reverted). The `refs/hermes/archive/*` refs are only reclaimed by `hermes worktree prune`, which
reports them as reclaimable; nothing sweeps them on a timer.

## Mailbox worker

**Could it launch each task session in its own worktree instead of branch-switching the shared
checkout? Yes mechanically, but it already half-does, and making it official is a small, specific
change — not a rewrite.**

What exists today is evidence, not a proposal. `nordtronics` has 16 hand-made per-task trees
outside the repo, and the harness explicitly refuses to reclaim them:

```
$ hermes worktree list            # in ~/nordtronics
16 externally-registered worktree(s) (never touched by prune):
  /home/astroboy/nordtronics-0095  [hermes/0095-alert-pipeline]  ok
  /home/astroboy/wt-0041           [hermes/0041-config-comment-guard]  ok
  ... wt-0043/45/46/47/48/49/50/56/60/67/77/78/85/90 ...
21 local branch(es) fully merged/patch-equivalent upstream would also be deleted.

$ git -C ~/nordtronics worktree list | wc -l
17                                # 1 shared checkout + 16 trees, oldest 2026-10-01, newest 2026-10-02
$ git -C ~/nordtronics status --short
?? firmware/wildfire-node-v1/bench-override.ini
?? hardware/solar-gate-v2/
?? take_screenshots.py
```

So the worker's real isolation gap is not "it edits the shared checkout" — it is that its
per-task trees live **outside** the harness's `.worktrees/` convention, so `hermes worktree
prune` will never touch them, and the untracked files above are the debris that leaks into the
shared checkout anyway.

The exact change, in the job's prompt (job `c0be50a686c6`, Step 2), replacing the implicit
"branch in the shared checkout" with:

```
git -C /home/astroboy/nordtronics worktree add .worktrees/NNNN -b hermes/NNNN-<slug> origin/main
# then: terminal(workdir="/home/astroboy/nordtronics/.worktrees/NNNN", ...) for all task work
# transition (git mv + commit + push) stays in the SHARED checkout on main, always
```

i.e. one `git worktree add` under `.worktrees/`, a per-task `workdir` for the work steps, and the
existing main-checkout transition unchanged. That single change makes the trees visible to
`hermes worktree list/prune`, keeps the state machine's `git mv` on main in one place, and removes
the `~/wt-NNNN` sprawl.

Caveats that make it a *bad* idea to go further and run the whole worker as `hermes -w` or
`/worktree new` per tick:

- Cron/one-shot sessions never get the keep/archive prompt — their trees are simply left for the
  stale sweep (idle `hermes-*`/`subagent-*` trees 24h, named trees 72h). So a worker that used
  `-w` would silently accumulate a tree per tick in the worst case (a tick that dies before
  cleanup), which is the `~/wt-NNNN` problem again with a different directory name.
- The worktree is checked out with repo hooks, `core.fsmonitor` and clean/smudge filters off, and
  LFS files land as pointers — a CI-equivalence risk for firmware tasks that need the real bytes.
- The mailbox transition must happen on main in a checkout that is not a task branch; a
  worktree-based worker still needs an authoritative main checkout, so it removes one collision
  surface and adds one moving part.

**Parallel pickup (Stephen's question).** Today it is one-at-a-time **by design, stated three
times in the prompt verbatim**:

```
## Step 2 — choose the task: LOWEST task number first
1. If `mailbox/active/` holds any task with `status: in_progress`, RESUME that
   task — it outranks everything in `mailbox/inbox/`. ...
2. Else pick the lowest-numbered file in `mailbox/inbox/` with `status: inbox`.
   Pickup: `git mv mailbox/inbox/NNNN-*.md mailbox/active/NNNN-*.md`, set
   `status: in_progress`, `iteration: iteration + 1`, commit + push. ...
3. If both `active/` and `inbox/` are empty of actionable tasks, stop and report:
   nothing to do.

Work ONE task per run. Iteration cap 5: ...
```

The scheduler reinforces it: one fire at a time per job (`cron/scheduler.py` keys in-flight work
by `_inflight_key(job_id, home)` and refuses a second concurrent fire of the same job). So
parallel pickup is not "one worker that could do more" — it is a single session per tick with a
single selector.

Parallel pickup would take: N jobs (or N external workers) sharing the same mailbox, plus a claim
protocol the mailbox does not have. What breaks first, in order:

1. **The mailbox itself.** Every transition is `git mv` + commit + push on `main` in one checkout.
   Two workers picking up at the same second push non-fast-forward commits; the loser either
   force-pushes (forbidden) or rebases its own `git mv` onto the other's commit and can resurrect
   or double-move a file — exactly the class of failure rule 0 documents (wrong-file commits,
   archived files restored). There is no claim file, no lock, and no `status: claimed_by`; the
   only serialisation is "one session per tick".
2. **The shared checkout**, if both workers also do the work there — solved by the per-task
   worktree above, which is why that answer bears on this one.
3. **Model/API contention** — last, and real but minor: DeepSeek bills per token and tolerates
   concurrency; two sessions at once would double the per-tick cost but not corrupt anything.
   The 15-minute cadence means throughput, not parallelism, is the current limit.

A yes/no without a mechanism would be a lie here, so: **no parallel pickup today; yes it is
possible; and the mailbox transition layer — not the agent layer — is what must be redesigned
first.**

## Newton

`delegation.worktree_isolation` **does not affect Newton at all.** It is read in
`tools/delegate_tool_config.py:168` and consumed only in `tools/delegate_tool_child_run.py:453-460`
(`_create_isolated_worktree`), which is the code path that builds a **Hermes child agent** from
`delegate_task`; it is also gated to the local terminal backend
(`subagent_worktree.local_backend_active()`). Newton is an `antigravity` CLI process living in a
Herdr pane (`hc ls` → `t19`, name `agy-main`, agent `antigravity`, cwd `/home/astroboy`, currently
`AGY-OK 1.5.8` on Gemini 3.8 Flash) — a separate program Hermes never spawns as a child, so no
delegation config reaches him. I know this from the call path, not from documentation: the flag is
read in exactly two files, and neither is on any Herdr/pane path.

What would give Newton worktree separation is therefore outside Hermes: start his pane **in a
worktree checkout** — either `hc run --cwd <path> antigravity` on an existing tree, or, if the
Herdr worktree feature ever runs here, Herdr's own `herdr worktree create`. With `copilot-main`
(`t17`) and `agy-main` (`t19`) both sitting in `/home/astroboy`, neither currently has any repo
checkout, which is the cheapest fix available today and needs no feature.

## Herdr worktrees

**Which product carries it.** The **`herdr` 0.9.5 client** (protocol 22) — *not* the live manager.
`herdr worktree --help` lists `list | create | open | remove`, and `hc 0.1.21` has no worktree verb
at all (`hc --help` covers terminals, `run`, `exec`, `file`, `machine`, `connect`, `schema` only;
`hc templates` returns `[]`). And the client's server is not even running:

```
$ herdr status
client:  version: 0.9.5   channel: stable   protocol: 22
server:  status: not running   socket: /home/astroboy/.config/herdr/herdr.sock
$ herdr worktree list
{"id":"cli:worktree:list","error":{"code":"server_not_running", ...}}

$ hc server status
{"core":{"id":"b5f627c1098bc294d1c08d5017248613","name":"home"},"live":6,"pid":5079,
 "uptime_ms":224222456,"version":"0.1.21"}
```

So on this machine the worktree feature belongs to a product whose server is stopped, while the
manager that actually holds Stephen's panes (`herdr-core` 0.1.21, 6 live terminals, ~2.6 days
uptime) is the one without it. **The screenshot's feature is not reachable through `hc`.**

**Settings file and directory.** The installed client's settings file is
`~/.config/herdr/config.toml` (19 bytes: `onboarding = false`); `herdr config check` → `config: ok`.
There is **no** `settings.json` anywhere under `~/.config/herdr` or `~/.local/state`, and **no**
`~/.herdr` directory. The binary carries the default and a commented template line:
`~/.herdr/worktrees` and `# directory = "~/.herdr/worktrees"`. So: nothing is configured on this
machine, and the documented default `~/.herdr/worktrees` would apply — but it does not exist yet
because no Herdr worktree was ever created here. I could not find a `worktrees.directory` key in
any file on disk; the name in the screenshot corresponds to nothing present, and the on-disk
config format here is TOML, not `settings.json` — a discrepancy to resolve against a newer build
rather than assume.

**How a pane opens in a Herdr worktree.** With `hc`, there is no worktree verb, so the only route
today is a cwd: `hc run --cwd <worktree-path> antigravity` (or `hc exec --cwd <path> ...`,
`--cwd` documented on both). With the `herdr` client, `herdr worktree create|open` would open the
checkout as a workspace — but only against its own server, which is down.

**Collide or compose?** They compose, cleanly, by construction — and the task's own proof shows
why: the harness puts checkouts at `<repo>/.worktrees/<name>` on branch `hermes/<name>`, Herdr's
default is `~/.herdr/worktrees` with an arbitrary `--branch`/`--path`. Different directories, so
two agents cannot scribble in one tree. The one real collision point is git's own rule that a
branch can be checked out in only one worktree at a time: if Herdr is told to use
`hermes/<name>` while the harness already has that branch checked out, `git worktree add` fails
rather than corrupting anything. Keep the naming disjoint (harness `hermes/NNNN-*`, Herdr
`herdr/<slug>` or its own default) and the two systems coexist with no shared state — the harness
sweeps `.worktrees/` only, and `hermes worktree prune` will never touch `~/.herdr/worktrees`
(exactly as it refuses the 16 `~/wt-NNNN` trees above).

## Cost

Model `deepseek-flash`, provider direct, `rate_window: off-peak` (PEAK: OFF-PEAK 15:15 UTC).
Token and cost fields are `unknown`: this cron harness exposes no per-call usage object to the
session, and the two interactive CLI sessions plus the one delegated child report their own
counts in their own sessions. Not estimated — see the `cost:` block.

---

## Original task file (unchanged body, for the record)

# 0193 — Hermes git-worktree isolation: prove one per-task worktree, set subagent isolation if it fits

## Assignment
- **Hermes** (host-local CLI work on your own machine): all steps.
- **Newton:** not used — no code build. One question in this task is how worktree isolation applies to agents you route; answer it from your own setup's evidence, do not route this task to Newton.
- **Copilot:** not used — no GitHub deliverable; this is local harness configuration and proof. Nothing is pushed to origin in this task.

## Context
Juno's daily learning for 2026-11-16, verified against the official Git Worktrees docs page (https://hermes-agent.nousresearch.com/docs/user-guide/git-worktrees), found built-in worktree isolation in your harness:

- `/worktree new <name>` from an interactive session creates `.worktrees/<name>/` on branch `hermes/<name>` (based on the freshly-fetched remote tip) and retargets that session's terminal and file tools into it, no restart.
- `hermes -w` does the same automatically with a disposable tree and branch (`hermes/hermes-<hash>`).
- Each worktree carries its own checkpoint history, so `/rollback` in one tree never touches another (pairs with the 2026-11-15 checkpoints finding).
- Subagent isolation is a config flag, `delegation.worktree_isolation: true`: a finished child leaves a clean tree removed, or a dirty tree archived (full working state as a commit at `refs/hermes/archive/<tree>`, restorable with `hermes worktree restore`) and removed.
- Caveats from the docs: worktrees are checked out with repo hooks, `core.fsmonitor`, and clean/smudge filters off; Git LFS files land as pointers until `git lfs pull`; one-shot runs (`-q`/`-z`), cron jobs, and gateway sessions never get the keep/archive prompt — their trees are kept for the stale-worktree sweep (idle `hermes-*`/`subagent-*` trees 24h, named trees 72h, never a tree a live process sits in).

Why it matters here: you and Newton both work in the same repos (nordtronics, cabin-play), and your mailbox worker switches branches in a single shared checkout — two agents' edits can collide in one working tree, and a wedged task leaves its branch state tangled with the next.

Stephen approved this task on 2026-11-16: enable what fits, and prove it on one task. This task is that one task.

**There are two worktree systems in play — keep them separate in your reply.** The docs above describe the **Hermes Agent harness's** worktrees (`.worktrees/` inside the repo). Separately, **Herdr** has its own worktree feature: Stephen photographed Herdr's what's-new panel on 2026-11-16 — "New worktree can put checkouts in a folder you choose: `worktrees.directory` in settings.json. The default stays `~/.herdr/worktrees`." Herdr is also two products (the `herdr` 0.9.5 client vs the live **herdr-core 0.1.21**, driven via `hc` — proven in 0189/0190). Which product carries the worktree feature and where its settings live on your machine is yours to establish from `hc` help / the actual settings file, not from the screenshot.

## Task
1. **Inventory the current state.** Capture raw output:
   - Your installed version, and the help text proving `-w` / `/worktree` / `hermes worktree` exist in the installed build (excerpts are fine, enough to show the flags/commands are real).
   - The relevant config sections from `~/.hermes/config.yaml`: the current `delegation:` block and the current `checkpoints:` state. Redact anything secret — only these two sections are wanted.
   - How your mailbox worker currently launches task sessions: the actual cron job command(s) / profile from `hermes cron list`, and a plain statement of whether it branch-switches a shared checkout today.
   - In your local **cabin-play** clone: `git status`, current branch, HEAD SHA, and `git worktree list` — this is the before state.
2. **Live per-task worktree proof (cabin-play).** From a fresh, separate interactive CLI session (not the session running this mailbox task, and not a one-shot/cron/gateway session):
   - Create a named worktree for this task using the documented route (`/worktree new 0193-proof` or the installed build's equivalent).
   - Inside the worktree only, create one scratch file, `WORKTREE-PROOF-0193.txt`, containing one line: `0193 worktree proof`.
   - Capture evidence: the worktree path and its branch (`hermes/0193-proof` or the build's equivalent), `git status` inside the worktree showing the scratch file, and `git status` in the shared/main checkout showing it does **not** contain the scratch file and is otherwise unchanged.
   - Clean up with the documented route: remove the scratch change and the worktree. Capture `git worktree list`, `git status`, branch, and HEAD SHA in the shared checkout afterward.
3. **Subagent isolation.** Set `delegation.worktree_isolation: true` in `~/.hermes/config.yaml`. Prove it with one short delegated subagent run in cabin-play (a trivial read-only or scratch-file job of your choosing — keep it small): capture what happened to the child's tree afterward (clean tree removed, or dirty tree archived — cite the `refs/hermes/archive/...` ref if one was created). If the proof passes, leave the flag **on**. If it fails, revert the flag, and report the exact error.
4. **Mailbox worker questions — report only, change nothing.** Based on the docs caveats and your worker's actual launch mechanism from step 1, answer both plainly:
   - **Per-task worktrees:** could the mailbox worker launch each task session in its own worktree (`hermes -w`, or `/worktree new <task-number>` per task) instead of branch-switching the shared checkout? Name the exact change that would do it (file/job/command), and any caveat that makes it a bad idea (for example the cron/one-shot keep-prompt caveat).
   - **Parallel pickup (Stephen's question, 2026-11-16):** could the worker pick up and run two or more mailbox tasks at the same time, or is it one-at-a-time by construction? Quote the worker's actual task selector (the prompt/command lines that choose the next inbox task) and state whether one-per-cycle is by design or by accident. Name the exact change parallel pickup would take, and what breaks first — evaluate at least: the shared-checkout collision (your per-task-worktree answer above bears on this), the mailbox itself (task files are state transitions in the nordtronics repo — two workers doing `git mv` + commit + push on the same checkout/repo at once), and model/API contention. A yes/no with no mechanism does not pass.
   Do **not** change the worker's cron jobs, commands, or launch mechanism in this task — Stephen decides any switch from your report.
5. **Newton routing question — one short paragraph.** You route Newton through Herdr panes, not through your own delegation system. State whether `delegation.worktree_isolation` affects Newton at all, and if Newton needs worktree separation, what would actually give it to him (for example Newton working in a worktree checkout inside his pane). Evidence over assumption: say how you know.
6. **Herdr worktrees — report only, change nothing.** Establish from your machine's evidence: which Herdr product carries the worktree feature, where its settings file is, the current `worktrees.directory` value (or that the default `~/.herdr/worktrees` applies), and how a pane is opened in a Herdr worktree checkout via `hc`. Then answer plainly: is the Herdr route the practical way to give the pane agents (Newton, Copilot) isolated checkouts, and do the two systems collide or compose (different checkout directories, different branch naming — state what you find)? Do **not** change any Herdr setting in this task.

## Success criteria (falsifiable)
- Before/after evidence in the reply shows: a named worktree existed on its own branch, the scratch file existed **only** inside it, and after cleanup the shared cabin-play checkout is on its original branch at its original HEAD SHA with a clean status and no leftover worktree.
- `delegation.worktree_isolation` is either ON with a cited subagent proof (tree removed, or archive ref named), or reverted with the exact failure error quoted.
- The mailbox-worker questions are answered with the exact changes named — including the worker's task selector quoted verbatim for the parallel-pickup question. A yes/no with no mechanism does not pass.
- The Newton paragraph states plainly whether the delegation flag touches Newton, and what would.
- The Herdr section names the product, settings file, and current/default worktree directory from machine evidence (not the screenshot), and states whether the two worktree systems collide or compose.

## Constraints
- **Do NOT enable checkpoints** in this task. `checkpoints.enabled` is a separate open decision (2026-11-15 finding); record its current state in step 1, change nothing about it.
- **No pushes to origin, no PRs, no merges.** Local branches/worktrees only, all cleaned up by the end. A local `refs/hermes/archive/...` ref created by the subagent-isolation machinery itself is expected behaviour, not a violation — report it, leave it.
- Do not change any cron job: no schedules, commands, models, enable/disable state, or the mailbox worker's launch mechanism.
- Do not disrupt a live session: if worktree creation would retarget a session doing other work, use a fresh separate CLI session for the proof (step 2 already requires one).
- If the installed build's commands differ from the docs page, follow the installed build's help and say so in the reply.
- If any step fails, stop at that gate and report the exact error — the error is data; do not improvise around it, and revert any config change that the failed step depended on.
- **Cost:** one short run on the standard tier (DeepSeek Flash). This is CLI output capture, one small proof, and applying one config flag — no model escalation, no Newton session.

## Proof
Host-local task: no branch on origin, no CI run — `branch: none` / `run: none` is the honest state. The staged reply carries the raw terminal outputs (before/during/after) as the record; Juno verifies them against the docs behaviour quoted in Context.

## Reply format
One staged reply with these sections:
- Gate failures, if any, stated plainly at the top
- Commands run (verbatim, in order)
- Before: version + worktree help excerpts, `delegation:` / `checkpoints:` config state, mailbox worker launch mechanism, cabin-play before state (status / branch / HEAD / worktree list)
- Worktree proof: worktree path + branch, git status inside the worktree, git status in the shared checkout during the proof
- Cleanup: worktree list / status / branch / HEAD after, matching the before state
- Subagent isolation: flag set (yes/no), the proof run, and the child's tree outcome (removed, or the archive ref)
- Mailbox worker: could it launch per-task worktrees — yes/no, the exact change, and the caveats; and parallel pickup — the selector quoted, one-per-cycle by design or accident, the exact change, and what breaks first
- Newton: whether the delegation flag affects him, and what would give him worktree separation
- Herdr worktrees: product, settings file, current/default `worktrees.directory`, how a pane opens in one, and collide-vs-compose with the harness's worktrees
- Cost block
