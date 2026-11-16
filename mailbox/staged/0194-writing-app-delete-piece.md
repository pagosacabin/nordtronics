---
task_id: "0194"
protocol_version: 1.2.0
status: staged
iteration: 1
expect-reply-within: 24h
proof:
  branch: hermes/0194-delete-piece
  sha: f1b3394556dfccbba6e08704b828ffbfa4760085
  run: none
  run_reason: >-
    cabin-play carries no CI at all: `ls -la .github/workflows` -> "No such file or directory"
    on main, and `gh run list -R pagosacabin/cabin-play --limit 5` returns zero runs for the
    whole repo (not just this branch), so no workflow exists to run. The acceptance evidence for
    this task is the pytest suite + live smoke + browser proof below. No ntfy receipt is owed
    either: there is no workflow to publish a run URL from and the task names no topic.
  pr: https://github.com/pagosacabin/cabin-play/pull/4
  merged: "2026-11-16T18:49:15Z"
  main_sha: 2b40e343ceb6802c46ab6353e13b032e93a6f9da
  copilot_review: https://github.com/pagosacabin/cabin-play/pull/4#issuecomment-6925660446
  copilot_followup: https://github.com/pagosacabin/cabin-play/pull/4#issuecomment-6925778693
  copilot_verdict: "SHIP at f1b3394 (third pass, after a BLOCK on the second fix commit)"
  files:
    - writing-peer-review/app/main.py
    - writing-peer-review/static/app.js
    - writing-peer-review/static/styles.css
    - writing-peer-review/static/sw.js
    - writing-peer-review/tests/test_smoke.py
    - writing-peer-review/tests/live_smoke.py
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
  newton: "one Gemini 3.8 Flash High headless session in Herdr t1j, 1246 s, one turn; per-call token usage not reported by agy -p, so unknown rather than estimated. Gemini pool 34% -> 14% remaining across the run; Claude/GPT pool untouched at 100%."
  notes: "The cron harness exposes no per-call DeepSeek usage to this session, so every token and cost field is unknown rather than estimated."
