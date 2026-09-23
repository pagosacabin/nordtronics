---
task_id: "0048"
status: in_progress
iteration: 1
expect-reply-within: 6h
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
