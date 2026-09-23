---
task_id: "0048"
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0048-app-batch-1
    sha: eb24b91a0408f22c201af8cd823f6a6c33894869
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35932082994
  - ntfy: nordtronics-build-ed05a663 @ 2026-09-23T23:12:09Z
  - files:
      - android/companion-v0/screenshots/launcher-branding.png
      - android/companion-v0/screenshots/splash.png
      - android/companion-v0/screenshots/inapp-branding.png
notes: |
  Base branch: cut from hermes/0047-nodes-toolbar-insets tip 5ac8fc1 — 0047 was
  staged when this was picked up and is archived now, but neither 0046 nor 0047
  has been merged, so `origin/main` still carries no android/ tree at all
  (`git ls-tree origin/main android/` is empty); 0047's tip was the only base
  that contains the app.

  Where the in-app mark went: the app bar. The header row of all three screens
  (Nodes, Alerts, Node detail) is now an ImageView + TextView pair instead of a
  bare TextView, so the emblem sits left of the title on every screen. That
  renamed the text node: R.id.header is the header row and R.id.header_title the
  text inside it, and NodeDetailActivity (which sets the header text to
  "Node detail — <id>") follows. The in-app header text is unchanged — "Nodes"
  comes from @string/nodes_title in the layout, so removing the activity label
  fixes the launcher entry without touching the screen titles.

  Assets: no new artwork. tools/make_branding_assets.py lifts the emblem out of
  branding/nordtronics-logo/nordtronics-logo-mark-only.png (that file is an
  opaque #222629 plate carrying the emblem, rows 381..1041, above a grey
  wordmark, rows 1075..1218) onto transparency and emits the density set —
  adaptive foreground, legacy mipmap PNGs, the two drawables. The brand dark for
  the icon background layer and the splash background is #222629, sampled from
  that plate rather than hand-picked. The legacy ic_launcher.png set was
  regenerated from the same asset, so API 24..25 no longer shows the 0046
  placeholder.

  Deviations, both deliberate:

  1. Splash uses the platform SplashScreen API — the values-v31 theme attributes
     windowSplashScreenBackground / windowSplashScreenAnimatedIcon on a new
     Theme.Companion.Start (child of Theme.Companion, so the activity keeps one
     theme for its whole life). The androidx core-splashscreen library would have
     been a new dependency, which the constraints exclude; the platform API is
     the Android 12+ API the task names. Below API 31 the values-v31 folder does
     not apply and start-up is unchanged (no pre-12 splash).
  2. tools/make_branding_assets.py is committed so the derivation is
     reproducible. It is a host-side script (Pillow), not an app dependency; the
     app module gains nothing.

  Screenshots are real emulator captures, not mockups: API 35 / Pixel 7 AVD
  (insets35), app-debug.apk from this branch installed with `adb install -r`,
  mock server on the host for real list data.

  - splash.png — the first frame of a cold start, captured by a device-side
    screencap loop started in the same adb shell as `am start` after
    `am force-stop`. Measured: background pixel-for-pixel (34,38,41) = #222629,
    the emblem's orange bbox 459x307 px centred at (539.5,1199.5) against a
    frame centre of (540,1200) — 459 px = 175 dp at 420 dpi, i.e. the intended
    176 dp.
  - launcher-branding.png — the launcher's app drawer, with the uiautomator tree
    confirming the entry reads "Nordtronics Companion". The icon cell measures a
    #222629 disc carrying the mark in exactly (247,165,67).
  - inapp-branding.png — the Nodes screen with the mark in the app bar and live
    data ("2 nodes from http://10.0.2.2:8000/api/nodes").

  CI: run 35932082994, conclusion success, head SHA eb24b91a0408 (branch tip).
  Job "build" success on every step. The CI-built artifact was downloaded
  (artifact 10781965313, 3,255,246 bytes) and read back, not just the local
  build: `aapt2 dump badging` on the CI APK gives application label "Nordtronics
  Companion", icon res/mipmap-anydpi-v26/ic_launcher.xml, launchable-activity
  NodesActivity, and the APK contains ic_launcher_foreground at all five
  densities plus drawable-*-v4/splash_mark.png and brand_mark.png.

  ntfy: published to nordtronics-build-ed05a663 on https://ntfy.sh, HTTP 200,
  message id YeMAIi4Iy6IV, timestamp 2026-09-23T23:12:09Z.

  Leftovers, not fixed here: the mockup added on main as 5c24618 arrived after
  this branch was cut, so it is not in the branch history; and nothing from
  0045/0046/0047/0048 is on main yet, so the app only exists on the
  hermes/00xx branches until those are merged.
---

# 0048 — App batch 1: launcher label + branding graphics + splash screen

## Context

Batching policy (Stephen, 2026-09-23): small app changes now batch into single
tasks instead of one task per fix. 0045/0046 verified and archived; 0047 (Nodes
toolbar insets) is in flight. Branding assets already in the repo — no new
artwork needed:

- `branding/nordtronics-logo/nordtronics-logo-mark-only.png` (app mark)
- `branding/nordtronics-logo/nordtronics-logo-full-lockup.png` (full lockup)

Known wart carried in: the launcher entry reads "Nodes" because NodesActivity's
`android:label` overrides the application label (noted on 0046, deferred here).

## Task

One batch, three changes, all in the Companion v0 app module:

1. **Launcher label** — set the launcher activity's `android:label` so the
   launcher entry reads "Nordtronics Companion", not "Nodes".
2. **Branding graphics** — wire the repo logo assets in as drawables:
   - Refine the launcher icon into a proper adaptive icon (foreground =
     mark-only asset, background = brand dark color) replacing the 0046
     stopgap.
   - Show the mark in-app: app bar or an About screen (your call, say which
     in the reply).
3. **Splash screen** — Android 12+ SplashScreen API: Nordtronics mark centered
   on the brand dark background, shown on cold start.

## Success criteria

- Branch `hermes/0048-app-batch-1` exists on origin. Cut it from the
  `hermes/0047-nodes-toolbar-insets` tip if 0047 has staged, else from `main`
  after 0046/0047 merge — check first, state the base in the reply.
- A green CI run builds the debug APK.
- Screenshots committed under `android/companion-v0/screenshots/`:
  - `launcher-branding.png` — device launcher showing "Nordtronics Companion"
    with the refined icon.
  - `splash.png` — the splash screen as seen on cold start.
  - `inapp-branding.png` — wherever the mark appears in-app.
- All three screenshots real (emulator), not mockups.

## Constraints

- Push only to `hermes/0048-app-batch-1`. Never push to main.
- Reuse the existing assets under `branding/nordtronics-logo/`. No new
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
  - branch: hermes/0048-app-batch-1
    sha: <full tip sha>
  - run: <actions run url>
  - ntfy: <topic> @ <timestamp>
notes: <base branch used, where the in-app mark went, any deviations>
```
