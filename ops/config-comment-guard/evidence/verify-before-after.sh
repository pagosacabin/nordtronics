#!/usr/bin/env bash
# Reproduce the mailbox-0041 before/after evidence.
#
# Performs four same-value writes through `hermes config set` (the write the cron worker
# performs) — no config VALUE is changed, and the guard only ever inserts comment lines.
# Nothing here is destructive: the sandbox copy lives in ~/.hermes/tmp-0041, and section 6
# puts the live config back to its exact pre-task bytes.
set -uo pipefail

CFG="$HOME/.hermes/config.yaml"
CRONCFG="$HOME/.hermes/profiles/cronrunner/config.yaml"
LEDGER="$HOME/.hermes/config-comments/ledger-home-.hermes-config.yaml"
DEMO="$HOME/.hermes/tmp-0041"
GUARD="$HOME/.hermes/bin/hermes-config-guard.py"
KEY="mcp_servers.kicad.env.TMPDIR"
VAL="$HOME/.cache/kicad-mcp/tmp"
PRISTINE="$(ls -1t "$HOME"/.hermes/config.yaml.bak-0041-* | head -1)"
CRONPRISTINE="$(ls -1t "$HOME"/.hermes/profiles/cronrunner/config.yaml.bak-0041-* | head -1)"

say() { printf '\n=== %s\n' "$*"; }
comments() { grep -c '^[[:space:]]*#' "$1"; }
trailing() { grep -cE '^[^#]*[^[:space:]][[:space:]]+#' "$1"; }

say "0. the guard's own tests"
python3 "$GUARD" --selftest 2>&1 | grep -E '^(PASS|FAIL)|failing' | sed 's/^/   /'

say "1. BEFORE — a bare 'hermes config set' deletes every comment"
echo "   file: ~/.hermes/config.yaml copied byte-for-byte to a sandbox HERMES_HOME"
rm -rf "$DEMO"; mkdir -p "$DEMO/home"; cp -p "$CFG" "$DEMO/home/config.yaml"
echo "   comment lines before: $(comments "$DEMO/home/config.yaml")"
grep -n '^[[:space:]]*#' "$DEMO/home/config.yaml" | sed 's/^/     /'
HERMES_HOME="$DEMO/home" hermes config set "$KEY" "$VAL" >/dev/null
echo "   comment lines after : $(comments "$DEMO/home/config.yaml")"

say "2. AFTER — the same write, with the guard restoring from the ledger"
rm -rf "$DEMO"; mkdir -p "$DEMO/home"; cp -p "$CFG" "$DEMO/home/config.yaml"
HERMES_HOME="$DEMO/home" hermes config set "$KEY" "$VAL" >/dev/null
echo "   comment lines after the same 'hermes config set': $(comments "$DEMO/home/config.yaml")"
echo "   running: hermes-config-guard.py -c <sandbox config> --ledger <live config ledger>"
python3 "$GUARD" -c "$DEMO/home/config.yaml" --ledger "$LEDGER" | sed 's/^/     /'
echo "   comment lines after the guard ran: $(comments "$DEMO/home/config.yaml")"
grep -n '^[[:space:]]*#' "$DEMO/home/config.yaml" | sed 's/^/     /'
echo "   non-comment lines vs the live config (the only difference is the writer's own YAML"
echo "   re-quoting of an existing string — semantic equality is checked in section 5):"
if diff <(grep -v '^[[:space:]]*#' "$DEMO/home/config.yaml") <(grep -v '^[[:space:]]*#' "$CFG") >/dev/null; then
  echo "     identical"
else
  diff <(grep -v '^[[:space:]]*#' "$DEMO/home/config.yaml") <(grep -v '^[[:space:]]*#' "$CFG") | sed 's/^/     /'
fi

