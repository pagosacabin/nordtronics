---
task_id: "0145"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - branch: main
    repo: pagosacabin/cabin-play
    sha: e251a1691ca6734dc315a2d1d8f4281823e07cd7
  - run: none
  - files:
      - guition-jc4827w543/jc4827w543.yaml
      - guition-jc4827w543/tools/preview.py
      - guition-jc4827w543/tools/audit_layout.py
      - guition-jc4827w543/preview-bms-lcars.png
      - guition-jc4827w543/README.md
  - ntfy: topic nordtronics-build-ed05a663, id zIJr3MMHi9nK, published 2026-10-17T00:22:22Z.
proof_note: |
  Cross-repo commit, the same shape as 0136 / 0137 / 0138 / 0140 / 0141 / 0144: the deliverable
  is a change to pagosacabin/cabin-play, so `branch`/`sha` point at the DESTINATION repo's main,
  pushed over SSH from the bench host (the github skill's token is nordtronics-scoped and 403s
  on cabin-play writes). Verified on origin, not just locally:
  `git ls-remote origin refs/heads/main` = e251a1691ca6734dc315a2d1d8f4281823e07cd7 and
  `git rev-parse HEAD` returns the same sha on a fast-forwarded main. Blob-level check after the
  push, each file re-fetched through `gh api
  repos/pagosacabin/cabin-play/contents/<path>?ref=e251a169...` and compared with
  `git rev-parse <sha>:<path>` --
  jc4827w543.yaml 2d5482ea8a35377d50e43a144c2d274c4f78342f (31117 B),
  tools/preview.py 57c85ad37b4d54e4ad8b82e56ab9204a39bfda0f (9286 B),
  tools/audit_layout.py 8f42832a943dd1f08c95a4129cccc037145b89aa (9800 B),
  README.md 7f7fb5a689cff75cea52a3c379a7bcf8a5453067 (12636 B),
  preview-bms-lcars.png 89400008e3f5fa023d1f91bc33ab61f496863c24 (32162 B) -- all five identical.
  (raw.githubusercontent.com returns 404 for these paths: cabin-play is a private repo.)

  `run: none` is earned by enumeration. cabin-play carries NO CI:
  `gh api repos/pagosacabin/cabin-play/contents/.github` -> HTTP 404 "Not Found", and
  `git ls-tree -r --name-only e251a169... | grep -c '^\.github'` -> 0. No nordtronics workflow
  covers it either: nordtronics' workflows trigger on branch allow-lists and on path filters
  under android/, backend/, firmware/ and website/, and this tree is not in nordtronics at all.
  The build evidence is therefore the local ESPHome compile plus the OTA flash quoted in the
  reply, not a run URL.

  The render is a durable layout pointer committed with the change, the way 0138 committed its
  build log: `gh api .../contents/guition-jc4827w543/preview-bms-lcars.png?ref=<sha>` returns
  blob 89400008..., 32162 bytes, equal to the local file, which is itself byte-identical to the
  0141 revision's committed PNG (sha256
  d9377704d97d6b470223ada1da5865fd5b6968848c619a707c03e170e2d1cd4e). It is a PIL mirror of the
  lambda, so it is layout evidence, not a photograph -- see the photo note in the reply.

  ntfy: topic nordtronics-build-ed05a663, id zIJr3MMHi9nK, published 2026-10-17T00:22:22Z
  (the topic 0141 / 0144 established for this artifact; 0145's spec names no topic of its own).
  `Workflow` and `Artifacts` read `none` with the reason inline, because cabin-play has no CI --
  the same shape 0138 / 0140 / 0141 used.
notes: |
  STAGED 2026-10-17 00:23 UTC / 2026-10-16 18:23 MDT. Off-peak (PEAK: OFF-PEAK 00:15 UTC).
  PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md). Gate: mailbox changed
  -> main-not-at-origin/main, inbox-non-empty. Queue state at pickup: mailbox/active/ held only
  .gitkeep; the lowest inbox task was 0142, which carries `hold_until:
  "2026-10-17T08:00:00-06:00"` and body text "Do NOT execute before Saturday 2026-10-17, 08:00
  MDT -- picking this up Friday defeats the purpose", so it was left in inbox/ untouched and
  0145 (next lowest) was picked at 18:16 MDT. 0142 is due at or after 08:00 MDT Saturday.

  ALL FOUR SUCCESS CRITERIA MET; the photograph is Stephen's step and is NOT claimed.
  Criterion 1: the whole `jc4827w543.yaml` is byte-identical to 2ee02d8, so the lambda region is
  trivially identical -- `git diff 2ee02d8 e251a169 -- guition-jc4827w543/jc4827w543.yaml`
  returns empty, and `git diff 2ee02d8 e251a169 --stat -- guition-jc4827w543/` is empty across
  the whole directory. Criterion 2: restored build compiles under ESPHome 2026.9.1, exit 0, and
  `tools/audit_layout.py` reports 42 checks / 42 passed / 0 failed, exit 0. Criterion 3: one OTA
  flash; the device's own readback moved from `compiled on 2026-10-16 17:23:46 -0600` (the 0144
  build the spec named) to `compiled on 2026-10-16 18:17:06 -0600`, read both from the ESPHome
  native API (`device_info.compilation_time`) and from the console banner, and the app is drawing
  the BMS page. Criterion 4: the palette block hashes identically in both revisions (sha256
  75656057c6760a2e5856523d870bf3bb6774300d4c57fcc76ad5d6513f5f7a6d), and the file-level identity
  makes that stronger than a block diff.

  SCOPE DECISION BEYOND THE SPEC, declared: the spec said tools/audit_layout.py "may stay -- it's
  tooling, not geometry", and I did not keep it. Both tools encode THIS layout's geometry as
  constants, so keeping the 0144 versions would have failed criterion 2 outright (0144's audit
  asserts the 0144 boxes -- "no content sits on the bottom edge", three equal stat columns -- and
  0141's arrangements violate them) and the 0144 preview would have re-rendered the 0144 page
  under a restored lambda, making the committed render a statement about the wrong layout. Both
  were therefore reverted with the lambda, together with the README's layout section and the
  committed PNG, so lambda / mirror / audit / render / docs all describe the 0141 page. The one
  tooling artifact NOT reverted is `.gitignore`'s `guition-jc4827w543/tools/audit-page.png` entry
  added in 0144: the reverted audit writes that same file, so the entry is still correct.

  PHYSICAL ACTION DECLARED: ONE OTA flash of 192.168.1.72 (`esphome upload`, port 3232), OTA
  successful in 8.76 s (prepare 0.07, upload 8.46, commit 0.23). No USB, no erase_flash, no NVS
  write, no partition change, no power cycle; nothing else on the host or the bench was touched.
  Panel cell at the time: 4.276 V, 100 %, state CHG (on the charger).

  PUSH SHAPE DECLARED: the commit was built on a temporary branch
  `hermes/0145-guition-geometry-rollback` so it could be verified before landing, then
  fast-forwarded onto main (8e5024b -> e251a16) and pushed, and the temporary branch deleted from
  origin. cabin-play main's history is the same two commits 0140/0141/0144 used, and the sha in
  `proof` is the landed main tip; the branch no longer exists on origin, so a verifier should
  check main rather than a branch name.

---

# 0145 — Guition: roll geometry back to the 0141 layout

## Context
Stephen's verdict on the 0144 geometry rework: "I like the other one better."
The 0141 layout is the one he wants on the glass. The color-pipeline artifacts
(blue/lime instead of the canonical palette) persist in the current build —
that investigation stays parked; this task is geometry only.

- 0141 geometry: cabin-play commit 2ee02d829e0f7a7dbe0e735a77b081f8fc3d2c82
  (the layout in mailbox/files/0143-panel-photo.jpg).
- 0144 geometry: cabin-play commit 8e5024b98d71f02e17d237d8462ef46ab26cb478
  (current on-panel build, "compiled on 2026-10-16 17:23:46 -0600").

## Task
Restore the BMS page display lambda's geometry to the 0141 state:
- The display lambda geometry in `guition-jc4827w543/jc4827w543.yaml` should
  match the 0141 revision (2ee02d8). Cleanest path: check out the lambda region
  from 2ee02d8, or reverse-apply 0144's lambda diff — your call, same result.
- Keep 0141's palette declarations, roboto_80 font block, and behaviors
  (NAN→"--", page-select, dimming, repaint) exactly as they are.
- tools/audit_layout.py (added in 0141, extended in 0144) may stay — it's
  tooling, not geometry. The render PNG should be regenerated from the
  restored lambda.

## Constraints
- Geometry revert only. No color/palette changes, no behavior changes.
- The restored lambda must compile clean under ESPHome 2026.9.1 with no new
  warnings, and audit_layout.py must pass on the restored arrangement.
- Push via SSH to pagosacabin/cabin-play as with 0140/0141/0144 (the gh.py
  token is nordtronics-scoped and 403s on cabin-play writes).
- Keep model/provider cost economical; no premium tier needed.

## Success criteria
1. The BMS page geometry matches the 0141 revision (prove with a diff of the
   lambda region against 2ee02d8 — expect no differences).
2. Compiles clean, audit checks pass.
3. Panel OTA-flashed; new "compiled on" timestamp reported.
4. Palette byte-identical to 0141 (unchanged by this task).

## Proof
- Commit SHA on cabin-play origin.
- Lambda-region diff against 2ee02d8 showing no differences.
- Render PNG regenerated from the restored lambda.
- New "compiled on" timestamp read back from the panel (Stephen photographs
  the glass — he's the eyes).

## Reply format
Per mailbox protocol: staged reply with status, the commit SHA, the diff
result, the new build timestamp, and notes on anything unexpected.

# Reply — 0145 staged: geometry rolled back to the 0141 layout; panel flashed, new build time read back

## Status

`staged`. Commit **`e251a1691ca6734dc315a2d1d8f4281823e07cd7`** on **main** of
`pagosacabin/cabin-play`, pushed over SSH. No CI run — see `proof_note` for the enumeration
that earns `run: none`. Layout gate: `tools/audit_layout.py`, **42 checks, 42 passed, 0
failed**, exit 0. Panel flashed once over OTA; the device's own readback confirms it is running
this revision.

## What was done

`guition-jc4827w543/` is now byte-identical to the 0141 revision `2ee02d8`. The revert was
taken by checking the panel directory's files out of `2ee02d8` and committing the result, which
is the spec's first suggested path and the one that cannot leave a half-reverted region behind:

```
$ git diff 2ee02d8 e251a169 --stat -- guition-jc4827w543/
(empty)

$ git diff 2ee02d8 e251a169 -- guition-jc4827w543/jc4827w543.yaml
(empty)
```

So the lambda region — criterion 1's literal ask — has **no differences**, and neither does any
other file in the directory. The five files that move relative to 0144's `main`:

| file | change vs 0144 |
|---|---|
| `guition-jc4827w543.yaml` | lambda geometry + its descriptive comments, back to the 0141 wording |
| `tools/preview.py` | the PIL mirror, back to 0141's geometry constants |
| `tools/audit_layout.py` | the pixel audit, back to 0141's 42 checks |
| `preview-bms-lcars.png` | regenerated render (byte-identical to 0141's committed PNG) |
| `README.md` | the Layout (0144) section and the paragraphs that described it, back to 0141's |

What came back, in the geometry's own terms: the elbow frame at y 4..263 with the header bar at
y 4..31 (0144 had moved both to an 8 px top margin), the purple rule under the header at
y 36..42, the SOC label pill at x 44, the 80 pt SOC number at (44, 76), the two-tone meter at
y 212 with its five-block footer row under it, and the three readings as 85x15 label pills at
x 250 with their values at x 342 — instead of 0144's single x = 72 content margin, SOC block and
three equal 126 px columns.

Nothing the spec froze was touched, and that is provable at file level: the palette block hashes
identically in both revisions (sha256 `75656057c6760a2e5856523d870bf3bb6774300d4c57fcc76ad5d6513f5f7a6d`
for the `const Color PEACH … INK(0, 0, 0)` block in each), the `roboto_80` font block with its
explicit `glyphs:` set is the 0141 block verbatim, and the behaviours are untouched — the `--`
NAN guards, the `low_batt_pct` page select, the inactivity dimming and the once-a-minute repaint
are all in place; the console below shows the running path, and the status-page select is still
armed by the same condition.

## Criterion 2 — compile and audit, base and revision

Both revisions were compiled in their own build tree, the restored one in the working copy and
the 0144 base in a scratch checkout of `8e5024b` (`git archive 8e5024b | tar -x`), each with
`.esphome/build/<name>/src` removed first so `main.cpp` — the translation unit the lambda lives
in — was really rebuilt rather than served from cache:

```
restored (e251a169): exit 0, 0 compiler warnings, Total image size 867523 B
base 0144 (8e5024b): exit 0, 0 compiler warnings, Total image size 867399 B
both: identical config warnings -- 2, and the same two:
  WARNING The 'qspi_dbi' component is deprecated, it is recommended to use 'mipi_spi' instead.
  WARNING esphome->platformio_options->board_build.flash_mode is ignored when building with the
          native 'esp-idf' toolchain
```

So "no new warnings" is a comparison rather than a claim: the warning *sets* are equal, and
neither build emits a C/C++ warning at all. The 124 B image delta is the 0141 glyph/font
bookkeeping, not a change in scope.

The audit, run against the regenerated render (exit 0):

```
42 checks over palette hex counts, the banned green/cyan hue families, the retired colours,
the elbow's black-blob and monotonicity tests, every label inside its pill, every reading
inside the right margin, the meter fill against 86 %, the alarm / 0 % / NAN states, and the
footer block row.
42 checks, 42 passed, 0 failed
```

## Criterion 1 — the regenerated render is the 0141 page

`tools/preview.py` was re-run against the restored lambda and the result compared with the
committed render:

```
sha256 of regenerated  : d9377704d97d6b470223ada1da5865fd5b6968848c619a707c03e170e2d1cd4e
sha256 of committed    : d9377704d97d6b470223ada1da5865fd5b6968848c619a707c03e170e2d1cd4e
sha256 of 2ee02d8's PNG: d9377704d97d6b470223ada1da5865fd5b6968848c619a707c03e170e2d1cd4e
sha256 of 8e5024b's PNG: 1ea99f3a93190a22214514c7308cb8c05cf2897568af52ad33b144e22fcdba24
```

All three of the restored render, the committed blob and 0141's PNG are the same bytes — the
mirror is deterministic, so this is evidence that the committed PNG is a render of the restored
lambda and not a stale file left behind.

## Criterion 3 — flashed, and read back from the panel

The upload, and the device's own build time before and after:

```
INFO Uploading .../jc4827w543.bin (867632 bytes)      # esphome upload, 192.168.1.72:3232
INFO Update took 8.76 seconds (prepare 0.07, upload 8.46, commit 0.23)
INFO OTA successful

# device_info over the ESPHome native API, before the flash:
compilation_time  : 2026-10-16 17:23:46 -0600        # <- the 0144 build the spec names
# ... and after:
compilation_time  : 2026-10-16 18:17:06 -0600        # <- the restored build

# console banner and steady state after the flash:
[18:20:53.844][I][app:151]: ESPHome version 2026.9.1 compiled on 2026-10-16 18:17:06 -0600
[18:20:58.227][I][safe_mode:154]: Boot seems successful; resetting boot loop counter
[18:21:04.681][I][bms:542]: page=bms soc=93 V=52.78 I=-9.6 T=67.8 local=100%
```

`18:17:06 -0600` is exactly the restored firmware's own `build_time_str` from the compile log,
so the glass is running the revision in `proof`, not 0144 and not a cached image. The photo of
the glass is Stephen's step and is not claimed here — this host has no camera that can see the
bench (re-verified in 0141: five webcam frames, 0 % of pixels in the page's dominant fills or
their complements), and the panel has no framebuffer readback.

## ntfy receipt

Topic `nordtronics-build-ed05a663`, id `zIJr3MMHi9nK`, published 2026-10-17T00:22:22Z:

```
Branch: main (pagosacabin/cabin-play)
SHA: e251a1691ca6734dc315a2d1d8f4281823e07cd7
Workflow: none
Artifacts: none
Status: success
```

`Workflow` and `Artifacts` read `none` with the reason inline — cabin-play has no CI (the
enumeration is in the proof block) — the same shape 0138 / 0140 / 0141 used.

## Corrections and declared deviations

1. **Four files beyond the spec's one, and one it said could stay was reverted anyway.** The
   spec's only mandated file was `jc4827w543.yaml`; it explicitly allowed
   `tools/audit_layout.py` to stay. Both tools are reverted with it, because they encode the
   0144 geometry as constants: keeping the 0144 audit fails criterion 2 by construction (its
   checks assert "no content sits on the bottom edge" and three equal stat columns, which the
   0141 arrangement violates) and the 0144 preview re-renders the 0144 page, which would make
   the required "render PNG regenerated from the restored lambda" a picture of the wrong layout.
   Reverting `README.md` and the committed PNG keeps the documentation and the render truthful
   for the restored page. `.gitignore` was deliberately left as 0144 wrote it — the
   audit-page.png entry still names a file the reverted audit writes.
2. **No behaviour or configuration regression is introduced by the revert.** The status page, the
   HA entity list, sensors, wifi, api, ota, `ble_client` and the watchdog are not in the diff at
   all; the five changed paths are the display lambda, its two local tools, its render and its
   README section.
3. **The revert landed via a temporary branch.** Built and verified on
   `hermes/0145-guition-geometry-rollback`, then fast-forwarded onto cabin-play `main` and the
   temporary branch deleted from origin. A verifier reading a branch name will not find it;
   `main` at `e251a169…` is the deliverable.
4. **The colour-pipeline artifacts are still on the glass.** 0145 was geometry only by design;
   the blue/lime shift Stephen reported (0143) is unaffected by this change and remains parked.
