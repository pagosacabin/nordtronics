#!/usr/bin/env bash
# Pre-flash guard for the nordtronics ESPHome skeleton.
#
# Why this exists: secrets.yaml.example ships a VALID-FORMAT placeholder key so
# `esphome config` / `esphome compile` can be verified against the skeleton as
# committed. A valid-format placeholder is also a *usable* one — flash it by
# accident and every board shares a publicly known API encryption key. This guard
# makes that default fail closed at the only step that matters: before flashing.
#
#   ./check-secrets.sh [secrets.yaml] [config.yaml]
#
# Exit codes: 0 ok (warnings allowed), 2 no/unreadable secrets file,
#             3 placeholder key still in place, 4 key is not 32 bytes base64.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
secrets="${1:-$here/secrets.yaml}"
config="${2:-$here/esphome-base.yaml}"
example="$here/secrets.yaml.example"

read_key() {  # $1 = file
  [ -r "$1" ] || return 0
  sed -n 's/^[[:space:]]*api_encryption_key:[[:space:]]*["'"'"']\{0,1\}\([^"'"'"'#]*\)["'"'"']\{0,1\}.*$/\1/p' "$1" | head -n 1 | tr -d '[:space:]'
}

if [ ! -r "$secrets" ]; then
  printf 'REFUSED: no %s.\nGenerate one per device (keep it out of git, mode 600):\n  printf '"'"'api_encryption_key: "%%s"\\n'"'"' "$(python3 -c '"'"'import base64,os;print(base64.b64encode(os.urandom(32)).decode())'"'"')" > %s\n  chmod 600 %s\n' \
    "$secrets" "${secrets##*/}" "${secrets##*/}"
  exit 2
fi

key="$(read_key "$secrets")"
placeholder="$(read_key "$example")"

if [ -z "$key" ]; then
  printf 'REFUSED: %s has no api_encryption_key entry.\n' "$secrets"
  exit 2
fi

if [ -n "$placeholder" ] && [ "$key" = "$placeholder" ]; then
  printf 'REFUSED: %s still holds the committed example key.\n' "$secrets"
  printf 'It is public — every board flashed with it shares one API encryption key.\n'
  printf 'Generate a real one:\n  printf '"'"'api_encryption_key: "%%s"\\n'"'"' "$(python3 -c '"'"'import base64,os;print(base64.b64encode(os.urandom(32)).decode())'"'"')" > %s\n' "${secrets##*/}"
  exit 3
fi

bytes="$(printf '%s' "$key" | base64 -d 2>/dev/null | wc -c || true)"
if [ "${bytes:-0}" -ne 32 ]; then
  printf 'REFUSED: api_encryption_key decodes to %s bytes; ESPHome requires exactly 32 (44 base64 chars).\n' "${bytes:-0}"
  exit 4
fi

# Non-fatal: the bench setup-AP default. It guards only the provisioning page,
# but it is public, so a device that leaves the bench should carry its own.
ap="$(sed -n 's/^[[:space:]]*setup_ap_password:[[:space:]]*"\?\([^"#]*\)"\?.*$/\1/p' "$config" 2>/dev/null | head -n 1 | tr -d '[:space:]')"
if [ "$ap" = "provision-me" ]; then
  printf 'WARNING: setup_ap_password is the public bench default (provision-me).\n'
  printf 'Set a unique value per board before it leaves the bench.\n'
fi

printf 'OK: %s carries a per-device key (32 bytes); safe to flash.\n' "${secrets##*/}"
