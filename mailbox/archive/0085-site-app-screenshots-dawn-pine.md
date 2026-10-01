---
task_id: "0085"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0085-site-app-screenshots-dawn-pine
    sha: 98ff1bcccd432d9d03a3df4a5b1199f60da5a443
  - run: none — no workflow triggers on this branch (triggers enumerated in notes)
  - files:
      - website/screenshots/v01-nodes.png
      - website/screenshots/v01-node-detail.png
      - website/screenshots/v01-alerts.png
notes: |
  CAPTURE SOURCE, PER FILE — fresh emulator captures (the task's option 2), taken
  from the re-skinned app build, on an API 35 / pixel_7 emulator (1080x2400):
    - website/screenshots/v01-nodes.png       <- NodesActivity, mock node list loaded
    - website/screenshots/v01-node-detail.png <- NodeDetailActivity, reached by
      tapping the node-01 row on the nodes screen (topResumedActivity confirmed
      com.nordtronics.companion/.NodeDetailActivity)
    - website/screenshots/v01-alerts.png      <- AlertsActivity, reached by tapping
      the Alerts nav item (topResumedActivity confirmed .../.AlertsActivity)

  APP BUILD THE CAPTURES CAME FROM: run 36818994897, workflow "Android Companion
  v0", headBranch hermes/0084-app-reskin-dawn-pine, headSha
  3c368eeb1a4a9fa6c4747c846c3240226977c5b0 (re-checked with `gh run view --json
  headSha,conclusion` -> success). Installed from that run's
  `companion-v0-debug-apk` artifact, app-debug.apk sha256
  b1ea199986b9a059e46b778e794a9bb797854b0464ff7373ce890b63aba921ec. The CI debug
  APK was used deliberately instead of a local gradle build: on this 7 GB host a
  concurrent gradle + emulator run gets the qemu process OOM-killed. The build
  served the app through `android/companion-v0/mock-server/server.py` at the same
  revision on 127.0.0.1:8000 (the debug build type dials 10.0.2.2:8000), on AVD
  `insets35` (system-images;android-35;google_apis;x86_64, pixel_7, 1080x2400
  @420dpi) with `adb shell svc wifi disable` so the virtio default route cannot
  swallow 10.0.2.2. One first launch rendered "Request failed: Failed to connect
  to /10.0.2.2:8000" because the guest network had not validated yet; the app was
  force-stopped and relaunched and the captures are from that second launch, after
  the mock answered 200 on /api/nodes.

  WHY OPTION 1 WAS REJECTED (the artifact's own screenshots), with evidence: the
  `companion-v0-screenshots` artifact from run 36818994897 holds 13 PNGs, and
  neither group is a new-palette capture of the site's three screens.
    - Its three site-named files (v01-nodes.png, v01-node-detail.png,
      v01-alerts.png) are byte-identical by sha256 to the CURRENT website files
      (e.g. v01-nodes.png 051490f187d59508... on both sides): they are the
      old-palette captures already on the site, committed by 0049 and untouched by
      0084 (`git log 3c368ee --not main -- android/companion-v0/screenshots` names
      0cc5e3d "0049: v0.1 screen screenshots" as the newest commit touching them).
    - Its other screen-named files (nodes.png, node-detail.png, alerts.png) are
      1080x2340 with a LIGHT background — e.g. nodes.png is 77.8 % pixels in the
      #F8F0F0 family — i.e. the 0045-era light-theme captures at pixel_5 framing,
      not comparable to the site's dark 1080x2400 v01 screens the page embeds.

  VERIFICATION PERFORMED (numeric, per pixel; nothing here is eyeballed):
    1. Palette, exact-RGB census over every pixel of each replacement: all flat
       colours rendered are dawn-pine tokens (e.g. v01-nodes.png carries nt_bg
       #0B120F 1,111,278 px, nt_surface_2 #1C2E28 895,653 px, nt_rail #101B15
       415,678 px, nt_ok #9DBE9C 597 px, nt_accent #F2A33C 512 px), and ZERO pixels
       equal any of the 17 old-palette tokens (old #121718 / #242D2D / #FF9E36 /
       #65C99B / #F2F4EF / ... all 0) in all three files.
    2. Dimensions: each replacement is 1080x2400 — identical to the file it
       replaces. No markup change is needed and none was made; index.html refers
       to the screenshots by filename only (lines 637 / 641 / 645).
    3. Same screens, comparable framing: XOR of the non-background masks of each
       replacement against the file it replaces gives nodes 2,969 differing px of
       2,592,000 (0.11 %), node-detail 6,036 (0.23 %), alerts 2,005 (0.08 %). The
       only band contributing more than 200 differing pixels in any single row is
       node-detail rows 577-587: one text row spanning x 59..965 with 1,746
       new-only vs 1,750 old-only ink pixels — the same string rendered in the new
       ink colour, whose contrast straddles the diff threshold. No row of content
       is added or removed in any of the three.
    4. In place on the page: the branch worktree was served over http and the page
       rendered headless (1280 px wide). The screenshot gallery region (y
       1900-2160 of the render) carries 25,617 exact dawn-pine token pixels and 0
       old-palette pixels, while the same region of the pre-change page carries
       312,443 old-palette and 0 new-palette pixels. The three PNGs served by the
       branch page hash equal to the committed branch blobs.
    5. Diff scope: `git diff --stat 2f8ef8008696c20c2363217af4af51800b168bf3` names
       exactly the three PNGs (190,587 -> 193,814 / 197,492 -> 201,531 /
       198,589 -> 201,482 bytes). website/index.html is byte-identical to the 0083
       tip: sha256 92ac646ef62bffda3bd90379b3ecf21825b7c8d3c121fc6b1f79f11066a257d8
       on both sides (sha256sum of the worktree file vs `git show 2f8ef80:...`).

  CI: `run: none`, earned rather than assumed. Triggers enumerated from
  .github/workflows/: android-build.yml pushes on branch
  `android-toolchain-setup` only; platformio.yml on paths
  firmware/tank-monitor/**, firmware/node-v1/** and its own file; website-check.yml
  on push branches [main, hermes/0068-site-email-refresh, hermes/0070-site-rewrite]
  and PRs targeting main. This push changed website/screenshots/*.png only, so no
  workflow matches, and `gh run list --branch
  hermes/0085-site-app-screenshots-dawn-pine` returns empty. The branch was
  deliberately NOT added to website-check's branch list: success criterion 1
  requires the diff against the 0083 tip to name exactly the three screenshot PNGs,
  so a workflow edit would violate the task's own scope rule — this is the
  0082/0083 pattern the spec refers to. Juno's merge of this branch to main is what
  will start a Website Check run.

  ntfy: not applicable, and stated rather than left out — the five-line receipt is
  the convention for build tasks with a compiled artifact (the task's own
  constraint section scopes it that way). This branch produces no build and starts
  no workflow, so there is no workflow URL or artifact URL to publish.

  Base branch and constraints: cut from hermes/0083-website-reskin-dawn-pine @
  2f8ef8008696c20c2363217af4af51800b168bf3 exactly as specified (tip re-checked on
  origin before branching). Pushed only to
  hermes/0085-site-app-screenshots-dawn-pine; no merge to main, no deploy, no edit
  to website/index.html, website/assets/ or any app source (the app was installed
  from the CI artifact and never modified; the only local app tree is the cron
  host's untracked build debris under android/companion-v0/, which is not part of
  any commit here).

  Housekeeping: emulator killed and mock backend stopped at the end of the run; no
  process left running.
---

# 0085 — Refresh the website's app screenshots to the dawn-pine palette

## Context

- 0083 (verified, archived) re-skinned `website/index.html` to the dawn-pine
  palette on branch `hermes/0083-website-reskin-dawn-pine`, tip
  `2f8ef8008696c20c2363217af4af51800b168bf3`. The three app screenshots the
  site embeds — `website/screenshots/v01-nodes.png`,
  `website/screenshots/v01-node-detail.png`,
  `website/screenshots/v01-alerts.png` — still show the OLD app palette.
- 0084 (verified, archived) re-skinned the app itself on branch
  `hermes/0084-app-reskin-dawn-pine`, tip
  `3c368eeb1a4a9fa6c4747c846c3240226977c5b0`; its green CI run
  https://github.com/pagosacabin/nordtronics/actions/runs/36818994897
  produced a `companion-v0-screenshots` artifact alongside the release APK.
- Stephen's call, 2026-09-30: HOLD the website deploy until the palette and
  the screenshots land together. This task prepares that combined state. It
  does not deploy anything.

## Task

One deliverable: branch `hermes/0085-site-app-screenshots-dawn-pine`,
branched from `hermes/0083-website-reskin-dawn-pine` @
`2f8ef8008696c20c2363217af4af51800b168bf3`, in which exactly the three
screenshot files above are replaced with new-palette captures of the same
three screens (nodes list, node detail, alerts) from the re-skinned app
build (0084 branch / its release APK).

Capture source, in order of preference:

1. The `companion-v0-screenshots` artifact from run 36818994897, if it
   contains the same three screens in comparable framing.
2. Fresh captures through the existing emulator screenshot pipeline, if the
   artifact does not cover all three screens comparably.

State in `notes` which source was used, per file.

## Success criteria

1. The branch diff against `2f8ef8008696c20c2363217af4af51800b168bf3` names
   exactly the three screenshot PNGs — no other files, and `index.html` is
   byte-identical to the 0083 tip.
2. Each replacement shows the corresponding screen rendered in the
   dawn-pine palette (ink-pine background #0B120F family, sage #9DBE9C
   status, amber #F2A33C accents) — not the old palette.
3. Replacement dimensions match the current files' pixel dimensions; if a
   capture's dimensions genuinely differ, say so in `notes` with the new
   dimensions and confirm the page needs no markup change (it references
   the screenshots by filename only).
4. `notes` states the capture source per file (CI artifact name or fresh
   emulator capture) and the app build the captures came from (branch +
   SHA or run URL).

## Constraints

- Push only to branch `hermes/0085-site-app-screenshots-dawn-pine`.
- Include no merge to `main` and no deploy — the go-live is Stephen's
  separate call after Juno verifies this branch.
- Include no edits to `website/index.html`, `website/assets/`, or any app
  source.
- Cost: standard worker tier per current config.

## Proof

- Stage with `proof` pointers: branch
  `hermes/0085-site-app-screenshots-dawn-pine` + tip SHA + the files list.
- If an Actions run triggers on the branch, include its URL; if none
  triggers, state that plainly (the 0082/0083 pattern), never a placeholder.
- Juno verifies by fetching the three PNGs at the branch tip, confirming
  the diff scope, and rendering the branch's page to eyeball the new shots
  in place.

## Reply format

Move this file to `mailbox/staged/` with `status: staged` and the `proof`
list filled in. In `notes`: capture source per file, the app build the
captures came from, the dimensions outcome, and any deviation — stated
plainly.