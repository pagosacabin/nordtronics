> Re-issued 2026-09-18 16:30 MDT — previous outbox reply was a phantom and has been removed. Clean slate, please run fresh.

# 0008 — Make CI actually build node-v1 (re-issued)

## The goal in one sentence

Right now GitHub Actions compiles only `firmware/tank-monitor`. Change the workflow
so it really compiles `firmware/node-v1`, and prove it from the build log.

## Why this matters

Last time the CI run was green, but green only meant "something compiled" — the build
job never touched node-v1. Triggering CI is not the same as building node-v1. This
task is done only when the log shows node-v1 being compiled.

## Steps

### 1. Read the workflow file (2 min)

Open `.github/workflows/platformio.yml` in your repo. Find the build job and the exact
`pio run` command it runs. Notice which firmware directory it points at.

Why: you can't fix what you haven't read.

### 2. Make a branch

```bash
git checkout -b hermes/0008-build-node-v1
```

(If that branch is still around locally from before, delete it first:
`git branch -D hermes/0008-build-node-v1`.)

Why a branch: your work stays separate from main until it's verified.

### 3. Edit the workflow so it builds node-v1

Simplest fix: add a second build step running `pio run -d firmware/node-v1`
(a matrix over both firmware directories also works — keep it small).

Why a second step instead of replacing: the existing tank-monitor build keeps
working, and node-v1 gets compiled too.

### 4. Commit, push, watch the real run

```bash
git add .github/workflows/platformio.yml
git commit -m "0008: CI builds firmware/node-v1"
git push -u origin hermes/0008-build-node-v1
```

Then watch GitHub Actions and check the log yourself:

```bash
gh run list --branch hermes/0008-build-node-v1 --limit 3
gh run watch <run-id>
gh run view <run-id> --log | grep -i "node-v1"
```

Why: you must see node-v1 in the log with your own eyes before claiming done.
A green checkmark without node-v1 in the log is a fail.

### 5. Reply last

Write `handoff/outbox/0008-build-node-v1.md` on main (switch back with
`git checkout main` first, then add, commit, and push the reply). It MUST contain
all four:

- Status: done
- Branch: hermes/0008-build-node-v1
- Commit SHA: <your real commit SHA — get it with `git rev-parse HEAD`>
- Actions run URL and its conclusion

Why all four: the last reply was missing the branch and SHA, so the work couldn't
be matched to the report. No citation, no credit.

## How it will be checked

The branch, the commit, and the Actions run will be looked up independently.
Pass = the run is green AND its log shows node-v1 compiling. Anything less is not
done — reply with Status: blocked and describe what went wrong instead.
