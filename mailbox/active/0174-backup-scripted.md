---
task_id: "0174"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 6h
proof: []
---

# 0174 — Whole backup scripted and on GitHub: MQTT backup + restore docs

## Assignment

- Hermes: make the repo changes below, push to the branch. No VPS access needed — Juno deploys the repo scripts to the VPS and tests the run after this stages.
- Newton: not involved.

## Context

Recovery method for the nordtronics VPS (Contabo 89.117.21.105, Ubuntu 24.04) is: replay the setup scripts, then apply the backed-up data. Two runbooks already exist and are copy-pasteable: `docs/vps-hardening-runbook.md` (provisioning through TLS) and `backend/DEPLOY.md` (456 lines, app install steps 1–12). What is missing:

1. The daily backup scripts live ONLY on the VPS at `/home/deploy/bin/db-backup.sh` and `/home/deploy/bin/db-backup.py`. They are not in the repo. The repo must become their source of truth.
2. The backup covers only the SQLite sensor database. The MQTT broker's persistence file (`/var/lib/mosquitto/mosquitto.db`, owned `mosquitto:mosquitto 0600`, `persistence true`) is backed up nowhere.
3. `backend/DEPLOY.md` has no restore procedure. Step 6 creates an EMPTY database; nothing documents restoring a backup dump into it.

Current scripts, verbatim from the VPS (use these as the starting point — do not re-derive them):

`/home/deploy/bin/db-backup.sh`:
```bash
#!/bin/bash
# Daily wildfire sensor DB dump. Creates a dated gzip in /home/deploy/backups/.
# Juno pushes it to the nordtronics repo (VPS has no push credentials by design).
set -euo pipefail
DATE=$(date -u +%Y%m%dT%H%M%SZ)
sudo -n python3 /home/deploy/bin/db-backup.py "/tmp/wildfire-${DATE}.db"
sudo -n chown deploy:deploy "/tmp/wildfire-${DATE}.db"
gzip -9 "/tmp/wildfire-${DATE}.db"
mkdir -p /home/deploy/backups
mv "/tmp/wildfire-${DATE}.db.gz" /home/deploy/backups/
# keep last 7 local dumps
ls -t /home/deploy/backups/wildfire-*.db.gz 2>/dev/null | tail -n +8 | xargs -r rm
echo "dumped /home/deploy/backups/wildfire-${DATE}.db.gz"
```

`/home/deploy/bin/db-backup.py`:
```python
import sqlite3, sys
src = sqlite3.connect("file:/var/lib/nordtronics/wildfire.db?mode=ro", uri=True)
dst = sqlite3.connect(sys.argv[1])
src.backup(dst)
dst.close(); src.close()
print("backup ok")
```

Facts the implementation must respect:
- The daily run is driven by Juno's cron (`wildfire-db-backup`, daily 03:15), which SSHes in, runs `db-backup.sh`, and commits the dump to `backups/` in this repo. The script itself runs as user `deploy` with passwordless sudo (`sudo -n`).
- The broker persistence file is 240 bytes today (no live nodes yet, nothing retained). The SQLite database is the system of record; broker state is reconstructable because nodes republish on their next check-in. A live copy of `mosquitto.db` may be mid-write — that is acceptable here and must be documented as such, not silently assumed safe.
- Secrets are never stored in git: `/etc/mosquitto/passwd` and `/etc/nordtronics/ingest.env` are generated fresh during DEPLOY.md steps 5 and 7. A replay means new passwords and re-entering the node password at the base station.
- The repo's `backups/` directory already holds the daily DB dumps (30 files). The new MQTT dumps go to the same local directory (`/home/deploy/backups/`) so the same cron flow picks them up.

## Task

On branch `hermes/0174-backup-scripted`, do all three:

1. **Commit the backup scripts to the repo** under `backend/scripts/` as `db-backup.sh` and `db-backup.py`, with the MQTT addition below. The repo copy is the source of truth; the VPS copy is a deployment of it.
2. **Add the MQTT broker persistence backup to `db-backup.sh`:** after the DB dump, `sudo -n cp /var/lib/mosquitto/mosquitto.db` to a temp file, `sudo -n chown deploy:deploy`, gzip it to `/home/deploy/backups/mosquitto-${DATE}.db.gz`, keep the last 7 the same way as the DB dumps, and echo the dumped path. Add a comment stating the live copy may be mid-write and that this is acceptable because the SQLite database is the system of record and nodes republish retained messages on their next check-in.
3. **Add a `## 13. Restore (disaster recovery)` section to `backend/DEPLOY.md`** documenting the full replay, in order:
   - Fresh box: `docs/vps-hardening-runbook.md`, then DEPLOY.md steps 1–12 — EXCEPT step 6 creates an empty database, so after step 6 and before starting the services in step 8, restore the database: stop `wildfire-ingest` and `wildfire-api` if running, `gunzip -c` the latest `backups/wildfire-*.db.gz` dump over `/var/lib/nordtronics/wildfire.db`, then `chown wildfire-ingest:wildfire-data` and `chmod 0660` on it (same ownership/mode step 6 sets).
   - Restore the broker state the same way: stop `mosquitto`, copy the latest `backups/mosquitto-*.db.gz` dump (gunzipped) over `/var/lib/mosquitto/mosquitto.db`, `chown mosquitto:mosquitto`, `chmod 0600`, start `mosquitto`.
   - Secrets: regenerate the MQTT passwords (step 5) and `ingest.env` (step 7) — new passwords; re-enter the node password at the base station.
   - Verify with the step 10 checks (`systemctl is-active`, `/healthz`, a test publish through the real path).

## Success criteria

1. `backend/scripts/db-backup.sh` and `backend/scripts/db-backup.py` exist on the branch; the `.sh` contains the MQTT backup step with the mid-write comment.
2. `backend/DEPLOY.md` has a section 13 covering, in order: runbook + steps 1–12 replay, DB restore with the stop/copy/own/0660 sequence, broker restore with the stop/copy/own/0600 sequence, secrets regeneration, step 10 verification.
3. The shell script passes `bash -n` (no execution on the VPS — Hermes has no VPS access; Juno tests the run after staging).

## Constraints

- Push only to branch `hermes/0174-backup-scripted`. Do NOT merge to main.
- Do not put any secret, password, key, or credential value in the repo — the scripts and docs must contain none. Passwords are generated at deploy time only.
- Do not change the DB dump behavior (same filenames, same 7-file retention, same `/home/deploy/backups/` location). Mirror it for the MQTT dumps.
- Do not invent wire colors, IP addresses, or hostnames beyond what this spec and the two runbooks already state.

## Proof

- branch: `hermes/0174-backup-scripted`, tip SHA
- files: `backend/scripts/db-backup.sh`, `backend/scripts/db-backup.py`, `backend/DEPLOY.md`
- run: none expected (no workflow covers these paths); confirm live with `gh run list` and state the result.
- check: `bash -n backend/scripts/db-backup.sh` output (paste the result, not just the claim).

## Reply format

```
DONE — 0174 whole backup scripted
branch: hermes/0174-backup-scripted @ <sha>
files:
  - backend/scripts/db-backup.sh
  - backend/scripts/db-backup.py
  - backend/DEPLOY.md (section 13 added)
checks: <bash -n result, what you verified>
notes: <anything unclear, any assumption made, CI run-list result>
cost: <per protocol 1.1.0 cost format>
```
