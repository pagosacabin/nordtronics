---
task_id: "0116"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 6h
---

# 0116 — Upwork MCP in the cronrunner profile: investigate, do not change

## Context

0114 verified Upwork MCP read-only, but only in the DEFAULT profile
(/home/astroboy/.hermes): `hermes mcp list` there shows `upwork`
(https://mcp.upwork.com/mcp) enabled, 32/32 tools. The cron worker's own
`cronrunner` profile does NOT carry the server — its `mcp list` shows only
kicad and freecad, so cron ticks have no `mcp__upwork__*` tools. Stephen's
interactive OAuth (2026-10-06 ~07:53 MDT) left cached tokens at
/home/astroboy/.hermes/mcp-tokens/upwork.json (+ .client.json + .meta.json).
Stephen asked whether cron can be made to work. This task answers that
question. It makes no changes.

## Task

Investigate READ-ONLY how Hermes profiles are laid out and write up the exact,
minimal, REVERSIBLE move that would expose the already-authorized upwork MCP
server to the cronrunner profile, reusing Stephen's cached OAuth tokens. No new
interactive consent is available for this (Stephen is not at a browser) — the
write-up must either reuse the cached tokens or state plainly that fresh
consent is required and why.

## Success criteria

1. Profile layout documented: filesystem paths for the DEFAULT profile vs the
   cronrunner profile — config file(s), the `mcp_servers` section, the token
   directory — with pasted (redacted) listings.
2. The proposed move written out step by step (files to copy/symlink, config
   edits, commands), each step reversible, with a rollback note.
3. Risks stated plainly: token sharing across profiles, what background tasks
   could then do with Upwork access, anything that could break the interactive
   profile.
4. ZERO changes made to either profile. Config files read, not written.

## Constraints

- READ-ONLY investigation. Do not modify any profile config, do not copy any
  token file, do not run `hermes mcp add` / `login` / `test` under cronrunner.
  (`hermes mcp list` under cronrunner to confirm current state is fine.)
- Never print token, secret, or credential VALUES. Paths, filenames, key names,
  and structure only.
- Keep model cost on DeepSeek Flash. State the tier in the reply.
- If the answer is "cannot be done without fresh interactive consent," that IS
  an acceptable result — report it with the reason, do not work around it.

## Proof

- Pasted redacted listings showing both profiles' config/token layout.
- The step-by-step move write-up itself, in the reply notes.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, the verdict first ("possible without fresh consent: yes/no"),
then the layout evidence and the proposed move. No `proof` pointers to
branches — there are none; the write-up is the deliverable.
