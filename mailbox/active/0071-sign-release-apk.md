---
task_id: "0071"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
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