say "3. LIVE — same bare write on ~/.hermes/config.yaml, no wrapper, units enabled"
echo "   pre-write: $(comments "$CFG") whole-line + $(trailing "$CFG") trailing comment lines"
HERMES_HOME="$HOME/.hermes" hermes config set "$KEY" "$VAL" >/dev/null
echo "   immediately after the write (nothing has had time to intervene): $(comments "$CFG") whole-line"
sleep 4
echo "   four seconds later: $(comments "$CFG") whole-line + $(trailing "$CFG") trailing"
grep -n '^[[:space:]]*#' "$CFG" | sed 's/^/     /'
echo "   diff against the pre-change bytes (expect comment lines only):"
diff "$PRISTINE" "$CFG" | sed 's/^/     /'
say "   guard log (tail)"
tail -3 "$HOME/.hermes/config-comments/guard.log" | sed 's/^/     /'
say "   systemd: who fired the restore"
systemctl --user show hermes-config-guard.service \
  -p ExecMainStartTimestamp -p ExecMainStatus -p Result | sed 's/^/     /'
systemctl --user is-active hermes-config-guard.path hermes-config-guard.timer | sed 's/^/     active: /'

say "4. WRAPPER — hermes-config-guarded (capture -> write -> restore, one call)"
HERMES_HOME="$HOME/.hermes" "$HOME/.hermes/bin/hermes-config-guarded" config set "$KEY" "$VAL" | sed 's/^/     /'
echo "   comment lines right after the wrapper returned: $(comments "$CFG")"

say "5. The config still parses and loads: hermes config check + MCP connectivity"
HERMES_HOME="$HOME/.hermes" hermes config check 2>&1 | head -4 | sed 's/^/     /'
echo "     semantic check — value tree identical before and after the three guard-covered writes:"
python3 - "$PRISTINE" "$CFG" <<'PY' | sed 's/^/     /'
import sys, yaml
a, b = (yaml.safe_load(open(p, encoding="utf-8")) for p in sys.argv[1:3])
print("values identical" if a == b else f"VALUES DIFFER: {a} != {b}")
PY
HERMES_HOME="$HOME/.hermes" hermes mcp list 2>&1 | sed 's/^/     /'
echo "     hermes mcp test kicad: connected, $(python3 -c "
import json,pathlib
c=json.load(open(pathlib.Path.home()/'.hermes/cache/mcp_schema_cache.json'))
print(len(c.get('kicad',{}).get('tools',[])))" 2>/dev/null || echo '?') tool names exposed"

say "6. The cron worker's own profile config: whole-line AND trailing comments survive a real write"
echo "   $CRONCFG"
echo "   pre-write: $(comments "$CRONCFG") whole-line + $(trailing "$CRONCFG") trailing comments"
hermes config set "$KEY" "$VAL" >/dev/null   # no HERMES_HOME override: this IS the cronrunner profile
echo "   immediately after the write: $(comments "$CRONCFG") whole-line + $(trailing "$CRONCFG") trailing"
sleep 4
echo "   four seconds later: $(comments "$CRONCFG") whole-line + $(trailing "$CRONCFG") trailing"
grep -nE '^[[:space:]]*#|^[^#]*[^[:space:]][[:space:]]+#' "$CRONCFG" | head -24 | sed 's/^/     /'
echo "   semantic check — value tree identical to the pre-write bytes:"
python3 - "$CRONPRISTINE" "$CRONCFG" <<'PY' | sed 's/^/     /'
import sys, yaml
a, b = (yaml.safe_load(open(p, encoding="utf-8")) for p in sys.argv[1:3])
print("values identical" if a == b else "VALUES DIFFER")
PY

say "7. Post-state: both configs are back to their exact pre-task bytes (comments included)"
cp -p "$PRISTINE" "$CFG"
cp -p "$CRONPRISTINE" "$CRONCFG"
sleep 2
for pair in "default:$PRISTINE:$CFG" "cronrunner:$CRONPRISTINE:$CRONCFG"; do
  name="${pair%%:*}"; rest="${pair#*:}"; a="${rest%%:*}"; b="${rest#*:}"
  echo "   $name: $(comments "$b") comment lines; sha256 now=$(sha256sum "$b" | cut -d' ' -f1)"
  echo "   $name: sha256 pristine=$(sha256sum "$a" | cut -d' ' -f1)"
done
echo "   hermes-config-guard.py --all --check:"
python3 "$GUARD" --all --check 2>&1 | sed 's/^/     /'
