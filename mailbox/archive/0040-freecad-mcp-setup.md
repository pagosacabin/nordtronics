---
task_id: "0040"
status: staged
iteration: 1
proof:
  - branch: hermes/0040-freecad-hello
    sha: 7c69f9a54aa2adef0c080de1c64114aeae9cdbf6
notes: |
  Status: done. FreeCAD Robust MCP server (freecad-robust-mcp 0.6.1, spkane)
  registered in ~/.hermes/config.yaml as mcp_servers.freecad, running embedded
  headless inside the FreeCAD 1.1.1 snap's own Python 3.12; 83 tools discovered
  by Hermes (`hermes mcp test freecad` -> Connected, 83 tools). Test part built
  through MCP tools only: 60x40x20 mm block, centered 10 mm through-hole, all
  dimensions as named spreadsheet parameters; FCStd/STEP/STL on the branch.
  No CI run exists for this branch (see body), so the proof is branch + tip SHA
  only -- there is deliberately no `run:` pointer to resolve.
---

# 0040 — FreeCAD MCP: install, register, prove it

## Reply

**Status: done**

- **Branch:** `hermes/0040-freecad-hello` @ `7c69f9a54aa2adef0c080de1c64114aeae9cdbf6`
- **Files on branch:** `hardware/hello-freecad/hello-freecad.FCStd` (5322 B),
  `hardware/hello-freecad/hello-freecad.step` (8305 B),
  `hardware/hello-freecad/hello-freecad.stl` (7284 B), plus `README.md`. All
  re-read from origin with `git ls-tree -r --long origin/hermes/0040-freecad-hello`.
- **No CI run exists for this branch.** `.github/workflows/android-build.yml` is
  pinned to `branches: [android-toolchain-setup]`; `platformio.yml` is path-filtered
  to `firmware/**`. The Actions API agrees: `GET /repos/pagosacabin/nordtronics/actions/runs?branch=hermes/0040-freecad-hello`
  → `total_count: 0`. Hence proof is branch + tip SHA only, no `run:` pointer.

### 1. FreeCAD version and install method

`freecad.cmd --version` → `FreeCAD 1.1.1 Revision: 44227 +647 (Git)`.
MCP `get_freecad_version` → `{"version":"1.1.1","build_date":"44227 +647 (Git)","python_version":"3.12.3 ...","gui_available":0}`.

Install method: **snap**, channel `latest/stable`, revision **2741**
(`snap list` → `freecad 1.1-g0108fd4b 2741 latest/stable freecad-project-association**`),
console binary `freecad.cmd`, bundled CPython 3.12.3. I did **not** install or
upgrade FreeCAD. Machine is headless in this session (no GUI available).

### 2. Server choice and justification

Chosen: **spkane/freecad-addon-robust-mcp-server** (PyPI `freecad-robust-mcp`, 0.6.1, MIT).

- `bradsjm/freecad-embedded-mcp` is ruled out by the environment: it requires
  FreeCAD **1.1.3+** (installed snap is 1.1.1) *and* a running FreeCAD GUI, and its
  server binds inside that GUI process over HTTP. I am headless, so that one needs a
  FreeCAD upgrade plus a display before it can even start — two costs for no gain.
- `seansackowitz/mcp-freecad` (~70 tools) needs an XML-RPC bridge **workbench addon
  installed inside FreeCAD and a FreeCAD process running**; it is also the least
  actively maintained of the three.
- `spkane/…` has a real **`embedded` mode: `sys.path.insert(0, <FreeCAD lib>)` +
  `import FreeCAD` in the MCP server's own process** — genuinely headless, no GUI, no
  addon, no second process. The snap ships `FreeCAD.so` plus its matching Python 3.12,
  so in-process import is possible here; the only environment work was making the
  server run under the snap's 3.12 interpreter (below). Linux-only embedded mode is
  exactly my platform, and the repo is the most active/maintained of the three.

### 3. Install + registration

- `snap run freecad.pip install freecad-robust-mcp` → 0.6.1 into
  `~/snap/freecad/common/.local/lib/python3.12/site-packages`
  (the snap's own pip, so the package lands in the snap's 3.12 site-packages).
- Two dependency fixes were needed inside the snap, both real blockers found by running:
  1. `ImportError: cannot import name 'Sentinel' from 'typing_extensions'`
     (`/snap/freecad/2741/usr/lib/python3/dist-packages/typing_extensions.py`) — FreeCAD's
     snap `Init.py` re-prepends `dist-packages`, whose copy is older than pydantic_core
     needs. Fixed by importing the newer `typing_extensions` up front and re-prepending
     `/snap/freecad/current/lib/python3.12/site-packages` after FreeCAD loads.
  2. `ModuleNotFoundError: No module named 'mcp.server.fastmcp' ... This is mcp 2.x`
     — pip had installed `mcp 2.2.0`; the server is v1-API. Fixed with
     `snap run freecad.pip install "mcp<2"` → `mcp 1.30.0`.
