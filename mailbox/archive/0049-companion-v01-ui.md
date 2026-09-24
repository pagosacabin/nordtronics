---
task_id: "0049"
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0049-companion-v01-ui
    sha: 0cc5e3d66c42ad66932c6db88868df58f05e4d4a
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35943464226
  - ntfy: nordtronics-build-ed05a663 @ 2026-09-24T01:40:13Z
  - files:
      - android/companion-v0/screenshots/v01-nodes.png
      - android/companion-v0/screenshots/v01-node-detail.png
      - android/companion-v0/screenshots/v01-node-detail-ping.png
      - android/companion-v0/screenshots/v01-alerts.png
      - android/companion-v0/screenshots/v01-system.png
      - android/companion-v0/screenshots/splash.png
notes: |
  Base branch: cut from the hermes/0048-app-batch-1 tip eb24b91 (0048 is
  archived/verified but not merged, so origin/main still carries no android/
  tree at all — `git ls-tree origin/main android/` is empty).

  The four mockup screens are implemented in the Companion v0 app module, dark
  Nordtronics palette taken from the mockup itself: window #121718, surface
  #242D2D, rail #171D1E, ink #F2F4EF, accent #FF9E36, ok #65C99B, warn #FF9B6E,
  blue #74BBD1. Every screen carries the brand row (mark + NORDTRONICS / Wildfire
  companion + "Prototype data" pill) and the Nodes / Alerts / System bottom nav,
  and each screen is selected in it.

  - Nodes ("Property line"): network-consensus hero (reporting count, state
    disc, all-clear/watch copy), 2x2 stat grid (PM2.5 median, air temperature,
    humidity, reporting N/N), "Field nodes" head with the All/Watch filter, and
    one card per node carrying status, PM2.5, temperature, humidity, battery
    voltage and last seen; tapping a card opens the drill-down (proved by the
    activity switch, not just clickable=true).
  - Node detail: back link, node header, 2x2 reading grid, "Link & hardware"
    bars, the node's own recent history from /api/alerts, and the ping-test
    button. The ping round trip was driven for real: POST
    /api/nodes/node-01/ping -> 200 {"ok": true, "node_id": "node-01"}, shown in
    v01-node-detail-ping.png and in the mock server's log line.
  - Alerts: consensus statement hero ("No active alerts" — nothing inside the
    6 h active window), All/Smoke/Battery/Heat filter, history rows with type,
    node, message, timestamp and a derived severity label.
  - System (new activity): backend connection status from a live GET /api/nodes
    (ONLINE + address + "2 NODES · <time>"), the privacy panel (only
    environmental telemetry leaves the property; cameras/audio NONE; location
    trail NONE), the prototype/mock-mode indicator naming the mock backend as
    the data source, and About showing version v0.1 (from
    BuildConfig.VERSION_NAME).

  Splash: the theme now points windowSplashScreenAnimatedIcon at a new
  res/drawable-*/splash_lockup.png — the full lockup, emblem with the NORDTRONICS
  wordmark beneath it, derived from
  branding/nordtronics-logo/nordtronics-logo-full-lockup.png by
  tools/make_branding_assets.py (measured lockup box rows 232..1068 / cols
  345..1567; the faint texture band between emblem and wordmark is ramped out of
  the alpha channel so it does not ghost). splash.png is re-captured and
  measured: 98.6 % of the frame is exactly #222629, the emblem band sits at rows
  1041..1240 and the wordmark band at rows 1317..1357, both centred on the frame
  (x ≈ 545 vs 540). No new artwork and no new dependency; splash_mark.png stays
  in the tree from 0048.

  Deviations from the mockup, all deliberate:

  1. The mock payload has no LoRa RSSI and no packet-delivery percentage, so the
     detail screen's health panel uses values the API does return — packet
     recency, PM2.5 headroom against the 35 ug/m3 watch level, and battery
     voltage — instead of inventing dBm figures to match the mockup's bars.
  2. Cards show battery voltage (the task asks for battery voltage) rather than
     the mockup's battery percentage; the detail screen shows both (87 % derived
     from a 3.00-4.20 V cell, with the volts).
  3. Node names are rendered from the API ids ("Node 01"), since the payload has
     no name field; the mock backend itself is unchanged, as the constraints
     require.
  4. The hero reads "Watch", not "All clear": the mock data has node-02 at
     47.9 ug/m3, so the consensus header reports the state of the data it was
     given rather than the mockup's placeholder copy. With only clean readings it
     shows "All clear".
  5. The module is dark-first now, so the status/navigation bar icons were
     switched to light in WindowInsetsHelper — with 0048's dark-icon setting they
     were invisible against #121718 (the 0047 failure mode, inverted).

  Screenshots are real emulator captures: API 35 / Pixel 7 AVD (insets35),
  emulator WiFi disabled so the guest reaches the host loopback mock on
  10.0.2.2:8000. They come from the CI-built APK, not a local build — the
  committed PNGs were captured from artifact 10784833398 (run 35942579948, the
  code commit 2756817), and artifact 10786190782 (this run, 0cc5e3d) was
  installed and re-rendered as a control: the two artifacts are byte-identical
  except classes*.dex ordering, and their rendered screens differ only in the
  status-bar clock and the System screen's heartbeat clock, i.e. no visual
  difference at all outside the two clock fields.

  CI: run 35943464226, conclusion success, head SHA 0cc5e3d66c42ad66932c6db88868df58f05e4d4a
  = the live branch tip. Job "build" green on every step, including "Assemble
  debug APK". Artifacts: companion-v0-debug-apk 10786190782,
  companion-v0-screenshots 10785951643. The workflow's on.push.branches had to
  list hermes/0049-companion-v01-ui (per-task branch pinning) — the workflow file
  is itself in the workflow's paths, so that edit is what triggers the build.

  ntfy: published to nordtronics-build-ed05a663 on https://ntfy.sh, HTTP 200,
  message id k1ZbcOG308ji, timestamp 2026-09-24T01:40:13Z.

  Leftovers, not fixed here: the older 0045-0048 screenshots (nodes.png,
  alerts.png, node-detail.png, nodes-insets.png, launcher-*.png,
  inapp-branding.png) still sit in screenshots/ showing the pre-0049 light UI;
  they are earlier tasks' evidence and were left in place rather than deleted.
  Nothing from 0045..0049 is merged to main yet, so the app still exists only on
  the hermes/00xx branches.
