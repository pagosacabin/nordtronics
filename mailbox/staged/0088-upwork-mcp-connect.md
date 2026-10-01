---
task_id: "0088"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof: []
notes: |
  BLOCKED AT CONSENT — staged on the task's own instruction (Task step 1: "If the
  consent step stalls because Stephen isn't there, stage a 'blocked at consent'
  status instead of waiting silently").

  THERE IS NO PROOF BLOCK AND NO CONFIG CHANGE. `proof` is empty on purpose:
  this task produced no branch, no CI run, and no config entry, because it
  stopped before the first state-changing step. Nothing was registered, nothing
  was written to ~/.hermes/config.yaml, no OAuth client was created, no tool was
  enabled, no Upwork API call was made. Verified: config.yaml comment count is
  unchanged (46 before, 46 after) and `grep upwork ~/.hermes/config.yaml` is
  empty; `hermes mcp list` still shows only kicad and freecad.

  WHAT BLOCKS IT — the exact error, not a paraphrase. `hermes mcp add upwork
  --url https://mcp.upwork.com/mcp --auth oauth` refuses to build the OAuth
  provider in this environment:
    tools.mcp_oauth.OAuthNonInteractiveError: MCP OAuth for 'upwork':
    non-interactive environment and no cached tokens found. Run
    `hermes mcp login upwork` interactively first to complete initial
    authorization.
  Mechanism (tools/mcp_oauth_manager.py:316, tools/mcp_oauth.py:1236): the
  provider is only built when `_is_interactive()` is true (a TTY on stdin) OR a
  dashboard consent flow is registered OR cached tokens exist. This run is a cron
  tick: no TTY, and no dashboard flow is registered in its process. The add then
  falls through to an unauthenticated connect and gets HTTP 401 from
  https://mcp.upwork.com/mcp, and the final "Save config anyway?" prompt defaults
  to N, so nothing is saved.

  WHY I DID NOT WORK AROUND IT. Consent is a human action — Stephen logging in to
  Upwork and clicking Allow. I cannot perform it, and I cannot verify he is
  present at the laptop. The supported non-TTY path is the desktop/dashboard
  consent flow, which still needs a human at the browser. Hand-writing an
  `mcp_servers.upwork` entry (with `auth: oauth` or without) to get past the
  failed setup is exactly the "manual registration, no workaround" the
  Constraints section forbids, so I did not do it.

  WHAT I DID VERIFY READ-ONLY (live responses, not docs):
  - OAuth metadata discovery SUCCEEDS from this host. 401 body carries
    resource_metadata=https://mcp.upwork.com/.well-known/oauth-protected-resource/mcp;
    that document returns {"resource":"https://mcp.upwork.com/mcp",
    "authorization_servers":["https://mcp.upwork.com"],"resource_name":"Upwork MCP"};
    https://mcp.upwork.com/.well-known/oauth-authorization-server returns
    authorization_endpoint=https://www.upwork.com/ab/account-security/oauth2/authorize,
    token_endpoint=https://www.upwork.com/api/v3/oauth2/token,
    registration_endpoint=https://www.upwork.com/register,
    grant_types=[authorization_code, refresh_token, client_credentials],
    PKCE S256 only, client_id_metadata_document_supported=true. So this is NOT
    the "Registration failed: 404" failure mode — the endpoint exists (GET
    returns 405 Method Not Allowed, i.e. present but POST-only).
  - No device-code flow is advertised (no device_authorization_endpoint, and
    device_code is not in grant_types_supported), so `hermes mcp login --flow
    device` is not available for this server; browser PKCE is the only path.
  - Token store location (contents not read, never printed):
    HERMES_HOME/mcp-tokens/upwork.json — currently absent.
  - Success criteria 2, 3 and 4 CANNOT be met and I did not fake them: with no
    consent there are no tools to enumerate, no proposal-status tool to probe,
    and no consent screen was ever rendered, so the granted scopes were never
    observed. Criterion 1 is unmet (no `upwork` line in `hermes mcp list`).
    Criterion 5 is met below.

  THE DECISION / FIX OWED. This needs one interactive `hermes mcp add upwork
  --url https://mcp.upwork.com/mcp --auth oauth` run from Stephen's own terminal
  (or the desktop app's MCP connect, which registers the dashboard consent flow).
  In a TTY the provider builds, the browser consent opens, and tokens are
  acquired on first connection; the tools then enumerate and steps 2-4 can run in
  a follow-up read-only task. No spec defect, no credential problem, no Upwork
  side issue — purely that a cron tick cannot host a browser consent.
---

# 0088 — Connect Hermes to Upwork MCP (read-only check)

# Context

0087 confirmed your MCP client handles remote HTTP servers with OAuth 2.1 +
dynamic client registration: `hermes mcp add upwork --url
https://mcp.upwork.com/mcp --auth oauth`. Stephen has approved connecting the
official Upwork MCP server. This task connects it and does a **read-only**
first look — nothing else.

Stephen's standing rule, unchanged: leads only, nothing submitted. No
proposal, message, offer, or Connects spend happens without his explicit
per-item approval. That is not granted here.

One record-correction for your archive, unrelated to Upwork: in 0086 the
backup passphrase file's final disposition is **shredded** (`shred -u -z
-n 3`), not "kept at 0600" as the archived text says — Juno archived before
the shred. Your staged reply for this task carries the one-line correction
(see success criteria).

# Task

1. Add the Upwork MCP server: `hermes mcp add upwork --url
   https://mcp.upwork.com/mcp --auth oauth`. Stephen is at the laptop and
   will do the browser consent when it opens; enable the discovered tools
   when prompted. If the consent step stalls because Stephen isn't there,
   stage a "blocked at consent" status instead of waiting silently.
2. Once connected, enumerate the server's tools (names + count).
3. Read-only lookups only: Stephen's Upwork profile basics (name, title,
   hourly rate, availability) and the status of his five proposals submitted
   2026-09-14 (job title, client, date, current status, any views or
   replies).
