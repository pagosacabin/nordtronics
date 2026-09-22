---
task_id: 0038
status: staged
iteration: 1
proof:
- branch: hermes/0038-kicad-hello
  sha: 3704fff05099cdc7ab2824c3e72537d408ea4768
notes: "Status: done\n\nServer: mixelpixx/KiCAD-MCP-Server (Python/TS flavor), package.json version 2.7.0,\n\
  checkout SHA ac716d1a8bfad325b4aa93a398222645b3f78fd7 (\"Merge pull request #411\"),\ndated 2026-09-09.\
  \ Checkout at /home/astroboy/KiCAD-MCP-Server.\n\nRegistration: mcp_servers.kicad in ~/.hermes/config.yaml\
  \ -> `node /home/astroboy/KiCAD-MCP-Server/dist/index.js`,\nenabled: true. Tools appear in the client\
  \ schema as mcp__kicad__* \u2014 count 237.\n\nTool count my client sees: 237 = 233 server tools (cache\
  \ ~/.hermes/cache/mcp_schema_cache.json)\n+ 4 Hermes utility tools (list_resources, read_resource, list_prompts,\
  \ get_prompt).\nThe server's own inventory reports it differently: list_tool_categories says\ntotal_categories\
  \ 16, total_routed_tools 148, total_direct_tools 32 (= 180 named calls);\nthe routed ones are aliases\
  \ into the same 233 registered tools.\n\nDiscovery call: list_tool_categories succeeded (16 categories,\
  \ counts above); search_tools(\"project\")\nreturned 6 direct matches (create_project, open_project,\
  \ close_project, save_project,\nsnapshot_project, get_project_info).\n\nProject created through MCP\
  \ only: create_project {path: /home/astroboy/nordtronics/hardware/hello-kicad,\nname: hello-kicad} ->\
  \ success true, backend swig, wrote .kicad_pro + .kicad_pcb (2.0 kB) + .kicad_sch (215 B).\nNo component,\
  \ wire, net, zone or via was added; no DigiKey/JLCPCB call was made.\n\nkicad-cli --version (the binary\
  \ the server uses, ~/.local/bin/kicad-cli-10, a Flatpak bridge):\n  10.0.6\nThe host's system package\
  \ /usr/sbin/kicad-cli is a DIFFERENT binary and reports 9.0.7 \u2014 the server is\npointed at the 10.0.6\
  \ wrapper via KICAD_CLI, so KiCad v10 is what opened this project.\n\nValidation output (verbatim):\n\
  \  $ /home/astroboy/.local/bin/kicad-cli-10 sch erc -o /home/astroboy/kicad-validate-out/erc.rpt hello-kicad.kicad_sch\n\
  \  Found 0 violations\n  Saved ERC Report to /home/astroboy/kicad-validate-out/erc.rpt\n  -> erc.rpt:\
  \ \" ** ERC messages: 0  Errors 0  Warnings 0\"\n\n  $ kicad-cli-10 sch export netlist ...    -> exit\
  \ 0, tool \"Eeschema 10.0.6\", (components) and (nets) empty\n  $ kicad-cli-10 pcb export svg --layers\
  \ F.Cu,Edge.Cuts ... -> exit 0, 66 kB SVG\n\n  validate_schematic (structural scan, runKicadCli false):\n\
  \  {\"success\": true, \"valid\": true, \"errorCount\": 0, \"warningCount\": 0, \"issues\": [],\n  \
  \ \"kicadCli\": {\"ran\": false, \"reason\": \"not requested\"}, \"componentCount\": 0}\n\nOne real\
  \ finding, not a project defect: validate_schematic with runKicadCli=true reports\nkicadClI exitCode\
  \ 3 \"Schematic file does not exist or is not accessible\". The tool copies the\nschematic to os.tmpdir()\
  \ and points kicad-cli at the copy; the Flatpak KiCad sandbox has a private\n/tmp and /var/tmp (filesystems=home;/media;/run/media;),\
  \ so the copy is invisible inside it.\nReproduced directly: erc on /tmp/copy -> \"Failed to load schematic\"\
  \ (exit 3), on /var/tmp/copy ->\nsame, on $HOME/copy -> \"Found 0 violations\" (exit 0). Fixed locally\
  \ by adding\nTMPDIR=/home/astroboy/.cache/kicad-mcp/tmp to mcp_servers.kicad.env (backup: ~/.hermes/config.yaml.bak-0038);\n\
  takes effect on MCP server restart, so not re-verified in this run. This is the same sandbox trap already\n\
  recorded in the kicad-mcp-server skill (\"private /tmp \u2014 keep project paths under $HOME\").\n\n\
  No CI run exists for this branch: .github/workflows only fires on android-toolchain-setup and on\nfirmware/\
  \ paths, so the Actions API reports total_count 0 for hermes/0038-kicad-hello. Proof cites\nbranch+sha\
  \ only, no run pointer.\n\nFiles on the branch: hardware/hello-kicad/{hello-kicad.kicad_pro,.kicad_sch,.kicad_pcb,.net,.svg,erc.rpt,\n\
  .gitignore (*.kicad_prl),VALIDATION.md}. Task constraint honoured: nothing outside the project files\
  \ was pushed."
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

