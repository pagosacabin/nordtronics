---
task_id: "0084"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
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
