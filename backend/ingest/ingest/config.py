"""Runtime configuration for the ingest worker, read from the environment.

No secret ever lives in the repository. On the VPS the values below come from
`/etc/nordtronics/ingest.env` (mode 0600, root-owned) referenced by
`EnvironmentFile=` in `wildfire-ingest.service` — see DEPLOY.md.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Mapping

DEFAULT_TOPIC = "nordtronics/wildfire/+/telemetry"
DEFAULT_DB_PATH = "/var/lib/nordtronics/wildfire.db"
DEFAULT_CA_FILE = "/etc/letsencrypt/live/nordtronics.io/chain.pem"


def _lookup(env: Mapping[str, str], name: str, default: str | None = None) -> str | None:
    value = env.get(name)
    return default if value in (None, "") else value


def _require(env: Mapping[str, str], name: str, default: str) -> str:
    value = _lookup(env, name, default)
    assert value is not None  # default is never None here
    return value


@dataclass
class IngestConfig:
    mqtt_host: str
    mqtt_port: int
    mqtt_username: str | None
    mqtt_password: str | None
    mqtt_ca_file: str | None
    mqtt_client_id: str
    topic: str
    db_path: str
    log_level: str

    @classmethod
    def from_env(cls, env: Mapping[str, str] | None = None) -> "IngestConfig":
        import os

        env = os.environ if env is None else env

        port_text = _require(env, "WILDFIRE_MQTT_PORT", "8883")
        try:
            port = int(port_text)
        except ValueError:
            raise ValueError(f"WILDFIRE_MQTT_PORT must be an integer, got {port_text!r}") from None
        if not 1 <= port <= 65535:
            raise ValueError(f"WILDFIRE_MQTT_PORT out of range: {port}")

        username = _lookup(env, "WILDFIRE_MQTT_USERNAME")
        password = _lookup(env, "WILDFIRE_MQTT_PASSWORD")
        if bool(username) != bool(password):
            raise ValueError(
                "WILDFIRE_MQTT_USERNAME and WILDFIRE_MQTT_PASSWORD must be set together"
            )

        return cls(
            mqtt_host=_require(env, "WILDFIRE_MQTT_HOST", "127.0.0.1"),
            mqtt_port=port,
            mqtt_username=username,
            mqtt_password=password,
            mqtt_ca_file=_lookup(env, "WILDFIRE_MQTT_CA", DEFAULT_CA_FILE),
            mqtt_client_id=_require(env, "WILDFIRE_MQTT_CLIENT_ID", "wildfire-ingest"),
            topic=_require(env, "WILDFIRE_TOPIC", DEFAULT_TOPIC),
            db_path=_require(env, "WILDFIRE_DB_PATH", DEFAULT_DB_PATH),
            log_level=_require(env, "WILDFIRE_LOG_LEVEL", "INFO").upper(),
        )
