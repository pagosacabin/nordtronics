---
task_id: "0095"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 24h
---

# 0095 — Alert/event pipeline: backend ingest + API + app un-gate

# Context

The telemetry path is proven end to end (2026-10-02): base-01 publishes
`nordtronics/wildfire/<node-id>/telemetry` to mqtt.nordtronics.io:8883, the
ingest worker validates and stores it, `GET /v1/nodes` and
`/v1/nodes/<id>/readings` serve it. The missing leg is Watch/alert delivery.
Architecture (Stephen, fixed): the base runs v0.2 consensus; the backend
persists and serves the resulting alerts; the app displays and acknowledges
only. Right now the backend has no alerts route and the app (0090) gates its
Alerts screen off.

Broker is ready: Juno added `topic write nordtronics/wildfire/+/events` for
user base-01 (2026-10-02, ACL reloaded). Nodes stay constrained to their own
`%u` telemetry subtree; only the base may publish events.

# Task

1. Ingest worker: subscribe `nordtronics/wildfire/+/events`. Validate and
   store Watch/alert events in a new `alerts` table (do not reuse the readings
   table). Reject anything that fails validation with a logged reason, same
   strictness as telemetry.
2. Event JSON schema (the wire contract the 0094 firmware publishes against —
   implement exactly this, no extra required fields):
   `{"node_id": "<elevated node, or \"network\" for multi-node alerts>",
   "event": "watch_raised | watch_cleared | alert_raised | alert_cleared",
   "observed_utc": "<ISO-8601 Z>",
   "pm25": <triggering reading, optional on clears>,
   "baseline": <frozen baseline at trigger, optional>,
   "nodes": ["<confirming node ids, alerts only>"],
   "window_min": <correlation window used>}`
   `event` and `observed_utc` are required; everything else optional except
   where noted. Timestamps accept the same spellings as telemetry.
3. API: `GET /v1/alerts` (latest-first, limit param) and
   `GET /v1/nodes/<node-id>/alerts`. Same read-only posture as the telemetry
   routes; no auth changes in this task.
4. App: un-gate the Alerts screen from 0090 against `GET /v1/alerts`, keeping
   the mock-API fallback for offline/demo. Acknowledge still acts locally
   (backend ack is a later task, not this one).

# Success criteria

- Backend tests green (ingest validation + API routes) in CI.
- End-to-end proof: publish the exact JSON above as base-01 to
  `nordtronics/wildfire/bench-01/events` on the live broker, then show it
  stored and served by `GET /v1/alerts`. Invalid events (bad `event` value,
  missing `observed_utc`) are rejected and logged, not stored.
- App builds green with the Alerts screen live against the new route.

# Constraints

- Do not change the telemetry path, its topics, or its validation.
- Do not widen any MQTT ACL pattern; the base-01 events grant is already in
  place — if it is missing, stop and say so instead of working around it.
- 0094 owns the firmware side of event publishing; if its event payload
  differs from the schema above, the schema above wins and 0094 gets amended.
- Worker cost: standard tier, off-peak preferred. State the tier used.

# Proof

- Branch `hermes/0095-alert-pipeline`, pushed; SHA on origin.
- Actions runs (backend tests, android build) green at the tip SHAs.
- The live end-to-end publish + `GET /v1/alerts` output quoted verbatim.
- `reply_format`: staged file per the mailbox protocol with proof pointers,
  scope extensions, self-caught defects, and anything left undone.

# Reply format

Stage `mailbox/staged/0095-alert-pipeline.md` per `mailbox/README.md`.
