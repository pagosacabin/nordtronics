#!/bin/bash
# Daily wildfire backups: the SQLite sensor database and the Mosquitto broker
# persistence file. Creates dated gzips in /home/deploy/backups/.
# Juno pushes them to the nordtronics repo (the VPS has no push credentials by
# design), so the committed dumps under backups/ are the durable copy.
#
# THIS COPY IS THE SOURCE OF TRUTH. The VPS copy at /home/deploy/bin/db-backup.sh
# is a deployment of it — see backend/DEPLOY.md section 13.
set -euo pipefail
DATE=$(date -u +%Y%m%dT%H%M%SZ)

# --- SQLite sensor database ------------------------------------------------
# The database is the system of record. sqlite3's backup API (db-backup.py)
# takes a consistent copy of a live database, so this dump is authoritative.
sudo -n python3 /home/deploy/bin/db-backup.py "/tmp/wildfire-${DATE}.db"
sudo -n chown deploy:deploy "/tmp/wildfire-${DATE}.db"
gzip -9 "/tmp/wildfire-${DATE}.db"
mkdir -p /home/deploy/backups
mv "/tmp/wildfire-${DATE}.db.gz" /home/deploy/backups/
# keep last 7 local dumps
ls -t /home/deploy/backups/wildfire-*.db.gz 2>/dev/null | tail -n +8 | xargs -r rm
echo "dumped /home/deploy/backups/wildfire-${DATE}.db.gz"

# --- MQTT broker persistence -----------------------------------------------
# /var/lib/mosquitto/mosquitto.db (owned mosquitto:mosquitto 0600) is copied
# while the broker is running, so the live copy MAY BE MID-WRITE: the dump can
# carry a torn tail, and mosquitto does not offer a consistent-snapshot command.
# That is acceptable here and deliberate — the SQLite database above is the
# system of record, and nodes republish their retained messages on their next
# check-in, so broker state is reconstructable rather than authoritative. Never
# treat this dump as a source of truth for anything but the broker's own state,
# and never restore it over a database (backend/DEPLOY.md section 13).
sudo -n cp /var/lib/mosquitto/mosquitto.db "/tmp/mosquitto-${DATE}.db"
sudo -n chown deploy:deploy "/tmp/mosquitto-${DATE}.db"
gzip -9 "/tmp/mosquitto-${DATE}.db"
mv "/tmp/mosquitto-${DATE}.db.gz" /home/deploy/backups/
# keep the last 7, the same rule as the database dumps
ls -t /home/deploy/backups/mosquitto-*.db.gz 2>/dev/null | tail -n +8 | xargs -r rm
echo "dumped /home/deploy/backups/mosquitto-${DATE}.db.gz"
