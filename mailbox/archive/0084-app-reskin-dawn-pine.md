---
task_id: "0084"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0084-app-reskin-dawn-pine
    sha: 3c368eeb1a4a9fa6c4747c846c3240226977c5b0
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36818994897
  - ntfy: nordtronics-build-ed05a663 @ 2026-10-01T05:19:14Z
  - files:
      - android/companion-v0/app/src/main/res/values/colors.xml
      - android/companion-v0/app/src/main/java/com/nordtronics/companion/WindowInsetsHelper.java
      - .github/workflows/android-companion-v0.yml
notes: |
  ntfy receipt: topic nordtronics-build-ed05a663 on https://ntfy.sh, HTTP 200,
  message id eKFp0nvWnoMd, timestamp 2026-10-01T05:19:14Z. The five-line body
  round-trips from the topic as five separate lines (verified by polling
  https://ntfy.sh/nordtronics-build-ed05a663/json). A first publish at
  2026-10-01T05:19:05Z (id D17sTQBwJvhU) went out with the lines collapsed into
  one: curl `-d` posts form-encoded, which dropped the newlines. Re-published
  with `--data-binary` + `Content-Type: text/plain`; eKFp0nvWnoMd is the receipt.

  Base branch: cut from hermes/0069-app-api-dns @
  2829d8f1c954221c261758491bc826955f535252 exactly as specified. That tip was
  re-checked on origin (`git ls-remote --heads origin`) before branching. main
  still carries no android/ tree, as the spec says.

  Deliverable: the nt_* token table applied verbatim to colors.xml (all 17
  values), brand_dark still #222629.

  Sweep for hardcoded colour literals over android/companion-v0/app/src/main
  (java + res): none found. Every layout, drawable and Java call site already
  goes through the nt_* tokens (`@color/nt_*` in XML, `Ui.col(this,
  R.color.nt_*)` in Java), so no literal needed routing and no new token was
  added. The one hex string outside colors.xml is a comment in
  WindowInsetsHelper.java:53 that named the old background value #121718 while
  documenting the dark-first choice; it was reworded to reference the nt_bg
  token (#0B120F) so the old-palette grep comes back clean. Comment-only, no
  code path change; declared because it is an edit beyond the token table.

  Verification of the artefact, not of the source: the CI release APK
  (companion-v0-release-apk, artifact 11142886481, sha256
  7f30e4940680b2336b2396f9d30666dc51b2f65c7a043ac1b067407828935dba) was
  downloaded and dumped with
  `aapt2 dump resources app-release-unsigned.apk`. All 17 compiled tokens match
  the table exactly (nt_bg #ff0b120f, nt_rail #ff101b15, nt_surface_2
  #ff1c2e28, nt_surface_3 #ff274237, nt_ink #ffede6d6, nt_muted #ffa8b09e,
  nt_line #ff3a554b, nt_accent #fff2a33c, nt_accent_strong #fff7b955,
  nt_accent_soft #ff4a3319, nt_ok #ff9dbe9c, nt_ok_soft #ff1d3324, nt_warn
  #ffe76f51, nt_warn_soft #ff46271d, nt_blue #ff86b8cc, nt_blue_soft #ff1c3038,
  nt_hero_text #ffd8d2be) and brand_dark is still #222629. A case-insensitive
  grep for the 17 old values over app/src/main returns zero matches, and over
  the compiled resource table also zero.

  Theme carry-through, confirmed rather than assumed: themes.xml and
  values-v31/themes.xml are untouched by the diff, and the compiled style shows
  Theme.Companion resolving android:windowBackground -> @color/nt_bg,
  colorPrimary / colorAccent -> @color/nt_accent, colorPrimaryDark ->
  @color/brand_dark, android:textColorPrimary -> @color/nt_ink,
  android:textColorSecondary -> @color/nt_muted. The splash attributes still
  point at @color/brand_dark, which is unchanged.

  CI: run 36818994897, conclusion success, head SHA
  3c368eeb1a4a9fa6c4747c846c3240226977c5b0 = the live branch tip
  (`git ls-remote --heads origin hermes/0084-app-reskin-dawn-pine`). Job "build"
  green on every step, including "Assemble debug APK", "Assemble release APK
  (unsigned)" and both uploads. Artifacts: companion-v0-release-apk
  11142886481, companion-v0-debug-apk 11142781981, companion-v0-screenshots
  11142811876.

  Scope extension (one line): .github/workflows/android-companion-v0.yml gained
  hermes/0084-app-reskin-dawn-pine in on.push.branches. That list is an
  allow-list and the workflow also has a paths filter, so without the edit the
  push starts no run at all and criterion 3 would have no attainable proof. The
  workflow file is itself inside the workflow's paths, so the edit is what
  triggered the run that is cited above.

  Diff is 3 files, 24 insertions / 19 deletions against the 0069 tip: no
  behaviour, API, layout, string, signing, keystore or version change.
---

# 0084 — Re-skin the companion app to the dawn-pine palette

## Context

- Stephen's call, 2026-09-30: the app (like the website) reads "drab and
  evil" — neutral near-black plus amber, nothing else. He approved the
  "dawn in the pines" palette from Juno's mockup: darks retinted pine-green,
  bone text, solar amber kept as the brand/action color, sage for healthy
  status, muted desert-sky for data, terracotta warn so alerts stop reading
  as brand amber. Task 0083 applies this same table to the website; the
  table is duplicated in full in both specs so each stands alone.
- The app lives at `android/companion-v0/` — and ONLY on branch
  `hermes/0069-app-api-dns`, tip `2829d8f1c954221c261758491bc826955f535252`
  (Juno verified 2026-09-30). The `android/` tree does not exist on `main`
  yet, so this task branches from the 0069 tip, not from `main`.
- The palette lives in
  `android/companion-v0/app/src/main/res/values/colors.xml` as `nt_*`
  colors, currently byte-identical values to the website tokens — the two
  surfaces share one palette by design; keep it that way.
  `values/themes.xml` references the tokens (`@color/nt_bg`, `@color/nt_ink`,
  etc.), so the change should carry through the theme without theme edits —
  confirm that rather than assuming it.
- `brand_dark` (#222629) in the app `colors.xml` is sampled from the logo
  artwork plate and feeds the launcher-icon and splash backgrounds. It
  stays unchanged in this task — a logo refresh is a separate job.

## Task

One deliverable: branch `hermes/0084-app-reskin-dawn-pine`, based on
`hermes/0069-app-api-dns` @ `2829d8f1c954221c261758491bc826955f535252`, with:

1. The token table below applied exactly to `colors.xml` (`brand_dark`
   excluded — unchanged).
2. A sweep of `android/companion-v0/app/src/main` (java + res) for hardcoded
   color literals outside `colors.xml`: any found get routed through the
   existing `nt_*` tokens. If a found literal has no matching token, add
   one token to `colors.xml` and report it in `notes`.

Token table (old → new):

- `nt_bg`: #121718 → #0B120F
- `nt_rail`: #171D1E → #101B15
- `nt_surface_2`: #242D2D → #1C2E28
- `nt_surface_3`: #2C3535 → #274237
- `nt_ink`: #F2F4EF → #EDE6D6
- `nt_muted`: #AAB3B0 → #A8B09E
- `nt_line`: #394342 → #3A554B
- `nt_accent`: #FF9E36 → #F2A33C
- `nt_accent_strong`: #FFB35E → #F7B955
- `nt_accent_soft`: #4B3321 → #4A3319
- `nt_ok`: #65C99B → #9DBE9C
- `nt_ok_soft`: #203D31 → #1D3324
- `nt_warn`: #FF9B6E → #E76F51
- `nt_warn_soft`: #4B2C24 → #46271D
- `nt_blue`: #74BBD1 → #86B8CC
- `nt_blue_soft`: #213C45 → #1C3038
- `nt_hero_text`: #CBD1CF → #D8D2BE
- `brand_dark`: #222629 → UNCHANGED

## Success criteria

1. `colors.xml` `nt_*` values match the table exactly; `brand_dark` still
   holds #222629.
2. A case-insensitive grep over `android/companion-v0/app/src/main` for the
   old palette values (#121718, #171D1E, #242D2D, #2C3535, #F2F4EF,
   #AAB3B0, #394342, #FF9E36, #FFB35E, #4B3321, #65C99B, #203D31, #FF9B6E,
   #4B2C24, #74BBD1, #213C45, #CBD1CF) returns matches only inside
   `colors.xml` comments, if anywhere — and no hardcoded hex color literals
   remain in java/res outside `colors.xml`. Any exception is listed in
   `notes` with file and line.
3. The release build is green in GitHub Actions on
   `hermes/0084-app-reskin-dawn-pine`, and the run's head SHA equals the
   branch tip SHA in `proof`.
4. Build-green ntfy published to topic `nordtronics-build-ed05a663` with
   the publish receipt (topic + timestamp) recorded in `notes`, per the
   mailbox README build-task rule.
5. The diff touches `colors.xml`, plus only the files needed to replace
   hardcoded literals with token references — no behavior, API, layout, or
   string changes.

## Constraints

- Base branch and new branch exactly as named above. Include no changes
  under `android-hermes-test/` (that is the old toolchain stub, not the app).
- `brand_dark` stays #222629; include no signing, keystore, or version
  changes.
- Use the exact hex values in this spec; introduce no additional colors
  beyond a reported sweep token if one is genuinely needed.
- Cost: standard worker tier per current config.

## Proof

- Stage with `proof` pointers: branch `hermes/0084-app-reskin-dawn-pine`,
  tip SHA, and the green Actions run URL whose head SHA matches the tip.
  Juno verifies all three against the live API, plus the `colors.xml`
  values at the branch tip and the ntfy receipt in `notes`.

## Reply format

Move this file to `mailbox/staged/` with `status: staged` and the `proof`
list filled in. In `notes`: the ntfy receipt, any sweep literals found and
where they were routed, and any deviation from the table — stated plainly.
