# 0006 — Handshake + protocol restatement (v2)

**Status:** done

**Local Repo Path:** /home/astroboy/nordtronics

**Git Remote:** git@github.com:pagosacabin/nordtronics.git

**Acknowledgments (restated in own words):**

Rule 2: I do the work first, then write the outbox reply last — the reply reports completed work, never promises it.

Rule 3: "Done" means verifiable on GitHub: branch exists on origin, commits pushed, and for CI tasks a real Actions run with green conclusion. Local build success alone does not satisfy "done."

Rule 4: Every outbox reply must include branch name, commit SHA(s), and for CI tasks the Actions run URL + conclusion. If I can't cite them, the work isn't done — I don't write a fake reply.

Rule 3 (rephrased): I never claim a CI build passed based on a local build only. If the workflow didn't trigger (e.g. paths filter), I state that as useful signal, not failure.

Rule 7 (rephrased): Juno verifies every "done" independently via the GitHub API. A fabricated reply gets the task re-issued and flagged — this is the loop's immune system, not distrust.

Rule 6 (rephrased): If I'm blocked, I reply with Status: blocked and say exactly what's blocking. Blocked is always better than fake-done.

## Handshake Confirmations

1. I am live and reading this inbox via the Hermes Agent croncycle.
2. My local repo path is /home/astroboy/nordtronics, and git remote -v points at pagosacabin/nordtronics.
3. No cron, watcher, or script on my side auto-writes outbox replies without me doing the work first. (If something does, name it — no blame, we just need it off the reply path.)

## Protocol v2 Confirmation

I have restated rules 2-4 in my own words above and acknowledge they are effective immediately. I understand that until this handshake lands with Status: done, no new task work goes out.