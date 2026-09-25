"""Tests for the ingest worker: topic parsing, message handling, counters."""

from __future__ import annotations

import json

import pytest

from common import db as db_module
from ingest import worker as worker_module
from ingest.config import IngestConfig
from ingest.worker import (
    DUPLICATE,
    IGNORED_TOPIC,
    INVALID,
    NODE_MISMATCH,
    STORED,
    IngestWorker,
    parse_node_id,
)

TOPIC = "nordtronics/wildfire/node-01/telemetry"

GOOD_PAYLOAD = {
    "node_id": "node-01",
    "pm25": 12.3,
    "temperature_c": 18.5,
    "humidity_pct": 42.0,
    "battery_v": 3.92,
    "observed_utc": "2026-09-24T23:00:00Z",
}


def make_config(db_path) -> IngestConfig:
    return IngestConfig(
        mqtt_host="127.0.0.1",
        mqtt_port=8883,
        mqtt_username=None,
        mqtt_password=None,
        mqtt_ca_file=None,
        mqtt_client_id="wildfire-ingest-test",
        topic="nordtronics/wildfire/+/telemetry",
        db_path=str(db_path),
        log_level="INFO",
    )


@pytest.fixture()
def worker(tmp_path):
    instance = IngestWorker(make_config(tmp_path / "wildfire.db"))
    instance.open_database()
    yield instance
    if instance.conn is not None:
        instance.conn.close()


def encode(**overrides) -> bytes:
    document = dict(GOOD_PAYLOAD)
    document.update(overrides)
    return json.dumps(document).encode()


@pytest.mark.parametrize(
    "topic,expected",
    [
        ("nordtronics/wildfire/node-01/telemetry", "node-01"),
        ("nordtronics/wildfire/NODE_7/telemetry", "NODE_7"),
        ("nordtronics/wildfire/a.b-c_d/telemetry", "a.b-c_d"),
        ("nordtronics/wildfire/1234567890/telemetry", "1234567890"),
    ],
)
def test_parse_node_id_accepts_well_formed_topics(topic, expected):
    assert parse_node_id(topic) == expected


@pytest.mark.parametrize(
    "topic",
    [
        "nordtronics/wildfire/telemetry",                    # no node segment
        "nordtronics/wildfire/node-01/telemetry/extra",      # too deep
        "nordtronics/wildfire/node-01/status",               # wrong leaf
        "other/wildfire/node-01/telemetry",                  # wrong namespace
        "nordtronics/wildfire/../telemetry",                 # path traversal
        "nordtronics/wildfire/-leading/telemetry",           # leading separator
        "nordtronics/wildfire/no spaces/telemetry",          # space
        "nordtronics/wildfire/n" + "x" * 64 + "/telemetry",  # too long
        "",
    ],
)
def test_parse_node_id_rejects_malformed_topics(topic):
    assert parse_node_id(topic) is None


def test_valid_message_is_stored(worker):
    assert worker.handle_message(TOPIC, encode()) == STORED
    assert worker.counters[STORED] == 1

    node = worker.conn.execute("SELECT * FROM nodes").fetchone()
    assert node["node_id"] == "node-01"
    assert node["reading_count"] == 1

    reading = worker.conn.execute("SELECT * FROM readings").fetchone()
    assert reading["pm25"] == 12.3
    assert reading["topic"] == TOPIC
    assert json.loads(reading["raw_payload"])["pm25"] == 12.3


def test_duplicate_message_is_counted_but_not_duplicated(worker):
    assert worker.handle_message(TOPIC, encode()) == STORED
    assert worker.handle_message(TOPIC, encode()) == DUPLICATE
    assert worker.counters[DUPLICATE] == 1
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM readings").fetchone()["n"] == 1


def test_invalid_payload_is_rejected(worker):
    assert worker.handle_message(TOPIC, b'{"pm25": "hot"}') == INVALID
    assert worker.counters[INVALID] == 1
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM readings").fetchone()["n"] == 0
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM nodes").fetchone()["n"] == 0


def test_message_on_a_foreign_topic_is_ignored(worker):
    assert worker.handle_message("somewhere/else", encode()) == IGNORED_TOPIC
    assert worker.counters[IGNORED_TOPIC] == 1
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM readings").fetchone()["n"] == 0


def test_payload_node_id_must_match_the_topic(worker):
    assert worker.handle_message(TOPIC, encode(node_id="node-99")) == NODE_MISMATCH
    assert worker.counters[NODE_MISMATCH] == 1
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM readings").fetchone()["n"] == 0


def test_payload_without_node_id_uses_the_topic(worker):
    document = {k: v for k, v in GOOD_PAYLOAD.items() if k != "node_id"}
    assert worker.handle_message(TOPIC, json.dumps(document).encode()) == STORED
    assert worker.conn.execute("SELECT node_id FROM readings").fetchone()["node_id"] == "node-01"


def test_two_nodes_are_tracked_separately(worker):
    second_topic = "nordtronics/wildfire/node-02/telemetry"
    assert worker.handle_message(TOPIC, encode()) == STORED
    assert worker.handle_message(second_topic, encode(node_id="node-02")) == STORED
    assert worker.conn.execute("SELECT COUNT(*) AS n FROM nodes").fetchone()["n"] == 2


def test_on_message_swallows_handler_errors(worker):
    """One bad message must never kill the subscription loop."""
    worker.handle_message = lambda topic, payload: (_ for _ in ()).throw(RuntimeError("boom"))

    class Message:
        topic = TOPIC
        payload = b"{}"

    worker.on_message(None, None, Message())  # must not raise


class _StubClient:
    def __init__(self):
        self.subscriptions = []
        self.tls = None

    def subscribe(self, topic, qos=0):
        self.subscriptions.append((topic, qos))


def test_on_connect_subscribes_when_the_broker_accepts(worker):
    client = _StubClient()
    worker.on_connect(client, None, None, 0)
    assert client.subscriptions == [("nordtronics/wildfire/+/telemetry", 1)]


def test_on_connect_does_not_subscribe_when_refused(worker):
    client = _StubClient()
    worker.on_connect(client, None, None, 5)  # not authorised
    assert client.subscriptions == []


def test_build_client_configures_tls_and_callbacks(worker):
    client = worker.build_client()
    assert client.on_message == worker.on_message
    assert client.on_connect == worker.on_connect
    assert client.on_disconnect == worker.on_disconnect
    # every connection this worker makes is TLS, even without a CA file
    assert client._ssl_context is not None


def test_build_client_sets_the_credentials(worker):
    config = make_config(worker.config.db_path)
    config.mqtt_username = "wildfire-base"
    config.mqtt_password = "s3cret"
    instance = IngestWorker(config)
    client = instance.build_client()
    assert client._username == b"wildfire-base"
    assert client._password == b"s3cret"


def test_open_database_creates_the_file_and_schema(tmp_path):
    config = make_config(tmp_path / "wildfire.db")
    instance = IngestWorker(config)
    conn = instance.open_database()
    assert (tmp_path / "wildfire.db").exists()
    assert db_module.schema_version(conn) == db_module.SCHEMA_VERSION
    conn.close()


def test_process_payload_is_dispatched_by_topic_search(monkeypatch, tmp_path):
    """The subscribed filter must be able to match the per-node topics."""
    config = make_config(tmp_path / "wildfire.db")
    from ingest.validation import TOPIC_TEMPLATE

    assert worker_module.parse_node_id(TOPIC_TEMPLATE.replace("+", "node-01")) == "node-01"
    assert config.topic == TOPIC_TEMPLATE
