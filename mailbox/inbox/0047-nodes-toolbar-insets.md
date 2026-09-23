---
task_id: "0047"
status: inbox
iteration: 0
expect-reply-within: 6h
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
