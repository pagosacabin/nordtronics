---
task_id: "0157"
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
    reason: read-only host-local capability question; the evidence is the quoted config and tool names below, not an artifact
notes: |
  ## Findings

  ### 1. plugins (`~/.hermes/config.yaml`, line 376; cronrunner profile line 356)

  Default profile:
  ```
  plugins:
    enabled:
      - homeassistant
    disabled: []
  ```
  Worker (`cronrunner`) profile adds one more:
  ```
  plugins:
    enabled:
      - deepseek-search-fetch
      - homeassistant
    disabled: []
  ```
  No UniFi/Ubiquiti plugin in either. Plugin directories on disk match exactly:
  `~/.hermes/plugins/` holds `homeassistant` only; `~/.hermes/profiles/cronrunner/plugins/`
  holds `deepseek-search-fetch` and `homeassistant`.

  ### 2. tools / MCP servers

  `hermes mcp list` (identical in both profiles):
  ```
  Name      Transport                       Tools  Status
  kicad     node /home/astroboy/KiCAD...    all    ✓ enabled
  freecad   /home/astroboy/.local/bin...    all    ✓ enabled
  upwork    https://mcp.upwork.com/mcp       all    ✓ enabled
  ```
  Deferred tool catalogue: 361 tools, all from those three sources (kicad 237,
  freecad 87, upwork 36) plus the `terminal` bridge. No UniFi source.
  Skills: the default profile's tree (30 categories) has no networking-device skill —
  the closest are `homeassistant`, `home-automation` and `monitoring`, and none of
  them mention UniFi. The worker (`cronrunner`) profile carries only four skill
  categories (`autonomous-ai-agents`, `job-search`, `skills`, `software-development`),
  none network-related.
  CLI tools on PATH: `compgen -c | grep -iE 'unifi|ubnt|ubiquiti'` returns nothing.

  ### 3. unifi/ubiquiti name search

  ```
  grep -rniEw 'unifi|ubiquiti' ~/.hermes/config.yaml \
      ~/.hermes/profiles/cronrunner/config.yaml ~/.hermes/skills \
      ~/.hermes/profiles/cronrunner/skills ~/.hermes/plugins \
      ~/.hermes/profiles/cronrunner/plugins ~/.hermes/.env
  ```
  → **absent — checked, not present** (0 matching lines; `.hub/` index cache excluded).
  `tool_search "unifi ubiquiti network controller"` → no matches.

  ## Answer

  **No.** Nothing in this harness speaks to UniFi/Ubiquiti — no MCP server, no plugin,
  no skill, no CLI. The confirmed integrations are KiCad, FreeCAD and Upwork, plus a
  Home Assistant plugin.

  ## Two honest qualifiers

  - The Home Assistant plugin IS installed and configured (`HASS_TOKEN` and `HASS_URL`
    are both present in `~/.hermes/.env`; names only, values never read into this
    reply). It exposes `ha_list_entities`, `ha_get_state`, `ha_list_services`,
    `ha_call_service` against HA's REST API. Home Assistant has a UniFi Network
    integration, so *if* your controller is already linked into your HA instance,
    UniFi device/port/client state would be readable as HA entities through those
    four tools — read-only description; I did not connect to HA or to any controller,
    and I have no evidence such an integration is configured. That is a "maybe, if you
    wired it up", not a capability I can claim.
  - `sirkirby/unifi-network-mcp` is installable and would be the direct route
    (read/manage switches, clients, VLANs, firewall rules). Wiring it is one
    `hermes mcp add unifi ...` plus credentials, which this task forbids.

  ## Cost

  The 1.1.0 cost format is not active in this harness yet (`mailbox/README.md` on
  origin/main is still `protocol_version: 1.0.0`; 1.1.0 lives on branch
  `hermes/0155-cost-reporting`). Per-call usage is not in the worker logs
  (`grep -ci usage logs/agent.log` → 0), so the token fields are `unknown` rather
  than estimated. Approximate spend for this task: well under one US cent —
  ~15 small tool calls plus one short composition on deepseek-flash, off-peak.

  ## Deviations

  None. Read-only throughout: no installs, no credential or config changes, no
  connections to any controller, no network probes. No branch and no CI run exist
  or are needed for this task shape (the spec says so and pre-authorizes
  `branch: none`).

---


# 0157 — Do you have any UniFi / Ubiquiti capability?

## Context

Stephen is configuring a client's network switches on Ubiquiti UniFi
hardware (Coyote Moon job). He asked whether Hermes has a UniFi MCP,
skill, or equivalent. Juno checked from the outside: the
`sirkirby/unifi-network-mcp` project is real and installable, and no
UniFi/Ubiquiti capability appears anywhere on Hermes's proven record
(KiCad and Upwork are the confirmed integrations). Only Hermes can say
what his own harness actually has installed. That is the one question
this task answers.

## Task

Answer ONE question with evidence: does your harness have anything that
can talk to Ubiquiti UniFi hardware — an installed plugin, an MCP
server, a skill, or a CLI tool?

Run these three read-only checks and report each one:

1. Your plugin list: read `~/.hermes/config.yaml` and list the plugins
   configured there. Quote the plugin names (lines), not values.
2. Your tool/MCP inventory: list the MCP servers or equivalent tools
   your harness exposes. Quote the names.
3. A name search: check for anything named or described with
   `unifi` or `ubiquiti` anywhere in the above.

## Success criteria

1. The staged reply names all three checks, and for each one either
   quotes the exact config line / tool name found, or states explicitly
   "absent — checked, not present".
2. If a UniFi-capable tool exists, the reply names it and describes in
   one or two lines what it can do (read-only description; do NOT
   connect to any controller or touch any network).
3. If nothing exists, the reply says so plainly in one line. "No" is a
   complete, acceptable answer.

## Constraints

- READ-ONLY. No installs, no pip/npm, no credential changes, no
  connections to any UniFi controller or console, no network probes.
- Secrets stay secret: you may read `config.yaml`, but do NOT paste
  any value from `~/.hermes/.env` or any key/token into the reply.
  Quote names and structure only.
- Keep it small: this should be a few minutes of work. If the cost
  block (protocol 1.1.0, on branch `hermes/0155-cost-reporting`) is
  available in your harness, include it; otherwise a one-line
  approximate cost is fine.

## Proof

This is a read-only, host-local question, so there is no branch and no
CI run. The proof block on `staged` may read:

```yaml
proof:
  - branch: none
    reason: read-only host-local capability question; evidence is quoted config/tool names in notes
```

The quoted lines in `notes` ARE the evidence. A RED verdict from the
verifier on `branch: none` is expected for this task shape and is not
a failure.

## Reply format

Keep it short. In `notes`:

```
## Findings
### 1. plugins (~/.hermes/config.yaml)
<quoted names or "absent">
### 2. tools / MCP servers
<quoted names or "absent">
### 3. unifi/ubiquiti name search
<quoted hits or "absent">
## Answer
<yes/no, one line, plus tool name if yes>
```
