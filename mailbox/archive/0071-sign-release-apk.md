---
task_id: "0071"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0069-app-api-dns
    sha: 2829d8f1c954221c261758491bc826955f535252
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36294156654
  - files: []
    # no repository file changed by this task: it is an apply task on this host
  - signed_apk_sha256: a883b12251b8983924f299013ee8aa7cc6bd46e5ca1488c4f20f24ab1ca33578
  - unsigned_apk_sha256: 890ba8c78bfed1e1edcd9df716ac50cc9639ba39dc01b82b53b8258599c3495a
  - cert_sha256: e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
  - applied_to:
      - /home/astroboy/.config/nordtronics/nordtronics-release.jks
      - /home/astroboy/.config/nordtronics/release-keystore.pw
      - /home/astroboy/nordtronics-release/nordtronics-companion-v0-release-signed.apk
      - /home/astroboy/nordtronics-release/evidence/signing-and-perms.txt
      - /home/astroboy/nordtronics-release/evidence/nodes-live.png
      - /home/astroboy/nordtronics-release/evidence/ui-nodes.xml
notes: |
  Deliverable: a production-signed, installable release APK from the 0069 build,
  the production keystore + password on this machine, and an install + launch on
  an emulator. Model: deepseek-flash (provider deepseek), the tier the spec named.

  No branch and no CI run of this task exists, by construction: nothing in the
  repository changed, and the deliverable is a local signing operation. The
  branch/sha/run pointers above are the revision and green run the unsigned APK
  was built from - the object I signed - not work I authored. Verified before
  use: run 36294156654 conclusion success, headSha
  2829d8f1c954221c261758491bc826955f535252, headBranch hermes/0069-app-api-dns,
  and `git ls-remote --heads origin hermes/0069-app-api-dns` returns that same
  tip; the workflow's own `Assemble release APK (unsigned)` and `Upload release
  APK` steps both concluded success. APK source: downloaded artifact
  `companion-v0-release-apk` (id 10923476721), NOT a rebuild.

  Signed artifact sha256 a883b12251b8983924f299013ee8aa7cc6bd46e5ca1488c4f20f24ab1ca33578;
  its installed base.apk on the emulator hashes to the same value, so the bytes
  that installed are the bytes I signed. Signature verifies v2 true, v3 true;
  v1 false is correct at minSdk 24 (v2 covers API 24+). zipalign -c -P 16 4 ->
  "Verification succesful".

  What the verifier CAN check independently: the unsigned artifact's
  sha256 890ba8c78bfed1e1edcd9df716ac50cc9639ba39dc01b82b53b8258599c3495a
  against a fresh download of run 36294156654, and the run/branch pointers.
  What it CANNOT: the signature, the keystore and the install - the signed APK
  and keystore are deliberately not in the repo, so the apksigner and adb
  transcripts below are claims corroborated by the evidence files on this host
  at the paths listed above. Pin cert SHA-256
  e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd if a future
  check is wanted.

  No ntfy publish: this is not a build task (no CI-built artifact; the build was
  0069's, already staged and archived) and the spec's Proof section names no
  topic. Declared so it is not read as an omission.

  Deviations, all deliberate: (1) the task shipped with NO front-matter, unlike
  0070 - pickup added task_id/protocol_version/status/iteration and moved the
  stray `expect-reply-within: 6h` body line into the front-matter, because the
  state machine has nowhere else to record status. (2) `zipalign -P 16 4`
  replaced `-p` because build-tools 35 rejects the combination ("-P and -p
  cannot be used in combination"); -P 16 is the targetSdk-35 correct form and
  the APK has no .so files (0 matches), so -p was a no-op anyway. (3) keystore
  store type is PKCS12, not JKS, despite the .jks extension: the spec's literal
  keytool command omits -storetype and the JDK default is PKCS12 - read, not
  assumed (`keytool -list` prints "Keystore type: PKCS12"). apksigner accepted
  it with no --ks-type override. (4) minted with JDK 17's keytool (the JDK the
  Android build and AGP 8.3 use), not the system JDK 25. (5) -dname supplied
  because the spec's command has none and would have prompted:
  CN=Nordtronics Companion, OU=Android, O=Nordtronics - no city/state/country
  invented. (6) a debug-signed build of the same package left on the AVD by an
  earlier task had to be uninstalled before the production-signed APK would
  install; that is a local test emulator, not Stephen's phone.

  Blocked: nothing. Unverified claims in the proof block: none.
---

# 0071 — Mint the production release key, sign the 0069 release APK, install

## Context

Task 0069 built the release APK on branch `hermes/0069-app-api-dns`
(tip `2829d8f1c954221c261758491bc826955f535252`, CI run 36294156654 green).
The artifact `companion-v0-release-apk` (id 10923476721) contains
`app-release-unsigned.apk` — it points at the live production API
(`https://api.nordtronics.io`) but is unsigned. Stephen has now decided:
mint the PRODUCTION release key now (not a throwaway test key), on his
machine. This key is the app's permanent identity — every future update must
be signed with it. Model: deepseek-flash. Report the model used in your reply.

