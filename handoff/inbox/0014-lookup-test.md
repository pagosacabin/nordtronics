# 0014 — Ping + history lookup (executor test)

## The goal in one sentence

Prove the loop is responsive AND that you can dig a fact out of git history:
create a branch, commit a small file with a secret word, push it, then reply
with one historical fact.

## Steps

### 1. Make a branch

```bash
git checkout -b hermes/0014-lookup
```

### 2. Create the ping file

Create `handoff/tests/0014.txt` with exactly these two lines (no extra whitespace):

```text
0014
juniper
```

### 3. Commit and push

```bash
git add handoff/tests/0014.txt
git commit -m "0014: ping + history lookup"
git push -u origin hermes/0014-lookup
```

### 4. Look up one historical fact (do this AFTER `git fetch origin`)

Find the SHA of the merge commit on `main` that merged the
`hermes/0008-build-node-v1` branch into main (the 0011 task).
Hint: `git log --merges --oneline origin/main` will show it.

### 5. Reply last

Write `handoff/outbox/0014-lookup-test.md` on main with all four:

- Status: done
- Branch: hermes/0014-lookup
- Commit SHA: <`git rev-parse HEAD`>
- 0011 merge commit SHA: <what you found in step 4>

## How it will be checked

The branch must exist on origin with exactly the two lines in the file, the
commit SHA must match, and the merge-commit SHA must be correct against the
live repo. Cite what you actually measured — no guessing.
