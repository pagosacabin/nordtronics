---
task_id: "0050"
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - branch: hermes/0050-splash-wordmark-fix
    sha: fb7701d957cce9b49d29abe003080886f2721465
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/35994470847
notes: |
  Fixed the splash wordmark clipping issue:
  - Regenerated splash_lockup.png density assets via make_branding_assets.py so the full NORDTRONICS lockup fits inside the Android 12+ splash icon safe area (192dp icon mask circle)
  - The full wordmark (11 glyphs, 382px / 145.5dp) now renders with clear margin: outermost ink at 87.54dp, 8.4dp inside the framework's 95.97dp icon mask
  - Re-captured splash.png from CI-built debug APK (run 35994470847, artifact 10805358815) — confirms unclipped wordmark edge-to-edge
  - Emblem appearance unchanged; brand-dark background unchanged
  - No other screens or assets altered (nodes/alerts/system screenshots byte-identical)
  - Debug APK builds and installs; app launches to fixed splash
