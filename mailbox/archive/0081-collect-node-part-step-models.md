---
task_id: "0081"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 12h
proof:
  branch: hermes/0081-collect-node-part-step-models
  sha: 494ad4efa4d3b5704fe4dee26d535c3addb07ab1
  run: none — no workflow in this repo triggers on this branch or these paths (enumeration below)
  main_transition_run: https://github.com/pagosacabin/nordtronics/actions/runs/36704366262
  files:
    - hardware/wildfire-node-v1/3d/SOURCES.md
    - hardware/wildfire-node-v1/3d/jst-ph-2pin-smd.step
    - hardware/wildfire-node-v1/3d/jst-gh-2pin-1.25mm.step
    - hardware/wildfire-node-v1/3d/ntc-10k-disc.step
    - hardware/wildfire-node-v1/3d/miniboost-4654.step
    - hardware/wildfire-node-v1/3d/as3935-bqft-qfn16.step
    - hardware/wildfire-node-v1/3d/bme688-lga8.step
    - hardware/wildfire-node-v1/3d/lm393-soic8.step
    - hardware/wildfire-node-v1/3d/ao3401-sot23.step
    - hardware/wildfire-node-v1/3d/bat54-sot23.step
    - hardware/wildfire-node-v1/3d/tl431-sot23.step
    - hardware/wildfire-node-v1/3d/2n3904-to92.step
  geometry:
    parts_in_spec: 13
    parts_with_a_step_file: 11
    part_specific_manufacturer_models: 4
    package_generic_models: 7
    gaps: 2
    step_files_bytes: 1631510
    total_added_bytes: 1642087
    verification: "each file opened in FreeCAD 1.1.1 (Part.read -> isValid()==True, >=1 solid) and its solid bbox measured; no dimension off by >2x"
    url_replication: "the 4 part-specific source URLs were re-fetched at review time and hash-compared: JST x2 and TDK hash-identical, Adafruit re-fetched at 179,502 bytes"
notes: "11/13 parts covered. 4 are the manufacturer's own CAD (JST S2B-PH-K-S, JST SM02B-GHS-TB, TDK B57164K/K164, Adafruit 4654); 7 are package-generic models from KiCad's official package library (AS3935-BQFT, BME688, LM393, AO3401, BAT54, TL431, 2N3904) — correctly dimensioned but NOT part-specific, flagged as such per file in SOURCES.md. Gaps: PMS5003 (Plantower publishes no CAD at all) and RXEF075 (Littelfuse's only model asset is a registration-walled partner ECAD entry). No dimension is off by >2x; three sub-2x observations are flagged in SOURCES.md notes (LGA-8/QFN-16 body-only heights, AS3935 exposed pad 2.5 vs 2.70 mm) plus one file gotcha: miniboost-4654.step carries far-off construction geometry, so a naive point-cloud bbox reports a nonsense size while the real solid is 11.43 x 17.78 x 5.07 mm. Part-specific models for the five small discretes + AS3935 + BME688 exist only behind login/CAPTCHA (SnapMagic, Ultra Librarian, DigiKey, onsemi) and were not obtained, per the task's no-signup rule."
---

# 0081 — Reply: STEP 3D models for the node v1 BOM

**Status: staged.** Deliverable is on `hermes/0081-collect-node-part-step-models` @
`494ad4efa4d3b5704fe4dee26d535c3addb07ab1`. `git ls-remote --heads origin
hermes/0081-collect-node-part-step-models` reports that SHA as the tip, and the local
branch tip equals it (checked after the push).

## File listing (`hardware/wildfire-node-v1/3d/`, with sizes)

| File | Bytes |
|---|---|
| `SOURCES.md` | 10,577 |
| `jst-ph-2pin-smd.step` | 205,022 |
| `jst-gh-2pin-1.25mm.step` | 243,791 |
| `ntc-10k-disc.step` | 403,005 |
| `miniboost-4654.step` | 179,502 |
| `as3935-bqft-qfn16.step` | 192,117 |
| `bme688-lga8.step` | 74,172 |
| `lm393-soic8.step` | 125,320 |
| `ao3401-sot23.step` | 60,296 |
| `bat54-sot23.step` | 60,296 |
| `tl431-sot23.step` | 60,296 |
| `2n3904-to92.step` | 27,693 |
| **total** | **1,642,087 (1.57 MB)** — limit ~120 MB |

