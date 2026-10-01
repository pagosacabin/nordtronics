---
task_id: "0086"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
notes: |
  STAGED 2026-10-01 — blocker 1 cleared. The GPG recovery test was performed in
  an interactive session with Stephen supplying the backup passphrase; it
  PASSED. Blocker 2 (spec defect) is unchanged and declared as a deviation
  below. Staging is Stephen-directed; the earlier blocked run was the cron
  worker on deepseek-flash.

  RECOVERY TEST (step 1) — PERFORMED AND PASSED, 2026-10-01 10:26 MDT:
  fetched the encrypted bundle from the VPS (deploy@89.117.21.105:
  /home/deploy/backups/nordtronics-release-key-backup-2026-09-27.tar.gz.gpg) and
  compared it by content hash to the local copy — both
  sha256 8b65971419cde5c3ddd83a3b0441d732c4a3af4a12084046d42deef4a80ab6b7.
  Decrypted non-interactively: gpg --batch --pinentry-mode loopback with the
  passphrase supplied BY FILE PATH from a 0600 file. The value never entered
  chat, a command line, an argv list, the repo or any log. gpg exit 0.
  Bundle crypto (from gpg --list-packets, no passphrase needed): AES256.CFB,
  s2k mode 3, hash 10, MDC present.
  Archive contents: nordtronics-release.jks (4386 B) + release-keystore.pw (44 B).
  RESTORE VERIFIED BY CONTENT, NOT BY EXIT CODE:
    recovered .jks  sha256 ed6da2fef7b9dd638f2af9621406b5640f65317258340225b2677ecad97f9537
      == live ~/.config/nordtronics/nordtronics-release.jks (same hash)
    recovered .pw   sha256 d92853e256866d3d8f0b907816b8418ae82a727e72ed0c0973ac3cde4c232ef8
      == live ~/.config/nordtronics/release-keystore.pw (same hash)
  keytool -list -v on the RECOVERED keystore: alias nordtronics-release, cert
  SHA-256 E4:38:9C:E4:CB:17:3D:A1:55:B6:3C:AD:F2:4C:99:97:23:13:0C:20:9B:D5:5C:A2:E1:1B:BF:82:A2:A4:4D:DD
  == the production identity pinned by 0071. So the backup is restorable and the
  restored material is the production key, not a lookalike.
  The plaintext intermediates were shredded in the same session, gpg-agent's
  cached passphrase was cleared (gpgconf --kill gpg-agent), and a sweep for
  stray copies or editor leftovers found none.

  CRITERION 2 AS WRITTEN IS STILL UNSATISFIABLE (unchanged, needs Juno):
  run 36818994897 uploads app-release-unsigned.apk and it carries no signature,
  so there is no CI cert digest to equal. Verbatim, re-run in this session with
  apksigner 35.0.0: "DOES NOT VERIFY / ERROR: Missing META-INF/MANIFEST.MF".
  Substituted reference, declared as a deviation: the production identity pinned
  by 0071, e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd.

  ARTIFACT (public digests only — certificate digests are printed on every
  signed APK):
  - install target /home/astroboy/nordtronics-release/0086/
      nordtronics-companion-v0-0084-release-signed.apk
      2,785,109 B
      sha256 fa15cfdc7708f5cec0d41d8e911fda29c0d6bb666c36e64fcfd97e3e6954d1f6
      cert SHA-256 e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
      apksigner: Verifies; signer CN=Nordtronics Companion, OU=Android, O=Nordtronics
      v2 true (v1 false is correct at minSdk 24); zipalign -c -P 16 4 -> OK
  - cross-check, 0071 apksigner recipe (v2+v3, has .idsig)
      nordtronics-companion-v0-0084-release-signed-apksigner.apk
      2,826,580 B  sha256 b46dd91a91c59856dab0f7b4be76694876c14240c38da6c9f7fdebefab12811b
      cert SHA-256 e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd (identical)
  - unsigned input app-release-unsigned-0084.apk, 2,776,917 B
      sha256 7f30e4940680b2336b2396f9d30666dc51b2f65c7a043ac1b067407828935dba
      (the same hash the blocked run recorded for the CI artifact of run
      36818994897 — that pointer rests on the earlier run's download; gh is
      unauthenticated on this host, so it was not re-downloadable here)

  KEYSTORE (step 2, unchanged from 0071, outside the repo):
  ~/.config/nordtronics/nordtronics-release.jks, mode 0600, PKCS12, alias
  nordtronics-release. Nothing was copied, moved or re-imported — the decrypt
  was a read-only recovery test, and the live originals were not touched.
  Contents never read out into any output.

  STEPHEN'S REMAINING MANUAL STEPS:
  1. Uninstall the debug build of com.nordtronics.companion from the phone once
     (a production key is a new app identity; installing over a debug-signed
     build fails with INSTALL_FAILED_UPDATE_INCOMPATIBLE).
  2. Install the signed APK above — DONE 2026-10-01 10:40 MDT, performed at
     Stephen's instruction over adb, and verified ON THE DEVICE rather than
     from adb's exit status: the installed APK was pulled back off the phone and
     hashes to
     fa15cfdc7708f5cec0d41d8e911fda29c0d6bb666c36e64fcfd97e3e6954d1f6
     — identical to the deliverable file — and its signer cert SHA-256 is
     e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd, the
     production identity. The app launches (PID live, nothing in the crash
     buffer) and renders the dawn-pine UI against https://api.nordtronics.io/v1/nodes.
     Two hiccups, recorded rather than smoothed over: `adb install -r` first
     returned INSTALL_PARSE_FAILED_NOT_APK against its own streamed temp file and
     then succeeded on the retry, and `adb uninstall` returned
     DELETE_FAILED_INTERNAL_ERROR because this package was NOT installed on the
     phone at all — so the clean-slate uninstall had nothing to remove and no
     app data was lost. The end state is verified directly on the device, which
     is why neither hiccup changes the outcome.
  3. Disposition of ~/.config/nordtronics/backup-passphrase.pw — DECIDED by
     Stephen 2026-10-01: KEPT in place at 0600 (16 bytes), so a future run can
     re-test recovery from this host without fetching the passphrase from his
     password manager, which remains the off-host copy. Nothing was shredded.

  DECISION STILL OWED BY JUNO: accept the substituted signature reference, or
  name the artifact whose digest should be compared instead.

  DEVIATIONS, all deliberate and stated: (1) signature reference substituted
  (above); (2) signing config committed on the spec's branch — build plumbing
  only (app/build.gradle, .gitignore), no app source, colour or API change;
  (3) a second signed APK produced via the 0071 apksigner recipe as a
  cross-check; (4) the build ran --offline against the local Gradle cache; (5)
  no CI run exists for hermes/0086-local-release-signing, so none is cited;
  (6) the recovery test was run in an interactive session with Stephen present,
  as the spec requires, rather than by the unattended worker.
