---
task_id: "0130"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - revision: main @ 8d69cea62b3a1d392ac4d2eca7538accaf7e990b
    note: "== origin/main; the worktree HEAD the release APK was built from. Confirmed
      to contain both 9dfc091 (backend X-API-Key gate, fail-closed) and 9c3e268
      (app sends X-API-Key from BuildConfig.API_KEY)."
  - branch: none
    note: "the spec says 'No branch needed -- this builds from main'; no branch was
      created and nothing was pushed except the mailbox state transitions on main."
  - run: none
    note: "no CI run exists for this task and none can: .github/workflows/android-companion-v0.yml
      triggers only on push to an explicit branch allow-list (hermes/0045-*, 0046-*,
      0047-*, 0048-*, 0049-*, 0069-*, 0084-*, 0090-*, 0095-*) and on
      workflow_dispatch, with a path filter on android/companion-v0/**; no branch
      in that list was pushed and no file under that path changed. The only runs on
      main for these commits are 'Website Check' (37838238468, 37838252741), which
      do not build the APK. The build was local, on this host."
  - artifact: "/home/astroboy/nordtronics-release/0130/nordtronics-companion-v0-0130-release-signed.apk
      -- 2,834,828 B, sha256 0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1,
      cert SHA-256 e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd"
  - unsigned_input: "/home/astroboy/nordtronics-release/0130/app-release-unsigned-0130.apk
      -- 2,782,917 B, sha256 33a965c90a7e3356f4a98d9cae59657de3293ff99380ba201989cc52b41ebab7"
  - key_check: "BuildConfig.java API_KEY literal length 43 (byte-equal to the env key)
      and the same 43-byte string present in classes.dex, both in the unsigned APK
      and in the signed APK -- value withheld"
  - evidence: "/home/astroboy/nordtronics-release/0130/signing-and-perms.txt"
  - files: []
    reason: "no repository file changed -- this task builds an APK from main with no
      code change; the only commits this run made to main are the mailbox state
      transitions (pickup, then this staging)."
  - ntfy: nordtronics-build-ed05a663 @ 2026-10-08T20:31:06Z (id t1N5uFGJeOhD)
notes: |
  STAGED (iteration 1), 2026-10-08 20:15-20:31 UTC. Off-peak (PEAK: OFF-PEAK 20:15
  UTC). PROTOCOL: MATCH protocol_version=1.0.0. Model tier: DeepSeek Flash
  (deepseek-flash / provider deepseek) -- the cron worker's cheapest tier, as the
  Constraints section requires.

  DELIVERABLE. A release APK built from main with the API key baked in, signed with
  the production identity, at
  /home/astroboy/nordtronics-release/0130/nordtronics-companion-v0-0130-release-signed.apk
  (2,834,828 B, sha256 0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1,
  cert SHA-256 e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd --
  the same production identity 0071 minted and 0086 restored). That directory is the
  established delivery location (/home/astroboy/nordtronics-release/<NNNN>/, cf.
  0071, 0086). Build was `./gradlew --no-daemon --offline assembleRelease` with
  JDK 17, ANDROID_HOME=/home/astroboy/android-sdk and NORTRONICS_API_KEY set in the
  build environment only; BUILD SUCCESSFUL, zero source changes.

  KEY-PRESENCE VERIFICATION (criterion 2), value never printed: the generated
  app/build/generated/source/buildConfig/release/.../BuildConfig.java holds an
  API_KEY literal of length 43 that is byte-equal to the env key, and the same
  43-byte string appears once in classes.dex inside BOTH the unsigned APK and the
  signed APK (checked by scanning the zip's .dex entries). Length and a truncated
  sha256 of the key are quoted; the value itself is withheld everywhere.

  KEY HYGIENE (criterion 4). The key was read from /home/astroboy/.hermes/api.key
  into a shell variable, exported to the Gradle process, and written to no other
  file by me. It is NOT in the delivery directory, NOT in the reply, NOT in any
  log. Declared with the same honesty as 0071/0086: the one non-obvious place the
  value lands is Gradle's own project-local build state
  (android/companion-v0/.gradle/8.14.3/executionHistory/executionHistory.bin, a
  byte-identical match) and the build intermediates -- both inside the gitignored
  .gradle/ and build/ trees, i.e. inside the build environment, not committed and
  not delivered. A filesystem sweep of the repo and ~/.config for the key value
  found no other copy. Observation, not a change: /home/astroboy/.hermes/api.key
  is mode 0644; left as Stephen placed it.

  WHAT WAS NOT DONE -- on-device install. This run did NOT install the APK. No
  device was attached (`adb devices` empty), and the companion33 AVD aborted
  during boot under host memory pressure: `-m 2048` requested against 2,019 MB
  available with 4,684/7,252 MB of swap already in use (it reached the guest
  fingerprint HAL, then exited). So "ready to install" rests on static checks --
  apksigner verify (Verifies; v2 true, v3 true; v1 false is correct at minSdk 24),
  zipalign -c -P 16 4 -> successful, and aapt2 badging (com.nordtronics.companion,
  versionCode 1, versionName 0.1, minSdk 24, targetSdk 35) -- not on an install.
  Install it on a device or retry the AVD on a less loaded host if that matters to
  the verifier.

  INSTALL NOTE FOR STEPHEN: the phone currently runs the debug build; a
  production-signed APK is a new app identity, so it needs one uninstall of
  com.nordtronics.companion first (INSTALL_FAILED_UPDATE_INCOMPATIBLE otherwise).
  Same situation 0086 recorded.

  ROLLOUT CONTRACT HONOURED. Nothing was deployed to the VPS; the backend flip is
  Juno's. ntfy build notification published to the companion build topic
  nordtronics-build-ed05a663 (the topic this worker uses for companion builds; the
  spec names no topic) so Stephen hears the artifact exists -- id t1N5uFGJeOhD @
  2026-10-08T20:31:06Z, carrying the revision, the APK path, its sha256 and Status:
  success. It deliberately has no Workflow/run URL field because no CI run covers
  this build (enumerated in proof.run).

  DEVIATIONS, all deliberate: (1) the pickup commit's content edit (status /
  iteration) was left unstaged by an aborted `git add` that named the file's old
  path -- a first rename-only commit went out (4d23a56) and a follow-up (8d69cea)
  recorded the front-matter, exactly the failure mode the mailbox rules warn
  about; both are on main and the tree is clean. (2) No branch and no CI run, as
  the spec directs; the build is local. (3) ntfy published without a Workflow field
  (no run exists). (4) The key value exists inside Gradle's gitignored build state
  (declared above).
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

---

# Reply — Hermes → Juno — **STAGED**

**Model note:** the cron worker on `deepseek-flash` (`deepseek`), the cheapest tier, per Constraints.

## 1. Build

Source: `main` @ `8d69cea62b3a1d392ac4d2eca7538accaf7e990b` (`origin/main`, the worktree
HEAD at build time). It contains `9dfc091` (backend gate) and `9c3e268` (app sends
the header), both confirmed as ancestors of `origin/main`.

```
$ export JAVA_HOME=/home/astroboy/jdks/jdk-17.0.20.1+1
$ export ANDROID_HOME=/home/astroboy/android-sdk
$ export NORTRONICS_API_KEY="$(tr -d '\n' < /home/astroboy/.hermes/api.key)"   # 43 chars
$ ./gradlew --no-daemon --offline assembleRelease
BUILD SUCCESSFUL in 21s
42 actionable tasks: 17 executed, 25 up-to-date
```

No source file was modified. `git status` on the tree shows only pre-existing
untracked files unrelated to this task.

## 2. Key-presence verification (value withheld)

| Check | Result |
|---|---|
| Generated `BuildConfig.java` `API_KEY` literal | length **43**, byte-equal to the env key: **True** |
| `classes.dex` in `app-release-unsigned.apk` | key present, **43 bytes**, 1 occurrence |
| `classes.dex` in the **signed** APK | key present, **43 bytes**, 1 occurrence |
| Key sha256 (first 12 hex, value never printed) | `a4e1fb370c74` |

## 3. The deliverable

```
/home/astroboy/nordtronics-release/0130/
  nordtronics-companion-v0-0130-release-signed.apk   2,834,828 B
    sha256    0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1
    cert       e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
  app-release-unsigned-0130.apk                      2,782,917 B
    sha256    33a965c90a7e3356f4a98d9cae59657de3293ff99380ba201989cc52b41ebab7
  app-release-aligned-0130.apk         (zipalign output; same bytes as the unsigned)
  signing-and-perms.txt                (digests + verification, no secrets)
```

```
$ apksigner verify --print-certs nordtronics-companion-v0-0130-release-signed.apk
Signer #1 certificate DN: CN=Nordtronics Companion, OU=Android, O=Nordtronics
Signer #1 certificate SHA-256 digest: e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
Verifies
Verified using v2 scheme (APK Signature Scheme v2): true
Verified using v3 scheme (APK Signature Scheme v3): true
$ zipalign -c -P 16 4  ->  Verification succesful
$ aapt2 dump badging   ->  com.nordtronics.companion, versionCode 1, versionName 0.1, minSdk 24, targetSdk 35
```

`v1 scheme: false` is correct at minSdk 24 (v2 covers API 24+), not a defect.

## 4. Key not persisted or disclosed

The key was read into a shell variable, exported to the Gradle process, and written
to no other file by me. It is **not** in the delivery directory, **not** in this
reply, **not** in any log, and **not** committed. The keystore and its password file
were used read-only and are not copied into the delivery directory.

One declared caveat, in the same spirit as 0071/0086: Gradle records its own task
inputs locally, so the key value also exists in
`android/companion-v0/.gradle/8.14.3/executionHistory/executionHistory.bin` and in
the build intermediates — both inside the gitignored `.gradle/` / `build/` trees,
i.e. *inside the build environment*, not in the repo history and not delivered. A
filesystem sweep of the repository and `~/.config` found no copy outside those.

## 5. Delivery + notification

Delivered at the established location `/home/astroboy/nordtronics-release/0130/`
(cf. 0071, 0086). ntfy build notification published to the companion build topic
`nordtronics-build-ed05a663` (`https://ntfy.sh`), HTTP 200, id `t1N5uFGJeOhD` @
`2026-10-08T20:31:06Z`, carrying the revision, the APK path, its sha256 and
`Status: success`.

## 6. Not performed: on-device install

No device was attached (`adb devices` empty) and the `companion33` AVD aborted
during boot under host memory pressure (`-m 2048` against 2,019 MB available, with
4,684/7,252 MB of swap in use). "Ready to install" therefore rests on the static
checks in §3, not on an install. The verifier should read it that way; if an
install-level proof is required, it needs a device or a less loaded host.
