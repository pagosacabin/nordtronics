#!/usr/bin/env bash
# mailbox-changed.sh — $0-cost wakeAgent gate + protocol/peak digest for the
# Juno <-> Hermes mailbox worker jobs (nordtronics task 0082, optimization 1).
#
# Contract (Hermes cron `script=`):
#   * stdout is injected into the agent prompt as the "Script Output" block.
#   * the LAST non-empty stdout line, when it is the JSON {"wakeAgent": false},
#     skips the agent run entirely for that tick — no LLM call, no delivery.
#
# "Mailbox unchanged" is measured against git, not file mtimes: Juno pushes new
# tasks to origin, so a local mtime stamp would never see new work arrive.
# Unchanged means ALL of:
#   * the repo exists and `git fetch origin main` succeeds
#   * local `main` == `origin/main`
#   * HEAD is on `main` (a tick that died mid-branch must be recovered)
#   * no tracked modifications inside mailbox/ (rule 17 debris blocks the pull)
#   * no unpushed local commits on main
#
# Any error (missing repo, failed fetch, stray branch) FAILS OPEN — the agent
# wakes — so a broken gate can never silently stall the queue.
#
# This script also runs the protocol-version checker, because a cron job carries
# exactly ONE `script` slot: the PROTOCOL:/PEAK: digest the worker prompt depends
# on (Step 0 / Step 0b) is emitted here alongside the gate verdict.
#
# Env overrides: MAILBOX_REPO, MAILBOX_PROTOCOL_CHECKER, MAILBOX_GATE_LOG.
set -uo pipefail

REPO="${MAILBOX_REPO:-/home/astroboy/nordtronics}"
CHECKER="${MAILBOX_PROTOCOL_CHECKER:-$HOME/.hermes/profiles/cronrunner/scripts/mailbox-protocol-check.py}"
GATE_LOG="${MAILBOX_GATE_LOG:-$HOME/.hermes/profiles/cronrunner/mailbox-gate.log}"

# ---- protocol + peak digest (subsumes the former pre-run script) ------------
if [ -f "$CHECKER" ]; then
  PY=""
  for cand in "$HOME/.hermes/hermes-agent/venv/bin/python" "$(command -v python3)" /usr/bin/python3; do
    if [ -n "$cand" ] && command -v "$cand" >/dev/null 2>&1; then PY="$cand"; break; fi
  done
  if [ -n "$PY" ]; then
    # The checker always exits 0 (non-strict) and prints PEAK:/PROTOCOL: lines.
    "$PY" "$CHECKER" 2>/dev/null || echo "PROTOCOL: ERROR checker exited non-zero"
  else
    echo "PROTOCOL: ERROR no python interpreter found for $CHECKER"
  fi
else
  echo "PROTOCOL: ERROR checker missing at $CHECKER"
fi

# ---- git-state gate ---------------------------------------------------------
reasons=""

if ! git -C "$REPO" rev-parse --git-dir >/dev/null 2>&1; then
  reasons="$reasons repo-unavailable"
else
  if ! git -C "$REPO" fetch --quiet origin main 2>/dev/null; then
    reasons="$reasons fetch-failed"
  fi
  local_main=$(git -C "$REPO" rev-parse main 2>/dev/null || echo "")
  remote_main=$(git -C "$REPO" rev-parse origin/main 2>/dev/null || echo "")
  if [ -z "$local_main" ] || [ -z "$remote_main" ]; then
    reasons="$reasons ref-unavailable"
  elif [ "$local_main" != "$remote_main" ]; then
    reasons="$reasons main-not-at-origin/main"
  fi
  head_ref=$(git -C "$REPO" rev-parse --abbrev-ref HEAD 2>/dev/null || echo "?")
  if [ "$head_ref" != "main" ]; then
    reasons="$reasons head-on-$head_ref"
  fi
  if [ -n "$(git -C "$REPO" status --porcelain --untracked-files=no -- mailbox 2>/dev/null)" ]; then
    reasons="$reasons dirty-mailbox"
  fi
  ahead=$(git -C "$REPO" rev-list --count origin/main..main 2>/dev/null || echo 0)
  if [ "${ahead:-0}" != "0" ]; then
    reasons="$reasons $ahead-unpushed-commit(s)"
  fi
fi

reasons="${reasons# }"
stamp=$(date -u +%Y-%m-%dT%H:%M:%SZ)

if [ -z "$reasons" ]; then
  echo "GATE: unchanged (main == origin/main, HEAD on main, mailbox clean) [$stamp]"
  echo "{\"wakeAgent\": false}"
  printf '%s GATE wake=false unchanged\n' "$stamp" >>"$GATE_LOG" 2>/dev/null
else
  echo "GATE: mailbox changed -> $reasons [$stamp]"
  echo "{\"wakeAgent\": true, \"context\": {\"mailbox_changed\": true, \"reasons\": \"$reasons\"}}"
  printf '%s GATE wake=true %s\n' "$stamp" "$reasons" >>"$GATE_LOG" 2>/dev/null
fi

# Keep the gate log bounded (~1 line per tick).
if [ -f "$GATE_LOG" ] && [ "$(wc -l <"$GATE_LOG" 2>/dev/null || echo 0)" -gt 1000 ]; then
  tail -n 500 "$GATE_LOG" >"$GATE_LOG.tmp" 2>/dev/null && mv "$GATE_LOG.tmp" "$GATE_LOG" 2>/dev/null
fi

exit 0
