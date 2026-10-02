---
task_id: "0096"
protocol_version: 1.0.0
status: verified
iteration: 1
proof:
  - branch: hermes/0096-app-live-api
    sha: cf81e6fe2aa8c54a2d72131b8d3eddf35f10bc3e
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37073926770
    job: "build - 'Assemble debug APK' + 'Assemble release APK' green; push event, headSha = the branch tip (the branch was added to the workflow's trigger list in the same commit, see scope extension 1)"
  - artifact: https://github.com/pagosacabin/nordtronics/actions/runs/37073926770/artifacts
    artifacts: companion-v0-debug-apk (3579800 B) - the installable APK Juno re-hosts; companion-v0-release-apk (2487266 B); companion-v0-screenshots (3527723 B)
  - ntfy:
      topic: nordtronics-build-ed05a663
      id: qWddMZmQUo2P
      time: "1790981146 (2026-10-02T22:45:46Z), HTTP 200"
  - files:
      - .github/workflows/android-companion-v0.yml
      - android/companion-v0/app/build.gradle
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/ApiClient.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/ApiSource.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/FallbackApi.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/HttpApiClient.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/Screens.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/SystemActivity.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/WildfireApi.java
      - android/companion-v0/app/src/main/res/layout/activity_system.xml
      - android/companion-v0/app/src/main/res/values/strings.xml
      - android/companion-v0/screenshots/v2-live-alerts.png
      - android/companion-v0/screenshots/v2-live-node-detail.png
      - android/companion-v0/screenshots/v2-live-property.png
      - android/companion-v0/screenshots/v2-live-system.png
  - screenshots_captured_from:
      run: https://github.com/pagosacabin/nordtronics/actions/runs/37072145022
      sha: 262ded687f298de61f23d706a593ba7528f097ad
      note: "artifact companion-v0-debug-apk (3579797 B); the tip cf81e6f differs from it by 'git diff --stat 262ded6 cf81e6f' = 4 files changed, 0 insertions, 0 deletions (the four PNGs), so the APK code is identical"
notes: >
  The app now talks to the live backend. The debug build type's dev override (the 10.0.2.2 mock
  dialect) is removed so every build type uses the one BuildConfig block pointed at
  https://api.nordtronics.io; ApiSource returns a new FallbackApi (HttpApiClient first, the bundled
  MockApi only when the backend is unreachable), and the PROTOTYPE - MOCK DATA banner plus the mock
  footer wording are shown only while the mock answers. Verified on the API-35 emulator with the
  CI-built debug APK: the Property screen lists the real bench-01 and node-01 from GET /v1/nodes,
  the Bench 01 card (tapped, activity switch proven) shows the real latest reading
  9.2 ug/m3 / 73 F (23.0 C) / 45 % RH / 4.05 V, and the banner is gone from the raster (measured:
  the mock run draws a content band at rows 156-172 of the 1080x2400 frame, the live run draws
  nothing in rows 100-189 and its content starts at row 190).

  The deployed backend is a SUBSET of contract v1 and the app had to degrade for it, all declared
  below: GET /v1/network/status and GET /v1/nodes/{id} answer 404 today, so the network summary is
  null (no watch banner - the app never computes consensus) and the node detail is served from the
  node-list entry; the readings payload is a "readings" array with every metric per row and newest
  first, not contract v1's points:[{t,v}], so readings() accepts either shape.
  POST /v1/alerts/{id}/acknowledge does not exist on the deployed backend either.

  Scope extensions: the CI workflow's trigger list (a push to a fresh branch starts no run
  otherwise), the System screen's prototype panel and badge, the footer wording, the
  ApiClient / HttpApiClient / FallbackApi degradations themselves, and Screens.ageOf (with no
  summary the header read 'Last packet received unknown' beside cards saying 'Updated 2 h ago').

  Self-caught defect: that 'unknown' header, fixed in the second commit. The Alerts screen was left
  exactly as 0090 left it, per the task, including its 0090 empty-state copy that still references
  the mock's Node 02 scenario - flagged for a follow-up rather than changed here.

  WORKER TIER: DeepSeek standard (deepseek-flash), off-peak (the pre-run script injected
  'PEAK: OFF-PEAK 22:15 UTC').
---

