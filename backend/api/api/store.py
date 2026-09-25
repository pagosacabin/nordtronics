"""Read queries used by the API.

Every query goes through a connection opened with SQLite's `mode=ro`, so the
API physically cannot write telemetry even if a handler is wrong.
"""

from __future__ import annotations

import sqlite3
from datetime import datetime, timezone

READING_COLUMNS = ("recorded_utc", "observed_utc", "pm25", "temperature_c", "humidity_pct", "battery_v")


def utc_now_iso() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _seconds_since(stamp: str) -> int | None:
    try:
        when = datetime.strptime(stamp, "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc)
    except (TypeError, ValueError):
        return None
    return max(0, int((datetime.now(timezone.utc) - when).total_seconds()))


def _row_to_reading(row: sqlite3.Row) -> dict:
    return {column: row[column] for column in READING_COLUMNS}


def list_nodes(conn: sqlite3.Connection, stale_after_seconds: int) -> list[dict]:
    """Every known node with its most recent reading and freshness."""
    rows = conn.execute(
        """
        SELECT node_id, first_seen_utc, last_seen_utc, reading_count,
               last_pm25, last_temperature_c, last_humidity_pct, last_battery_v
        FROM nodes
        ORDER BY node_id
        """
    ).fetchall()

    nodes: list[dict] = []
    for row in rows:
        age = _seconds_since(row["last_seen_utc"])
        if age is None:
            status = "unknown"
        elif age <= stale_after_seconds:
            status = "ok"
        else:
            status = "stale"
        nodes.append(
            {
                "node_id": row["node_id"],
                "first_seen_utc": row["first_seen_utc"],
                "last_seen_utc": row["last_seen_utc"],
                "reading_count": row["reading_count"],
                "age_seconds": age,
                "status": status,
                "latest": {
                    "pm25": row["last_pm25"],
                    "temperature_c": row["last_temperature_c"],
                    "humidity_pct": row["last_humidity_pct"],
                    "battery_v": row["last_battery_v"],
                },
            }
        )
    return nodes


def node_exists(conn: sqlite3.Connection, node_id: str) -> bool:
    return conn.execute("SELECT 1 FROM nodes WHERE node_id = ?", (node_id,)).fetchone() is not None


def list_readings(
    conn: sqlite3.Connection,
    node_id: str,
    *,
    limit: int,
    since: str | None = None,
) -> list[dict]:
    """Readings for one node, newest first."""
    sql = (
        "SELECT recorded_utc, observed_utc, pm25, temperature_c, humidity_pct, battery_v "
        "FROM readings WHERE node_id = ?"
    )
    params: list = [node_id]
    if since is not None:
        sql += " AND recorded_utc >= ?"
        params.append(since)
    sql += " ORDER BY recorded_utc DESC, id DESC LIMIT ?"
    params.append(limit)
    return [_row_to_reading(row) for row in conn.execute(sql, params).fetchall()]


def schema_version(conn: sqlite3.Connection) -> int:
    row = conn.execute("SELECT value FROM meta WHERE key = 'schema_version'").fetchone()
    return int(row["value"]) if row else 0
