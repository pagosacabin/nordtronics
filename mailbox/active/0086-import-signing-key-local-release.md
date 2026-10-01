---
task_id: "0086"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  BLOCKED — PARTIAL DELIVERY, NOT STAGED. No proof block is written below and
  this file was not moved to staged/, because two of the spec's success
  criteria cannot be met by this run. Both need Stephen.
  Model used: deepseek-flash (provider deepseek), the tier the spec named.

  BLOCKER 1 (hard, human-gated): step 1, the GPG recovery test. The spec
  requires the backup passphrase "he supplies in the session" with Stephen
  "live at the laptop", and this was an unattended cron tick with no channel
  to him. No passphrase exists anywhere on this host: ~/.hermes/.env holds no
  such key (checked, key names only), and no passphrase file exists under
  ~/.config/nordtronics/ or ~/.hermes/ (only the 0071 password file, which is
  the KEYSTORE password, not the backup passphrase). The spec forbids
  guessing or retrying against the passphrase, so the decrypt was NOT
  attempted. The recovery test therefore REMAINS UNPERFORMED — it is not
  failed, it simply has not happened.

  BLOCKER 2 (spec defect, needs a decision): success criterion 2 asks for the
  local APK's signer cert digest to equal "the CI release APK's digest from
  run 36818994897". That CI artifact HAS NO SIGNATURE, so no such digest
  exists. Downloaded artifact companion-v0-release-apk (id 11142886481,
  sha256 7f30e4940680b2336b2396f9d30666dc51b2f65c7a043ac1b067407828935dba);
  `apksigner verify --print-certs` on it returns "DOES NOT VERIFY / ERROR:
  Missing META-INF/MANIFEST.MF". CI builds the release APK unsigned by design
  (android-companion-v0.yml runs `assembleRelease` with no signingConfig and
  uploads app-release-unsigned.apk), which is exactly why 0071 signed that
  artifact locally. Substituted reference, declared as a deviation: the
  PRODUCTION identity pinned in 0071, cert SHA-256
  e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd, which is
  also the digest of the keystore on this host and of the 0071 signed APK.
  Local APK digest == that value, so the app identity does match; the literal
  comparison the spec asks for is impossible rather than unmet.

  COMPLETED AND VERIFIED THIS RUN (steps 2-5):
  - Step 2: the production keystore was already outside the repo from 0071 -
    ~/.config/nordtronics/nordtronics-release.jks (0600, PKCS12, alias
    nordtronics-release). Nothing was copied, moved or re-imported, because
    the decrypt that would have supplied a fresh copy did not run.
  - Step 3: committed on branch hermes/0086-local-release-signing @
    ee96e07cfe001adc2065c7b38370fad78945df0c (two files: app/build.gradle,
    .gitignore). The signing config reads PATHS from NORDTRONICS_KEYSTORE /
    NORDTRONICS_KEYSTORE_PASSWORD_FILE (or a gitignored keystore.properties
    holding paths only) and reads the password itself from the 0600 file; no
    secret value is in any committed byte. CI-safety proven, not assumed: with
    neither variable set the build still emits app-release-unsigned.apk
    byte-identical to the CI artifact; with them set it emits a signed
    app-release.apk. .gitignore covers keystore.properties.
  - Step 4: built locally from the 0084 tip 3c368eeb1a4a9fa6c4747c846c3240
    226977c5b0. The local unsigned release APK came out BYTE-IDENTICAL to the
    CI artifact (7f30e494...), so the signed APK is provably built from the
    same bytes CI produced for that revision.
  - Step 5: signature proven on both local signed APKs.

  EVIDENCE (paths, sizes, public digests only):
  - unsigned input  /home/astroboy/nordtronics-release/0086/app-release-unsigned-0084.apk
      2,776,917 B  sha256 7f30e4940680b2336b2396f9d30666dc51b2f65c7a043ac1b067407828935dba  (= CI artifact)
  - signed (gradle step-3 path) /home/astroboy/nordtronics-release/0086/
      nordtronics-companion-v0-0084-release-signed.apk
      2,785,109 B  sha256 fa15cfdc7708f5cec0d41d8e911fda29c0d6bb666c36e64fcfd97e3e6954d1f6
      apksigner: Verifies; v2 true (v1 false is correct at minSdk 24); zipalign -c -P 16 4 OK
      cert SHA-256 e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
  - signed (0071 apksigner recipe, v2+v3, has .idsig) .../nordtronics-companion-v0-0084-release-signed-apksigner.apk
      2,826,580 B  sha256 b46dd91a91c59856dab0f7b4be76694876c14240c38da6c9f7fdebefab12811b
      cert SHA-256 e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd (identical)

  REQUESTED MANUAL STEPS FOR STEPHEN:
  1. Uninstall the debug build of com.nordtronics.companion from the phone
     once (a production key is a new app identity; installing over a
     debug-signed build fails with INSTALL_FAILED_UPDATE_INCOMPATIBLE).
  2. Install /home/astroboy/nordtronics-release/0086/
     nordtronics-companion-v0-0084-release-signed.apk (adb install, or copy it
     to the phone).
  3. Supply the backup passphrase in an interactive session so the GPG
     recovery test can be run and this task staged.

  DECISIONS OWED by Juno: (a) confirm the substituted signature reference is
  acceptable, or name the artifact whose digest should be compared; (b) after
  Stephen's passphrase step, this file can be staged with the two cert
  digests, the APK path/size and the branch pointer below.

  Deviations, all deliberate and stated: signing config added as a COMMIT on
  the spec's branch (step 3 asks for exactly that; the spec allows the push);
  the signature reference substituted (BLOCKER 2); a second signed APK
  produced via the 0071 apksigner recipe as a cross-check; the build ran
  `--offline` against the local Gradle cache; no emulator install/launch was
  performed this run (not required by the spec - the install is Stephen's
  manual step - and claimed nowhere).
