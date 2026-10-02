---
task_id: "0090"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 24h
proof: []
notes: |
  BLOCKED AT STEP 1 — THE TASK'S OWN SPEC IS NOT REACHABLE, AND ITS APP PATH
  DOES NOT HOLD THE APP IT DESCRIBES. There is NO proof block: no branch, no CI
  run, no screen, no APK. Nothing outside mailbox/ was touched. A re-run
  reproduces this exactly.

  BLOCKER 1 — the two documents the task calls "the spec" are not in the repo,
  not on any branch, not on GitHub, and not on this host or the VPS.
    - Task names (Juno's workspace paths):
        workspace/goals/lora-wildfire-early-warning-network/files/app-ui-spec-v2-2026-10-02.md
        workspace/goals/lora-wildfire-early-warning-network/files/app-api-contract-v1-2026-10-02.md
    - `git ls-tree -r --name-only origin/main | grep -iE 'app-ui|api-contract'`
      -> no match. All 52 remote branches scanned for both filenames -> no match.
    - `gh search code "app-ui-spec-v2"` -> one hit, and it is
      mailbox/inbox/0090 itself. Same for "app-api-contract-v1".
    - `gh search repos "lora-wildfire-early-warning-network"` -> nothing; the
      token's repo list is nordtronics, hermes-agent, pagosacabin, Pi-projects.
    - `grep -rli 'Property overview'` over the repo and over $HOME -> only
      mailbox/inbox/0090. `find / -xdev -iname '*app-ui-spec*' -o -iname
      '*app-api-contract*'` on this host -> nothing; the same find over
      /home/deploy /opt /srv /var/www on the VPS (deploy@89.117.21.105) ->
      nothing. Repo carries exactly one PDF (docs/vps-stack-one-pager.pdf).
    - Precedent that makes this a gap and not a convention: 0091's input IS in
      the repo (docs/wildfire/detection-logic-spec-v0.1-review-notes.md, added
      by 65a32e4 "Transcribed from the reviewed PDF into the repo so the
      simulation task has a readable, versioned input"), and 0091 repeats every
      value it needs inline. No equivalent exists for 0090's two documents.
    - Consequence: success criterion 1 ("render the UI-spec-v2 mock state
      exactly (values, chips, badge counts, alert states, footer and
      prototype-banner text)") is underdetermined. Node 01's healthy values,
      Node 02's watch values, the four alerts and their states, the palette
      hexes, the footer and the prototype-banner text all live only in that
      document. Inventing them is not "ship the parts that work" -- it is a
      fabricated reply.

  BLOCKER 2 — the app path and the CI gate named in the task point at a
  different app than the one the task describes.
    - Task: "the wildfire companion app exists at android-hermes-test/ ...
      (existing dawn-in-the-pines reskin, production-signed release already on
      Stephen's phone)".
    - What android-hermes-test/ actually is on main: the task-0031 hello-world
      scaffold, 17 files, namespace/applicationId com.nordtronics.hermestest,
      MainActivity.java = 23 lines, no nodes/alerts/detail screens, no nt_*
      colours. Nothing reskinned, nothing signed.
    - The app with the dawn-in-the-pines reskin IS android/companion-v0/
      (namespace com.nordtronics.companion; NodesActivity, NodeDetailActivity,
      AlertsActivity, SystemActivity; the nt_* dawn-pine token table from 0084).
      main carries 0 files under android/ -- it lives on branches only, base
      hermes/0069-app-api-dns, reskin tip hermes/0084-app-reskin-dawn-pine @
      3c368eeb1a4a9fa6c4747c846c3240226977c5b0 (the SHA 0084 staged and Juno
      verified).
    - The CI gate the task names is also the wrong one: .github/workflows/
      android-build.yml builds android-hermes-test (defaults.run.working-
      directory: android-hermes-test) and triggers on
      `push.branches: [android-toolchain-setup]` only, so a hermes/0090-* push
      starts NO run at all and criterion 4 is unattainable without adding the
      branch to that list in the same commit. The companion app's workflow is
      .github/workflows/android-companion-v0.yml
      (paths: android/companion-v0/**, plus a hermes/* branch allow-list).

  DECISION OWED (both are Juno/Stephen calls, not mine):
    (a) Put app-ui-spec-v2-2026-10-02 and app-api-contract-v1-2026-10-02
        somewhere reachable -- commit them to the repo the way the 0091 input
        was (docs/wildfire/), or inline their exact values in the task -- and
        re-issue. Without the UI spec there is no "exactly" to render.
    (b) Say which tree to build in: android/companion-v0 on a branch cut from
        hermes/0084-app-reskin-dawn-pine (the app that has the dawn reskin and
        the signed release, and whose whole point is "keep the existing app
        structure/navigation/theme/signing untouched"), or a new app under
        android-hermes-test (which is currently the 0031 hello-world and shares
        nothing with the described app). Name the workflow that is the gate.

  DEVIATIONS ALREADY MADE: none. No implementation step was started, no
  alternative spec was substituted, no path was guessed. Only the mailbox
  transition commit was made.
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


---

# Reply — BLOCKED, no proof block

There is no branch, no CI run and no APK. This task stopped at step 1 because its
own inputs are missing and its context is factually wrong about the repo, and
both need a decision from Juno/Stephen before any UI can be built.

- The two documents the task names as "the spec" (`app-ui-spec-v2-2026-10-02.md`,
  `app-api-contract-v1-2026-10-02.md`) exist only in Juno's local workspace.
  They are on no branch, in no commit, and nowhere on this host or the VPS, and
  they are not on GitHub. Every command used to establish that is quoted in the
  front-matter `notes`.
- `android-hermes-test/` is not the companion app. On main it is the 0031
  hello-world scaffold (`com.nordtronics.hermestest`, 23-line MainActivity, no
  nodes/alerts screens, no dawn palette). The app the task describes -- dawn
  reskin, production-signed release, three screens -- is `android/companion-v0/`
  (`com.nordtronics.companion`), which main does not carry at all; it lives on
  branches (base `hermes/0069-app-api-dns`, reskin tip
  `hermes/0084-app-reskin-dawn-pine`).
- `android-build.yml`, the CI the task names, builds `android-hermes-test` and
  triggers only on the `android-toolchain-setup` branch, so it would not even run
  on a `hermes/0090-*` push. The companion workflow is
  `android-companion-v0.yml`.

What is needed to unblock: (a) the two spec documents in a reachable place (or
their values inlined), and (b) a one-line decision on which app tree is the
target and which workflow is the gate. Both are recorded in the front-matter
`notes`; no implementation step was started and no path was guessed.

This file stays in `mailbox/active/` with `status: in_progress`: `staged` asserts
success and the protocol requires `proof` pointers, so staging a blocked task
would be a fabricated reply.
