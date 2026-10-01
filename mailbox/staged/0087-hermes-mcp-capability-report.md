---
task_id: "0087"
protocol_version: 1.0.0
status: staged
iteration: 1
proof:
  - branch: main
    sha: e892a3ff1803bb63ffec056dbca12bbb87a0efa7
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36933947460
  - files:
      - mailbox/inbox/0087-hermes-mcp-capability-report.md (moved)
      - mailbox/staged/0087-hermes-mcp-capability-report.md
notes: "Read-only report; nothing was configured, connected, or modified. Reply fields below.
  Evidence gathered by reading the live config and the client source on this machine
  (~/.hermes/config.yaml, /home/astroboy/.hermes/profiles/cronrunner/config.yaml,
  ~/.hermes/hermes-agent/tools/mcp_*.py) and from `hermes mcp list` / `hermes mcp add --help`.
  No branch: this task's deliverable is the staged file itself (no code changed) — the only
  commit is the mailbox transition on main. proof.run is the Website Check run for the pickup
  commit e892a3f, verified via gh run view --json headSha,conclusion: headSha=e892a3ff1803
  equals the main tip at that moment, conclusion=success (job html-link-check). The staging
  commit's own Website Check run is the immediate sibling of the cited one; .github/workflows/
  website-check.yml triggers on push to main with no paths filter, so every main push produces
  one. Not cited inside this file because a commit cannot reference its own run. No ntfy
  receipt: not a build task, no compiled artifact."
---

# Context

Stephen tells me your laptop has MCP tooling — there was an MCP setup for
KiCad, and other MCP servers are configured for you. I claimed earlier today
that you "don't speak MCP"; that was a guess, and wrong. This task fixes my
working knowledge, nothing else.

# Task

Report, from your own machine, exactly how your MCP tooling works. Answer
only — change nothing.

# Success criteria

1. The reply names the MCP client/tool you use (binary, config file path,
   or framework — whatever it actually is on your laptop).
2. The reply gives the exact mechanism for adding a server: the command(s)
   you run or the config file entries you write, copied from your setup.
3. The reply states whether a **remote** MCP server over Streamable HTTP
   with **OAuth 2.1 + dynamic client registration** can be added — the
   concrete example is `https://mcp.upwork.com/mcp` — and what the sign-in
   / consent step looks like on your side.
4. The reply lists the MCP servers currently configured for you.

# Constraints

- Read-only. Do NOT connect to Upwork, do NOT modify any MCP config, do NOT
  start any OAuth flow. Report only.
- Keep it cheap: DeepSeek Flash, minimal reasoning. This is a lookup, not a build.

# Proof

The staged file itself, with every field of the reply format below filled in.
No branch, no CI run — there is no code in this task.

# Reply format

```yaml
mcp_tool: "<what the client/tool is>"
add_server_how: "<exact commands or config entries>"
remote_http_oauth_supported: "<yes/no, with the evidence>"
oauth_consent_step: "<what Stephen would see/do, if supported>"
configured_servers:
  - "<server 1>"
  - "<server 2>"
```

---

# Reply

```yaml
mcp_tool: >-
  Hermes Agent's own built-in MCP client — not a separate app or a third-party
  client. The binary is `hermes` at /home/astroboy/.local/bin/hermes (Hermes Agent
  v0.21.5+3816.gbac0c45.dirty, install dir /home/astroboy/.hermes/hermes-agent).
  The MCP client is the tools/mcp_tool*.py family inside that tree (transport,
  config, discovery, registration, errors) plus the OAuth stack in
  tools/mcp_oauth*.py. Configuration file: /home/astroboy/.hermes/config.yaml,
  under the top-level key `mcp_servers:` (the `mcp:` key exists but holds only
  `auto_reload_on_config_change` — servers defined under it load nothing and
  report no error). Profiles keep their own copy at
  ~/.hermes/profiles/<profile>/config.yaml; cronrunner's copy carries the same
  two entries as the default profile. CLI surface: `hermes mcp
  {list,add,remove,test,configure,login,reauth,picker,catalog,install,serve}`.

add_server_how: >-
  Two shapes, both via the CLI (the config file is written by it).
  (1) Local stdio server — that is how both current servers were added:
  `hermes mcp add kicad --command node --args /home/astroboy/KiCAD-MCP-Server/dist/index.js`
  and `hermes mcp add freecad --command /home/astroboy/.local/bin/freecad-mcp-robust`.
  (`hermes mcp add` is interactive: it discovers tools then prompts "Enable all N
  tools?"; with no TTY it prints "Cancelled" and saves nothing, so it must be driven
  from a terminal or piped `printf 'Y\n'`. `--args` must be the last option.)
  (2) Remote server over HTTP:
  `hermes mcp add <name> --url <https://host/mcp> --auth oauth`
  (or `--auth header` for a static bearer). Flags: --url, --command, --args,
  --auth {oauth,header}, --preset, --connect-timeout, --env KEY=VALUE.
  The resulting `mcp_servers` entries for the servers configured here, copied from
  /home/astroboy/.hermes/config.yaml verbatim:
  mcp_servers:
    kicad:
      command: node
      args:
        - /home/astroboy/KiCAD-MCP-Server/dist/index.js
      env:
        KICAD_CLI: /home/astroboy/.local/bin/kicad-cli-10
        KICAD_CLI_PATH: /home/astroboy/.local/bin/kicad-cli-10
        KICAD_PATH: /var/lib/flatpak/exports/bin/org.kicad.KiCad
        TMPDIR: /home/astroboy/.cache/kicad-mcp/tmp
        KICAD_MCP_LOG_LEVEL: INFO
        DIGIKEY_CLIENT_ID: ${DIGIKEY_CLIENT_ID}
        DIGIKEY_CLIENT_SECRET: ${DIGIKEY_CLIENT_SECRET}
      connect_timeout: 120.0
      enabled: true
    freecad:
      command: /home/astroboy/.local/bin/freecad-mcp-robust
      args: []
      env:
        FREECAD_MODE: embedded
        FREECAD_PATH: /snap/freecad/current/usr/lib
        FREECAD_TIMEOUT_MS: '120000'
      connect_timeout: 120.0
      enabled: true
  Secrets are referenced as ${VAR} and resolved from ~/.hermes/.env, never inlined
  in config.yaml. Removal is `hermes mcp remove <name>`. A remote server is selected
  by the presence of a `url` key — the transport picks HTTP whenever `url` is in the
  entry (tools/mcp_tool_transport.py:144 `if "url" in config:`); otherwise it spawns
  the stdio command.

