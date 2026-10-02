"""End-to-end API tests.

These drive the real FastAPI app against a real SQLite file that is filled
through the *ingest* write path — the same two processes that run on the VPS,
pointed at one database.
"""

from __future__ import annotations

import json
import sqlite3
from datetime import datetime, timedelta, timezone

import pytest
from fastapi.testclient import TestClient

from api.app import create_app
from api.config import ApiConfig
from common import db as db_module
from ingest.store import record_reading

READING = {
    "pm25": 12.3,
    "temperature_c": 18.5,
    "humidity_pct": 42.0,
    "battery_v": 3.92,
    "observed_utc": "2026-09-24T23:00:00Z",
}


def stamp(minutes_ago: int = 0) -> str:
    when = datetime.now(timezone.utc) - timedelta(minutes=minutes_ago)
    return when.strftime("%Y-%m-%dT%H:%M:%SZ")


@pytest.fixture()
def db_path(tmp_path):
    return tmp_path / "wildfire.db"


@pytest.fixture()
def writer(db_path):
    conn = db_module.init_db(db_path)
    yield conn
    conn.close()


@pytest.fixture()
def client(db_path, writer):
    app = create_app(ApiConfig(db_path=str(db_path), host="127.0.0.1", port=8000,
                               stale_after_seconds=900, log_level="INFO"))
    with TestClient(app) as test_client:
        yield test_client


def test_healthz_reports_schema_version(client):
    response = client.get("/healthz")
    assert response.status_code == 200
    body = response.json()
    assert body["status"] == "ok"
    assert body["database"] == "ok"
    assert body["schema_version"] == db_module.SCHEMA_VERSION


def test_healthz_is_degraded_when_the_database_is_missing(tmp_path):
    app = create_app(ApiConfig(db_path=str(tmp_path / "absent.db"), host="127.0.0.1",
                               port=8000, stale_after_seconds=900, log_level="INFO"))
    with TestClient(app) as test_client:
        response = test_client.get("/healthz")
    assert response.status_code == 503
    assert response.json()["status"] == "degraded"


def test_nodes_endpoint_is_empty_before_any_telemetry(client):
    body = client.get("/v1/nodes").json()
    assert body["nodes"] == []
    assert body["count"] == 0
    assert body["stale_after_seconds"] == 900


def test_nodes_endpoint_returns_a_node_with_its_latest_reading(client, writer):
    record_reading(writer, node_id="node-01", reading=dict(READING), recorded_utc=stamp(1))
    body = client.get("/v1/nodes").json()

    assert body["count"] == 1
    node = body["nodes"][0]
    assert node["node_id"] == "node-01"
    assert node["reading_count"] == 1
    assert node["status"] == "ok"
    assert node["age_seconds"] is not None and node["age_seconds"] < 900
    assert node["latest"] == {
        "pm25": 12.3,
        "temperature_c": 18.5,
        "humidity_pct": 42.0,
        "battery_v": 3.92,
    }


def test_node_goes_stale_after_the_threshold(client, writer):
    record_reading(writer, node_id="node-01", reading=dict(READING), recorded_utc=stamp(60))
    node = client.get("/v1/nodes").json()["nodes"][0]
    assert node["status"] == "stale"
    assert node["age_seconds"] >= 3500


def test_nodes_are_sorted_by_id(client, writer):
    for node_id in ("node-03", "node-01", "node-02"):
        record_reading(writer, node_id=node_id, reading=dict(READING), recorded_utc=stamp())
    ids = [node["node_id"] for node in client.get("/v1/nodes").json()["nodes"]]
    assert ids == ["node-01", "node-02", "node-03"]


def test_readings_are_returned_newest_first(client, writer):
    for minutes in (30, 10, 20):
        record_reading(
            writer,
            node_id="node-01",
            reading=dict(READING, pm25=float(minutes), observed_utc=f"2026-09-24T22:{minutes:02d}:00Z"),
            recorded_utc=stamp(minutes),
        )
    body = client.get("/v1/nodes/node-01/readings").json()
    assert body["count"] == 3
    assert [reading["pm25"] for reading in body["readings"]] == [10.0, 20.0, 30.0]