## Task

One deliverable: a production-signed, installable release APK from the 0069
build, installed on the connected Android test device (or handed to Stephen
as a file if no device is attached), with the new production keystore and
its password stored safely on the machine.

## Success criteria

- Download the `companion-v0-release-apk` artifact from run 36294156654
  (or rebuild `assembleRelease` from the 0069 tip — either is fine, say which).
- Generate a strong random password: `openssl rand -base64 32`, one line, no
  trailing newline issues. Store it in `~/.config/nordtronics/release-keystore.pw`
  with `0600` permissions. This file is the ONLY place the password lives.
- Mint the production keystore with `keytool`:
  `keytool -genkeypair -keystore ~/.config/nordtronics/nordtronics-release.jks -alias nordtronics-release -keyalg RSA -keysize 4096 -validity 10950 -storepass:file <pw file>`
  (use the same password for the key itself). Validity 10950 days = 30 years.
- `zipalign` then `apksigner sign --ks ~/.config/nordtronics/nordtronics-release.jks --ks-pass:file <pw file>` the APK, then
  `apksigner verify --print-certs` — the verify must pass.
- If an Android device is attached via adb: `adb install` the signed APK and
  confirm the package installs (report the package name and install result).
  If no device is attached: place the signed APK at a stable path on the
  machine and report the exact path so Stephen can grab it.
- Launch the app once if a device is attached and report what the node list
  shows against the live API (nodes visible? any error text?). Do not
  troubleshoot the backend in this task — just report what you see.

## Constraints

- NEVER put the password, or the keystore itself, in the repo, in the reply
  file, or in any log. The reply reports PATHS only:
  `~/.config/nordtronics/nordtronics-release.jks`,
  `~/.config/nordtronics/release-keystore.pw`, alias `nordtronics-release`.
- Do NOT commit the keystore, the password file, or the signed APK to the repo.
- Stephen must copy the keystore AND the password file to his backup. Say
  this explicitly in the reply notes: if that laptop dies without a backup,
  the app can never be updated under the same package name again.
- The unsigned APK stays as CI built it; you are only adding the signature.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- `apksigner verify --print-certs` output (paste it — this is the signature
  evidence, not a summary).
- `adb install` result line, or the exact filesystem path of the signed APK.
- `ls -l` of the two `~/.config/nordtronics/` files showing `0600`/`-rw-------`.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field stating the
keystore/alias/password-file paths, the backup warning for Stephen, and what
the app showed on launch (if tested).

---

# Reply — Hermes → Juno

**Status: staged.** This task produces no branch and no CI run — nothing in the
repository changes; the deliverable is a locally produced production-signed APK
plus a keystore on this machine. The proof pointers therefore name the revision
and green run the unsigned APK was built from, not work I authored. **Model/tier:
`deepseek-flash`** (provider `deepseek`).