---

# 0086 — Import the production signing key locally; build an installable signed release APK

## Context

- Stephen's phone runs the debug build (Android debug key). The release
  build is signed with the production key, so Android refuses to install
  release over debug (signature mismatch), and a locally built release APK
  is not production-signed at all. Stephen, 2026-10-01: getting the
  properly signed app onto his phone is today's priority.
- The production keystore exists as an encrypted backup on the VPS:
  `/home/deploy/backups/nordtronics-release-key-backup-2026-09-27.tar.gz.gpg`.
  The passphrase lives in Stephen's password manager. It has never been
  recovery-tested — this task's decrypt step IS that test.
- Current app source: branch `hermes/0084-app-reskin-dawn-pine` @
  `3c368eeb1a4a9fa6c4747c846c3240226977c5b0` (companion-v0, dawn-pine
  palette). The CI release APK from run 36818994897 is the reference for
  the production signature.

## Task

One deliverable: a locally built, production-signed release APK of the
0084 app source, proven signed with the same key as the CI release APK,
with the key imported in a way that keeps every secret out of git.

Steps:

1. With Stephen live at the laptop: fetch the encrypted backup from the
   VPS and decrypt it using the passphrase he supplies in the session.
   Report plainly whether the decrypt succeeded (this is the recovery
   test). If it fails, STOP and report — no guessing, no retries against
   the passphrase.
2. Install the keystore OUTSIDE the repo working tree (e.g. under
   `~/.android/`). Record its path in `notes`; never its contents.
3. Configure the local release signing in `android/companion-v0` so the
   keystore path and passwords come from environment variables or a
   gitignored local properties file. Anything committed to git may
   reference those variables only — no keystore file, no passwords, no
   passphrase in any committed file. Confirm `.gitignore` covers the local
   properties file before building.
4. Build the signed release APK locally from the 0084 branch source.
5. Prove the signature: run `apksigner verify --print-certs` (or aapt2
   equivalent) on the local APK and on the CI release APK from run
   36818994897, and compare the signer certificate SHA-256 digests. They
   must match. Report both digests (certificate digests are public —
   they are printed on every signed APK).

## Success criteria

1. Decrypt outcome reported truthfully (success = recovery test passed).
2. Local release APK built; its signer cert SHA-256 digest equals the CI
   release APK's digest from run 36818994897.
3. `git status` on the repo shows no keystore, properties-with-secrets, or
   passphrase material staged or committed; the only repo changes, if any,
   are signing-config references to external variables.
4. `notes` gives the APK's local path and size, the keystore's path
   (outside the repo), and both cert digests.
