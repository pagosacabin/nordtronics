# 0071 — Sign the 0069 release APK and install on test device

expect-reply-within: 6h

## Context

Task 0069 built the release APK on branch `hermes/0069-app-api-dns`
(tip `2829d8f1c954221c261758491bc826955f535252`, CI run 36294156654 green).
The artifact `companion-v0-release-apk` (id 10923476721) contains
`app-release-unsigned.apk` — it points at the live production API
(`https://api.nordtronics.io`) but is unsigned, so Android will not install
it. Juno's VM has no Java/Android SDK, so signing must happen on your
machine, which has the SDK build-tools. Model: deepseek-flash. Report the
model used in your reply.

## Task

One deliverable: a signed, installable release APK from the 0069 build,
installed on the connected Android test device (or handed to Stephen as a
file if no device is attached).

## Success criteria

- Download the `companion-v0-release-apk` artifact from run 36294156654
  (or rebuild `assembleRelease` from the 0069 tip — either is fine, say which).
- Generate a fresh TEST keystore with `keytool` (RSA 2048+, validity 25+
  years). This is a throwaway test key, not Stephen's future production
  release key — name the alias accordingly (e.g. `nordtronics-test`).
- `zipalign` then `apksigner sign --ks <test keystore>` the APK, then
  `apksigner verify --print-certs` — the verify must pass.
- If an Android device is attached via adb: `adb install` the signed APK and
  confirm the package installs (report the package name and install result).
  If no device is attached: place the signed APK at a stable path on the
  machine and report the exact path so Stephen can grab it.
- Launch the app once if a device is attached and report what the node list
  shows against the live API (nodes visible? any error text?). Do not
  troubleshoot the backend in this task — just report what you see.

## Constraints

- Do NOT commit the keystore, its passwords, or the signed APK to the repo.
  Report the keystore path and alias in the reply notes; never put passwords
  in the reply or the repo.
- Do NOT create or claim a production release key. The production keystore
  is Stephen's decision, later.
- The unsigned APK stays as CI built it; you are only adding the signature.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- `apksigner verify --print-certs` output (paste it — this is the signature
  evidence, not a summary).
- `adb install` result line, or the exact filesystem path of the signed APK.
- Keystore path + alias (no passwords).

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field stating the
keystore location/alias and what the app showed on launch (if tested).
