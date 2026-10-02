"""Tests for the Watch/alert event path (task 0095).

Two layers are covered and they are deliberately separate:

* `ingest.events` — the wire contract: what the base station publishes, which
  payloads are refused, and how an event maps onto the alert object the app
  renders;
* `ingest.worker` — the dispatch: which topic a message must arrive on, and the
  duplicate suppression that makes a QoS-1 redelivery harmless.
"""

from __future__ import annotations

import json

import pytest

from ingest import worker as worker_module
from ingest.events import EVENT_KINDS, alert_id_for, validate_event
from ingest.worker import (
    EVENT_DUPLICATE,
    EVENT_INVALID,
    EVENT_STORED,
    IGNORED_TOPIC,
    IngestWorker,
    STORED,
    KIND_EVENT,
    KIND_TELEMETRY,
    parse_node_id,
    parse_topic,
)

from test_worker import make_config  # noqa: E402  (sibling test module, same dir)

EVENTS_TOPIC = "nordtronics/wildfire/base-01/events"

#: the exact payload task 0095 specifies, published as the base station
WATCH_RAISED = {
    "node_id": "bench-01",
    "event": "watch_raised",
    "observed_utc": "2026-10-02T21:00:00Z",
    "pm25": 47.9,
    "baseline": 12.4,
    "nodes": ["bench-01"],
    "window_min": 20,
}

ALERT_RAISED = {
    "node_id": "network",
    "event": "alert_raised",
    "observed_utc": "2026-10-02T21:12:00Z",
    "pm25": 61.2,
    "baseline": 12.4,
    "nodes": ["bench-01", "bench-02"],
    "window_min": 20,
}


@pytest.fixture()
def worker(tmp_path):
    instance = IngestWorker(make_config(tmp_path / "wildfire.db"))
    instance.open_database()
    yield instance
    if instance.conn is not None:
        instance.conn.close()


def encode(document: dict) -> bytes:
    return json.dumps(document).encode()



# --------------------------------------------------------------------- topics


@pytest.mark.parametrize(
    "topic,kind,node",
    [
        ("nordtronics/wildfire/base-01/events", KIND_EVENT, "base-01"),
        ("nordtronics/wildfire/node-01/telemetry", KIND_TELEMETRY, "node-01"),
    ],
)
def test_parse_topic_classifies_both_leaf_kinds(topic, kind, node):
    assert parse_topic(topic) == (kind, node)


@pytest.mark.parametrize(
    "topic",
    [
        "nordtronics/wildfire/base-01/status",   # unknown leaf
        "nordtronics/wildfire/base-01/event",    # singular: not the contract
        "other/wildfire/base-01/events",         # wrong namespace
        "nordtronics/wildfire/../events",         # traversal
        "nordtronics/wildfire/no spaces/events",
    ],
)
def test_parse_topic_rejects_everything_else(topic):
    assert parse_topic(topic) is None


def test_parse_node_id_stays_telemetry_only():
    """The events topic carries a publisher, not a reporting node: an events
    topic must not be readable as a telemetry topic."""
    assert parse_node_id("nordtronics/wildfire/base-01/events") is None
    assert parse_node_id("nordtronics/wildfire/node-01/telemetry") == "node-01"


# ---------------------------------------------------------------- validation


@pytest.mark.parametrize("kind", sorted(EVENT_KINDS))
def test_every_event_kind_validates_and_maps_to_the_contract(kind):
    result = validate_event(encode({
        "node_id": "bench-01",
        "event": kind,
        "observed_utc": "2026-10-02T21:00:00Z",
        "pm25": 47.9,
        "baseline": 12.4,
        "nodes": ["bench-01", "bench-02"],
        "window_min": 20,
    }))
    assert result.ok, result.summary
    expected_type, expected_severity, expected_state = EVENT_KINDS[kind]
    assert result.event["type"] == expected_type
    assert result.event["severity"] == expected_severity
    assert result.event["state"] == expected_state
    assert result.event["observed_utc"] == "2026-10-02T21:00:00Z"
    assert result.event["alert_id"].startswith("ev-bench-01-" + kind)


def test_optional_fields_may_be_absent():
    """A clear carries no reading: only `event` and `observed_utc` are required."""
    result = validate_event(encode({"event": "watch_cleared",
                                    "observed_utc": "2026-10-02T21:30:00Z"}))
    assert result.ok, result.summary
    assert result.event["node_id"] == "network"
    assert result.event["pm25"] is None
    assert result.event["baseline"] is None
    assert result.event["window_min"] is None
    assert result.event["nodes"] == []
    assert result.event["state"] == "cleared"


@pytest.mark.parametrize(
    "document,expect",
    [
        ({"observed_utc": "2026-10-02T21:00:00Z"}, "event"),
        ({"event": "alarm", "observed_utc": "2026-10-02T21:00:00Z"}, "event"),
        ({"event": "watch_raised"}, "observed_utc"),
        ({"event": "watch_raised", "observed_utc": "yesterday"}, "observed_utc"),
        ({"event": "watch_raised", "observed_utc": "2026-10-02T21:00:00Z", "pm25": -4}, "pm25"),
        ({"event": "watch_raised", "observed_utc": "2026-10-02T21:00:00Z", "pm25": "hot"}, "pm25"),
        ({"event": "watch_raised", "observed_utc": "2026-10-02T21:00:00Z",
          "window_min": 0}, "window_min"),
        ({"event": "watch_raised", "observed_utc": "2026-10-02T21:00:00Z",
          "window_min": 99999}, "window_min"),
        ({"event": "watch_raised", "observed_utc": "2026-10-02T21:00:00Z",
          "nodes": "bench-01"}, "nodes"),
        ({"event": "watch_raised", "observed_utc": "2026-10-02T21:00:00Z",
          "nodes": ["bench-01", 7]}, "nodes"),
        ({"event": "watch_raised", "observed_utc": "2026-10-02T21:00:00Z",
          "node_id": ""}, "node_id"),
        (b"{not json", "payload"),
    ],
)
def test_invalid_events_name_the_offending_field(document, expect):
    result = validate_event(document if isinstance(document, bytes) else encode(document))
    assert not result.ok
    assert any(expect in error for error in result.errors), result.errors


