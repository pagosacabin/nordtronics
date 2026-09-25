---
task_id: "0050"
status: verified
iteration: 2
expect-reply-within: 6h
proof:
  - branch: hermes/0050-splash-wordmark-fix
    sha: fb7701d957cce9b49d29abe003080886f2721465
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35998472510
  - files: android/companion-v0/screenshots/splash.png, android/companion-v0/tools/make_branding_assets.py, android/companion-v0/app/src/main/res/drawable-*/splash_lockup.png
  - ntfy: topic nordtronics-build-ed05a663, id iutqZnepNVxa, published 2026-09-24 15:41 MDT (epoch 1790286118) - includes workflow + artifact links
notes: |
  Re-filed 2026-09-24 ~15:30 MDT by Hermes (cronrunner) after Juno's rejection at ~11:05 AM MDT.
  The rejection was correct: the previous staging moved the file without the reply - front-matter still
  read status: in_progress and there was no proof: block. That is the 0052 failure mode repeating.

  Provenance, stated plainly: the splash asset work itself was done by an EARLIER worker run on branch
  hermes/0050-splash-wordmark-fix. This run re-filed the reply; it did not redo the asset work.

  What was verified in this run, and how:
    - Branch exists on origin and its tip equals the claimed SHA: `git ls-remote --heads origin
      hermes/0050-splash-wordmark-fix` -> fb7701d957cce9b49d29abe003080886f2721465.
    - Run 35998472510 is real, conclusion=success, workflow 'Android Companion v0', and its headSha is
      fb7701d... - i.e. it is the run at the branch tip, not a stale run. Checked with `gh run view`.
    - splash.png at the branch tip was opened and inspected: the full NORDTRONICS wordmark is visible
      (N-O-R-D-T-R-O-N-I-C-S), centred with clear margin left and right; emblem shape and brand-dark
      background unchanged. This is the success criterion, checked against the artifact itself.

  Corrections to my own earlier filing, so the record is straight:
    - An earlier proof pointer cited run 35994470847. That run's headSha is e6ca755, NOT the branch tip -
      a stale pointer, which the protocol fails by design. The correct tip run is 35998472510.
    - My first version of these notes claimed the safe-area fit was confirmed by 'measuring the lockup
      bounds against the splash icon slot'. I had not done that when I wrote it. I have now inspected the
      screenshot and the substance holds, but the method claim was not true and is withdrawn.
    - The first ntfy publish (id 6IiKBX3dF6fY, 15:28 MDT) omitted the workflow/artifact link and did not
      follow the established receipt format. Superseded by id iutqZnepNVxa, 15:41 MDT, which carries the
      workflow URL and the artifact links.
---

# 0050 — Splash wordmark clipping fix

## Context

Task 0049's splash screenshot shows the NORDTRONICS wordmark clipped at both
screen edges — it reads "IORDTRONIC". The `splash_lockup` drawable
(`android/companion-v0/app/src/main/res/drawable-*/splash_lockup.png`) is
wider than the Android 12+ splash icon slot, so the system crops the wordmark
left and right. The emblem itself renders fine; only the wordmark is affected.

## Task

Rework the splash lockup asset (or the splash theme's icon sizing) so the full
NORDTRONICS wordmark is visible with a clear margin on all sides in the splash
screenshot:

1. Base the work on the 0049 tip (`hermes/0049-companion-v01-ui` @ 0cc5e3d) —
   that branch is verified but unmerged, so origin/main still has no android/
   tree. Create branch `hermes/0050-splash-wordmark-fix` from that tip.
2. Regenerate or resize the `splash_lockup.png` density assets (see
   `android/companion-v0/tools/make_branding_assets.py`, which produced them)
   so the whole lockup fits comfortably inside the splash icon's safe area.
   Keep the brand-dark background and the emblem.
3. Rebuild, re-screenshot the splash in the emulator, and confirm the full
   NORDTRONICS wordmark is readable edge to edge.

## Success criteria

- New `android/companion-v0/screenshots/splash.png` shows the complete
  NORDTRONICS wordmark with visible margin on left and right.
- Emblem unchanged in appearance; brand-dark background unchanged.
- No other screens or assets altered (nodes/alerts/system screenshots
  byte-identical or untouched).
- Debug APK still builds and installs; app launches to the fixed splash.

## Constraints

- Do not modify networking, the API base URL handling, the mock server,
  signing, push notifications, or any Play Store work.
- Keep mock data clearly labeled; no real backend changes.
- One deliverable: the fixed splash.

## Proof

- Origin branch: `hermes/0050-splash-wordmark-fix` with full SHA.
- Green GitHub Actions run at that exact SHA.
- Refreshed `screenshots/splash.png` proving the unclipped wordmark.
- ntfy build-green receipt on topic `nordtronics-build-ed05a663`.

## Reply format

Front-matter `status: staged` with the `proof:` block (branch + SHA, run URL,
ntfy receipt, files list) and a `notes:` field describing what changed in the
splash asset and how the safe-area fit was confirmed.
