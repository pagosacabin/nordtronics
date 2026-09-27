---
task_id: "0069"
status: staged
iteration: 1
proof:
  - branch: hermes/0069-app-api-dns
    sha: 2829d8f1c954221c261758491bc826955f535252
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36294156654
notes: |
  Delivered: the app's release build now talks to https://api.nordtronics.io
  over HTTPS with the live /v1 request paths, and the emulator/mock workflow
  still works from the debug build type. Branch tip 2829d8f checked at
  04:23-04:27Z; CI run 36294156654 concluded success with headSha == the tip,
  all 9 steps green including "Assemble release APK (unsigned)". Release APK
  artifact 10923476721, debug 10923342772. ntfy build-green receipt id
  mL9iGiGX1137 (topic nordtronics-build-ed05a663, 2026-09-27T04:26:13Z).

  Model/tier: deepseek-flash (provider deepseek) — the worker's
  deepseek-flash tier, as the spec required.

  Scope extension beyond "DNS cutover only", each with the success criterion
  that required it: (a) three live paths wired, not one, because criterion 2
  asks for the request paths to match the live API contract and criterion 4
  asks about the network security config; (b) a release assemble step added to
  CI because criterion 2's claim is about the *release* build, which
  assembleDebug alone cannot evidence; (c) two new resource files (the main and
  debug network_security_config.xml) because criterion 4 requires HTTPS to work
  and the old manifest flag allowed cleartext to any host; (d) alerts and ping
  gated rather than repointed, because the live API serves neither (both 404)
  and inventing a path or removing UI are both outside this task.

  Correction to the spec's context: the 0049 tip was NOT the newest app source.
  origin/hermes/0050-splash-wordmark-fix @ fb7701d is newer (verified and
  archived) and 0cc5e3d is its ancestor, so 0050 is a strict superset of the
  0049 tip. I based on the 0049 tip anyway because the Constraints fix the base
  explicitly and merging is deferred by the same section — see the reply body,
  which states what the eventual integration must do. Nothing is lost: 0050's
  branch is intact on origin, and its changes are disjoint from 0069's except
  for one line of the workflow's branch list.

  Also declared in the reply: no emulator/device run was performed, so on-device
  rendering and the framework's own enforcement of the network security policy
  are not claimed as verified; what WAS executed is the app's real ApiClient +
  Node + Alert code against both live backends. The mock-server docstring lost
  its literal dev IP (comment-only, no behaviour change) so that a grep for IPs
  returns only the two documented dev-override sites plus the out-of-scope
  mock-side loopback literals.

  No blocker. No unverified claim in the proof block.
---

# 0069 — Android app: point API base URL at api.nordtronics.io

## Context

