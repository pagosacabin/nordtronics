# wildfire-node-v1 — FreeCAD mechanical assembly

Static import of the Heltec WiFi LoRa 32 V4 module into a FreeCAD assembly
document, so enclosure work starts from the real board geometry.

| File | What it is |
|---|---|
| `wildfire-node-v1.FCStd` | FreeCAD 1.1.1 document, 78 objects, 51 solids, no GUI document (built headless) |
| `README.md` | this file |

## Source part

| | |
|---|---|
| STEP file | `hardware/wildfire-node-v1/heltec-lora-32-v4.step` |
| Commit | `2207c0fe95092b94f33be16fdf5624cae5981598` |
| sha256 | `46f80761b5f3e5de5fc2eddbe4ed4837b4dfc0d403025d80b4e64f17abcebaae` |
| Size / format | 11,212,274 bytes, STEP AP214 (`FILE_NAME('Heltec LoRa 32 V4.stp')`), full module assembly |

The STEP file was **read only** — `git diff` against it is empty after this task.

## Orientation choice

The STEP's own authored frame has the PCB standing in the X–Z plane (its
1.559 mm thickness along Y, component side toward +Y). This document rotates the
whole assembly a quarter turn and centres it, giving a flat, plottable datum:

- **Board plane: X–Y.** PCB thickness runs along **Z**; the PCB's underside face
  sits on the plane **Z = 0**, its component face at **Z = +1.559 mm**.
- **Up = +Z** — the **component side (USB-C receptacle, OLED display module,
  switches) faces +Z**; the few bottom-side parts hang below the board
  (down to Z = −3.6 mm).
- **USB-C faces −X.** The receptacle is on the board's short edge at the −X end
  and protrudes 0.971 mm past the board outline (board edge at X = −25.845,
  connector front at X = −26.816). Plugging in a cable comes from −X.
- The board outline is **centred on the origin** in X and Y; the board's long
  axis (51.689 mm, USB-C end at −X) is X and its short axis (25.4 mm) is Y.

Implemented as the placement of the single component node:

```
Heltec_LoRa32_V4.Placement = base (−25.234, −7.753, 11.254) + 90° about the X axis
```

Everything inside is untouched, so the transform is rigid: every solid moves with
the assembly and no internal relationship changes (verified — see below).

## Document structure

```
wildfire-node-v1  [App::Document]  78 objects
└── Heltec_LoRa32_V4  [App::Part]  the named component; carries the placement above
    └── Heltec LoRa 32 V4  [App::Part]  the STEP file's own root assembly, preserved as imported
        ├── HTIT-WB32LAF_V4.2                     [51.69 × 1.56 × 25.4 mm]  main PCB
        ├── HTIT-WB32LAF_V4.2 (157)               [7.6 × 3.97 × 11.34 mm]  USB-C receptacle
        ├── HTIT-WB32LAF_V4.2 (317), (319) ×2     [4.65 × 2.51 × 3.3 mm]   switch/button pair
        ├── HTIT-WB32LAF_V4.2 (200), (970) ×2     [5.19 × 3.4 × 7.65 mm]   under-board parts
        ├── HTIT-WB32LAF_V4.2 (127)               [5.3 × 3.6 × 15.0 mm]    under-board part
        ├── HTIT-WB32LAF_V4.2 (988)               [3.1 × 1.26 × 3.0 mm]    edge connector, +X end
        ├── … 32 further board-level solids (RF shield, passives, IPEX, etc.)
        └── display_module_assembly  [App::Part]  children 13
            ├── HTIT-WB32LAF_0.96OLED_SSD1315_V0.5 (154)  [30.9 × 0.8 × 16.6 mm]  OLED PCB
            ├── B-PK-F                                    [33.0 × 3.5 × 18.6 mm]  display backplate
            ├── 0.96 Lora 32-v4.2…屏蔽                     [24.7 × 1.7 × 17.3 mm]  shield
            ├── _5c_X_5c_C2…F9 / F001 / F002 / F003 ×4     [4.0 × 3.0 × 4.0 mm]   螺柱 standoffs
            ├── M2_5c_X_5c_C2…BF / BF001 / BF002 / BF003 ×4 [3.85 × 7.0 × 3.85 mm] 螺丝 M2 screws
            ├── HTIT-WB32LAF_0.96OLED_SSD1315_V0.5         [3.42 × 1.41 × 6.62 mm] OLED glass
            └── view_area                                  [22.0 × 1.0 × 11.4 mm]  display window
```

38 board-level solids + 13 display-module solids = **51 solids**; the document
holds 78 objects in total (the remaining 27 are the App::Part containers and
their `App::Origin` axis/plane features).

