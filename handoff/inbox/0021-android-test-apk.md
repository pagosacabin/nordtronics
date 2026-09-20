# 0021 — Finish the Android toolchain and ship a test APK

The environment work from 0016 was foundation only. Now prove the toolchain end-to-end: build a real APK and put it where Stephen can download and install it on his phone.

## Step 1 — complete the toolchain

Using `sdkmanager` at `/home/astroboy/android-sdk`:
1. Install a SDK platform (e.g., `platforms;android-34` — pick a current stable API level and say which).
2. Install matching `build-tools` and `platform-tools`.
3. Install Gradle (system package or wrapper — your choice, say which).
4. Run `sdkmanager --list_installed` and report exactly what is now present.

## Step 2 — build a minimal sample app

Create a tiny native Android app (no Flutter needed for this test): one screen, "Hello from Hermes" text, one button that changes the text when tapped. Package name something clearly test-scoped, e.g. `com.nordtronics.hermestest`.

Build a **debug APK** (`./gradlew assembleDebug`). Debug builds are auto-signed and installable — that is what we want for this test.

## Step 3 — verify the APK yourself

Before shipping it:
1. `apksigner verify --print-certs` on the APK — report the result.
2. `aapt dump badging` — report package name, version, target SDK.
3. Confirm the file size is sane (a hello-world debug APK should be a few MB, not KB and not 100MB).

## Step 4 — publish it for download

Create a GitHub release on the nordtronics repo (tag like `android-test-001`, title "Hermes Android toolchain test"), attach the APK, and note in the release description that it is an unsigned-test debug build for sideloading.

## Reply (outbox file `0021-*.md`)

- Status: done | blocked
- What you installed (platform API level, build-tools version, Gradle version)
- Build result: success or the exact error
- `apksigner` and `aapt` verification output
- The release URL Stephen can download the APK from
- Anything you were unsure about
