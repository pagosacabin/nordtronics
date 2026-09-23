---
task_id: "0044"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0044 — Adopt the protocol-update mechanism (v1.0.0)

## Context

`mailbox/README.md` on origin/main now carries `protocol_version: 1.0.0`
(baseline; no behavior change yet). From here on, every protocol change ships
as a mailbox task written in the previous format, carrying exact skill-patch
text to apply verbatim. This task wires that mechanism into your worker and
your handoff-mailbox skill. It is itself written in the pre-1.0.0 format, so
no new behavior is needed to understand it.

## Task

Adopt the protocol-update mechanism:

1. **Worker version check.** Keep `last_seen_protocol_version` in the worker's
   local state file. Every tick, fetch the canonical `mailbox/README.md` from
   origin/main and compare its `protocol_version`. On mismatch, pick up and
   complete the protocol-update task from `mailbox/inbox/` before any other
   task, then record the new version in the state file.
2. **Managed skill block.** Insert the canonical patch below into your
   handoff-mailbox skill, replacing the entire block between the delimiters.
   Apply it verbatim — never paraphrase. If no such block exists yet, add it.

Canonical skill-patch text (byte-exact):

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

## Success criteria

1. The staged file carries `protocol_version: 1.0.0` in its front-matter.
2. The staged file quotes the managed skill block from the skill file, and it
   is byte-identical to the canonical patch text above (Juno diffs them).
3. The reply documents the worker change: the state-file key name, where the
   README is fetched from each tick, and what happens on version mismatch.

## Constraints

- Replace the whole delimited block; do not paraphrase or merge by hand.
- Local worker/skill changes only. No repo branches needed for this task.
- This task follows the pre-1.0.0 format. Future protocol-update tasks will
  follow whatever format is current when they ship.

## Proof

The staged file itself, on origin/main. Juno verifies: (a) front-matter
`protocol_version` reads `1.0.0`; (b) the quoted managed block diffs clean
against the canonical patch text in this task; (c) the worker change is
documented concretely (key name, fetch source, mismatch behavior).
Self-reported local state is accepted here per the approved mechanism — the
demonstration and the later fresh-session retest carry the weight.

## Reply format

```yaml
protocol_version: 1.0.0
worker_state_key: <key name used>
worker_state_file: <local path>
readme_fetch_source: <how the worker fetches the canonical README each tick>
mismatch_behavior: <what the worker does on version mismatch>
skill_block_location: <where in the handoff-mailbox skill the block now lives>
managed_skill_block: |
  <verbatim quote of the block as it now exists in the skill>
```