## What was produced

| Item | Value |
|---|---|
| Signed APK | `/home/astroboy/nordtronics-release/nordtronics-companion-v0-release-signed.apk` — 2,826,580 bytes, sha256 `a883b12251b8983924f299013ee8aa7cc6bd46e5ca1488c4f20f24ab1ca33578` |
| Unsigned input (CI artifact, untouched) | `/home/astroboy/nordtronics-release/unsigned/app-release-unsigned.apk` — sha256 `890ba8c78bfed1e1edcd9df716ac50cc9639ba39dc01b82b53b8258599c3495a` |
| Keystore | `~/.config/nordtronics/nordtronics-release.jks` — 4386 bytes, `0600`, PKCS12 |
| Password file | `~/.config/nordtronics/release-keystore.pw` — 44 bytes, `0600` |
| Alias | `nordtronics-release` |
| Keystore directory | `~/.config/nordtronics/` — mode `0700` |
| Package | `com.nordtronics.companion`, versionCode 1, versionName 0.1, minSdk 24, targetSdk 35 |

APK source: I downloaded the `companion-v0-release-apk` artifact (id 10923476721)
from run 36294156654 rather than rebuilding. Before using it I re-read the run
from the live API: `conclusion: success`, `headSha:
2829d8f1c954221c261758491bc826955f535252`, `headBranch: hermes/0069-app-api-dns`;
`git ls-remote --heads origin hermes/0069-app-api-dns` returns that same tip; and
the workflow's own steps `Assemble release APK (unsigned)` and `Upload release
APK` both concluded `success`.

## apksigner verify — the signature evidence, pasted verbatim

```
$ apksigner verify --print-certs nordtronics-companion-v0-release-signed.apk

Signer #1 certificate DN: CN=Nordtronics Companion, OU=Android, O=Nordtronics
Signer #1 certificate SHA-256 digest: e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
Signer #1 certificate SHA-1 digest: 94bc0f6474dce668e5ef73e1acfd14d391c15d1c
Signer #1 certificate MD5 digest: bf6b776704d8cdee5b73d322243a15b9

$ apksigner verify --verbose nordtronics-companion-v0-release-signed.apk

Verifies
Verified using v1 scheme (JAR signing): false
Verified using v2 scheme (APK Signature Scheme v2): true
Verified using v3 scheme (APK Signature Scheme v3): true
Verified using v3.1 scheme (APK Signature Scheme v3.1): false
Verified using v4 scheme (APK Signature Scheme v4): false
Verified for SourceStamp: false
Number of signers: 1
```

`v1: false` is correct here, not a defect: minSdk is 24, and APK Signature
Scheme v2 covers API 24+ on its own; v1 would only be needed below API 24.
Alignment of the signed APK was re-checked after signing:
`zipalign -c -P 16 -v 4 …` → `Verification succesful`.

## Proof pointers, including what they do and do not cover

- **`branch` + `sha`** — `hermes/0069-app-api-dns` @
  `2829d8f1c954221c261758491bc826955f535252`, re-read with
  `git ls-remote --heads origin` while writing this.
- **`run`** — <https://github.com/pagosacabin/nordtronics/actions/runs/36294156654>:
  `conclusion: success`, `headSha` equal to that tip.
- **`files: []`** — no repository file changed. This is an apply task on one
  host (the keystore and the APK live outside the repo, as the constraints
  require), so there is no branch and no CI run of this work to point at.
- **`applied_to`** — the host paths this run created or wrote, listed in the
  front-matter.

**Independent-verification limit, stated plainly.** Juno *can* check the
unsigned artifact: a fresh download of that run's `companion-v0-release-apk`
hashes to `890ba8c7…`. Juno *cannot* check the signature, the keystore or the
install, because the signed APK and keystore are deliberately not in the repo
and this task has no CI run. The `apksigner` output and the `adb` transcripts
below are therefore claims corroborated by the evidence files on this host
(`/home/astroboy/nordtronics-release/evidence/signing-and-perms.txt`,
`nodes-live.png`, `ui-nodes.xml`). The value worth pinning is the certificate
SHA-256 `e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd`.

## `ls -l` of the two `~/.config/nordtronics/` files

```
$ ls -l ~/.config/nordtronics/
total 12
-rw-------. 1 astroboy astroboy 4386 Sep 27 00:16 nordtronics-release.jks
-rw-------. 1 astroboy astroboy   44 Sep 27 00:16 release-keystore.pw

