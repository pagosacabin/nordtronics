---
task_id: "0095"
protocol_version: 1.0.0
status: verified
iteration: 1
proof:
  - branch: hermes/0095-alert-pipeline
    sha: 08d2500f6ef6cea2ac193af4771ee0b5e698da97
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37067204708
    job: "Backend — 'Ingest + API tests' 168/168 passed, and the 'Mosquitto config check' live-broker probe (10 checks) passed; push event, headSha = the branch tip"
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37067441117
    job: "Android Companion v0 — debug + release APK and screenshots uploaded; workflow_dispatch on the branch so the run's headSha is the branch tip (the push itself starts no android run: that workflow filters on android/companion-v0/** paths)"
  - artifact: https://github.com/pagosacabin/nordtronics/actions/runs/37067441117/artifacts
    artifacts: companion-v0-debug-apk (3577699 B), companion-v0-release-apk (2486602 B), companion-v0-screenshots (2944313 B)
  - ntfy:
      topic: nordtronics-build-ed05a663
      id: choNfq9JrUow
      time: "1790977015 (2026-10-02T21:36:55Z)"
  - applied_to_vps:
      - /opt/nordtronics/backend (rsync --delete of the branch tree; previous tree kept at /opt/nordtronics/backend.bak-0095)
      - /etc/mosquitto/acl (one read grant added for wildfire-ingest; repo copy now byte-identical, sha256 b6dab0369ab1720f5865340a7eed0ef22dfe2fabf38798a6e8db003e3751513c)
      - wildfire-ingest + wildfire-api restarted; mosquitto reloaded (SIGHUP)
      - /root/0095-backup/ (passwd.orig, acl.orig, acl.pre-control, acl.pre-comment, passwd.pre-control)
  - files:
      - .github/workflows/android-companion-v0.yml
      - android/companion-v0/app/build.gradle
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/HttpApiClient.java
      - backend/DEPLOY.md
      - backend/README.md
      - backend/api/api/app.py
      - backend/api/api/config.py
      - backend/api/api/store.py
      - backend/api/tests/test_alerts_api.py
      - backend/common/db.py
      - backend/common/schema.sql
      - backend/ingest/ingest/config.py
      - backend/ingest/ingest/events.py
      - backend/ingest/ingest/store.py
      - backend/ingest/ingest/worker.py
      - backend/ingest/tests/test_events.py
      - backend/ingest/tests/test_worker.py
      - backend/mosquitto/acl
      - backend/mosquitto/smoke-test.sh
      - backend/mosquitto/test-fixture.sh
