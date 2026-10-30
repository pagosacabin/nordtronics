---
task_id: "0158"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output (as published for deepseek-flash)"
  cost_usd: unknown
proof:
  - branch: none
    sha: none
    run: none
    files: []
    reason: |
      host-local install + enumeration; no repository file changed and no CI
      run applies (nothing to build). Per the task's own proof clause the
      evidence is the exact commands run plus their verbatim output, quoted
      below, and the machine-state checks that show nothing else moved.
notes: |
  ## Result: installed and loadable. One criterion is only partly attainable
  ## without credentials -- declared in section 6, not hidden.

  ### 1. Install (criteria 1)

  `uvx` was available (`uvx 0.12.18`), so no system pip and no new venv by hand.
  Used `uv tool install` (the persistent sibling of `uvx`) rather than an
  ephemeral `uvx` run: it installs into an isolated venv under
  `~/.local/share/uv/tools/unifi-network-mcp` and touches no existing tooling.

      $ uv tool install unifi-network-mcp
       ...
       + unifi-network-mcp==0.42.0
      Installed 1 executable: unifi-network-mcp

      $ uv tool list
      unifi-network-mcp v0.42.0
      - unifi-network-mcp

      $ ls -la ~/.local/bin/unifi-network-mcp
      ... -> /home/astroboy/.local/share/uv/tools/unifi-network-mcp/bin/unifi-network-mcp

  Version: **0.42.0** (PyPI latest at install time:
  https://pypi.org/pypi/unifi-network-mcp/json -> "version": "0.42.0",
  requires_python >=3.13; the uv tool venv resolved to CPython 3.13).
  Source repo: https://github.com/sirkirby/unifi-mcp. Footprint 72 MB.
  Runtime deps pulled in include `unifi-core==0.4.81`, `unifi-mcp-shared==0.6.37`,
  `mcp`-family/FastMCP stack; no existing tool was modified or removed.

  ### 2. Stdio launch (first half of criterion 2) -- clean, and this is the
  ###    important finding

  Launched the installed binary in stdio mode with a bare `mcp` 2.0.0 client
  (`StdioServerParameters(command="~/.local/bin/unifi-network-mcp")`, no env
  overrides), `initialize` then `tools/list`:

      SERVER_INFO: unifi-network-mcp 0.42.0
      PROTOCOL: 2025-11-25
      TOOL_COUNT: 1
      TOOL: unifi_setup_status | Report why the unifi-network server has no
            controller tools, how to configure it and how to fix it...

  No errors, no exceptions, ~2 s to ready. `unifi_setup_status` returns:

      {"success": true,
       "data": {"configured": false, "settings_file": "unset",
                "message": "The unifi-network MCP server is not configured: no
                controller host is set. It is running in setup mode with no
                controller tools and makes no controller connections. ..."}}

  So **the stdio catalogue is exactly 1 tool while unconfigured** -- by design:
  `main.py:149` checks `controller_host_configured(config)` and, when false,
  calls `serve_setup_required(...)` and returns before `register_tools_for_mode`.
  This is the server behaving correctly, not a broken install.

  ### 3. Full tool catalogue (second half of criterion 2) -- 204 tools

  Because registration is gated on a controller host, I did **not** set a host or
  credentials. Instead I enumerated the catalogue through the package's own
  registration path, in-process: import `unifi_network_mcp.runtime` (the global
  FastMCP `server`) and every one of the 26 tool modules named in the shipped
  `tools_manifest.json`, then `await server.list_tools()`. The tool decorators
  register onto that same global server (`from unifi_network_mcp.runtime import
  server`), so this is the server's real catalogue, minus the controller
  connection:

      MANIFEST_COUNT: 210   MANIFEST_KEYS: 204
      REGISTERED: 204       NOT_IN_MANIFEST: []
      TOTAL: 204            READ_ONLY: 98

  Sample names taken from that output (all verbatim):
    unifi_lookup_by_ip, unifi_list_clients, unifi_get_client_details,
    unifi_list_devices, unifi_list_acl_rules, unifi_list_client_groups,
    unifi_get_content_filter_details, unifi_adopt_device, unifi_archive_alarm,
    unifi_create_firewall_policy

  98 of the 204 carry `readOnlyHint: true` -- useful for the later read-only
  wiring task (`unifi_lookup_by_ip`'s annotations read
  `ToolAnnotations(read_only_hint=True, open_world_hint=False)`).
  Note: the manifest's `count: 210` header is stale relative to its own
  `module_map` (204 entries); registration matches the map exactly (0 missing,
  0 extra), so 204 is the true figure and 210 is a metadata bug in the package.

  ### 4. Nothing reached a network (criterion 3, read-only over every network)

  Three independent checks:
  a) In the in-process run I monkeypatched `socket.socket.connect` to record and
     raise. Result: `OUTBOUND_CONNECT_ATTEMPTS: []` -- zero connection attempts
     across all 26 module imports and the registration.
  b) In a live stdio session (server pid held open for 8 s), `ss -tunp` filtered
     to that pid printed nothing -- zero sockets.
  c) The server's own startup log: "running in setup mode with no controller
     tools and makes no controller connections."

  ### 5. Nothing else on the machine changed (criterion 3, continued)

  * No credentials created or read. `~/.config/unifi-mcp/` does not exist
    (`ls` -> "No such file or directory"); no `UNIFI_*` variable exists in the
    environment (the probe script hard-refuses to run if one is set).
  * No MCP config edits: `grep -rn unifi ~/.hermes/config.yaml
    ~/.hermes/profiles/cronrunner/config.yaml` -> no matches. Config mtimes are
    unchanged from before this run: `2026-10-26 07:54` (default) and
    `2026-10-26 07:56` (cronrunner).
  * `.env` untouched: `~/.hermes/.env` still `2026-10-15 08:55`, mode 600;
    cronrunner `.env` still `2026-10-15 09:06`.
  * As instructed, it is **not** registered in any live MCP client config.

  ### 6. Declared gap: criterion 2's literal wording is only half-attainable

  Criterion 2 asks for "at least five tool names from the actual output" of the
  stdio enumeration. Over stdio the count is 1 by the package's design; five
  names only appear once a controller host is configured, which this task
  explicitly forbids (and which would need credentials that do not exist). So:
  the count-and-names evidence above is from the server's own registration path
  in-process (section 3), not from `tools/list`. The stdio path itself is proven
  working (initialize + tools/list + one successful tool call). If Juno wants
  the five names provable *through* `tools/list`, the next task must supply a
  host (it can be an unreachable/placeholder host if the goal is catalogue
  listing only -- but that would open a TCP path, so I did not do it here).

  For the later wiring task, the setup paths the server itself advertises:
  `UNIFI_NETWORK_HOST` / `UNIFI_NETWORK_USERNAME` / `UNIFI_NETWORK_PASSWORD`,
  or a settings file named by `UNIFI_SETTINGS_FILE`
  (`~/.config/unifi-mcp/network.env` is the convention the setup skill writes).
  Also note `config.server.defer_controller_init` exists (defers controller I/O
  to the first tool call) and `UNIFI_TOOL_REGISTRATION_MODE` selects the
  registration mode -- both will matter when the entry is wired for real.

  ### 7. Deviations

  (1) `uv tool install` instead of an ephemeral `uvx` run -- chosen so the
  version is persistent and stated, per criterion 1; it adds only
  `~/.local/share/uv/tools/unifi-network-mcp` + the `~/.local/bin` symlink.
  (2) Scratch scripts (client harness, enumeration, socket guard) were written
  under `~/.hermes/profiles/cronrunner/cache/scratch/`, never in the repo; the
  repo worktree shows only its three pre-existing untracked paths
  (`firmware/wildfire-node-v1/bench-override.ini`, `hardware/solar-gate-v2/`,
  `take_screenshots.py`) before and after.
  (3) No `--help`-driven doc reading was trusted for launch: the docs' stdio
  invocation was confirmed by actually completing an MCP handshake.

