---
task_id: "0053"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - artifact: mailbox/staged/0053-protocol-stickiness-demo.md
  - commit: 145f71bd5afcb25d766a08dee270b644914c31b7 (pickup of 0053, inbox -> active, on main; the demonstration ran on top of this commit)
  - checker: /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py
  - state_file: "/home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json (last_seen_protocol_version 1.0.0)"
  - skill_file: "/home/astroboy/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md (sha256 6ccd9867faacc782b30fc3ef24aa2e84f7d3b03b4a41b5fc32bdc0bdabcfdbff)"
  - cron_wiring: "worker job c0be50a686c6 - script mailbox-protocol-check.py, no_agent false"
  - note: "no branch and no CI run apply - the task's own Constraint is 'Local worker/skill changes only. No repo branches, no CI runs.' The only repository artifact is this staged file on main."
notes: |
  Staged 2026-09-24 ~22:19 MDT by Hermes (cronrunner), from a fresh cron tick with no
  carried-over conversation. The demonstration is the transcript in the body (sections
  [1]-[6]); all four stated success criteria are met.

  Verdict line in the normal path, from two independent sources: PROTOCOL: MATCH
  protocol_version=1.0.0 (origin/main:mailbox/README.md) - injected by the scheduler's
  pre-run script into this tick's prompt ([3b]), and reproduced by in-session runs of the
  checker both without flags and with --strict ([3]).

  Three stale pointers / discrepancies are flagged in the body rather than smoothed:
  (1) the task's Proof and Reply-format sections ask for iteration: 1, but mailbox/README.md
  is authoritative and its Pickup rule increments iteration on inbox -> active, so this
  reply carries iteration: 2 (0053 arrived at 1, unlike every other task, which arrived
  at 0); (2) 0044's "891 bytes / md5 31717685dbcb00a367495465d1a1734d" for the managed
  block counts it without a trailing newline - with it the figures are 892 bytes / md5
  9282926226d79b7c6b875e72881540fb, and section [6] prints both; (3) the transcript is the
  third execution of the demonstration - attempt 1 hit a transient DNS failure and
  exercised the checker's ERROR path (exit=20, quoted in the body), attempt 2 was
  discarded for a mislabelled command echo, attempt 3 is unedited and is what is printed.
  Section [2]'s state file updated_at (04:16:51Z) is therefore attempt 2's value, not this
  run's (04:17:45Z); nothing was edited or reordered, the script was re-run.

  No branch, no CI run, no handoff/outbox write; the PROTOCOL-MANAGED block was read, not
  written. This staged file on main is the reply.
---

# 0053 — Demonstrate protocol stickiness from a fresh session (0044 follow-up)

## Context

0044 (staged, iteration 1) installed the protocol-update mechanism v1.0.0:
every tick's Step 0 fetches mailbox/README.md from origin/main and compares
protocol_version against the local state file. But the 0052 incident (model
switch to Nemotron 3 Ultra) proved a fresh session can still carry stale
protocol knowledge ("staged files live on feature branches") until corrected
against the README. 0044's mechanism needs a live demonstration that a fresh
session actually follows the README, not memory. Stephen asked for this
follow-up.

## Task

From a fresh agent session (a normal cron tick qualifies — every tick starts
with no carried-over conversation), demonstrate the protocol check working as
installed:

1. Step 0 fetches mailbox/README.md from origin/main (git fetch + git show —
   never the worktree copy).
2. It compares the README's protocol_version against
   /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json.
3. It reports the verdict line (PROTOCOL: MATCH/MISMATCH) into the session.

Then prove the mismatch path is armed: temporarily seed the state file with an
older version, run the checker with --strict, show MISMATCH plus the prescribed
action, then --record back to 1.0.0 and show MATCH again. (0044 did this once;
repeat it from the fresh session to prove the wiring survived.)

## Success criteria

1. Fresh-session transcript shows the README fetched from origin/main and the
   version comparison performed — not asserted from memory.
2. Verdict line reads PROTOCOL: MATCH protocol_version=1.0.0 in the normal path.
3. Mismatch path: seeded old version -> MISMATCH with the correct prescribed
   action -> --record -> MATCH, all from the fresh session.
4. The PROTOCOL-MANAGED skill block in the handoff-mailbox SKILL.md is still
   byte-identical to 0044's canonical text (scripted comparison, not by eye).

## Constraints

- Local worker/skill changes only. No repo branches, no CI runs.
- Do not edit the PROTOCOL-MANAGED block by hand; the demonstration only reads it.
- One deliverable: the staged reply.

## Proof

Reply staged to mailbox/staged/0053-protocol-stickiness-demo.md on main with:
status: staged, iteration: 1, the full fresh-session transcript (commands and
output), and the byte-comparison result. The staged file on main is the proof;
a pasted transcript anywhere else is a claim.

## Reply format

Follow mailbox/README.md: front-matter (task_id, protocol_version, status,
iteration, proof), then the reply body. Include reply notes per the 0052
precedent: flag any stale pointer you catch in your own citations.

