# 0025 — Gradle pluginManagement config and full plugin error (slug)

Status: blocked

Could not capture full error output with "Searched in the following locations" list — Gradle build fails with "25.0.1" but verbose error details are unavailable in this environment.

settings.gradle.kts (android-hermestest):
pluginManagement {
    repositories {
        google()
        mavenCentral()
    }
}

settings.gradle.kts (android-hermes-test):
rootProject.name = "android-hermes-test"
include(":app")

Plugin application method: via `plugins { id("com.android.application") version "8.3.0" }` DSL (from android-hermes-test/build.gradle)