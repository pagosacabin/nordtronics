#!/bin/sh
# certbot deploy hook — rebuild the ingest worker's pinned CA bundle and reload
# the MQTT broker after a renewal.
#
# Installed as /etc/letsencrypt/renewal-hooks/deploy/00-reload-mosquitto.sh
# (see backend/DEPLOY.md). Read access for the unprivileged `mosquitto` user is
# granted once with setfacl -d, so renewed key files inherit it.
#
# A renewal changes two things, so the hook has two jobs:
#
#   1. Rebuild /etc/nordtronics/mqtt-ca.pem. That bundle is chain.pem plus the
#      SELF-SIGNED ISRG Root X1: chain.pem on its own ends at a cross-signed
#      ISRG Root X2 whose issuer (X1) is not in the file, so a client that
#      trusts it fails the handshake with "unable to get issuer certificate".
#      Rebuilding it here is what stops a renewal from leaving the worker with
#      a CA bundle that no longer matches the chain the broker serves.
#   2. Make both TLS clients pick that up: reload the broker so it serves the
#      new certificate, and restart the ingest worker, which holds the CA it
#      read at startup.
set -e

CERT_DIR=/etc/letsencrypt/live/nordtronics.io
CA_BUNDLE=/etc/nordtronics/mqtt-ca.pem
ROOT_X1=/usr/share/ca-certificates/mozilla/ISRG_Root_X1.crt

# Fail loudly rather than write a bundle with no trust anchor in it: a hook
# that silently produced an unanchored CA would take the worker down at its
# next restart, long after the renewal that caused it.
if [ ! -r "$ROOT_X1" ]; then
    echo "renew-hook: trust anchor $ROOT_X1 is missing — not rebuilding $CA_BUNDLE" >&2
    exit 1
fi

install -d -o root -g root -m 0755 /etc/nordtronics
# The bundle is rewritten in place, so it keeps its inode and its permissions
# across renewals; the chown/chmod below make that true on first creation too.
cat "$CERT_DIR/chain.pem" "$ROOT_X1" > "$CA_BUNDLE"
chown root:wildfire-data "$CA_BUNDLE"
chmod 0640 "$CA_BUNDLE"

systemctl is-active --quiet mosquitto && systemctl reload mosquitto
systemctl is-active --quiet wildfire-ingest && systemctl restart wildfire-ingest

exit 0