---

## Reply

Staged 2026-09-24 ~22:19 MDT by Hermes (cronrunner), from a fresh cron tick with
no carried-over conversation. The first thing this tick saw was the scheduler's
pre-run Script Output, quoted verbatim in section [3b]. No protocol knowledge is
asserted from memory anywhere below — every verdict comes from a command whose
output is printed with it.

### Criterion 1 — README fetched from origin/main, not the worktree copy ✅

Section [1]: `git fetch --quiet origin main`, then
`git show origin/main:mailbox/README.md`. The README was read out of the object
store at the commit `origin/main` points at (145f71b…), never from the checkout
on disk. Section [2] prints the local state file the version is compared
against; the comparison itself is section [3].

### Criterion 2 — normal-path verdict ✅

```
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
```

Reported twice over, independently: by the scheduler's pre-run invocation of the
checker for THIS tick ([3b]), and by in-session runs of the same checker both
without flags (as the cron `script` field invokes it) and with `--strict` ([3]).
Exit code 0 in both modes.

### Criterion 3 — mismatch path armed and exercised ✅

Section [4] seeds `{"last_seen_protocol_version": "0.9.0"}` — the same
0.9.0-vs-1.0.0 shape 0044 used — and shows, in order: `--strict` returns
`PROTOCOL: MISMATCH local='0.9.0' remote='1.0.0'` plus the prescribed
`ACTION: complete the protocol-update task in mailbox/inbox/ before any other
task, then run: mailbox-protocol-check.py --record`, with **exit=10**; `--strict`
leaves the state file untouched (still 0.9.0); `--record` writes 1.0.0; the next
`--strict` returns MATCH with **exit=0**; and the state file is back to 1.0.0.
The stale 0.9.0 was seeded deliberately for the demonstration and cleared by the
mechanism's own `--record` in the same run.

### Criterion 4 — PROTOCOL-MANAGED block byte-identical to 0044's canonical text ✅

Section [6], scripted comparison (no eye-diffing):

- canonical source: the `managed_skill_block:` scalar in 0044's task file,
  `/home/astroboy/nordtronics/mailbox/staged/0044-protocol-version-check.md`
- subject: the delimited block in
  `/home/astroboy/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md`
- result: `identical : True` — 891 bytes / md5
  `31717685dbcb00a367495465d1a1734d` without the trailing newline, 892 bytes /
  md5 `9282926226d79b7c6b875e72881540fb` with it. My extraction of 0044's
  canonical text reproduces 0044's own recorded md5 exactly, so the equality is
  between the right two things rather than between two copies of my own mistake.
- exactly 1 PROTOCOL-MANAGED block in the skill file; skill file sha256
  `6ccd9867faacc782b30fc3ef24aa2e84f7d3b03b4a41b5fc32bdc0bdabcfdbff`.

The block was read, never written, by this run.

### Transcript (fresh session, unedited)

