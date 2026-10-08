"""End-to-end tests for the alerts routes (task 0095).

Same posture as `test_api.py`: a real SQLite file filled through the ingest
write path, then read through the real FastAPI app — the two processes that run
on the VPS, pointed at one database.
"""

from __future__ import annotations

import json

import pytest
from fastapi.testclient import TestClient

from api.app import create_app
from api.config import ApiConfig
from common import db as db_module
from ingest.events import validate_event
from ingest.store import record_event, record_reading

READING = {
    "pm25": 12.3,
    "temperature_c": 18.5,
    "humidity_pct": 42.0,
    "battery_v": 3.92,
    "observed_utc": "2026-10-02T20:00:00Z",
}

#: the contract's alert object, in order of the fields it promises
CONTRACT_FIELDS = (
    "alert_id", "type", "node_id", "severity", "state",
    "title", "detail", "created_utc", "updated_utc",
)


def watch_raised(node_id: str, observed_utc: str, **overrides) -> dict:
    document = {
        "node_id": node_id,
        "event": "watch_raised",
        "observed_utc": observed_utc,
        "pm25": 47.9,
        "baseline": 12.4,
        "nodes": [node_id],
        "window_min": 20,
    }
    document.update(overrides)
    return document


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
                               stale_after_seconds=900, log_level="INFO",
                               api_key="test-key"))
    with TestClient(app, headers={"X-API-Key": "test-key"}) as test_client:
        yield test_client


def store_event(conn, document: dict, publisher: str = "base-01") -> str:
    """Put one event in through the ingest validation + write path."""
    result = validate_event(json.dumps(document).encode())
    assert result.ok, result.summary
    return record_event(
        conn,
        publisher=publisher,
        event=result.event,
        topic=f"nordtronics/wildfire/{publisher}/events",
        raw_payload=json.dumps(document),
    )


# ------------------------------------------------------------------ the feed


def test_empty_feed_answers_with_an_empty_list(client):
    response = client.get("/v1/alerts")
    assert response.status_code == 200
    body = response.json()
    assert body["alerts"] == []
    assert body["count"] == 0


def test_feed_is_newest_first_and_carries_the_contract_fields(client, writer):
    store_event(writer, watch_raised("bench-01", "2026-10-02T21:00:00Z"))
    store_event(writer, watch_raised("bench-02", "2026-10-02T21:12:00Z"))

    body = client.get("/v1/alerts").json()
    assert body["count"] == 2
    assert [a["node_id"] for a in body["alerts"]] == ["bench-02", "bench-01"]
    for alert in body["alerts"]:
        for field in CONTRACT_FIELDS:
            assert field in alert, field
    newest = body["alerts"][0]
    assert newest["type"] == "watch"
    assert newest["severity"] == "watch"
    assert newest["state"] == "active"
    assert "47.9" in newest["title"]
    assert newest["observed_utc"] == "2026-10-02T21:12:00Z"
    assert newest["publisher"] == "base-01"


def test_a_cleared_event_maps_to_the_cleared_state(client, writer):
    store_event(writer, {"node_id": "bench-01", "event": "watch_cleared",
                         "observed_utc": "2026-10-02T21:30:00Z"})
    alert = client.get("/v1/alerts").json()["alerts"][0]
    assert alert["event"] == "watch_cleared"
    assert alert["state"] == "cleared"
    assert alert["pm25"] is None


def test_limit_is_honoured_and_capped(client, writer):
    for minute in range(5):
        store_event(writer, watch_raised("bench-01", f"2026-10-02T21:0{minute}:00Z"))

    assert client.get("/v1/alerts?limit=2").json()["count"] == 2
    assert client.get("/v1/alerts?limit=2").json()["alerts"][0]["observed_utc"] == \
        "2026-10-02T21:04:00Z"

    assert client.get("/v1/alerts?limit=0").status_code == 422
    assert client.get("/v1/alerts?limit=100000").status_code == 422