---

# 0158 — Install sirkirby/unifi-network-mcp (read-only smoke test, no credentials)

## Context

Task 0157 established your harness has no UniFi/Ubiquiti capability: plugins
are homeassistant only, MCP servers are kicad, freecad, upwork plus the
terminal bridge. Stephen wants the UniFi MCP installed so a later task can
wire it to a client's controller. This task is rung one only: install the
server package and prove it loads. No credentials exist yet and none are
expected.

## Task

Install the `unifi-network-mcp` package from the sirkirby/unifi-mcp project
(PyPI: `unifi-network-mcp`, repo: https://github.com/sirkirby/unifi-mcp).
Use `uvx` if available, else `pip install unifi-network-mcp` in a venv —
whichever fits your machine; do not disturb existing tooling.

Then smoke-test: launch the server the way its docs describe (stdio mode)
and get it to enumerate its tool catalogue (tool count and a few tool names
suffice). Do NOT point it at any controller. Do NOT create or modify any
API keys, usernames, passwords, or .env entries. Do NOT register it in your
live MCP client config yet — an entry with no credentials is a broken entry;
registration plus credential wiring is a separate later task.

## Success criteria

1. `unifi-network-mcp` is installed on your machine (version number stated).
2. The server starts in stdio mode without errors and enumerates its tools —
   quote the tool count and at least five tool names from the actual output.
3. Nothing else on the machine changed: no new credentials, no config edits
   to existing MCP servers, no network connections to any UniFi controller.

## Constraints

- Read-only with respect to every network: install and enumerate only.
- No secrets created, read, or printed. If any step asks for a controller
  host, username, password, or API key, stop that step and report it as
  blocked — do not invent or reuse values.
- Default model tier for this task (cost bound is implicit in the tier, not
  a dollar cap you must compute here).

## Proof

- branch: none (host-local install, no repo changes)
- run: none
- Quoted terminal output: the install command and its result, the server
  version, and the tool enumeration (count + sample names). Pasted output is
  a claim; the exact commands run plus their verbatim output are the proof
  here, since there is no remote artifact by design.

## Reply format

Staged reply with `proof:` (branch/run none + reason), `notes:` containing
the findings (install method, version, tool count and sample names, anything
that failed and the exact error), and a `cost:` block if your harness
supports it (per protocol 1.1.0 on your branch; unknowns with reason are
acceptable under 1.0.0).