```
session started: 2026-09-24 22:17 MDT (fresh cron tick — no carried-over conversation)

=== [1] Step 0 — README read from origin/main, not the worktree copy
$ git -C /home/astroboy/nordtronics fetch --quiet origin main
fetch exit=0
$ git -C /home/astroboy/nordtronics rev-parse origin/main   # exact commit the README is read from
145f71bd5afcb25d766a08dee270b644914c31b7
$ git -C /home/astroboy/nordtronics show origin/main:mailbox/README.md | sed -n '1,4p'
# Mailbox protocol — global rules

protocol_version: 1.0.0

$ git -C /home/astroboy/nordtronics show origin/main:mailbox/README.md | grep -m1 '^protocol_version:'
protocol_version: 1.0.0

=== [2] local worker state file
$ cat /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json
{
  "last_seen_protocol_version": "1.0.0",
  "updated_at": "2026-09-25T04:16:51+00:00",
  "readme_ref": "origin/main:mailbox/README.md"
}

=== [3] normal path — comparison performed, verdict reported
$ python3 /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py          # exactly how the cron pre-run script invokes it (no flags)
PEAK: OFF-PEAK 04:17 UTC -- safe to work.
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
exit=0
$ python3 /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py --strict
PEAK: OFF-PEAK 04:17 UTC -- safe to work.
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
exit=0

=== [3b] what the scheduler injected into THIS tick's prompt (from its own Script Output)
PEAK: OFF-PEAK 04:15 UTC -- safe to work.
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)

=== [4] mismatch path armed — stale version seeded deliberately
$ printf '{"last_seen_protocol_version": "0.9.0"}\n' > /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json
{"last_seen_protocol_version": "0.9.0"}
$ python3 /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py --strict
PEAK: OFF-PEAK 04:17 UTC -- safe to work.
PROTOCOL: MISMATCH local='0.9.0' remote='1.0.0' (origin/main:mailbox/README.md)
ACTION: complete the protocol-update task in mailbox/inbox/ before any other task, then run: mailbox-protocol-check.py --record
exit=10
$ cat /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json                     # --strict must not write state
{"last_seen_protocol_version": "0.9.0"}
$ python3 /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py --record
PEAK: OFF-PEAK 04:17 UTC -- safe to work.
PROTOCOL: RECORDED last_seen_protocol_version=1.0.0 in /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json
RECORDED last_seen_protocol_version=1.0.0 in /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json
exit=0
$ python3 /home/astroboy/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py --strict
PEAK: OFF-PEAK 04:17 UTC -- safe to work.
PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md)
exit=0
$ cat /home/astroboy/.hermes/profiles/cronrunner/mailbox-worker-state.json                     # back to 1.0.0
{
  "last_seen_protocol_version": "1.0.0",
  "updated_at": "2026-09-25T04:17:45+00:00",
  "readme_ref": "origin/main:mailbox/README.md"
}

=== [5] worker wiring — the checker is attached as the job's pre-run script
$ python3 - <jobs.json reader>
job id   : c0be50a686c6
schedule : {'kind': 'cron', 'expr': '15 4-18,22,23 * * *', 'display': '15 4-18,22,23 * * *'}
script   : mailbox-protocol-check.py
no_agent : False
model    : deepseek-flash
=== [6] criterion 4 — PROTOCOL-MANAGED block byte comparison (scripted, not by eye)
$ python3 /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0053-block-compare.py
canonical : /home/astroboy/nordtronics/mailbox/staged/0044-protocol-version-check.md
            15 lines, 892 bytes, md5 9282926226d79b7c6b875e72881540fb
subject   : /home/astroboy/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md
            15 lines, 892 bytes, md5 9282926226d79b7c6b875e72881540fb
identical : True
           subject without trailing newline: 891 bytes, md5 31717685dbcb00a367495465d1a1734d
           subject with trailing newline: 892 bytes, md5 9282926226d79b7c6b875e72881540fb
PROTOCOL-MANAGED blocks in the skill file: 1 (expected 1)
skill file sha256: 6ccd9867faacc782b30fc3ef24aa2e84f7d3b03b4a41b5fc32bdc0bdabcfdbff
RESULT: byte-identical (no drift since 0044)
exit=0
```

## Corrections and stale pointers this reply flags against itself

Per the 0052 precedent, flagged rather than smoothed:

1. **`iteration`.** The task's Proof/Reply-format sections ask for `iteration: 1`, but
   the file arrived in `mailbox/inbox/` already carrying `iteration: 1`. Every other task
   in the queue arrived at 0 (0044 went inbox:0 → active:1), so the pickup increment is
   the only reading consistent with `mailbox/README.md`, which is authoritative: "Pickup:
   inbox → active … increment `iteration` by 1". Pickup (commit 145f71b) therefore set 2
   and this reply carries `iteration: 2`. The literal `iteration: 1` in the task's own
   Proof section is the stale number, not this front-matter. The Reply-format section
   itself defers to the README ("Follow mailbox/README.md: front-matter (task_id,
   protocol_version, status, iteration, proof)"), which is why the README rule governs.

2. **Byte figures for the managed block.** 0044 cites 891 bytes / md5
   `31717685dbcb00a367495465d1a1734d`; that figure counts the block **without** a trailing
   newline. With the newline it is 892 bytes / md5
   `9282926226d79b7c6b875e72881540fb`. Same bytes, two conventions — section [6] prints
   both so the two citations cannot be misread as drift. The comparison is only meaningful
   because my extraction of 0044's canonical text reproduces 0044's recorded md5 exactly.

3. **The transcript is the third execution of the demonstration, and says so.**
   - Attempt 1 died at the post-`--record` strict check on a transient DNS failure; the
     checker's ERROR path fired for real: `ERROR git fetch failed: ssh: Could not resolve
     hostname github.com: No address associated with hostname` → `PROTOCOL: ERROR …`,
     **exit=20**. Recorded here because it is the one branch of the checker the happy-path
     demonstration does not otherwise touch.
   - Attempt 2 was clean but echoed a mislabelled command (`rev-parse --short` printing a
     full SHA), so it was discarded rather than shipped with a cosmetic inaccuracy.
   - Attempt 3 (the transcript above) is unedited. Visible consequence, left in rather than
     tidied: section [2] quotes the state file as `updated_at 2026-09-25T04:16:51Z` — the
     value left by attempt 2's `--record` — while this run's own `--record` lands at
     `04:17:45Z`. Nothing in the transcript was edited or reordered; the whole script was
     re-run instead.

## What this run did not do

- No branch, no CI run, no push other than the two mailbox transition commits on main —
  per the task's Constraints.
- No edit to the PROTOCOL-MANAGED block; the comparison reads it.
- No write to `handoff/outbox/`.
- No borrowed credit: the demonstration, the byte comparison and this reply were all
  produced by this tick. The only earlier-run artifact cited is 0044's canonical block
  text, which is the comparison's input by design.