## What was found

**11 of the 13 parts in the spec have geometry. 4 are part-specific, 7 are
package-generic.**

### Part-specific — the manufacturer's own published CAD

| File | Part, exactly as sourced |
|---|---|
| `jst-ph-2pin-smd.step` | JST official STEP for **S2B-PH-K-S** — PH 2.00 mm, 2-circuit, **side-entry SMT** (the SMD variant the spec asks for, not the through-hole `B2B`) |
| `jst-gh-2pin-1.25mm.step` | JST official STEP for **SM02B-GHS-TB** — GH 1.25 mm, 2-circuit, side-entry SMT |
| `ntc-10k-disc.step` | TDK/EPCOS official 3D outline **K164** (= B57164K, 10 kΩ leaded NTC disc) |
| `miniboost-4654.step` | Adafruit official CAD repo, **PID 4654** MiniBoost 5 V @ 1 A (TPS61023) |

### Package-generic — real package geometry, NOT part-specific

`as3935-bqft-qfn16.step` (QFN-16-1EP 4×4 P0.65), `bme688-lga8.step` (Bosch LGA-8
3×3 P0.8), `lm393-soic8.step` (SOIC-8 3.9×4.9 P1.27), `ao3401-sot23.step` /
`bat54-sot23.step` / `tl431-sot23.step` (SOT-23), `2n3904-to92.step` (TO-92 inline)
— all from KiCad's official package library.

**These are not AS3935/BME688/LM393/AO3401/BAT54/TL431/2N3904 models.** They carry
the correct JEDEC/package envelope and nothing part-specific — no pin-1 marking, no
part silhouette. Each is labelled as package-generic in `SOURCES.md`. The three
SOT-23 files are byte-identical (one package model, three part names).

## Gaps (2)

1. **PMS5003** — Plantower publishes no CAD at all. The product page and download
   centre (reachable only behind a cookie gate; an unauthenticated request gets
   403) contain one company PDF and zero 3D links. Distributors were no help
   (Mouser Akamai-blocked, DigiKey 403 on every locale, Arrow timeout,
   Farnell/element14 403) and no distributor sells the bare module. KiCad has no
   Plantower/PMS model. Community file-share models exist but the task excludes
   them — **not downloaded**.
2. **Littelfuse RXEF075** — the model is registration-walled. Littelfuse's only
   asset on the product page is a `Partner ECAD Models` (SamacSys) entry; Mouser/
   DigiKey/Farnell 403 and Arrow times out. TE Connectivity (the alternate
   manufacturer) has no such product page (404) and states CAD is by e-mail only.
   KiCad's `Fuse` library has no radial PPTC disc, and the BelFuse `0ZRE0075FF`
   model was **not** substituted because its body (L11.5 × W4.8 mm) does not match
   the RXEF075's ≈10.2 × 3 mm — that would have been a mis-sized stand-in.

