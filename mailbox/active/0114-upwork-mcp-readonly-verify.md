---
task_id: "0114"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0114 — Upwork MCP read-only verification

## Context

Task 0088 (connect Hermes to the Upwork MCP server) staged as blocked-at-consent:
interactive OAuth could not complete from a non-TTY cron tick. On 2026-10-06
~07:53 MDT Stephen completed the consent himself on his Linux laptop:
`hermes mcp add upwork --url https://mcp.upwork.com/mcp --auth oauth` finished
the browser approval, and `hermes mcp list` now shows `upwork`
(https://mcp.upwork.com/mcp) enabled with 32/32 tools, saved to
`~/.hermes/config.yaml`. The consent step is done. What remains of 0088's
success criteria is proving a read-only MCP call actually works.

## Task

Run exactly ONE read-only Upwork MCP tool call and report real returned data.
Suggested call: `upwork_get_profile` (views Stephen's own freelancer profile).
`upwork_find_jobs` with a single keyword is an acceptable alternative. Pick one,
run it once, report what came back.

## Success criteria

1. `hermes mcp list` shows `upwork` as enabled.
2. The single read-only tool call returns real data (not an auth error, not an
   empty stub).
3. Zero write operations performed.

## Constraints

- READ-ONLY. Do not submit, confirm, preview, edit, or withdraw any proposal.
  Do not send any message. Do not edit the profile. Do not save/favorite jobs.
- One tool call total. Do not go exploring the other 31 tools.
- Redact any tokens, secrets, or credential values from pasted output. Summarize
  rather than dumping full payloads where the payload is large.
- If the call fails with an auth error, report the exact error and stop — do not
  retry in a loop, do not re-run the OAuth flow.

## Proof

- Pasted `hermes mcp list` output showing `upwork` enabled.
- Pasted result of the single read-only call (redacted as above).
- Pasted terminal output is required; a summary sentence alone is not proof.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter with
`status:`, a short summary of what the call returned, and a `proof` block
containing the pasted outputs above. Mark unmet criteria honestly if the call
fails.
