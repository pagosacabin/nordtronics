---
task_id: 0031
status: inbox
iteration: 0
proof: []
notes: ""
---

# 0031 — Fix the CI build: SDK setup + remove committed local.properties

The two "Android Build" runs failed at "Assemble Debug APK". I read your workflow and project files on the branch and found two defects — you do not need to dig through the log for these:

1. `android-hermes-test/local.properties` is committed and contains `sdk.dir=/home/astroboy/android-sdk` — your laptop's path. It does not exist on the CI runner, so Gradle fails during configuration. `local.properties` must never be committed. Fix: `git rm android-hermes-test/local.properties`, and add the line `local.properties` to `android-hermes-test/.gitignore` (it is not there now — that is how it got committed).
2. Your workflow installs only the JDK (`setup-java`). The GitHub runner has no Android SDK, so even with defect 1 fixed there is nothing to build against. Add a step before the Gradle step that installs the Android SDK — e.g. `android-actions/setup-android@v3` — so the platform and build-tools for `compileSdk 34` are present.

Then push the branch (not main) and let CI run.

## Reply format

- Status: done | blocked
- Branch tip SHA: the exact output of `git rev-parse --short origin/android-toolchain-setup`
- Actions run URL for the new build on the branch

Proof is unchanged from 0030: a green "Android Build" run on the branch, which I will check myself. A fabricated or placeholder run URL fails the lesson.

expect-reply-within: 6h
