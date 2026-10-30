---
task_id: "0159"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0159 — Wire the UniFi MCP to Coyote Moon (read-only live proof)

## Context

Task 0158 installed sirkirby/unifi-network-mcp 0.42.0 and smoke-tested it:
204 tools enumerated, 98 read-only, stdio clean, nothing registered, no
credentials. Stephen has now staged the Coyote Moon UniFi controller API key
at `/home/astroboy/.hermes/coyote-moon.api` on your machine. This task is
rung two: register the server, wire the key, and prove a live read-only
connection. Nothing gets changed on the controller.

## Task

1. Register the installed `unifi-network-mcp` server in your MCP client
   config (whichever profile is your default working profile), wired to the
   Coyote Moon controller using the API key at
   `/home/astroboy/.hermes/coyote-moon.api`. Read the key from that exact
   path. Follow the project's own documented setup for API-key auth
   (read-only key is expected and sufficient).
2. Prove the live connection with read-only calls only: list the sites on
   the controller, then list the devices (name, model, status) for the
   Coyote Moon site. Quote the site count, device count, and at least three
   device names/models from the actual output.
3. If the key lacks permission for any call, report the exact denial — do
   not retry with different credentials, do not escalate, do not invent
   values.

## Success criteria

1. The MCP server is registered and enabled in your config; the registration
   entry is quoted (with the key itself redacted).
2. Live read-only proof: site list + device list from the Coyote Moon
   controller, quoted from actual tool output (counts + sample names).
3. Zero mutations: no device adopted, blocked, restarted, renamed, or
   reconfigured; no firewall/VLAN/WLAN changes; no config written to the
   controller. The 98 read-only tools are the only ones you may call.
4. The API key value is never printed, logged, or staged anywhere.

## Constraints

- Read-only with respect to the Coyote Moon network. Any tool without
  `readOnlyHint: true` is out of scope for this task.
- Secrets discipline: the key lives at `/home/astroboy/.hermes/coyote-moon.api`
  and is read from there directly. It never appears in your staged reply,
  in logs you quote, or in any file you create.
- Default model tier for this task.

## Proof

- branch: none (host-local config + live read-only queries, no repo changes)
- run: none
- Quoted terminal output: the registration entry (key redacted), the site
  list, and the device list. Verbatim output is the proof; there is no
  remote artifact by design.

## Reply format

Staged reply with `proof:` (branch/run none + reason), `notes:` containing
the findings (registration method, site/device counts and sample names,
any permission denials with exact text, anything that failed and the exact
error), and a `cost:` block if your harness supports it (unknowns with
reason are acceptable under 1.0.0).
