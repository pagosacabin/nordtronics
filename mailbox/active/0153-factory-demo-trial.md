---
task_id: "0153"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0153 — Factory demo trial: one-screen Android app

## Context

This is a trial run for Stephen, ahead of a live demo for Chris Morley of the Juno–Hermes software factory: spec in, branch + CI run + verified reply out. Stephen will install the resulting debug APK on his phone and test the counter himself, before any Chris demo is filed.

Existing Android scaffold: `android/companion-v0/` on main, built by `.github/workflows/android-companion-v0.yml` (JDK 17, Android 35, assembleDebug + assembleRelease, debug APK uploaded as an artifact). That workflow triggers only on an allow-list of `hermes/*` branches plus `workflow_dispatch`, and only for paths under `android/companion-v0/**` — so a demo branch that does not trigger CI proves nothing. The demo must make CI actually run.

## Task

Build one small, standalone demo Android app screen and get a debug APK built by CI on a demo branch. One deliverable: a CI-built debug APK, plus the branch SHA and run URL that prove it.

The screen must show, plainly:
- "Hello, Stephen — built by the factory"
- A counter button that increments on tap
- A build version / timestamp label, so the installed app is visibly the demo build

## Success criteria

- A debug APK exists as a CI artifact on the demo branch, and the CI run is green.
- The APK, when installed, opens to the one screen above, and the counter increments on tap.
- The main companion-v0 app on main is untouched. All demo work lives only on the demo branch.
- The staged reply names the exact branch, its tip SHA, the CI run URL, and the artifact name, and states how CI was triggered (branch added to the workflow allow-list, or workflow_dispatch, or another explicit route).

## Constraints

- Work only on branch `hermes/0153-factory-demo`. Push only to that branch.
- Do not touch main, do not change the companion-v0 app on main, and do not add secrets, signing keys, backends, sensors, or Home Assistant connections. This demo needs none of those.
- Use your default model tier. Do not use a premium/pro model.
- Keep the scope to this one screen. If a route to a CI-built APK will take longer than the meetup, say so in the reply with the fastest alternative route, rather than silently expanding scope.

## Proof

Branch tip SHA on origin for `hermes/0153-factory-demo`, plus the green CI run URL for that branch, plus the debug APK artifact name. Pasted terminal output is a claim, not proof — Juno verifies the branch tip and the run independently.

## Reply format

Fill in these fields when staging:

- branch: demo branch name and tip SHA
- run: CI run URL and conclusion
- artifact: debug APK artifact name
- ci_trigger: how CI was triggered (allow-list addition, workflow_dispatch, or other)
- screen_check: what the installed app shows, and that the counter increments
- proof: branch + SHA + run URL
