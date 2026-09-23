---
task_id: "0047"
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0047-nodes-toolbar-insets
    sha: 5ac8fc1756b9a37251a9b99244cf05562f974d6b
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35928274738
  - ntfy: nordtronics-build-ed05a663 @ 2026-09-23T22:30:36Z
  - files:
      - android/companion-v0/screenshots/nodes-insets.png
notes: >
  Base branch: cut from hermes/0046-companion-icon tip 89e9ca8 — 0046 is
  verified and archived, but it was never merged, so `main` carries no android/
  tree at all and 0046's tip was the only base that contains the app.

  Root cause, measured rather than assumed. On the API 35 emulator before the
  fix, uiautomator reported the content frame (android:id/content) at
  [0,0][1080,2400] — the full window — with the app's own first row (REFRESH /
  ALERTS, clickable=true) at y=32..158, in the same place as the status bar,
  while the AppCompat action bar was offset to [0,136][1080,283]. The toolbar
  was drawn underneath the "Nodes" header bar and taps landed on the header,
  exactly as reported on the physical device. That is targetSdk 35 edge-to-edge
  enforcement: on Android 15+ the decor no longer lays content below the action
  bar. The 0045 screenshots came from an Android 13 image, where the framework
  still padded the window, which is why the overlap was invisible there and why
  an emulator-only check missed it.

  Fix (branch tip, one commit): window insets — systemBars plus displayCutout —
  are applied to the content area of every screen by a new
  WindowInsetsHelper.java, which also sets setDecorFitsSystemWindows(false) so
  the behaviour is identical from API 24 up instead of depending on how a given
  framework version pads the window; and the framework action bar is replaced by
  an in-layout header bar per screen (theme parent Theme.AppCompat.Light
  .NoActionBar), so nothing can be laid over the toolbar row. Light status- and
  navigation-bar icon appearance is set so the reserved strips stay readable.

  Verified on the CI-built branch, not just locally: Android 15 emulator
  (system-images;android-35;google_apis;x86_64, device profile pixel_7,
  1080x2400 @420dpi, status bar 136 px, gesture-nav inset 63 px). After the fix
  the header bar is at [0,136][1080,281] — below the status bar — and REFRESH
  [565,313][796,439] and ALERTS [817,313][1048,439] are fully inside the screen
  below it, with the node list rendered from the mock server two nodes deep.
  Tapability was checked by tapping ALERTS at its centre, which navigated to
  AlertsActivity, and tapping a node row, which opened NodeDetailActivity with
  its own header ("Node detail — node-01"); the Alerts screen shows the same
  corrected structure (header [0,136][1080,281], buttons [565,313][796,439] and
  [817,313][1048,439]). The screenshot at
  android/companion-v0/screenshots/nodes-insets.png is that device profile, real
  pixels, captured with adb exec-out screencap.

  Deviations to know about:
  (1) The fix is a theme-level change, so it applies to all three screens —
  Alerts and Node detail got the same header bar and inset handling; their node
  id / screen title now lives in the header (NodeDetailActivity no longer calls
  setTitle). Leaving two screens on the action bar would have kept the same bug
  there.
  (2) .github/workflows/android-companion-v0.yml triggered only on
  hermes/0045-companion-v0 and hermes/0046-companion-icon, so this branch name
  was added to the push list or no build would have run at all (same change 0046
  needed).
  (3) The earlier three screenshots (nodes.png, alerts.png, node-detail.png)
  were captured on the Android 13 image and still show the old action bar. They
  are accurate as history and were left untouched; only the new file was added,
  as specified.
  (4) The mock server, the API base URL mechanism and the network code are
  unchanged, as constrained; no push notifications, no signing, no release.
---

# 0047 — Companion v0: Nodes screen toolbar hidden under header on physical device

## Context

0045 verified and archived. Stephen installed the CI APK on his physical phone:
the Nodes screen's toolbar (REFRESH / ALERTS buttons) renders underneath the
"Nodes" header bar and cannot be tapped. The emulator screenshots in 0045 did
not show this — it is a system-bar insets / edge-to-edge layout issue that only
appears with real device insets. (The `10.0.2.2` connection failure on his phone
is expected — that alias only works from the emulator — and is NOT part of this
task. The mock server stays localhost-only.)

## Task

Fix the Nodes screen layout so the toolbar (REFRESH / ALERTS) is fully visible
and tappable below the header on a physical device: handle window insets
properly (fitsSystemWindows / WindowInsets) instead of assuming emulator
insets.

## Success criteria

- Branch `hermes/0047-nodes-toolbar-insets` exists on origin, cut from the
  `hermes/0046-companion-icon` tip (or `main` if 0046 has merged — check first,
  say which in the reply).
- A green CI run builds the debug APK.
- An emulator screenshot taken with a device profile that applies real
  system-bar insets (not the inset-free profile used in 0045), showing the
  Nodes screen with REFRESH and ALERTS both fully visible below the header,
  committed at `android/companion-v0/screenshots/nodes-insets.png`.

## Constraints

- Push only to `hermes/0047-nodes-toolbar-insets`. Never push to main.
- Do not change the mock server, the API base URL mechanism, or any network code.
- No push notifications, no Play Store, no release signing.

## Proof

- `branch` + `sha` (branch exists on origin, tip equals `sha`).
- `run`: Actions run URL, green, head SHA matches branch tip.
- `ntfy`: build-green publish receipt — topic + timestamp, published to
  `nordtronics-build-ed05a663` on `https://ntfy.sh` once the run is green.

## Reply format

```yaml
proof:
  - branch: hermes/0047-nodes-toolbar-insets
    sha: <full tip sha>
  - run: <actions run url>
  - ntfy: <topic> @ <timestamp>
notes: <base branch used, screenshot path, any deviations>
```

---

## Reply

```yaml
base_branch: >
  hermes/0046-companion-icon @ 89e9ca86e53d9430e9bfc78a9de1a014f8dfca30 —
  0046 is verified/archived but was never merged, and main has no android/ tree,
  so 0046's tip was the only base containing the app.

screenshot_path: >
  android/companion-v0/screenshots/nodes-insets.png — Android 15 (API 35,
  system-images;android-35;google_apis;x86_64) on the pixel_7 device profile,
  1080x2400 @420dpi, status bar 136 px and a 63 px gesture-nav inset. Header bar
  at [0,136][1080,281]; REFRESH [565,313][796,439] and ALERTS [817,313][1048,439]
  both fully visible below it and inside the screen.

inset_handling: >
  WindowInsetsHelper.applySystemBarInsets(this) in each activity: the content
  area is padded by the live systemBars + displayCutout insets, with
  setDecorFitsSystemWindows(false) making that the single source of layout truth
  on every API level (24+). The framework action bar is gone — each screen owns
  its header bar — so no decor view is left to draw over the toolbar row.

green_run: https://github.com/pagosacabin/nordtronics/actions/runs/35928274738
build_green_ntfy: nordtronics-build-ed05a663 @ 2026-09-23T22:30:36Z (id T7VVE1n2E5Me)

notes: >
  Base branch, root cause with uiautomator bounds, the four deviations, and the
  tap-test evidence are in the front-matter `notes` above.
```