def test_readings_limit_is_honoured(client, writer):
    for minutes in range(5):
        record_reading(
            writer,
            node_id="node-01",
            reading=dict(READING, observed_utc=f"2026-09-24T21:0{minutes}:00Z"),
            recorded_utc=stamp(minutes),
        )
    body = client.get("/v1/nodes/node-01/readings", params={"limit": 2}).json()
    assert body["count"] == 2
    assert body["limit"] == 2


def test_readings_since_filter(client, writer):
    record_reading(writer, node_id="node-01", reading=dict(READING), recorded_utc=stamp(120))
    record_reading(
        writer,
        node_id="node-01",
        reading=dict(READING, observed_utc="2026-09-24T22:00:00Z", pm25=55.0),
        recorded_utc=stamp(2),
    )
    cutoff = stamp(60)
    body = client.get("/v1/nodes/node-01/readings", params={"since": cutoff}).json()
    assert body["since"] == cutoff
    assert body["count"] == 1
    assert body["readings"][0]["pm25"] == 55.0


def test_readings_since_accepts_an_offset_timestamp(client, writer):
    record_reading(writer, node_id="node-01", reading=dict(READING), recorded_utc=stamp(2))
    body = client.get(
        "/v1/nodes/node-01/readings", params={"since": "2020-01-01T00:00:00-06:00"}
    ).json()
    assert body["count"] == 1
    assert body["since"] == "2020-01-01T06:00:00Z"


def test_readings_for_a_single_node_only(client, writer):
    record_reading(writer, node_id="node-01", reading=dict(READING), recorded_utc=stamp())
    record_reading(writer, node_id="node-02", reading=dict(READING), recorded_utc=stamp())
    body = client.get("/v1/nodes/node-02/readings").json()
    assert body["node_id"] == "node-02"
    assert body["count"] == 1


def test_unknown_node_is_404(client):
    response = client.get("/v1/nodes/node-99/readings")
    assert response.status_code == 404
    assert "unknown node" in response.json()["detail"]


def test_malformed_node_id_is_rejected_by_the_schema(client):
    assert client.get("/v1/nodes/-bad-/readings").status_code == 422


@pytest.mark.parametrize("limit", [0, -1, 1001, "many"])
def test_out_of_range_limit_is_rejected(client, limit):
    response = client.get("/v1/nodes/node-01/readings", params={"limit": limit})
    assert response.status_code == 422


def test_malformed_since_is_a_400(client):
    response = client.get("/v1/nodes/node-01/readings", params={"since": "last week"})
    assert response.status_code == 400
    assert "ISO-8601" in response.json()["detail"]


def test_the_api_connection_is_read_only(db_path, writer):
    """The API's own handle must not be able to write telemetry."""
    conn = db_module.connect(db_path, read_only=True)
    try:
        with pytest.raises(sqlite3.OperationalError, match="readonly"):
            conn.execute(
                "INSERT INTO readings (node_id, recorded_utc, pm25) VALUES ('node-01', ?, 1.0)",
                (stamp(),),
            )
    finally:
        conn.close()


def test_api_reads_a_database_written_by_the_ingest_worker(client, writer):
    """WAL keeps the reader working while the writer holds the file open."""
    record_reading(writer, node_id="node-01", reading=dict(READING), recorded_utc=stamp())
    writer.commit()
    assert client.get("/v1/nodes").json()["count"] == 1


def test_response_is_json_serialisable(client, writer):
    record_reading(writer, node_id="node-01", reading=dict(READING), recorded_utc=stamp())
    payload = client.get("/v1/nodes").json()
    json.dumps(payload)  # raises if any value is not serialisable
    assert payload["generated_utc"].endswith("Z")
