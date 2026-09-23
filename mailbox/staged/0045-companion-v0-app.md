---
task_id: "0045"
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0045-companion-v0
    sha: 9937a1654566cbae5727e1d35a857c81052970f1
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35899478176
  - files:
      - android/companion-v0/screenshots/nodes.png
      - android/companion-v0/screenshots/alerts.png
      - android/companion-v0/screenshots/node-detail.png
      - android/companion-v0/mock-server/ping-test.log
notes: >
  Built and verified. Two environment quirks and two build fixes are recorded in
  the Reply section below; none of them change the specified JSON shapes or the
  three screens. The cited run is green at the cited SHA, with the debug APK and
  the committed screenshots attached as run artifacts.
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

---

## Reply

```yaml
ui_verification_method: >
  Android Emulator 37.1.11.0 (build 15917651), headless, on the Fedora 43 host.
  AVD "companion33" created with avdmanager from
  system-images;android-33;default;x86_64 (pixel_5 device profile). Debug APK
  installed with `adb install -r`, screens driven with `adb shell input tap`,
  screenshots captured with `adb exec-out screencap -p` (1080x2340 PNGs).
  Every screen was confirmed against the live view tree (`uiautomator dump`)
  at capture time — e.g. the node-detail capture was taken while
  id/ping_result read 'Ping confirmed by server: {"ok": true, "node_id":
  "node-01"}'. These are real pixels from the running app, not mockups.

base_url_location: >
  android/companion-v0/app/build.gradle:26 — the BuildConfig field
  `buildConfigField "String", "API_BASE_URL", "\"http://10.0.2.2:8000\""`.
  ApiClient.java:20 (`public static final String BASE_URL =
  BuildConfig.API_BASE_URL;`) is the single reader. Nothing under
  app/src/main hardcodes a host (the only other URLs there are the XML
  `xmlns:android` namespace declarations). Repointing at the VPS is one line.

mock_server_command: >
  python3 android/companion-v0/mock-server/server.py --port 8000 >
  android/companion-v0/mock-server/ping-test.log 2>&1
  (run from the repository root with the worktree on this branch)

notes: >
  Deliverable matches the spec. Four things worth knowing:

  (1) Emulator rendering. Emulator 37.1.11 segfaults in its packaged
  SwiftShader GLES renderer under `-gpu swiftshader_indirect` / `-gpu off`
  while booting, on this host (SIGSEGV in qemu-system-x86_64-headless, adb
  never leaves `offline`). `-gpu host` boots normally in ~30s (AMD Renoir,
  Mesa 25.2.7). This is an upstream emulator bug, not an app defect.

  (2) Reaching the mock from the guest. With the emulator's default
  virtio-wifi as the default network, 10.0.2.2 routes out the virtual AP and
  never reaches the host loopback — the first ping attempt failed with
  "Failed to connect to /10.0.2.2:8000". `adb shell svc wifi disable` makes
  the NAT interface the default route and the round-trip then succeeds. The
  base URL stays the standard emulator alias; the mock stays bound to
  127.0.0.1, so the loopback-only constraint is intact.

  (3) Build fix carried in the branch. appcompat 1.7.0 pulls
  kotlin-stdlib-jdk8 1.6.21 alongside kotlin-stdlib 1.8.22 and the dex merge
  fails with duplicate kotlin.streams.jdk8.StreamsKt classes. Pinned
  `implementation platform("org.jetbrains.kotlin:kotlin-bom:1.8.22")` in
  app/build.gradle.

  (4) Workflow fix. The first CI run on this branch (35899135887) was red:
  set up Android SDK failed because android-actions/setup-android@v3 has no
  api-level / build-tools-version inputs (those belong to the emulator
  runner). It now passes `packages: 'platforms;android-35
  build-tools;35.0.0 platform-tools'`. The cited run 35899478176 is the
  green run at the cited tip.

  The committed ping-test.log is one clean session: server start, the app's
  GET /api/nodes and GET /api/alerts, then
  `<- POST /api/nodes/node-01/ping ... body={}` followed by
  `-> 200 /api/nodes/node-01/ping (34 bytes) body={"ok": true, "node_id":
  "node-01"}`.
```
