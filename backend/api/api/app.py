"""FastAPI application for the wildfire API."""

from __future__ import annotations

import logging
from contextlib import contextmanager
from datetime import datetime, timezone

from fastapi import FastAPI, HTTPException, Path, Query, Request
from fastapi.responses import JSONResponse

from common import db as db_module

from . import store
from .config import (
    DEFAULT_READINGS_LIMIT,
    MAX_READINGS_LIMIT,
    ApiConfig,
)

LOG = logging.getLogger("wildfire.api")

#: node ids are validated with the same rule the ingest worker uses
NODE_ID_PATTERN = r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$"


def normalize_since(value: str) -> str:
    """Normalise an ISO-8601 `since` filter to the stored fixed-width form."""
    text = value.strip()
    if text.endswith(("Z", "z")):
        text = text[:-1] + "+00:00"
    try:
        parsed = datetime.fromisoformat(text)
    except ValueError:
        raise HTTPException(
            status_code=400,
            detail="since must be an ISO-8601 timestamp, e.g. 2026-09-24T00:00:00Z",
        ) from None
    if parsed.tzinfo is None:
        parsed = parsed.replace(tzinfo=timezone.utc)
    return parsed.astimezone(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def create_app(config: ApiConfig | None = None) -> FastAPI:
    config = config or ApiConfig.from_env()

    app = FastAPI(
        title="Nordtronics Wildfire API",
        version="1.0.0",
        description="Node status and telemetry history for the wildfire early-warning network.",
    )
    app.state.config = config

    @contextmanager
    def read_connection():
        """One read-only connection per request — never a shared handle."""
        try:
            conn = db_module.connect(config.db_path, read_only=True)
        except FileNotFoundError as exc:
            LOG.error("%s", exc)
            raise HTTPException(status_code=503, detail="telemetry database is not available") from None
        try:
            yield conn
        finally:
            conn.close()

    @app.exception_handler(Exception)
    async def unhandled(request: Request, exc: Exception):  # pragma: no cover - safety net
        LOG.exception("unhandled error on %s %s", request.method, request.url.path)
        return JSONResponse(status_code=500, content={"detail": "internal error"})

    @app.get("/healthz", tags=["ops"])
    def healthz():
        try:
            with read_connection() as conn:
                version = store.schema_version(conn)
        except HTTPException as exc:
            return JSONResponse(
                status_code=exc.status_code,
                content={"status": "degraded", "database": "unavailable", "detail": exc.detail},
            )
        return {
            "status": "ok",
            "database": "ok",
            "schema_version": version,
            "generated_utc": store.utc_now_iso(),
        }

    @app.get("/v1/nodes", tags=["telemetry"])
    def get_nodes():
        """Every node the network has heard from, newest reading included."""
        with read_connection() as conn:
            nodes = store.list_nodes(conn, config.stale_after_seconds)
        return {
            "nodes": nodes,
            "count": len(nodes),
            "stale_after_seconds": config.stale_after_seconds,
            "generated_utc": store.utc_now_iso(),
        }

    @app.get("/v1/nodes/{node_id}/readings", tags=["telemetry"])
    def get_readings(
        node_id: str = Path(..., pattern=NODE_ID_PATTERN),
        limit: int = Query(DEFAULT_READINGS_LIMIT, ge=1, le=MAX_READINGS_LIMIT),
        since: str | None = Query(None, description="ISO-8601 lower bound on recorded_utc"),
    ):
        """Reading history for one node, newest first."""
        since_normalized = normalize_since(since) if since is not None else None
        with read_connection() as conn:
            if not store.node_exists(conn, node_id):
                raise HTTPException(status_code=404, detail=f"unknown node: {node_id}")
            readings = store.list_readings(conn, node_id, limit=limit, since=since_normalized)
        return {
            "node_id": node_id,
            "count": len(readings),
            "limit": limit,
            "since": since_normalized,
            "readings": readings,
            "generated_utc": store.utc_now_iso(),
        }

    return app


def main() -> int:  # pragma: no cover - exercised by systemd, not by tests
    import uvicorn

    config = ApiConfig.from_env()
    logging.basicConfig(
        level=getattr(logging, config.log_level, logging.INFO),
        format="%(asctime)s %(levelname)s %(name)s %(message)s",
    )
    # loopback only: nginx terminates TLS and proxies to this socket
    uvicorn.run(create_app(config), host=config.host, port=config.port, log_level=config.log_level.lower())
    return 0


if __name__ == "__main__":  # pragma: no cover
    raise SystemExit(main())