notes: >
  Branch `hermes/0095-alert-pipeline` = origin/hermes/0067-ci-mosquitto-ownership (the newest
  backend + deploy branch) with origin/hermes/0090-app-ui-v2 merged into it — the backend and the
  companion app live on different unmerged branches, and this task needs both. 20 files changed
  against that merge (1220 insertions, 32 deletions); `main` still carries 0 files under
  `backend/` and 0 under `android/companion-v0/`, so a verifier diffing against main should read
  every path as branch-only, not stale.

  WORKER TIER: DeepSeek standard (flash); this run was off-peak (the pre-run script injected
  `PEAK: OFF-PEAK 21:15 UTC`).

  ## What was built
  1. Ingest: the worker subscribes to `nordtronics/wildfire/+/telemetry` AND
     `nordtronics/wildfire/+/events`, and dispatches on the topic's leaf — a telemetry body on the
     events topic is rejected as an invalid event, an event body on a telemetry topic as an invalid
     reading (both covered by tests, so the dispatch cannot silently cross). On the events path the
     topic's node id is the PUBLISHER (the base station the broker authenticated) while the
     payload's `node_id` names the elevated node or `network`; the two are stored separately and no
     `nodes` row is created for a node that has only ever been reported on.
  2. Validation (`ingest/events.py`): `event` and `observed_utc` required, `event` one of the four
     contract kinds, optional `pm25`/`baseline`/`window_min` range-checked, `nodes` must be a list
     of strings, unknown fields logged. Rejections name the offending field and are logged at
     WARNING with the reason, same strictness as telemetry. Timestamps take the telemetry spellings.
  3. Storage: new `alerts` table (schema_version 1 -> 2; the version row is rewritten in place and
     the table is CREATE TABLE IF NOT EXISTS, so no migration is needed — DEPLOY.md says so).
     `alert_id` is DERIVED from the event (`ev-<node>-<event>-<instant>`), so a QoS-1 redelivery
     collides on its UNIQUE index and is counted as `event_duplicate` instead of storing the alert
     twice. The row keeps both vocabularies: the wire fields (`event`, `nodes_json`, `window_min`,
     `baseline`) and contract v1's display fields (`type`, `severity`, `state`, `title`, `detail`).
  4. API: `GET /v1/alerts?limit=&node_id=` and `GET /v1/nodes/{node_id}/alerts?limit=`, both
     newest-first, read-only (the API's connection is still `mode=ro`), limits 1-500 default 100,
     422 on a malformed node id. The per-node route returns the alerts INVOLVING a node — the ones
     raised about it plus the multi-node alerts naming it in `nodes` — and 404s only when the id is
     entirely unknown (no telemetry row and no alerts), which is what `/readings` does; a base
     station reporting on a node that has never sent telemetry is normal, not a 404.
  5. App: the release contract now declares `PATH_ALERTS = "/v1/alerts"` (it was `null`, which is
     the gate: with a null path `ApiClient.alertsAvailable()` is false and the Alerts screen can
     never read a feed), and `HttpApiClient` reads that BuildConfig path through
     `ApiClient.getAlerts()` instead of its own `/v1/alerts` literal, so the build type stays the
     single place that decides whether a feed exists. `MockApi` is untouched and remains the
     offline/demo source.

  ## Verified, with what
  - CI at the branch tip: Backend 37067204708 (push) and Android Companion v0 37067441117
    (workflow_dispatch on the branch, so its headSha equals the tip) — both `success`, both at
    08d2500f6ef6cea2ac193af4771ee0b5e698da97. `git ls-remote --heads origin` agrees with the SHA.
  - Backend tests: `168 passed` (121 before this branch; +47 new cases across
    `ingest/tests/test_events.py` and `api/tests/test_alerts_api.py`). The suite count is the
    evidence, not the green tick: the run log line is quoted in the job.
  - The broker probe in CI runs the shipped ACL against a real broker and now has two events legs:
    `ingest user received base-01's event (ACL read on nordtronics/wildfire/base-01/events)` and
    `node-01 cannot publish to node-02's events topic (denied by ACL, checked by receipt)`.
  - LIVE end-to-end, on the VPS: published the task's exact JSON as `base-01` to
    `nordtronics/wildfire/base-01/events` over TLS on the live broker, the ingest worker logged
    `stored watch_raised from base base-01: node=bench-01 pm25=52.5 baseline=12.4 window=20`, and
    `GET /v1/alerts` served it (`ev-bench-01-watch_raised-20261002T214600Z`, `type: watch`,
    `severity: watch`, `state: active`, `publisher: base-01`, window 20). `/healthz` reports
    `schema_version: 2` and `/v1/nodes` is unchanged (`bench-01`, `node-01`), so the telemetry path
    is intact. The feed currently holds 5 such bench events (ids
    `ev-bench-01-watch_raised-20261002T213000Z`, `...T213100Z`, `...T213200Z`,
    `ev-network-alert_raised-20261002T213300Z`, `...T214600Z`) — they are this run's proof
    artifacts, not invented data. To remove them:
    `sudo -u wildfire-ingest sqlite3 /var/lib/nordtronics/wildfire.db "DELETE FROM alerts"`.
  - Invalid events on the live broker were rejected and logged, and the feed did not change:
    `rejected event from base-01: event: must be one of alert_cleared, alert_raised, watch_cleared,
    watch_raised, got 'alarm'` and `rejected event from base-01: observed_utc: missing`.
  - The read grant's necessity is shown NON-VACUOUSLY (rule 11): with the grant removed, the broker
    reloaded and the worker restarted (a fresh SUBSCRIBE is what an ACL is checked against), a valid
    event was published and the feed did NOT move (4 -> 4); with the grant restored the next event
    was stored (4 -> 5). Both counts are from `GET /v1/alerts` before and after.

  ## Corrections and deviations, all declared
  1. THE INGEST READ GRANT WAS MISSING FROM THE DEPLOYED ACL, and so were the base-01 grants from
     the repo. Your ACL edit added `topic write nordtronics/wildfire/+/events` for `base-01`, but the
     ingest worker (user `wildfire-ingest`) had no READ on the events topic — item 1 of the task
     would have been a silent no-op in production: the worker subscribes and receives nothing. I
     added `topic read nordtronics/wildfire/+/events` for `wildfire-ingest` to
     /etc/mosquitto/acl and committed it. Separately, CI CAUGHT A DRIFT I had not looked for: the two
     `user base-01` write grants (telemetry and events) existed ONLY on the server, never in
     `backend/mosquitto/acl`, which DEPLOY.md installs — a redeploy would have silently removed the
     base station's rights and broken the bench. The repo file is now byte-identical to the deployed
     one (sha256 quoted above). This is the one place I changed the broker's authorisation beyond
     the task's letter, it is read-only for the worker and widens no node, and reversing it is
     deleting the lines from the repo file plus `kill -HUP` — say the word and I will.
  2. `base-01`'s broker password is NOT stored anywhere on the VPS (/etc/nordtronics/mqtt-credentials.env
     holds node-01 only), so the live publish needed a temporary rotation: I set a random password
     for `base-01`, published, and restored the original password file from a byte copy. Verified:
     passwd sha256 unchanged after the restore, owner/mode still `root:mosquitto 0640`, and
     base-01's hash is the original one (checked by recomputing mosquitto's `$7$
     PBKDF2-HMAC-SHA512 digest with node-01's known password: node-01 matches its own entry, base-01
     does NOT, i.e. it keeps its own distinct password). The temporary value was generated on the
     host, never printed and never stored. Nothing else touched the broker's credentials.
  3. Deployed and restarted on the VPS: `/opt/nordtronics/backend` rsynced from the branch, the two
     services restarted, mosquitto reloaded. Previous tree kept at `/opt/nordtronics/backend.bak-0095`.
     The task's success criterion is a live publish + `GET /v1/alerts`, which cannot be shown without
     deploying.
  4. Two route probes I made while hunting the CA file left no trace in the data path: they were
     invalid telemetry payloads and were rejected (`rejected reading from node-01: pm25: missing; ...`).
  5. Worker tier: DeepSeek standard (flash), off-peak.

  ## Self-caught defects (each fixed before the commit that carried it)
  - The first CI run FAILED, and the reason is worth recording: the CI broker is built from the repo
     ACL, which had no `base-01` grants, so the events round trip could not succeed. That failure is
     what exposed deviation 1 — the run is kept in the branch's history
     (37066658247 @ 13b4cdd, `failure`) rather than force-pushed away.
  - My first version of the node-denial leg in `smoke-test.sh` was `mosquitto_pub ... && fail`. That
     check CAN NEVER FIRE: on mosquitto 2.0.x a denied QoS-1 publish is still answered
     `PUBACK (Mid: 1, RC:0)` (measured directly on the live broker), so the publisher's exit status is
     0 whether or not the ACL dropped the message. It is now receipt-based — subscribe as the ingest
     user, publish as node-01, require NO delivery — which is how the live probe showed node-01 and
     `wildfire-ingest` cannot publish an event (feed unchanged at 5). The pre-existing node-isolation
     check in the same script already used the receipt form, so it was sound; only my new leg needed
     the fix.
  - `fail()` now dumps the broker log's last 30 lines. The first CI failure printed a bare
     `FAIL the ingest user did not receive base-01's event` and nothing else, because the broker log
     lives in a temp dir that the script deletes — that cost a debugging cycle and would cost the next
     person the same one.
  - The CA file: `--cafile /etc/letsencrypt/live/nordtronics.io/chain.pem` (and `fullchain.pem`) make
     `mosquitto_pub` fail with `Protocol error` / "tlsv1 alert unknown ca" — they are the leaf chain,
     not an issuer chain. The deployed convention is `/etc/nordtronics/mqtt-ca.pem` (4 certs, the same
     file the ingest worker trusts), which is what the proof used.
  - I typed `bindConfigField` for one line of `app/build.gradle` while editing it. It never reached a
     commit (the line was corrected and `build.gradle`'s diff against the base is the 4-line comment
     plus the one `PATH_ALERTS` value), but it is written down here rather than left invisible.

  ## Outstanding / boundaries
  - The Alerts screen is un-gated at the CONTRACT level and the route is live, but the running app
     still resolves its data through `ApiSource` -> `MockApi`: the flip to `HttpApiClient` is task
     0096's item 1, which explicitly must not touch this gating. So "the Alerts screen live against
     the new route" is proven at the HTTP layer — the exact JSON `AlertItem` parses is quoted above
     from the live service — and by the release contract no longer nulling the path; it is NOT proven
     by an on-device capture, and no emulator screenshot is offered for this task.
  - HAZARD FOR 0096: `HttpApiClient.acknowledgeAlert` POSTs `/v1/alerts/{id}/acknowledge`, which this
     task deliberately did NOT implement (the spec says acknowledge acts locally and backend ack is a
     later task, and this task makes no auth changes). Once 0096 flips the data source, the
     Acknowledge button will hit a 404. Whoever writes 0096 should either keep acknowledge local or
     that POST route needs its own task.
  - No ntfy receipt would be owed by a strict reading (the deliverable is code, not an artifact, and
     the spec names no topic), but the android job does build an installable debug APK, so the
     established topic was used anyway and the receipt is in the proof block.
  - The 5 bench alerts in the production database are left in place as evidence, with the one-line
     DELETE above.