4. Report what scopes the consent screen requested (from the flow itself,
   not the docs) so Stephen can see exactly what he granted.
5. Include the 0086 one-line correction from Context above.

# Success criteria

1. `hermes mcp list` shows `upwork` enabled, backed by the staged reply.
2. Reply enumerates the Upwork tools (names + count) and states which are
   read vs write by their descriptions.
3. Reply lists each of the five 2026-09-14 proposals with its current
   status, or states plainly that the server exposes no proposal-status
   tool.
4. Reply states the scopes actually granted at consent.
5. Reply contains the one-line 0086 final-disposition correction.

# Constraints

- Read-only after connect. Do NOT submit a proposal, spend Connects, send a
  message, or accept/decline anything. Enabled tools ≠ authorized tools;
  any write action needs Stephen's explicit approval in a future task.
- Never print token contents or secrets; report the token store location
  only.
- If OAuth metadata discovery fails (e.g. "Registration failed: 404"),
  report the exact error and stop — no manual registration, no workaround.
- If the consent screen shows anything unexpected (payment, financial),
  stop before approving and report what it shows.
- Keep it cheap: DeepSeek Flash, minimal reasoning.

# Proof

The staged reply itself: `hermes mcp list` output, the tool enumeration, the
proposal statuses, the granted scopes. No branch, no code — config change
plus report. Verify what you claim against live server responses, not docs.

# Reply format

```yaml
connected: "<yes/no — hermes mcp list line for upwork>"
tools:
  - "<tool name — read|write>"
proposal_status:
  - "<submitted 2026-09-14 — job title — client — status — views/replies>"
scopes_granted: "<scopes shown at consent>"
token_store: "<location, not contents>"
correction_0086: "<one-line final disposition of the passphrase file>"
notes: "<anything Stephen should know>"
```

---

# Reply — STAGED 2026-10-01, status: BLOCKED AT CONSENT

Staged per Task step 1's own instruction ("stage a 'blocked at consent' status
instead of waiting silently"). **There is no `proof` block and no config
change** — the run stopped before the first state-changing step. Nothing was
registered, no OAuth client was created, no tool was enabled, no Upwork API
call was made, and `~/.hermes/config.yaml` is byte-identical in comment count
(46 before, 46 after) with no `upwork` key.

