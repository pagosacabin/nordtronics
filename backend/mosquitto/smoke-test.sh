#!/usr/bin/env bash
# Start the broker with the shipped configuration and prove its security
# properties, rather than only its syntax.
#
#   mosquitto --test-config -c <conf>   # cheap syntax check, mosquitto >= 2.1
#   this script                          # start it and probe it, any 2.x
#
# Both are attempted: `--test-config` when the installed broker supports it,
# then the live probe. Run from anywhere; paths are overridable so the same
# script works in CI on a scratch prefix.
#
# Required environment:
#   MQTT_TEST_NODE_PASS    password for the test node user
#   MQTT_TEST_INGEST_PASS  password for the ingest user
set -euo pipefail

CONF="${MOSQUITTO_CONF:-/etc/mosquitto/nordtronics.conf}"
CAFILE="${MQTT_CA_FILE:-/etc/letsencrypt/live/nordtronics.io/fullchain.pem}"
HOST="${MQTT_HOST:-127.0.0.1}"
PORT="${MQTT_PORT:-8883}"
NODE_USER="${MQTT_TEST_NODE_USER:-node-01}"
OTHER_NODE="node-02"
INGEST_USER="${MQTT_TEST_INGEST_USER:-wildfire-ingest}"
NODE_PASS="${MQTT_TEST_NODE_PASS:?set MQTT_TEST_NODE_PASS}"
INGEST_PASS="${MQTT_TEST_INGEST_PASS:?set MQTT_TEST_INGEST_PASS}"
WORK="$(mktemp -d)"
BROKER_LOG="$WORK/broker.log"
RECEIVED="$WORK/received.txt"
BROKER_PID=""

readonly TOPIC_OWN="nordtronics/wildfire/$NODE_USER/telemetry"
readonly TOPIC_OTHER="nordtronics/wildfire/$OTHER_NODE/telemetry"

pass() { printf '  ok   %s\n' "$1"; }
fail() { printf '  FAIL %s\n' "$1" >&2; exit 1; }

cleanup() {
  if [[ -n "$BROKER_PID" ]] && kill -0 "$BROKER_PID" 2>/dev/null; then
    kill "$BROKER_PID" 2>/dev/null || true
    wait "$BROKER_PID" 2>/dev/null || true
  fi
  rm -rf "$WORK"
}
trap cleanup EXIT

echo "mosquitto: $(printf '%s\n' "$(mosquitto -h 2>&1 || true)" | sed -n '1p')"

# ---------------------------------------------------------------- syntax ---
if mosquitto --help 2>&1 | grep -q -- '--test-config'; then
  if mosquitto --test-config -c "$CONF"; then
    pass "--test-config accepted $CONF"
  else
    fail "--test-config rejected $CONF"
  fi
else
  echo "  skip --test-config (needs mosquitto >= 2.1; this build has no such flag)"
fi

# ----------------------------------------------------------------- start ---
mosquitto -c "$CONF" >"$BROKER_LOG" 2>&1 &
BROKER_PID=$!
echo "  ..   broker pid $BROKER_PID, log $BROKER_LOG"

if ! grep -q . "$BROKER_LOG" 2>/dev/null; then
  : # the log may still be empty; the probe below is the real check
fi

probe() {
  mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CAFILE" \
    -u "$NODE_USER" -P "$NODE_PASS" -t "$TOPIC_OWN" -m hello -q 1
}

ready=0
for _ in $(seq 1 40); do
  if ! kill -0 "$BROKER_PID" 2>/dev/null; then
    echo "--- broker log ---"; cat "$BROKER_LOG" >&2; fail "broker exited during startup"
  fi
  # shellcheck disable=SC2188
  if (exec 3<>"/dev/tcp/$HOST/$PORT") 2>/dev/null && probe 2>/dev/null; then
    ready=1
    break
  fi
  sleep 0.5
done
[[ "$ready" == 1 ]] || { echo "--- broker log ---"; cat "$BROKER_LOG" >&2; fail "no TLS listener on $HOST:$PORT within 20s"; }
pass "broker is listening on $HOST:$PORT over TLS"

