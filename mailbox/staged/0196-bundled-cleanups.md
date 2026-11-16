---
task_id: "0196"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 24h
notes: |
  Picked up 2026-11-16T17:15Z (10:15 MST), off-peak (PEAK: OFF-PEAK 17:15 UTC),
  PROTOCOL MATCH 1.2.0. 0142/0159/0187 in active/ were all decision-blocked or
  superseded and were left untouched; 0176 (lowest inbox number) carries
  `status: superseded` and "Do not pick up", so 0196 is the lowest actionable task.
---

# 0196 — Two cleanups in one pass: `.worktrees/` into .gitignore (both repos) + retire the dead herdr 0.9.5 client

## Assignment
- **Hermes** (host-local work on your own machine): all steps, both parts.
- **Newton:** not used — no code.
- **Copilot:** not used — Part A is a two-line ignore-file chore committed directly to main in both repos, verified by Juno from origin; Part B is host-local cleanup with no GitHub deliverable.
- **Juno:** verifies Part A by reading both `.gitignore` files back from origin, verifies Part B against the before/after `hc` state, archives on green.

## Context — why this is one task
This task **replaces** the separately filed 0196 (.gitignore) and 0197 (herdr cleanup); Juno withdrew both from the inbox and bundled them at Stephen's direction (2026-11-16 09:09) to save a worker cycle — the queue is serial, one task per hourly run, and both parts are small host-local chores surfaced by 0193. The parts are independent: do Part A, then Part B, and report each on its own merits. A gate failure in one part does not block the other — complete the part that can complete, and put any failure at the top of the reply.

---

## Part A — Commit `.worktrees/` to .gitignore in nordtronics and cabin-play

### Context
Finding from 0193 (verified and archived 2026-11-16): every time the Hermes harness creates a worktree — the `/worktree new` route and delegated subagent trees both do it — it appends `.worktrees/` to the **shared checkout's** `.gitignore` as an uncommitted edit. The edit persists after the tree is removed, so every worktree creation leaves the shared checkout dirty on `main` until someone reverts the line by hand. Hermes reverted it twice during 0193.

Stephen's call, 2026-11-16: commit the line once, in both repos, so worktree creation stops dirtying the shared checkouts.

### Task
A1. In each repo — **pagosacabin/nordtronics** and **pagosacabin/cabin-play**, local clones on your machine:
   - Read the root `.gitignore`. If it already contains a `.worktrees/` entry, change nothing in that repo and say so.
   - Otherwise append exactly one entry, `.worktrees/`, at the end of the file (create the file with just that entry if the repo has none). Change nothing else in the file.
   - Commit **only** the `.gitignore` change to `main` and push. Commit message: `gitignore: ignore Hermes worktree checkouts (.worktrees/)`.
A2. **Prove the fix** in cabin-play: from a fresh interactive CLI session, create a named worktree once (the 0193 route), then show `git status --short` in the shared checkout — it must be clean, with **no** `.gitignore` modification. Clean up the worktree the documented way and show the shared checkout still clean, on main, at the new HEAD.

### Success criteria (falsifiable)
- Both repos' root `.gitignore` on origin `main` contain `.worktrees/` exactly once, and each repo's commit for it touches only `.gitignore` (state the SHA and the one-line diffstat for each).
- The cabin-play proof shows: worktree created → shared checkout `git status` clean (no `.gitignore` line) → worktree removed → still clean.
- If either repo already had the entry, that repo has no new commit and the reply says so plainly.

### Constraints
- Only `.gitignore` changes. No other file in either commit — if a checkout is dirty with unrelated changes, stop Part A and report rather than committing around them.
- Do **not** touch the 16 external `~/wt-NNNN` / `~/nordtronics-NNNN` trees or their branches — separate decision, not this task.
- Pushes over SSH as usual (the gh.py credential cannot write cabin-play). Do NOT source `gh-env.sh` for anything in this task.

---

## Part B — Inventory the dead herdr 0.9.5 client and retire it if nothing depends on it

### Context
From 0193's inventory (verified on this machine, 2026-11-16): the live pane manager is **herdr-core 0.1.21** (`hc`, pid 5079, up since 2026-11-13 08:02 MST, 6 live terminals). The older **`herdr` 0.9.5** client's server has been down since its log ended 2026-11-13 03:18 — the morning herdr-core took over. The client binary still sits on the machine (0193 read strings from `~/.local/bin/herdr`) and still answers questions: `herdr status` reports "server: not running" and `herdr worktree list` errors `server_not_running`. Those answers are about the dead product, but they read as answers about the live one — that name collision already caused one real failure (the 2026-11-15 08:15 cron run concluded Herdr was down and routed around a healthy system). Since 0189/0190, all routing goes through `hc`; the handoff-mailbox skill records "drive via `hc`, never `herdr`."

Stephen's question, 2026-11-16: it's a cleanup issue — why is a dead binary still sitting there? Is there a reason? Nobody chose to keep it; it is migration residue from the Nov 13 switch. And his intent, stated plainly 2026-11-16 09:05: he wants to **prevent the excuse itself** — an unattended cron run reporting "I can't talk to Herdr" while he is out of reach or out of connection, and work stalling or routing around a healthy system on a false premise. So this part is authorized to finish the job in one pass: references in **your own** tooling get fixed here (step B2), not parked for a report round-trip Stephen may not be around to answer. The stop-gate remains only for things that are his, or genuinely ambiguous.

