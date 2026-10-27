---
task_id: "0153"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: hermes/0153-factory-demo
  sha: 3cebc1a19ffbecdc1d1c400b0c3b716fd8e95f42
  run: https://github.com/pagosacabin/nordtronics/actions/runs/40558806559
  conclusion: success
  files:
    - .github/workflows/factory-demo.yml
    - android/factory-demo/.gitignore
    - android/factory-demo/build.gradle
    - android/factory-demo/settings.gradle.kts
    - android/factory-demo/gradle.properties
    - android/factory-demo/gradlew
    - android/factory-demo/gradlew.bat
    - android/factory-demo/gradle/wrapper/gradle-wrapper.jar
    - android/factory-demo/gradle/wrapper/gradle-wrapper.properties
    - android/factory-demo/app/build.gradle
    - android/factory-demo/app/src/main/AndroidManifest.xml
    - android/factory-demo/app/src/main/java/com/nordtronics/factorydemo/MainActivity.java
    - android/factory-demo/screenshots/emulator-counter-0.png
    - android/factory-demo/screenshots/emulator-counter-2.png
  artifact: factory-demo-debug-apk
  ci_trigger: push to hermes/0153-factory-demo matching .github/workflows/factory-demo.yml's own branch allow-list and path filter
notes: |
  DELIVERED. A standalone one-screen demo app under android/factory-demo/ (new
  Gradle root + single `app` module, AGP 8.3.0 / compileSdk 35 / JDK 17,
  com.nordtronics.factorydemo), built by a NEW workflow
  .github/workflows/factory-demo.yml, with a green CI run at the branch tip and
  the debug APK verified installed on a booted emulator.

  CI ROUTE (criterion 4 asks how CI was triggered). The existing
  android-companion-v0.yml was NOT touched: its branch allow-list does not
  contain hermes/0153-factory-demo and its path filter only watches
  android/companion-v0/**. Rather than add a branch to a workflow that builds a
  different tree, this branch carries its own workflow whose `on.push.branches`
  is [hermes/0153-factory-demo] and whose paths include the workflow file
  itself, so the first push of the branch started run 40558428043. No
  workflow_dispatch was used; both runs listed below are push-triggered.

  RUNS. Two, both green, both verified headSha == the tip at the time:
    - 40558428043 @ 3bd3484 — first build (app only), green, artifact
      factory-demo-debug-apk (8593-byte APK, sha256 b8cccb89…4064). This APK was
      the one installed and exercised first.
    - 40558806559 @ 3cebc1a — the tip cited in `proof`. Its only change over
      3bd3484 is the two committed screenshots, and its own APK re-installed and
      re-verified below, so the cited run's artifact is the one whose screen was
      last read. Job steps checked individually via the jobs API: "Assemble debug
      APK" = success, "Upload debug APK" = success, artifact
      factory-demo-debug-apk present (5408 B zip). The green tick alone was not
      taken as evidence.

  SCREEN CHECK — done on a real emulator, from the CI artifact, not from a local
  build and not from a screenshot alone. AVD companion33 (android-33, x86_64),
  headless, -gpu host. `adb install` of the downloaded artifact, cold start of
  com.nordtronics.factorydemo/.MainActivity, then uiautomator dump:
    run-1 APK: 'Hello, Stephen — built by the factory' | 'Juno–Hermes software
      factory · demo 0153' | '0' | 'taps' | 'Tap to count' | 'Build: 2026-10-27
      18:17 UTC · 3bd3484'
  Two taps on the button's own bounds center (540,1057) gave '1' then '2' in
  successive dumps — the counter increments. Screenshots committed at
  android/factory-demo/screenshots/emulator-counter-0.png and
  ...-2.png are those two states (1080x2340 PNG, real frames).
    run-2 APK (the cited run): reinstalled after an uninstall — the two runs'
    debug keystores differ, so an in-place update legitimately fails
    INSTALL_FAILED_UPDATE_INCOMPATIBLE — and it dumped the same greeting and
    'Build: 2026-10-27 18:20 UTC · 3cebc1a' with the counter going '0' -> '1' on
    one tap. Note the committed screenshots carry run-1's stamp (18:17 · 3bd3484)
    because they were captured before the screenshot commit existed; the code is
    identical between the two runs, only the stamp differs. The verification was
    done, not assumed.

  SCOPE / MAIN UNTOUCHED. `git diff --stat main...HEAD` lists exactly the 14
  files in `proof.files` (587 insertions, plus the two PNGs) and
  `git diff --stat main...HEAD -- android/companion-v0
  .github/workflows/android-companion-v0.yml` is empty — the companion-v0 app and
  its workflow are byte-identical to main. All demo work lives only on
  hermes/0153-factory-demo; nothing was merged to main.

  DESIGN NOTES. No dependencies at all (platform widgets only, platform
  Theme.Material.Light.NoActionBar), so no resource-merging or AndroidX/Kotlin
  dex-dedup class of failure. No secrets, no signing config, no backend, no
  sensors: the debug APK is signed by the runner's generated debug keystore, which
  is why two runs are not update-compatible. Build stamp is a Configure-time UTC
  timestamp plus GITHUB_SHA's first 7 chars, so an installed APK visibly names the
  build that produced it.

  ntfy: NOT APPLICABLE and deliberately omitted — the spec names no topic, and the
  nordtronics-build-ed05a663 topic is the companion-v0 build receipt; publishing
  this demo there would misattribute a companion build.

  PROCESS. Pickup commit 338c6c2 moved inbox -> active with status in_progress and
  iteration 1. Local prevalidation: assembleDebug with JDK 17 + ~/android-sdk
  before pushing, so the only CI failure mode left was the runner image. Emulator
  was killed (adb emu kill) and local Gradle artifacts removed afterwards; only
  source and screenshots are committed.
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

---

# Reply — STAGED (2026-10-27)

| Field | Value |
|---|---|
| branch | `hermes/0153-factory-demo` |
| SHA (branch tip, on origin) | `3cebc1a19ffbecdc1d1c400b0c3b716fd8e95f42` |
| run | https://github.com/pagosacabin/nordtronics/actions/runs/40558806559 — **success** (headSha = the tip above) |
| artifact | `factory-demo-debug-apk` (contains `app-debug.apk`) |
| ci_trigger | push to `hermes/0153-factory-demo`, matched by the branch allow-list **and** path filter of the new `.github/workflows/factory-demo.yml` (the workflow file is itself a watched path, so the first push started the run). No `workflow_dispatch`. |
| screen_check | Installed the downloaded CI artifact on AVD `companion33` and dumped the live view tree: `Hello, Stephen — built by the factory`, `Juno–Hermes software factory · demo 0153`, counter `0`, `taps`, button `Tap to count`, `Build: 2026-10-27 18:20 UTC · 3cebc1a`. Taps on the button's bounds centre increment the counter — read `1`, then `2` in successive dumps. Screenshots committed under `android/factory-demo/screenshots/`. |

**What was built.** `android/factory-demo/` — a standalone Gradle root with one `app` module (`com.nordtronics.factorydemo`), AGP 8.3.0, compileSdk 35, JDK 17, **zero dependencies** (platform widgets only), one Activity building the screen programmatically, and a `BuildConfig` stamp of UTC build time + short `GITHUB_SHA`. A new workflow `.github/workflows/factory-demo.yml` runs `assembleDebug` and uploads `factory-demo-debug-apk`; it carries its own branch allow-list and path filter because the companion workflow's allow-list does not include this branch and its path filter does not watch this tree.

**Two green runs, honestly labelled.** 40558428043 @ `3bd3484` built the app alone; its APK was installed first and is the APK in the committed screenshots (`Build: … 18:17 UTC · 3bd3484`). 40558806559 @ `3cebc1a` (the tip cited above) adds only the screenshots; its own APK was downloaded, installed (after uninstall — the two runs' debug keystores differ, so `INSTALL_FAILED_UPDATE_INCOMPATIBLE` is expected) and re-verified: same screen, stamp `18:20 UTC · 3cebc1a`, counter `0` → `1` on a tap. Job steps were checked individually through the jobs API (`Assemble debug APK`, `Upload debug APK` = success), not inferred from the run's green tick.

**Main untouched.** `git diff --stat main...HEAD` shows exactly the 14 files listed in the front-matter and nothing else; the same diff scoped to `android/companion-v0` and `android-companion-v0.yml` is empty. Nothing was merged to main, and this branch is the only place demo work lives.

**For Stephen.** The APK to install is `app-debug.apk` from the `factory-demo-debug-apk` artifact of run 40558806559 (or the run-1 artifact, which differs only in the stamp). Because debug APKs are signed with the runner's throwaway debug keystore, installing a *newer* run over an older one requires uninstalling the old one first.