proof:
  - branch: hermes/0086-local-release-signing
    sha: ee96e07cfe001adc2065c7b38370fad78945df0c
  - files: android/companion-v0/app/build.gradle, android/companion-v0/.gitignore
---

# 0086 — Import the production signing key locally; build an installable signed release APK

## Context (as filed)

Stephen's phone runs the debug build, and Android refuses to install a release
APK over it. The production keystore exists as an encrypted backup on the VPS;
the passphrase lives in Stephen's password manager and had never been
recovery-tested. Source: `hermes/0084-app-reskin-dawn-pine` @
`3c368eeb1a4a9fa6c4747c846c3240226977c5b0`.

---

# Reply — Hermes → Juno — **STAGED**

**Model note:** the blocked run was the cron worker on `deepseek-flash`
(`deepseek`). This staging step is an interactive, Stephen-directed session.

## 1. The recovery test (step 1) — PASSED

This was the blocker that parked the task: an unattended cron tick has no
channel to Stephen, and the spec forbids guessing or retrying the passphrase.
With Stephen present, the passphrase was written by him into a 0600 file
(`read -s`, so it stayed off screen and out of shell history) and read by gpg
**by path** — never through chat, a command line, or an argv list:

```
gpg --batch --quiet --yes --pinentry-mode loopback \
    --passphrase-file ~/.config/nordtronics/backup-passphrase.pw \
    -d vps-backup.tar.gz.gpg > recovered.tar.gz      # exit 0
```

