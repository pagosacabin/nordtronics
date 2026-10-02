---
task_id: "0090"
protocol_version: 1.0.0
status: verified
iteration: 2
expect-reply-within: 24h
proof:
  - branch: hermes/0090-app-ui-v2
    sha: e8bbc1526aecf000e75f1350642d38a5608ac1a8
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37021388166
    result: success
    head_sha: e8bbc1526aecf000e75f1350642d38a5608ac1a8
  - artifacts: https://github.com/pagosacabin/nordtronics/actions/runs/37021388166/artifacts
    names: [companion-v0-debug-apk, companion-v0-release-apk, companion-v0-screenshots]
  - ntfy: https://ntfy.sh/nordtronics-build-ed05a663
    id: BxY0j9Ki7ezr
    published_utc: "2026-10-02T14:42:33Z"
  - files:
      - .github/workflows/android-companion-v0.yml
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/WildfireApi.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/MockApi.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/HttpApiClient.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/ApiSource.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/NodeInfo.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/AlertItem.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/NetworkStatus.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/TrendPoint.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/Screens.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/SparklineView.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/TrendChartView.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/PropertySchematicView.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/NodesActivity.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/NodeDetailActivity.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/AlertsActivity.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/SystemActivity.java
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/Ui.java
      - android/companion-v0/app/src/main/res/layout/activity_nodes.xml
      - android/companion-v0/app/src/main/res/layout/activity_node_detail.xml
      - android/companion-v0/app/src/main/res/layout/activity_alerts.xml
      - android/companion-v0/app/src/main/res/layout/bottom_nav.xml
      - android/companion-v0/app/src/main/res/drawable/bg_badge.xml
      - android/companion-v0/app/src/main/res/values/colors.xml
      - android/companion-v0/app/src/main/res/values/strings.xml
      - android/companion-v0/screenshots/v2-property.png
      - android/companion-v0/screenshots/v2-property-nodes.png
      - android/companion-v0/screenshots/v2-node-detail.png
      - android/companion-v0/screenshots/v2-node-detail-lower.png
      - android/companion-v0/screenshots/v2-alerts.png
      - android/companion-v0/screenshots/v2-alerts-acknowledged.png
notes: |
  DONE — all three v2 screens built against a mock WildfireApi, verified on an
  API 35 emulator by reading the rendered UI hierarchy, CI green at the branch
  tip. Deviations are listed in the reply under "Deviations (declared)".
---

# 0090 — Wildfire companion app: build the three v2 screens against a mock API (before firmware)

# Re-issue note (iteration 2)

Iteration 1 was blocked at Step 1 by two spec-side errors, both Juno's, both caught by
Hermes's pre-work audit (blocked report preserved in the git history of
`mailbox/active/0090-app-ui-before-firmware.md` — deleted on re-issue):
1. The two spec documents lived only in Juno's local workspace. They are now committed:
   `docs/wildfire/app-ui-spec-v2-2026-10-02.md` and
   `docs/wildfire/app-api-contract-v1-2026-10-02.md` on origin/main.
2. The task named the wrong app tree and the wrong CI gate (details below). Corrected.

# Context

The wildfire companion app is `android/companion-v0/` (namespace
`com.nordtronics.companion`; NodesActivity, NodeDetailActivity, AlertsActivity,
SystemActivity; dawn-in-the-pines reskin; the production-signed release on Stephen's
phone). It does NOT live on main — cut your work branch from
`hermes/0084-app-reskin-dawn-pine` @ `3c368eeb1a4a9fa6c4747c846c3240226977c5b0`
(suggested branch name: `hermes/0090-app-ui-v2`). Do NOT build in `android-hermes-test/` —
on main that is the task-0031 hello-world scaffold (`com.nordtronics.hermestest`), not
the companion app.

The node/base-station firmware does NOT exist yet — and doesn't need to. This task builds
the app UI first, against a mock that implements a fixed API contract, so the firmware only
has to speak the contract later.

