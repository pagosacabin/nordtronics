# 0088 — Connect Hermes to Upwork MCP (read-only check)

expect-reply-within: 6h

# Context

0087 confirmed your MCP client handles remote HTTP servers with OAuth 2.1 +
dynamic client registration: `hermes mcp add upwork --url
https://mcp.upwork.com/mcp --auth oauth`. Stephen has approved connecting the
official Upwork MCP server. This task connects it and does a **read-only**
first look — nothing else.

Stephen's standing rule, unchanged: leads only, nothing submitted. No
proposal, message, offer, or Connects spend happens without his explicit
per-item approval. That is not granted here.

One record-correction for your archive, unrelated to Upwork: in 0086 the
backup passphrase file's final disposition is **shredded** (`shred -u -z
-n 3`), not "kept at 0600" as the archived text says — Juno archived before
the shred. Your staged reply for this task carries the one-line correction
(see success criteria).

# Task

1. Add the Upwork MCP server: `hermes mcp add upwork --url
   https://mcp.upwork.com/mcp --auth oauth`. Stephen is at the laptop and
   will do the browser consent when it opens; enable the discovered tools
   when prompted. If the consent step stalls because Stephen isn't there,
   stage a "blocked at consent" status instead of waiting silently.
2. Once connected, enumerate the server's tools (names + count).
3. Read-only lookups only: Stephen's Upwork profile basics (name, title,
   hourly rate, availability) and the status of his five proposals submitted
   2026-09-14 (job title, client, date, current status, any views or
   replies).
4. Report what scopes the consent screen requested (from the flow itself,
   not the docs) so Stephen can see exactly what he granted.
5. Include the 0086 one-line correction from Context above.

# Success criteria

1. `hermes mcp list` shows `upwork` enabled, backed by the staged reply.
2. Reply enumerates the Upwork tools (names + count) and states which are
   read vs write by their descriptions.
3. Reply lists each of the five 2026-09-14 proposals with its current
   status, or states plainly that the server exposes no proposal-status
   tool.
4. Reply states the scopes actually granted at consent.
5. Reply contains the one-line 0086 final-disposition correction.

# Constraints

- Read-only after connect. Do NOT submit a proposal, spend Connects, send a
  message, or accept/decline anything. Enabled tools ≠ authorized tools;
  any write action needs Stephen's explicit approval in a future task.
- Never print token contents or secrets; report the token store location
  only.
- If OAuth metadata discovery fails (e.g. "Registration failed: 404"),
  report the exact error and stop — no manual registration, no workaround.
- If the consent screen shows anything unexpected (payment, financial),
  stop before approving and report what it shows.
- Keep it cheap: DeepSeek Flash, minimal reasoning.

# Proof

The staged reply itself: `hermes mcp list` output, the tool enumeration, the
proposal statuses, the granted scopes. No branch, no code — config change
plus report. Verify what you claim against live server responses, not docs.

# Reply format

```yaml
connected: "<yes/no — hermes mcp list line for upwork>"
tools:
  - "<tool name — read|write>"
proposal_status:
  - "<submitted 2026-09-14 — job title — client — status — views/replies>"
scopes_granted: "<scopes shown at consent>"
token_store: "<location, not contents>"
correction_0086: "<one-line final disposition of the passphrase file>"
notes: "<anything Stephen should know>"
```
