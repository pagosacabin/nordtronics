---
task_id: 0035
status: inbox
iteration: 0
proof: []
notes: ""
---

# 0035 — Prove Home Assistant access, list the light entities

Stephen says you can now reach the Home Assistant server (your 0020 diagnosis was correct: your host's scanner was blocking raw-IP HTTP URLs). The token is in the `HASS_TOKEN` env var on your laptop. Before we build the 6:00 AM entry-lights automation, I need the exact light entity IDs — from the live API, not from memory.

## Task

Run ONE authenticated read-only request against the Home Assistant HTTP API:

`GET /api/states`

From the response, extract every entity whose `entity_id` starts with `light.` and report each one's `entity_id` plus its `friendly_name` attribute.

## Success criteria

1. The request authenticated with the real `HASS_TOKEN` env var — no pasted tokens, no new tokens created.
2. Your reply lists every `light.*` entity_id with its friendly name, copied from the actual API response body.
3. Nothing was created, changed, or toggled in Home Assistant — strictly read-only.

## Constraints

- Read-only: `GET /api/states` only. No POST, no service calls, no state changes.
- Never print, log, or paste the token value. If you get a 401, stop and report `blocked` with the exact HTTP status — do not retry with a different credential.
- If the host still blocks the URL, report `blocked` with the exact error text.

## Reply format

Move this file to `mailbox/staged/` with `status: staged` (per the mailbox README — the staged file IS the reply). Include:

- Status: done | blocked
- The exact curl command used, with the token redacted as `$HASS_TOKEN`
- The exact HTTP status code returned
- The full list of `light.*` entity_ids with friendly names (or the exact error text if blocked)

expect-reply-within: 6h
