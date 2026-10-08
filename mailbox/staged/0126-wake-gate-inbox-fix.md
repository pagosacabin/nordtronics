---
task_id: "0126"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: main
  sha: "54949e7"
  run: none — no CI run exists for this task; it is a local worker/skill change and
    its own Constraints forbid repo branches, CI runs and repo commits for the
    task's work. The mailbox pickup (54949e7) and this staging commit are the only
    main-line commits it produced.
  files: []
  files_note: no repository file changed — the edit is outside the repo, in the
    worker's pre-run script.
  applied_to:
    - /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh
    - /home/astroboy/.hermes/scripts/mailbox-changed.sh
  local_artifacts:
    script_sha256_before: d763fe0706fad44589c1d0e503f39e6e13115b335cca714a6cf0153302436ed4
    script_sha256_after: ac0cea428e2157d66edb61d51385d153927d9d3de6a87c6c25b6dfd7db49ecfc
    backup: /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh.bak-0126
    protocol_checker: "PROTOCOL: MATCH protocol_version=1.0.0 (strict rc=0)"
notes: |
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-08 16:15 UTC.
  Off-peak (PEAK: OFF-PEAK 16:15 UTC). The filed front-matter carried no
  `protocol_version`; it was added at pickup (protocol field).

  DONE. The durable wake-gate fix is applied on the worker host and verified;
  evidence below. No repo branch, no CI run, no repo file changed — as the
  task's Constraints require. One declared correction to the spec's wording is
  in section 2, and one declared deviation from a success criterion is in
  section 4.
---

# 0126 — Durable wake-gate fix: wake on a non-empty `mailbox/inbox/`

## 1. The exact condition changed

`mailbox-changed.sh` (the cron job's single pre-run `script` slot), installed at
both paths, byte-identical:

| Path | sha256 |
|---|---|
| `/home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-changed.sh` | `ac0cea428e2157d66edb61d51385d153927d9d3de6a87c6c25b6dfd7db49ecfc` |
| `/home/astroboy/.hermes/scripts/mailbox-changed.sh` | `ac0cea428e2157d66edb61d51385d153927d9d3de6a87c6c25b6dfd7db49ecfc` |

Before: `d763fe0706fad44589c1d0e503f39e6e13115b335cca714a6cf0153302436ed4`
(same at both paths). Backup of the pre-change bytes:
`.../scripts/mailbox-changed.sh.bak-0126`. `bash -n` clean; file mode unchanged
(`-rwxr-xr-x`).

The diff, in full — one added condition inside the existing repo-available
branch:

```diff
@@ -73,6 +73,14 @@
   if [ "${ahead:-0}" != "0" ]; then
     reasons="$reasons $ahead-unpushed-commit(s)"
   fi
+  # A task filed LOCALLY on this machine (Stephen/Juno writing the file here, not
+  # pushed) moves no git ref, so every check above is blind to it and the tick is
+  # skipped forever (seen: 0124, 2026-10-07). Wake on a non-empty mailbox/inbox/
+  # so such a task is picked up with no 'Juno push' git-state faking (task 0126).
+  inbox_task=$(find "$REPO/mailbox/inbox" -maxdepth 1 -type f -name '*.md' 2>/dev/null | head -n1)
+  if [ -n "$inbox_task" ]; then
+    reasons="$reasons inbox-non-empty"
+  fi
 fi
```

The gate's verdict line and the trailing `{"wakeAgent": ...}` JSON are
untouched; nothing else in the tick flow changed. `.gitkeep` cannot trip it
(the test is `-name '*.md'`).

## 2. Declared correction to the spec's wording (intent implemented, not the literal phrasing)

The task's Task section says the condition "becomes 'mailbox/inbox/ is
non-empty' **rather than** 'git state differs from origin'". Implemented
literally as a **replacement**, that would have blinded the worker to Juno's
own work: Juno pushes tasks to `origin` and the gate only runs `git fetch`, so
until the agent pulls, a Juno-filed task is not in the local working tree's
`mailbox/inbox/` at all — a replace-only gate would see an empty inbox, skip,
and never pull, stalling every task Juno files forever. The change was
therefore implemented as an **additional** wake reason (the "one condition"
the Context block describes), keeping `main != origin/main`,
`head-on-<branch>`, `dirty-mailbox` and `<N>-unpushed-commit(s)` intact. The
net effect is a strict superset of the old wake set: it can only wake more
often, never less.

A second, unplanned benefit, which the live repo demonstrated: the old gate
also stalled a **multi-task** inbox. After a tick picked one task up and pushed
its pickup commit, `main == origin/main` again, so the old gate answered
`unchanged` and skipped — the remaining queued tasks sat until the next Juno
push. The new condition drains the queue one task per tick.

## 3. Demonstration — the defect reproduced, then fixed (live repo)

State of the live repo at the moment of the two runs (a genuinely non-empty
inbox of **real** queued tasks, clean tree, nothing ahead):

```
main == origin/main: 54949e7552562b954de9edf4dcfaf145a3d5d343 / 54949e7552562b954de9edf4dcfaf145a3d5d343
HEAD: main
tracked modifications under mailbox/: ''
inbox .md files:
  mailbox/inbox/0127-upwork-sep-proposals-status.md
  mailbox/inbox/0128-0119-live-half-proof.md
  mailbox/inbox/0129-upwork-ble-posting-viability.md
  mailbox/inbox/0130-companion-apk-with-api-key.md
```

Pre-fix gate (the backup), against exactly that state:

```
GATE: unchanged (main == origin/main, HEAD on main, mailbox clean) [2026-10-08T16:17:19Z]
{"wakeAgent": false}
```

Post-fix gate, same state, a second later:

```
GATE: mailbox changed -> inbox-non-empty [2026-10-08T16:17:22Z]
{"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "inbox-non-empty"}}
```

## 4. Demonstration — the task's own scenario (locally-filed, unpushed)

Criterion 1 asks for "a test task file placed locally in `mailbox/inbox/` (not
pushed anywhere)" waking the next tick. A scratch repo
(`<scratch>/0126-probe/repo`, a clone of a local bare origin) was driven to the
identical git state — `main == origin/main`, `HEAD` on `main`, no tracked
modifications under `mailbox/`:

