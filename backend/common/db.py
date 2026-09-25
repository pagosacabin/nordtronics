"""SQLite access shared by the ingest worker (writer) and the API (reader).

Why SQLite: v1 serves 2-3 field nodes. A single file, no extra daemon, no
network hop, and `sqlite3` is in the standard library — the smallest thing
that meets the requirement. WAL mode lets the API read while the ingest
worker writes.
"""

from __future__ import annotations

import sqlite3
from pathlib import Path

SCHEMA_PATH = Path(__file__).with_name("schema.sql")

#: Kept in step with the `schema_version` row written by schema.sql.
SCHEMA_VERSION = 1


def connect(path: str | Path, *, read_only: bool = False, timeout: float = 10.0) -> sqlite3.Connection:
    """Open the telemetry database.

    `read_only=True` uses SQLite's `mode=ro` URI so the API can never mutate
    telemetry through a bug or a stray statement. Note that WAL readers still
    need write access to the `-shm`/`-wal` sidecars, which is why the API
    shares the `wildfire-data` group with the ingest worker (see DEPLOY.md).
    """
    path = Path(path)
    if read_only:
        if not path.exists():
            raise FileNotFoundError(
                f"telemetry database {path} does not exist — start the ingest worker first"
            )
        conn = sqlite3.connect(
            f"file:{path.as_posix()}?mode=ro", uri=True, timeout=timeout, check_same_thread=False
        )
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        conn = sqlite3.connect(str(path), timeout=timeout, check_same_thread=False)

    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA busy_timeout = 5000")
    conn.execute("PRAGMA foreign_keys = ON")
    if not read_only:
        conn.execute("PRAGMA journal_mode = WAL")
        conn.execute("PRAGMA synchronous = NORMAL")
    return conn


def init_db(path: str | Path) -> sqlite3.Connection:
    """Create/upgrade the schema and return an open read-write connection."""
    conn = connect(path)
    conn.executescript(SCHEMA_PATH.read_text(encoding="utf-8"))
    conn.commit()
    return conn


def schema_version(conn: sqlite3.Connection) -> int:
    row = conn.execute("SELECT value FROM meta WHERE key = 'schema_version'").fetchone()
    return int(row["value"]) if row else 0
