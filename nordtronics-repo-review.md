# Nordtronics repo review

## Overall

**Nordtronics looks active and reasonably well tested, with a coherent wildfire-monitoring pipeline. But I’d treat the repository as having two important security concerns to resolve: public database backups and a shared API key that may be embedded in the Android app.** I reviewed the repository structure, selected backend and firmware files, deployment documentation, and recent Actions results—not every source file or a live deployment.

## Repo health

**Good signs**
- The repo is active: its metadata showed a recent push, no open issues, and three recently merged PRs.
- CI is more substantive than a basic compile check. The backend workflow runs tests, shell linting, systemd unit checks, and a Mosquitto broker probe; firmware CI runs builds and native tests against portable firmware code. [Backend workflow](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/.github/workflows/backend.yml) · [PlatformIO workflow](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/.github/workflows/platformio.yml)
- Recent results include successful Backend and Website Check runs. The results query returned **20 of 746 Actions runs**; [view Actions](https://github.com/pagosacabin/nordtronics/actions).

**Notable gaps**
- The root README is behind the repo’s current scope: it omits major areas such as `backend/`, `android/`, and `website/`, and ends with test/CI placeholder text. That makes onboarding and understanding the project harder. [Root README](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/README.md#L8-L32)
- Android CI coverage looks uneven. The companion workflow is limited to pushes on an explicit branch list and manual runs; it has no `pull_request` trigger. The separate Android build workflow is pinned to a legacy setup branch. [Companion workflow](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/.github/workflows/android-companion-v0.yml#L3-L9) · [Android build workflow](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/.github/workflows/android-build.yml#L3-L6)
- The firmware workflow’s path filters don’t include every firmware directory listed in the repo, so changes in those other projects may not receive the same automated build coverage. [Firmware path filters](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/.github/workflows/platformio.yml#L5-L19)

## Code and architecture review

The main wildfire data path is clearly separated: field devices publish MQTT/TLS, Mosquitto enforces topic access, a single-threaded ingest worker validates and stores data in SQLite, and a read-only FastAPI service exposes it through nginx. The firmware’s unified node/base-station design and host-side tests are a good fit for a small deployment. [Backend architecture](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/backend/README.md#L1-L31) · [Firmware design](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/firmware/wildfire-node-v1/README.md#L1-L12)

### High priority

1. **Production backup files are committed to a public repository.** The repo is public, and `backups/` contains dated database and Mosquitto dumps. The deployment guide calls committed database dumps the durable restore copy. I haven’t inspected their contents, so I can’t say what data they contain—but treat this as a potential data exposure until audited. [Backup restore documentation](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/backend/DEPLOY.md#L425-L459)

2. **The API authentication story is inconsistent, and a client-bundled shared key is not secret.** The backend README says the API has no authentication, while the deployment guide says `/v1/*` is behind `X-API-Key`. The Android Gradle config reads `NORTRONICS_API_KEY` into `BuildConfig`; any key included in an APK should be considered extractable. Confirm what the running API actually enforces, align the docs, and don’t rely on a shared APK-embedded key as an authorization boundary. [Backend README](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/backend/README.md#L108-L115) · [Deployment guide](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/backend/DEPLOY.md#L545-L548) · [Android API key config](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/android/companion-v0/app/build.gradle#L31-L40)

### Medium priority

3. **Schema evolution needs a real migration path.** The schema file writes version `2` and creates tables idempotently; the deployment guide says column changes require a migration, but none exists yet. That is manageable at this stage, but the next schema change should use explicit, versioned migrations rather than relying on `CREATE TABLE IF NOT EXISTS`. [Schema version](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/backend/common/schema.sql#L10-L16) · [Deployment notes](https://github.com/pagosacabin/nordtronics/blob/cd9627347a80404b4a77d49067e8f8b4b7e25029/backend/DEPLOY.md#L411-L419)

4. **The repository is a broad hardware/software monorepo, but the top-level documentation doesn’t describe its current boundaries.** Backend, firmware, Android, website, CAD/hardware, installs, and operational handoff material coexist. That can work well for a small project, but the README should explain which projects are active, where their tests/builds live, and which hardware or firmware directories are archival.

**Bottom line:** solid foundations for a small deployed IoT system—especially around backend integration tests, broker checks, and firmware host tests. The public backup exposure and unclear API-key boundary deserve attention before treating the repo as safe for ongoing production data.

---

## Suggested markdown file name

`nordtronics-repo-review.md`
