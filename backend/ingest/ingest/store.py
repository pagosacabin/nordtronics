"""Write path into the telemetry database (owned by the ingest worker)."""

from __future__ import annotations

import json
import sqlite3
from datetime import datetime, timezone


def utc_now_iso() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def record_reading(
    conn: sqlite3.Connection,
    *,
    node_id: str,
    reading: dict,
    topic: str | None = None,
    raw_payload: str | None = None,
    recorded_utc: str | None = None,
) -> str:
    """Insert one reading and refresh the node's summary row.

    Returns `"stored"` or `"duplicate"`. A reading that carries the node's own
    `observed_utc` is suppressed when that exact sample is already present,
    which makes QoS-1 redelivery after a reconnect harmless. Readings without
    an `observed_utc` are always stored — nothing identifies them as repeats.

    The insert and the node upsert share one transaction: a crash can never
    leave a reading counted in `nodes.reading_count` but missing from
    `readings`.
    """
    recorded_utc = recorded_utc or utc_now_iso()
    observed_utc = reading.get("observed_utc")

    with conn:  # transaction: commits on success, rolls back on exception
        if observed_utc is not None:
            existing = conn.execute(
                "SELECT 1 FROM readings WHERE node_id = ? AND observed_utc = ? LIMIT 1",
                (node_id, observed_utc),
            ).fetchone()
            if existing:
                return "duplicate"

        # the node row comes first: `readings.node_id` is a foreign key and
        # `PRAGMA foreign_keys` is on, so a reading for an unknown node would
        # otherwise be rejected on a node's very first message
        conn.execute(
            """
            INSERT INTO nodes
                (node_id, first_seen_utc, last_seen_utc, reading_count,
                 last_pm25, last_temperature_c, last_humidity_pct, last_battery_v)
            VALUES (?, ?, ?, 1, ?, ?, ?, ?)
            ON CONFLICT(node_id) DO UPDATE SET
                last_seen_utc      = excluded.last_seen_utc,
                reading_count      = nodes.reading_count + 1,
                last_pm25          = excluded.last_pm25,
                last_temperature_c = excluded.last_temperature_c,
                last_humidity_pct  = excluded.last_humidity_pct,
                last_battery_v     = excluded.last_battery_v
            """,
            (
                node_id,
                recorded_utc,
                recorded_utc,
                reading.get("pm25"),
                reading.get("temperature_c"),
                reading.get("humidity_pct"),
                reading.get("battery_v"),
            ),
        )

        conn.execute(
            """
            INSERT INTO readings
                (node_id, recorded_utc, observed_utc, pm25, temperature_c,
                 humidity_pct, battery_v, topic, raw_payload)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                node_id,
                recorded_utc,
                observed_utc,
                reading.get("pm25"),
                reading.get("temperature_c"),
                reading.get("humidity_pct"),
                reading.get("battery_v"),
                topic,
                raw_payload,
            ),
        )
    return "stored"


def record_event(
    conn: sqlite3.Connection,
    *,
    publisher: str,
    event: dict,
    topic: str | None = None,
    raw_payload: str | None = None,
    recorded_utc: str | None = None,
) -> str:
    """Insert one Watch/alert event. Returns `"stored"` or `"duplicate"`.

    `publisher` is the topic's node id — the base station the broker
    authenticated. `event["node_id"]` is the elevated node (or `network`),
    which is a different thing and is allowed to be a node with no telemetry
    row, so no foreign key is declared on `alerts`.

    The event carries its own `observed_utc` (required by the contract), so
    QoS-1 redelivery is suppressed the same way telemetry is: on the derived
    `alert_id`, which is a UNIQUE column. Note that events deliberately do NOT
    create or touch a `nodes` row: a base station reporting on `bench-01` is
    not the same thing as `bench-01` having reported telemetry.
    """
    recorded_utc = recorded_utc or utc_now_iso()
    observed_utc = event["observed_utc"]

    with conn:
        existing = conn.execute(
            "SELECT 1 FROM alerts WHERE alert_id = ? LIMIT 1", (event["alert_id"],)
        ).fetchone()
        if existing:
            return "duplicate"

        conn.execute(
            """
            INSERT INTO alerts
                (alert_id, publisher, node_id, event, type, severity, state, title,
                 detail, pm25, baseline, window_min, nodes_json, observed_utc,
                 recorded_utc, created_utc, updated_utc, topic, raw_payload)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                event["alert_id"],
                publisher,
                event["node_id"],
                event["event"],
                event["type"],
                event["severity"],
                event["state"],
                event["title"],
                event["detail"],
                event.get("pm25"),
                event.get("baseline"),
                event.get("window_min"),
                json.dumps(event.get("nodes") or []),
                observed_utc,
                recorded_utc,
                observed_utc,
                observed_utc,
                topic,
                raw_payload,
            ),
        )
    return "stored"
