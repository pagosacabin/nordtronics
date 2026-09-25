"""Wildfire telemetry ingest worker.

Subscribes to `nordtronics/wildfire/<node-id>/telemetry` on the Mosquitto
broker, validates each payload, and appends it to the telemetry SQLite
database. Runs as the `wildfire-ingest` service user.
"""

__version__ = "1.0.0"
