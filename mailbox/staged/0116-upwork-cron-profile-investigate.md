---
task_id: "0116"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof: []
notes: |
  VERDICT FIRST — possible without fresh consent: YES, but only by SHARING the
  DEFAULT profile's cached OAuth state, which the product docs state is not a
  supported configuration ("`auth: oauth` servers are never shared across
  profiles"); the design-correct route is a per-profile login, and that one needs
  a browser/consent. Config route detail: the device-code flow is NOT available
  for this server (its AS metadata advertises no device authorization endpoint),
  so browser-free consent routes do not apply here.

  READ-ONLY: ZERO changes to either profile. No branch, no CI run, no artifact —
  this is an investigation whose deliverable IS this reply, so `proof: []` is real,
  not an omission, exactly as the task's Reply format states ("No proof pointers to
  branches — there are none"). Read-only commands run: `hermes -p cronrunner mcp
  list`; `hermes mcp add|login|profile|config --help`; `hermes config get …`;
  `stat`/`ls`/`grep`/two `_get_token_dir()` probes. NOT run: `mcp add`, `mcp
  login`, `mcp test`, `mcp remove`, any config write, any copy/symlink of token
  files. No token, secret or credential VALUE was printed anywhere (key NAMES and
  file metadata only).

  NO WRITES, EVIDENCED BY MTIME (not by assertion): both config files predate this
  run — default config.yaml mtime 2026-10-06 07:50:33 -0600 (size 9413, mode 600),
  cronrunner config.yaml mtime 2026-09-30 10:22:46 -0600 (size 8214, mode 600).
  The cronrunner profile has NO mcp-tokens/ directory at all. What my `mcp list`
  did touch is session/cache state inside the profile (state.db, cache/,
  .mcp-discovery.lock, mcp-stderr.log) — inherent to listing, not a config or
  credential change; declared so it is not mistaken for an omission.

  1. THE THREE LAYOUT FACTS (criterion 1)
     DEFAULT profile (/home/astroboy/.hermes):
       config.yaml        mode 600, 9413 B, mtime 07:50 today
       mcp_servers keys   kicad, freecad, upwork
       upwork entry       {url: https://mcp.upwork.com/mcp, auth: oauth,
                           enabled: true}
       token dir          /home/astroboy/.hermes/mcp-tokens (mode 700), holding
                          upwork.json (267 B, 0600, 07:49), upwork.client.json
                          (346 B, 07:44), upwork.meta.json (733 B, 07:49, = the AS
                          metadata document). Key NAMES: tokens =
                          [access_token, expires_at, expires_in, hermes_issuer,
                          refresh_token, token_type]; client = [client_id,
                          grant_types, issuer, redirect_uris, response_types,
                          token_endpoint_auth_method]; meta = [authorization_endpoint,
                          registration_endpoint, revocation_endpoint, token_endpoint,
                          issuer, grant_types_supported, response_types_supported,
                          code_challenge_methods_supported,
                          token_endpoint_auth_methods_supported,
                          client_id_metadata_document_supported]. No
                          device_authorization_endpoint -> device-code login is
                          unavailable for this server.
     CRONRUNNER profile (/home/astroboy/.hermes/profiles/cronrunner/):
       config.yaml        mode 600, 8214 B, mtime 2026-09-30 10:22
       mcp_servers keys   kicad, freecad  (no upwork)
       token dir          does not exist
       its own .env, cache/, cron/, logs/, state.db are all inside this directory
     `hermes -p cronrunner mcp list` -> kicad (node .../KiCAD-MCP-Server/dist/
     index.js) and freecad enabled; upwork absent.

  2. THE MECHANISM (why the profile has no upwork — code, with line refs)
     - Token path is profile-scoped: tools/mcp_oauth.py:229-233
       `_get_token_dir()` returns `get_hermes_home() / "mcp-tokens"`, and
       HermesTokenStorage._path (:426) builds `<server>.json`, `<server>.client.json`,
       `<server>.meta.json` under it. get_hermes_home() is
       context-override -> HERMES_HOME -> platform default (hermes_constants.py:
       111-118).
     - Probe run under both homes with the repo's own venv python (read-only,
       prints paths only):
         HERMES_HOME=/home/astroboy/.hermes -> get_hermes_home() same, token dir
           /home/astroboy/.hermes/mcp-tokens exists=True
         HERMES_HOME=/home/astroboy/.hermes/profiles/cronrunner ->
           get_hermes_home() same, token dir
           /home/astroboy/.hermes/profiles/cronrunner/mcp-tokens exists=False
     - Cron ticks run scoped to the profile home: cron/scheduler_provider.py:96-110
       `_profile_cron_scope` installs a hermes-home override per tick, so a cron
       session resolves the cronrunner config AND the cronrunner token dir.
     - So the missing server is a one-line config omission; the missing TOKEN is
       the real constraint, and it is per-profile by design:
       website/docs/user-guide/multi-profile-gateways.md:585-587 — "`auth: oauth`
       servers are never shared across profiles: each profile holds its own token
       under its own `mcp-tokens/` and opens its own connection." Non-OAuth servers
       with identical route+credentials may share a connection; OAuth ones never.
       (github.com/NousResearch/hermes-agent AGENTS.md records the same intent: a
       PR adding live config inheritance from the default profile was closed
       because profiles are independent islands on purpose.)

  3. THE PROPOSED MINIMAL REVERSIBLE MOVE — DIR-LEVEL SYMLINK, NOT A FILE COPY
     The symlink must be on the DIRECTORY, and that is not a style preference —
     it is forced by the refresh fence. tools/mcp_oauth.py:77-78 puts an advisory
     lock at `<tokens>.json.refresh.lock` (a sibling of the token file) and :81-135
     documents why: refresh tokens are single-use, so two refreshers of the SAME
     token interleave as "A: POST R1 -> R2 / B: POST R1 -> 400, credential already
     burned / B: clear_tokens() -> user is logged out". The fence only excludes
     peers reading the SAME lock file. Copying (or per-file symlinking) the token
     into a second directory gives the two profiles two DIFFERENT lock files, so
     the fence cannot protect them — the first expiry after sharing logs one
     profile out. A directory-level symlink keeps one real path for both the token
     and the lock, so the fence still serializes. (A per-file symlink would also
     survive the atomic write — utils.py:167-172 `atomic_replace` resolves the
     symlink and writes the real file in place — but for the lock reason above it
     is the wrong variant.)
     Steps (numbered, each reversible; NOT executed — criterion 4 forbids it):
       0. Backup: `cp -p ~/.hermes/profiles/cronrunner/config.yaml \
          ~/.hermes/profiles/cronrunner/config.yaml.bak-0116`
       1. `ln -s /home/astroboy/.hermes/mcp-tokens \
          /home/astroboy/.hermes/profiles/cronrunner/mcp-tokens`
       2. Add the server entry — tokens FIRST, so discovery finds them and cannot
          start an interactive login. NOTE: `hermes config set` is NOT the right
          tool here; verified this run, `hermes config get mcp_servers.upwork`
          answers "Config key not set" (dynamic server names are not addressable
          schema leaves), so expect `config set` to need `--force` or refuse.
          Use the MCP-aware command instead:
            `printf 'Y\n' | hermes -p cronrunner mcp add upwork \
               --url https://mcp.upwork.com/mcp --auth oauth`
          (interactive prompt is the tool-enable list; it must be piped).
       3. Verify: `hermes -p cronrunner mcp list` shows upwork enabled, then a
          read-only call from a FRESH cronrunner session (`list_accounts`).
          MCP tools are discovered at startup, so the current session would not
          see them; editing mcp_servers triggers auto-reload of connections, but
          the tool schema needs a new session.
     ROLLBACK (full): `hermes -p cronrunner mcp remove upwork`; `rm
     ~/.hermes/profiles/cronrunner/mcp-tokens` (removes the symlink only, never
     the target); restore config.yaml.bak-0116 if anything else drifted. Confirm
     with `stat` on both config files and `ls -ld` on the symlink path.
     THE ONE THING I COULD NOT VERIFY: whether `mcp add` with the token already in
     place connects cleanly instead of prompting for an interactive login; I could
     not run it (criterion 4). If it does prompt, that IS the fresh-consent path
     and step 2 cannot complete unattended.

  4. RISKS, PLAINLY (criterion 3)
     - Sharing one OAuth session couples the profiles. Either profile calling
       clear/remove tokens (`hermes mcp logout upwork`, a rejected refresh, a
       revoked grant) unlinks BOTH: cron loses access and so does Stephen's
       interactive profile, and recovering either needs a browser again.
     - Background tasks would hold LIVE Upwork authority. This profile runs the
       mailbox worker every 15 min plus other cron jobs; after this change an
       unattended session can call upwork's tool surface with Stephen's account
       (0114 verified 32 tools, and the server's surface includes write-capable
       actions such as proposals/messages — 0114 deliberately called reads only).
       The worker's inputs are untrusted text (mailbox tasks, repo files, web
       content), so injected instructions would be executed with that authority.
       This is the strongest argument for not doing it.
     - It is an UNSUPPORTED configuration by the product's own statement, so a
       Hermes update may change token resolution (or refuse a symlinked token dir)
       and silently drop cron's access — or, worse, drop the interactive profile's.
     - File permissions buy nothing here: both profiles run as the same UID, so
       0700/0600 does not create isolation between them.
     - Do NOT use the copy/per-file-symlink variant (see 3): it turns the first
       token expiry into a double-refresh race that logs one profile out.

  5. RECOMMENDATION
     Preferred: a per-profile login when Stephen is next at a browser —
     `hermes -p cronrunner mcp login upwork` (browser PKCE + loopback; Desktop
     relay, paste-back or an SSH forward are the documented headless variants,
     features/mcp.md:383-390; `--flow device` is unavailable here, §1). That is the
     designed path, gives cron its own token, needs no shared state, and can be
     re-done without touching the interactive profile. If the server lets the
     client request narrower scopes, a read-only grant for cron is the least-
     authority option — not verified for this server, so it is an option to test,
     not a claim.
     Only if cron access is needed before a browser is available: the §3
     directory-symlink move, treated as temporary and reverted once cron has its
     own token.

  Cost: `deepseek-flash`, standard tier; `PEAK: OFF-PEAK 16:15 UTC` at pickup,
  `PROTOCOL: MATCH protocol_version=1.0.0`.
