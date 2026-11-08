# Newton headless token measurement — one clean daily sweep (2026-11-07)

Task 0171. One Upwork lead sweep run headless through the Antigravity CLI (`agy -p
--output-format json`) with the paging fix applied, quota meter read before and after from the
same source, against Hermes's own 05:00 MST sweep for the same day.

Run window: **2026-11-08T01:19:02Z → 01:40:45Z** (2026-11-07 18:19:02 → 18:40:45 MST), wall
**1,303 s = 21 min 43 s** (`duration_seconds` in the JSON: 1299.787). Model: **Gemini 3.8 Flash
(High)**, free "Antigravity Starter Quota" tier, `nordlus@gmail.com`.

## The answer

**One clean, fully paged daily sweep costs Newton ≈ 22.9 points of his 100-point weekly Gemini
pool — about 23 % of a week's quota per sweep — and 290,709 billed tokens (244,209 input /
46,500 output / 29,442 of it thinking, plus 2,869,956 cache-read).** At that rate the free weekly
pool covers **≈ 4.4 sweeps**, so a *daily* sweep (7/week) does **not** fit: it would exhaust the
Gemini pool in roughly four and a half days. Hermes's same sweep cost 2.4 cents and 2 minutes.

## Comparison — one sweep each, same day

| | Newton (Antigravity CLI, free tier) | Hermes (deepseek-flash, off-peak) |
|---|---|---|
| Sweep run | 2026-11-08T01:19:02Z → 01:40:45Z (18:19–18:40 MST) | 2026-11-07T12:00:02Z → 12:02:03Z (05:00–05:02 MST) |
| Wall clock | **21 min 43 s** | **2 min 01 s** |
| Model | `Gemini 3.8 Flash (High)` | `deepseek-flash` |
| Input tokens | 244,209 | 69,463 |
| Output tokens | 46,500 (incl. 29,442 thinking) | 19,173 (incl. 10,299 reasoning) |
| Cache-read tokens | 2,869,956 | 806,144 |
| Total billed (`total_tokens`) | **290,709** | 88,636 (+ cache-read) |
| API calls / tool calls | 1 model turn; 16 MCP calls + 30 shell calls from the record | 16 API calls / 28 tool calls |
| Quota | **−22.91 pts weekly Gemini (54.92 % → 32.01 %)**; Claude/GPT pool unchanged | n/a |
| Cost | free tier (quota-metered) | **$0.02434168** (`session_model_usage`, `estimated`) |
| In-window postings listed | 17 distinct IDs (3 leads, 14 skipped) | 26 distinct IDs (4 leads, 19 skipped) |
| MCP errors | 5 failed calls after 1 SSE disconnect | none |

## Quota meter (verbatim)