## Reference dimensions (measured on the saved file)

| Quantity | Value |
|---|---|
| PCB outline | 51.689 × 25.4 mm, 1.559 mm thick, underside at Z = 0 |
| Assembly envelope | 52.801 × 25.4 × 10.66 mm |
| Height above PCB component face | 5.501 mm (display module top at Z = +7.06) |
| Depth below PCB underside | 3.6 mm (bottom-side parts, Z = −3.6) |
| Assembly Z range | −3.6 … +7.06 mm |

### USB-C port window (for enclosure cut-outs)

| | X | Y | Z |
|---|---|---|---|
| receptacle solid, world | −26.816 … −19.218 | −5.67 … +5.67 | +0.859 … +4.831 |

The plug opening is the −X face at X = −26.816; a cut-out should be sized from the
8.94 mm × 3.16 mm shell section below, not from the 11.34 mm overall width, which
includes the shield mounting legs.

## How the USB-C was identified

The STEP carries no product name for the receptacle (its 47 `PRODUCT` names are
the module part number, `display_module_assembly`, and GBK-mojibake Chinese names
for the standoffs 螺柱, screws 螺丝 and shield 屏蔽 — no "USB" string anywhere), so
it was identified geometrically:

- Exactly **two** solids overhang the PCB outline. The larger one — "HTIT-WB32LAF_V4.2 (157)" —
  overhangs the −X short edge by **0.971 mm**, rises **3.272 mm** above the board's
  component face, and sits on the component side.
- Its axial cross-section at the mouth measures **8.94 mm × 3.16 mm** — the
  standard USB-C receptacle shell width — with an internal **6.21 × 0.65 mm**
  tongue appearing 0.6 mm behind the mouth and small 0.59/0.37 mm contact-pin
  sections on the tongue. That is a USB-C receptacle, not a generic block.
- The second overhanging solid is 3.1 × 1.26 × 3.0 mm with a 0.141 mm overhang at
  the opposite (+X) end — consistent with the IPEX/U.FL antenna connector, not a
  power connector.

## STEP import notes

- FreeCAD's STEP importer produced **69 objects**: 51 solids, the
  `display_module_assembly` container, the file's own root assembly node, and
  origin features.
- **A deviation worth knowing:** the importer's per-object placements are not
  uniform — the 38 board-level solids share one placement, while the 13
  display-module solids each carry an individual placement. Re-parenting
  individual solids into a new container therefore *displaced the display module*
  (measured: the display's solids landed tens of mm off the board). To avoid that,
  **no solid was re-parented**: the STEP's root assembly node was moved as a
  single object under a container created with an identity placement, which
  changed no geometry at all (all 51 bounding boxes byte-identical before and
  after the move), and the orientation was applied to the container afterwards.
- No import warnings are capturable in this headless setup (there is no report
  view); import soundness was checked structurally instead — see below.
- This is a **static import**: no parametric rebuild of the Heltec model, no
  values driven by expressions or a spreadsheet.

## Verification performed

Checked by driving FreeCAD 1.1.1 headless through the `freecad` MCP server
(`execute_python`), on the saved and re-opened file:

- Document re-opens from disk: 78 objects, component `Heltec_LoRa32_V4` present
  **by internal name and by label**, 1 child (the STEP root), 51 `Part::Feature`
  solids, `display_module_assembly` still holding its 13 solids.
- `recompute()` after reload returns **0** objects still to recompute;
  **0** objects in an `Invalid`/`Error` state; **0** invalid or null shapes.
- Geometry is unchanged by the transform step: every one of the 51 solid bounding
  boxes is identical before and after (list-compared, rounded to 1 µm).
- The saved file is a valid archive: `zipfile.testzip()` → `None`, 52 entries,
  `Document.xml` present, no `GuiDocument.xml` (headless build).
- `sha256(wildfire-node-v1.FCStd)` = `9e253f77641f1b621e36adb6a5046c29e7df8a230f68523f8357d60c94c02b57`
  (1,688,556 bytes).

Not performed: no rendered screenshot. This machine runs FreeCAD headless
(`gui_available: false`), so orientation was established by measuring the world
bounding boxes of the PCB, the USB-C solid and the display module in the saved
document — not by looking at a view.

## Toolchain

- FreeCAD **1.1.1** (rev 44227), snap install, `embedded` headless mode,
  `gui_available: false`.
- `freecad-robust-mcp` **0.6.1** (83 tools) registered as the Hermes MCP server
  `freecad`; the document was built in a single server session because FreeCAD
  documents do not survive between MCP server processes — saving is what makes
  them durable.
