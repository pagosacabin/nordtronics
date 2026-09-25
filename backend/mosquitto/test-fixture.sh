#!/usr/bin/env bash
# TEST FIXTURE — not part of the deployment.
#
# The Mosquitto config references files that only exist on a deployed server:
# the Let's Encrypt certificate, the password file created with
# mosquitto_passwd, and the ACL. This script fabricates them on a throwaway
# machine (CI runner or a container) so `smoke-test.sh` can start the real
# config and probe it.
#
# It deliberately mirrors the deploy-time steps in backend/DEPLOY.md, including
# the Setfacl step that lets the unprivileged `mosquitto` user read certbot's
# root-only private key — so if that step is wrong, CI fails here.
#
# Never run this on the VPS: it would overwrite the real certificate with a
# self-signed one. Requires root.
set -euo pipefail

DOMAIN="${DOMAIN:-nordtronics.io}"
LE_DIR="/etc/letsencrypt/live/$DOMAIN"
ARCHIVE="/etc/letsencrypt/archive/$DOMAIN"
MOSQUITTO_DIR="/etc/mosquitto"
DATA_DIR="/var/lib/mosquitto"

# obviously fake, CI-only credentials — the real ones are created at deploy time
INGEST_USER="wildfire-ingest"
NODE_USERS="node-01 node-02"
TEST_PASSWORD="ci-fixture-not-a-secret"

[[ "$(id -u)" == "0" ]] || { echo "test-fixture.sh must run as root" >&2; exit 1; }

echo "== certificate (self-signed stand-in for $DOMAIN) =="
mkdir -p "$LE_DIR" "$ARCHIVE"
openssl req -x509 -newkey rsa:2048 -sha256 -days 2 -nodes \
  -keyout "$ARCHIVE/privkey1.pem" -out "$ARCHIVE/fullchain1.pem" \
  -subj "/CN=mqtt.$DOMAIN" \
  -addext "subjectAltName=DNS:mqtt.$DOMAIN,DNS:$DOMAIN,DNS:localhost,IP:127.0.0.1" 2>/dev/null
ln -sf "../../archive/$DOMAIN/fullchain1.pem" "$LE_DIR/fullchain.pem"
ln -sf "../../archive/$DOMAIN/privkey1.pem" "$LE_DIR/privkey.pem"
ln -sf "../../archive/$DOMAIN/fullchain1.pem" "$LE_DIR/chain.pem"

# certbot's own permissions: the private key is root-only and unreadable by the
# broker, which is exactly why DEPLOY.md grants access with an ACL.
chmod 0755 /etc/letsencrypt /etc/letsencrypt/live /etc/letsencrypt/archive \
  "$LE_DIR" "$ARCHIVE"
chmod 0644 "$ARCHIVE/fullchain1.pem"
chmod 0600 "$ARCHIVE/privkey1.pem"

echo "== granting the mosquitto user read access (DEPLOY.md step) =="
setfacl -R  -m u:mosquitto:rX /etc/letsencrypt/live /etc/letsencrypt/archive
setfacl -R -d -m u:mosquitto:rX /etc/letsencrypt/live /etc/letsencrypt/archive

echo "== password file =="
mkdir -p "$MOSQUITTO_DIR"
mosquitto_passwd -c -b "$MOSQUITTO_DIR/passwd" "$INGEST_USER" "$TEST_PASSWORD"
for user in $NODE_USERS; do
  mosquitto_passwd -b "$MOSQUITTO_DIR/passwd" "$user" "$TEST_PASSWORD"
done
chown root:mosquitto "$MOSQUITTO_DIR/passwd"
chmod 0640 "$MOSQUITTO_DIR/passwd"

echo "== acl =="
cp "$(dirname "$(readlink -f "$0")")/acl" "$MOSQUITTO_DIR/acl"
chown root:mosquitto "$MOSQUITTO_DIR/acl"
chmod 0640 "$MOSQUITTO_DIR/acl"

echo "== persistence directory =="
mkdir -p "$DATA_DIR"
chown mosquitto:mosquitto "$DATA_DIR"
chmod 0750 "$DATA_DIR"

echo "test fixture ready"
