# 0009 — Ping test (small and quick)

Goal: prove the full loop still works, fast. No CI this time — just git.

## Why this task

A "done" reply only counts if the work is visible on GitHub. This test is small on
purpose: one branch, one tiny file, one push. If I can see the branch and the file
on GitHub, you pass.

## Steps

### 1. Make a branch

Branch name: `hermes/0009-ping`

Why: your work lives on a branch, never directly on main. That keeps main clean
and lets the work be checked separately.

```bash
git checkout -b hermes/0009-ping
```

### 2. Add the ping file

Create `handoff/ping/0009.txt` with exactly these two lines:

```
0009
cactus
```

Why the odd word: it proves the file is new and written for this task, not copied
from an older one.

### 3. Commit and push

```bash
git add handoff/ping/0009.txt
git commit -m "0009: ping test"
git push -u origin hermes/0009-ping
```

Why push: until the branch is on GitHub (`origin`), it can't be verified.
Local-only work doesn't count as done.

### 4. Reply last

Only after the push succeeds, write the reply file `handoff/outbox/0009-ping-test.md`
on main (switch back first: `git checkout main`, then add, commit, and push the reply).
The reply must contain:

- Status: done
- Branch: hermes/0009-ping
- Commit SHA: <the SHA from your commit>

Why reply last: the reply is a report of finished work, not the work itself.

## How it will be checked

The branch will be looked up on GitHub and the file read there. If the branch
isn't on GitHub, the task isn't done — reply with Status: blocked and describe
what went wrong instead.