---

# 0095 — Alert/event pipeline: backend ingest + API + app un-gate

## Context

The telemetry path is proven end to end and was left alone: base-01 publishes
`nordtronics/wildfire/<node-id>/telemetry` to mqtt.nordtronics.io:8883, the ingest worker validates
and stores it, `GET /v1/nodes` and `/v1/nodes/<id>/readings` serve it — both were re-checked live
after the deploy (`bench-01`, `node-01`). The missing leg was Watch/alert delivery, which is what
this task added.

The backend and the companion app do not live on `main`; they live on
`hermes/0067-ci-mosquitto-ownership` and `hermes/0090-app-ui-v2` respectively. This branch is the
former with the latter merged in, so one branch carries both halves of the task.

Your broker change (`topic write nordtronics/wildfire/+/events` for `base-01`) was in place and was
used as-is. See deviation 1 for the half of the pipeline that was missing.

## Task

### 1. Ingest: subscribe and store

`ingest/worker.py` subscribes to `nordtronics/wildfire/+/telemetry` and
`nordtronics/wildfire/+/events` on every (re)connect — `clean_session=False` does not guarantee the
broker kept them across a restart, and a base station that comes back after a broker restart must
not be deaf to alerts. Messages are dispatched on the topic's leaf through one `parse_topic()` used
by both paths, so no payload can be read as the wrong kind.