def test_feed_can_be_filtered_by_involved_node(client, writer):
    store_event(writer, watch_raised("bench-01", "2026-10-02T21:00:00Z"))
    store_event(writer, watch_raised("bench-02", "2026-10-02T21:12:00Z"))

    body = client.get("/v1/alerts?node_id=bench-01").json()
    assert body["count"] == 1
    assert body["alerts"][0]["node_id"] == "bench-01"

    assert client.get("/v1/alerts?node_id=no spaces").status_code == 422


# ------------------------------------------------------------- per-node feed


def test_node_feed_returns_the_alerts_about_that_node(client, writer):
    store_event(writer, watch_raised("bench-01", "2026-10-02T21:00:00Z"))
    store_event(writer, watch_raised("bench-02", "2026-10-02T21:12:00Z"))

    body = client.get("/v1/nodes/bench-01/alerts").json()
    assert body["node_id"] == "bench-01"
    assert body["count"] == 1
    assert body["alerts"][0]["node_id"] == "bench-01"


def test_node_feed_includes_the_confirming_nodes_of_a_network_alert(client, writer):
    store_event(writer, {
        "node_id": "network",
        "event": "alert_raised",
        "observed_utc": "2026-10-02T21:20:00Z",
        "pm25": 61.2,
        "baseline": 12.4,
        "nodes": ["bench-01", "bench-02"],
        "window_min": 20,
    })

    for node in ("bench-01", "bench-02"):
        body = client.get(f"/v1/nodes/{node}/alerts").json()
        assert body["count"] == 1, node
        assert body["alerts"][0]["node_id"] == "network"
        assert body["alerts"][0]["nodes"] == ["bench-01", "bench-02"]

    # a node that neither raised nor confirmed it sees nothing
    assert client.get("/v1/nodes/bench-03/alerts").status_code == 404


def test_a_network_alert_is_reachable_by_its_own_pseudo_node(client, writer):
    """`network` is not a telemetry node, but it is a real value of `node_id` on
    the feed: asking for it must answer, not 404."""
    store_event(writer, {"node_id": "network", "event": "alert_cleared",
                         "observed_utc": "2026-10-02T21:40:00Z"})
    body = client.get("/v1/nodes/network/alerts").json()
    assert body["count"] == 1
    assert body["alerts"][0]["state"] == "cleared"


def test_an_alert_for_a_node_with_no_telemetry_is_not_a_404(client, writer):
    """The base station reports on nodes it hears over LoRa, which have no
    telemetry row on this host; that is normal, not an unknown node."""
    store_event(writer, watch_raised("bench-09", "2026-10-02T21:00:00Z"))
    response = client.get("/v1/nodes/bench-09/alerts")
    assert response.status_code == 200
    assert response.json()["count"] == 1


def test_a_genuinely_unknown_node_is_a_404(client, writer):
    record_reading(writer, node_id="bench-01", reading=READING)
    assert client.get("/v1/nodes/bench-01/alerts").json()["count"] == 0
    assert client.get("/v1/nodes/ghost-node/alerts").status_code == 404


def test_node_feed_limit_is_honoured(client, writer):
    for minute in range(4):
        store_event(writer, watch_raised("bench-01", f"2026-10-02T21:0{minute}:00Z"))
    body = client.get("/v1/nodes/bench-01/alerts?limit=3").json()
    assert body["count"] == 3
    assert body["limit"] == 3


# ------------------------------------------------------- the telemetry guard


def test_the_telemetry_path_is_unchanged(client, writer):
    """Task 0095 must not disturb the routes that already worked."""
    record_reading(writer, node_id="bench-01", reading=READING)
    store_event(writer, watch_raised("bench-01", "2026-10-02T21:00:00Z"))

    nodes = client.get("/v1/nodes").json()
    assert nodes["count"] == 1
    assert nodes["nodes"][0]["node_id"] == "bench-01"

    readings = client.get("/v1/nodes/bench-01/readings").json()
    assert readings["count"] == 1
    assert readings["readings"][0]["pm25"] == 12.3

    health = client.get("/healthz").json()
    assert health["schema_version"] == db_module.SCHEMA_VERSION
