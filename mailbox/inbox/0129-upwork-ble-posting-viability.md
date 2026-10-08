---
task_id: "0129"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0129 — ESP32/BLE posting: read-only viability check

## Context

- Upwork posting ~022106710423569691901 ("Firmware with remote control" — ESP32/BLE remote-control pair, Canadian verified client, brand-new account, no rate listed). Stephen drafted a proposal at $35/hr on 2026-10-04; it sits at DRAFT, never submitted. It's now 2026-10-08, day 4.
- Stephen suspects the posting is dead. This is a READ-ONLY viability check before he decides whether to submit or drop it.

## Task

Check the posting's current state: still open or closed/expired? Has the client hired anyone or shortlisted? How many proposals are in? One-line verdict: worth submitting the DRAFT or dead — drop it.

## Success criteria

- Staged reply gives the posting status, hire/proposal counts, and the one-line verdict.
- No write action taken (state this explicitly).

## Constraints

- Read-only. Do not submit, message, bid, or modify anything on Upwork.
- Cost: use your cheapest model tier for this task. Do not use premium/pro models.

## Proof

Staged reply with the posting-state facts above; no branch or CI run needed (same pattern as 0127).

## Reply format

Stage with front-matter noting read-only MCP use. Body: posting status, numbers, verdict, explicit "no writes performed."
