---
task_id: "0053"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 6h
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
