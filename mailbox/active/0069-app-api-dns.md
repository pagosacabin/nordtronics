---
task_id: "0069"
status: in_progress
iteration: 1
---

# 0069 — Android app: point API base URL at api.nordtronics.io

## Context

The companion app source is `android/companion-v0/`, newest on branch
`hermes/0049-companion-v01-ui` (main only holds the `android-hermes-test/`
skeleton — cut this task's branch from the 0049 tip, not from main).
The app centralizes its backend address in one place, which is good:
`android/companion-v0/app/build.gradle` line 26 sets
`buildConfigField "String", "API_BASE_URL", "\"http://10.0.2.2:8000\""`
and `ApiClient.java` line 20 does `BASE_URL = BuildConfig.API_BASE_URL`.
`10.0.2.2:8000` is the emulator's alias for the dev mock server — it must not
ship in a release build.

The production backend is live and verified: `https://api.nordtronics.io`
serves `/v1/nodes` (HTTP 200) and `/healthz` (HTTP 200). Note the path
mismatch: the app appends `/api/nodes`, `/api/alerts`,
`/api/nodes/{id}/ping` to BASE_URL, but the live API contract is `/v1/...`.

## Task

One deliverable: make the app's release build talk to the production API over
`nordtronics.io` DNS, with no hardcoded IPs, while keeping the emulator/mock
workflow intact for local development.

## Success criteria

- No hardcoded IPv4/IPv6 address anywhere under `android/companion-v0/`,
  except the documented dev override below.
- Release build's effective base URL is `https://api.nordtronics.io` and its
  request paths match the live API contract (probe `GET /v1/nodes` and
  `/healthz` yourself to confirm the exact paths before wiring them).
- Debug build keeps working against the local mock: `http://10.0.2.2:8000`
  remains available via a debug-only buildConfigField or equivalent override
  (document which one in the reply).
- HTTPS to the production host works from the app (check the network security
  config — the current http dev value implies cleartext may have been allowed).
- CI builds the APK green on the branch.

## Constraints

- Branch: `hermes/0069-app-api-dns`, cut from `origin/hermes/0049-companion-v01-ui`
  tip. Do NOT merge to main in this task (main's android tree is a separate
  skeleton; merging is a later decision).
- Out of scope: ESP32 firmware MQTT IPs (192.168.1.x in `firmware/` — LAN
  devices, not the app) and the mock server's own 127.0.0.1 loopback defaults.
  Mention any you find in the notes, but don't change them.
- Run this task on the deepseek-flash worker tier. State the model used in your
  reply.
- One deliverable per task: DNS cutover only. No UI changes, no auth/push
  wiring.

## Proof

- `proof` front-matter: branch `hermes/0069-app-api-dns` + tip SHA, and the
  green Actions run URL whose head SHA matches that tip.
- I will verify the pointers and grep the branch for hardcoded IPs myself.

## Reply format

Stage the file with `status: staged` and fill in:

```yaml
proof:
  - branch: hermes/0069-app-api-dns
    sha: <40-char tip sha>
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/<id>
```

Plus in the body or notes:
- Base branch used and why (confirm 0049 tip was still the newest app source).
- Files changed and what changed in each (include the live-API paths you
  probed and wired).
- How the debug/emulator override is selected.
- `grep` evidence showing no hardcoded IPs remain in `android/companion-v0/`
  outside the debug override.
- Model/tier used.
