---
task_id: "0083"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: hermes/0083-website-reskin-dawn-pine
  sha: 2f8ef8008696c20c2363217af4af51800b168bf3
  run: "none — no workflow in this repo triggers on this branch or on website/ paths; enumeration below plus a live `gh run list --branch` returning empty"
  files:
    - website/index.html
  htmlhint_local: "npx htmlhint@1.1.4 website/index.html --config .htmlhintrc -> 'Scanned 1 files, no errors found' (exit 0), i.e. the exact CI validation step, run locally because CI does not trigger on this branch"
notes: |
  Palette applied exactly as specified; no deviation from the token table, the
  three gradient stop pairs, or the two rgba shadows.

  Deliverable: website/index.html at branch tip
  2f8ef8008696c20c2363217af4af51800b168bf3 (verified with
  `git ls-remote --heads origin hermes/0083-website-reskin-dawn-pine`).
  One commit on the branch: "0083: re-skin nordtronics.io to the dawn-pine palette".

  Criteria checks, all run against the branch tip file:
  1. :root holds all 18 new token values (lines 16-33).
  2. `grep -inE '#121718|#171D1E|#242D2D|#2C3535|#F2F4EF|#AAB3B0|#394342|#FF9E36|
     #FFB35E|#4B3321|#65C99B|#203D31|#FF9B6E|#4B2C24|#74BBD1|#213C45|#CBD1CF|
     #222629|#9CDDE8|#8AE5B8|255,158,54' website/index.html` returns zero matches
     (grep exit 1).
  3. grad-accent #F2A33C->#F7B955, grad-blue #86B8CC->#B7D6E2,
     grad-ok #9DBE9C->#C4D8BE.
  4. `header::before` background stack gained the dawn layer
     `linear-gradient(180deg, rgba(134,184,204,0.14) 0%, rgba(134,184,204,0) 55%),`
     as its first line; the two radial layers are byte-identical otherwise.
  5. Diff is 24 insertions / 23 deletions, all in website/index.html. Proven
     value-only by normalising every removed line through the old->new palette
     map and requiring it to equal an added line: all 23 removed lines map onto
     23 of the added lines, and the single unmatched added line is the new dawn
     layer. No copy, layout, link, or structure line moved; nothing under
     website/assets/ and no new CSS/JS files (git diff --stat names one file).

  Correction worth flagging for the record (found and fixed in this run, not
  shipped): my first mechanical pass replaced the literal `rgba(255,158,54`
  (without its trailing comma) and produced `rgba(242,163,60),0.15)` — a broken
  value. Both shadows were corrected to `rgba(242,163,60,0.15)` /
  `rgba(242,163,60,0.3)` before the commit; the committed file carries the
  correct comma form. The alpha values are unchanged as specified.

  CI: no run triggers on this branch, so `proof.run` is `none` rather than a
  placeholder. Enumeration: .github/workflows holds three files —
  android-build.yml (`on.push.branches: [android-toolchain-setup]`),
  platformio.yml (`on.push.paths: firmware/tank-monitor/**,
  firmware/node-v1/**, .github/workflows/platformio.yml`), and website-check.yml
  (`on.push.branches: [main, hermes/0068-site-email-refresh,
  hermes/0070-site-rewrite]`). This branch is not in the website-check list, and
  website/ is not in the platformio paths, so a push to it starts nothing.
  Confirmed live: `gh run list -R pagosacabin/nordtronics --branch
  hermes/0083-website-reskin-dawn-pine` returns empty, while the same command
  without `--branch` shows the Website Check runs on main (e.g. 36814275009),
  so the empty result is a real absence, not an auth failure.

  I did NOT add this branch to website-check.yml's trigger list: the spec states
  one deliverable (website/index.html) and criterion 5 bounds the diff to colour
  values, gradient defs and the header::before stack, so widening the trigger
  would have been an undeclared scope extension. To compensate I ran the exact
  CI validation step locally against the branch tip file
  (`npx htmlhint@1.1.4 website/index.html --config .htmlhintrc` -> no errors,
  exit 0) — that is a local run, not a CI run, and is labelled as such. If Juno
  wants a real run on the branch, adding `hermes/0083-website-reskin-dawn-pine`
  to that branch list in a follow-up commit is a one-line change.
---

# 0083 — Re-skin nordtronics.io to the dawn-pine palette

## Context

- Stephen's call, 2026-09-30: the current site colors read "drab and evil" —
  neutral near-black plus amber, nothing else. He approved the "dawn in the
  pines" direction from Juno's mockup (approved in chat 2026-09-30): keep the
  dark gradient-to-black, retint the darks pine-green, warm the text to bone,
  sage for healthy status, muted desert-sky for data, and a terracotta warn
  so a real alert no longer reads as brand amber.