5. Stephen's remaining manual steps are stated in `notes`: uninstall the
   debug app, then install this APK.

## Constraints

- Secrets (passphrase, keystore passwords, keystore file) never enter
  git, GitHub, chat, or logs. The passphrase step happens only with
  Stephen live at the laptop.
- Include no changes to app source, colors, or API configuration — this
  is signing and build plumbing only.
- Push nothing to any branch unless a signing-config commit is needed; if
  one is, use branch `hermes/0086-local-release-signing` and list it in
  `proof`. The APK itself is a local artifact, not a git artifact.
- Cost: standard worker tier per current config.

## Proof

- Stage with `proof` pointers: the two signer cert SHA-256 digests
  (local APK and CI run 36818994897 APK) shown equal, the APK local path
  + size, and a branch + SHA only if step 3 required a commit.
- Juno verifies by re-downloading the CI release APK, comparing its cert
  digest against the staged value, and checking that no secret-bearing
  file appears in the repo tree.

## Reply format

Move this file to `mailbox/staged/` with `status: staged` and the `proof`
list filled in. In `notes`: decrypt outcome, keystore path, APK path and
size, both cert digests, Stephen's remaining manual steps, and any
deviation — stated plainly.


---

# Reply — Hermes → Juno — **BLOCKED, not staged**

**Model used: `deepseek-flash` (provider `deepseek`).**

No `proof:` block is written and this file stays in `mailbox/active/` with
`status: in_progress`, because criterion 1 (the decrypt recovery test) could
not be attempted without Stephen and criterion 2 names a CI artifact that has
no signature to compare against. Everything mechanically executable **was**
done and verified, and the deliverable APK exists:

```
/home/astroboy/nordtronics-release/0086/nordtronics-companion-v0-0084-release-signed.apk
2,785,109 bytes   sha256 fa15cfdc7708f5cec0d41d8e911fda29c0d6bb666c36e64fcfd97e3e6954d1f6
cert SHA-256      e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd
```

## 1. The decrypt (step 1) — NOT performed, not failed

The spec puts this step "with Stephen live at the laptop" using the
passphrase "he supplies in the session". This run was an unattended cron
tick; there is no channel to Stephen and no passphrase on this host (checked:
`~/.hermes/.env` has no such key; no passphrase file under
`~/.config/nordtronics/` or `~/.hermes/` — the only file there is the 0071
keystore password, which is a different secret). The spec's own rule is
"no guessing, no retries against the passphrase", so the decrypt was not
attempted. **The recovery test has therefore still never been run** — please
run it in an interactive session; that single step is what parks this task.

## 2. Criterion 2 as written is unsatisfiable — the CI APK is unsigned

```
$ gh api .../actions/artifacts/11142886481/zip > art.zip && unzip art.zip
$ sha256sum app-release-unsigned.apk
7f30e4940680b2336b2396f9d30666dc51b2f65c7a043ac1b067407828935dba

$ apksigner verify --print-certs app-release-unsigned.apk
DOES NOT VERIFY
ERROR: Missing META-INF/MANIFEST.MF
```

Run 36818994897 (conclusion `success`, headSha
`3c368eeb1a4a9fa6c4747c846c3240226977c5b0`, headBranch
`hermes/0084-app-reskin-dawn-pine`) uploads `app-release-unsigned.apk`; the
workflow declares no signing config. There is no CI cert digest to equal.

**Substituted reference** (declared as a deviation): the production identity
pinned by 0071,
`e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd`. The
local signed APK's cert digest is that value, and so is the digest of
`~/.config/nordtronics/nordtronics-release.jks` (read with `keytool -list -v`)
and of 0071's signed APK. Same key, three ways.

## 3. What was completed (steps 2-5)

| Step | Result |
|---|---|
| 2. Keystore outside the repo | Already in place from 0071: `~/.config/nordtronics/nordtronics-release.jks`, 0600, PKCS12, alias `nordtronics-release`. Nothing copied or re-imported (the decrypt that would have produced it did not run). |
| 3. Local release signing config | Branch `hermes/0086-local-release-signing` @ `ee96e07cfe001adc2065c7b38370fad78945df0c`, two files: `android/companion-v0/app/build.gradle`, `android/companion-v0/.gitignore`. |
| 4. Local release build | `./gradlew assembleRelease` from the 0084 tip; unsigned output byte-identical to the CI artifact. |
| 5. Signature proof | `apksigner verify --print-certs` on both local signed APKs → cert SHA-256 `e4389ce4…4ddd`. |

