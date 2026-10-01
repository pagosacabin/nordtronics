---
task_id: "0086"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
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