Source: `agy -p "/usage" --output-format json` (free; the print-mode slash command is answered
locally from the CLI's own quota state and consumes no quota). The `/usage` JSON is quoted in
full — it is the only place the exact fraction appears. This free tier exposes **weekly buckets
only**; there is no 5-hour bucket in the response, so no 5-hour figure can be quoted.

**Before** — 2026-11-08T01:16:56Z (18:16:56 MST):

```
Gemini Models	Weekly Limit Remaining	55%	2026-11-14T13:49:47Z
Claude and GPT models	Weekly Limit Remaining	98%	2026-11-14T23:46:00Z
```

```json
[{"name":"Gemini Models","buckets":[{"id":"gemini-weekly","window":"weekly",
  "remaining_fraction":0.5492135882377625,"reset_time":"2026-11-14T13:49:47Z"}]},
 {"name":"Claude and GPT models","buckets":[{"id":"3p-weekly","window":"weekly",
  "remaining_fraction":0.9848099946975708,"reset_time":"2026-11-14T23:46:00Z"}]}]
```

**After** — 2026-11-08T01:43:10Z (18:43:10 MST):

```
Gemini Models	Weekly Limit Remaining	32%	2026-11-14T13:49:47Z
Claude and GPT models	Weekly Limit Remaining	98%	2026-11-14T23:46:00Z
```

```json
[{"name":"Gemini Models","buckets":[{"id":"gemini-weekly","window":"weekly",
  "remaining_fraction":0.3201240003108978,"reset_time":"2026-11-14T13:49:47Z"}]},
 {"name":"Claude and GPT models","buckets":[{"id":"3p-weekly","window":"weekly",
  "remaining_fraction":0.9848099946975708,"reset_time":"2026-11-14T23:46:00Z"}]}]
```

Delta: Gemini **0.5492135882377625 − 0.3201240003108978 = 0.2290895879268647** (22.909 pts).
Claude/GPT delta: **0.0000000000000000** — the fraction is byte-identical across the two reads.
No quota or rate-limit message appeared at any point in the run.

## Was the window clean?

The criterion is "no interactive Newton session live". Precisely:

- An interactive `agy` **process** does exist (pid 2831685, up 11.9 h at the start of the run,
  Herdr `agy-main` pane `w1:p5`). It was **idle**, not generating: Herdr's snapshot reported
  `agent_status: "idle"`, `interactive_ready: true` for that pane, and the pane's conversation
  db `a6c1541a-…` had not been written since **16:50:47 MST — 88 minutes before the sweep
  started** and unchanged at the end.
- The **Claude/GPT pool fraction is byte-identical** before and after
  (`0.9848099946975708` twice), which is what "nothing else generated on this account during the
  window" looks like numerically: a concurrent generation consumes some quota, which would move
  that fraction.
- Therefore the 22.909-point Gemini delta is attributable to the sweep.

Residual uncertainty, stated rather than hidden: an idle interactive process sharing the account
is not the same as *no* process, and quota is account-level. If Stephen had typed into `agy-main`
during the 24 minutes, it would have landed in this delta. The db mtime and the unchanged
Claude/GPT fraction are the evidence that he did not.

## Paging fix — applied, and where it did not terminate

Every query was paged with the response's own `cursor` until `hasMore` was false, until an empty
page, or until the 6-page cap I set. From the conversation record (`call_mcp_tool` arguments in
`conversations/4c9956b2-….db`), the find_jobs calls were:

| Query | Pages | Cursor values used | Rows | In-window | Terminated by |
|---|---|---|---|---|---|
| `smart_search` `{mode:"most_recent", days_posted:1, limit:10}` | 2 | `t:1794036969226` | 15 | 15 | `hasMore:false` |
| `search` `"ESP32 embedded firmware"` | 6 | `10`, `20`, `30`, `40`, `50` | 60 | 9 | **6-page cap — `hasMore` still true** |
| `search` `"LoRa LoRaWAN MQTT Home Assistant wireless sensor network"` | 1 | — | 7 | 0 | `hasMore:false` |
| `search` `"off-grid solar LiFePO4 battery energy monitoring system"` | 1 | — | 1 | 0 | `hasMore:false` |

Two facts the reply must not paper over:

1. **The paging fix works** — three of four queries ran to natural termination, and the ESP32
   feed was walked through six pages where 0170's run took exactly one.
2. **The ESP32 feed did not terminate.** `action=search` pages by a numeric offset (`cursor:
   "10"` … `"50"`), not the `t:<ms>` token `smart_search` uses, and 60 rows in it was still
   `hasMore: true`. That is a real gap: the 24 h window edge for that feed was never reached, so
   any in-window row past row 60 was not seen. The 3-of-4 figure is the honest score, not 4-of-4.

The model's own trailing counts are internally inconsistent and should not be trusted
(`smart_search` reported `total_matching=5` alongside `rows=15`; the per-query in-window counts
sum to 24 against a 17-ID listing). The ID listing and the cursor arguments above are the
reliable record.

## MCP disconnect

At **18:31:20 MST** the CLI logged:

```
W1107 18:31:20.015677  mcp_manager.go:2216] MCP server connection closed unexpectedly for
upwork: standalone SSE stream: exceeded 5 retries without progress (session ID: rZeMJM3b/…)
```

After that every Upwork call failed — the record shows `find_jobs action=get` for
`2119128953001268732` attempted **three times** (idx 68, 70, 72, all status 7) and
`list_accounts` attempted **twice** more (idx 80, 86, both status 7). So the run hit
**1 disconnect and 5 failed retries**, not the "1 disconnects, 2 retries" the model reported.
The sweep's search phase had finished before the drop, so no query is reported as 0 results on
the strength of a dead feed: the three leads stand on pages that completed. The `get` enrichment
(connects_cost, exact budget) did **not** complete, so every lead below carries its search-phase
data only — `connects_cost` is unknown ("?"), and one lead shows `total_spent $0.00 / jobs_posted
0` straight from search.

## What the sweep found (read-only; nothing written to Upwork)

3 in-window leads, 14 skipped, 76 distinct rows exposed across all pages:

| ID | Title | Budget | Posted | Client | Connects |
|---|---|---|---|---|---|
| `2119128953001268732` | Electronics Engineer for ESP32 PCB Design | hourly | 7.2 h | Estonia, verified | ? |
| `2119069742500247508` | STM based PCB schematic review and PCB Design | $400 fixed | 11.2 h | UK, not verified | 13 |
| `2118975222252005778` | Systems engineer, not app dev: Rust edge runtime with BLE on Android — PoC phase 1 | $1,500 fixed | 17.4 h | Germany, verified, $6,210 spent, 5.0★ | 21 |

Coverage against Hermes's same-day file (`2026-11-07.md`, 26 distinct IDs): **7 IDs shared, 10
Newton-only, 19 Hermes-only.** The 10 Newton-only IDs were all posted after Hermes's 12:00Z
sweep, i.e. late arrivals a second run later in the day is *supposed* to catch. The 19
Hermes-only IDs are not directly comparable — Newton's own 24 h window starts at 01:19Z on
2026-11-07, so postings from the earlier part of Hermes's window (2026-11-06T12:20Z onward) are
outside it by construction. The comparison is indicative, not a like-for-like recall score.

## Hermes side (`session_model_usage`, same-day sweep)

The scheduled 05:00 MST sweep is the job `892982733bba`; today's run is session
`local-81d0497ee21c9c7539364cbdd64f0c8b33db24ee70177818e24e696968390ddd`, started
2026-11-07T12:00:02Z, last activity 12:02:03Z. I did **not** run an extra sweep — its rows were
read from this profile's `state.db` (the same source 0155 and 0170 used):

```
model=deepseek-flash  provider=deepseek  task=''  api_call_count=16
input=69463  output=19173  cache_read=806144  cache_write=0  reasoning=10299
estimated_cost_usd=0.02434168  cost_status=estimated  cost_source=official_docs_snapshot
sessions row: messages=45  tool_calls=28
```

Cross-check of Newton's token count against the CLI's own per-generation store: **not
performed, because the store does not exist in this CLI version** — the conversation db
(`conversations/4c9956b2-….db`) has `steps`, `gen_metadata`, `executor_metadata` and
`trajectory_*` tables and **no `generations` table**, and `tokstat` is not installed on this host.
The headless JSON block above is therefore the only token source, quoted verbatim and
uncontradicted.

## Verdict

**One clean sweep = 22.9 % of the weekly Gemini pool and 290,709 tokens, in 21 min 43 s.** The
free tier holds for *about four sweeps a week* — not for a daily one. A daily sweep needs ~160
points/week against a 100-point pool. The measurement also moves the number substantially from
0170's contaminated "≤31 points": that run shared its window with a live interactive session and
took one page per query; this one was clean, but paged six deep into one feed, so the 22.9 points
is the cost of a sweep that actually pages. The trade against Hermes is stark in both directions
— 10× the wall clock and a quarter of the weekly quota for the same lead count class, versus 2.4
cents — and the free tier is only viable as an occasional second opinion, not a daily driver.
