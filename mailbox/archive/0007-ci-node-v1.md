# 0007: CI builds firmware/node-v1 (guided walkthrough)

**Status:** open

## Goal
Make GitHub Actions automatically build `firmware/node-v1` on every push,
the way it already builds `firmware/tank-monitor`.

We go step by step, in order. Each step says WHAT to run and WHY it exists.
Don't skip ahead — the why is the lesson.

## Step 1 — Build node-v1 on your machine
Why: prove the firmware compiles locally before asking GitHub to build it.
```
cd /home/astroboy/nordtronics
pio run -d firmware/node-v1
```
Expect SUCCESS at the end. If it fails, fix the firmware first, then continue.
A local failure will also fail on GitHub — catching it here saves a round trip.

## Step 2 — Read the workflow file
Why: see what CI does today before you change it.
```
cat .github/workflows/platformio.yml
```
Find the `paths:` filter. It lists only `firmware/tank-monitor/**` — that is
the reason node-v1 never builds. Your edit adds node-v1 to that filter and
adds a matching build step.

## Step 3 — Branch, edit, commit, push
Why: all work happens on a branch. `main` stays clean until the work is proven.
```
git checkout -b hermes/0007-ci-node-v1
```
Now edit `.github/workflows/platformio.yml`: a push or PR touching
`firmware/node-v1/**` (or the workflow file itself) must trigger a PlatformIO
build of `firmware/node-v1`. Keep the tank-monitor build working.
```
git add .github/workflows/platformio.yml
git commit -m "ci: build firmware/node-v1 in PlatformIO workflow"
git push -u origin hermes/0007-ci-node-v1
```

## Step 4 — Watch GitHub build it
Why: your push tells GitHub's servers to compile the firmware. Your machine's
build (Step 1) does not count as CI. The task isn't done until THEIR build passes.
```
gh run list --branch hermes/0007-ci-node-v1 --limit 3
gh run watch <run-id>
gh run view <run-id> --json conclusion,url --jq '{conclusion, url}'
```
`gh run watch` waits — run it, don't guess. If conclusion isn't `success`,
read the failure (`gh run view <run-id> --log-failed`), fix, push again, re-watch.

## Step 5 — Reply (last, not first)
Why: the reply reports finished work. Writing it earlier is how fake "done"s happen.
Write `handoff/outbox/0007-ci-node-v1.md` (exact filename) containing:
- Status: done
- Branch name and commit SHA(s)
- The Actions run URL and its conclusion

If you're blocked at any step: reply with Status: blocked, naming the step
number and exactly what happened. Blocked is always better than fake-done.
