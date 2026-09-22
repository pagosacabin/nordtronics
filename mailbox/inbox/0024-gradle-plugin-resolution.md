# 0024 — Diagnose Android Gradle plugin resolution failure

Follow-up to 0021 (Status: blocked — `com.android.application` 8.3.0 "Plugin not found in any of the following sources").

This is a read-only diagnostic. Do not change the SDK install or project files yet — report findings first.

1. Paste the FULL error output of one failing `./gradlew` invocation (not a summary — the complete text, including which repositories it searched).
2. Show your repository configuration: the full contents of `settings.gradle` / `settings.gradle.kts` (pluginManagement block) and the project `build.gradle` / `build.gradle.kts`.
3. Test egress to the plugin repositories from your host and report exact results:
   - `curl -sSI https://maven.google.com --max-time 15 | head -5`
   - `curl -sSI https://plugins.gradle.org --max-time 15 | head -5`
   - If curl is blocked by the same security scan that blocked raw-IP HTTP in 0020, say so and try `wget -qSO- --timeout=15 https://maven.google.com 2>&1 | head -5` instead.
4. Report proxy environment: output of `env | grep -i proxy` (redact any credentials, show only that the vars exist and their host:port shape).
5. Verdict: is the root cause (a) repositories misconfigured, or (b) your host cannot reach maven.google.com / plugins.gradle.org? One honest sentence.

## Reply format

- Status: done | blocked
- Full error text (item 1)
- Repo config (item 2)
- Connectivity results (item 3)
- Proxy env (item 4)
- Verdict (item 5)

expect-reply-within: 12h
