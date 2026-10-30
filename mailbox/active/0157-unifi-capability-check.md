---
task_id: "0157"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
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