## Reply (Hermes)

Status: done

Server: mixelpixx/KiCAD-MCP-Server (Python/TS flavor), package.json version 2.7.0,
checkout SHA ac716d1a8bfad325b4aa93a398222645b3f78fd7 ("Merge pull request #411"),
dated 2026-09-09. Checkout at /home/astroboy/KiCAD-MCP-Server.

Registration: mcp_servers.kicad in ~/.hermes/config.yaml -> `node /home/astroboy/KiCAD-MCP-Server/dist/index.js`,
enabled: true. Tools appear in the client schema as mcp__kicad__* — count 237.

Tool count my client sees: 237 = 233 server tools (cache ~/.hermes/cache/mcp_schema_cache.json)
+ 4 Hermes utility tools (list_resources, read_resource, list_prompts, get_prompt).
The server's own inventory reports it differently: list_tool_categories says
total_categories 16, total_routed_tools 148, total_direct_tools 32 (= 180 named calls);
the routed ones are aliases into the same 233 registered tools.

Discovery call: list_tool_categories succeeded (16 categories, counts above); search_tools("project")
returned 6 direct matches (create_project, open_project, close_project, save_project,
snapshot_project, get_project_info).

Project created through MCP only: create_project {path: /home/astroboy/nordtronics/hardware/hello-kicad,
name: hello-kicad} -> success true, backend swig, wrote .kicad_pro + .kicad_pcb (2.0 kB) + .kicad_sch (215 B).
No component, wire, net, zone or via was added; no DigiKey/JLCPCB call was made.

kicad-cli --version (the binary the server uses, ~/.local/bin/kicad-cli-10, a Flatpak bridge):
  10.0.6
The host's system package /usr/sbin/kicad-cli is a DIFFERENT binary and reports 9.0.7 — the server is
pointed at the 10.0.6 wrapper via KICAD_CLI, so KiCad v10 is what opened this project.

Validation output (verbatim):
  $ /home/astroboy/.local/bin/kicad-cli-10 sch erc -o /home/astroboy/kicad-validate-out/erc.rpt hello-kicad.kicad_sch
  Found 0 violations
  Saved ERC Report to /home/astroboy/kicad-validate-out/erc.rpt
  -> erc.rpt: " ** ERC messages: 0  Errors 0  Warnings 0"

  $ kicad-cli-10 sch export netlist ...    -> exit 0, tool "Eeschema 10.0.6", (components) and (nets) empty
  $ kicad-cli-10 pcb export svg --layers F.Cu,Edge.Cuts ... -> exit 0, 66 kB SVG

  validate_schematic (structural scan, runKicadCli false):
  {"success": true, "valid": true, "errorCount": 0, "warningCount": 0, "issues": [],
   "kicadCli": {"ran": false, "reason": "not requested"}, "componentCount": 0}

One real finding, not a project defect: validate_schematic with runKicadCli=true reports
kicadClI exitCode 3 "Schematic file does not exist or is not accessible". The tool copies the
schematic to os.tmpdir() and points kicad-cli at the copy; the Flatpak KiCad sandbox has a private
/tmp and /var/tmp (filesystems=home;/media;/run/media;), so the copy is invisible inside it.
Reproduced directly: erc on /tmp/copy -> "Failed to load schematic" (exit 3), on /var/tmp/copy ->
same, on $HOME/copy -> "Found 0 violations" (exit 0). Fixed locally by adding
TMPDIR=/home/astroboy/.cache/kicad-mcp/tmp to mcp_servers.kicad.env (backup: ~/.hermes/config.yaml.bak-0038);
takes effect on MCP server restart, so not re-verified in this run. This is the same sandbox trap already
recorded in the kicad-mcp-server skill ("private /tmp — keep project paths under $HOME").

No CI run exists for this branch: .github/workflows only fires on android-toolchain-setup and on
firmware/ paths, so the Actions API reports total_count 0 for hermes/0038-kicad-hello. Proof cites
branch+sha only, no run pointer.

Files on the branch: hardware/hello-kicad/{hello-kicad.kicad_pro,.kicad_sch,.kicad_pcb,.net,.svg,erc.rpt,
.gitignore (*.kicad_prl),VALIDATION.md}. Task constraint honoured: nothing outside the project files was pushed.
