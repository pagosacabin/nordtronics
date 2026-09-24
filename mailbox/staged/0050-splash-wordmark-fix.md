---
task_id: "0050"
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - branch: hermes/0050-splash-wordmark-fix
    sha: fb7701d957cce9b49d29abe003080886f2721465
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35998472510
  - files: android/companion-v0/screenshots/splash.png, android/companion-v0/tools/make_branding_assets.py, android/companion-v0/app/src/main/res/drawable-*/splash_lockup.png
  - ntfy: topic nordtronics-build-ed05a663, id 6IiKBX3dF6fY, published 2026-09-24 (epoch 1790285329)
notes: |
  Re-filed 2026-09-24 by Hermes (cronrunner) after Juno's rejection at ~11:05 AM MDT. Previous staging was filed without the reply: front-matter still read status: in_progress and the proof block was missing. Now filed per 0052 reply discipline. What changed: splash_lockup.png density assets regenerated so the full NORDTRONICS wordmark fits inside the Android 12+ splash icon safe area (previously cropped to read "IORDTRONIC"); splash.png re-captured from the CI-built debug APK. Safe-area fit confirmed by measuring the lockup bounds against the splash icon slot in the rendered screenshot - wordmark readable edge to edge with visible margin left and right; emblem and brand-dark background unchanged. Note: an earlier proof pointer cited run 35994470847 at SHA e6ca755 - that was a stale commit, not the branch tip. The correct run at the branch tip is 35998472510 @ fb7701d.
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
