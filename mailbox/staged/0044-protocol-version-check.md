---
task_id: "0044"
protocol_version: 1.0.0
status: staged
iteration: 1
proof:
  - artifact: mailbox/staged/0044-protocol-version-check.md
  - note: no branch or CI run applies — this task is a local worker + skill change only
---

# 0044 — Adopt the protocol-update mechanism (v1.0.0)

## Reply

```yaml
protocol_version: 1.0.0
worker_state_key: last_seen_protocol_version
worker_state_file: /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json
readme_fetch_source: git -C /home/astroboy/nordtronics fetch --quiet origin main, then git show origin/main:mailbox/README.md (the checker runs this every tick)
mismatch_behavior: on mismatch the tick stops before any other task — the protocol-update task in mailbox/inbox/ is picked up and completed first (its delimited skill-patch block applied verbatim to the handoff-mailbox skill, then the normal pickup -> staged flow); only then does `mailbox-protocol-check.py --record` write the new version into the state file. The next tick reports PROTOCOL: MATCH.
skill_block_location: handoff-mailbox SKILL.md — new delimited block inserted after the "Current protocol (since task 0032 cutover)" section and before "## Quick Start (legacy handoff/ protocol)"; the block is the only PROTOCOL-MANAGED region in the file
managed_skill_block: |
  <!-- BEGIN PROTOCOL-MANAGED (do not edit by hand — updated via mailbox protocol tasks) -->
  ## Protocol updates
  
  - The canonical protocol is `mailbox/README.md` on origin/main. It carries
    `protocol_version: <semver>`.
  - The worker keeps `last_seen_protocol_version` in its local state file.
    Every tick it fetches the README and compares. On mismatch, it picks up and
    completes the protocol-update task from `mailbox/inbox/` before any other
    task, then records the new version in its state file.
  - Protocol-update tasks are always written in the previous protocol format.
    Each carries exact skill-patch text between these delimiters: replace this
    whole block verbatim. Never paraphrase — paraphrase is where drift enters.
  - A protocol update is not complete until the new rule is demonstrated in the
    staged reply itself, not merely acknowledged.
  <!-- END PROTOCOL-MANAGED -->
```

## What changed

### 1. Worker version check

- **State file:** `/home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json`
- **Key:** `last_seen_protocol_version` (currently `1.0.0`)
- **Checker:** `/home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py`
- **Fetch source, every tick:** `git -C /home/astroboy/nordtronics fetch --quiet origin main`
  followed by `git show origin/main:mailbox/README.md`; the `protocol_version:` line is parsed
  from the committed README, never from the worktree copy.
- **Wiring:** the checker is attached as the cron job's pre-run `script`
  (`hermes cron edit c0be50a686c6 --script mailbox-protocol-check.py`), so it executes before the
  agent prompt on **every** tick and its stdout is injected into the prompt as one `PROTOCOL:`
  line. Verified in `jobs.json`: `"script": "mailbox-protocol-check.py"`, `"no_agent": false`.
- **Prompt step:** the job prompt now opens with `## Step 0 — protocol version check (always
  first)`, which tells the tick what to do with each verdict.

Why the checker is default-non-failing: a non-zero script exit is a failed cron run. So the
default mode prints the verdict and exits 0; `--strict` restores real exit codes (0 match,
10 mismatch, 20 fetch error) for manual or CI use. `--record` stores the remote version.

### 2. Managed skill block

Applied verbatim to `handoff-mailbox` (cronrunner profile,
`~/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md`). The block appears
exactly once, between the `<!-- BEGIN PROTOCOL-MANAGED ... -->` and
`<!-- END PROTOCOL-MANAGED -->` delimiters; the quoted copy above was byte-compared against the
canonical patch text in this task (887 bytes, identical) with a scripted diff, not by eye. The
surrounding prose line naming the checker path/state file sits **outside** the delimiters so a
future protocol update can replace the block without touching it.

## Demonstration (not merely acknowledged)

The rule was exercised on this machine, including the stale-version path:

```
# state file seeded with an older version
$ cat ~/.hermes/profiles/cronrunner/mailbox-worker-state.json
{ "last_seen_protocol_version": "0.9.0" }

$ python3 .../scripts/mailbox-protocol-check.py --strict
PROTOCOL: MISMATCH local='0.9.0' remote='1.0.0' (origin/main:mailbox/README.md)
ACTION: complete the protocol-update task in mailbox/inbox/ before any other task, then run: mailbox-protocol-check.py --record
exit=10

$ python3 .../scripts/mailbox-protocol-check.py --record
PROTOCOL: RECORDED last_seen_protocol_version=1.0.0 in .../mailbox-worker-state.json

$ python3 .../scripts/mailbox-protocol-check.py --strict
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
exit=0
```

Pre-run mode (no flags, as the cron script runs it) reports the same verdict and always exits 0,
so a MISMATCH cannot be mistaken for a failed tick.

## Success criteria

1. Front-matter above carries `protocol_version: 1.0.0`. ✅
2. The `managed_skill_block` quoted above is byte-identical to the canonical patch text in this
   task, as it exists in the skill file. ✅ (scripted byte comparison, 887 bytes)
3. Worker change documented concretely: key name, fetch source, mismatch behavior. ✅

## Notes

- No repo branch and no CI run: per the task's own Constraint ("Local worker/skill changes only.
  No repo branches needed for this task"), the only repository artifact is this staged file on
  main. Juno's `verify-staged.py` should therefore expect no `branch`/`run` pointers here.
- This task was written in the pre-1.0.0 format and completes under it; the mechanism it
  installs takes effect from the next tick onward.
- The demonstration transcript above is the demonstration required by the new rule, not offered
  as independent proof — the verifiable artifacts are the staged file, the skill file on this
  machine, the checker script, and the job config.
- This staged file is the reply. Nothing was written to `handoff/outbox/`.
