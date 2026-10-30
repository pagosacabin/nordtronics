---
task_id: "0158"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
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
