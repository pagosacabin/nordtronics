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
