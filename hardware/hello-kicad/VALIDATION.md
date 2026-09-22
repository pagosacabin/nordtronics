# 0038 — KiCad MCP handshake — validation evidence

Project `hello-kicad` was created entirely through MCP tool calls against the
MixelPix KiCAD-MCP-Server. No file in this directory was hand-written; the only
file added by hand is this evidence log.

## Server under test

| Field | Value |
|---|---|
| Repo | `mixelpixx/KiCAD-MCP-Server` (checkout `/home/astroboy/KiCAD-MCP-Server`) |
| Checkout SHA | `ac716d1a8bfad325b4aa93a398222645b3f78fd7` |
| `package.json` version | `2.7.0` |
| Registration | `mcp_servers.kicad` in `~/.hermes/config.yaml`, `command: node dist/index.js`, `enabled: true` |

## MCP calls made (in order)

1. `list_tool_categories` → `total_categories: 16`, `total_routed_tools: 148`, `total_direct_tools: 32`
2. `search_tools` (`query: "project"`) → 6 matches, all direct tools
   (`create_project`, `open_project`, `close_project`, `save_project`,
   `snapshot_project`, `get_project_info`)
3. `create_project` `{path: /home/astroboy/nordtronics/hardware/hello-kicad, name: hello-kicad}`
   → `success: true`, backend `swig`, wrote `.kicad_pro` + `.kicad_pcb` + `.kicad_sch`
4. `validate_schematic` (`runKicadCli: false`) → `valid: true`, 0 errors, 0 warnings
5. `validate_schematic` (`runKicadCli: true`) → see known issue below

## Validation, verbatim

`~/.local/bin/kicad-cli-10 --version`:

```
10.0.6
```

`/usr/sbin/kicad-cli --version` (host system package, distinct binary):

```
9.0.7
```

`kicad-cli-10 sch erc`:

```
$ kicad-cli-10 sch erc -o erc.rpt hello-kicad.kicad_sch
Found 0 violations
Saved ERC Report to /home/astroboy/kicad-validate-out/erc.rpt
```

`erc.rpt` (committed alongside):

```
ERC report (2026-09-22T14:19:51, Encoding UTF8)
Report includes: Errors, Warnings

***** Sheet /

 ** ERC messages: 0  Errors 0  Warnings 0
```

`kicad-cli-10 sch export netlist` → exit 0, `tool "Eeschema 10.0.6"`,
`(components)` / `(nets)` both empty, i.e. a blank sheet (committed as
`hello-kicad.net`).

`kicad-cli-10 pcb export svg --layers F.Cu,Edge.Cuts` → exit 0, 66 kB SVG
(committed as `hello-kicad.svg`).

`validate_schematic` with the tool's own structural scanner:

```json
{"success": true, "valid": true, "errorCount": 0, "warningCount": 0,
 "issues": [], "kicadCli": {"ran": false, "reason": "not requested"},
 "componentCount": 0, "semanticChecksRan": true, "_backend": "swig"}
```

## Known issue: validate_schematic(runKicadCli=true) vs the Flatpak sandbox

With `runKicadCli: true` the tool reports:

```json
{"success": true, "valid": false, "errorCount": 1,
 "message": "hello-kicad.kicad_sch is invalid: 1 error(s), first: Structure scan found nothing, but kicad-cli refused the file: Schematic file does not exist or is not accessible",
 "kicadCli": {"ran": true, "ok": false, "exitCode": 3,
              "output": "Schematic file does not exist or is not accessible"}}
```

The tool copies the schematic to `os.tmpdir()` and points kicad-cli at the copy.
`kicad-cli-10` is a bridge script running KiCad 10 through Flatpak, whose sandbox
gives the app a **private `/tmp` and `/var/tmp`** (`flatpak info
--show-permissions org.kicad.KiCad` → `filesystems=home;/media;/run/media;`,
`[Environment] TMPDIR=/var/tmp`). A copy under the host `/tmp` is therefore
invisible inside the sandbox. Reproduced directly:

```
$ kicad-cli-10 sch erc -o out.rpt /tmp/probe-copy.kicad_sch        # exists on host
Failed to load schematic            (exit 3)
$ kicad-cli-10 sch erc -o out.rpt /var/tmp/probe-sch.kicad_sch     # exists on host
Failed to load schematic            (exit 3)
$ kicad-cli-10 sch erc -o out.rpt /home/astroboy/tmpprobe/p.kicad_sch
Found 0 violations                  (exit 0)
```

Fix applied: `TMPDIR=/home/astroboy/.cache/kicad-mcp/tmp` added to the
`mcp_servers.kicad.env` block in `~/.hermes/config.yaml`, so the scratch copy
lands under `$HOME` where the sandbox can read it. Takes effect when the MCP
server (Hermes) is restarted; not re-verified in this run. `kicad-cli` itself
never had a problem with the real project path — the failure was the temp copy,
not the project.
