# 0007: CI builds firmware/node-v1

**Status:** open

## Background
The verified node-v1 firmware scaffold (`firmware/node-v1/`, from task 0004) exists on main, but the CI workflow (`.github/workflows/platformio.yml`) only builds `firmware/tank-monitor/` — its `paths` filter ignores node-v1. So node-v1 currently has no automated build.

## The work
1. Update `.github/workflows/platformio.yml` so a push or PR touching `firmware/node-v1/**` (or the workflow file itself) triggers a PlatformIO build of `firmware/node-v1`, the same way tank-monitor builds today.
2. Keep the existing tank-monitor build working — don't break it.
3. Do it on a branch named `hermes/0007-ci-node-v1`, commit, push.

Small task on purpose. If the workflow needs restructuring to build both firmware dirs cleanly, do the minimal change that works.

## Definition of done (protocol v2)
- Branch `hermes/0007-ci-node-v1` exists on origin with your commits pushed.
- A real GitHub Actions run for that branch, building node-v1, with green conclusion.
- Your outbox reply is `handoff/outbox/0007-ci-node-v1.md` (mirror this inbox filename exactly) with Status, branch name, commit SHA(s), and the Actions run URL + conclusion.

Work first, reply last. If you're blocked, reply with Status: blocked and say what's blocking.

## How to verify the CI run (do this before writing your reply)
After pushing, the build runs on GitHub — not on your machine. Check it with:
```
gh run list --branch hermes/0007-ci-node-v1 --limit 3
gh run watch <run-id>    # waits until the run finishes; run this, don't guess
gh run view <run-id> --json conclusion,url --jq '{conclusion, url}'
```
Do NOT write your outbox reply until `conclusion` is `success`. If it fails, read the logs (`gh run view <run-id> --log-failed`), fix, push again, and re-check. The run URL goes in your reply — a reply without one doesn't count.
