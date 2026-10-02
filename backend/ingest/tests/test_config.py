"""Ingest configuration tests."""

from __future__ import annotations

import pytest

from ingest.config import DEFAULT_DB_PATH, IngestConfig


def test_defaults_are_loopback_and_tls():
    config = IngestConfig.from_env({})
    assert config.mqtt_host == "127.0.0.1"
    assert config.mqtt_port == 8883
    assert config.topic == "nordtronics/wildfire/+/telemetry"
    assert config.db_path == DEFAULT_DB_PATH
    assert config.mqtt_username is None
    assert config.mqtt_ca_file == "/etc/letsencrypt/live/nordtronics.io/chain.pem"


def test_environment_overrides_are_applied():
    config = IngestConfig.from_env(
        {
            "WILDFIRE_MQTT_HOST": "mqtt.nordtronics.io",
            "WILDFIRE_MQTT_PORT": "8884",
            "WILDFIRE_MQTT_USERNAME": "wildfire-base",
            "WILDFIRE_MQTT_PASSWORD": "hunter2",
            "WILDFIRE_DB_PATH": "/srv/wildfire.db",
            "WILDFIRE_LOG_LEVEL": "debug",
        }
    )
    assert config.mqtt_host == "mqtt.nordtronics.io"
    assert config.mqtt_port == 8884
    assert config.mqtt_username == "wildfire-base"
    assert config.mqtt_password == "hunter2"
    assert config.db_path == "/srv/wildfire.db"
    assert config.log_level == "DEBUG"


def test_blank_values_fall_back_to_defaults():
    config = IngestConfig.from_env({"WILDFIRE_MQTT_HOST": "", "WILDFIRE_MQTT_PORT": ""})
    assert config.mqtt_host == "127.0.0.1"
    assert config.mqtt_port == 8883


@pytest.mark.parametrize("value", ["", "eighty", "0", "70000", "-1"])
def test_bad_port_is_rejected(value):
    if value == "":
        # blank means "use the default" rather than an error
        assert IngestConfig.from_env({"WILDFIRE_MQTT_PORT": value}).mqtt_port == 8883
        return
    with pytest.raises(ValueError):
        IngestConfig.from_env({"WILDFIRE_MQTT_PORT": value})


def test_username_without_password_is_rejected():
    with pytest.raises(ValueError, match="must be set together"):
        IngestConfig.from_env({"WILDFIRE_MQTT_USERNAME": "user"})


def test_password_without_username_is_rejected():
    with pytest.raises(ValueError, match="must be set together"):
        IngestConfig.from_env({"WILDFIRE_MQTT_PASSWORD": "secret"})
