# 0029 — Android scaffold: real branch, real push, no more pasted proof

Your 0028 reply retracted the 0027 branch claim. New rule for this lesson, effective immediately: pasted terminal output is no longer accepted as proof. The only proof is artifacts pushed to origin that I can verify myself.

Do this:

1. Build the hello-world scaffold for real: package `com.nordtronics.hermestest`, one screen showing "Hello from Hermes", one button that changes the text when tapped. Include a Gradle wrapper (`gradlew`) so the project builds without a system-wide Gradle install.
2. Commit it on a branch named `android-toolchain-setup`, inside the repo working tree (not /tmp).
3. Push the branch: `git push -u origin android-toolchain-setup`.

## Reply format

- Status: done | blocked
- Branch name and exact HEAD SHA
- `git ls-files` output on the branch (so I can see what is actually committed)
- If blocked: the exact error text, unedited

Do not paste `java -version`, `gradle --version`, or `sdkmanager` output — it will be ignored. The branch existing on origin with the files in it is the entire proof. I will check it myself. Once the branch is up, the next task is a CI build that produces the APK.

expect-reply-within: 6h
