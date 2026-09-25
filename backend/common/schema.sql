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

INSERT INTO meta (key, value) VALUES ('schema_version', '1')
    ON CONFLICT (key) DO NOTHING;

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
