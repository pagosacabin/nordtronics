---
task_id: "0080"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: hermes/0080-heltec-step-freecad-assembly
  sha: b29526237c85aa460ff826d8e742d0080bae49a2
  run: none — no workflow in this repo triggers on this branch or these paths (enumeration below)
  files:
    - hardware/wildfire-node-v1/freecad/wildfire-node-v1.FCStd
    - hardware/wildfire-node-v1/freecad/README.md
  artifact:
    path: hardware/wildfire-node-v1/freecad/wildfire-node-v1.FCStd
    bytes: 1688556
    sha256: 9e253f77641f1b621e36adb6a5046c29e7df8a230f68523f8357d60c94c02b57
    valid_zip: true (52 entries, zipfile.testzip() -> None, Document.xml present, no GuiDocument.xml)
  component: Heltec_LoRa32_V4 — App::Part, present by internal name AND label, 1 child (the STEP file's own root assembly), 51 solids beneath it (38 board-level + 13 display module)
  recompute: 0 objects pending, 0 objects in Invalid/Error state, 0 invalid or null shapes — checked in the build session and again after reload from disk
  source_step:
    path: hardware/wildfire-node-v1/heltec-lora-32-v4.step
    commit: 2207c0fe95092b94f33be16fdf5624cae5981598
    sha256: 46f80761b5f3e5de5fc2eddbe4ed4837b4dfc0d403025d80b4e64f17abcebaae
    modified: no — git diff against the STEP path is empty after the task
  toolchain: FreeCAD 1.1.1 (rev 44227, snap, embedded headless) + freecad-robust-mcp 0.6.1 (83 tools)
  model: deepseek-flash
---

# 0080 — Import the Heltec V4 STEP into the wildfire-node FreeCAD assembly

**Status: staged.** Deliverable is on `hermes/0080-heltec-step-freecad-assembly` @
`b29526237c85aa460ff826d8e742d0080bae49a2`. Re-read from the remote after the push:
`git ls-remote --heads origin hermes/0080-heltec-step-freecad-assembly` and the local
tip both report that SHA. The branch was cut from main `dbd7a06`; main was
fast-forwarded to origin and the pickup commit `a121128` pushed before the branch did
any work.

## Reply — what changed

Two new files, one directory:

1. `hardware/wildfire-node-v1/freecad/wildfire-node-v1.FCStd` — FreeCAD 1.1.1
   assembly document, 78 objects, 51 solids, no GUI document (built headless).
2. `hardware/wildfire-node-v1/freecad/README.md` — records the orientation choice,
   the STEP commit SHA, the document structure, measured reference dimensions, the
   USB-C port window, and the verification performed.

The STEP file itself was **not modified** — read only, and `git diff` against it is
empty.

### ASCII model-tree dump (the spec's third proof item)

```
wildfire-node-v1  [App::Document]  objects=78
`-- Heltec_LoRa32_V4  [App::Part]  (children 1)  placement +90deg about X, base [-25.234, -7.753, 11.254]
    `-- Heltec LoRa 32 V4  [App::Part]  (the STEP file's own root assembly, 39 children)
        |-- HTIT-WB32LAF_V4.2          [51.69 x 1.56 x 25.4 mm]   main PCB
        |-- HTIT-WB32LAF_V4.2 (157)    [7.6 x 3.97 x 11.34 mm]    USB-C receptacle
        |-- HTIT-WB32LAF_V4.2 (317)(319) x2  [4.65 x 2.51 x 3.3 mm]   switch/button pair
        |-- HTIT-WB32LAF_V4.2 (200)(970) x2  [5.19 x 3.4 x 7.65 mm]   under-board parts
        |-- HTIT-WB32LAF_V4.2 (127)    [5.3 x 3.6 x 15.0 mm]      under-board part
        |-- ... 33 further board-level solids
        `-- display_module_assembly  [App::Part]  children=13
            |-- HTIT-WB32LAF_0.96OLED_SSD1315_V0.5 (154)   [30.9 x 0.8 x 16.6 mm]  OLED PCB
            |-- B-PK-F                                     [33.0 x 3.5 x 18.6 mm]  display backplate
            |-- 0.96 Lora 32-v4.2…屏蔽                      [24.7 x 1.7 x 17.3 mm]  shield
            |-- _5c_X_5c_C2…F9 / F001 / F002 / F003 x4      [4.0 x 3.0 x 4.0 mm]   螺柱 standoffs
            |-- M2_5c_X_5c_C2…BF / BF001 / BF002 / BF003 x4 [3.85 x 7.0 x 3.85 mm] 螺丝 M2 screws
            |-- HTIT-WB32LAF_0.96OLED_SSD1315_V0.5          [3.42 x 1.41 x 6.62 mm] OLED glass
            `-- view_area                                   [22.0 x 1.0 x 11.4 mm] display window
```

38 board-level solids + 13 display-module solids = 51; the remaining objects are the
App::Part containers and their `App::Origin` axis/plane features. The labels above are
the STEP's own (the mojibake is GBK Chinese: 螺柱 = standoff, 螺丝 = screw, 屏蔽 = shield).

### Orientation choice (a reply field the spec asks for)

The STEP's authored frame has the PCB standing in the X–Z plane with its 1.559 mm
thickness along Y and the component side toward +Y. The document rotates the whole
assembly +90° about X and translates it, giving:

- **Board plane X–Y, thickness along Z**; PCB underside face on the plane **Z = 0**,
  component face at **Z = +1.559 mm**.
- **Up is +Z: the component side faces +Z.** The USB-C receptacle, the OLED display
  module and the switches are all on the +Z side; the handful of bottom-side parts hang
  below the board's underside (assembly reaches Z = −3.6).
- **The USB-C opening faces −X.** The receptacle is on the board's short edge at the −X
  end and protrudes 0.971 mm past the board outline (board edge X = −25.845, connector
  front X = −26.816), so a cable plugs in from −X.
- The board outline is centred on the origin in X and Y; the board's long axis
  (51.689 mm) is X and its short axis (25.4 mm) is Y.

Implemented as `Heltec_LoRa32_V4.Placement = base (−25.234, −7.753, 11.254) + 90° about
X` — on the single component node, for the reason below.

### Import warnings and one deviation

- **No import warnings were produced or capturable.** This FreeCAD runs headless
  (`gui_available: false`), so there is no report view to quote. Import soundness was
  checked structurally instead: 0 invalid/null shapes, 0 objects in an Invalid/Error
  state, and `recompute()` returns 0 pending objects both in the build session and after
  reloading the saved file.
- **Deviation, worth knowing for later enclosure work:** the STEP importer's per-object
  placements are not uniform. The 38 board-level solids share one placement, but the 13
  solids inside `display_module_assembly` each carry an individual placement.
  Re-parenting *individual solids* into a new container therefore displaced the display
  module (measured: the display's solids landed tens of mm off the board) — an approach
  abandoned mid-task, and not what shipped. The delivered document preserves the import
  exactly: no solid was re-parented, the STEP file's own root assembly node was moved as
  a **single** object under a container created with an identity placement, and the
  orientation was applied to that container afterwards. Verified neutral: all 51 solid
  bounding boxes are identical (rounded to 1 µm) before and after the move, and again
  after save and reload.

### Versions used

FreeCAD **1.1.1** (rev 44227, snap install, `embedded` headless mode) driven through the
Hermes MCP server `freecad` — **freecad-robust-mcp 0.6.1**, 83 tools — via
`execute_python`. The whole document was built in one MCP server session, because
FreeCAD documents do not persist between server processes. Model: `deepseek-flash`.

## How it was verified

Every claim above is a measurement on the saved file, re-opened from disk in a fresh
session (nothing is quoted from the build session alone):

| Check | Result |
|---|---|
| Component present | `doc.getObject("Heltec_LoRa32_V4")` → App::Part, label `Heltec_LoRa32_V4` |
| Contents | 1 child (STEP root); 51 `Part::Feature` solids; `display_module_assembly` still holds 13 |
| Recompute after reload | 0 objects pending; 0 `Invalid`/`Error` states; 0 invalid or null shapes |
| Geometry preserved | all 51 solid bounding boxes identical pre/post move and after save+reload |
| Board plane | PCB world box `[−25.845, −12.7, 0.0] … [25.845, 12.7, 1.559]` → 51.689 × 25.4 × 1.559 mm, thin axis Z |
| USB-C world box | `[−26.816, −5.67, 0.859] … [−19.218, 5.67, 4.831]` → 0.971 mm past the −X edge, above the board |
| Assembly envelope | `[−26.816, −12.7, −3.6] … [25.985, 12.7, 7.06]` → 52.801 × 25.4 × 10.66 mm |
| File integrity | valid zip, 52 entries, `Document.xml` present, no `GuiDocument.xml` |

The USB-C was identified geometrically, not by name: the STEP contains no "USB" string
anywhere (its 47 `PRODUCT` names are the module part number, `display_module_assembly`,
and GBK-mojibake Chinese names for standoffs / screws / shield). Exactly two solids
overhang the PCB outline; the larger one, `HTIT-WB32LAF_V4.2 (157)`, overhangs the −X
short edge by 0.971 mm, rises 3.272 mm above the component face, and its axial section
at the mouth measures **8.94 × 3.16 mm** — the standard USB-C receptacle shell width —
with an internal 6.21 × 0.65 mm tongue and contact-pin sections behind the mouth. The
second overhanger is a 3.1 × 1.26 × 3.0 mm part overhanging the opposite edge by
0.141 mm, consistent with the IPEX/U.FL antenna connector.

**Not verified, and not claimed:** there is no rendered screenshot. FreeCAD here has no
GUI (`gui_available: false`), so the orientation above rests on measured world bounding
boxes in the saved document, not on a view of the model. The README says the same, so a
verifier is not left expecting a render that does not exist.

### Why `run: none` is earned

Enumerated from the repo, not from memory:

| Workflow | Trigger | Fires on this branch? |
|---|---|---|
| `.github/workflows/android-build.yml` | `on.push.branches: [android-toolchain-setup]` | no — branch not in the allow-list |
| `.github/workflows/platformio.yml` | `on.push.paths: firmware/tank-monitor/**, firmware/node-v1/**, .github/workflows/platformio.yml` | no — this change is under `hardware/` |
| `.github/workflows/website-check.yml` | `on.push.branches: [main, hermes/0068-site-email-refresh, hermes/0070-site-rewrite]` | no — branch not in the allow-list |

`gh run list -R pagosacabin/nordtronics --branch hermes/0080-heltec-step-freecad-assembly --json databaseId,headSha,name,conclusion`
returns `[]` — zero runs started, matching the enumeration. **No ntfy receipt is
included because this is not a build task**: nothing is compiled, no workflow runs and
no artifact is produced, so the `nordtronics-build-ed05a663` topic does not apply.

For completeness, the pickup commit on main (`a121128`, "0080: pickup (inbox → active,
iteration 1)") did trigger Website Check run
<https://github.com/pagosacabin/nordtronics/actions/runs/36672593448>, conclusion
`success`, `headSha` = `a1211283c56ea199f2807c2866f39cfc3576341e` = the main tip at the
time. That is the main-branch transition being green, not a run on the task branch.

---

## Spec (as received)

### Context

- Stephen supplied the official Heltec LoRa 32 V4 3D model, now on main at
  `hardware/wildfire-node-v1/heltec-lora-32-v4.step` (commit `2207c0fe95092b94f33be16fdf5624cae5981598`;
  STEP AP214, 11 MB, full module assembly: main PCB, display module, 0.96" OLED).
- You proved FreeCAD 1.1.1 + freecad-robust-mcp (0.6.1, 83 tools) in task 0040.
- Goal: the wildfire-node mechanical assembly in FreeCAD starts with the real
  board geometry, so enclosure work later fits the actual module.

### Task

One deliverable: a FreeCAD assembly document for the wildfire node v1
containing the Heltec STEP model, pushed to a branch.

### Success criteria

- A `.FCStd` file exists on the branch (suggested path:
  `hardware/wildfire-node-v1/freecad/wildfire-node-v1.FCStd`, create dirs as needed).
- The Heltec STEP is imported into the assembly as a named component
  (`Heltec_LoRa32_V4`).
- The document recomputes with no errors.
- A `README.md` next to the FCStd records the orientation choice
  (which way USB-C faces, which face is up) and the STEP file's commit SHA.

### Constraints

- Work only on your task branch; do not push to main.
- Do not modify `hardware/wildfire-node-v1/heltec-lora-32-v4.step` itself.
- Keep model cost sane: this is a static import, no parametric rebuild of the Heltec model.

### Proof

- Branch name + SHA on origin (`git ls-remote` checkable).
- File listing of the new `hardware/wildfire-node-v1/freecad/` directory.
- One screenshot or ASCII model-tree dump showing the `Heltec_LoRa32_V4` component.

### Reply format

- status: staged | blocked
- branch, sha
- notes: orientation choice, any STEP import warnings, FreeCAD/MCP versions used