### Task
B1. **Inventory (read-only).** Establish and quote:
   - How the 0.9.5 client is installed: binary path, file type (standalone binary / symlink / script), owning package if any (rpm/dnf, pip, cargo, manual install), and whether `hc` / herdr-core comes from the same install or a separate one.
   - Everything that invokes the `herdr` binary by name: your cron jobs and worker prompts, your skills (including handoff-mailbox), shell rc files and aliases, Herdr templates, any autostart/session files. Grep output quoted, with each hit classified: functional dependency, or informational mention (docs/notes saying "never use herdr" are mentions, not dependencies).
   - Whether Stephen's own access paths touch it: his phone reaches herdr-core over Tailscale (established) — state whether anything in the client binary's config (`~/.config/herdr/`) is shared with herdr-core's state, or fully separate.
B2. **Fix your own tooling's references.** For every *functional* `herdr` invocation the inventory finds in **your own** tooling — your cron job definitions and worker prompts, your skills, your scripts — rewrite it to the equivalent `hc` invocation in this task. Quote each change in the reply (file, before line, after line). Touch nothing else in those files: no rewording, no other behavior change. Mentions stay as they are — they are not invocations. A functional reference in **Stephen's** interactive setup (his shell rc, his aliases, an autostart he uses) is NOT yours to rewrite — that is a stop-gate item (step B4), reported, left alone.
B3. **Retire — only if the inventory clears it.** If (a) after step B2, no functional dependency invokes the `herdr` binary, and (b) it is separable from herdr-core's install, and (c) its config/state is not shared with herdr-core: **rename** the binary in place to `herdr-legacy-0.9.5` (same directory). Do not delete anything. Then verify and quote:
   - `hc server status` — same core id, same pid, same live terminal count as before the rename.
   - `hc ls` — the same panes (copilot-main, agy-main, shells), unchanged.
   - `herdr status` now fails as command-not-found (the loud failure is the point — a stale reference should break visibly, not answer wrongly).
   - Note plainly in the reply that the next mailbox worker cycle is the end-to-end check that nothing in the worker path needed the binary; do not simulate a cycle.
B4. **Stop gate.** If a functional dependency remains that step B2 does not cover (Stephen's interactive setup, or anything you cannot classify confidently), or the binary cannot be separated from herdr-core's install, or config/state is shared: do **not** rename the binary, and report exactly what blocks it and why. (Step-B2 rewrites already made stand — they are correct regardless — and are reported as made.) That report is a complete, successful outcome for this part.
B5. **Report-only appendix.** While you are in the same bin directories, list any other plainly dead binaries you notice (no current product, no references found) — names and paths only. Do not rename, move, or delete any of them; Stephen decides on the list.

### Success criteria (falsifiable)
- The reply quotes the install facts (path, type, package, relation to herdr-core's install) and the dependency grep with each hit classified.
- Every functional `herdr` invocation found in Hermes's own tooling is quoted as rewritten to its `hc` equivalent (file, before, after); a final grep of that tooling shows zero remaining functional `herdr` invocations.
- Either: the rename happened and the before/after `hc server status` + `hc ls` outputs match (same pid, same terminals), with `herdr` now command-not-found — or the rename was withheld and the blocking dependency is named exactly.
- herdr-core was never stopped or restarted in this task (same pid and uptime continuity in the before/after outputs prove it).

### Constraints
- **Rename only — delete nothing.** Every change in Part B must be reversible by renaming back.
- Do not stop, restart, or reconfigure herdr-core; do not disturb any live pane.
- Prompt/skill/script edits are authorized for exactly one thing: swapping a functional `herdr` invocation for its `hc` equivalent in **your own** tooling (step B2), each quoted in the reply. No other edits to any cron job, worker prompt, skill, or config. Stephen's interactive setup is never edited in this task.
- If the installed layout differs from what Context describes, follow the machine's evidence and say so.

---

## Shared constraints
- Do not change any cron schedule, the `delegation.worktree_isolation` flag (set by 0193, stays as it is), or any other config beyond what the two parts specify. Do not enable checkpoints.
- Do not touch the mailbox itself beyond the standard pickup/stage transitions for this task.
- **Cost:** one run on the standard tier (DeepSeek Flash) covering both parts. CLI and git work only — no model escalation, no Newton session.

## Proof
- Part A: commit SHA on origin for each repo (or a plain "already present, no commit"), each with its one-line diffstat, plus the cabin-play before/during/after `git status` outputs. Juno reads both `.gitignore` files back from origin herself; pasted output alone is a claim, not proof.
- Part B: host-local — no branch, no CI; `branch: none` / `run: none` is the honest state. The staged reply carries the raw outputs (inventory, grep, rewrites, before/after `hc` state) as the record; Juno verifies them against the facts quoted in Context.

## Reply format
One staged reply covering both parts, any gate failure stated plainly at the very top:
- **Part A:** the two repo SHAs (or already-present notes) with diffstats; the proof-step `git status` outputs.
- **Part B:** gate outcome first (retired / stopped and why); install facts; dependency inventory with classifications; references rewritten (file, before, after) + final zero-hit grep; rename + verification outputs (if done); appendix of other dead binaries (report only).
- Cost block for the whole run.
