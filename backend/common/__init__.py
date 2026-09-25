"""Shared code for the Nordtronics wildfire backend.

`common` is imported by both the ingest worker and the API so that the SQLite
schema and the connection rules live in exactly one place. It is not a
distributable package: the services put `backend/` on `PYTHONPATH` (see the
systemd units in `backend/*/*.service`), and the tests get the same path from
`backend/conftest.py`.
"""

__all__ = ["db"]
