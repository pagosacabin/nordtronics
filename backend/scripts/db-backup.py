"""Consistent copy of the live wildfire SQLite database.

Deployed to /home/deploy/bin/db-backup.py by backend/scripts/db-backup.sh's
daily run on the VPS; this repo copy is the source of truth (see
backend/DEPLOY.md section 13).

sqlite3's backup API reads a live database under a consistent snapshot, so the
copy is safe to take while the ingest worker and the API are running.
"""
import sqlite3, sys

src = sqlite3.connect("file:/var/lib/nordtronics/wildfire.db?mode=ro", uri=True)
dst = sqlite3.connect(sys.argv[1])
src.backup(dst)
dst.close(); src.close()
print("backup ok")