- `FreeCAD.so` is ABI-bound to CPython 3.12 and this host has 3.11/3.14, so the server
  must run under the snap's interpreter. Launcher: `/home/astroboy/.local/bin/freecad-mcp-robust`
  → `snap run --shell freecad -c 'exec /snap/freecad/current/bin/python3.12 ~/.local/share/freecad-mcp/bootstrap.py'`
  with `FREECAD_MODE=embedded`. `bootstrap.py` also redirects stdout to stderr for the
  `import FreeCAD` step, because the snap's `SnapSetup/Init.py` **prints a banner to
  stdout** on import and that would corrupt the MCP stdio JSON-RPC stream.
- Registered in `~/.hermes/config.yaml` next to `kicad` (same `mcp_servers` pattern,
  backup kept at `~/.hermes/config.yaml.bak-0040`):

  ```yaml
  mcp_servers:
    freecad:
      command: /home/astroboy/.local/bin/freecad-mcp-robust
      args: []
      env:
        FREECAD_MODE: embedded
        FREECAD_PATH: /snap/freecad/current/usr/lib
        FREECAD_TIMEOUT_MS: "120000"
      connect_timeout: 120.0
      enabled: true
  ```

### 4. Reload / tool visibility

`hermes mcp list` shows both servers. `hermes mcp test freecad`:

```
  Transport: stdio → /home/astroboy/.local/bin/freecad-mcp-robust
  ✓ Connected (1618ms)
  ✓ Tools discovered: 83
```

**Server name:** `freecad`; **server:** freecad-robust-mcp **0.6.1**
(reports `serverInfo: {"name": "freecad-mcp", "version": "1.30.0"}` — that is the
MCP SDK version it advertises, not its own); **tool count: 83** (independently
confirmed by my own `tools/list` client: 83).

Honest caveat: the tools are registered and discoverable, but they are *not* injected
into an already-running agent session — this cron session picked up no new tools, so
the part below was driven with an MCP stdio client (`~/.local/share/freecad-mcp/mcpcl.py`)
speaking JSON-RPC to the exact command Hermes spawns. Every modelling step is an MCP
tool call; no FreeCAD Python was run outside the MCP server process.

### 5. Test part (MCP tools only)

`create_document` → `create_object(Spreadsheet::Sheet "Params")` → `create_box` →
`create_cylinder` → `boolean_operation(cut)` → `save_document` → `export_step` →
STL export, with the expression wiring done through the `execute_python` MCP tool.

Named parameters (all geometry bound by expressions; nothing hardcoded in the tree):

| Cell | Alias            | Value | Drives                                   |
|------|------------------|-------|------------------------------------------|
| A1   | `block_length`   | 60    | `Block.Length`, hole X centre (`/2`)     |
| A2   | `block_width`    | 40    | `Block.Width`, hole Y centre (`/2`)      |
| A3   | `block_height`   | 20    | `Block.Height`, hole height (`+2*clr`)   |
| A4   | `hole_diameter`  | 10    | `Hole.Radius` (`hole_diameter / 2`)      |
| A5   | `hole_clearance` | 10    | hole over-travel top/bottom (through)    |

Verification (in-session, via `execute_python`):
`bbox: (0,0,0)-(60,40,20)`, `volume 46429.20 mm^3` vs analytic `46429.20` (delta 0.0000),
`1` valid solid. Change test: `block_length` 60→80 → bbox 80x40x20 / vol 62429.2;
`hole_diameter` 10→16 → 59978.8 mm³ (matches analytic 59978.8); both restored to 60/10
before saving/exporting.

### 6. Server bug found (reported, not hidden)

The server's `export_stl` (also `export_3mf`, `export_obj` — same template) fails on
FreeCAD 1.1.1:

```
File "<mcp>", line 30, in <module>
IndexError: list index out of range
```

Root cause, verified in-session: `Shape.tessellate(tol)` returns
`([8 vertices], [12 index triples])`, and the tool passes element `[0]` — the **vertex
list** — to `Mesh.Mesh().addFacets()`, which expects facets as **point triples**. Proof:

```
addFacets(t[0] (vertices))            -> IndexError: list index out of range
addFacets(list(t[1]) (index triples)) -> TypeError: expect a sequence of floats or Vector
addFacets(point triples)              -> ok, facets=12   # [tuple(t[0][i] for i in tri) for tri in t[1]]
```

For task 0040 I did **not** patch the installed package; the STL was produced through
the `execute_python` MCP tool with `MeshPart.meshFromShape(..., LinearDeflection=0.05,
AngularDeflection=0.5, Relative=False)` → `mesh facets: 144`, written to
`hardware/hello-freecad/hello-freecad.stl`. `export_step` and `save_document` worked
out of the box (`save_document` takes `path`, not `file_path`).

Everything here is free/OSS; no spending, no license purchases. This is a test part —
no fabrication data, and it has not been reviewed by Stephen.
