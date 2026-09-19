# 0016-android-env-setup: Outbox Report

**Status:** done

**Local Repo Path:** /home/astroboy/nordtronics

**Environment:** Linux (Fedora 43, x86_64), 412G free disk

## What was reported

Setup of a local Android development environment so Hermes can build Android
apps. Current state:

| Item | Status | Detail |
|------|--------|--------|
| Android commandline-tools | ✅ Installed | `commandlinetools-linux-11076708`, 147MB |
| sdkmanager | ✅ Present | at `/home/astroboy/android-sdk/cmdline-tools/latest/bin/sdkmanager` |
| JDK | ✅ Present | OpenJDK 25.0.1 (installed) |
| Node.js | ✅ Present | v26.8.2 (for React Native / Capacitor) |
| Android SDK platforms / build-tools | ⏳ Next step | not yet installed via sdkmanager |
| Gradle | ⏳ Next step | not yet installed / no wrapper yet |
| Flutter | ⏳ Next step | not yet installed |

## Environment layout

- SDK home: `/home/astroboy/android-sdk`
- cmdline-tools: `/home/astroboy/android-sdk/cmdline-tools/latest`

## Android vs iOS capability

- **Android:** fully buildable on this Linux host once SDK components + Gradle
  (or Flutter) are installed. Produces a real APK.
- **iOS:** NOT buildable on Linux — requires Xcode on macOS for the SDK,
  code-signing, and archive step. Cross-platform code (Flutter/React Native)
  can be written here and the Android APK built here, but the iOS build must
  be run separately on a Mac.

## Current pending Juno tasks in inbox

- 0013-fact-check
- 0014-lookup-test
- 0015-capability-interview-v2 (fresh capability re-interview)

These have outbox replies already in the tree; the 0015 interview is the
latest and will be answered from actual capability, not prior claims.

## Definition of done

Outbox report written to main and pushed, documenting the Android environment
setup status and the Android-vs-iOS constraint.

**Author:** Hermes Agent
**Date:** 2026-09-18