# hello-freecad — 0040 parametric test part

Built through MCP tools only (FreeCAD Robust MCP server, embedded/headless mode)
by Hermes on 2026-09-22.

**Part:** 60 x 40 x 20 mm block with a centered 10 mm through-hole on the 60x40 face.

**Parametric:** every dimension lives in a `Spreadsheet::Sheet` named `Params`
with aliases, and the geometry is expression-bound to it — no hardcoded values
in the model tree:

| Cell | Alias             | Value | Drives |
|------|-------------------|-------|--------|
| A1   | `block_length`    | 60    | `Block.Length`, hole X centre |
| A2   | `block_width`     | 40    | `Block.Width`, hole Y centre |
| A3   | `block_height`    | 20    | `Block.Height`, hole height |
| A4   | `hole_diameter`   | 10    | `Hole.Radius` (= `hole_diameter / 2`) |
| A5   | `hole_clearance`  | 10    | hole over-travel top/bottom (through-hole) |

Change test performed in-session: `block_length` 60 -> 80 moved the bounding box to
80 x 40 x 20 and the volume to 62429.2 mm³; `hole_diameter` 10 -> 16 gave 59978.8 mm³
(identical to the analytic value); both restored to 60/10 afterwards.

Verified geometry: bbox `(0,0,0)-(60,40,20)`, volume 46429.20 mm³ vs analytic
46429.20 mm³ (delta 0.000), 1 valid solid.

## Files

- `hello-freecad.FCStd` — saved document
- `hello-freecad.step` — STEP export of `BlockDrilled`
- `hello-freecad.stl` — STL export of `BlockDrilled` (144 facets, tolerance 0.05 mm)

Test part only — no fabrication data, not reviewed by Stephen.
