# 0011 — Merge the node-v1 CI build into main

## The goal in one sentence

The `hermes/0008-build-node-v1` branch proves CI can compile `firmware/node-v1`.
Merge it into `main` so every push to main builds node-v1 too.

## Why this matters

Right now the fix only exists on the branch. `main` still compiles only
tank-monitor. Until this merges, the 0008 work isn't live.

## Steps

### 1. Update main and look at what's coming

```bash
git checkout main
git pull origin main
git log --oneline -3
git log --oneline main..hermes/0008-build-node-v1
```

Why: see exactly which commits you're about to merge. Expect one:
"0008: CI builds firmware/node-v1".

### 2. Merge

```bash
git merge hermes/0008-build-node-v1 --no-ff -m "Merge 0008: CI builds firmware/node-v1 into main"
```

If git reports a conflict: STOP. Do not force anything. Reply with
Status: blocked and paste the conflict output. A blocked reply is always
better than a broken main.

### 3. Push and watch CI

```bash
git push origin main
```

The push triggers the workflow on main. Watch it:

```bash
gh run list --branch main --limit 3
gh run watch <run-id>
```

Why: the merge isn't done until main's CI is green AND the log shows the
node-v1 build step succeeding.

### 4. Reply last

Write `handoff/outbox/0011-merge-0008-to-main.md` on main with all four:

- Status: done
- Merge commit SHA: <`git rev-parse HEAD` after the merge>
- Actions run URL on main + conclusion
- Confirmation the node-v1 step ran (step name from the log)

No citation, no credit — you know this one by now.

## How it will be checked

The merge commit on main, and the Actions run on main showing the node-v1
build step green. If main's CI is red, the task is not done.
