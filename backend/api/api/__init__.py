"""Read-only HTTPS API over the wildfire telemetry database.

Serves the Android companion app. Deliberately small: two endpoints the app
needs today plus a health probe, reading the same SQLite file the ingest
worker writes. It sits behind nginx on api.nordtronics.io and binds to
loopback only — nginx terminates TLS (see DEPLOY.md).
"""

__version__ = "1.0.0"
