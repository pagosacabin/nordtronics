-- Nordtronics wildfire backend — canonical SQLite schema (v1).
--
-- Applied by `common.db.init_db()`. Every statement is idempotent so the
-- ingest worker can safely apply it at every start.
--
-- Time columns are ISO-8601 UTC strings with a trailing `Z` (fixed width, so
-- lexicographic ordering equals chronological ordering and `>=`/`<` filters
-- work without date functions).

CREATE TABLE IF NOT EXISTS meta (
    key   TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

INSERT INTO meta (key, value) VALUES ('schema_version', '2')
    ON CONFLICT (key) DO UPDATE SET value = excluded.value;

-- One row per field node. Written by the ingest worker, read by the API.
CREATE TABLE IF NOT EXISTS nodes (
    node_id            TEXT PRIMARY KEY,
    first_seen_utc     TEXT    NOT NULL,
    last_seen_utc      TEXT    NOT NULL,
    reading_count      INTEGER NOT NULL DEFAULT 0,
    last_pm25          REAL,
    last_temperature_c REAL,
    last_humidity_pct  REAL,
    last_battery_v     REAL
);

-- Append-only telemetry. `recorded_utc` is when the server accepted the
-- message; `observed_utc` is the node's own timestamp when the base station
-- forwards one (used for duplicate suppression on QoS-1 redelivery).
CREATE TABLE IF NOT EXISTS readings (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id       TEXT NOT NULL REFERENCES nodes(node_id) ON DELETE CASCADE,
    recorded_utc  TEXT NOT NULL,
    observed_utc  TEXT,
    pm25          REAL,
    temperature_c REAL,
    humidity_pct  REAL,
    battery_v     REAL,
    topic         TEXT,
    raw_payload   TEXT
);

CREATE INDEX IF NOT EXISTS idx_readings_node_recorded
    ON readings (node_id, recorded_utc DESC);

CREATE INDEX IF NOT EXISTS idx_readings_node_observed
    ON readings (node_id, observed_utc);

CREATE INDEX IF NOT EXISTS idx_nodes_last_seen
    ON nodes (last_seen_utc DESC);

-- Watch/alert events published by a base station on
-- `nordtronics/wildfire/<base-id>/events` (task 0095). One row per event.
--
-- Two vocabularies meet in this table and both are stored, because neither can
-- be derived from the other without guessing:
--
--   * the WIRE fields (`event`, `nodes_json`, `window_min`, `baseline`) are the
--     base station's own vocabulary — `watch_raised` / `alert_cleared` / ... —
--     and are what the firmware contract (`docs/wildfire/radio-protocol-v1.md`)
--     names;
--   * the DISPLAY fields (`type`, `severity`, `state`, `title`, `detail`) are
--     contract v1's alert vocabulary (`docs/wildfire/app-api-contract-v1-...md`)
--     that `GET /v1/alerts` serves and the app renders.
--
-- `alert_id` is derived deterministically from the event itself (node, event
-- kind, observed instant), so a QoS-1 redelivery after a reconnect collides on
-- the UNIQUE index and is counted as a duplicate instead of stored twice.
-- `publisher` is the topic's node id — the base station that published it,
-- which the broker authenticated; `node_id` is the elevated node named in the
-- payload, or the literal `network` for a multi-node alert.
CREATE TABLE IF NOT EXISTS alerts (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    alert_id     TEXT NOT NULL UNIQUE,
    publisher    TEXT NOT NULL,
    node_id      TEXT NOT NULL,
    event        TEXT NOT NULL,
    type         TEXT NOT NULL,
    severity     TEXT NOT NULL,
    state        TEXT NOT NULL,
    title        TEXT NOT NULL,
    detail       TEXT NOT NULL,
    pm25         REAL,
    baseline     REAL,
    window_min   INTEGER,
    nodes_json   TEXT NOT NULL DEFAULT '[]',
    observed_utc TEXT NOT NULL,
    recorded_utc TEXT NOT NULL,
    created_utc  TEXT NOT NULL,
    updated_utc  TEXT NOT NULL,
    topic        TEXT,
    raw_payload  TEXT
);

CREATE INDEX IF NOT EXISTS idx_alerts_observed
    ON alerts (observed_utc DESC, id DESC);

CREATE INDEX IF NOT EXISTS idx_alerts_node_observed
    ON alerts (node_id, observed_utc DESC);
