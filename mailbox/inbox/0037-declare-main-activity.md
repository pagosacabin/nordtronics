---
task_id: "0037"
status: inbox
iteration: 0
proof: []
notes: ""
---

# 0037 — Declare MainActivity in the manifest (app has no launcher icon)

## Context

Your 0031 build is green and the APK installs — Stephen installed it on his phone and Android said "HermesTest installed." But the app never appears in the launcher, because `android-hermes-test/app/src/main/AndroidManifest.xml` on branch `android-toolchain-setup` has an EMPTY `<application>` block. `MainActivity.java` exists at `android-hermes-test/app/src/main/java/com/nordtronics/hermestest/MainActivity.java` but is not declared anywhere, so Android has no launchable entry point.

## Task

On branch `android-toolchain-setup`, edit `android-hermes-test/app/src/main/AndroidManifest.xml`: inside the `<application>` block, declare the activity with a launcher intent filter:

```xml
<activity
    android:name=".MainActivity"
    android:exported="true">
    <intent-filter>
        <action android:name="android.intent.action.MAIN" />
        <category android:name="android.intent.category.LAUNCHER" />
    </intent-filter>
</activity>
```

Commit and push the branch (not main). Let CI run.

## Success criteria

1. The manifest on the branch contains the `<activity>` declaration above.
2. A new green "Android Build" run on `android-toolchain-setup` with a fresh `android-apk` artifact.

## Constraints

- Branch `android-toolchain-setup` only. Do not touch `main`.
- Do not paste log output as proof — the proof is the pointers below.

## Proof

Stage this file to `mailbox/staged/` with `status: staged` and fill in `proof` with:
- branch: `android-toolchain-setup`, sha: the new tip SHA
- run: the Actions run URL for the new green build

## Reply format

The staged file IS the reply. `notes` must say what you changed (one or two sentences). An empty `notes` field is not a reply.

expect-reply-within: 6h
