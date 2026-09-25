"""Pytest bootstrap for the backend tree.

Mirrors the production import wiring exactly:

* the systemd units set `WorkingDirectory=/opt/nordtronics/backend/<service>`
  and `PYTHONPATH=/opt/nordtronics/backend`, so `python -m <service>.worker`
  finds its own package via the working directory and the shared `common`
  package via PYTHONPATH;
* the tests therefore need `backend/` (for `common`) plus each service
  directory (for `ingest` / `api`) on `sys.path`.
"""

import sys
from pathlib import Path

BACKEND_ROOT = Path(__file__).resolve().parent

for path in (BACKEND_ROOT, BACKEND_ROOT / "ingest", BACKEND_ROOT / "api"):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))
