---
task_id: "0056"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0056 — Backend v1: Mosquitto + ingest worker + HTTPS API (repo build)

## Context

The wildfire pipeline's missing middle. The Android app (0049,
`hermes/0049-companion-v01-ui`) is built with the API URL centralized in
`BuildConfig.API_BASE_URL` and currently hits mock data. The VPS
(89.117.21.105) is hardened: `deploy` user with passwordless sudo, UFW allows
22/80/443/8883 only, nginx serves nordtronics.io, Let's Encrypt cert covers
apex/www/api/mqtt. Sensors arrive in ~5-8 business days. Standing principles:
laptop builds, VPS serves, GitHub is source of truth. Each service runs under
its own service user — never root, never `deploy`.

## Task

On a new branch `hermes/0056-backend-v1`, build the v1 backend tree under
`backend/`:

1. **Mosquitto** — `backend/mosquitto/mosquitto.conf`: TLS on 8883 using the
   existing Let's Encrypt cert paths, password-file auth, no anonymous
   access. Include a systemd unit for a dedicated `mosquitto` service user.
2. **Ingest worker** — `backend/ingest/`: subscribes to
   `nordtronics/wildfire/<node-id>/telemetry`, validates payloads
   (PM2.5, temperature, humidity, battery voltage), writes to SQLite.
   Runs as its own service user (`wildfire-ingest`). Include unit tests.
3. **HTTPS API** — `backend/api/`: `GET /v1/nodes` and
   `GET /v1/nodes/{id}/readings`, reading from the same SQLite DB. Runs as
   its own service user (`wildfire-api`), designed to sit behind nginx on
   api.nordtronics.io. Include tests.
4. **Deploy runbook** — `backend/DEPLOY.md`: exact commands the `deploy`
   user runs on the VPS — create service users, install files, enable units,
   nginx snippet for the API. Copy-pasteable; no secrets in the repo
   (password files are created at deploy time).

Decisions already made, do not relitigate: SQLite for v1 telemetry (single
file, no extra service — right-sized for 2-3 nodes); topic namespace
`nordtronics/wildfire/<node-id>/telemetry`; Python for ingest and API.

## Success criteria

1. `mosquitto -t -c backend/mosquitto/mosquitto.conf` passes — run it in CI
   if mosquitto is available there, otherwise locally with the command and
   exit code recorded in the reply.
2. Ingest worker and API unit tests pass; CI is green on the branch.
3. Branch `hermes/0056-backend-v1` exists on origin; the green CI run is at
   its exact tip SHA.
4. `DEPLOY.md` is complete enough that a second person could deploy without
   asking questions.

## Constraints

- Cost bound: this task runs on deepseek-flash only. Prefer off-peak hours
  (22:00-24:00 and 00:00-04:00 MDT weekdays) for heavy model use.
- No secrets in the repo: no passwords, keys, or tokens anywhere under
  `backend/`.
- Nothing runs as root or `deploy` — one service user per service.
- Do not touch the Android app, the website, or DNS in this task.
- One deliverable: the backend tree on the branch.

## Proof

- Origin branch `hermes/0056-backend-v1` with full SHA.
- Green GitHub Actions run at that exact SHA (plus ntfy build-green receipt
  per the established pattern).
- These exist at the branch tip: `backend/mosquitto/mosquitto.conf`,
  `backend/ingest/`, `backend/api/`, `backend/DEPLOY.md`.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: what was built, test/config-check results, branch SHA and run
URL, and any deviations from the spec with reasons.