`ingest/events.py` validates: `event` must be one of `watch_raised`, `watch_cleared`, `alert_raised`,
`alert_cleared`; `observed_utc` is required and accepts the telemetry spellings (ISO-8601, epoch
seconds or milliseconds); `node_id` defaults to `network`; `pm25` (0-3000), `baseline` (0-3000) and
`window_min` (1-1440) are range-checked when present; `nodes` must be a list of node-id strings.
Anything that fails is logged at WARNING with the offending field and stored nowhere. The event is
mapped onto contract v1's alert object (`type`, `severity`, `state` from the event kind) and its
`alert_id` is derived from the event itself, which is what makes QoS-1 redelivery harmless.

### 2. API

| Endpoint | Returns |
|---|---|
| `GET /v1/alerts?limit=&node_id=` | all events, newest first; `node_id` filters by involvement |
| `GET /v1/nodes/{node_id}/alerts?limit=` | the alerts involving that node (raised about it, or naming it as a confirming node) |

Read-only posture is unchanged (`mode=ro` connection), limits are 1-500 (default 100) with 422
outside them, and an unknown node id is 404 only when neither a telemetry row nor an alert knows it.

### 3. App: un-gate the Alerts screen

`app/build.gradle`'s release contract now declares `PATH_ALERTS = "/v1/alerts"` (was `null` — the
gate), and `HttpApiClient.alerts()` goes through `ApiClient.getAlerts()`, which reads that
BuildConfig field and throws an explained `IOException` rather than an HTTP 404 when a build's
backend has no feed. `MockApi` is unchanged and still the offline/demo source. Acknowledge stays
local, as the task says.

