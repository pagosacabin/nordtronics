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
# From another project directory, name both files (the guard stays here, in the
# skeleton, so a project copy cannot drift from it):
#   firmware/skeleton/check-secrets.sh firmware/<project>/secrets.yaml \
#                                      firmware/<project>/<project>.yaml
#
# Exit codes: 0 ok (warnings allowed), 2 no/unreadable secrets file or no key,
#             3 placeholder key still in place, 4 key is not canonical 32-byte base64.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
secrets="${1:-$here/secrets.yaml}"
config="${2:-$here/esphome-base.yaml}"
example="$here/secrets.yaml.example"

# Read one YAML scalar. A QUOTED value is returned exactly as written — trailing or
# inner whitespace included — so the validator sees precisely what the file holds. An
# unquoted value is trimmed the way YAML itself trims a plain scalar, and a trailing
# `# comment` is dropped.
read_field() {  # $1 = file, $2 = key
  [ -r "$1" ] || return 0
  local v
  v="$(sed -n "s/^[[:space:]]*$2:[[:space:]]*\"\(.*\)\"[[:space:]]*\$/\1/p" "$1" | head -n 1)"
  if [ -z "$v" ]; then
    v="$(sed -n "s/^[[:space:]]*$2:[[:space:]]*'\(.*\)'[[:space:]]*\$/\1/p" "$1" | head -n 1)"
  fi
  if [ -z "$v" ]; then
    v="$(sed -n "s/^[[:space:]]*$2:[[:space:]]*\([^#]*\).*\$/\1/p" "$1" | head -n 1)"
    v="${v#"${v%%[![:space:]]*}"}"
    v="${v%"${v##*[![:space:]]}"}"
  fi
  printf '%s' "$v"
}

gen_hint() {
  printf 'Generate one per device (keep it out of git, mode 600):\n'
  printf "  printf 'api_encryption_key: \"%%s\"\\\\n' \"\$(python3 -c 'import base64,os;print(base64.b64encode(os.urandom(32)).decode())')\" > %s\n" "${secrets##*/}"
  printf '  chmod 600 %s\n' "${secrets##*/}"
}

if [ ! -r "$secrets" ]; then
  printf 'REFUSED: no %s.\n' "$secrets"
  gen_hint
  exit 2
fi

key="$(read_field "$secrets" api_encryption_key)"
placeholder="$(read_field "$example" api_encryption_key)"

if [ -z "$key" ]; then
  printf 'REFUSED: %s has no api_encryption_key entry.\n' "$secrets"
  gen_hint
  exit 2
fi

if [ -n "$placeholder" ] && [ "$key" = "$placeholder" ]; then
  printf 'REFUSED: %s still holds the committed example key.\n' "$secrets"
  printf 'It is public — every board flashed with it shares one API encryption key.\n'
  gen_hint
  exit 3
fi

# Strict, canonical validation of the value exactly as the file holds it: whitespace
# inside it, a non-alphabet character, wrong padding, or a non-canonical encoding
# (which `base64 -d` decodes leniently) is refused.
bytes="$(printf '%s' "$key" | python3 -c '
import base64, sys
k = sys.stdin.read()
if not k or any(c.isspace() for c in k):
    print(0); raise SystemExit
try:
    raw = base64.b64decode(k, validate=True)
except Exception:
    print(0); raise SystemExit
print(len(raw) if base64.b64encode(raw).decode() == k else 0)
' 2>/dev/null || echo 0)"

if [ "${bytes:-0}" -ne 32 ]; then
  printf 'REFUSED: api_encryption_key is not canonical 32-byte base64 (44 chars, one "=" pad, no inner whitespace); got %s bytes.\n' "${bytes:-0}"
  printf "  python3 -c 'import base64,os;print(base64.b64encode(os.urandom(32)).decode())'\n"
  exit 4
fi

# Non-fatal: the bench setup-AP default. It guards only the provisioning page, but it
# is public, so a device that leaves the bench should carry its own.
ap="$(read_field "$config" setup_ap_password)"
if [ "$ap" = "provision-me" ]; then
  printf 'WARNING: setup_ap_password is the public bench default (provision-me).\n'
  printf 'Set a unique value per board before it leaves the bench.\n'
fi

printf 'OK: %s carries a per-device key (32 bytes); safe to flash.\n' "${secrets##*/}"
