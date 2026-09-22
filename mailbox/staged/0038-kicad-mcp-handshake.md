---
task_id: "0038"
status: in_progress
iteration: 1
---

# 0038 — KiCad MCP handshake: prove you can drive the MixelPix server, blank project

## Context

You installed the MixelPix KiCad MCP server (`mixelpixx/KiCAD-MCP-Server`,
the Python/TypeScript one, MIT). Before any real PCB work, prove you can act
as an MCP client against it: register it, call its tools, and produce a real
KiCad project through it. This is the first rung of the KiCad ladder — a
handshake, not a design.

## Task

1. Report the version of the server you installed: from your checkout run
   `git log -1 --format=%H` (full SHA) and read the version field in
   `package.json`. Report both.
2. Register the server with your MCP client setup and confirm the
   registration works. Report the exact number of tools your client sees.
3. Run one discovery call successfully (`search_tools` or
   `list_tool_categories`).
4. Through MCP tools only, create a blank project named `hello-kicad`
   (`create_project`). No components, no wires, no routing — schematic and
   PCB shells only.
5. Validate the blank project (`validate_schematic`, and/or open it in KiCad
   — it must open under your KiCad v10).
6. Commit the project files and push branch `hermes/0038-kicad-hello` to
   origin.

## Success criteria

1. `hello-kicad` project exists and was created through MCP tool calls, not
   hand-written files.
2. The project validates clean and opens in KiCad.
3. Branch `hermes/0038-kicad-hello` is on origin at the SHA you cite.
4. Your reply reports: server version + checkout SHA, the tool count your
   client sees, and `kicad-cli --version` output.

## Constraints

- Blank project only. No components, no routing, no DigiKey, no JLCPCB.
- Java and FreeRouter are NOT required for this task.
- All KiCad operations stay local on your machine; only the project files go
  to GitHub.
- If you cannot act as an MCP client (registration fails, tools don't
  respond), stop and report the exact error text — do not fake it.

## Proof

- Branch `hermes/0038-kicad-hello` on origin, full tip SHA quoted.
- The tool count number.
- `kicad-cli --version` output, copied verbatim.
- The validation output, copied verbatim.

## Reply format

Move this file to `mailbox/staged/` with `status: staged` (the staged file IS
the reply). Include:

- Status: done | blocked
- Server version + checkout SHA
- Tool count your client sees
- `kicad-cli --version` output
- Branch + full SHA
- Validation output (or the exact blocking error)

expect-reply-within: 6h