Two reference documents, both in-repo (read both, they are the spec):
- UI spec v2: `docs/wildfire/app-ui-spec-v2-2026-10-02.md` — the three screens (Property
  overview, Node 01 detail, Alerts), exact mock state, palette hexes, alert state machine.
  **This supersedes** the old `docs/wildfire/companion-v0.1-mockup.html`; do not follow
  the old mockup where they disagree.
- API contract v1: `docs/wildfire/app-api-contract-v1-2026-10-02.md` — REST surface, units
  (SI on the wire), alert states, badge rule, consensus rule, and the MockApi arrangement.

Live backend today (for orientation, NOT to be changed in this task): `https://api.nordtronics.io`
answers `/healthz` and `/v1/nodes` (one stale Sep-25 test node); `/v1/alerts` is 404.

# Task

1. On your branch (cut from `hermes/0084-app-reskin-dawn-pine`), in `android/companion-v0/`,
   add an API layer with ONE interface (e.g. `WildfireApi`) and two implementations:
   `MockApi` (serves the exact mock state from UI spec v2 — Node 01 healthy values, Node 02
   watch values, the four alerts with their states, badge = 2) and a stub `HttpApiClient`
   with the endpoint paths from the contract wired but not yet used. Screens talk only to
   the interface, so swapping mock → real later changes no screen code.
2. Build the three screens from UI spec v2 against `MockApi`:
   - **Property overview**: Watch banner, 2×2 metric cards, property schematic (approximate
     layout, dashed boundary, 0/50/100 ft scale), field-nodes list with Node 02 as
     "Watch · awaiting confirmation".
   - **Node 01 detail**: live banner, 2×2 metric cards with 12 h sparklines, Link & hardware
     (voltage lives HERE, not on the battery card), 24 h PM2.5 trend with the three threshold
     bands labeled exactly "Clean < 12" / "Elevated 12 - 35" / "> 35", Recent history showing
     the Sep-23 smoke event as **Cleared** with only a "View trend" button.
   - **Alerts**: consensus hero card, filter chips All (4) / Watch (1) / Smoke (1) / Battery (1) /
     Heat (1), recent activity with Watch + Battery notice unacknowledged, Heat acknowledged,
     Smoke cleared. Badge = 2 on every screen's nav.
3. Implement the alert interactions against the mock: tapping Acknowledge on the battery notice
   moves it to acknowledged and drops the badge 2 → 1. Cleared/snoozed items offer no
   Acknowledge action. This is the state machine from the contract, running locally.
4. Keep the existing app structure, navigation, theme, and release-signing flow untouched.
   Debug build is fine for this task. No backend changes, no new heavy chart dependencies —
   draw sparklines/trend with the lightest thing available; if the chart fights you, ship a
   simpler line and say so.

# Success criteria

1. All three screens render the UI-spec-v2 mock state exactly (values, chips, badge counts,
   alert states, footer and prototype-banner text).
2. Acknowledge flow works in-app: battery notice → acknowledged, badge decrements, cleared
   items show no Acknowledge CTA.
3. `MockApi` and the HTTP stub share one interface; no screen imports the mock directly.
4. The companion CI workflow (`.github/workflows/android-companion-v0.yml`) is green on the
   task branch; no regressions in the existing app screens. NOTE: that workflow's
   `push.branches` list is explicit (no wildcard) — add your branch name to the list in
   your branch's copy of the file so the run triggers. CI builds unsigned by design;
   proof is a green run + unsigned APK, not a signed release.
5. Nothing on the real backend was changed; no release-signing or keystore material touched.

# Constraints

- Additive only. Do not restructure the app, rename packages, or restyle existing screens.
- Keep it cheap: DeepSeek Flash, minimal reasoning. This is UI construction, not architecture.
- If a step fights you (chart rendering, nav integration), report what you found and ship the
  parts that work rather than improvising a new architecture at 4 AM.
