#!/usr/bin/env bash
# Install the Hermes config comment guard on this machine.
#
#   ops/config-comment-guard/install.sh [--state DIR] [--no-units]
#
# Idempotent: re-run after editing the guard to refresh ~/.hermes/bin copies. Ledgers,
# logs and existing targets.conf are never overwritten.
set -euo pipefail

SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STATE="${HERMES_CONFIG_COMMENTS_STATE:-$HOME/.hermes/config-comments}"
BIN="$HOME/.hermes/bin"
UNITS="$HOME/.config/systemd/user"
TARGETS=("$HOME/.hermes/config.yaml" "$HOME/.hermes/profiles/cronrunner/config.yaml")
NO_UNITS=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --state) STATE="$2"; shift 2 ;;
    --no-units) NO_UNITS=1; shift ;;
    -h|--help) sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

mkdir -p "$BIN" "$STATE"
install -m 0755 "$SRC/hermes-config-guard.py" "$BIN/hermes-config-guard.py"
install -m 0755 "$SRC/hermes-config-guarded" "$BIN/hermes-config-guarded"
echo "installed: $BIN/hermes-config-guard.py, $BIN/hermes-config-guarded"

if [[ ! -f "$STATE/targets.conf" ]]; then
  : > "$STATE/targets.conf"
  for cfg in "${TARGETS[@]}"; do
    [[ -f "$cfg" ]] && printf '%s\n' "$cfg" >> "$STATE/targets.conf"
  done
  echo "wrote $STATE/targets.conf:"
  sed 's/^/  /' "$STATE/targets.conf"
else
  echo "kept existing $STATE/targets.conf"
fi

# Seed/extend every ledger from what the configs currently carry, before any writer can strip it.
python3 "$BIN/hermes-config-guard.py" --state "$STATE" --all --capture

if [[ $NO_UNITS -eq 0 ]]; then
  mkdir -p "$UNITS"
  install -m 0644 "$SRC"/hermes-config-guard.service "$UNITS/"
  install -m 0644 "$SRC"/hermes-config-guard.path "$UNITS/"
  install -m 0644 "$SRC"/hermes-config-guard.timer "$UNITS/"
  systemctl --user daemon-reload
  systemctl --user enable --now hermes-config-guard.path hermes-config-guard.timer
  echo "units enabled:"
  systemctl --user list-unit-files 'hermes-config-guard*' --no-pager | sed 's/^/  /'
fi

echo
echo "verifying (a stripped config exits 1 here, everything present exits 0):"
python3 "$BIN/hermes-config-guard.py" --state "$STATE" --all --check || echo "  ^ some blocks are missing; run the guard without --check to restore"
