---
task_id: "0049"
status: in_progress
iteration: 1
expect-reply-within: 6h
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
