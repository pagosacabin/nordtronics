---
task_id: "0096"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
---

# 0096 — App: wire the live API and cut an installable build

# Context

0090 shipped the v2 UI with `MockApi` active and `HttpApiClient` wired but
dormant. Stephen is running that build and seeing mock data. The live backend
is up and proven (2026-10-02): `https://api.nordtronics.io/v1/nodes` and
`/v1/nodes/<id>/readings` serve real stored readings (e.g. node `bench-01`
today). The app API contract is `docs/wildfire/app-api-contract-v1-2026-10-02.md`.
The Alerts screen stays gated until 0095 lands `/v1/alerts` — do not un-gate it
here; keep it exactly as 0090 left it.

# Task

1. Flip the app from `MockApi` to `HttpApiClient` pointed at
   `https://api.nordtronics.io`, as a BuildConfig field (not a hardcoded
   string), with the mock still available as the offline fallback.
2. Verify against the LIVE backend: the Property screen lists real nodes and
   the Node detail screen shows the latest real reading (bench-01 exists today;
   if the backend is unreachable from the emulator, say so and prove against
   a locally served copy of a real `/v1/nodes/bench-01/readings` response).
3. Cut the build: CI `android-build.yml`, debug APK artifact (installable —
   the 0090 lesson stands: GitHub artifact downloads 404 without a login, so
   the proof must include the artifact; Juno will re-host the APK for
   Stephen's phone).

# Success criteria

- CI green at the branch tip; debug APK artifact present and installable.
- Emulator screenshot showing a REAL backend reading (not mock data — the
  mock banner/placeholder must be gone when online).
- No changes to the Alerts screen gating, no backend changes, no keystore or
  signing changes.

# Constraints

- Read `docs/wildfire/app-ui-spec-v2-2026-10-02.md` and
  `docs/wildfire/app-api-contract-v1-2026-10-02.md` before touching code.
- Worker cost: standard tier, off-peak preferred. State the tier used.

# Proof

- Branch `hermes/0096-app-live-api`, pushed; SHA on origin.
- Actions run URL, green at the tip SHA; artifact name + size.
- Screenshot(s) showing live data, plus the exact API response the screen
  rendered (quoted verbatim).
- `reply_format`: staged file per the mailbox protocol with proof pointers,
  scope extensions, self-caught defects, and anything left undone.

# Reply format

Stage `mailbox/staged/0096-app-live-api.md` per `mailbox/README.md`.
