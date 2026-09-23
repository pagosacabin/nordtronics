---
task_id: "0046"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0046 — Companion v0 launcher icon

## Context

0045 (Companion v0) is verified and archived. Juno decompiled the CI-built APK
(run `https://github.com/pagosacabin/nordtronics/actions/runs/35899478176`,
artifact `companion-v0-debug-apk`): the manifest's `android:label` resolves to
"Nordtronics Companion", but the `<application>` element has NO `android:icon`
or `android:roundIcon` — the launcher shows the default Android robot. The
0037 manifest fix (verified, archived) established the launcher-icon pattern
for this repo; it did not carry into the new app module.

## Task

Add the Nordtronics launcher icon to the Companion v0 app module: set
`android:icon` and `android:roundIcon` on the `<application>` element, reusing
the existing launcher mipmap assets from the 0037 fix. No new artwork.

## Success criteria

- Branch `hermes/0046-companion-icon` exists on origin, cut from the
  `hermes/0045-companion-v0` tip (`9937a1654566cbae5727e1d35a857c81052970f1`).
- A green CI run builds the debug APK.
- The decompiled APK manifest shows `android:icon` and `android:roundIcon`
  set on `<application>`.
- One emulator screenshot of the app in the device launcher, visibly showing
  the Nordtronics icon, committed at
  `android/companion-v0/screenshots/launcher-icon.png`.

## Constraints

- Push only to `hermes/0046-companion-icon`. Never push to main.
- Reuse the 0037 icon assets. No new artwork, no new dependencies.
- No push notifications, no Play Store, no release signing.

## Proof

- `branch` + `sha` (branch exists on origin, tip equals `sha`).
- `run`: Actions run URL, green, head SHA matches branch tip.
- `ntfy`: build-green publish receipt — topic + timestamp. Per the mailbox
  README proof format: once the run is green, publish to the ntfy build topic
  `nordtronics-build-ed05a663` on `https://ntfy.sh` so Stephen is notified.
  A green build he never hears about is a failed handoff.

## Reply format

```yaml
proof:
  - branch: hermes/0046-companion-icon
    sha: <full tip sha>
  - run: <actions run url>
  - ntfy: <topic> @ <timestamp>
notes: <launcher screenshot path, any deviations>
```
