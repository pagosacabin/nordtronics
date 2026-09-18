# 0008: CI actually compiles node-v1

**Status:** open

## What happened in 0007 (read this first)
You ran the loop correctly: branch, commit, push, watched the run, replied
with the URL. That part passed — this is how the loop is supposed to work.

But the green run only compiled tank-monitor. Look at your own diff: you added
`firmware/node-v1/**` to the `paths:` trigger, so CI *starts* when node-v1
changes — but the `build` job still only compiles `firmware/tank-monitor`.
Triggering is not building. The task isn't done until CI compiles node-v1.

## The work
On a new branch `hermes/0008-build-node-v1`, edit `.github/workflows/platformio.yml`
so the build job also compiles `firmware/node-v1`. Minimal change wins — e.g. a
second build step running `pio run -d firmware/node-v1`, or a matrix over both
firmware dirs. Your choice. Commit, push.

## Verify (same as 0007)
```
gh run list --branch hermes/0008-build-node-v1 --limit 3
gh run watch <run-id>
gh run view <run-id> --json conclusion,url --jq '{conclusion, url}'
```
Then prove node-v1 actually compiled — the run log must show it:
```
gh run view <run-id> --log | grep -i "node-v1"
```
If the log doesn't show a node-v1 build, the task isn't done.

## Reply
`handoff/outbox/0008-build-node-v1.md` (exact filename): Status, branch,
commit SHA(s), run URL + conclusion.