Also gap, worth recording because it answers the spec's explicit question:
**every part-specific model for AS3935-BQFT, BME688, LM393, TL431, 2N3904, BAT54
and AO3401 lives behind a login or CAPTCHA** (SnapMagic/SnapEDA, Ultra Librarian —
TI's own CAD export sits behind a reCAPTCHA; DigiKey model pages read "You must be
logged-in to download the model"; onsemi and diodes.com are bot-walled). Per the
task's constraint these were **not** obtained by signing up. Note also that
ScioSense (AS3935, ex-ams) ships a 10.95 MB product archive which was downloaded
and fully extracted — datasheets, gerbers, BOM, firmware, **no CAD** — and its
entire media library (749 items) holds only PNG renders.

## Dimension check — nothing off by >2×

Every file was opened in FreeCAD 1.1.1: `Part.read()` → `isValid() == True`, ≥1
solid, then the **solid** bbox measured. Measured values and their package
standards are tabulated in `SOURCES.md`. Closest calls, all well inside 2×:

- `miniboost-4654.step` 11.43 × 17.78 × 5.07 mm vs Adafruit's published
  17.8 × 11.3 × 5.6 mm (the published Z includes full header pin height).
- `bme688-lga8.step` 3.0 × 3.0 × **0.78** and `as3935-bqft-qfn16.step`
  4.0 × 4.0 × **0.77** against the datasheet maxima 0.93 / 0.9 mm — library
  models stop at the moulded body. Matters only for a clearance stack-up.
- The QFN-16 exposed pad is 2.5 mm against the AS3935-BQFT's 2.70 mm nominal
  (−0.2 mm); fine for assembly visualisation, know it for stencil work.
- `2n3904-to92.step` is TO-92 with inline leads, 9.8 mm overall (body 4.83 mm
  diameter) — the classic 2N3904 package. The spec said "confirm package per BOM"
  and the BOM records no package, so TO-92 was chosen; the SOT-23 geometry in this
  directory is reusable verbatim if the BOM turns out to say SOT-23.

## One file gotcha worth knowing before the assembly imports it

`miniboost-4654.step` contains construction geometry far outside the board (a
`CARTESIAN_POINT` at z ≈ 5×10⁵ mm), so **any naive min/max over the file's points
reports an absurd bounding box**. The real solid is 11.43 × 17.78 × 5.07 mm. This
was checked the way the assembly will load it — `import_step` into a fresh FreeCAD
document — which produces 11 clean solids (PCB 1.57 mm thick, 4-pin header,
components), origin at (0,0,0), no stray geometry.

## How it was verified (what was actually run)

1. **Every STEP opened as a solid**, measured in FreeCAD through the MCP bridge —
   results in the `SOURCES.md` tables.
2. **Every source URL is live and the committed bytes match, proven by re-fetching
   at review time, not by trusting the download:**
   - `jst-ph` / `jst-gh`: the JST flow was reproduced end to end by hand — product
     page (sets `PHPSESSID`) → licence interstitial → `download.php?…` returning
     24,894 B / 28,074 B ZIPs, extracted to `S2B-PH-K-S.STEP` (205,022 B) and
     `SM02B-GHS-TB.STEP` (243,791 B) — **sha256 identical** to the committed files.
   - `tdk`: re-fetched (403 to `curl` — WAF; 200 to node/undici) as 403,005 B —
     **sha256 identical**.
   - `adafruit`: re-fetched as 179,502 B, byte-identical.
   - KiCad package models: fetched directly from GitLab raw URLs (HTTP 200 each).
3. **The pushed blobs are the verified bytes** — `git ls-tree` on the branch tip vs
   `git hash-object` on disk, all 12 blobs equal; the SOT-23 trio share one blob
   hash as documented.
4. **Nothing was hand-modelled and nothing was invented.** Every committed byte came
   over HTTP from the URL in `SOURCES.md`.

The two JST files supersede the package-level JST connectors KiCad's library
provides (those were fetched first, then replaced once the official JST data was
obtained). Both alternatives, plus two further verified-but-unused files, are listed
in `SOURCES.md` with their URLs, so nothing researched is lost.

## CI — why `run: none` is correct here (enumerated, not assumed)

`ls .github/workflows/` → `android-build.yml`, `platformio.yml`, `website-check.yml`.
None can fire for this push:

- `android-build.yml`: `on.push.branches: [android-toolchain-setup]` — not this branch.
- `platformio.yml`: `on.push.paths` = `firmware/tank-monitor/**`, `firmware/node-v1/**`,
  `.github/workflows/platformio.yml` — this branch touches `hardware/wildfire-node-v1/3d/**` only.
- `website-check.yml`: `on.push.branches: [main, hermes/0068-site-email-refresh,
  hermes/0070-site-rewrite]` — not this branch.

Confirmed live, not recalled:
`gh api "repos/pagosacabin/nordtronics/actions/runs?branch=hermes/0081-collect-node-part-step-models"`
returns `total_count: 0`, and `gh run list --branch …` is empty. The branch tip SHA
above is therefore the proof pointer.

**No ntfy receipt:** the build topic in this repo is for CI builds that produce a
compiled artifact (APK / firmware image). This task compiles nothing — it adds STEP
files and a markdown index — and the spec names no topic for it.

## Scope notes

1. **The spec's part list mixes two BOMs.** Items 5–10 (AO3401, LM393, TL431, 2N3904,
   BAT54, RXEF075) and 12–13 (JST-GH, NTC disc) are solar-gate/temp-gate BOM parts,
   not node v1 parts — the Rev C sheet carries U2 PMS5003, U3 AS3935, U4 BME68x,
   U5 MiniBoost, J1 (JST-PH), J2, F1, plus C/R/Q/TP. All 13 were hunted as asked;
   the node-v1-only gaps are PMS5003 and F1/RXEF075.
