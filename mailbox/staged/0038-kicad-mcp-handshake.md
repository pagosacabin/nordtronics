---
task_id: "0038"
status: staged
iteration: 1
proof:
  - branch: hermes/0038-kicad-hello
    sha: 3704fff05099cdc7ab2824c3e72537d408ea4768
notes: |
  Status: done

  Server: mixelpixx/KiCAD-MCP-Server (Python/TS), package.json version 2.7.0, checkout SHA
  ac716d1a8bfad325b4aa93a398222645b3f78fd7, at /home/astroboy/KiCAD-MCP-Server.
  Registration: mcp_servers.kicad in ~/.hermes/config.yaml (node dist/index.js, enabled true).
  Tool count my client sees: 237 = 233 server tools + 4 Hermes utility tools.
  kicad-cli --version: 10.0.6 (Flatpak bridge ~/.local/bin/kicad-cli-10, via KICAD_CLI).
  Created through MCP only: create_project -> hardware/hello-kicad (blank; ERC 0 violations, netlist
  components/nets empty, opens and plots under KiCad 10.0.6). No CI run exists for this branch:
  no workflow in .github/workflows matches it, so proof is branch+sha only, no run pointer.
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

- **Status: done**

### Server version + checkout SHA
- Repo `mixelpixx/KiCAD-MCP-Server` (Python/TypeScript flavor, MIT), checkout `/home/astroboy/KiCAD-MCP-Server`
- `git log -1 --format=%H` -> `ac716d1a8bfad325b4aa93a398222645b3f78fd7` ("Merge pull request #411 from AmirF194/fix/399-edit-component-footprint-exchange", 2026-09-09)
- `package.json` -> `"name": "kicad-mcp"`, `"version": "2.7.0"`

### Registration + tool count my client sees
- `mcp_servers.kicad` in `~/.hermes/config.yaml`: `command: node`, `args: [/home/astroboy/KiCAD-MCP-Server/dist/index.js]`, `enabled: true`; tools reach the client as `mcp__kicad__*`.
- **237 tools** = 233 server tools (from `~/.hermes/cache/mcp_schema_cache.json`, the discovery snapshot) + 4 Hermes utility tools (`list_resources`, `read_resource`, `list_prompts`, `get_prompt`).
- The server counts differently about itself: `list_tool_categories` -> `total_categories: 16`, `total_routed_tools: 148`, `total_direct_tools: 32` (= 180 named calls; the "routed" ones are aliases onto the same 233 registered tools).

### Discovery call
- `list_tool_categories` succeeded: 16 categories (export 27, schematic 26, symbol_library 17, board 15, component 15, drc 7, library 7, autoroute 4, ...).
- `search_tools` with `query: "project"` -> 6 matches, all direct: `create_project`, `open_project`, `close_project`, `save_project`, `snapshot_project`, `get_project_info`.

### Project: created through MCP tool calls only
`create_project {path: /home/astroboy/nordtronics/hardware/hello-kicad, name: hello-kicad}` ->
`{"success": true, "message": "Created project: hello-kicad", "_backend": "swig"}`; it wrote
`hello-kicad.kicad_pro` (1868 B), `hello-kicad.kicad_pcb` (2062 B), `hello-kicad.kicad_sch` (215 B).
Nothing was added afterwards: 0 components, 0 wires, 0 nets, no zone/via, no DigiKey or JLCPCB call.

### `kicad-cli --version` output
```
$ /home/astroboy/.local/bin/kicad-cli-10 --version
10.0.6
```
The binary the server is pointed at (`KICAD_CLI`, a Flatpak bridge) is KiCad 10.0.6. Note the host's
system package is a *different* binary: `/usr/sbin/kicad-cli --version` -> `9.0.7`.

### Validation output (verbatim)
```
$ kicad-cli-10 sch erc -o /home/astroboy/kicad-validate-out/erc.rpt hello-kicad.kicad_sch
Found 0 violations
Saved ERC Report to /home/astroboy/kicad-validate-out/erc.rpt
```
`erc.rpt`: ` ** ERC messages: 0  Errors 0  Warnings 0`

```
$ kicad-cli-10 sch export netlist -o hello-kicad.net hello-kicad.kicad_sch      # exit 0
   (design (tool "Eeschema 10.0.6") ... (components) (nets))   <- both empty: blank sheet
$ kicad-cli-10 pcb export svg --layers F.Cu,Edge.Cuts -o hello-kicad.svg hello-kicad.kicad_pcb   # exit 0
   Plotted to '.../hello-kicad.svg'. Done.        (66469 B)
```
`validate_schematic` (structural scan, `runKicadCli: false`):
```json
{"success": true, "valid": true, "errorCount": 0, "warningCount": 0, "issues": [],
 "kicadCli": {"ran": false, "reason": "not requested"}, "componentCount": 0, "semanticChecksRan": true}
```

### Branch + full SHA
- `hermes/0038-kicad-hello` on origin at `3704fff05099cdc7ab2824c3e72537d408ea4768` (verified with `git ls-remote`).
- Files: `hardware/hello-kicad/{hello-kicad.kicad_pro,hello-kicad.kicad_sch,hello-kicad.kicad_pcb,hello-kicad.net,hello-kicad.svg,erc.rpt,VALIDATION.md,.gitignore}`. Nothing else was pushed.
- **No CI run exists for this branch**: `.github/workflows` only fires on `android-toolchain-setup` and on `firmware/**` paths, and the Actions API reports `total_count: 0` for `hermes/0038-kicad-hello`. Proof therefore cites branch+sha only — no run pointer.

### Finding: `validate_schematic(runKicadCli: true)` and the Flatpak private tmp
With `runKicadCli: true` the tool returns `valid: false`, `kicadCli.exitCode: 3`,
`"Schematic file does not exist or is not accessible"`. The tool copies the schematic to `os.tmpdir()`
before calling kicad-cli, and the Flatpak KiCad sandbox has a private `/tmp` + `/var/tmp`
(`flatpak info --show-permissions org.kicad.KiCad` -> `filesystems=home;/media;/run/media;`). Reproduced
directly on an identical copy in each location:
```
$ kicad-cli-10 sch erc ... /tmp/probe-copy.kicad_sch       -> Failed to load schematic        (exit 3)
$ kicad-cli-10 sch erc ... /var/tmp/probe-sch.kicad_sch    -> Failed to load schematic        (exit 3)
$ kicad-cli-10 sch erc ... /home/astroboy/tmpprobe/p.kicad_sch -> Found 0 violations          (exit 0)
```
So this is a temp-copy visibility trap, not a defect in the project (kicad-cli reads the real project
path fine). Fix applied locally: `TMPDIR=/home/astroboy/.cache/kicad-mcp/tmp` added to
`mcp_servers.kicad.env` (config backed up to `~/.hermes/config.yaml.bak-0038`); it takes effect when the
MCP server restarts, so it is **not** re-verified in this run. Same class of trap as the "private /tmp"
note in the `kicad-mcp-server` skill.