**The build reproduces CI exactly:** the locally assembled unsigned release
APK hashes to `7f30e494…` — the same sha256 as the artifact from run
36818994897. So the signed APK is the 0084 app, not a lookalike; separately
confirmed by reading the packaged resources (`aapt2 dump resources` shows the
`nt_*` dawn-pine palette, e.g. `colorAccent=@color/nt_accent`).

**The signing config carries no secret.** It reads two PATHS
(`NORDTRONICS_KEYSTORE`, `NORDTRONICS_KEYSTORE_PASSWORD_FILE`, or the same keys
in a gitignored `keystore.properties`) and reads the password at build time
from the existing 0600 file, so the password stays in the one place 0071 put
it. CI-safety was proven rather than assumed — two builds from the same tree:

```
# no signing env  -> app-release-unsigned.apk  sha256 7f30e494…   (== CI, unchanged)
# signing env set -> app-release.apk           sha256 fa15cfdc…   signed, cert e4389ce4…
```

`zipalign -c -P 16 4` → `Verification succesful` on the gradle-signed APK.
`v1 scheme: false` is correct at minSdk 24, not a defect.

## 4. Repository hygiene (criterion 3)

- `git status` on the repo shows **no keystore, no password file, no
  passphrase material** staged or committed. The only untracked entries are
  pre-existing leftovers (`android/` with a stale `companion-v0` shell,
  `hardware/solar-gate-v2/`, `take_screenshots.py`); none contains a secret.
- The signing change is two files on the task branch and nothing on `main`.
- No file in the tree names a keystore path, a password, or the backup
  passphrase; `local.properties` and `keystore.properties` are gitignored.

## 5. No CI run, and why (earned, not assumed)

Workflows on the pushed commit: `android-build.yml` (push limited to branch
`android-toolchain-setup`), `android-companion-v0.yml` (push limited to a
listed set of `hermes/NNNN-*` branches that does not include
`hermes/0086-local-release-signing`), `platformio.yml` (`firmware/` paths),
`website-check.yml` (`main` + two named branches). None matches this branch,
and empirically `gh run list --branch hermes/0086-local-release-signing` is
empty — so `run:` is deliberately absent. No ntfy receipt either: this is not
a CI-built artifact, so the build topic does not apply.

## 6. Stephen's remaining manual steps

1. **Uninstall the debug build** of `com.nordtronics.companion` from the phone
   once — a production key is a new app identity, so installing over the
   debug-signed build fails with `INSTALL_FAILED_UPDATE_INCOMPATIBLE`.
2. **Install**
   `/home/astroboy/nordtronics-release/0086/nordtronics-companion-v0-0084-release-signed.apk`
   (adb install, or copy the file across). 2,785,109 bytes; the `.idsig`
   sidecar next to it came from the alternate apksigner build.
3. **Give the backup passphrase** in an interactive session so the GPG
   recovery test can be run.

## 7. Decisions owed

- (a) Accept the substituted signature reference (`e4389ce4…4ddd`) or name the
  artifact whose digest should be compared instead.
- (b) Once (3) above happens, this file stages with the two cert digests, the
  APK path/size, and the branch pointer
  `hermes/0086-local-release-signing` @
  `ee96e07cfe001adc2065c7b38370fad78945df0c`.

## 8. Deviations

1. Signing config committed on the spec's branch (step 3 requires it; spec
   permits the push). Files: `app/build.gradle`, `.gitignore`. No app source,
   colour or API change — `git diff` on the branch is build plumbing only.
2. Signature reference substituted (section 2).
3. A second signed APK produced via the 0071 apksigner recipe as a
   cross-check (`…-apksigner.apk`, v2+v3, sha256 `b46dd91a…`), same cert.
4. Build ran `--offline` against the local Gradle cache (no network needed).
5. No emulator install or app launch this run — the install is Stephen's
   manual step per the spec, and no install claim is made anywhere above.
6. The recovery test was not attempted (section 1) — recorded as unperformed,
   not as failed.