```
== PROBE 1: clean tree, empty inbox (no .md) ==
GATE: unchanged (main == origin/main, HEAD on main, mailbox clean) [...]
{"wakeAgent": false}

== PROBE 2: SAME git state, one LOCAL unpushed task in mailbox/inbox/ ==
?? mailbox/inbox/9999-local-test.md          <- untracked, unpushed; no ref moved
GATE: mailbox changed -> inbox-non-empty [...]
{"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "inbox-non-empty"}}

== gate log ==
... GATE wake=false unchanged
... GATE wake=true inbox-non-empty
```

And end-to-end through the scheduler's **own** runner and parser, with the
stored job dict (`cron.jobs.load_jobs` → `cron.scheduler_script._run_job_script`
→ `cron.scheduler_prompt._parse_wake_gate`), `HERMES_HOME` pinned to the
cronrunner profile — not a manual `bash` call:

```
JOB: {"id":"c0be50a686c6","name":"Mailbox worker (DeepSeek) Mon-Thu",
      "script":"mailbox-changed.sh","workdir":"/home/astroboy/nordtronics",
      "model":"deepseek-flash","provider":"deepseek"}
SCRIPT ok: True
PEAK: OFF-PEAK 16:17 UTC -- safe to work.
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
GATE: mailbox changed -> inbox-non-empty [2026-10-08T16:17:09Z]
{"wakeAgent": true, "context": {"mailbox_changed": true, "reasons": "inbox-non-empty"}}
wakeAgent (scheduler parser): True
```

**Declared deviation on this criterion.** I did **not** inject a fabricated test
file into the live `mailbox/inbox/`: the next tick would then move it to
`active/` and push a pickup commit for a task that does not exist, which Juno
would see in the real queue. The criterion's mechanism — the gate waking on a
locally-filed inbox file with no git-state faking — is instead proven in the
scratch repo whose git state is byte-identical to origin's, and on the live
repo's own real queue (section 3). The scratch test file
(`0126-probe/repo/mailbox/inbox/9999-local-test.md`) was created there and
deliberately left in that scratch clone as the artifact; the **live** repo was
never given a test file, so there was nothing to remove there — the live
repo's `git status` under `mailbox/` is clean apart from this task's own
`inbox → active` pickup.

## 5. Criterion 2 — the protocol checker still reports MATCH

```
$ /usr/bin/python3 ~/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py --strict
PEAK: OFF-PEAK 16:17 UTC -- safe to work.
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
strict rc=0
```

`mailbox-worker-state.json` is unchanged
(`last_seen_protocol_version: "1.0.0"`).

## 6. Scope / honesty notes

- Cost: this run is on the worker's own tier, `deepseek-flash` / provider
  `deepseek` — the cheapest tier, as the Constraints require.
- The seven tasks in `active/` (0097/0098/0106/0107/0109/0110/0119) are all
  decision-blocked and were left completely untouched, per the worker loop
  rules. 0119 stays parked: `0128`, the inbox task carrying its unblocking fix,
  is a later task number and was not worked this run.
- Nothing beyond this task's own instruction was applied on my own authority;
  no cron job field, schedule or `next_run_at` was touched.
