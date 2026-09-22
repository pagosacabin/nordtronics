---
task_id: "0040"
status: inbox
---

# 0040 — FreeCAD MCP: install, register, prove it

## Context

KiCad MCP is working (0038). Next rung: FreeCAD, for future mechanical
work — the wildfire node enclosure will need real CAD eventually, and
that means a parametric modeler Hermes can drive. Same playbook as 0038:
you set it up on your own machine, then prove it end to end through MCP
tools.

Candidate servers (evaluate before picking):

FreeCAD 1.1.1 is already installed on the machine (per Stephen,
2026-09-22) — confirm it in step 1, no install needed unless you
choose to upgrade.

- spkane/freecad-addon-robust-mcp-server — 150+ tools, installs from
  PyPI (`pip install freecad-robust-mcp`), workbench via FreeCAD Addon
  Manager, works headless and GUI, actively maintained, MIT.
- bradsjm/freecad-embedded-mcp — MCP server embedded INSIDE FreeCAD's
  GUI over local HTTP, 25 tools, ships an installable agent skill;
  requires FreeCAD 1.1.3+ (dev version) and a running FreeCAD GUI —
  the installed 1.1.1 does NOT qualify, so this option would need a
  FreeCAD upgrade first. Factor that cost into your evaluation.
- seansackowitz/mcp-freecad — ~70 small parametric-first tools,
  token-efficient, XML-RPC bridge workbench; less actively maintained.

## Task

1. Confirm the installed FreeCAD version (Stephen reports 1.1.1) and
   how it was installed (native, flatpak, etc.).
2. Evaluate the candidates above against YOUR machine (headless or GUI?
   FreeCAD version? flatpak/sandbox quirks?). Pick one and justify the
   choice in one paragraph. If none of them work in your environment,
   stop and report exactly why — do not install something unproven.
3. Install the server and register it in your MCP config alongside kicad
   (the same `mcp_servers` pattern you proved in 0034/0038).
4. Reload so the tools are visible. Report the server name, version, and
   exact tool count.
5. Through MCP tools ONLY, build a parametric test part: a 60x40x20mm
   block with a centered 10mm through-hole on the 60x40 face, all
   dimensions as named parameters (not hardcoded). Export STEP and STL.
   Save the .FCStd.
6. Push the three files to branch `hermes/0040-freecad-hello` under
   `hardware/hello-freecad/` on origin.

## Success criteria

1. FreeCAD version reported; install method reported if you installed it.
2. Server choice justified against your environment.
3. MCP server registered; tool count reported.
4. Test part built through MCP tools only; STEP + STL + FCStd on the
   branch at the cited SHA.

## Constraints

- No spending, no license purchases — everything here is free/OSS.
- If the server needs a running FreeCAD GUI and you are headless, say so
  instead of faking it.
- Report exact errors, not summaries.

## Proof

- Branch `hermes/0040-freecad-hello` on origin, full tip SHA quoted.
- The three files listed on the branch; STEP file size quoted.
- Server name + version + tool count.

## Reply format

Move this file to `mailbox/staged/` with `status: staged` (the staged file IS
the reply). Include: status, branch + SHA, FreeCAD version, server choice
+ justification, tool count, and the exact blocking error if blocked.

expect-reply-within: 6h
