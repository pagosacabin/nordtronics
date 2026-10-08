---
task_id: "0130"
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0130 — Companion app release APK with API key

## Context

- api.nordtronics.io had zero authentication (anyone could curl the sensor feed). Juno has closed it: the backend now requires an `X-API-Key` header on all `/v1/*` routes (commit 9dfc091, tests pass, fail-closed).
- The app side is done too (commit 9c3e268): `android/companion-v0/app/build.gradle` reads a `NORTRONICS_API_KEY` env var into `BuildConfig.API_KEY` at build time, and `ApiClient.request()` sends it as the `X-API-Key` header. Empty key = header omitted (debug builds, local mock — unchanged behavior).
- The backend is NOT deployed yet. It flips the moment this APK is ready, so the old app will break against the new backend — the rollout is: you deliver the APK, Juno deploys the backend, Stephen installs immediately.
- The API key itself is NOT in this task and NOT in git. Read it from `/home/astroboy/.hermes/api.key` on your machine (Stephen placed it there). Never commit it, never paste it into a staged reply or log.

## Task

Build the release APK for `android/companion-v0` from origin/main with the API key baked in:

1. Pull latest origin/main (must include commits 9dfc091 and 9c3e268).
2. Read the API key from `/home/astroboy/.hermes/api.key`; export it as `NORTRONICS_API_KEY` in your build environment only. Do not copy it anywhere else.
3. Build the release APK (`./gradlew assembleRelease` in `android/companion-v0`).
4. Sanity-check the APK: confirm `BuildConfig.API_KEY` is non-empty in the built output (e.g. decompile check or a build-log echo of its length — never the value).
5. Deliver the APK the usual way you deliver release builds to Stephen.

## Success criteria

- Release APK builds clean from origin/main with no code changes.
- The built APK carries a non-empty API key (verified without exposing the value).
- APK is delivered to Stephen and ready to install.
- Staged reply states the APK location, the verification performed, and explicitly that the key was not written to disk outside the build env or included in any reply.

## Constraints

- Do NOT deploy anything to the VPS. Juno handles the backend flip.
- Do NOT commit the API key anywhere. Do NOT include it in the staged reply, logs, or any file.
- Cost: use your cheapest model tier for this task. Do not use premium/pro models.

## Proof

Staged reply with the APK delivery location and the key-presence verification (length only, never the value). No branch needed — this builds from main.

## Reply format

Stage with front-matter. Body: APK location, build result, key verification (non-empty, length), explicit "key not persisted or disclosed."