- The site is one file: `website/index.html` on `main`. Every color flows
  from the `:root` token block (lines 15–34: `--nt-*` plus `--brand-dark`),
  three inline SVG gradient defs (`grad-accent`, `grad-blue`, `grad-ok`,
  ~lines 373–381), and two `rgba(255,158,54,…)` shadows (hero-logo
  drop-shadow, btn-primary hover). `website/assets/` holds only
  `favicon.png` + `og-image.png` — images are out of scope (see Constraints).
- Juno verified the current token values from `main` on 2026-09-30; the old
  values are in the table below. Task 0084 applies this same table to the
  Android app — the table is duplicated in full in both specs so each stands
  alone.

## Task

One deliverable: `website/index.html` on branch
`hermes/0083-website-reskin-dawn-pine` (branched from `main`) with the new
palette applied exactly as specified here — token values, SVG gradient
stops, rgba shadows, and one added dawn layer in the hero.

Token table (old → new):

- `--nt-bg`: #121718 → #0B120F
- `--nt-rail`: #171D1E → #101B15
- `--nt-surface-2`: #242D2D → #1C2E28
- `--nt-surface-3`: #2C3535 → #274237
- `--nt-ink`: #F2F4EF → #EDE6D6
- `--nt-muted`: #AAB3B0 → #A8B09E
- `--nt-line`: #394342 → #3A554B
- `--nt-accent`: #FF9E36 → #F2A33C
- `--nt-accent-strong`: #FFB35E → #F7B955
- `--nt-accent-soft`: #4B3321 → #4A3319
- `--nt-ok`: #65C99B → #9DBE9C
- `--nt-ok-soft`: #203D31 → #1D3324
- `--nt-warn`: #FF9B6E → #E76F51
- `--nt-warn-soft`: #4B2C24 → #46271D
- `--nt-blue`: #74BBD1 → #86B8CC
- `--nt-blue-soft`: #213C45 → #1C3038
- `--nt-hero-text`: #CBD1CF → #D8D2BE
- `--brand-dark`: #222629 → #171F1A

SVG gradient stops:

- `grad-accent`: #F2A33C → #F7B955
- `grad-blue`: #86B8CC → #B7D6E2
- `grad-ok`: #9DBE9C → #C4D8BE

rgba shadows: every `rgba(255,158,54,…)` becomes `rgba(242,163,60,…)` with
the alpha unchanged.

Hero dawn layer: in `header::before`, add as the FIRST layer of the
background stack:
`linear-gradient(180deg, rgba(134,184,204,0.14) 0%, rgba(134,184,204,0) 55%)`
The two existing radial layers keep their positions and sizes — their colors
retint on their own through `--nt-accent-soft` and `--nt-blue-soft`.

## Success criteria

1. Every token in the table holds exactly the new value in `:root`.
2. A grep of `website/index.html` for any old-palette value — #121718,
   #171D1E, #242D2D, #2C3535, #F2F4EF, #AAB3B0, #394342, #FF9E36, #FFB35E,
   #4B3321, #65C99B, #203D31, #FF9B6E, #4B2C24, #74BBD1, #213C45, #CBD1CF,
   #222629, #9CDDE8, #8AE5B8 (case-insensitive) — and for `255,158,54`
   returns zero matches.
3. The three SVG gradients hold exactly the new stop pairs listed above.
4. `header::before` gains the dawn layer as its first background layer; the
   radial layers are otherwise untouched.
5. The diff touches color values, the gradient defs, and the `header::before`
   background stack only — no copy, layout, link, or structure changes, and
   no new external CSS/JS files.

## Constraints

- Push only to branch `hermes/0083-website-reskin-dawn-pine`.
- Include no changes under `website/assets/` — favicon and og-image
  regeneration is a separate follow-up Stephen has not approved yet.
- Use the exact hex values in this spec; introduce no additional colors.
- Cost: standard worker tier per current config. This is a single-file
  value edit — keep it small.

## Proof

- Stage with `proof` pointers: branch `hermes/0083-website-reskin-dawn-pine`
  + tip SHA. Juno fetches `website/index.html` at the branch tip and checks
  the token values and old-value greps herself.
- If an Actions run triggers on the branch, include its URL. If none
  triggers, state that plainly in `proof` (the 0082 pattern), never a
  placeholder.

## Reply format

Move this file to `mailbox/staged/` with `status: staged` and the `proof`
list filled in. In `notes`: any deviation from the table or criteria, stated
plainly — a flagged deviation beats a silent one.
