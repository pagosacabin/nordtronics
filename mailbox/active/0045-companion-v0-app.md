---
task_id: "0045"
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0045 — Companion v0: realistic test app + mock backend

## Context

The Android toolchain lesson is done: a CI-built debug APK installs, opens,
stays running, and shows the Nordtronics icon (verified commit
`2a92766af59084a4d920d8e9fca69795736db538`, green run
`https://github.com/pagosacabin/nordtronics/actions/runs/35754454447`; 0037
manifest fix verified and archived).

The real product will be a wildfire companion app: node status (PM2.5,
temperature, humidity, battery), alert history, and push alerts served from
the VPS API. The VPS is not provisioned yet, so this task builds the app
against a mock backend with the exact API shapes the real backend will
implement later.

## Task

Build "Companion v0": an Android app with three screens, backed by a tiny
mock HTTP server running on your laptop.

Screens:

1. **Nodes** — list of sensor nodes, each row showing PM2.5, temperature,
   humidity, battery voltage, and last-seen time.
2. **Alerts** — alert history list (type, node, message, timestamp).
3. **Node detail** — full readings for one node plus a "Send test ping"
   button. Tapping it POSTs to the mock server; the app displays the
   server's confirmation response on screen.

Mock backend (runs on your laptop, localhost only):

- `GET /api/nodes` returns a JSON array of node objects:
  `{"id":"node-01","pm25":12.4,"temp_c":21.3,"humidity":38,"battery_v":4.05,"last_seen":"2026-09-23T14:00:00Z"}`
- `GET /api/alerts` returns a JSON array of alert objects:
  `{"id":"a1","node_id":"node-01","type":"smoke","message":"PM2.5 rising fast","at":"2026-09-23T13:58:00Z"}`
- `POST /api/nodes/{id}/ping` returns `{"ok":true,"node_id":"node-01"}`.

The app's API base URL lives in exactly one place (a single config constant
or BuildConfig field) so pointing it at the real VPS later is a one-line
change. Serve at least two nodes and three alerts from the mock.

## Success criteria

- Branch `hermes/0045-companion-v0` exists on origin; all work is on it.
- A green CI run builds a debug APK; the APK is downloadable as a run artifact.
- Screenshots of all three screens are committed in the branch under
  `android/companion-v0/screenshots/` (real screenshots, not mockups).
- The mock server's request log from a real ping round-trip is committed in
  the branch at `android/companion-v0/mock-server/ping-test.log`
  (timestamped lines showing the POST arriving and the 200 response).
- The app screenshot of the node-detail screen shows the ping confirmation.

## Constraints

- Push only to branch `hermes/0045-companion-v0`. Never push to main.
- The mock server binds to localhost only. No exposure to the LAN or internet.
- No real credentials, tokens, or production URLs anywhere in the branch.
- No push notifications, no Play Store, no release signing — those come later
  with the VPS. This task is the UI + HTTP round-trip only.
- Keep the JSON shapes above exactly; the real backend will implement them.

## Proof

Stage with `proof` pointers in this form:

```yaml
proof:
  - branch: hermes/0045-companion-v0
    sha: <full tip SHA on origin>
  - run: <Actions run URL, green, head SHA == branch tip>
  - files:
      - android/companion-v0/screenshots/nodes.png
      - android/companion-v0/screenshots/alerts.png
      - android/companion-v0/screenshots/node-detail.png
      - android/companion-v0/mock-server/ping-test.log
```

Pasted terminal output is a claim, not proof. The screenshots and the
ping-test log must be committed files Juno can fetch from origin.

## Reply format

When staging, fill in:

```yaml
ui_verification_method: <how you captured the screenshots: emulator? physical device? which?>
base_url_location: <file and line of the single API base-URL constant>
mock_server_command: <exact command you ran to start the mock>
notes: <anything that deviated from the spec>
```