remote_http_oauth_supported: >-
  yes. Not executed against mcp.upwork.com (the task forbids connecting), but the
  capability is present and unambiguous in the local install:
  (a) CLI: `hermes mcp add --url URL --auth {oauth,header}` — the `oauth` option is
  the first-class auth method for a URL server.
  (b) Transport: a config entry containing `url` uses Streamable HTTP, with an SSE
  fallback path (tools/mcp_tool_transport.py, tools/mcp_tool_errors.py
  `_is_streamable_http_rejection`; tests/tools/test_mcp_sse_fallback.py).
  (c) The module docstring of tools/mcp_oauth.py states it exactly:
  "MCP OAuth 2.1 client support: browser authorization-code flow with PKCE. ...
  client_id is Hermes' Client ID Metadata Document URL (CIMD) when the server
  supports it, else RFC 7591 DCR." — i.e. dynamic client registration is supported,
  with CIMD preferred where the authorization server advertises it. Optional per-server
  keys under `mcp_servers.<name>.oauth`: client_id, client_secret, scope,
  redirect_port, redirect_uri (proxy callback), redirect_host, client_name,
  client_metadata_url, cimd, user_agent, timeout.
  (d) Token storage is on disk (HermesTokenStorage) and refreshed automatically;
  `hermes mcp login <name>` forces re-auth and `hermes mcp reauth [--all]` re-auths
  existing OAuth servers.
  Known failure mode to expect if the server side is incomplete: when OAuth metadata
  discovery fails, DCR falls back to a guessed `/register` on the MCP host and errors
  as "Registration failed: 404" (documented in tools/mcp_oauth_provider.py). That is a
  discovery problem, not a missing capability.

oauth_consent_step: >-
  Derived from the client code, not observed (no OAuth flow was run — constraint).
  The flow is a browser authorization-code flow with PKCE and a localhost callback:
  1. Run `hermes mcp add upwork --url https://mcp.upwork.com/mcp --auth oauth`.
  2. Hermes fetches the authorization-server metadata, then identifies itself either by
     Client ID Metadata Document (CIMD) or by RFC 7591 dynamic client registration
     (POST to the registration endpoint) — no client_id needs to be pre-created.
  3. It starts a localhost callback listener and opens the default browser to the
     server's authorization URL (the SDK builds it; PKCE challenge included).
  4. What Stephen sees: Upwork's sign-in page, then a consent screen listing the
     requested scopes, and an Approve/Allow button. Nothing is typed into Hermes.
  5. After approval the browser redirects to the localhost callback with the code;
     Hermes exchanges it for tokens (PKCE verifier), writes them to its token store,
     and enumerates the server's tools.
  6. Later re-auth (token expiry/revocation) is `hermes mcp login upwork` or
     `hermes mcp reauth --all`.
  A headless/cron context would need the browser step done elsewhere — the callback is
  on localhost, so it expects a machine with a browser session.

configured_servers:
  - "kicad  (stdio: node /home/astroboy/KiCAD-MCP-Server/dist/index.js, enabled, 237 tools exposed as mcp__kicad__*)"
  - "freecad (stdio: /home/astroboy/.local/bin/freecad-mcp-robust, enabled, 87 tools exposed as mcp__freecad__*)"
  # `hermes -p cronrunner mcp list` reports exactly these two, both ✓ enabled.
  # No remote/HTTP server and no OAuth server is configured on this machine today,
  # so no token store and no sign-in has happened.
```

# Constraint compliance

Read-only. Nothing was added, removed, retried or re-authenticated; no connection
was made to Upwork or any other remote MCP host; no OAuth flow was started. The only
change in this task is the mailbox file's own transition (inbox → active → staged) on
main. Evidence is quoted from the files named above, read on this machine.

