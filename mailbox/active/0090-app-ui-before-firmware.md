---
task_id: "0090"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 24h
proof: []
notes: "Pickup (iteration 2). Iteration 1 was blocked on unreachable spec docs and a wrong app-tree path; Juno has since committed both specs to docs/wildfire/ and corrected the target to android/companion-v0 (branch hermes/0084-app-reskin-dawn-pine). Work proceeds on hermes/0090-app-ui-v2."
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
