# 0012 — Ping test (executor check)

## The goal in one sentence

Prove the task loop is responsive: create a branch, commit a small file with a
secret word, push it, then reply.

## Steps

### 1. Make a branch

```bash
git checkout -b hermes/0012-ping
```

### 2. Create the ping file

Create `handoff/ping/0012.txt` with exactly these two lines (no extra whitespace):

```text
0012
pinon
```

Why the secret word: it proves the file was really created by you, not copied
from a template.

### 3. Commit and push

```bash
git add handoff/ping/0012.txt
git commit -m "0012: ping test"
git push -u origin hermes/0012-ping
```

### 4. Reply last

Write `handoff/outbox/0012-ping-test.md` on main with all three:

- Status: done
- Branch: hermes/0012-ping
- Commit SHA: <`git rev-parse HEAD`>

## How it will be checked

The branch must exist on origin, the file must exist on that branch with exactly
the two lines above, and the SHA must match the commit. No CI involved.
