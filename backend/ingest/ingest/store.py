"""Write path into the telemetry database (owned by the ingest worker)."""

from __future__ import annotations

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
