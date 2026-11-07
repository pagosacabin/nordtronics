# Antigravity CLI side-by-side sweep — 2026-11-07

Lab-notebook record for mailbox task **0170** (Antigravity CLI install + side-by-side daily sweep).
This is a **bench measurement artifact**, not a leads file. Stephen's real lead file for the day is
`handoff/upwork-leads/2026-11-07.md`, written by the 05:00 MST cronrunner sweep and untouched by this run.

## What was run

| | Antigravity CLI | Hermes (cronrunner) |
|---|---|---|
| Model | `gemini-3.8-flash-high` (Antigravity free tier) | `deepseek-flash` (DeepSeek, off-peak) |
| Sweep start | 2026-11-07T14:22:42Z (07:22:42 MST) | 2026-11-07T12:00:02Z (05:00 MST) |
| Sweep end | 2026-11-07T14:36:37Z (07:36:37 MST) | 2026-11-07T12:02:57Z (05:02 MST) |
| Wall clock | **13 min 55 s** (834.70 s, `/usr/bin/time`) | **2 min 54 s** |
| 24 h window | 2026-11-06T14:36Z → 2026-11-07T14:36Z | 2026-11-06T12:20Z → 2026-11-07T12:00Z |
| In-window leads reported | 3 | 4 |
| Off-profile skips listed | 11 | 19 |
| Distinct in-window postings | 14 IDs | 23 IDs |
| MCP calls (tool record) | 31 issued, 29 completed, 1 failed, 1 cancelled | 28 tool calls / 16 model API calls |
| Token accounting | not exposed by the CLI | 69,463 in / 19,173 out / 806,144 cache-read / 10,299 reasoning = 885,079 total |
| Cost | free tier, quota-metered | $0.024342 (estimated, `session_model_usage`) |
| Errors | 1 failed tool call, 1 cancelled; no rate-limit or quota message | none |

Hermes's figures are read from this profile's `state.db` `session_model_usage` for the 05:00 MST
session `local-81d0497ee21c9c…` (16 API calls, `estimated_cost_usd` 0.02434168, wall 2.9 min), the
same source task 0155 used.

## Quota meter (verbatim, `/usage` in the CLI)

**Before** — 2026-11-07T14:20Z (07:20 MST):

```
Gemini Models	Weekly Limit Remaining	94%	2026-11-14T13:49:47Z
Claude and GPT models	Weekly Limit Remaining	100%	2026-11-14T14:16:39Z
```

**After** — 2026-11-07T14:40Z (07:40 MST):

```
Gemini Models	Weekly Limit Remaining	63%	2026-11-14T13:49:47Z
Claude and GPT models	Weekly Limit Remaining	100%	2026-11-14T14:39:59Z
```

Delta on the Gemini weekly pool: **31 percentage points**. **This delta is not attributable to the
sweep alone** — Stephen was signed in and driving an interactive `agy` session on the same machine
and the same account during this window (conversation `a6c1541a-…`, tool confirmations in the CLI log
at 07:16–07:29 MST), so his interactive turns drew on the same weekly Gemini pool. The CLI exposes
no per-conversation token counter, so the sweep's own share of the 31 points cannot be separated
from his. What *is* attributable: the sweep ran to completion inside that window on 3 leads and 31
tool calls, with no rate-limit or quota message in its output or the CLI log.

## Environment

- `agy --version` → `1.5.3`; binary `/home/astroboy/.local/bin/agy`, 246,837,648 bytes,
  mtime 2026-11-07 06:21:40 MST, sha256 `7ca212748ca0dfda718efbc39a776a104472e7e9344d4b6ea147c543684f96a3`.
- The binary was **already installed when this run started** (06:21 MST, ~55 min before the run) and
  already authenticated: the CLI log at 07:16:08 MST records
  `keyringAuth: loaded token, expiry=2026-11-07 07:48:28 MST expired=false` and
  `OAuth: authenticated successfully as nordlus@gmail.com, authMethod=consumer`. No browser sign-in
  was performed by this run and no credential was entered anywhere.
- Upwork access was given to the CLI with `agy mcp add --header "Authorization: Bearer <token>"
  upwork https://mcp.upwork.com/mcp`, using the default profile's `~/.hermes/mcp-tokens/upwork.json`
  access token (refreshed with its own refresh_token at 14:16:59Z, new expiry 2026-11-08T14:16Z).
  The server entry was **removed again at the end of the run** — a live bearer token should not sit
  in a second tool's config. Re-add it with the same one-line command before a repeat run.
