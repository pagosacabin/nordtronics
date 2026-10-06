"""Read queries used by the API.

Every query goes through a connection opened with SQLite's `mode=ro`, so the
API physically cannot write telemetry even if a handler is wrong.
"""

from __future__ import annotations

import json
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


def _battery_pct(battery_v):
    """Single-cell Li-ion charge percent: 3.00 V = 0 %, 4.20 V = 100 %.

    Matches the thresholds in the Android app's Node.batteryPercent().
    Returns None when there is no real reading.
    """
    if battery_v is None or battery_v <= 0:
        return None
    pct = (battery_v - 3.00) / (4.20 - 3.00) * 100.0
    return int(round(max(0.0, min(100.0, pct))))


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
                "battery_pct": _battery_pct(row["last_battery_v"]),
                "latest": {
                    "pm25": row["last_pm25"],
                    "temperature_c": row["last_temperature_c"],
                    "humidity_pct": row["last_humidity_pct"],
                    "battery_v": row["last_battery_v"],
                },
            }
        )
    return nodes


def list_alerts(
    conn: sqlite3.Connection,
    *,
    limit: int,
    node_id: str | None = None,
) -> list[dict]:
    """Watch/alert events, newest first (task 0095).

    Returns contract v1's alert object — the shape `GET /v1/alerts` promises and
    `AlertItem` reads — with the base station's own wire fields carried
    alongside it so a client can tell a Watch from an Alert and see what
    triggered it. Unknown keys are ignored by the app, so the extras are
    additive.

    `node_id` filters by involvement, not by authorship: an alert raised by a
    base station *about* `bench-01` is returned for `bench-01`. Multi-node
    alerts carry `node_id = "network"` with the confirming nodes in `nodes`, so
    the confirming-node match is done on the stored JSON list. That match uses
    INSTR on the quoted id rather than json_each(), because JSON1 is a
    compile-time option of the SQLite build and the production database is
    whatever the VPS ships — a self-contained string test cannot be missing at
    runtime. The ids are constrained to `[A-Za-z0-9._-]` at both ends (the
    ingest worker's NODE_ID_RE), so no id can contain a quote and the test
    cannot be spoofed by a crafted id.
    """
    sql = (
        "SELECT alert_id, publisher, node_id, event, type, severity, state, title, detail, "
        "pm25, baseline, window_min, nodes_json, observed_utc, created_utc, updated_utc "
        "FROM alerts"
    )
    params: list = []
    if node_id is not None:
        sql += " WHERE node_id = ? OR INSTR(nodes_json, ?) > 0"
        params.extend([node_id, f'"{node_id}"'])
    sql += " ORDER BY observed_utc DESC, id DESC LIMIT ?"
    params.append(limit)

    alerts: list[dict] = []
    for row in conn.execute(sql, params).fetchall():
        alerts.append(
            {
                "alert_id": row["alert_id"],
                "type": row["type"],
                "node_id": row["node_id"],
                "severity": row["severity"],
                "state": row["state"],
                "title": row["title"],
                "detail": row["detail"],
                "created_utc": row["created_utc"],
                "updated_utc": row["updated_utc"],
                # wire fields (additive; the app ignores what it does not use)
                "event": row["event"],
                "observed_utc": row["observed_utc"],
                "pm25": row["pm25"],
                "baseline": row["baseline"],
                "window_min": row["window_min"],
                "nodes": _json_list(row["nodes_json"]),
                "publisher": row["publisher"],
            }
        )
    return alerts


def _json_list(text: str | None) -> list:
    """Parse a stored JSON list, degrading to `[]` rather than raising."""
    if not text:
        return []
    try:
        value = json.loads(text)
    except (ValueError, TypeError):
        return []
    return value if isinstance(value, list) else []


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