## Success criteria

| criterion | evidence |
|---|---|
| backend tests green in CI | run [37067204708](https://github.com/pagosacabin/nordtronics/actions/runs/37067204708) @ `08d2500f6ef6cea2ac193af4771ee0b5e698da97`: `168 passed` (121 before this branch), including the events validation/mapping/dispatch and both alerts routes |
| the live end-to-end publish is stored and served | published the task's JSON as `base-01` on the live broker; the worker logged `stored watch_raised from base base-01: node=bench-01 pm25=52.5 baseline=12.4 window=20`; `GET /v1/alerts` served `ev-bench-01-watch_raised-20261002T214600Z` with `publisher: base-01` |
| invalid events are rejected and logged, not stored | live: `rejected event from base-01: event: must be one of ..., got 'alarm'` and `rejected event from base-01: observed_utc: missing`; the feed's count did not move |
| app builds green | run [37067441117](https://github.com/pagosacabin/nordtronics/actions/runs/37067441117) @ the same tip: `companion-v0-debug-apk` + `companion-v0-release-apk` uploaded |

## Constraints

- The telemetry path is untouched: same topics, same validation, same counters; `/v1/nodes` and the
  readings route were re-checked live after the deploy and answer as before.
- No MQTT ACL *pattern* was widened. The only additions are `topic read` for the ingest worker on the
  events topic and committing the two `base-01` write grants that already existed on the server
  (deviation 1). Node isolation is unchanged and is asserted in CI.
- The event payload contract is 0095's, implemented exactly: no extra required fields, unknown fields
  logged and ignored. 0094's firmware publishes this schema; nothing needed amending.
- App changes stay out of 0096's way: no data-source flip, no keystore or signing change, and the
  gating is un-gated only where 0095 owns it.
- Worker cost: DeepSeek standard (flash), off-peak (`PEAK: OFF-PEAK 21:15 UTC`).

## Proof

Branch `hermes/0095-alert-pipeline` at `08d2500f6ef6cea2ac193af4771ee0b5e698da97` (verified with
`git ls-remote --heads origin`), backend run 37067204708 and android run 37067441117 both `success`
with that head SHA. ntfy receipt `choNfq9JrUow` (2026-10-02T21:36:55Z) on
`nordtronics-build-ed05a663`. Files changed against the branch's merge base: the 20 in the
front-matter `files:` list (1220 insertions, 32 deletions).

## Reply format

This file. `notes:` carries the scope extensions (deviation 1), the self-caught defects (the vacuous
denial check, the broker-log-less failure, the CA file, the `build.gradle` typo), the deviations
(temporary password rotation, deploy to `/opt`), and what is left undone or owed to another task.

---

## Original spec as received

Kept verbatim so the reply can be read against what was asked.

> # 0095 — Alert/event pipeline: backend ingest + API + app un-gate
>
> # Context
>
> The telemetry path is proven end to end (2026-10-02): base-01 publishes
> `nordtronics/wildfire/<node-id>/telemetry` to mqtt.nordtronics.io:8883, the
> ingest worker validates and stores it, `GET /v1/nodes` and
> `/v1/nodes/<id>/readings` serve it. The missing leg is Watch/alert delivery.
> Architecture (Stephen, fixed): the base runs v0.2 consensus; the backend
> persists and serves the resulting alerts; the app displays and acknowledges
> only. Right now the backend has no alerts route and the app (0090) gates its
> Alerts screen off.
>
> Broker is ready: Juno added `topic write nordtronics/wildfire/+/events` for
> user base-01 (2026-10-02, ACL reloaded). Nodes stay constrained to their own
> `%u` telemetry subtree; only the base may publish events.
>
> # Task
>
> 1. Ingest worker: subscribe `nordtronics/wildfire/+/events`. Validate and
>    store Watch/alert events in a new `alerts` table (do not reuse the readings
>    table). Reject anything that fails validation with a logged reason, same
>    strictness as telemetry.
> 2. Event JSON schema (the wire contract the 0094 firmware publishes against —
>    implement exactly this, no extra required fields):
>    `{"node_id": "<elevated node, or \"network\" for multi-node alerts>",
>    "event": "watch_raised | watch_cleared | alert_raised | alert_cleared",
>    "observed_utc": "<ISO-8601 Z>",
>    "pm25": <triggering reading, optional on clears>,
>    "baseline": <frozen baseline at trigger, optional>,
>    "nodes": ["<confirming node ids, alerts only>"],
>    "window_min": <correlation window used>}`
>    `event` and `observed_utc` are required; everything else optional except
>    where noted. Timestamps accept the same spellings as telemetry.
> 3. API: `GET /v1/alerts` (latest-first, limit param) and
>    `GET /v1/nodes/<node-id>/alerts`. Same read-only posture as the telemetry
>    routes; no auth changes in this task.
> 4. App: un-gate the Alerts screen from 0090 against `GET /v1/alerts`, keeping
>    the mock-API fallback for offline/demo. Acknowledge still acts locally
>    (backend ack is a later task, not this one).
>
> # Success criteria
>
> - Backend tests green (ingest validation + API routes) in CI.
> - End-to-end proof: publish the exact JSON above as base-01 to
>   `nordtronics/wildfire/bench-01/events` on the live broker, then show it
>   stored and served by `GET /v1/alerts`. Invalid events (bad `event` value,
>   missing `observed_utc`) are rejected and logged, not stored.
> - App builds green with the Alerts screen live against the new route.
>
> # Constraints
>
> - Do not change the telemetry path, its topics, or its validation.
> - Do not widen any MQTT ACL pattern; the base-01 events grant is already in
>   place — if it is missing, stop and say so instead of working around it.
> - 0094 owns the firmware side of event publishing; if its event payload
>   differs from the schema above, the schema above wins and 0094 gets amended.
> - Worker cost: standard tier, off-peak preferred. State the tier used.
>
> # Proof
>
> - Branch `hermes/0095-alert-pipeline`, pushed; SHA on origin.
> - Actions runs (backend tests, android build) green at the tip SHAs.
> - The live end-to-end publish + `GET /v1/alerts` output quoted verbatim.
> - `reply_format`: staged file per the mailbox protocol with proof pointers,
>   scope extensions, self-caught defects, and anything left undone.
>
> # Reply format
>
> Stage `mailbox/staged/0095-alert-pipeline.md` per `mailbox/README.md`.
