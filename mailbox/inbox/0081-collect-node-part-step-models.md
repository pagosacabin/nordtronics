---
task_id: "0081"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 12h
---

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