```yaml
connected: "no — `hermes mcp list` shows only: `kicad  node /home/astroboy/KiCAD...  all  ✓ enabled` and `freecad  /home/astroboy/.local/bin...  all  ✓ enabled`. No `upwork` line exists: the add flow failed before writing config."
tools: []
# No tools enumerable. The server answers every request with HTTP 401 until a
# bearer token exists, and no token exists because consent never ran. Listing
# names here would be fabrication; there is no pre-consent tool catalogue.
proposal_status: []
# Not queried. No server connection exists, so no proposal-status tool could be
# called. This is NOT "the server exposes no proposal-status tool" — that
# question is still open and can only be answered after connect.
scopes_granted: "unknown — no consent screen was ever rendered, so no scope set was observed. Nothing was granted (and nothing was refused). The requested scopes are only knowable from the flow itself; I did not substitute the docs."
token_store: "HERMES_HOME/mcp-tokens/upwork.json (default profile: /home/astroboy/.hermes/mcp-tokens/upwork.json) — currently ABSENT. Contents never read or printed."
correction_0086: "The 0086 backup passphrase file ~/.config/nordtronics/backup-passphrase.pw was finally SHREDDED with `shred -u -z -n 3` — it was not kept at 0600; the archived 0086 text ('KEPT in place at 0600 (16 bytes) ... Nothing was shredded') was written before the shred and is superseded."
notes: "Add failed on environment, not on Upwork. Exact error: `tools.mcp_oauth.OAuthNonInteractiveError: MCP OAuth for 'upwork': non-interactive environment and no cached tokens found. Run \`hermes mcp login upwork\` interactively first to complete initial authorization.` The OAuth provider is only built when stdin is a TTY, or a desktop/dashboard consent flow is registered, or cached tokens exist (tools/mcp_oauth_manager.py:316). This run is a cron tick, so none of the three held. Discovery itself is fine: the protected-resource and authorization-server documents both resolve from this host and https://www.upwork.com/register exists (GET -> 405, not 404), so the 'Registration failed: 404' mode described in Constraints does not apply. No device flow is advertised for this server (no device_authorization_endpoint; device_code absent from grant_types_supported), so `--flow device` is not an option and browser PKCE is the only path. Nothing was worked around: I did not hand-write an mcp_servers.upwork entry, since that is the manual registration the Constraints forbid and it would leave a permanently-401 server in every session's startup. FIX OWED — one interactive run from Stephen's terminal: `hermes mcp add upwork --url https://mcp.upwork.com/mcp --auth oauth`, complete the browser consent, enable the discovered tools. Then steps 2-4 are a cheap read-only follow-up. Success criteria 2, 3 and 4 are unmet because they depend on that consent; I did not fabricate tool names, proposal statuses or scopes to appear complete. Only criterion 5 is met."
```

## Evidence (live probes, run 2026-10-01 23:15-23:18 UTC)

1. **`hermes mcp list`** — shows `kicad` and `freecad` only. No `upwork`.

2. **`hermes mcp add upwork --url https://mcp.upwork.com/mcp --auth oauth`**
   (non-TTY stdin) —

   ```
   ... non-interactive environment and no cached tokens found. Run
       `hermes mcp login upwork` interactively first to complete initial
       authorization.
     This server may not support OAuth.
     Continue without authentication? [Y/n]:
     Connecting to 'upwork'...
     ✗ Failed to connect: MCP server 'upwork': both Streamable HTTP and SSE
       transports failed (Streamable HTTP: Server returned an error response
       (HTTP 401 from POST https://mcp.upwork.com/mcp) ...
     Save config anyway (you can test later)? [y/N]:
   ```

   The prompt defaulted to N (stdin closed), so **no config was written**.

3. **The underlying exception**, raised by the same code path via the Hermes
   venv python (not inferred from the CLI text) —

   ```
   tools.mcp_oauth.OAuthNonInteractiveError: MCP OAuth for 'upwork':
   non-interactive environment and no cached tokens found. Run
   `hermes mcp login upwork` interactively first to complete initial
   authorization.
     at tools/mcp_oauth_manager.py:317 _build_provider
   ```

   Guard at `tools/mcp_oauth_manager.py:316`:
   `if get_dashboard_oauth_flow() is None and not _is_interactive() and not
   storage.has_cached_tokens(): raise OAuthNonInteractiveError(...)`.

4. **Discovery succeeds** (so the failure is not a metadata problem):

   ```
   GET https://mcp.upwork.com/mcp
     HTTP/2 401
     www-authenticate: Bearer error="invalid_token",
       error_description="The access token is missing or invalid",
       resource_metadata="https://mcp.upwork.com/.well-known/oauth-protected-resource/mcp"

   GET .../.well-known/oauth-protected-resource/mcp
     {"resource":"https://mcp.upwork.com/mcp",
      "authorization_servers":["https://mcp.upwork.com"],
      "bearer_methods_supported":["header"],"resource_name":"Upwork MCP"}

   GET https://mcp.upwork.com/.well-known/oauth-authorization-server
     {"issuer":"https://mcp.upwork.com",
      "authorization_endpoint":"https://www.upwork.com/ab/account-security/oauth2/authorize",
      "token_endpoint":"https://www.upwork.com/api/v3/oauth2/token",
      "revocation_endpoint":"https://www.upwork.com/api/v3/oauth2/token/revoke",
      "registration_endpoint":"https://www.upwork.com/register",
      "response_types_supported":["code"],
      "grant_types_supported":["authorization_code","refresh_token","client_credentials"],
      "token_endpoint_auth_methods_supported":["none","client_secret_basic","client_secret_post","private_key_jwt"],
      "code_challenge_methods_supported":["S256"],
      "client_id_metadata_document_supported":true}

   GET https://www.upwork.com/register
     HTTP/2 405   (present, POST-only — not the 404 registration-failure mode)
   ```

5. **No config mutation** — `grep -cE '^[[:space:]]*#' ~/.hermes/config.yaml`
   = 46 both before and after the attempt; `grep -n upwork ~/.hermes/config.yaml`
   returns nothing.

6. **Token store absent** — `HERMES_HOME/mcp-tokens/` holds no `upwork.*` files.
