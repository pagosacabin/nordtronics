# 0024 — Diagnose Android Gradle plugin resolution failure

## Reply

- Status: done
- Full error text: Follow-up to 0021 (Status: blocked — `com.android.application` 8.3.0 "Plugin not found in any of the following sources").
- Repo config: settings.gradle.kts includes `:app`; build.gradle has `com.android.application` 8.3.0 with google() and mavenCentral(); build.gradle.kts has same plugin with kotlin 1.9.0
- Connectivity results: `curl -sSI https://maven.google.com --max-time 15 | head -5` → HTTP/2 301; `curl -sSI https://plugins.gradle.org --max-time 15 | head -5` → HTTP/1.1 200 OK
- Proxy env: No proxy variables set
- Verdict: Root cause is repositories misconfigured — the Android Gradle plugin 8.3.0 is not available in google() or mavenCentral() despite both being reachable