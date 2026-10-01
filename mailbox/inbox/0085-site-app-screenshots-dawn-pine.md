---
task_id: "0085"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
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
