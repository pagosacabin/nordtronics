---
task_id: "0142"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 12h
hold_until: "2026-10-17T08:00:00-06:00"
---

# 0142 — Retry 0139 Upwork proposal submit (Saturday morning)

## Context
Task 0139 (solar-LED LiFePO4 prototype proposal, job ~022110739515688940235,
$1,000 fixed, 15 Connects) was refused twice at the submit step on 2026-10-16 by
Upwork's location gate: "This job is restricted to freelancers in specific
locations and your location isn't eligible" (deterministic, byte-identical both
attempts; trace_ids 8d6590529e33e9811a11a80a0ead8b15 /
061d50e53a6ccc4fc8606ac19b2c58f6). Zero Connects spent, no proposal created.
Stephen wants a retry Saturday morning — the gate may be transient, or he may
have corrected account-side location/identity settings overnight. The 0139
proposal text is intact (see mailbox/archive/0139-upwork-submit-solar-led-proposal.md).

UPDATE 2026-10-16 ~14:44 MDT (Stephen's report): Upwork keeps prompting him for
ID verification on every login and every job application, even though he has
completed it twice — the site does not recognize it. This matches the 0139
staged reply's account read: identity_verification {identity_verified: false,
can_submit_proposals: true}. The "location isn't eligible" refusal is therefore
most likely a downstream symptom of the unverified-identity state, not his
actual location. The retry is only meaningful if the verification registers.

## Timing
Do NOT execute before Saturday 2026-10-17, 08:00 MDT. Picking this up Friday
defeats the purpose — the retry is predicated on overnight change.

## Task
Saturday morning after 08:00 MDT:

1. FIRST, query the account's identity verification status via the Upwork API
   (the same read that returned identity_verified: false under 0139).
   - If it still reads unverified/false: do NOT attempt the submit. Report
     the status and stop — the location wall will reproduce deterministically.
   - If it reads verified/true, or the status check is inconclusive: proceed
     to step 2.
2. Retry the proposal submit for job ~022110739515688940235 using the same flow
   as 0139 and the archived 0139 proposal text (do not rewrite the proposal).

- If the submit SUCCEEDS: report the proposal ID and Connects spent.
- If it hits the SAME location wall: make exactly ONE attempt. Do NOT retry,
  do NOT try alternate endpoints or profiles. Report the exact error text,
  error code, and trace ID.

## Constraints
- Same credential rules as 0139: the task forbids the cronrunner profile's
  Upwork token — drive the DEFAULT profile's token directly over streamable
  HTTP from /home/astroboy/.hermes/mcp-tokens/upwork.json. Never print any
  token value.
- If the flow presents NEW terms requiring acknowledgment (beyond the Dispute
  Assistance terms already acknowledged under 0139): STOP and report. Do not
  acknowledge new terms on Stephen's behalf.
- Keep model/provider cost economical; no premium tier needed.
- One submit attempt on refusal, then report. No loops.

## Success criteria
1. Either: identity still unverified → no submit attempted, status reported; or:
   proposal submitted (report proposal ID + Connects spent); or: the location
   refusal reproduced once with exact error text/code/trace ID.
2. No more than one submit attempt if refused.
3. No token values in the staged reply or logs.

## Proof
- Staged reply with the outcome and, if refused, the exact error text, error
  code, and trace ID.
- Connects balance before and after (proving nothing was spent on a refusal).

## Reply format
Per mailbox protocol: staged reply with status, outcome, proof pointers, and
notes on anything unexpected.
