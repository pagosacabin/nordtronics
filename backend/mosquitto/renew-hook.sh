#!/bin/sh
# certbot deploy hook — reload the MQTT broker after a renewal.
#
# Installed as /etc/letsencrypt/renewal-hooks/deploy/00-reload-mosquitto.sh
# (see backend/DEPLOY.md). Read access for the unprivileged `mosquitto` user is
# granted once with setfacl -d, so renewed key files inherit it and this hook
# only has to make the broker pick up the new certificate.
set -e
systemctl is-active --quiet mosquitto && systemctl reload mosquitto
