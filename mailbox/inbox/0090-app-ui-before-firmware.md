---
task_id: "0090"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
proof:
  branch: ""
  sha: ""
  run: ""
---

# 0090 — Wildfire companion app: build the three v2 screens against a mock API (before firmware)

# Context

The wildfire companion app exists at `android-hermes-test/` in pagosacabin/nordtronics
(existing dawn-in-the-pines reskin, production-signed release already on Stephen's phone).
The node/base-station firmware does NOT exist yet — and doesn't need to. This task builds
the app UI first, against a mock that implements a fixed API contract, so the firmware only
has to speak the contract later.

Two reference documents (read both, they are the spec):
- UI spec v2: `workspace/goals/lora-wildfire-early-warning-network/files/app-ui-spec-v2-2026-10-02.md`
  — the three screens (Property overview, Node 01 detail, Alerts), exact mock state, palette
  hexes, alert state machine. **This supersedes** the old `docs/wildfire/companion-v0.1-mockup.html`;
  do not follow the old mockup where they disagree.
- API contract v1: `workspace/goals/lora-wildfire-early-warning-network/files/app-api-contract-v1-2026-10-02.md`
  — REST surface, units (SI on the wire), alert states, badge rule, consensus rule,
  and the MockApi arrangement.

Live backend today (for orientation, NOT to be changed in this task): `https://api.nordtronics.io`
answers `/healthz` and `/v1/nodes` (one stale Sep-25 test node); `/v1/alerts` is 404.

# Task

1. In `android-hermes-test/`, add an API layer with ONE interface (e.g. `WildfireApi`) and two
   implementations: `MockApi` (serves the exact mock state from UI spec v2 — Node 01 healthy
   values, Node 02 watch values, the four alerts with their states, badge = 2) and a stub
   `HttpApiClient` with the endpoint paths from the contract wired but not yet used.
   Screens talk only to the interface, so swapping mock → real later changes no screen code.
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
4. Android CI build (`android-build.yml`) is green on the task branch; no regressions in the
   existing app screens.
5. Nothing on the real backend was changed; no release-signing or keystore material touched.

# Constraints

- Additive only. Do not restructure the app, rename packages, or restyle existing screens.
- Keep it cheap: DeepSeek Flash, minimal reasoning. This is UI construction, not architecture.
- If a step fights you (chart rendering, nav integration), report what you found and ship the
  parts that work rather than improvising a new architecture at 4 AM.
- Never put secrets, keystores, or signing passwords in the branch or the reply.

# Proof

Branch name and SHA on origin, the android-build CI run URL and its result, and the APK
artifact path if CI produces one. Screen-by-screen checklist against UI spec v2 (what matches,
what deviates and why). The staged reply is the deliverable.

# Reply format

```yaml
branch: "<branch name>"
sha: "<origin SHA>"
run: "<android-build run URL + result>"
apk: "<artifact path or 'none'>"
screens: "<property / node-detail / alerts — match vs spec, deviations>"
mock_api: "<interface + MockApi + stub done? anything hardcoded in screens?>"
interactions: "<acknowledge flow observed behavior>"
regressions: "<existing screens still build/pass — result>"
notes: "<anything Stephen should know>"
```