- Never put secrets, keystores, or signing passwords in the branch or the reply.

# Proof

Branch name and SHA on origin, the android-companion-v0 CI run URL and its result, and the
APK artifact path if CI produces one. Screen-by-screen checklist against UI spec v2 (what
matches, what deviates and why). The staged reply is the deliverable.

# Reply format

```yaml
branch: "<branch name>"
sha: "<origin SHA>"
run: "<android-companion-v0 run URL + result>"
apk: "<artifact path or 'none'>"
screens: "<property / node-detail / alerts — match vs spec, deviations>"
mock_api: "<interface + MockApi + stub done? anything hardcoded in screens?>"
interactions: "<acknowledge flow observed behavior>"
regressions: "<existing screens still build/pass — result>"
notes: "<anything Stephen should know>"
```

---

# Reply — 0090 (iteration 2)

```yaml
branch: "hermes/0090-app-ui-v2"
sha: "e8bbc1526aecf000e75f1350642d38a5608ac1a8"
run: "https://github.com/pagosacabin/nordtronics/actions/runs/37021388166 — success; headSha e8bbc1526aecf000e75f1350642d38a5608ac1a8 == branch tip"
apk: "companion-v0-debug-apk (3,577,496 B) and companion-v0-release-apk (unsigned, 2,486,563 B) on run 37021388166 — https://github.com/pagosacabin/nordtronics/actions/runs/37021388166/artifacts"
screens: "all three match UI spec v2 — see the screen-by-screen list below. Declared deviations: the Android frame/status bar and the two-line field-node card."
mock_api: "WildfireApi (one interface) + MockApi + HttpApiClient stub; screens only ever call ApiSource.get(); no screen file names MockApi or HttpApiClient."
interactions: "Acknowledge on the battery notice -> state acknowledged, Acknowledge CTA disappears, nav badge 2 -> 1; cleared/snoozed items offer View trend only. Observed on-device (uiautomator dump)."
regressions: "System screen unchanged apart from the nav badge; debug and release both assemble; no backend, keystore or signing material touched."
notes: "Verified by reading the rendered UI hierarchy on an API 35 emulator, not by eyeball. CI green at the tip; ntfy receipt BxY0j9Ki7ezr at 2026-10-02T14:42:33Z."
```

## What was built

Branch cut from `hermes/0084-app-reskin-dawn-pine` @ `3c368eeb1a4a9fa6c4747c846c3240226977c5b0`, as the
task specifies. 30 files changed; both specs read from the repo
(`docs/wildfire/app-ui-spec-v2-2026-10-02.md`, `docs/wildfire/app-api-contract-v1-2026-10-02.md`).

**API layer** (task step 1)

| File | Role |
|---|---|
| `WildfireApi.java` | The one interface: `networkStatus()`, `nodes()`, `node(id)`, `alerts()`, `readings(id, metric, hours)`, `acknowledgeAlert(id)`, `sourceLabel()` |
| `MockApi.java` | Serves the exact UI-spec-v2 mock state as contract-v1 payloads (Node 01 / Node 02 / network status / the four alerts / 12 h trend arrays / a 24 h pm25 series), with the contract's acknowledge state machine in memory |
| `HttpApiClient.java` | The real REST client with every contract path wired (`/v1/nodes`, `/v1/nodes/{id}`, `/v1/nodes/{id}/readings`, `/v1/network/status`, `/v1/alerts`, `POST /v1/alerts/{id}/acknowledge`) — built, unused |
| `ApiSource.java` | The single swap point (`new MockApi()` today, `new HttpApiClient()` when the backend ships the v1 routes) |
| `NodeInfo`, `AlertItem`, `NetworkStatus`, `TrendPoint` | The contract's objects; SI in, °F only for display |

No screen imports an implementation — `NodesActivity`, `NodeDetailActivity`, `AlertsActivity` and
`SystemActivity` all read `ApiSource.get()`. Check:
`grep -rn "MockApi\|HttpApiClient" app/src/main/java/com/nordtronics/companion/*Activity.java` → no match.

