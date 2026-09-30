---
task_id: "0080"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0080 — Import the Heltec V4 STEP into the wildfire-node FreeCAD assembly

## Context

- Stephen supplied the official Heltec LoRa 32 V4 3D model, now on main at
  `hardware/wildfire-node-v1/heltec-lora-32-v4.step` (commit `2207c0fe95092b94f33be16fdf5624cae5981598`;
  STEP AP214, 11 MB, full module assembly: main PCB, display module, 0.96" OLED).
- You proved FreeCAD 1.1.1 + freecad-robust-mcp (0.6.1, 83 tools) in task 0040.
- Goal: the wildfire-node mechanical assembly in FreeCAD starts with the real
  board geometry, so enclosure work later fits the actual module.

## Task

One deliverable: a FreeCAD assembly document for the wildfire node v1
containing the Heltec STEP model, pushed to a branch.

## Success criteria

- A `.FCStd` file exists on the branch (suggested path:
  `hardware/wildfire-node-v1/freecad/wildfire-node-v1.FCStd`, create dirs as needed).
- The Heltec STEP is imported into the assembly as a named component
  (`Heltec_LoRa32_V4`).
- The document recomputes with no errors.
- A `README.md` next to the FCStd records the orientation choice
  (which way USB-C faces, which face is up) and the STEP file's commit SHA.

## Constraints

- Work only on your task branch; do not push to main.
- Do not modify `hardware/wildfire-node-v1/heltec-lora-32-v4.step` itself.
- Keep model cost sane: this is a static import, no parametric rebuild of the Heltec model.

## Proof

- Branch name + SHA on origin (`git ls-remote` checkable).
- File listing of the new `hardware/wildfire-node-v1/freecad/` directory.
- One screenshot or ASCII model-tree dump showing the `Heltec_LoRa32_V4` component.

## Reply format

- status: staged | blocked
- branch, sha
- notes: orientation choice, any STEP import warnings, FreeCAD/MCP versions used