notes: |
  STAGED 2026-11-16T18:49Z (11:49 MST), off-peak (PEAK: OFF-PEAK 18:15 UTC), PROTOCOL MATCH
  1.2.0. Deliverable merged to cabin-play main as 2b40e34.

  WHAT SHIPPED. Delete-a-piece, end to end. `DELETE /api/pieces/{piece_id}` in app/main.py is a
  single `DELETE FROM pieces WHERE id = ?` that relies on the schema's existing ON DELETE CASCADE
  (reviews.piece_id -> pieces.id, messages.review_id -> reviews.id) with the `PRAGMA foreign_keys
  = ON` that get_connection() already sets; 404 with {"detail": "Piece not found"} when the row
  is absent; {"deleted": true, "piece_id": N} on success. In the History list each card now
  carries a Delete control that asks `window.confirm("Delete '<title>'? Its reviews and
  discussions go too — this can't be undone.")`, removes the card on confirm with no manual
  refresh, and changes nothing on cancel; if the deleted piece was open in the editor it is
  reset. CACHE_NAME bumped v2 -> v3 so an installed PWA actually adopts the new app shell.
  No soft delete, no trash, no undo.

  VERIFIED BY THIS RUN, not by quotes from Newton's report:
   - `pytest -q` at the branch tip f1b3394 -> 18 passed (13 pre-existing + Newton's tests 15/16
     + my test 17). Re-run on main after the merge: 18 passed at 2b40e34.
   - `tests/live_smoke.py` at f1b3394 -> 54/54 checks passed, rc 0, against a real uvicorn server
     and on-disk SQLite, including "Direct DB: cascade deleted piece, review, and discussion
     messages — pieces=0 reviews=0 messages=0". (One earlier run of the same script returned
     53/54; the single failure was above the capture window and did not reproduce in three later
     runs at the tip, so I record it rather than claim a clean first run.)
   - Browser proof (Playwright + Chromium 1280x900, stub provider so it cost nothing), 12/12
     checks: delete control present on every card; the confirmation names the piece; cancel
     leaves the row in the DOM and in SQLite; confirm removes the card with no manual refresh;
     SQLite shows the cascade fired for that piece; the other piece is untouched; after a page
     reload the deleted piece is still gone. Screenshots (not committed; the repo carries no test
     artifacts): .../cronrunner/cache/scratch/0194-ui/0194-history-with-delete.png and
     0194-history-after-delete.png. I asserted on the DOM and the database rather than eyeballing
     the PNGs.
   - Criterion 4 needed a test, not a fix: export_portfolio_endpoint already skips a missing id
     (`if not p_row: continue`), so test 16 pins that behaviour for both md and pdf, including an
     all-stale request.

  COPILOT REVIEW through the route the task named (**hc**, herdr-core 0.1.21, pane t17
  "copilot-main", Copilot Fusion / gpt-6-luna), posted on the PR by Copilot itself:
   1. First pass read the whole diff and returned VERDICT: SHIP with no findings (not posted).
   2. Second, targeted pass (I asked it to check every sqlite3.connect path, the confirm-string
      and stale-id handling, whether test 15 discriminates, and the card click handler) raised
      ONE real finding, posted as #issuecomment-6925660446: test 15 proves the child rows are
      gone but would pass equally if the endpoint hand-rolled per-table deletes, so it does not
      show the cascade is the mechanism. VERDICT: SHIP.
   3. I fixed it with test 17 (my code, not Newton's) and pushed 22b3168. Copilot then returned
      VERDICT: BLOCK on that commit for a genuine defect I had introduced: `monkeypatch.undo()`
      also reverted the no-live-API stub, so the positive case would have made a real DeepSeek
      call whenever a key was configured. Fixed in f1b3394 (save/restore only the connection
      factory, plus an explicit assertion that the stub is still in force). Third pass:
      "RESOLVED ... VERDICT: SHIP", posted as #issuecomment-6925778693. Only then was the PR
      merged.

  HOW TEST 17 DISCRIMINATES (negative control, run twice — once before and once after the
  BLOCK fix): it drives the SAME endpoint over a connection with `PRAGMA foreign_keys = OFF` and
  requires the reviews and messages to SURVIVE as orphans — an implementation that deleted child
  rows itself would have cleared them regardless of the pragma — then asserts the FK-on path
  clears all three. Substituting a hand-rolled per-table delete in app/main.py makes it fail with
  "endpoint deleted the reviews itself -- cascade is not the mechanism"; restored, 18 passed.
  app/main.py was restored from a copy and `git diff` on it is empty.

  DEVIATIONS, all declared:
   - 0194 arrived with **no front-matter**; the protocol fields were added at pickup (rule 16),
     commit c6decae on main.
   - app.js uses `window.confirm`, so the confirmation carries no escaping risk (it is plain text,
     not HTML); a stale id falls back to the title "Untitled Piece".
   - The build brief asked Newton for `--mode auto`, which this account does not offer
     ("continuing in the default mode"); it ran headless with `--dangerously-skip-permissions`
     in Herdr terminal t1j and finished in 1246 s.
   - Newton's first cut was sound but its tests did not cover the cascade-vs-hand-rolled
     distinction (Copilot's finding). Test 17 is mine. The three follow-up commits after
     Newton's e95a32b are mine (test 17, the stub fix).
   - PR create and merge used the **keyring OAuth login** (`gh auth status` -> pagosacabin,
     keyring, scopes gist/read:org/repo); `gh-env.sh` was NOT sourced and no GITHUB_TOKEN was
     exported, per the task. Write capability was probed BEFORE the build: a PR POST with a bogus
     head returned 422 Validation Failed (permission present), not 403 — the 0186 failure mode
     did not recur. Pushes went over SSH.
   - Queue discipline: active/ 0142 and 0187 are `status: superseded` and were not resumed;
     active/ 0159 is decision-blocked, its file was not touched, not re-investigated, iteration
     unchanged; inbox/ 0176 is `superseded` ("Do not pick up"). 0194 was the lowest actionable
     task. 0198 remains queued.
   - No secret appears in this file, the PR, the commits or the screenshots; the DeepSeek key
     stayed in .env and no test artifact was committed.
   - Gemini pool is now at 14% remaining, i.e. below protocol 1.2.0's 20% switch point: the next
     Newton job should run on the Claude/GPT pool (100% remaining).

  COST. Newton: one Gemini Flash High headless session, 1246 s, one turn, usage not reported by
  `agy -p` so it is stated as unknown rather than estimated. Hermes: DeepSeek Flash, per-call
  usage not exposed to this cron session, so all cost fields are `unknown` with that reason —
  not an estimate.
---

# 0194 — delete a piece from history

**Status: staged — shipped and merged.** Branch `hermes/0194-delete-piece` (tip
`f1b3394`), PR https://github.com/pagosacabin/cabin-play/pull/4, merged to `main` as
**2b40e34** at 2026-11-16T18:49:15Z. Copilot's review is on the PR (findings below).

## What shipped

`DELETE /api/pieces/{piece_id}` — one `DELETE FROM pieces WHERE id = ?` that lets the
schema's `ON DELETE CASCADE` take the piece's reviews and their discussion messages (the
connection pool already sets `PRAGMA foreign_keys = ON`). A missing id is a 404
(`{"detail": "Piece not found"}`), not a 500; success returns
`{"deleted": true, "piece_id": N}`.

In the History list every piece card now carries a **Delete** control. It stops the card
click from opening the piece, asks

> Delete '<title>'? Its reviews and discussions go too — this can't be undone.

and on confirm removes the card immediately — no manual refresh — resetting the editor if
that piece was open. Cancelling changes nothing. `CACHE_NAME` went `v2` → `v3` so an
installed PWA actually loads the new `app.js` instead of serving the cached one. No soft
delete, no trash, no undo, as specified.

## Success criteria — evidence

| # | Criterion | Evidence |
|---|---|---|
| 1 | Piece + review + message all gone; a second piece untouched | test 15 (SQLite row reads before/after) + live smoke "Direct DB: cascade deleted piece, review, and discussion messages — pieces=0 reviews=0 messages=0"; test 17 pins that the cascade, not hand-rolled deletes, is the mechanism |
| 2 | DELETE of a non-existent id is 404 | test 15 asserts 404 and the exact body; live smoke "DELETE non-existent piece returns 404 — st=404" |
| 3 | UI: control present, confirmation names the piece, cancel keeps it, confirm removes it with no refresh | Playwright/Chromium 12/12 checks, DOM **and** SQLite asserted; screenshots `0194-history-with-delete.png`, `0194-history-after-delete.png` |
| 4 | Portfolio export excludes a deleted piece and tolerates a stale id | already the behaviour (`if not p_row: continue`), so it is pinned rather than fixed: test 16 covers md + pdf and an all-stale request |
| 5 | Suite passes plus new tests; live smoke extended | `18 passed`; live smoke `54/54`, rc 0 — both re-run at the tip and again on `main` after merge |
| 6 | PR on cabin-play, Copilot review posted, findings addressed, merged after review | PR #4; review #issuecomment-6925660446; follow-up #issuecomment-6925778693; main `2b40e34` |

## Copilot review (Herdr pane t17, `hc`, herdr-core 0.1.21, Copilot Fusion / gpt-6-luna)

1. **First pass** — whole-diff read, `VERDICT: SHIP`, no findings.
2. **Second, targeted pass** — one real finding, posted as
   [issuecomment-6925660446](https://github.com/pagosacabin/cabin-play/pull/4#issuecomment-6925660446):
   *minor — test 15 would pass equally if the endpoint hand-rolled per-table deletes, so it
   does not show the cascade is the mechanism.* **Fixed** with test 17: it runs the same
   endpoint over a `foreign_keys = OFF` connection and requires the children to survive as
   orphans, then asserts the FK-on path clears them. Negative control: substituting a
   hand-rolled per-table delete in `app/main.py` makes it fail
   ("endpoint deleted the reviews itself -- cascade is not the mechanism"); restored, green.
3. **Third pass on that fix** — `VERDICT: BLOCK` for a defect I had introduced:
   `monkeypatch.undo()` also reverted the no-live-API stub, so the positive case would have
   made a real DeepSeek call whenever a key was configured. **Fixed** in `f1b3394` (patch and
   restore only the connection factory, plus an explicit assertion that the stub is still in
   force). Copilot then returned `RESOLVED ... VERDICT: SHIP`, posted as
   [issuecomment-6925778693](https://github.com/pagosacabin/cabin-play/pull/4#issuecomment-6925778693).
   Merge happened only after that verdict.

## How it was built and checked

Newton (Antigravity CLI, Gemini 3.8 Flash High, headless, `--dangerously-skip-permissions`)
built the feature from a written brief in Herdr terminal `t1j`: 1246 s, one turn. Hermes
wrote the brief, reviewed the diff, ran every check itself, and wrote test 17 and the stub
fix. Newton's `agy -p` does not print token usage, so its per-call usage is recorded as
unknown rather than estimated. The Gemini pool went 34% → 14% across the run; the Claude/GPT
pool is untouched at 100% and, being below protocol 1.2.0's 20% switch point, is where the
next Newton job should run.

## Cost

DeepSeek Flash, off-peak. Per-call usage is not exposed to this cron session, so every token
and cost field is `unknown` with that reason rather than an estimate.
