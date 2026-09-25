"""The ingest worker: MQTT subscription -> validation -> SQLite.

Runs in the foreground under systemd (`loop_forever`), so no threads and no
daemonisation: the MQTT network loop and the SQLite writes share one thread,
which is also the thread that owns the database connection.

The message-handling path is a plain method (`handle_message`) rather than a
closure over the paho client, so the tests drive the real code with the real
database and no broker.
"""

from __future__ import annotations

import logging
import os
import re
import signal
import sys
from pathlib import Path

import paho.mqtt.client as mqtt

from common import db as db_module

from .config import IngestConfig
from .store import record_reading
from .validation import TOPIC_TEMPLATE, validate_payload

LOG = logging.getLogger("wildfire.ingest")

#: topic prefix and suffix the broker filters on, used to extract the node id
_TOPIC_PARTS = TOPIC_TEMPLATE.split("/")  # ['nordtronics', 'wildfire', '+', 'telemetry']
_NAMESPACE = "/".join(_TOPIC_PARTS[:2])
_TRAILING = _TOPIC_PARTS[3]

#: node ids appear in topics and in the database key, so keep them boring
NODE_ID_RE = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")

#: result codes returned by handle_message (also used as metric labels)
STORED = "stored"
DUPLICATE = "duplicate"
INVALID = "invalid"
IGNORED_TOPIC = "ignored_topic"
NODE_MISMATCH = "node_mismatch"


def parse_node_id(topic: str) -> str | None:
    """Extract the node id from `nordtronics/wildfire/<node-id>/telemetry`.

    Returns None for any topic outside that shape, or for an id that fails
    NODE_ID_RE — a hostile or fat-fingered topic must never reach the database.
    """
    parts = topic.split("/")
    if len(parts) != 4:
        return None
    if parts[0] != _TOPIC_PARTS[0] or parts[1] != _TOPIC_PARTS[1] or parts[3] != _TRAILING:
        return None
    node_id = parts[2]
    if not NODE_ID_RE.match(node_id):
        return None
    return node_id