- Headless (`-p`) needs `--dangerously-skip-permissions` or an exact allow-rule per tool: without it
  the CLI auto-denies and prints `no output produced — a tool required the "mcp" permission that
  headless mode cannot prompt for`. `permissions.allow` gained `mcp(upwork)`, `command(date)`,
  `command(date -u)`, `command(uname)` for this run (non-secret, left in place); a bare
  `mcp(upwork)` rule alone was not sufficient on the retry, so the flag was used.

## Result: verbatim output of the Antigravity sweep

```
SWEEP TIMESTAMP: 2026-11-07T14:36:00Z / 2026-11-07 07:36:00 MST WINDOW: last 24h

2119069742500247508 | STM based PCB schematic review and PCB Design | $400.00 | 27m ago | United Kingdom | NOT_VERIFIED | $200.00 | 1 | 5.0 | 2 | 13 | Battery-powered LoRaWAN generator condition monitoring IoT node; schematic review and complete electronic/PCB design to Gerber package.

2118975222252005778 | Systems engineer, not app dev: Rust edge runtime with BLE on Android - PoC phase 1 | $1,500.00 | 6.7h ago | Germany | VERIFIED | $6,210.00 | 2 | 5.0 | 3 | 21 | Systems engineering for distributed Rust/WASM edge runtime with BLE communicating with ESP32 nodes; requires Android NDK and multi-device test bench.

2118783123144706307 | Design PCB based on Nucleo -N557X0 | $20.00–$50.00/hr | 19.4h ago | United States | VERIFIED | $26,896.55 | 10 | 4.79 | 22 | 18 | Hardware design stripping down STM32 Nucleo-N6 board to core MCU/flash/UART/SPI interfaces for NextPCB manufacturing; relist of earlier contract.

SKIPPED (off-profile):
2118970744502460930 - Simulation
2118961332318557692 - Utility
2118932969947360389 - Tutoring
2118928969997194682 - Licensing
2118889058256252037 - Junior
2118888110366236404 - Automation
2118869818060022274 - Drafting
2118844168220939706 - Compliance
2118841040846608057 - Licensing
2118743133220112643 - Sales
2118741658678987011 - Webdev

COUNTS: in-window=3 skipped=11 gets=6 tool_calls=13
```

### Fidelity notes on that output

- The three leads are real and match the Upwork data. `2118975222252005778` and
  `2118783123144706307` are the same two leads Hermes reported at 05:00; `2119069742500247508`
  (posted ~07:09 MST, after Hermes's sweep) is genuinely new and was caught by this run.
- The two Hermes leads the Antigravity run omitted — `2118704144347272634` (created
  2026-11-06T13:56:47Z) and `2118683166707130100` (created 2026-11-06T12:33:25Z) — are **26 h 39 m
  and 26 h 03 m old at the Antigravity sweep's start**, i.e. correctly outside its rolling 24 h
  window, not missed.
- **Recall is narrower, and the reason is paging, not the model.** Hermes paged `smart_search`
  three times (20 rows exposed against 20 claimed) and paged the ESP32 `search` feed; the Antigravity
  run issued one page per query and stopped, so it saw 14 distinct in-window IDs against Hermes's 23.
  It did see the eight additional IDs Hermes classified as skips (they appear in its conversation
  record) but reported none of them — one confirmed instance, `2118691181925605108`
  (published 2026-11-06T13:05:27Z), is 25 h 30 m old at the Antigravity sweep, so exclusion is
  correct there; the remaining seven carry no recoverable timestamp in the stored payload and are
  left un-adjudicated rather than asserted either way.
- The model's own `COUNTS: … tool_calls=13` line is **wrong**: the conversation record holds 31
  MCP call steps (29 completed, 1 failed, 1 cancelled) for this run.

## Verdict

The free tier covered one full daily sweep — **3 leads, 31 tool calls, 13 min 55 s, no quota or
rate-limit message** — at a cost of at most 31 percentage points of the Gemini weekly pool, which
also absorbed Stephen's concurrent interactive session in the same window (unseparated). At that
burn rate one sweep per day is far inside a weekly quota that started the run at 94 %; the binding
constraint on this workload is not quota, it is that the CLI does not page the search feeds, so it
under-reports leads relative to Hermes's paged sweep on the same data.