def test_unknown_fields_are_surfaced_but_not_fatal():
    result = validate_event(encode({
        "event": "watch_raised",
        "observed_utc": "2026-10-02T21:00:00Z",
        "confidence": 0.9,
        "rssi": -71,
    }))
    assert result.ok, result.summary
    assert result.ignored_fields == ["confidence", "rssi"]


def test_alert_id_is_deterministic_and_keyed_on_the_instant():
    a = alert_id_for("bench-01", "watch_raised", "2026-10-02T21:00:00Z")
    b = alert_id_for("bench-01", "watch_raised", "2026-10-02T21:00:00Z")
    c = alert_id_for("bench-01", "watch_raised", "2026-10-02T21:01:00Z")
    assert a == b
    assert a != c
    assert ":" not in a and " " not in a


# --------------------------------------------------------------------- worker


def test_event_is_stored_with_the_publisher_from_the_topic(worker):
    assert worker.handle_message(EVENTS_TOPIC, encode(WATCH_RAISED)) == EVENT_STORED
    assert worker.counters[EVENT_STORED] == 1

    row = worker.conn.execute("SELECT * FROM alerts").fetchone()
    assert row["publisher"] == "base-01"        # the topic, i.e. the base station
    assert row["node_id"] == "bench-01"         # the payload, i.e. the elevated node
    assert row["event"] == "watch_raised"
    assert row["type"] == "watch"
    assert row["state"] == "active"
    assert row["pm25"] == 47.9
    assert row["baseline"] == 12.4
    assert row["window_min"] == 20
    assert json.loads(row["nodes_json"]) == ["bench-01"]

    # an event does NOT invent a telemetry row for the node it is about
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM nodes").fetchone()["n"] == 0


def test_redelivered_event_is_counted_but_not_stored_twice(worker):
    assert worker.handle_message(EVENTS_TOPIC, encode(WATCH_RAISED)) == EVENT_STORED
    assert worker.handle_message(EVENTS_TOPIC, encode(WATCH_RAISED)) == EVENT_DUPLICATE
    assert worker.counters[EVENT_DUPLICATE] == 1
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM alerts").fetchone()["n"] == 1


def test_two_bases_may_report_the_same_event_without_colliding(worker):
    """The alert id is keyed on the event, not on who published it: two base
    stations repeating one consensus event is one alert, not two."""
    second = "nordtronics/wildfire/base-02/events"
    assert worker.handle_message(EVENTS_TOPIC, encode(WATCH_RAISED)) == EVENT_STORED
    assert worker.handle_message(second, encode(WATCH_RAISED)) == EVENT_DUPLICATE
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM alerts").fetchone()["n"] == 1


def test_a_multi_node_alert_names_the_network(worker):
    assert worker.handle_message(EVENTS_TOPIC, encode(ALERT_RAISED)) == EVENT_STORED
    row = worker.conn.execute("SELECT * FROM alerts").fetchone()
    assert row["node_id"] == "network"
    assert json.loads(row["nodes_json"]) == ["bench-01", "bench-02"]


def test_telemetry_on_the_events_topic_is_rejected_not_stored(worker):
    """The dispatch is by topic, so a telemetry body on the events topic is an
    invalid event — never a reading."""
    telemetry_body = {
        "pm25": 12.3, "temperature_c": 18.5, "humidity_pct": 42.0,
        "battery_v": 3.92, "observed_utc": "2026-10-02T21:00:00Z",
    }
    assert worker.handle_message(EVENTS_TOPIC, encode(telemetry_body)) == EVENT_INVALID
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM alerts").fetchone()["n"] == 0
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM readings").fetchone()["n"] == 0


def test_an_event_on_the_telemetry_topic_is_rejected_not_stored(worker):
    """And the other way round: an event body on a telemetry topic is an
    invalid reading."""
    assert worker.handle_message(
        "nordtronics/wildfire/bench-01/telemetry", encode(WATCH_RAISED)
    ) == worker_module.INVALID
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM alerts").fetchone()["n"] == 0


def test_events_and_telemetry_coexist_on_one_worker(worker):
    reading = {
        "node_id": "bench-01", "pm25": 12.3, "temperature_c": 18.5,
        "humidity_pct": 42.0, "battery_v": 3.92, "observed_utc": "2026-10-02T21:00:00Z",
    }
    assert worker.handle_message(
        "nordtronics/wildfire/bench-01/telemetry", encode(reading)
    ) == STORED
    assert worker.handle_message(EVENTS_TOPIC, encode(WATCH_RAISED)) == EVENT_STORED
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM readings").fetchone()["n"] == 1
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM alerts").fetchone()["n"] == 1


def test_a_foreign_topic_is_still_ignored(worker):
    assert worker.handle_message("somewhere/else", encode(WATCH_RAISED)) == IGNORED_TOPIC
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM alerts").fetchone()["n"] == 0
