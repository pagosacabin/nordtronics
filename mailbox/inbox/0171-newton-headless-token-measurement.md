# 0171 — Newton headless token measurement + paging fix

## Assignment
- **Newton runs the sweep** (steps 3–6): the headless `agy -p` sweep with paging, his own token count from the JSON output, the quota reads before and after, and the MCP reconnect/retry. Hermes hands him these steps via Herdr.
- **Hermes runs the frame**: the clean-window check (step 1), his own sweep plus `session_model_usage` (step 7), the comparison table and verdict (step 8), writing and committing the artifact, and staging the reply.
- **Hermes is the mailbox reader**: he picks this task up and routes the Newton steps to Newton. Newton does not poll the mailbox — if a step says Newton, Hermes drives it through him.

## Context
Task 0170 proved the Antigravity free tier covers a daily sweep, but its quota number is contaminated: Gemini weekly remaining went 94% → 63%, and Hermes declared himself that the 31-point drop is an upper bound, not the sweep's alone — Stephen's live interactive Newton session shared the same window and account, and the CLI exposes no per-conversation token counter in interactive mode, so the shares cannot be separated. Stephen asked (2026-11-07, ~10:26 MST) whether Newton has an API to determine tokens remaining or used. Juno's answer: tokens *used* are measurable — headless `agy -p` with `--output-format json` returns per-task input/output/thinking/cache/total counts, and the CLI keeps per-generation tokens and cost in `~/.gemini/antigravity-cli/conversations/*.db` (a `generations` table; the tokstat tool reads it, but it is reverse-engineered and experimental). Tokens *remaining* have no official API and no token count anywhere — remaining is always a fraction, via an undocumented local endpoint (community tool `agy-cli-usage` wraps it, with a `GET /quota` HTTP endpoint) or Google's Cloud Code `retrieveUserQuotaSummary` endpoint. Stephen agreed ("Exactly") to the plan: run the sweep headless with JSON output for Newton's own token count, and read the quota fraction before and after with nothing else live.

Second gap from 0170, also to fix in this run: recall was a paging artefact, not a model limit. Newton took one page per query while Hermes paged; the fix is explicit `next_cursor` paging until `hasMore=false`.

Third gap, found in Newton's own "how did you do?" report (2026-11-07 ~16:28 MST, pane photo): the Upwork MCP SSE stream dropped mid-run as query 2d was dispatched and hit the 5-retry limit — query 2d (off-grid solar / LiFePO4, Stephen's core lane) returned 0 rows, and follow-up `find_jobs action=get` calls errored out until the connection reset. A dead feed is a silent recall loss: it looks like an empty result when the query never ran.

Newton is installed and signed in as nordlus@gmail.com on the free "Antigravity Starter Quota", Gemini 3.8 Flash (High), in the Herdr `agy-main` pane. Chain of command: Juno (boss) → Hermes → Newton. Hermes drives Newton via Herdr's agent-to-agent path, as in 0170. CLI only — NOT the desktop IDE. Free tier only.

The question to answer: what does one daily sweep cost Newton in tokens (his own count) and in weekly quota fraction, measured cleanly with nothing else live?

## Task
1. Confirm the measurement window is clean BEFORE starting: no interactive Newton session live in the `agy-main` pane during the run. If Stephen is using Newton interactively, wait for a window when he is not — do not run the measurement while his session shares the quota window, or the number is contaminated again. Record that the window was clean.
2. Record the Antigravity quota meter BEFORE the run — remaining fraction and reset time per model, weekly and 5-hour windows — via the undocumented local endpoint (`agy-cli-usage --json` or its `GET /quota` endpoint) or the Cloud Code `retrieveUserQuotaSummary` endpoint. Quote verbatim. Note the weekly reset date (2026-11-14 per 0170).
3. Run the daily sweep as the test workload, HEADLESS: `agy -p` with `--output-format json`, same Upwork sweep Hermes's cronrunner runs (task 0121 definition — same sources, same filters). Read-only: no bids, no applications, no messages, no account creation. Hermes drives Newton via Herdr as in 0170; the prompt tells the CLI not to write files or run git — Hermes writes/commits the artifact from the CLI's verbatim output, as in 0170.
4. Apply the paging fix: page every sweep query with `next_cursor` until `hasMore=false`. Record distinct in-window posting count and leads found, and whether recall now matches Hermes's paged sweep.
5. Apply the MCP resilience rule: if the Upwork MCP SSE stream drops mid-run, reconnect and retry the failed query (and any failed `get` calls) before reporting. Record any disconnect and retry in the reply. If a feed cannot be re-established, report that query as NOT RUN — never report it as 0 results.
6. Capture Newton's own token count from the headless JSON output: input, output, thinking, cache, total — per task. Cross-check optionally against the `generations` table in `~/.gemini/antigravity-cli/conversations/*.db` (tokstat); if the two disagree, quote both and say so — do not pick one silently.
7. Record the quota meter AFTER the run, same source as step 2, verbatim, plus wall-clock time and any quota/rate-limit messages verbatim.
8. Run Hermes's own sweep for the same day as usual and record its session token usage + cost from `session_model_usage` (the same source 0155 used).
9. Deliver the comparison: one table — Newton tokens used (headless JSON), Newton quota fraction before/after, Hermes tokens + DeepSeek cost, wall time each, leads each, errors on either side — plus a one-line verdict: what one clean sweep costs Newton in tokens and quota, and whether the free tier holds at that pace.

## Success criteria
- Measurement window confirmed clean (no interactive Newton session live) and stated in the reply.
- Quota fraction before AND after, quoted verbatim from the same source.
- Newton's own token count from headless JSON output (input/output/thinking/cache/total).
- Paging fix applied (`next_cursor` until `hasMore=false`); distinct posting count and leads quoted.
- Any MCP disconnect and retry recorded; no query reported as 0 rows when it actually never ran.
- Hermes's same-day sweep tokens + cost quoted from `session_model_usage`.
- Comparison table + verdict in the staged reply.

## Constraints
- CLI only — no desktop IDE, no paid plan, no payment details.
- DeepSeek flash, off-peak preferred; keep the spend ordinary (this is a measurement task, not a big build). State the cost constraint explicitly in any sub-task to Newton.
- Read-only toward Upwork / Freelancer / AuraOne — no bids, applications, or messages.
- Do not touch Hermes's own config, cron, or sweep schedule. Do not replace the 5 AM sweep.
- Token hygiene, same as 0170: if the CLI needs Upwork MCP access, use the 0170 pattern (temporary `agy mcp add` with a refreshed bearer, remove the MCP entry after the run, no bearer left in Newton's config, no secrets in task files or git). Do not store tokens in task files or git.
- If any step is blocked (headless JSON has no token counts, quota endpoint unreachable, window not clean), report exactly where it stopped — do not invent readings. A failed measurement reported honestly beats a guessed number.

## Proof
- Quota meter before/after (verbatim output from the endpoint/tool used + which tool).
- Headless JSON output (file path or pasted token block) + wall time.
- Sweep output (file path or pasted summary) + distinct posting count + leads.
- Hermes `session_model_usage` rows for the same-day sweep.
- Statement that the measurement window was clean.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

expect-reply-within: 6h
