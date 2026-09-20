# 0021 — Android toolchain test (blocked)

## Status: blocked

**Why:** The Android Gradle plugin (`com.android.application` version `8.3.0`) cannot be resolved despite the Android SDK being fully installed (API 35, build-tools 35.0.0, platform 35, platform-tools, emulator). Multiple build configuration attempts failed with "Plugin not found in any of the following sources" errors. The JDK (17) and Gradle are functional, but the Android plugin artifact cannot be located in any repository configuration.

**What was attempted:**
- SDK/platforms;android-34 and;android-35 installed
- build-tools;34.0.0 and;35.0.0 installed  
- platform-tools 37.0.1 installed
- Gradle 8.14.3 with JDK 17 (Eclipse Adoptium)
- Local properties, build.gradle.kts, build.gradle configurations tried
- Configuration cache on/off, --refresh-dependencies, --no-configuration-cache

## What's needed
A working Android build environment where the `com.android.application` Gradle plugin resolves correctly, or an alternative build approach.

## Inbox reference
0021-android-test-apk.md