class IngestWorker:
    """Subscribes to telemetry and appends validated readings to SQLite."""

    def __init__(self, config: IngestConfig):
        self.config = config
        self.conn = None
        self.client: mqtt.Client | None = None
        self.counters = {
            STORED: 0,
            DUPLICATE: 0,
            INVALID: 0,
            IGNORED_TOPIC: 0,
            NODE_MISMATCH: 0,
        }

    # ---------------------------------------------------------------- database

    def open_database(self):
        self.conn = db_module.init_db(self.config.db_path)
        LOG.info("database ready at %s (schema v%s)", self.config.db_path,
                 db_module.schema_version(self.conn))
        return self.conn

    # ------------------------------------------------------------------ broker

    def build_client(self) -> mqtt.Client:
        client = mqtt.Client(
            callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
            client_id=self.config.mqtt_client_id,
            clean_session=False,
        )
        if self.config.mqtt_username:
            client.username_pw_set(self.config.mqtt_username, self.config.mqtt_password)
        if self.config.mqtt_ca_file:
            ca_path = Path(self.config.mqtt_ca_file)
            if not ca_path.exists():
                client.tls_set()  # system trust store; still TLS, never plaintext
                LOG.warning("CA file %s missing — falling back to the system trust store",
                            self.config.mqtt_ca_file)
            elif not os.access(ca_path, os.R_OK):
                # Path.exists() swallows ENOENT/ENOTDIR but reports True for a
                # file this process may not open (EACCES), so tls_set() would
                # die with a raw traceback. An unreadable CA is a deployment
                # fault — name the path and stop rather than fall back to the
                # system trust store, which cannot validate a private CA.
                LOG.critical("FATAL: cannot read MQTT CA file %s: permission denied",
                             self.config.mqtt_ca_file)
                raise SystemExit(1)
            else:
                client.tls_set(ca_certs=self.config.mqtt_ca_file)
        else:
            client.tls_set()  # system trust store; still TLS, never plaintext
            LOG.warning("CA file %s missing — falling back to the system trust store",
                        self.config.mqtt_ca_file)
        client.reconnect_delay_set(min_delay=1, max_delay=60)
        client.on_connect = self.on_connect
        client.on_message = self.on_message
        client.on_disconnect = self.on_disconnect
        return client

    def on_connect(self, client, userdata, flags, reason_code, properties=None):
        if reason_code == 0:
            LOG.info("connected to %s:%s", self.config.mqtt_host, self.config.mqtt_port)
            client.subscribe(self.config.topic, qos=1)
            LOG.info("subscribed to %s (qos 1)", self.config.topic)
        else:
            LOG.error("broker refused the connection: %s", reason_code)

    def on_disconnect(self, client, userdata, flags, reason_code, properties=None):
        LOG.warning("disconnected from broker (%s); paho will reconnect", reason_code)

    def on_message(self, client, userdata, message):
        try:
            self.handle_message(message.topic, message.payload)
        except Exception:  # never let one bad message kill the loop
            LOG.exception("unhandled error processing a message on %s", message.topic)

    # ------------------------------------------------------------ message path

    def handle_message(self, topic: str, payload: bytes | str) -> str:
        """Validate and store one message. Returns a result code."""
        node_id = parse_node_id(topic)
        if node_id is None:
            self.counters[IGNORED_TOPIC] += 1
            LOG.warning("ignoring message on unsupported topic %r", topic)
            return IGNORED_TOPIC

        raw_text = payload.decode("utf-8", "replace") if isinstance(payload, (bytes, bytearray)) else payload
        result = validate_payload(payload)

        if not result.ok:
            self.counters[INVALID] += 1
            LOG.warning("rejected reading from %s: %s", node_id, result.summary)
            return INVALID

        if result.claimed_node_id is not None and result.claimed_node_id != node_id:
            # the topic is what the broker authenticated the publisher for;
            # a payload that disagrees is either a bug or an impersonation
            self.counters[NODE_MISMATCH] += 1
            LOG.warning(
                "rejected reading: topic says node %s, payload says %s",
                node_id, result.claimed_node_id,
            )
            return NODE_MISMATCH

        if result.ignored_fields:
            LOG.info("node %s sent unknown fields %s (ignored)",
                     node_id, ",".join(result.ignored_fields))

        assert result.reading is not None
        outcome = record_reading(
            self.conn,
            node_id=node_id,
            reading=result.reading,
            topic=topic,
            raw_payload=raw_text,
        )
        self.counters[outcome] += 1
        if outcome == STORED:
            LOG.info(
                "stored %s pm25=%s temp=%s rh=%s batt=%s",
                node_id,
                result.reading.get("pm25"),
                result.reading.get("temperature_c"),
                result.reading.get("humidity_pct"),
                result.reading.get("battery_v"),
            )
        else:
            LOG.info("skipped repeated sample from %s (%s)", node_id, result.reading.get("observed_utc"))
        return outcome

    # ---------------------------------------------------------------- lifecycle

    def run(self) -> int:
        self.open_database()
        self.client = self.build_client()

        def _stop(signum, frame):
            LOG.info("signal %s received, shutting down", signum)
            if self.client is not None:
                self.client.disconnect()

        signal.signal(signal.SIGTERM, _stop)
        signal.signal(signal.SIGINT, _stop)

        LOG.info("connecting to %s:%s", self.config.mqtt_host, self.config.mqtt_port)
        client = self.client
        assert client is not None
        client.connect(self.config.mqtt_host, self.config.mqtt_port, keepalive=60)
        client.loop_forever(retry_first_connection=True)
        LOG.info("stopped; counters=%s", self.counters)
        return 0


def configure_logging(level: str) -> None:
    logging.basicConfig(
        level=getattr(logging, level, logging.INFO),
        format="%(asctime)s %(levelname)s %(name)s %(message)s",
        stream=sys.stdout,
    )


def main(argv: list[str] | None = None) -> int:
    config = IngestConfig.from_env()
    configure_logging(config.log_level)
    return IngestWorker(config).run()


if __name__ == "__main__":
    raise SystemExit(main())