$ stat -c '%a %n' ~/.config/nordtronics/*
600 /home/astroboy/.config/nordtronics/nordtronics-release.jks
600 /home/astroboy/.config/nordtronics/release-keystore.pw
```

The password file is 44 bytes of `openssl rand -base64 32` with the trailing
newline stripped (`ends_with_newline: False`, verified by reading the bytes —
not by looking at the file). It is the only place the password exists.

## Install and launch — an Android device did answer to adb

No physical device was attached (`adb devices -l` empty; no phone in `lsusb`),
so the spec's fallback applies and the signed APK sits at the stable path above
for Stephen to grab. But this host has local AVDs from earlier tasks, so rather
than report the fallback alone I booted `companion33` (API 33, x86_64) and
satisfied the install/launch criteria on it:

```
$ adb -s emulator-5554 install nordtronics-companion-v0-release-signed.apk
Performing Incremental Install
Success
Install command complete in 184 ms

$ adb -s emulator-5554 shell dumpsys package com.nordtronics.companion
    versionName=0.1
    apkSigningVersion=4
```

The installed bytes are the signed bytes — the emulator's own
`sha256sum /data/app/…/com.nordtronics.companion-…/base.apk` returned
`a883b12251b8983924f299013ee8aa7cc6bd46e5ca1488c4f20f24ab1ca33578`, identical
to the local signed APK, so `adb install` did not re-sign or rewrite anything.

**Finding for Stephen:** the first attempt failed with
`INSTALL_FAILED_UPDATE_INCOMPATIBLE: Existing package com.nordtronics.companion
signatures do not match newer version`. The AVD already carried a *debug-signed*
build of the same package from an earlier task (`signatures:[b154b175]`,
firstInstallTime 2026-09-23). I uninstalled that build and installed fresh. If
the real test phone has a debug build installed, it must be uninstalled once
before the production-signed APK will install — the production key is a new
identity, and that is exactly the behaviour a permanent key produces.

## What the app showed against the live API

```
$ adb -s emulator-5554 shell am start -W -n com.nordtronics.companion/com.nordtronics.companion.NodesActivity
Status: ok
LaunchState: COLD
TotalTime: 943
```

Read from the `uiautomator` view tree 10 s after launch (`evidence/ui-nodes.xml`;
screenshot `evidence/nodes-live.png`, 1080x2340):

- Footer: **`1 nodes from https://api.nordtronics.io/v1/nodes`** — the release
  build is talking to the live production API and got an answer.
- `Network consensus · 1 of 1` → `All clear` → *"Readings are stable across the
  property. No smoke pattern is forming."*
- One node rendered: `Node 01`, `Healthy · received 35 h ago`, `7.5 µg/m³`
  PM2.5, `70°F`, `41% RH`, `3.80 V` battery.
- Summary row: PM2.5 median 8 µg/m³, 70°F, 41% RH, reporting `1/1` NODES.

**Nodes are visible, and there is no error text on the screen.** The screenshot
was confirmed by an independent pixel pass as a fully rendered app screen (dark
theme, 1,392 distinct sampled colours, 0 pure-red pixels, no dialog, no crash
banner, no launcher) — not a blank frame and not a claim from pixels alone,
since the view tree above is the authoritative record of the on-screen text.

Reported only, not diagnosed, per the constraint: the node's last packet is
~35 h old, so the telemetry rendered is stale, and the footer reads "1 nodes"
(grammar is Juno's copy, not mine to touch).

## Deviations and corrections

1. **Front-matter added at pickup.** Task 0071 shipped with no front-matter at
   all — no `task_id`, `status` or `iteration`, unlike 0070 — so the state
   machine had nowhere to record state. Pickup added `task_id: "0071"`,
   `protocol_version: 1.0.0`, `status`, `iteration: 1`, and moved the stray body
   line `expect-reply-within: 6h` into the front-matter.
2. **`zipalign -P 16 4` instead of `-p`.** Build-tools 35 rejects the
   combination outright: `ERROR: Invalid options: '-P <pagesize_kb>' and '-p'
   cannot be used in combination`. `-P 16` is the targetSdk-35-correct form
   (16 KiB page alignment); the APK contains no native libraries
   (`unzip -l | grep -c '\.so$'` → `0`), so `-p` would have been a no-op.
3. **Keystore store type is PKCS12, not JKS**, despite the `.jks` extension the
   spec fixes. The spec's literal `keytool` command omits `-storetype` and the
   JDK's default is PKCS12 — read, not assumed: `keytool -list` prints
   `Keystore type: PKCS12`. `apksigner` accepted it with no `--ks-type`
   override. If the Gradle `signingConfig` is written explicitly, set
   `storeType "pkcs12"` or leave it default.
4. **Minted with JDK 17's `keytool`** (`~/jdks/jdk-17.0.20.1+1/bin/keytool`) —
   the JDK the Android build and AGP 8.3 use — rather than the system JDK 25, so
   the keystore is written by the oldest JDK in the toolchain.
5. **`-dname` supplied.** The spec's command carries none, so `keytool` would
   have prompted. Used: `CN=Nordtronics Companion, OU=Android, O=Nordtronics`.
   No city, state or country was invented. The DN is baked into the certificate
   permanently — flag it now if it should read differently, because changing it
   later means a new key and therefore a new app identity.
6. **An AVD was uninstalled from** to prove the install; that is a local test
   emulator from an earlier task, not Stephen's phone and not the production API.
7. **No ntfy publish.** This is not a build task by the README's definition (the
   deliverable is not a CI-built artifact; 0069's build was already staged and
   archived) and the spec's Proof section names no topic, so nothing was
   published. Declared plainly so it is not read as an omission.

## Evidence on disk (nothing here is committed)

- `/home/astroboy/nordtronics-release/nordtronics-companion-v0-release-signed.apk`
- `/home/astroboy/nordtronics-release/unsigned/app-release-unsigned.apk` — as CI built it
- `/home/astroboy/nordtronics-release/app-release-aligned.apk` — intermediate
- `/home/astroboy/nordtronics-release/evidence/signing-and-perms.txt` — verify output, `ls -l`/`stat` of both config files, all three sha256s
- `/home/astroboy/nordtronics-release/evidence/nodes-live.png`, `evidence/ui-nodes.xml`

No keystore, no password file and no signed APK was committed. This reply names
paths and the alias only; it contains no secret value.

## The one action this task needs from Stephen

**Copy BOTH `~/.config/nordtronics/nordtronics-release.jks` and
`~/.config/nordtronics/release-keystore.pw` to your backup now.** That keystore
is the app's permanent identity for the package name `com.nordtronics.companion`;
every future update must be signed with this exact key. If that laptop dies with
no copy of both files, the app can never be updated under the same package name
again — the only recovery is a new package name and a fresh install for every
user. The password exists in exactly one place, the file above; it is not in the
repo, not in my run logs, and not in this reply.

No blocker. No unverified claim in the proof block.

Juno verification (2026-09-27 ~00:47 MDT): unsigned input hash 890ba8c7... recomputed from a fresh CI artifact download on Juno's side - MATCHES. Branch/run pointers re-confirmed live. Host-local claims (keystore mint, apksigner transcripts, adb install, launch) reported with pasted evidence; not remotely verifiable by design - see reply's own verification-limit section. Cert SHA-256 e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd pinned as the app identity reference.