| Check | Result |
|---|---|
| VPS blob vs local blob, by content hash | identical — `8b659714…b6b7` |
| Bundle crypto (`--list-packets`, no passphrase) | AES256.CFB, s2k mode 3, hash 10, MDC present |
| Archive contents | `nordtronics-release.jks` (4386 B) + `release-keystore.pw` (44 B) |
| Recovered `.jks` vs live file | identical — `ed6da2fe…f9537` |
| Recovered `.pw` vs live file | identical — `d92853e2…2ef8` |
| Cert inside the **recovered** keystore | `E4:38:9C:E4:…:4D:DD` = the 0071 production identity |

So the backup is not merely decryptable — it restores to the **same bytes** as
the live production keystore, and that keystore still holds the production
identity. The recovery test has now been run for the first time; before this
session it had never been attempted.

Hygiene, same session: plaintext intermediates shredded (`shred -u`), gpg-agent's
cached passphrase cleared (`gpgconf --kill gpg-agent`), scratch dir removed, and
a sweep of `~/.config` + `~/.hermes` for stray copies or editor leftovers found
nothing. The live originals were never modified — the recover was read-only.

## 2. Criterion 2 as written is impossible — the CI APK is unsigned

Unchanged from the blocked report, and re-established in this session rather
than quoted:

```
$ apksigner verify --print-certs app-release-unsigned-0084.apk
DOES NOT VERIFY
ERROR: Missing META-INF/MANIFEST.MF
```

Run 36818994897 (`success`, headSha `3c368eeb…c5b0`, headBranch
`hermes/0084-app-reskin-dawn-pine`) uploads `app-release-unsigned.apk`; the
workflow declares no signing config. There is no CI cert digest to equal.

**Substituted reference** (declared as a deviation): the production identity
pinned by 0071,
`e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd`. The local
signed APK's cert digest is that value, and so is the digest of the recovered
keystore (section 1) and of the live keystore. Same key, three ways.

## 3. The deliverable

```
/home/astroboy/nordtronics-release/0086/nordtronics-companion-v0-0084-release-signed.apk
2,785,109 bytes   sha256 fa15cfdc7708f5cec0d41d8e911fda29c0d6bb666c36e64fcfd97e3e6954d1f6
cert SHA-256      e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
```

Re-verified this session with `apksigner 35.0.0`:

```
Signer #1 certificate DN: CN=Nordtronics Companion, OU=Android, O=Nordtronics
Signer #1 certificate SHA-256 digest: e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
Verifies
zipalign -c -P 16 4  ->  Verification succesful
```

`v1 scheme: false` is correct at minSdk 24, not a defect. A second APK built
through the 0071 `apksigner` recipe (v2+v3, with `.idsig`) carries the same cert
digest — cross-check, not a substitute for the deliverable above.

**The build reproduces CI, so the signed APK is the 0084 app and not a
lookalike:** the locally assembled *unsigned* release APK hashes to
`7f30e494…5dba`, the same sha256 recorded for the CI artifact of run
36818994897, and the packaged resources show the `nt_*` dawn-pine palette.

## 3a. Installed and verified on the device (2026-10-01 10:40 MDT)

The install was performed over adb at Stephen's instruction and then verified by
reading the device back, because "adb said Success" is not evidence that the
production-signed build is what landed:

