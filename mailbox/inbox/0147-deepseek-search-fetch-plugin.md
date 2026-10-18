---
task_id: "0147"
status: inbox
iteration: 0
proof: []
notes: ""
---

# 0147 — Install deepseek-search-fetch plugin (DeepSeek web search for Hermes)

expect-reply-within: 6h

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