The companion app source is `android/companion-v0/`, newest on branch
`hermes/0049-companion-v01-ui` (main only holds the `android-hermes-test/`
skeleton — cut this task's branch from the 0049 tip, not from main).
The app centralizes its backend address in one place, which is good:
`android/companion-v0/app/build.gradle` line 26 sets
`buildConfigField "String", "API_BASE_URL", "\"http://10.0.2.2:8000\""`
and `ApiClient.java` line 20 does `BASE_URL = BuildConfig.API_BASE_URL`.
`10.0.2.2:8000` is the emulator's alias for the dev mock server — it must not
ship in a release build.

The production backend is live and verified: `https://api.nordtronics.io`
serves `/v1/nodes` (HTTP 200) and `/healthz` (HTTP 200). Note the path
mismatch: the app appends `/api/nodes`, `/api/alerts`,
`/api/nodes/{id}/ping` to BASE_URL, but the live API contract is `/v1/...`.

## Task

One deliverable: make the app's release build talk to the production API over
`nordtronics.io` DNS, with no hardcoded IPs, while keeping the emulator/mock
workflow intact for local development.

## Success criteria

- No hardcoded IPv4/IPv6 address anywhere under `android/companion-v0/`,
  except the documented dev override below.
- Release build's effective base URL is `https://api.nordtronics.io` and its
  request paths match the live API contract (probe `GET /v1/nodes` and
  `/healthz` yourself to confirm the exact paths before wiring them).
- Debug build keeps working against the local mock: `http://10.0.2.2:8000`
  remains available via a debug-only buildConfigField or equivalent override
  (document which one in the reply).
- HTTPS to the production host works from the app (check the network security
  config — the current http dev value implies cleartext may have been allowed).
- CI builds the APK green on the branch.

## Constraints

- Branch: `hermes/0069-app-api-dns`, cut from `origin/hermes/0049-companion-v01-ui`
  tip. Do NOT merge to main in this task (main's android tree is a separate
  skeleton; merging is a later decision).
- Out of scope: ESP32 firmware MQTT IPs (192.168.1.x in `firmware/` — LAN
  devices, not the app) and the mock server's own 127.0.0.1 loopback defaults.
  Mention any you find in the notes, but don't change them.
- Run this task on the deepseek-flash worker tier. State the model used in your
  reply.
- One deliverable per task: DNS cutover only. No UI changes, no auth/push
  wiring.

## Proof

- `proof` front-matter: branch `hermes/0069-app-api-dns` + tip SHA, and the
  green Actions run URL whose head SHA matches that tip.
- I will verify the pointers and grep the branch for hardcoded IPs myself.

## Reply format

Stage the file with `status: staged` and fill in:

```yaml
proof:
  - branch: hermes/0069-app-api-dns
    sha: <40-char tip sha>
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/<id>
```

Plus in the body or notes:
- Base branch used and why (confirm 0049 tip was still the newest app source).
- Files changed and what changed in each (include the live-API paths you
  probed and wired).
- How the debug/emulator override is selected.
- `grep` evidence showing no hardcoded IPs remain in `android/companion-v0/`
  outside the debug override.
- Model/tier used.

---

# Reply — Hermes → Juno

**Status: staged.** Branch `hermes/0069-app-api-dns` @
`2829d8f1c954221c261758491bc826955f535252`; CI run
<https://github.com/pagosacabin/nordtronics/actions/runs/36294156654> —
`conclusion: success`, `headSha: 2829d8f1c954221c261758491bc826955f535252`,
all 9 steps green (checked with `gh api .../jobs`, not inferred from the run's
overall verdict). Artifacts: release APK id `10923476721`, debug APK id
`10923342772`. ntfy receipt id **`mL9iGiGX1137`**, topic
`nordtronics-build-ed05a663`, published 2026-09-27T04:26:13Z. **Model/tier:
`deepseek-flash`** (provider `deepseek`).

## Base branch used, and why — please read, the spec's premise is stale

I used `origin/hermes/0049-companion-v01-ui` tip `0cc5e3d`, as the Constraints
instruct. But the Context's claim that the 0049 tip is the newest app source is
**not correct**, and I could not confirm it:

- `origin/hermes/0050-splash-wordmark-fix` @ `fb7701d957cce9b49d29abe003080886f2721465`
  is **newer** (2026-09-24 06:20 MDT vs 2026-09-23 19:33 MDT) and is
  **verified + archived** (`mailbox/archive/0050-splash-wordmark-fix.md`,
  `status: verified`).
- `git merge-base --is-ancestor 0cc5e3d fb7701d` → true, so 0050 is a **strict
  superset** of the 0049 tip. Its extra commits are `e6ca755` (fit the splash
  lockup inside the framework's 192dp icon mask) and `fb7701d` (re-capture
  splash.png from the CI-built debug APK).
- 0050 changed the five `res/drawable-*/splash_lockup.png` assets,
  `screenshots/splash.png`, `tools/make_branding_assets.py`, and one line of
  `.github/workflows/android-companion-v0.yml`.

I still based on the 0049 tip because the Constraints fix the base explicitly
*and* defer merging ("merging is a later decision"), so the base is your call,
not mine — and nothing is lost, because 0050's branch is intact on origin. The
practical consequence for that later merge: 0069's tree does **not** contain
0050's resized splash assets, and the only overlapping file is the workflow,
where both branch names must end up in the `on.push.branches` list (a one-line
resolution). If you would rather the cutover branch already carry 0050, say so
and I will rebase it onto `fb7701d` in one commit.

Confirmed as stated: `origin/main` has no `android/companion-v0/` tree at all
(it holds the `android-hermes-test/` skeleton), so main was correctly ruled out.

## Live API contract — probed before wiring

`curl` against `https://api.nordtronics.io` (2026-09-27 04:15Z):

| Path | Result |
|---|---|
| `GET /v1/nodes` | **200** — `{"nodes":[{"node_id":"node-01","last_seen_utc":…,"status":"stale","age_seconds":…,"latest":{"pm25":7.5,"temperature_c":21.0,"humidity_pct":41.0,"battery_v":3.8}}],"count":1,…}` |
| `GET /healthz` | **200** — `{"status":"ok","database":"ok","schema_version":1,…}` |
| `GET /v1/nodes/node-01/readings` | 200 (exists; not wired — see below) |
| `GET /api/nodes`, `GET /api/alerts`, `GET /v1/alerts` | **404** |
| `POST` ping | no route exists |

The service's own `/openapi.json` lists exactly three paths: `/healthz`,
`/v1/nodes`, `/v1/nodes/{node_id}/readings`. **There is no alerts endpoint and
no ping endpoint**, so the `/api/alerts` → `/v1/alerts` prefix swap the Context
implies would have been a 404 in release. I did not wire `/v1/nodes/{id}/readings`
into the detail screen's history panel either: those are *readings*, not alerts,
and presenting them under an "alert history" heading would be labelling
telemetry as alerts.

## Files changed

| File | Change |
|---|---|
| `app/build.gradle` | `defaultConfig` (release) now declares `API_BASE_URL = https://api.nordtronics.io`, `PATH_NODES = /v1/nodes`, `PATH_HEALTH = /healthz`, and `PATH_ALERTS`/`PATH_NODE_PING = null`. The debug build type re-declares the base URL, the three mock paths and `PATH_HEALTH = null`. Every address and path in the app is now one of these fields. |
| `ApiClient.java` | Path accessors (`nodesPath`, `healthPath`, `alertsAvailable`, `pingAvailable`, `pingPath`) plus `getNodes()`, `getAlerts()`, `getHealth()`, `postPing()`. `getNodes()` accepts either payload dialect — a bare array (mock) or `{"nodes":[…]}` (production). |
| `Node.java` | Reads the production shape (nested `latest`, `temperature_c`/`humidity_pct`, `node_id`, `last_seen_utc`) as well as the mock's flat shape, so one model serves both. |
| `NodesActivity.java` | Node list via `ApiClient.getNodes()`; on-screen request path from `ApiClient.nodesPath()`. |
| `NodeDetailActivity.java` | Node via `getNodes()`; alert history and ping button state that the production API serves no such endpoint and make **no request**, instead of calling a route that must 404. |
| `AlertsActivity.java` | Same gating: release says "Alerts unavailable — not served by the production API yet" and issues no request; debug still reads the mock feed. |
| `SystemActivity.java` | Reachability probe is now `ApiClient.healthPath()` (`/healthz` in release, `/api/nodes` in debug), with the node count from `nodesPath()`; the "protocol source" line shows the real probe path. |
| `AndroidManifest.xml` | `android:usesCleartextTraffic="true"` → `android:networkSecurityConfig="@xml/network_security_config"`. |
| `app/src/main/res/xml/network_security_config.xml` **(new)** | Release policy: `<base-config cleartextTrafficPermitted="false"/>`, no exception. |
| `app/src/debug/res/xml/network_security_config.xml` **(new)** | Debug override of the same resource: cleartext denied at the base and permitted for `10.0.2.2` only. |
| `.github/workflows/android-companion-v0.yml` | `hermes/0069-app-api-dns` added to `on.push.branches`; `assembleRelease` step added and the release APK uploaded as `companion-v0-release-apk`. |
| four layouts, `Alert.java` | Comment-only: endpoint references that named `/api/...` now describe the dialect instead, so a path grep is not confused by stale prose. |
| `mock-server/server.py` | Comment-only: the docstring no longer spells out the dev IP. No behaviour change; the loopback bind and its defaults are untouched. |

## How the debug/emulator override is selected

A **debug-only `buildConfigField` pair**, not a resource or a runtime check:
`defaultConfig` carries the production values and the `debug` block overrides
them, so `BuildConfig.API_BASE_URL` is `https://api.nordtronics.io` in any
non-debug build type and `http://10.0.2.2:8000` only in debug. The cleartext
exception that the mock needs is likewise debug-only, supplied by the debug
source set's `network_security_config.xml`. `10.0.2.2` therefore exists in
exactly two places, both debug-scoped.

## Verification actually performed

1. **CI-built artifacts, not my local build** — I downloaded both APKs from run
   36294156654 and inspected them. (The CI release APK is byte-identical to my
   local one: sha256 `890ba8c78bfed1e1edcd9df716ac50cc9639ba39dc01b82b53b8258599c3495a`.)
2. **Release APK dex** contains `https://api.nordtronics.io`, `/v1/nodes` and
   `/healthz`; contains **no** `10.0.2.2` and **no** `/api/nodes` or `/api/alerts`
   string — a release build cannot request a mock path or name an IP.
   **Debug APK dex** is the mirror image: `http://10.0.2.2:8000`, `/api/nodes`,
   `/api/alerts` present, production URL absent. The two probes make each other
   non-vacuous.
3. **Packaged network security policy**, read out of each APK with
   `aapt2 dump xmltree`: release has only `base-config
   cleartextTrafficPermitted=false` with no `domain-config` and no `10.0.2.2`
   anywhere; debug has that base plus `domain-config
   cleartextTrafficPermitted=true` for domain `10.0.2.2`
   (`includeSubdomains=false`) and nothing else. Both manifests carry
   `android:networkSecurityConfig=@0x7f100000`.
4. **The app's own code executed against both real backends.** I compiled
   `ApiClient.java`, `Node.java` and `Alert.java` unmodified against a stub
   `BuildConfig` (JDK 17, `org.json` reference jar) and ran a scratch harness:
   - release dialect → real HTTPS `GET https://api.nordtronics.io/v1/nodes`
     returned 1 node, parsed as `id=node-01 pm25=7.5 tempC=21.0 tempF=69.8
     humidity=41.0 batteryV=3.8 batteryPercent=67 lastSeen=2026-09-25T19:20:01Z
     ageSeconds=118955 statusWord=Healthy`; `GET /healthz` returned
     `{"status":"ok",…}`; `getAlerts()` and `postPing()` refused **without
     making a request**.
   - debug dialect → real HTTP against the repo's own `mock-server/server.py`
     running on host loopback returned 2 nodes (`node-01` 12.4 µg/m³, `node-02`
     47.9 µg/m³ → `Watch`), 3 alerts, `postPing(node-01)` → `{"ok": true,
     "node_id": "node-01"}`, and the health probe fell back to `/api/nodes`.
   The harness, the stub and the jar live in my scratch directory, never in the
   repo.
   The one substitution to declare: I pointed the debug stub at
   `http://127.0.0.1:8000` because the emulator alias cannot resolve from the
   host — the app's debug build genuinely carries `10.0.2.2`.
5. **`lintVitalRelease` green** locally before pushing, since `assembleRelease`
   runs it and a fatal lint issue would have failed CI.
6. **Not performed, and not claimed:** no emulator or device ran the app, so
   "renders live data on a screen" and the framework's own runtime enforcement
   of the network security policy are *not* verified. The HTTPS request the app
   issues was executed and returned 200 (point 4); the packaged policy that
   governs it was read out of the APK (point 3).

## `grep` evidence — no hardcoded IPs outside the dev override

`git ls-files android/companion-v0 | xargs grep -InE '\b([0-9]{1,3}\.){3}[0-9]{1,3}\b'`
returns exactly these, and nothing else:

```
app/build.gradle:56            buildConfigField "String", "API_BASE_URL", "\"http://10.0.2.2:8000\""   <- the documented dev override (debug block)
app/src/debug/res/xml/network_security_config.xml:15   <domain includeSubdomains="false">10.0.2.2</domain>  <- same override, debug source set only
app/src/debug/res/xml/network_security_config.xml:7    (its explanatory comment)
mock-server/server.py:28       LOOPBACK_HOSTS = {"127.0.0.1", "localhost", "::1"}     <- out of scope, unchanged
mock-server/server.py:128      --host default "127.0.0.1"                            <- out of scope, unchanged
mock-server/ping-test.log:1..15  127.0.0.1 lines from the mock's own runs            <- out of scope, unchanged
```

An IPv6 pattern returns nothing. **No IPv4 or IPv6 literal exists anywhere in
`app/src/main`** (the release source set). The extra occurrence worth naming per
your instruction is `mock-server/ping-test.log` — a committed log of the mock's
own loopback requests, which contains `127.0.0.1` and which I left untouched.
Also, for a naive path grep: `/api/` still appears in the *Javadoc* of `Node`,
`Alert` and `NodesActivity`, describing the mock dialect. Those are comments, and
the compiled release dex (point 2) contains no `/api/...` string at all.

## Deviations and scope declarations

- **Three live paths wired, not one** (criterion 2: "request paths match the live
  API contract"; criterion 4 names the network security config): `/v1/nodes` for
  the node list, `/healthz` as the reachability probe, and the mock-only features
  gated off in release.
- **A release build added to CI** (criterion 2 is about the release build, which
  `assembleDebug` alone cannot evidence) — hence a 4th artifact and a new step.
- **Two new resource files** for the network security config (criterion 4), since
  the old manifest flag permitted cleartext to *any* host in *every* build type.
- **The alerts feed and ping button were gated, not repointed.** Repointing them
  to a path that does not exist, or deleting the UI, are each worse; both are
  recorded here as a backend gap for you to close or to scope a follow-up task
  for. This is the only place where a screen behaves differently in release than
  in debug.
- **No UI change was made** in the sense the Constraints mean (no layout,
  styling, navigation or screen-content change); the gating above is the limit of
  it, and the layout edits are comments.
- **Base-branch correction** — described at the top; the Constraint was followed
  literally.

No blocker. Nothing is staged that I did not verify, and every pointer above was
re-read from the live GitHub API at the time of writing.
