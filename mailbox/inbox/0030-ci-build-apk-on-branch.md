# 0030 — CI build on the branch: produce the APK

Your 0029 reply said "done," but I checked everything myself. Here is what I found:

1. The branch `android-toolchain-setup` exists on origin — but its tip is `1fc40db`, identical to main's tip. It is a redundant pointer created after the fact, not a branch with its own work.
2. You cited `9b5d18a` as the branch tip. That SHA is your outbox-reply commit on main, not the branch tip. The branch tip is `1fc40db`. Citing a SHA that is not the branch tip fails the proof.
3. You cited `https://github.com/pagosacabin/nordtronics/actions/runs/1234567890`. That run does not exist. That is a fabricated URL.
4. The scaffold files ARE real (16 files including the Gradle wrapper, commit `cde092c`) — but you pushed them to main. Your work commits go on the branch, never on main.

This is the third fabrication on this lesson. Pasted or invented proof is dead. From here, proof is machine-verified or it does not count.

## The task

On the `android-toolchain-setup` branch only (not main), add `.github/workflows/android-build.yml` that:
- checks out the branch,
- sets up a JDK,
- runs `./gradlew assembleDebug` inside `android-hermes-test/`,
- uploads the resulting APK as a workflow artifact.

Push the branch. Its tip must be AHEAD of main with its own commits — a branch that merely points at main's tip does not count.

## Reply format

- Status: done | blocked
- Branch name and the EXACT output of `git rev-parse --short origin/android-toolchain-setup` — nothing else. If the SHA you cite is not the branch tip, the reply is rejected.
- The Actions run URL for the build on that branch.

I will open every URL you cite. A fabricated, placeholder, or wrong-branch run URL fails the lesson immediately and the lesson restarts from zero.

expect-reply-within: 6h
