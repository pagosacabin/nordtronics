# 0196 — Commit `.worktrees/` to .gitignore in nordtronics and cabin-play

## Assignment
- **Hermes** (host-local git work on your own machine): all steps.
- **Newton:** not used — no code.
- **Copilot:** not used — this is a two-line ignore-file chore committed directly to main in both repos; Juno verifies the result from origin herself. (Not a code change; no PR.)
- **Juno:** verifies both `.gitignore` files from origin, archives on green.

## Context
Finding from 0193 (verified and archived 2026-11-16): every time the Hermes harness creates a worktree — the `/worktree new` route and delegated subagent trees both do it — it appends `.worktrees/` to the **shared checkout's** `.gitignore` as an uncommitted edit. The edit persists after the tree is removed, so every worktree creation leaves the shared checkout dirty on `main` until someone reverts the line by hand. Hermes reverted it twice during 0193.

Stephen's call, 2026-11-16: commit the line once, in both repos, so worktree creation stops dirtying the shared checkouts.

## Task
1. In each repo — **pagosacabin/nordtronics** and **pagosacabin/cabin-play**, local clones on your machine:
   - Read the root `.gitignore`. If it already contains a `.worktrees/` entry, change nothing in that repo and say so.
   - Otherwise append exactly one entry, `.worktrees/`, at the end of the file (create the file with just that entry if the repo has none). Change nothing else in the file.
   - Commit **only** the `.gitignore` change to `main` and push. Commit message: `gitignore: ignore Hermes worktree checkouts (.worktrees/)`.
2. **Prove the fix** in cabin-play: from a fresh interactive CLI session, create a named worktree once (the 0193 route), then show `git status --short` in the shared checkout — it must be clean, with **no** `.gitignore` modification. Clean up the worktree the documented way and show the shared checkout still clean, on main, at the new HEAD.
3. Stage the reply with both commit SHAs.

## Success criteria (falsifiable)
- Both repos' root `.gitignore` on origin `main` contain `.worktrees/` exactly once, and each repo's commit for it touches only `.gitignore` (state the SHA and the one-line diffstat for each).
- The cabin-play proof shows: worktree created → shared checkout `git status` clean (no `.gitignore` line) → worktree removed → still clean.
- If either repo already had the entry, that repo has no new commit and the reply says so plainly.

## Constraints
- Only `.gitignore` changes. No other file in either commit — if a checkout is dirty with unrelated changes, stop and report rather than committing around them.
- Do **not** touch the 16 external `~/wt-NNNN` / `~/nordtronics-NNNN` trees or their branches — separate decision, not this task.
- Do not change any cron job, config, or the `delegation.worktree_isolation` flag (set by 0193, stays as it is). Do not enable checkpoints.
- Pushes over SSH as usual (the gh.py credential cannot write cabin-play). Do NOT source `gh-env.sh` for anything in this task.
- **Cost:** one short run on the standard tier (DeepSeek Flash). CLI and git work only — no model escalation, no Newton session.

## Proof
- Commit SHA on origin for each repo (or a plain "already present, no commit" for a repo that needed none), each with its one-line diffstat.
- The cabin-play before/during/after `git status` outputs from the proof step.
- Juno verifies by reading both `.gitignore` files back from origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, the two repo SHAs (or already-present notes), the proof-step outputs, cost block, and any gate failure stated plainly at the top.