---

# 0116 — Upwork MCP in the cronrunner profile: investigate, do not change

## Context

0114 verified Upwork MCP read-only, but only in the DEFAULT profile
(/home/astroboy/.hermes): `hermes mcp list` there shows `upwork`
(https://mcp.upwork.com/mcp) enabled, 32/32 tools. The cron worker's own
`cronrunner` profile does NOT carry the server — its `mcp list` shows only
kicad and freecad, so cron ticks have no `mcp__upwork__*` tools. Stephen's
interactive OAuth (2026-10-06 ~07:53 MDT) left cached tokens at
/home/astroboy/.hermes/mcp-tokens/upwork.json (+ .client.json + .meta.json).
Stephen asked whether cron can be made to work. This task answers that
question. It makes no changes.

## Task

Investigate READ-ONLY how Hermes profiles are laid out and write up the exact,
minimal, REVERSIBLE move that would expose the already-authorized upwork MCP
server to the cronrunner profile, reusing Stephen's cached OAuth tokens. No new
interactive consent is available for this (Stephen is not at a browser) — the
write-up must either reuse the cached tokens or state plainly that fresh
consent is required and why.

## Success criteria

1. Profile layout documented: filesystem paths for the DEFAULT profile vs the
   cronrunner profile — config file(s), the `mcp_servers` section, the token
   directory — with pasted (redacted) listings.
2. The proposed move written out step by step (files to copy/symlink, config
   edits, commands), each step reversible, with a rollback note.
3. Risks stated plainly: token sharing across profiles, what background tasks
   could then do with Upwork access, anything that could break the interactive
   profile.
4. ZERO changes made to either profile. Config files read, not written.

## Constraints

- READ-ONLY investigation. Do not modify any profile config, do not copy any
  token file, do not run `hermes mcp add` / `login` / `test` under cronrunner.
  (`hermes mcp list` under cronrunner to confirm current state is fine.)
- Never print token, secret, or credential VALUES. Paths, filenames, key names,
  and structure only.
- Keep model cost on DeepSeek Flash. State the tier in the reply.
- If the answer is "cannot be done without fresh interactive consent," that IS
  an acceptable result — report it with the reason, do not work around it.

## Proof

- Pasted redacted listings showing both profiles' config/token layout.
- The step-by-step move write-up itself, in the reply notes.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, the verdict first ("possible without fresh consent: yes/no"),
then the layout evidence and the proposed move. No `proof` pointers to
branches — there are none; the write-up is the deliverable.

---

# Reply — iteration 1 (2026-10-06 16:15-16:35 UTC)

**Verdict first — possible without fresh consent: YES, but only by sharing the
default profile's cached OAuth state, which the docs state is not a supported
configuration. The design-correct route (a per-profile login) does need a
browser. ZERO changes were made to either profile.**

## 1. Layout (criterion 1)

Default profile `/home/astroboy/.hermes/`: `config.yaml` (mode 600, 9413 B,
mtime 2026-10-06 07:50:33 -0600) carries `mcp_servers: kicad, freecad, upwork`,
the upwork entry being

```yaml
upwork:
  url: https://mcp.upwork.com/mcp
  auth: oauth
  enabled: true
```

and its token directory `/home/astroboy/.hermes/mcp-tokens/` (mode 700) holds
three 0600 files: `upwork.json` (267 B, 07:49), `upwork.client.json` (346 B,
07:44), `upwork.meta.json` (733 B, 07:49). Key names only, no values:
tokens = `access_token, expires_at, expires_in, hermes_issuer, refresh_token,
token_type`; client = `client_id, grant_types, issuer, redirect_uris,
response_types, token_endpoint_auth_method`; meta = the AS metadata document
(`authorization_endpoint, token_endpoint, registration_endpoint,
revocation_endpoint, issuer, …`) — it advertises **no**
`device_authorization_endpoint`, so `hermes mcp login upwork --flow device` is
not available for this server.

Cronrunner profile `/home/astroboy/.hermes/profiles/cronrunner/`: `config.yaml`
(mode 600, 8214 B, mtime 2026-09-30 10:22:46 -0600) carries
`mcp_servers: kicad, freecad` — no upwork; **no `mcp-tokens/` directory exists**.
`hermes -p cronrunner mcp list` confirms: kicad and freecad enabled, upwork
absent. Both config mtimes predate this run, which is the evidence that nothing
was written.

## 2. Why it is missing, and why that is by design (code, line refs)

`tools/mcp_oauth.py:229-233` resolves OAuth state as `get_hermes_home()/mcp-tokens`,
and `hermes_constants.py:111-118` resolves the home as context-override →
`HERMES_HOME` → platform default. A probe with the repo's own venv python under
both homes returned `/home/astroboy/.hermes/mcp-tokens` (`exists=True`) and
`/home/astroboy/.hermes/profiles/cronrunner/mcp-tokens` (`exists=False`).
Cron ticks are profile-scoped (`cron/scheduler_provider.py:96-110`,
`_profile_cron_scope`), so they read the profile's config *and* the profile's
token dir. The product states the intent directly
(`website/docs/user-guide/multi-profile-gateways.md:585-587`): "`auth: oauth`
servers are never shared across profiles: each profile holds its own token under
its own `mcp-tokens/` and opens its own connection."

So there are two gaps, not one: the missing `mcp_servers.upwork` entry (trivial)
and the missing per-profile token (the actual constraint).

## 3. The minimal reversible move — and the one variant that is unsafe

Symlink the **directory**, never copy the files:

```bash
cp -p ~/.hermes/profiles/cronrunner/config.yaml \
      ~/.hermes/profiles/cronrunner/config.yaml.bak-0116                 # 0 backup
ln -s /home/astroboy/.hermes/mcp-tokens \
      /home/astroboy/.hermes/profiles/cronrunner/mcp-tokens              # 1 share (dir, NOT file)
printf 'Y\n' | hermes -p cronrunner mcp add upwork \
      --url https://mcp.upwork.com/mcp --auth oauth                      # 2 entry (tokens first)
hermes -p cronrunner mcp list                                            # 3 verify
```

Rollback: `hermes -p cronrunner mcp remove upwork`; `rm` the symlink (the target
is untouched); restore the `.bak-0116` copy if anything else drifted.

Why the directory and not a copy — this is the substantive finding, not a style
preference. Refresh tokens are single-use, and `tools/mcp_oauth.py:77-78`
protects the read-modify-write with an advisory lock at
`<tokens>.json.refresh.lock`, **a sibling of the token file**; the class
docstring at `:81-135` names the exact interleaving it prevents: "A: POST R1 →
200, receives R2 / B: POST R1 → 400, credential already burned / B: clear_tokens()
→ user is logged out." The fence only excludes peers locking the *same* file, so
a copied (or per-file-symlinked) token in a second directory gets a second lock
file and no protection: the first expiry after sharing logs one profile out.
A directory symlink keeps one real path for both the token and its lock, so the
fence still serializes the two profiles. (`hermes config set` is not the right
tool for step 2 either: verified this run, `hermes config get mcp_servers.upwork`
answers "Config key not set" — dynamic server names are not addressable schema
leaves.)

Not verified, and declared: whether `mcp add` with the token already in place
connects cleanly rather than prompting for an interactive login. Criterion 4
forbade running it, so step 2 is a plan, not a tested command.

## 4. Risks (criterion 3)

- **Coupling.** Sharing one OAuth session means a `logout`, a rejected refresh or
  a revoked grant unlinks *both* profiles at once, and recovery needs a browser.
- **Cron would hold live Upwork authority.** This profile runs the mailbox worker
  every 15 minutes plus other jobs; afterwards an unattended session could call
  the server's full surface — 0114 verified 32 tools, including write-capable
  proposal/message actions it deliberately did not call — while consuming
  untrusted text (mailbox tasks, repo files, web pages). Prompt-injected
  instructions would execute with Stephen's account authority. That is the
  strongest argument against sharing.
- **Unsupported configuration.** The docs say OAuth servers are never shared, so
  an update may change token resolution (or refuse a symlinked token dir) and
  silently drop access — possibly for the interactive profile too.
- **Permissions buy nothing.** Both profiles run as the same UID; 0700/0600 does
  not isolate them.

## 5. Recommendation

Preferred: `hermes -p cronrunner mcp login upwork` next time Stephen is at a
browser (browser PKCE + loopback; Desktop relay / paste-back / SSH forward are the
documented headless variants — `website/docs/user-guide/features/mcp.md:383-390`;
device-code is unavailable for this server). It is the designed path, gives cron
its own token, touches no shared state, and cannot break the interactive profile.
If the server permits narrower scopes, a read-only cron grant is the
least-authority option — unverified for this server, so an option to test, not a
claim. Only if cron access is needed before that: the §3 directory-symlink move,
treated as temporary.

Cost: `deepseek-flash`, standard tier; off-peak (`PEAK: OFF-PEAK 16:15 UTC` at
pickup; `PROTOCOL: MATCH protocol_version=1.0.0`).
