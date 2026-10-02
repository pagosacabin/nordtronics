# Backend v1 — MQTT broker, ingest worker, HTTPS API

The "missing middle" of the wildfire pipeline: base stations publish
telemetry over MQTT/TLS to the VPS, the ingest worker validates and stores it,
and the Android companion app reads it back over HTTPS.

```
[base station] --MQTT/TLS 8883--> [Mosquitto] --subscribe--> [ingest] --write--> SQLite
                                                                                  |
                                          [Android app] <--HTTPS 443-- [nginx] <-- [api]
                                                                                (read-only)
```

Design notes and the wider system context live in
[`docs/wildfire/v1-communication-stream.md`](../docs/wildfire/v1-communication-stream.md);
the host-hardening steps this deployment assumes are in
[`docs/vps-hardening-runbook.md`](../docs/vps-hardening-runbook.md).

## Layout

| Path | What it is |
|---|---|
| `common/` | SQLite schema + connection rules, shared by both services |
| `mosquitto/` | broker config, ACL, systemd unit, certbot reload hook, test scripts |
| `ingest/` | MQTT subscriber → validation → SQLite writer (`wildfire-ingest`) |
| `api/` | FastAPI read-only endpoints (`wildfire-api`) |
| `DEPLOY.md` | exact commands to deploy to the VPS |

Both services import `common` by its top-level name; the systemd units supply
`PYTHONPATH=/opt/nordtronics/backend` and a per-service working directory, and
`conftest.py` reproduces that wiring for the tests.

## Telemetry contract

Topic: `nordtronics/wildfire/<node-id>/telemetry`, QoS 1.

```json
{
  "node_id": "node-01",
  "pm25": 12.3,
  "temperature_c": 18.5,
  "humidity_pct": 42.0,
  "battery_v": 3.92,
  "observed_utc": "2026-09-24T23:00:00Z"
}
```

- All four measurements are required and range-checked (`pm25` 0-2000 ug/m3,
  `temperature_c` -60-85 C, `humidity_pct` 0-100 %, `battery_v` 0-30 V).
  A reading outside those bounds is rejected and logged, never stored.
- `node_id` is optional in the payload but must match the topic when present.
  The topic segment is the authenticated identity, so a mismatch is treated as
  a fault rather than a rename.
- `observed_utc` is optional; `ts` and `timestamp` are accepted aliases, and
  Unix epoch seconds or milliseconds work too. It is normalised to
  `YYYY-MM-DDTHH:MM:SSZ` (fixed width, so text ordering is time ordering).
  When present it also de-duplicates QoS-1 redelivery.
- Unknown fields are ignored, so firmware can add sensors without breaking the
  backend.

Rejections are counted in memory and logged at WARNING with the reason
(`ingest.worker` — `stored`, `duplicate`, `invalid`, `ignored_topic`,
`node_mismatch`, and for events `event_stored`, `event_duplicate`,
`event_invalid`).

The worker subscribes to two topics: `nordtronics/wildfire/+/telemetry` (node
readings) and `nordtronics/wildfire/+/events` (a base station's Watch/alert
events, task 0095). They are dispatched on the topic's leaf, so a telemetry body
arriving on the events topic is rejected as an invalid event rather than stored
as a reading, and vice versa. On the events path the topic's node id is the
*publisher* (the base station, which is what the broker authenticated) while the
payload's `node_id` names the elevated node or `network` — so, unlike telemetry,
no agreement between the two is required, and no `nodes` row is created.

## Database

One SQLite file (`WILDFIRE_DB_PATH`, default
`/var/lib/nordtronics/wildfire.db`), WAL mode, schema in `common/schema.sql`:

- `nodes` — one row per node: first/last seen, reading count, latest values
- `readings` — append-only telemetry, `recorded_utc` (server) and
  `observed_utc` (node), with an index on `(node_id, recorded_utc DESC)`
- `alerts` — one row per Watch/alert event published by a base station on
  `nordtronics/wildfire/<base-id>/events` (task 0095). It stores both the wire
  fields (`event`, `nodes_json`, `window_min`, `baseline`) and contract v1's
  display fields (`type`, `severity`, `state`, `title`, `detail`);
  `alert_id` is derived from the event itself, so a QoS-1 redelivery collides
  on its UNIQUE index instead of storing the alert twice. `publisher` is the
  topic's node id (the base station); `node_id` is the elevated node, or the
  literal `network` for a multi-node alert — which is why there is no foreign
  key to `nodes`: a base station legitimately reports on nodes that have never
  sent telemetry here
- `meta` — `schema_version` (currently `2`)

The ingest worker opens it read-write; the API opens it `mode=ro` so it cannot
write telemetry even by accident.

## API

| Endpoint | Returns |
|---|---|
| `GET /healthz` | liveness + `schema_version`; 503 when the database is unreadable |
| `GET /v1/nodes` | every known node, latest reading, `age_seconds`, `status` (`ok` / `stale` / `unknown`) |
| `GET /v1/nodes/{node_id}/readings?limit=&since=` | that node's history, newest first (`limit` 1-1000, default 100) |
| `GET /v1/alerts?limit=&node_id=` | Watch/alert events, newest first (`limit` 1-500, default 100) — contract v1's alert objects, with the wire fields alongside |
| `GET /v1/nodes/{node_id}/alerts?limit=` | the alerts *involving* that node: the ones raised about it and the multi-node alerts naming it in `nodes` |

`stale` is decided by `WILDFIRE_STALE_AFTER_SECONDS` (default 900). Errors are
explicit: `404` unknown node, `400` malformed `since`, `422` out-of-range
`limit` or malformed node id.

**v1 has no authentication on the API.** It is read-only environmental
telemetry and sits behind nginx at `api.nordtronics.io`; any account or
per-node authorisation belongs in a later task, and nothing personal should be
stored until it exists.

## Running the tests

```bash
cd backend
python -m venv .venv && .venv/bin/pip install -r requirements-dev.txt
.venv/bin/python -m pytest -q
```

The API tests fill a real SQLite file through the ingest write path, so they
cover the same two-process arrangement that runs on the VPS.

### Checking the broker config

```bash
sudo bash backend/mosquitto/test-fixture.sh   # test machine ONLY: fake certs + password file
sudo MOSQUITTO_CONF=/etc/mosquitto/nordtronics.conf \
     MQTT_TEST_NODE_PASS=... MQTT_TEST_INGEST_PASS=... \
     bash backend/mosquitto/smoke-test.sh
```

`smoke-test.sh` starts the broker from the shipped config and asserts that the
TLS listener comes up, that anonymous clients are refused, that an
authenticated node's reading reaches the ingest subscriber, and that a node
cannot publish to another node's topic.

Note there is **no `mosquitto -t` flag**: the documented
validate-and-exit option is `--test-config`, and it only exists from Mosquitto
2.1 (Ubuntu 24.04 ships 2.0.18), so the script uses `--test-config` when
available and otherwise proves the config by starting the broker.

CI (`.github/workflows/backend.yml`) runs the test suite, `shellcheck`, and the
broker probe on Ubuntu 24.04 for every change under `backend/`.
