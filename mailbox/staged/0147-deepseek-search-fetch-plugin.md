---
task_id: "0147"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: none
  sha: none
  run: none
  files: []
  reason: "Host-only Hermes host/config task: no repository file changed, no branch and no CI run exists for it (the task's own Proof section specifies branch: none, run: none). Evidence is this reply's command outputs."
notes: |
  PICKED UP 2026-10-18 10:17 UTC (PEAK: OFF-PEAK, PROTOCOL MATCH 1.0.0).
  Protocol fields added at pickup: the task shipped without `protocol_version`
  and carried `expect-reply-within` as a body line; both are now front-matter.

  RESULT: DONE for the profile this worker owns (cronrunner). Installed
  deepseek-search-fetch 1.0.6 from the curated catalog (pinned c01f1702),
  enabled it, set web.search_backend=deepseek and web.extract_backend=deepseek.
  A live web_search returned real hits carrying DeepSeek-only provenance
  fields (rerank_score / authority / site_name / site_icon).

  PROFILE SCOPE — DECLARED INTERPRETATION, READ THIS. The task names no profile
  and says "Hermes's web_search and web_extract". This run is scoped to the
  cronrunner profile (HERMES_HOME=/home/astroboy/.hermes/profiles/cronrunner),
  and the standing rule for this worker is not to modify another profile's
  plugins/config. So the install targets cronrunner ONLY, and the DEFAULT
  profile was verified untouched afterwards (no search_backend/extract_backend
  key, no deepseek plugin dir). If the intent was Stephen's interactive/Desktop
  Hermes instead, the identical change there is two commands:
    hermes plugins install deepseek-search-fetch --enable
    hermes config set web.search_backend deepseek
    hermes config set web.extract_backend deepseek
  (run with the default profile's HERMES_HOME). Say so and I will apply it.

  HOW CRITERION 3 WAS VERIFIED, AND WHAT IT IS NOT. `hermes -z` (one-shot) with
  ANY tool call hangs to timeout in this environment — reproduced with the
  terminal tool ("echo TOOLOK"), so it is a pre-existing one-shot limitation,
  not this plugin. The live call was therefore made in-process through the
  SAME functions the agent loop dispatches to (tools.web_tools.web_search_tool /
  web_extract_tool) with HERMES_HOME=cronrunner, i.e. config -> provider
  registry -> plugin -> api.deepseek.com. It is a real API call with real
  results, but it is not a model-mediated agent turn; the distinction is stated
  rather than glossed.

  ROUTING PROOF BEYOND "IT RETURNED SOMETHING": the plugin degrades to
  success:false, which Hermes can silently rescue to a keyless backend, so a
  successful result alone does not prove the provider was used. A zero-cost
  stub probe (DEEPSEEK_BASE_URL -> local HTTP server) recorded
  POST /v1/tools/search and POST /v1/tools/fetch with an Authorization header
  present, and the stub's payload came back through the tool -> PASS=True.
  A rescue path would have left the stub empty.

  CONFIG WRITE: comments preserved 36 -> 36 lines; the whole diff is the two
  added keys (baseline copy kept at
  ~/.hermes/profiles/cronrunner/cache/scratch/config.yaml.pre-0147).
  `web.backend: tavily` was left in place; search_backend/extract_backend take
  precedence over it, and the task said change nothing else.

  TEST CALL COUNT: 2 successful live DeepSeek requests (1 search, 1 fetch) plus
  2 zero-cost local-stub requests. The two `hermes -z` attempts made no
  observable request (they hung before any output). Nothing was bulk-tested.
  No key value was printed anywhere; the key was read only through
  get_provider_env.
---

# 0147 — Install deepseek-search-fetch plugin (DeepSeek web search for Hermes)

## Context

DeepSeek launched Search & Fetch APIs on 2026-10-17: `POST /v1/tools/search` (web search, 1–128 char queries, up to 50 results) and `POST /v1/tools/fetch` (single URL → Markdown). Billed $0.75 per 1,000 *successful* requests; failed calls, empty results, and timeouts are free. Server-side tool calls are currently free of charge — per-request billing starts 00:00 Beijing time on 2026-11-01. Stephen's call: install now while it is free.

Community plugin `deepseek-search-fetch` (JasonSwindle/hermes-deepseek-search-fetch, MIT, in the Hermes plugin catalog, published 2026-10-16) registers `deepseek` as a Hermes web provider, routing `web_search` and `web_extract` through api.deepseek.com. Same `DEEPSEEK_API_KEY` already in `~/.hermes/.env` works — verified: the search/fetch endpoints use the same bearer-token auth as the chat API, no separate key and no permission step needed.

Relevant README facts (verified by Juno from the repo): install via `hermes plugins install deepseek-search-fetch --enable`; then `hermes config set web.search_backend deepseek` and `hermes config set web.extract_backend deepseek`; changes take effect on the next session (running sessions keep old routing); verify with `hermes plugins list` and `hermes tools`; revert with `hermes config unset web.search_backend` / `web.extract_backend` + `hermes plugins disable deepseek-search-fetch`. Failures never raise — Hermes rescues to the free keyless ring, so the failure mode of enabling this is billing, not breakage.

## Task

One deliverable: Hermes's `web_search` and `web_extract` route through DeepSeek's Search & Fetch APIs via the catalog plugin, verified end-to-end with a live test search.

## Success criteria

1. `hermes plugins list` shows `deepseek-search-fetch` as enabled.
2. `hermes config get web.search_backend` returns `deepseek`, and `hermes config get web.extract_backend` returns `deepseek`.
3. A live `web_search` call (e.g. query "Hermes Agent Nous Research") returns results with DeepSeek provenance — proving end-to-end routing, not just config. Keep test searches to a handful; billing is free until 2026-11-01 Beijing but do not bulk-test.
4. Gateway restarted and healthy after the change (`hermes gateway status` or equivalent).

## Constraints

- Use the existing `DEEPSEEK_API_KEY` in `~/.hermes/.env`. Do not create a new key. Never print the key value — in the reply, in logs, anywhere.
- Install from the plugin catalog: `hermes plugins install deepseek-search-fetch --enable`. If the catalog install fails, report the error verbatim and STOP — do not improvise a GitHub clone or manual install without asking (via staged notes, not by guessing).
- Keep test searches minimal (≤5). This is Juno's judgment call: free until Nov 1 is not a license to burn requests.
- Change nothing else in the config. If the plugin misbehaves, revert per the README (config unset + plugin disable) and report.
- Secrets stay out of the staged reply: config values yes, key values never.

## Proof

Host-only task: no branch, no CI run. Per the conduit's own rule this is honest non-proof, not fabrication — `branch: none`, `run: none`. The evidence is the staged reply carrying exact command outputs (see Reply format). Juno cannot independently verify host state; the outputs plus Hermes's attestation are the record.

## Reply format

Fill in exactly these fields when staging:

- `plugin_list_output`: exact output lines showing deepseek-search-fetch enabled
- `search_backend`: value of `web.search_backend`
- `extract_backend`: value of `web.extract_backend`
- `gateway_status`: gateway state after restart
- `test_search_query`: the query used
- `test_search_result`: 2–3 result titles/URLs returned, confirming DeepSeek provenance
- `test_search_count`: total number of test search/fetch calls made (must be ≤5)
- `notes`: anything unexpected, or the verbatim error if any step failed

---

# Reply — DONE (2026-10-18 10:2x UTC), cronrunner profile

Status: **staged**. Host-only task: `branch: none`, `sha: none`, `run: none`,
`files: []` — no repository file changed. All evidence below is from this run.

## Filled fields

**plugin_list_output** — `hermes plugins list` (row, untruncated from `--json`):

```
name:    deepseek-search-fetch
status:  enabled
version: 1.0.6
source:  catalog:community@c01f1702
path:    /home/astroboy/.hermes/profiles/cronrunner/plugins/deepseek-search-fetch
```

Installer output, verbatim:
```
✓ Installed
Plugin installed: https://github.com/JasonSwindle/hermes-deepseek-search-fetch
✓ Plugin deepseek-search-fetch enabled.
Gateway reloaded plugins; nothing of this plugin needs a session or restart.
```

**search_backend** — `deepseek` (`hermes config get web.search_backend`)

**extract_backend** — `deepseek` (`hermes config get web.extract_backend`)

**gateway_status** —
```
✓ Gateway is running via the default-profile multiplexer
  ✓ default          — PID 1001131
```
No restart was performed: the installer reported the gateway already reloaded
plugins and that nothing needs a session or restart. The cronrunner profile has
no gateway of its own (it runs through the default-profile multiplexer), so a
profile-level restart does not exist to do.

**test_search_query** — `Hermes Agent Nous Research` (limit 3)

**test_search_result** — 3 hits, all carrying DeepSeek's own provenance fields
(`rerank_score`, `authority`, `date`, `site_name`, `site_icon` under
`cdn.deepseek.com/site-icons/...`), which tavily/bing results do not have:
```
Nous Research raises $90M to bring Hermes AI agent to businesses
  https://thenextweb.com/news/nous-research-90m-1-5bn-hermes-agent-wsj
  site_name="The Next Web"  rerank_score=2.2714  authority=3
The open-source AI agent 'Hermes Agent' ... (Nous Research funding)
  https://gigazine.net/gsc_news/en/20261008-nous-research-fundraise
  site_name="GIGAZINE"     rerank_score=2.2545  authority=3
Orgo - Cloud computers for AI agents
  https://docs.orgo.ai/guides/hermes
  site_name="Orgo"         rerank_score=2.2207  authority=3
```
`web_extract` live result: `https://hermes-agent.nousresearch.com/docs` →
title `Hermes Agent Documentation | Hermes Agent`, 2,151 chars of Markdown,
`error: null`.

**test_search_count** — **2** live DeepSeek requests (1 search + 1 fetch), plus
2 zero-cost local-stub requests. No bulk testing.

**notes** — see front-matter `notes:` for the profile-scope interpretation, the
`hermes -z` limitation, the stub-based routing proof, and the config-write
comment-preservation check.

## Criterion-by-criterion

| # | Criterion | Result |
|---|---|---|
| 1 | `hermes plugins list` shows enabled | **PASS** — enabled, 1.0.6, catalog@c01f1702 |
| 2 | `web.search_backend` / `web.extract_backend` = `deepseek` | **PASS** — both read back `deepseek` |
| 3 | Live `web_search` with DeepSeek provenance | **PASS** — 3 real hits with `rerank_score`/`authority`/`site_name`; provider path additionally proven by a stub probe hitting `/v1/tools/search` + `/v1/tools/fetch` with auth |
| 4 | Gateway restarted and healthy | **PASS as-healthy, no restart needed** — gateway running (PID 1001131); installer reloaded plugins and stated no restart is required |

**Scope caveat, stated once more:** installed into the **cronrunner** profile
only. The default profile is untouched (verified). If the intention was
Stephen's interactive Hermes, say so and the same three commands will be
applied there.

