---
task_id: "0132"
protocol_version: 1.0.0
status: filed
expect-reply-within: 6h
---

# 0132 — Place API key in wildfire-api service environment on VPS

## Context

- The backend now gates every /v1/* route on the X-API-Key header (commit 9dfc091, on main). The API reads the server key from env var WILDFIRE_API_KEY via `EnvironmentFile=-/etc/nordtronics/api.env` in the wildfire-api.service unit.
- Stephen placed the shared API key at /home/astroboy/.hermes/api.key on your machine. The companion-app release APK (0130) already sends it.
- Current VPS state (checked 2026-10-08 by Juno): /etc/nordtronics/api.env does NOT exist; wildfire-api is active on the old (pre-auth) code; /opt/nordtronics/backend is an rsync'd tree, not a git checkout.

## Task

1. Read the key from /home/astroboy/.hermes/api.key.
2. As deploy on the VPS (89.117.21.105, passwordless sudo), create /etc/nordtronics/api.env containing exactly one line:
   WILDFIRE_API_KEY=<the key>
   Owner root:root, mode 0600 (matches /etc/nordtronics/ingest.env).
3. Restart the API: sudo systemctl restart wildfire-api.
4. Verify the service is active and /healthz returns 200 with NO key:
   curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/healthz
   NOTE: the pre-auth code is still deployed, so /v1/nodes will still return 200 without a key right now — that is expected. Juno deploys the auth code next and re-verifies the 401/200 gate herself. Do not "fix" this.

## Success criteria

- /etc/nordtronics/api.env exists, root:root, mode 0600, single WILDFIRE_API_KEY= line.
- wildfire-api active after restart; /healthz returns 200 with no key.

## Constraints

- The key NEVER goes in a git repo, a staged reply, a log, or chat. It travels only: your laptop file -> SSH -> /etc/nordtronics/api.env.
- Do not deploy backend code or change anything else on the VPS. Key placement + restart + verify only.
- Do not touch mosquitto config, ACLs, the database, or any other service.

## Proof

- SSH transcript showing: the api.env `ls -la` line (mode and owner only, NOT its contents), `systemctl is-active wildfire-api` output, and the /healthz HTTP code.
- State explicitly that the key value was not printed, logged, or committed anywhere.

## Reply format

Staged reply per mailbox/README.md: status, the proof pointers above, notes.
