"""Runtime configuration for the API, read from the environment."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Mapping

DEFAULT_DB_PATH = "/var/lib/nordtronics/wildfire.db"

#: a node that has not reported within this window is "stale" in /v1/nodes
DEFAULT_STALE_AFTER_SECONDS = 900

#: hard ceiling on readings per request, so one client cannot pull the table
MAX_READINGS_LIMIT = 1000
DEFAULT_READINGS_LIMIT = 100


def _lookup(env: Mapping[str, str], name: str, default: str) -> str:
    value = env.get(name)
    return default if value in (None, "") else value


@dataclass
class ApiConfig:
    db_path: str
    host: str
    port: int
    stale_after_seconds: int
    log_level: str

    @classmethod
    def from_env(cls, env: Mapping[str, str] | None = None) -> "ApiConfig":
        import os

        env = os.environ if env is None else env

        port_text = _lookup(env, "WILDFIRE_API_PORT", "8000")
        try:
            port = int(port_text)
        except ValueError:
            raise ValueError(f"WILDFIRE_API_PORT must be an integer, got {port_text!r}") from None

        stale_text = _lookup(env, "WILDFIRE_STALE_AFTER_SECONDS", str(DEFAULT_STALE_AFTER_SECONDS))
        try:
            stale_after = int(stale_text)
        except ValueError:
            raise ValueError(
                f"WILDFIRE_STALE_AFTER_SECONDS must be an integer, got {stale_text!r}"
            ) from None
        if stale_after < 0:
            raise ValueError("WILDFIRE_STALE_AFTER_SECONDS must not be negative")

        return cls(
            db_path=_lookup(env, "WILDFIRE_DB_PATH", DEFAULT_DB_PATH),
            host=_lookup(env, "WILDFIRE_API_HOST", "127.0.0.1"),
            port=port,
            stale_after_seconds=stale_after,
            log_level=_lookup(env, "WILDFIRE_LOG_LEVEL", "INFO").upper(),
        )
