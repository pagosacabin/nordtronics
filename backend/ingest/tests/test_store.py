"""Write-path tests: one reading in, one row out, no half-written readings."""

from __future__ import annotations

import sqlite3

import pytest

from common import db as db_module
from ingest.store import record_reading

READING = {
    "pm25": 12.3,
    "temperature_c": 18.5,
    "humidity_pct": 42.0,
    "battery_v": 3.92,
    "observed_utc": "2026-09-24T23:00:00Z",
}


@pytest.fixture()
def conn(tmp_path):
    connection = db_module.init_db(tmp_path / "wildfire.db")
    yield connection
    connection.close()


def count(conn, table) -> int:
    return conn.execute(f"SELECT COUNT(*) AS n FROM {table}").fetchone()["n"]


def test_schema_is_created_with_version(conn):
    tables = {
        row["name"] for row in conn.execute("SELECT name FROM sqlite_master WHERE type='table'")
    }
    assert {"nodes", "readings", "meta"} <= tables
    assert db_module.schema_version(conn) == db_module.SCHEMA_VERSION


def test_init_db_is_idempotent(tmp_path):
    path = tmp_path / "wildfire.db"
    first = db_module.init_db(path)
    record_reading(first, node_id="node-01", reading=dict(READING))
    first.close()
    second = db_module.init_db(path)
    assert db_module.schema_version(second) == db_module.SCHEMA_VERSION
    assert count(second, "readings") == 1
    second.close()


def test_first_reading_creates_node_and_reading(conn):
    outcome = record_reading(
        conn,
        node_id="node-01",
        reading=dict(READING),
        topic="nordtronics/wildfire/node-01/telemetry",
        raw_payload="{}",
        recorded_utc="2026-09-24T23:00:05Z",
    )
    assert outcome == "stored"
    node = conn.execute("SELECT * FROM nodes WHERE node_id = 'node-01'").fetchone()
    assert node["first_seen_utc"] == "2026-09-24T23:00:05Z"
    assert node["last_seen_utc"] == "2026-09-24T23:00:05Z"
    assert node["reading_count"] == 1
    assert node["last_pm25"] == 12.3
    assert node["last_battery_v"] == 3.92

    reading = conn.execute("SELECT * FROM readings").fetchone()
    assert reading["node_id"] == "node-01"
    assert reading["observed_utc"] == "2026-09-24T23:00:00Z"
    assert reading["recorded_utc"] == "2026-09-24T23:00:05Z"
    assert reading["pm25"] == 12.3
    assert reading["topic"] == "nordtronics/wildfire/node-01/telemetry"


def test_second_reading_updates_the_node_summary(conn):
    record_reading(conn, node_id="node-01", reading=dict(READING), recorded_utc="2026-09-24T23:00:05Z")
    second = dict(READING, pm25=99.9, observed_utc="2026-09-24T23:05:00Z")
    record_reading(conn, node_id="node-01", reading=second, recorded_utc="2026-09-24T23:05:05Z")

    node = conn.execute("SELECT * FROM nodes WHERE node_id = 'node-01'").fetchone()
    assert node["reading_count"] == 2
    assert node["first_seen_utc"] == "2026-09-24T23:00:05Z"
    assert node["last_seen_utc"] == "2026-09-24T23:05:05Z"
    assert node["last_pm25"] == 99.9
    assert count(conn, "readings") == 2


def test_repeated_sample_is_suppressed(conn):
    assert record_reading(conn, node_id="node-01", reading=dict(READING)) == "stored"
    assert record_reading(conn, node_id="node-01", reading=dict(READING)) == "duplicate"
    assert count(conn, "readings") == 1
    assert conn.execute("SELECT reading_count FROM nodes").fetchone()["reading_count"] == 1


def test_same_timestamp_from_two_nodes_is_kept(conn):
    record_reading(conn, node_id="node-01", reading=dict(READING))
    record_reading(conn, node_id="node-02", reading=dict(READING))
    assert count(conn, "readings") == 2
    assert count(conn, "nodes") == 2


def test_readings_without_a_timestamp_are_never_deduplicated(conn):
    untimed = dict(READING, observed_utc=None)
    assert record_reading(conn, node_id="node-01", reading=dict(untimed)) == "stored"
    assert record_reading(conn, node_id="node-01", reading=dict(untimed)) == "stored"
    assert count(conn, "readings") == 2


def test_nodes_are_ordered_and_readings_indexed(conn):
    indexes = {
        row["name"] for row in conn.execute("SELECT name FROM sqlite_master WHERE type='index'")
    }
    assert {"idx_readings_node_recorded", "idx_readings_node_observed", "idx_nodes_last_seen"} <= indexes


class _FailingConnection:
    """Delegates to a real connection but raises on the Nth execute()."""

    def __init__(self, real: sqlite3.Connection, fail_at: int):
        self._real = real
        self._fail_at = fail_at
        self._calls = 0

    def execute(self, *args, **kwargs):
        self._calls += 1
        if self._calls >= self._fail_at:
            raise sqlite3.OperationalError("simulated failure")
        return self._real.execute(*args, **kwargs)

    def __enter__(self):
        self._real.__enter__()
        return self

    def __exit__(self, *exc_info):
        return self._real.__exit__(*exc_info)

    def __getattr__(self, name):
        return getattr(self._real, name)


def test_a_failure_mid_write_leaves_nothing_behind(conn):
    """The node upsert and the reading insert share one transaction."""
    failing = _FailingConnection(conn, fail_at=3)  # SELECT, node upsert, then fail
    with pytest.raises(sqlite3.OperationalError):
        record_reading(failing, node_id="node-01", reading=dict(READING))

    assert count(conn, "readings") == 0
    assert count(conn, "nodes") == 0


def test_database_is_in_wal_mode(conn):
    mode = conn.execute("PRAGMA journal_mode").fetchone()[0]
    assert mode.lower() == "wal"


def test_foreign_key_is_enforced(conn):
    with pytest.raises(sqlite3.IntegrityError):
        conn.execute(
            "INSERT INTO readings (node_id, recorded_utc, pm25) VALUES ('ghost', '2026-09-24T23:00:00Z', 1.0)"
        )