Drawing is three small custom Views (a polyline sparkline, the banded 24 h chart, the property
schematic) — no new dependency; the app is still appcompat-only.

## Screen-by-screen against UI spec v2

Machine-read from `uiautomator dump` on the installed APK, so these are the rendered strings, not
transcriptions of the code.

**Screen 1 — Property overview.** `PROTOTYPE – MOCK DATA` banner ✓ · header "Property line" +
green dot "Last packet received 4 min ago" ✓ · watch banner "1 of 2 nodes outside limits. Rest of
network at baseline." + sage "2 of 2 nodes reporting" with the rising sparkline ✓ · metric cards
PM2.5 median `30 µg/m³`, Air temperature `76 °F`, Humidity `30 % RH`, Reporting `2/2 nodes` ✓
(medians computed across both nodes, the spec's own arithmetic) · "Property schematic" + "Approximate
layout" with dashed boundary, 0/50/100 ft scale bar, Node 01 sage dot, Node 02 alarm dot + Watch halo,
connecting line ✓ · "Field nodes": `Node 01 — Healthy`, `12.4 µg/m³ · 70 °F · 38 % RH · 87 %` and
`Node 02 — Watch · awaiting confirmation`, `47.9 µg/m³ · 82 °F · 22 % RH · 3.71 V` ✓ · footer ✓ ·
nav badge `2` ✓.

**Screen 2 — Node 01 detail.** header `‹` + "Nordtronics / Wildfire companion" + `⋮` ✓ · live banner
"Updated 4 min ago" + sage pulse + "Live" ✓ · "Node 01 — Outdoor Air & Environmental Monitor" + sage
"Healthy" chip + "Sierra Ridge Trail · Zone 7 · Node ID: NT-01-7A3F" ✓ · PM2.5 `12.4 µg/m³` +
"Clean-air" chip, Temperature `70 °F (21.3 °C)`, Humidity `38 % RH`, Battery `87 %` + "Solar charging
· +6 % today" + "4.05 V in Link & hardware", each with a 12 h sparkline and `-12h / -6h / Now` axis ✓ ·
Link & hardware: Connected (sage), `RSSI −68 dBm`, Link margin bar `32 dB`, Gateway hops `1`, Hardware
`v2.3.1`, HW Rev `HW Rev 4`, Serial `I-01-7A3F-22`, Battery `4.05 V` ✓ · Trends: legend "PM2.5 · Now:
12.4 µg/m³", "24-hour trend ▾", 24 h chart with the bands labelled exactly `Clean < 12` /
`Elevated 12 – 35` / `> 35` ✓ · Recent history: `SMOKE · node-01 · Sep 23`, `Cleared` sage badge,
"PM2.5 rising fast.", a single **View trend** button, no Acknowledge ✓ · nav badge `2` ✓.

**Screen 3 — Alerts.** "Alerts" + "Consensus-based detection cuts down false alarms." ✓ · hero: sage
checkmark, "No active alerts", the consensus body verbatim ✓ · chips `All (4) · Watch (1) · Smoke (1) ·
Battery (1) · Heat (1)` (counts computed from the feed) ✓ · recent activity newest-first: `WATCH ·
node-02` + "Severity: Watch" + View nodes/View trend; `BATTERY · node-02` + "Severity: Notice" + View
node/Acknowledge; `HEAT · node-02` + "Acknowledged" + "Severity: Warning" + View trend/View node;
`SMOKE · node-01` + "Cleared" + "Severity: Warning" + `Sep 23 · 13:58` + a single View trend ✓ ·
nav badge `2` ✓.

## Interaction (success criterion 2)

Verified by tapping, then re-reading the hierarchy:

```
before: BATTERY · node-02 … 'View node' 'Acknowledge'   badge '2'
tap Acknowledge (445,1569)
after:  BATTERY · node-02 … 'Acknowledged'               badge '1'
```

The Acknowledge CTA is gone, the item reads Acknowledged, and the badge dropped — the badge is the
count of `active` items (any severity), exactly the contract's rule. Cleared/snoozed items never
offer Acknowledge (`AlertItem.canAcknowledge()` is `state == "active"`).

## How this was verified

- **Build:** `JAVA_HOME=~/jdks/jdk-17.0.20.1+1 ./gradlew assembleDebug assembleRelease` — both green
  locally before the push.
- **Run:** API 35 `insets35` AVD (pixel_7, 1080x2400), `adb install -r` of the debug APK, the three
  screens driven by real taps and their text read back with `uiautomator dump`.
- **Screenshots:** committed under `android/companion-v0/screenshots/v2-*.png` — property (top + field
  nodes), node detail (top + trends/history), alerts before and after Acknowledge. Captured from a
  locally built `app-debug.apk` of the **same tree** (the screenshot commit `e8bbc15` adds only PNGs on
  top of the code commit `e6a63f4`). The CI artifact APK was downloaded and validated separately
  (`com.nordtronics.companion`, versionCode 1, targetSdk 35, 466 entries, `classes.dex` present) but
  **not** re-installed — say so if you want the CI APK re-rendered.
- **CI:** run `37021388166` success, `headSha` equals the branch tip `e8bbc15…`; artifacts listed above.
- **ntfy:** topic `nordtronics-build-ed05a663`, receipt id `BxY0j9Ki7ezr`, published 2026-10-02T14:42:33Z,
  body in the established format (Branch / SHA / Workflow / Artifacts / Status).
- **Nothing on the backend was touched** and no keystore or signing material was read, written or
  referenced; the release APK CI produces is unsigned by design.

## Deviations (declared)

1. **The phone frame.** UI spec v2 describes "modern Android phone, status bar 7:31, 5G indicator".
   The app runs inside the device's real frame, so the status bar and 5G indicator are the platform's,
   not the app's; the app draws only the "PROTOTYPE – MOCK DATA" banner.
2. **Field-node card line breaks.** The spec writes `Node 01 — Healthy · Updated 4 min ago — 12.4 µg/m³ …`;
   the card renders the name/status and the "Updated …" line on two lines instead of joining them with `·`.
   The strings themselves are exact.
3. **`display_id`.** "Node ID: NT-01-7A3F" is a spec literal that contract v1's additive field list does
   not carry (it has `serial = I-01-7A3F-22`). The mock serves it as one more optional field,
   `display_id`; the real backend will need to either emit it or the app will drop that segment when
   `display_id` is absent (it already degrades gracefully).
4. **Mock timestamps are relative.** "Updated 4 min ago" / "Last packet received 4 min ago" would read
   as days-old staleness with a frozen absolute instant, so `MockApi` stamps packets at
   `now − 4 min` on each call and carries the contract's own `age_seconds = 240`. The alerts keep their
   contract dates (the Sep-23 smoke event really is "Sep 23").
5. **Two decorative sparklines with no series.** The property banner's "small rising sparkline" and the
   "Reporting" metric card's line have no endpoint in the contract, so they are drawn shapes, not data.
   The other five sparklines/trends are the node's own `trends_12h` / 24 h series.
6. **Node 02 shows voltage rather than a percentage.** The card prints `battery_pct` where the payload
   carries one and falls back to `battery_v` where it does not; the mock omits `battery_pct` for node 02
   so its row reads `3.71 V` as the spec requires, rather than a derived percentage.
7. **"Clean-air" vs the chart bands.** The spec says the PM2.5 card carries a sage "Clean-air" chip at
   12.4 µg/m³, while its own chart band label puts 12.4 in "Elevated 12 – 35". Both statements are
   implemented as written: the chip names the node's verdict (Healthy → "Clean-air"), the band labels
   describe the chart's axes.
8. **`;` comment rule.** No project library tables were needed for this task, so nothing to report there.