---

# 0049 — Companion v0.1 UI: implement the approved mockup

## Context

Stephen approved the v0.1 UI mockup (2026-09-23). The mockup is committed in
the repo as the design reference — open it in a browser:

- `docs/wildfire/companion-v0.1-mockup.html` (commit `5c24618b1c6665963c47370b9fee0a2d82459d64`)

This task implements that mockup in the real Companion app. The 0048 batch
(launcher label, branding graphics, splash screen) is assumed landed — cut from
the 0048 branch tip once staged, else from `main` after 0048 merges.

## Task

Implement the v0.1 screens in the Companion v0 app module, matching the
mockup's layout, content, and dark Nordtronics branding:

1. **Nodes screen** — node cards showing status, PM2.5, temperature/humidity,
   battery voltage, and last-seen; network consensus status header.
2. **Node detail** — drill-down per node: current readings, recent history,
   ping-test button.
3. **Alerts screen** — alert history list with severity/type filtering.
4. **System / Privacy screen** — backend connection status, the privacy
   statement (only environmental telemetry leaves the house), mock-mode
   indicator.
5. App displays version **v0.1** (About/system screen).
6. **Splash logo** — the splash screen currently shows the bare emblem; change
   it to the full lockup: emblem with the NORDTRONICS wordmark beneath it
   (derive from `branding/nordtronics-logo/nordtronics-logo-full-lockup.png`,
   same approach as `tools/make_branding_assets.py`), matching the v0.1 mockup.

Data layer: keep the existing mock-server data layer from 0045 — screens show
mock data, clearly labeled as prototype/demo data. No real backend work.

## Success criteria

- Branch `hermes/0049-companion-v01-ui` exists on origin (base per context
  above; state the base in the reply).
- A green CI run builds the debug APK.
- Emulator screenshots committed under `android/companion-v0/screenshots/`:
  `v01-nodes.png`, `v01-node-detail.png`, `v01-alerts.png`,
  `v01-system.png` — each visibly matching the corresponding mockup screen.
- Splash screen and branding from 0048 still intact, with the splash now showing
  the full lockup (emblem + NORDTRONICS wordmark); re-capture `splash.png` as
  proof of the logo change.

## Constraints

- Push only to `hermes/0049-companion-v01-ui`. Never push to main.
- Reuse existing branding assets under `branding/nordtronics-logo/`. No new
  artwork, no new dependencies.
- Do not change the mock server, the API base URL mechanism, or network code.
- No push notifications, no Play Store, no release signing.

## Proof

- `branch` + `sha` (branch exists on origin, tip equals `sha`).
- `run`: Actions run URL, green, head SHA matches branch tip.
- `ntfy`: build-green publish receipt — topic + timestamp, published to
  `nordtronics-build-ed05a663` on `https://ntfy.sh` once the run is green.

## Reply format

```yaml
proof:
  - branch: hermes/0049-companion-v01-ui
    sha: <full tip sha>
  - run: <actions run url>
  - ntfy: <topic> @ <timestamp>
notes: <base branch used, screens implemented, any deviations from mockup>
```