# --------------------------------------------------------- file ownership ---
# DEPLOY.md step 5 hands /etc/mosquitto/passwd and /etc/mosquitto/acl to the
# user the broker runs as — mosquitto:mosquitto, 0600 for the password file and
# 0640 for the ACL. While those files belong to anyone else mosquitto warns on
# every start and a future version will refuse to load them, so this is the
# check that makes the deploy rule fail CI instead of scrolling past. Read-only:
# it reads the broker's own log and the files its config names.
if grep -q 'owner is not mosquitto' "$BROKER_LOG"; then
  echo "--- broker log ---"; cat "$BROKER_LOG" >&2
  fail "the broker warned that a config file is not owned by mosquitto (DEPLOY.md step 5)"
fi
pass "the broker loaded password_file and acl_file without an ownership warning"

# The broker only complains about ownership, so the modes need asserting too:
# a fixture that gets the owner right and the password file 0640 would pass the
# warning check while the runbook rule was still broken.
for want in password_file:mosquitto:mosquitto:600 acl_file:mosquitto:mosquitto:640; do
  IFS=: read -r key want_owner want_group want_mode <<<"$want"
  path="$(awk -v k="$key" '$1 == k { print $2; exit }' "$CONF")"
  [[ -n "$path" ]] || fail "$CONF names no $key"
  got="$(stat -c '%U:%G %a' "$path")"
  [[ "$got" == "$want_owner:$want_group $want_mode" ]] ||
    fail "$key $path is '$got'; DEPLOY.md step 5 requires '$want_owner:$want_group $want_mode'"
  pass "$key $path is $got"
done

# ------------------------------------------------------------- anonymous ---
if mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CAFILE" -t "$TOPIC_OWN" -m anon -q 1 2>/dev/null; then
  fail "anonymous publish was accepted (allow_anonymous must be false)"
fi
pass "anonymous publisher is refused"

if mosquitto_sub -h "$HOST" -p "$PORT" --cafile "$CAFILE" -t 'nordtronics/wildfire/+/telemetry' -C 1 -W 1 2>/dev/null; then
  fail "anonymous subscriber was accepted (allow_anonymous must be false)"
fi
pass "anonymous subscriber is refused"

# ------------------------------------------------------------ round trip ---
mosquitto_sub -h "$HOST" -p "$PORT" --cafile "$CAFILE" -u "$INGEST_USER" -P "$INGEST_PASS" \
  -t 'nordtronics/wildfire/+/telemetry' -C 1 -W 10 >"$RECEIVED" 2>/dev/null &
SUB_PID=$!
sleep 1
mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CAFILE" -u "$NODE_USER" -P "$NODE_PASS" \
  -t "$TOPIC_OWN" -m '{"pm25":12.3}' -q 1
wait "$SUB_PID" || fail "the ingest user did not receive $NODE_USER's own telemetry"
grep -q 'pm25' "$RECEIVED" || fail "payload did not survive the trip: $(cat "$RECEIVED")"
pass "ingest user received $NODE_USER's telemetry (TLS + password auth + ACL read)"

# ------------------------------------------------------------- isolation ---
# A node publishing to another node's topic must be dropped by the ACL.
mosquitto_sub -h "$HOST" -p "$PORT" --cafile "$CAFILE" -u "$INGEST_USER" -P "$INGEST_PASS" \
  -t 'nordtronics/wildfire/+/telemetry' -C 1 -W 3 >"$RECEIVED" 2>/dev/null &
SUB_PID=$!
sleep 1
mosquitto_pub -h "$HOST" -p "$PORT" --cafile "$CAFILE" -u "$NODE_USER" -P "$NODE_PASS" \
  -t "$TOPIC_OTHER" -m '{"pm25":6.6}' -q 1 2>/dev/null || true
if wait "$SUB_PID" 2>/dev/null; then
  fail "$NODE_USER was able to publish to $TOPIC_OTHER (ACL is not isolating nodes)"
fi
pass "$NODE_USER cannot publish to $OTHER_NODE's topic"

echo "mosquitto config: ALL CHECKS PASSED"