```
$ adb install -r nordtronics-companion-v0-0084-release-signed.apk
   … first attempt: INSTALL_PARSE_FAILED_NOT_APK on its streamed temp file
   … retry: Performing Streamed Install / Success
$ adb shell pm path com.nordtronics.companion
/data/app/~~4Ytdu…/com.nordtronics.companion-…/base.apk
$ adb pull <that> installed.apk && sha256sum installed.apk <the deliverable>
fa15cfdc7708f5cec0d41d8e911fda29c0d6bb666c36e64fcfd97e3e6954d1f6  installed.apk
fa15cfdc7708f5cec0d41d8e911fda29c0d6bb666c36e64fcfd97e3e6954d1f6  deliverable.apk
$ apksigner verify --print-certs installed.apk
Signer #1 certificate DN: CN=Nordtronics Companion, OU=Android, O=Nordtronics
Signer #1 certificate SHA-256 digest: e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
```

So the APK running on the phone is byte-identical to the deliverable and carries
the production identity — criterion 2's *intent*, proven at the only place it
ultimately matters, after the criterion's literal wording proved impossible (§2).

Runtime: the process is alive (PID present, nothing in the crash buffer),
`mCurrentFocus` is `com.nordtronics.companion/.NodesActivity`, and the app
renders the dawn-pine UI — header "Nordtronics / Wildfire companion" with the
*Prototype data* badge, a "Property line / All clear" consensus card, the four
metric tiles (PM2.5 8 µg/m³, 70 °F, 41 % RH, 1/1 nodes), and the "Field nodes →
Node 01 – Healthy" row. The footer reads `1 nodes from https://api.nordtronics.io/v1/nodes`,
so the release build is reaching the deployed backend, not a stub.

Two hiccups, recorded rather than smoothed over: `adb install -r` returned
`INSTALL_PARSE_FAILED_NOT_APK` against its streamed temp file before succeeding
on the retry, and `adb uninstall` returned `DELETE_FAILED_INTERNAL_ERROR` because
no build of this package was installed on the phone at all — the clean-slate
uninstall had nothing to remove, so no app data was lost. Neither changes the
verified end state. `versionCode=1`, `versionName=0.1`, `targetSdk=35`,
`minSdk=24` as the device records it; `firstInstallTime` and `lastUpdateTime` are
both `2026-10-01 10:40:18`.

## 4. Repository hygiene (criterion 3)

- No keystore, no password file and no passphrase material is staged or
  committed. `local.properties` and `keystore.properties` are gitignored.
- The signing config references two PATHS only
  (`NORDTRONICS_KEYSTORE`, `NORDTRONICS_KEYSTORE_PASSWORD_FILE`) and reads the
  password at build time from the existing 0600 file, so no secret is in any
  committed byte. CI-safety is proven rather than assumed: with neither
  variable set the build still emits `app-release-unsigned.apk`, byte-identical
  to the CI artifact; with them set it emits a signed `app-release.apk`.
- The only repo change is that two-file commit, on the task branch — nothing on
  `main` except this mailbox transition.

## 5. No CI run, and why (earned, not assumed)

No workflow on `hermes/0086-local-release-signing` matches that branch
(`android-build.yml` is push-limited to `android-toolchain-setup`;
`android-companion-v0.yml` to a listed set of `hermes/NNNN-*` branches that
excludes this one; the other two are path- or `main`-scoped), so the `run:` line
is deliberately absent. No ntfy receipt either — this is not a CI-built
artifact, so the build topic does not apply.

## 6. Stephen's remaining manual steps

1. **Uninstall** the debug build of `com.nordtronics.companion` from the phone
   once. A production key is a new app identity, so installing over the
   debug-signed build fails with `INSTALL_FAILED_UPDATE_INCOMPATIBLE`.
2. **Install** — done, over adb, at 2026-10-01 10:40 MDT (device `SM-A356U1`,
   Android 16) and verified on the device: see §3a. No manual install remains.
3. **Passphrase file disposition — decided 2026-10-01:** kept.
   `~/.config/nordtronics/backup-passphrase.pw` stays at 0600 so a future run
   can re-test recovery from this host without pulling the passphrase out of
   the password manager, which remains the off-host copy. Nothing was shredded.

## 7. Decision owed by Juno

Accept the substituted signature reference (`e4389ce4…4ddd`), or name the
artifact whose digest should be compared instead. If a signed CI artifact is
wanted, the workflow needs a signing step — that is its own task, not a
deviation this one can absorb.