2. **The task shipped without an `iteration` field** (it had `task_id`,
   `protocol_version`, `status`, `expect-reply-within`). `iteration: 1` was added at
   pickup, and that addition was declared in the pickup commit's front-matter note.
3. **Task 0080's branch was not used as the base.** The task says "on your branch";
   this one is cut from `main`, which already carries
   `hardware/wildfire-node-v1/heltec-lora-32-v4.step`. Main carries **0** files under
   `hardware/wildfire-node-v1/3d/`, so a verifier diffing against main will correctly
   see all 12 paths as new. The FreeCAD assembly itself lives on the 0080 branch
   (`hardware/wildfire-node-v1/freecad/`), which is not merged to main either.
4. **`.stp` renamed to `.step`** for the TDK file so every file shares one extension
   (same format, content untouched — hash verified after rename).
5. **No secrets, no credentials, no sign-ups.** Nothing was submitted to any form:
   the JST licence interstitial was used only to arm its per-download session flag,
   and no company or contact data was supplied.

<!-- original task spec, preserved -->

# 0081 — Collect STEP 3D models for the wildfire-node v1 parts

## Context

- Task 0080 starts the wildfire-node FreeCAD assembly with the Heltec V4 STEP
  (already on main at `hardware/wildfire-node-v1/heltec-lora-32-v4.step`).
- The 2026-09-29 KiCad footprint survey
  (`workspace/goals/lora-wildfire-early-warning-network/files/kicad-footprint-survey-2026-09-29.md`
  in Juno's workspace; key facts repeated below) found SnapMagic KiCad models for
  10 parts — SnapMagic also hosts STEP 3D models for most of them.
- Goal: as many STEP files as can be found for the node v1 BOM, in one place,
  so the FreeCAD assembly and later KiCad 3D views use real geometry.

## Task

One deliverable: a `hardware/wildfire-node-v1/3d/` directory on your branch
containing one STEP file per part below (as many as you can source), plus a
`SOURCES.md` listing each file's source URL and what it is.

## Parts to hunt (Rev C BOM)

Have STEP already or confirmed on SnapMagic — fetch these first:
1. PMS5003 particulate sensor (Plantower)
2. AS3935-BQFT bare IC (SnapMagic has the bare IC; the SparkFun SEN-15441
   breakout has no known model — get the bare IC, note the breakout gap)
3. BME688 bare IC (SnapMagic; this is the production pick — the BME680 is end-of-life per the 2026-09-29 keep/replace decision, so model the 688, not the 680; breakout TBD at bench — bare IC is fine)
4. JST-PH 2-pin SMD, 2.0 mm (J1 battery input)
5. AO3401 (SOT-23)
6. LM393 (SOIC-8 — confirm package against the Rev C schematic notes)
7. TL431 (confirm package: SOT-23 vs TO-92, per solar-gate-v2 BOM)
8. 2N3904 (confirm package per BOM)
9. BAT54 (SOT-23)

No known model — try manufacturer / distributor pages, note if truly missing:
10. Littelfuse RXEF075 (radial PTC, F1)
11. Adafruit MiniBoost 4654 module (check adafruit.com product page for 3D files)
12. JST-GH 2-pin, 1.25 mm (solar-gate J1/J2)
13. NTC disc thermistor (10k, per temp-gate BOM — generic disc size ok, note it)

## Success criteria

- `hardware/wildfire-node-v1/3d/` contains one `.step`/`.stp` per part found,
  filenames `<part>-<package>.step` (e.g. `ao3401-sot23.step`).
- Every STEP opens (non-empty solid; sanity-check bounding box vs the part's
  real dimensions — flag anything off by >2x).
- `SOURCES.md` lists every file with its download URL; parts not found are
  listed under "gaps" with where you looked.
- Total added size stays under ~120 MB.

## Constraints

- Work only on your task branch; do not push to main.
- Prefer manufacturer or SnapMagic/Ultra-Librarian sources; no random
  file-share binaries. If a source needs a login you don't have, note it as a
  gap rather than signing up for anything.
- Do not model stand-in geometry yourself in this task — gaps are just gaps.

## Proof

- Branch name + SHA on origin (`git ls-remote` checkable).
- File listing of `hardware/wildfire-node-v1/3d/` with sizes.
- `SOURCES.md` contents.

## Reply format

- status: staged | blocked
- branch, sha
- notes: parts found (count), gaps, any dimension mismatches flagged
