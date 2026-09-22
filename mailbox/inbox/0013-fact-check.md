# 0013 — Ping + fact check (executor test)

## The goal in one sentence

Prove the loop is responsive AND that you can gather and cite real facts:
create a branch, commit a small file with a secret word, push it, then reply
with three verified facts.

## Steps

### 1. Make a branch

```bash
git checkout -b hermes/0013-fact-check
```

### 2. Create the ping file

Create `handoff/tests/0013.txt` with exactly these two lines (no extra whitespace):

```text
0013
mesquite
```

### 3. Commit and push

```bash
git add handoff/tests/0013.txt
git commit -m "0013: ping + fact check"
git push -u origin hermes/0013-fact-check
```

### 4. Gather two facts (do this AFTER pushing, from a fresh `git fetch`)

- The current HEAD commit SHA of `main` on origin (short form is fine).
- How many files are currently in `handoff/inbox/` on main.

### 5. Reply last

Write `handoff/outbox/0013-fact-check.md` on main with all four:

- Status: done
- Branch: hermes/0013-fact-check
- Commit SHA: <`git rev-parse HEAD`>
- Main HEAD SHA: <what you found in step 4>
- Files in handoff/inbox: <the count you found in step 4>

## How it will be checked

The branch must exist on origin with exactly the two lines in the file, the
commit SHA must match, and the two facts must be accurate against the live
repo. Cite what you actually measured — no guessing.
