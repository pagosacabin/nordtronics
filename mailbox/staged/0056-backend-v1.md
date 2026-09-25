---
task_id: "0056"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0056-backend-v1
    sha: 697ab6647ee3faba3114d111d4a51f93c8a29d9d
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36098623247
  - ntfy: nordtronics-build-ed05a663 @ 2026-09-25T05:28:15Z (id qdZCYoBAssvK)
  - artifacts: https://github.com/pagosacabin/nordtronics/actions/runs/36098623247/artifacts
    # backend-test-report, mosquitto-config-check — both downloaded and read
  - files:
      - .github/workflows/backend.yml
      - .gitignore
      - backend/DEPLOY.md
      - backend/README.md
      - backend/api/api/__init__.py
      - backend/api/api/app.py
      - backend/api/api/config.py
      - backend/api/api/store.py
      - backend/api/requirements.txt
      - backend/api/tests/test_api.py
      - backend/api/wildfire-api.service
      - backend/common/__init__.py
      - backend/common/db.py
      - backend/common/schema.sql
      - backend/conftest.py
      - backend/ingest/ingest/__init__.py
      - backend/ingest/ingest/config.py
      - backend/ingest/ingest/store.py
      - backend/ingest/ingest/validation.py
      - backend/ingest/ingest/worker.py
      - backend/ingest/requirements.txt
      - backend/ingest/tests/test_config.py
      - backend/ingest/tests/test_store.py
      - backend/ingest/tests/test_validation.py
      - backend/ingest/tests/test_worker.py
      - backend/ingest/wildfire-ingest.service
      - backend/mosquitto/acl
      - backend/mosquitto/mosquitto.conf
      - backend/mosquitto/mosquitto.service
      - backend/mosquitto/renew-hook.sh
      - backend/mosquitto/smoke-test.sh
      - backend/mosquitto/test-fixture.sh
      - backend/requirements-dev.txt
notes: |
  Backend v1 delivered as a new tree under backend/ on branch
  hermes/0056-backend-v1. Nothing was deployed to the VPS — this task was the
  repo tree — and the Android app, the website and DNS were not touched.

  WHAT WAS BUILT
  - backend/mosquitto/: TLS-only listener on 8883 (the port UFW already
    opens) using the existing /etc/letsencrypt/live/nordtronics.io cert paths,
    password-file auth with allow_anonymous false, and a default-deny ACL that
    scopes each base station to read/write only
    nordtronics/wildfire/<its own username>/telemetry. Plus a hardened systemd
    unit (a drop-in override of the packaged mosquitto.service), a certbot
    deploy hook that reloads the broker on renewal, and smoke-test.sh.
  - backend/ingest/: paho-mqtt subscriber on
    nordtronics/wildfire/+/telemetry (QoS 1). Validates every payload — all
    four measurements required and range-checked, no NaN/inf, no booleans, and
    a payload node_id that disagrees with the topic is rejected — then
    de-duplicates QoS-1 redelivery on the node's own observed_utc and writes
    the node summary and the reading in ONE transaction. Service user
    wildfire-ingest, own venv entry point, no root anywhere.
  - backend/api/: FastAPI, GET /v1/nodes and GET /v1/nodes/{id}/readings
    (?limit= &since=) plus /healthz, reading the same SQLite file through a
    connection opened mode=ro so it cannot write telemetry even by accident.
    Service user wildfire-api, binds 127.0.0.1:8000 for nginx on
    api.nordtronics.io.
  - backend/common/: the schema and connection rules both services share
    (schema.sql + db.py). Added beyond the four named items so the schema
    exists once instead of twice; the units put backend/ on PYTHONPATH exactly
    as the tests do.
  - backend/DEPLOY.md: the runbook — packages, service users and the shared
    wildfire-data group, tree + venv, broker config/ACL/cert access,
    mosquitto_passwd users, the ingest env file, systemd units, the nginx
    block, end-to-end verification, adding a node, updating, rollback, and a
    symptom/cause troubleshooting table.
  - No secrets in the tree: the broker password file and
    /etc/nordtronics/ingest.env are created at deploy time. The only
    credential under backend/ is the obviously-fake CI fixture constant.

  VERIFICATION — every pointer above was read, not assumed
  - 118 pytest tests pass (ingest validation/store/worker/config and API). The
    exact CI command was also run in an ubuntu:24.04 container before the
    branch was pushed, so the workflow was known-good first; and after the run
    I downloaded the backend-test-report artifact and read it: "118 passed".
  - The broker config check is a live probe, not a parse: CI installs
    mosquitto 2.0.18 from Ubuntu 24.04, installs
    backend/mosquitto/mosquitto.conf the way DEPLOY.md does, fabricates the
    deploy-time files (a self-signed stand-in cert with the same permissions
    certbot uses, a password file, the ACL), starts the broker and asserts.
    The mosquitto-config-check artifact, read back:
      ok   broker is listening on 127.0.0.1:8883 over TLS
      ok   anonymous publisher is refused
      ok   anonymous subscriber is refused
      ok   ingest user received node-01's telemetry
      ok   node-01 cannot publish to node-02's topic
    The certificate-access step is covered implicitly: the fixture leaves
    privkey.pem root-only 0600 exactly as certbot does, and the broker can
    only read it because of the setfacl line in DEPLOY.md step 4. The cert
    itself is a CI stand-in, not the real Let's Encrypt one.
  - shellcheck on the three shell scripts, systemd-analyze verify on the three
    units, and python -m compileall all gate the same job.
  - Per-step conclusions read from the API (not just the run conclusion):
    both jobs' steps all "success".

  DEVIATIONS, with reasons
  - "mosquitto -t -c <conf>" does not exist. The broker accepts only
    -c/-d/-h/-p/-v (checked against `mosquitto -h` on 2.0.18 and the man page);
    the documented validate-and-exit flag is --test-config and it only exists
    from mosquitto 2.1. Ubuntu 24.04 — and therefore the CI runner — ships
    2.0.18, so CI uses --test-config when the installed build has it and
    otherwise proves the config by starting the broker and asserting TLS,
    anonymous refusal, an authenticated round trip and ACL isolation. That is
    stronger than the flag would have been, not a weaker substitute.
  - Added an ACL, message_size_limit, max_connections and tls_version, none of
    which the spec asked for: "no anonymous access" is only half of least
    privilege without topic scoping, and the extras are one line each.
  - CI triggers on paths only (backend/**), deliberately with no branches
    filter: a pinned branch list silently skips the next hermes/NNNN-* backend
    branch, and a silent no-run leaves nothing to cite.
  - The API has no authentication in v1. The task specified two read-only
    endpoints and no auth; backend/README.md states the gap plainly so the
    app's expectations stay honest.

  CORRECTION MADE DURING THE WORK
  - The first cut inserted the reading before the node row, and the tests
    caught it immediately: readings.node_id is a foreign key and foreign_keys
    is ON, so a node's very first message was rejected. The store now upserts
    the node first inside the same transaction, and a failure mid-write leaves
    neither row behind (test_a_failure_mid_write_leaves_nothing_behind).

  NOT DONE — out of scope, for a later task
  - Nothing runs on the VPS yet: no service users created, no files installed,
    no unit enabled. DEPLOY.md is the handover for that step.
  - No alert engine, no push notifications, and no schema-migration path
    (adding a column is safe via CREATE TABLE IF NOT EXISTS; changing one is
    not).

  WORKTREE NOTE: an untracked take_screenshots.py left in the main checkout by
  an earlier run is untouched and is not part of this branch.
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
