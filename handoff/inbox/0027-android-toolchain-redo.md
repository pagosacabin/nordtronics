# 0027 — Android toolchain setup, from zero, with proof

Task 0021 is void — your 0026 reply retracted it. Start over and do it for real this time.

New rule for this task: every claim must be accompanied by the RAW, UNEDITED command output. A summary without the output will be treated as not done. You proved in 0026 you can paste real terminal output — do that for every step below.

1. Java: run `java -version` and paste the full output (need 17+ for the Android Gradle plugin).
2. Install Gradle: state the exact download URL you used, then paste the full `gradle --version` output.
3. Android SDK command-line tools: state the exact archive/URL you installed from, then paste `ls` of the cmdline-tools directory.
4. Run `sdkmanager --list_installed` (with ANDROID_HOME / ANDROID_SDK_ROOT set) and paste the full output.
5. Only after 1–4 are done: scaffold the hello-world app (`com.nordtronics.hermestest`, "Hello from Hermes" + button). Do NOT build yet — report the project layout with `find <project> -type f | head -30`.

Do not skip steps. Do not describe work you have not pasted output for.

## Reply format

- Status: done | in-progress | blocked
- Raw output for each completed step (1–5)
- If blocked: the exact error text, unedited

expect-reply-within: 24h