# 0096 — Reply: the app runs against the live API

## Pointers

| | |
|---|---|
| Task | 0096 |
| Branch | `hermes/0096-app-live-api` |
| SHA (branch tip) | `cf81e6fe2aa8c54a2d72131b8d3eddf35f10bc3e` |
| CI | [run 37073926770](https://github.com/pagosacabin/nordtronics/actions/runs/37073926770) — success, `headSha` = the tip above |
| Artifact | `companion-v0-debug-apk` **3,579,800 B** (installable; Juno re-hosts it), `companion-v0-release-apk` 2,487,266 B, `companion-v0-screenshots` 3,527,723 B — [artifacts](https://github.com/pagosacabin/nordtronics/actions/runs/37073926770/artifacts) |
| ntfy | topic `nordtronics-build-ed05a663`, id `qWddMZmQUo2P`, 2026-10-02T22:45:46Z, HTTP 200 |
| Worker tier | DeepSeek **standard** (`deepseek-flash`), off-peak — the pre-run script injected `PEAK: OFF-PEAK 22:15 UTC` |

## What changed

1. **`app/build.gradle` — the flip.** The debug build type's dev override (the literal
   `http://10.0.2.2:8000` and the mock's `/api/...` dialect) is gone, so **every** build type now
   reads the single `defaultConfig` block: `API_BASE_URL = "https://api.nordtronics.io"`,
   `PATH_NODES = /v1/nodes`, `PATH_HEALTH = /healthz`, `PATH_ALERTS = /v1/alerts`. Proven in the
   generated sources rather than the Gradle text: `app/build/generated/source/buildConfig/debug/
   com/nordtronics/companion/BuildConfig.java` line 13 is
   `public static final String API_BASE_URL = "https://api.nordtronics.io";`, with the three `/v1`
   paths beside it. Still a `BuildConfig` field — no host or path literal exists anywhere in the
   app tree now (the debug network-security exception for `10.0.2.2` stays, so a developer can still
   run `mock-server/server.py` by hand; the app itself no longer names that address).
2. **`ApiSource` + new `FallbackApi` — the mock is the offline fallback.** `ApiSource.get()` used
   to return `new MockApi()`; it now returns `new FallbackApi()`, which holds an `HttpApiClient` and
   a bundled `MockApi` and tries them **per call**: live first, mock only when the live call throws
   (no network, DNS, TLS, a non-2xx the client does not degrade for). `ApiSource.usingMock()`
   reports which one answered. Screens still see only `WildfireApi`.
3. **`Screens` — the mock banner is conditional.** The `PROTOTYPE – MOCK DATA` banner and the
   footer's "Readings from local mock backend" wording are shown **only while the mock is
   answering**; a live read hides the banner and the footer names `https://api.nordtronics.io`.
   The banner is refreshed after every read, so it follows a mid-session fallback.
4. **`HttpApiClient` — degrades for routes the deployed backend does not serve** (details below).
5. **`SystemActivity` — same rule on the System screen** (prototype panel hidden, badge "Live data"
   when the probe succeeds).
6. **`.github/workflows/android-companion-v0.yml`** — the branch added to the trigger list
   (declared as a scope extension: without it a push to a fresh branch starts **no** run).

## Live verification (emulator against the deployed API)

Emulator API-35 `pixel_7` (`insets35`, 1080×2400); APK = the **CI-built** `companion-v0-debug-apk`
(see "Which APK", below).

Verbatim responses the screens rendered, fetched 2026-10-02T22:45:52Z / 22:45:53Z:

```json
{"nodes":[{"node_id":"bench-01","first_seen_utc":"2026-10-02T20:37:37Z","last_seen_utc":"2026-10-02T20:37:37Z","reading_count":1,"age_seconds":7695,"status":"stale","latest":{"pm25":9.2,"temperature_c":23.0,"humidity_pct":45.1,"battery_v":4.05}},{"node_id":"node-01","first_seen_utc":"2026-09-25T19:18:44Z","last_seen_utc":"2026-09-25T19:20:01Z","reading_count":2,"age_seconds":617151,"status":"stale","latest":{"pm25":7.5,"temperature_c":21.0,"humidity_pct":41.0,"battery_v":3.8}}],"count":2,"stale_after_seconds":900,"generated_utc":"2026-10-02T22:45:52Z"}
```

```json
{"node_id":"bench-01","count":1,"limit":100,"since":null,"readings":[{"recorded_utc":"2026-10-02T20:37:37Z","observed_utc":"2026-10-02T20:45:00Z","pm25":9.2,"temperature_c":23.0,"humidity_pct":45.1,"battery_v":4.05}],"generated_utc":"2026-10-02T22:45:53Z"}
```

and, for the two contract routes the backend has not implemented:

```
GET /v1/network/status  -> HTTP 404
GET /v1/nodes/bench-01  -> HTTP 404
```

**Property screen** (`v2-live-property.png`, `uiautomator` dump read back): "Property line / Last
packet received 2 h ago", then Field nodes **"Bench 01 — Stale · Updated 2 h ago · 9.2 µg/m³
PM2.5 · 73 °F Temperature · 45 % RH Humidity · 4.05 V Battery"** and **"Node 01 — Stale · Updated
7 d ago · 7.5 µg/m³ · 70 °F · 41 % RH · 3.80 V"**, footer "Readings from
`https://api.nordtronics.io` · Privacy-first · No cameras · Locations stay on your network". Every
figure maps 1:1 onto the JSON above (23.0 °C → 73 °F; 45.1 → "45" at the display's whole-number
humidity). The string `PROTOTYPE`/`MOCK` does not appear anywhere in the dump.

**Node detail** (`v2-live-node-detail.png`), reached by **tapping** the Bench 01 card — the activity
switch is the proof, not a clickable attribute: `topResumedActivity=…/.NodeDetailActivity`. It reads
"Bench 01 — Outdoor Air & Environmental Monitor", "Updated 2 h ago", chip "Stale", PM2.5 **9.2**
µg/m³, Temperature **73 °F (23.0 °C)**, Humidity **45 % RH**, Battery "4.05 V in Link & hardware",
and Trends "PM2.5 · Now: 9.2 µg/m³".

**The banner is gone — measured on the raster, not by eye.** Comparing the 1080×2400 frames:
with the mock answering there is a content band at **rows 156–172** (the banner strip, drawn under
the status bar) and page content starts at row 245; with the live backend **nothing is drawn in
rows 100–189** and content starts at **row 190** — a 55 px shift up where the banner used to be.

## The deployed backend is a subset of contract v1 — what the app degrades for

`GET /v1/network/status` and `GET /v1/nodes/{id}` are 404 today, and the readings payload does not
match the contract's `points:[{t,v}]`. Three deliberate degradations, none of them inventing data:

1. `networkStatus()` returns **null** for a 404 (`ApiClient.getIfPresent` treats 404 as an answer).
   The property screen already guarded a null status and simply draws **no watch banner** — the app
   does **not** compute consensus itself, per the contract's "never the app" rule.
2. `node(id)` falls back to that node's entry in `GET /v1/nodes`, which is the same node object with
   `latest`; only an id the backend has never seen throws.
3. `readings()` accepts **either** payload — contract v1's `points`, or the deployed
   `readings:[{recorded_utc, observed_utc, pm25, temperature_c, …}]` rows (every metric per row,
   newest first). It selects the requested metric off each row and reverses into the contract's
   oldest→newest order.

## Which APK produced the screenshots

From `companion-v0-debug-apk` of run **37072145022** (sha `262ded6`), downloaded through the
artifacts API. The tip is `cf81e6f`, and `git diff --stat 262ded6 cf81e6f` is **4 files changed,
0 insertions, 0 deletions** — the four PNGs themselves. So the APK code is identical to the tip,
and I also confirm the *local* build of the same commit renders the same dump text, so the screenshots
are not an artifact of one build's dex ordering.

## Scope extensions (declared, with the criterion as the reason)

1. **CI trigger list** — a one-line addition of `hermes/0096-app-live-api`; without it the criterion
   "CI green at the branch tip" has no attainable proof (a fresh branch starts no run).
2. **`SystemActivity`, `activity_system.xml`, `strings.xml` (`demo_badge_live`)** — the System
   screen asserted "Every reading and alert on these screens is demonstration data served by the
   local mock backend" and "PROTOTYPE — MOCK BACKEND". It now shows the prototype panel only when
   the probe fails and reads "Live data" when it succeeds. Same criterion as the banner (no mock
   claim on screen when online), a different screen.
3. **Footer wording** (`FOOTER_LIVE`) — the UI spec's footer names the local mock backend; when the
   API answers it names the API instead.
4. **`ApiClient` / `HttpApiClient` / `FallbackApi`** — these *are* the "wire the live API" work: the
   live backend cannot drive the screens without the 404 and payload-shape handling above.
5. **`Screens.ageOf`** — with no `/v1/network/status` the header read "Last packet received
   **unknown**" next to cards saying "Updated 2 h ago"; it now uses the freshest node's
   `age_seconds` when the summary carries no `last_packet_utc`.

## Self-caught defects

- The "Last packet received unknown" header above — written in the first commit, caught by reading
  the emulator dump against the spec, fixed in the second (`262ded6`).
- An intermediate capture showed mock data **with the banner**: the app had been launched before the
  emulator's cellular default network finished validating (`dumpsys connectivity` showed no
  validated network at that moment). Not a code defect — the fallback did exactly its job — but that
  frame is discarded; the deliverable was re-captured after `ping 8.8.8.8` succeeded on the device.

## Left undone, declared rather than hidden

- **Alerts screen untouched, per the task.** Its gating is exactly as it arrived (`PATH_ALERTS`
  non-null in both build types — that is 0095's change, not mine), and the live feed is empty today
  (`{"alerts":[],"count":0,…}`), so it renders the "No active alerts" empty state
  (`v2-live-alerts.png`). Its empty-state body copy — "…Node 02 is elevated on its own — shown as
  Watch until a second node confirms." — is 0090's own wording and still describes the mock's
  scenario; I did not change it because the task says keep the screen exactly as 0090 left it.
  **Flag for a follow-up task.**
- **No live alert was exercised.** `POST /v1/alerts/{id}/acknowledge` does not exist on the deployed
  backend (only `GET /v1/alerts` and `GET /v1/nodes/{id}/alerts`), so Acknowledge would 404 against
  live. The offline mock path still runs the state machine.
- **Visual gaps that are the backend's, not the app's**, and deliberately left as dashes rather than
  filled with invented values: node detail shows "Battery — %" and "On battery · +— % today", and
  "RSSI — dBm / Link margin — dB / Gateway hops —", because the node objects carry no `battery_pct`,
  `charging`, `rssi_dbm`, `link_margin_db` or `gateway_hops` yet (contract v1's optional fields);
  the 12 h sparklines are empty because `trends_12h` needs `GET /v1/nodes/{id}`, which is 404.
- **Status vocabulary mismatch, recorded not hidden:** the deployed backend says `ok|stale|unknown`
  where the contract says `live|stale`. The app's status word only special-cases `stale`, so a fresh
  node reads "Healthy" — fine — and "Reporting 0/2" is the truth today (both nodes are older than
  the 900 s threshold).
- **No release/keystore/signing change** (still unsigned `app-release-unsigned.apk`) and **no backend
  change**, per the constraints.

## Files changed (branch vs main — 15 files; main carries the project, 71 files under `app/src`)

```
.github/workflows/android-companion-v0.yml        |   2 +-
android/companion-v0/app/build.gradle             |  51 ++++-----
.../companion/ApiClient.java                      |  62 +++++++---
.../companion/ApiSource.java                      |  36 +++++--
.../companion/FallbackApi.java                    | 116 ++++++++++++++++++
.../companion/HttpApiClient.java                  |  84 +++++++++++---
.../companion/Screens.java                        |  77 +++++++++++---
.../companion/SystemActivity.java                 |  21 +++-
.../companion/WildfireApi.java                    |  20 ++--
.../res/layout/activity_system.xml                |   7 +-
.../res/values/strings.xml                        |   1 +
.../screenshots/v2-live-alerts.png                | Bin 0 -> 110732 bytes
.../screenshots/v2-live-node-detail.png           | Bin 0 -> 152331 bytes
.../screenshots/v2-live-property.png              | Bin 0 -> 177671 bytes
.../screenshots/v2-live-system.png                | Bin 0 -> 199658 bytes
15 files changed, 388 insertions(+), 89 deletions(-)
```

---


## Original spec as received

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
