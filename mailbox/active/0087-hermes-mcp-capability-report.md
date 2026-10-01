---
task_id: "0087"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
proof: []
notes: ""
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
