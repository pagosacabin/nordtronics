---
task_id: "0192"
protocol_version: 1.2.0
status: staged
iteration: 1
expect-reply-within: 24h
proof:
  - branch: hermes/0192-review-discussion
    sha: f2ec4d6ac684c2251e1b6b2f36545cc3877e9822
  - run: none
    reason: >-
      cabin-play has no .github/workflows directory at all, so no workflow exists
      to run: `ls -la .github/workflows` -> "no .github/workflows", and
      `gh run list -R pagosacabin/cabin-play --limit 3` returns zero runs for the
      whole repo (not just this branch). The acceptance evidence for this task is
      the test suite + live smoke + browser proof below, not a CI run.
  - pr: https://github.com/pagosacabin/cabin-play/pull/1
  - main_sha: a7d0dabaef19e8c7557995a4627bb579c7dba4fa
  - copilot_review: https://github.com/pagosacabin/cabin-play/pull/1#issuecomment-6904334810
  - copilot_verdict: https://github.com/pagosacabin/cabin-play/pull/1#issuecomment-6904408807
  - files:
      - writing-peer-review/discussion_prompt.md (new)
      - writing-peer-review/app/config.py
      - writing-peer-review/app/db.py
      - writing-peer-review/app/main.py
      - writing-peer-review/app/provider.py
      - writing-peer-review/app/export.py
      - writing-peer-review/static/app.js
      - writing-peer-review/static/index.html
      - writing-peer-review/static/styles.css
      - writing-peer-review/static/sw.js
      - writing-peer-review/tests/test_smoke.py
      - writing-peer-review/tests/live_smoke.py
      - writing-peer-review/README.md
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
notes: |
  STAGED 2026-11-16T01:03Z (2026-11-15 18:03 MST), off-peak (PEAK: OFF-PEAK 00:15
  UTC), PROTOCOL MATCH 1.2.0. Deliverable merged to cabin-play main as a7d0dab.

  WHAT SHIPPED. A discussion thread attached to each review, end to end: `messages`
  table keyed by review_id (non-destructive CREATE TABLE IF NOT EXISTS, same SQLite
  DB, one thread per review, survives restart); `discuss_review()` in app/provider.py
  with context order prompt -> piece -> review -> thread; `discussion_prompt.md` at
  the repo root re-read from disk on every request; GET/POST
  /api/reviews/{id}/messages; a "Discuss this review" panel under each review in the
  Write view; stub-marked (**STUB DISCUSSION**) replies whenever there is no key or
  the call fails; single-piece export in three scopes (piece only / piece + reviews /
  piece + reviews + discussions) in both .md and .pdf, with portfolio export
  untouched and `include_reviews` behaviour preserved.

  HOW IT WAS BUILT (protocol 1.2.0 model routing). Newton (Antigravity CLI, Gemini
  3.8 Flash High, `--dangerously-skip-permissions`, headless) built the feature in a
  Herdr terminal (hc run, terminal t1h) from a written brief: 1364 s, one turn,
  usage {input 238410, output 60079, cache_read 6401533}. Gemini pool was at 78%
  remaining before the run and the Claude/GPT pool at 100% — the first pool was used,
  no switch, so no pool note is owed. Hermes supervised, reviewed the diff and made
  its own changes on top (declared below).

  VERIFIED BY THIS RUN, not by quotes from Newton's report:
   - `pytest -q` -> 13 passed (7 original tests unmodified + 6 new: stub reply and
     restart persistence, separate threads for two reviews on one piece, export
     with/without discussions in md and pdf, per-request prompt reload, concurrent
     post ordering, service-worker cache version). Re-run on main after merge: 13
     passed at a7d0dab.
   - `tests/live_smoke.py` -> 35/35 checks, discussion reply from the LIVE DeepSeek
     provider (5.1 s), and "discussion thread survives a server restart — 2 messages
     kept".
   - Browser proof on the real served app (Playwright + Chromium, stub provider so
     the run cost nothing): History -> piece -> "Discuss this review" visible, message
     typed and sent through the UI, two bubbles rendered (user + peer reviewer,
     STUB-marked reply), and the same two messages read back from /api/reviews/{id}/
     messages — i.e. persisted to SQLite, not just painted in the DOM. Screenshot at
     /home/astroboy/.hermes/profiles/cronrunner/cache/scratch/0192-ui-discussion.png
     (208 KB, not committed — the repo carries no test artifacts). Re-run against
     main after merge: same PASS.
   - Criterion 3 ("loaded per request") is proven by test 12, which edits
     discussion_prompt.md between two messages and asserts the new marker reaches the
     model on the second call and not the first — an empirical reload proof, not a
     code reading. It also asserts the context order prompt -> piece -> review.

  COPILOT REVIEW, through the route the task named (**hc**, herdr-core 0.1.21, pane
  t17 "copilot-main", Copilot Fusion / gpt-6-luna). The pane runs in Manual Approval
  mode, so each shell/file tool call needed an approve keypress; six were approved and
  every command it ran was read-only (gh api reads, git diff/status, file reads). It
  reviewed the PR and raised TWO findings, both real:
   1. major — static/sw.js still carried `writing-peer-review-v1` and the worker bytes
      were unchanged, so an already-installed PWA would keep serving the cached
      pre-discussion app.js and never see the new UI. FIXED: CACHE_NAME bumped to v2
      (changing the worker's bytes is what makes a browser install it), guarded by
      test 14.
   2. minor — concurrent POSTs to the same review could read the same thread snapshot
      and write their user/assistant pairs in completion order. FIXED: a per-review
      lock around the read-insert-call-insert path, guarded by test 13. Negative
      control run: with the lock removed test 13 FAILS
      (test_13_concurrent_posts_do_not_interleave), with it restored it passes — the
      test discriminates rather than passing vacuously.
  A follow-up pass on the fix commit returned RESOLVED / RESOLVED / "VERDICT: SHIP",
  with one non-blocking nit (one Lock object retained per review id for the process
  lifetime) which I accepted and answered in the PR rather than churning the code.

  DEVIATIONS AND CORRECTIONS, all declared:
   - 0192 arrived with **no front-matter**; the protocol fields were added at pickup
     (rule 16) and the pickup commit is 58a222a on main.
   - Newton's first cut had a redundant duplicate column (`timestamp` alongside
     `created_at`) in the new messages table. Removed, and every SELECT/INSERT with it.
   - Newton did not cover criterion 3 with a test; test 12 is mine. Tests 13 and 14
     are mine, added in response to the Copilot findings. The README section is mine.
   - The task's `gh` constraint was followed literally: the PR was created with the
     **keyring OAuth login** (`gh auth status` -> account pagosacabin, keyring, scopes
     gist/read:org/repo), `gh-env.sh` was NOT sourced and no GITHUB_TOKEN was
     exported. PR create was probed for write capability *before* the build (a POST
     with a bogus head returned 422 Validation Failed, i.e. permission present, not
     403) — the 0186 failure mode did not recur.
   - Pushes went over SSH as usual, per the task.
   - Queue discipline: active/ 0142 and 0187 are `status: superseded` and were not
     resumed; active/ 0159 is decision-blocked and its file was not touched, not
     re-investigated and its iteration was not changed; inbox/ 0176 is superseded
     ("Do not pick up"). 0192 was the lowest actionable task.
   - No secret appears in this file, the PR, the commits or the screenshot: the
     DeepSeek key lives only in the gitignored .env, and the live smoke used it
     without printing it.
   - Juno owns the VPS redeploy after merge, per the task.
---


# 0192 — Writing app: close the loop — discussion thread on every review

## Assignment
- **Hermes:** mailbox owner. Route the build to Newton via Herdr, supervise, open the PR, drive the Copilot review, stage the reply.
- **Newton:** build the feature (software → Newton first, protocol 1.2.0).
- **Copilot:** peer review the PR, driven through Herdr via **`hc` (herdr-core)** — see Constraints; this task is the live test of that route.
- **Juno:** verifies from origin, archives on green, and redeploys to the VPS after merge (the deploy is Juno's, not this task's).

## Context
Bethany's feedback on the writing app (0186, live at writing.nordtronics.io), relayed by Stephen 2026-11-15: she submits a piece, gets the peer review — and the signal dies there. It's an **open loop**: no way to talk back to the reviewer about the review. A real writing-group peer hands back a critique and then you *discuss* it. This task closes the loop.

Repo: **pagosacabin/cabin-play**, project `writing-peer-review/`. The app is FastAPI + SQLite, PWA frontend, DeepSeek behind `provider.py`, review prompt in `review_prompt.md` (re-read per request). Rung-1 criteria and the existing test suite are the regression floor.

## Task
Add a **discussion thread attached to each review**:

1. **Data.** A messages store tied to `review_id`: role (user/assistant), text, timestamp, in the same SQLite DB. Each review gets its own thread; two reviews on one piece have fully separate threads. Threads persist across server restarts.
2. **Discussion endpoint + provider.** Extend the provider interface with a discussion call. Context sent to the model, in order: a discussion system prompt, the piece text, the review text, then the thread so far. The discussion prompt lives in the repo as **`discussion_prompt.md`**, re-read from disk on every request (same pattern as `review_prompt.md`). Its rules, in substance:
   - You are the peer who wrote this review. Discuss **this piece and this review** — nothing generic.
   - Answer her questions, explain your reasoning, point at her own lines when you make a claim.
   - You may illustrate a point with a short example (a sentence, a paragraph) when she asks or when it genuinely clarifies — never rewrite the whole piece uninvited.
   - Peer, not teacher: honest, specific, no empty praise. If she convinces you a point was wrong, concede it plainly — a peer can change their mind.
3. **UI.** Under each review, a "Discuss this review" panel: the thread, an input box, send. Works in the installed PWA. No streaming required — plain request/response is fine for this rung.
4. **Stub honesty.** If the provider is the stub, discussion replies are stub-marked the same way reviews are. Never fake a live reply.
5. **Export.** Single-piece export gains a third option: piece only / piece + reviews / **piece + reviews + discussions** — in both `.md` and `.pdf`. Portfolio export is unchanged.

**Out of scope:** cross-piece reviewer memory (the reviewer remembering her other pieces) — a later rung. No auth changes, no deploy work, no marketing.

## Success criteria (falsifiable)
1. Post a message to a review's thread → a reply comes back that engages the piece/review content (live provider) or is STUB-marked (stub provider). Thread survives a server restart.
2. Two reviews on the same piece: threads are separate; a message in one never appears in the other.
3. `discussion_prompt.md` exists in the repo and is loaded per request (prove by editing it between two messages and showing the behavior change, or by code pointer + test).
4. Export with the discussions option includes the thread text in both `.md` and `.pdf`; export without it contains none of the thread text.
5. Regression: the existing pytest suite passes, plus new tests for criteria 1, 2 and 4; the live smoke is extended to exercise the discussion flow end-to-end.
6. The PR exists on cabin-play, Copilot's review is posted on it, its findings are addressed in code or answered in the staged reply, and the branch is merged to `main` after the review. The reply cites the **main** SHA.

## Constraints
- **PR creation:** use `gh` with the **keyring OAuth login** (pagosacabin, `repo` scope). **Do NOT source `gh-env.sh`** and do not export its fine-grained PAT as `GITHUB_TOKEN` — that override caused 0186's 403 (proven in 0189/0190). State in the reply which auth `gh` used for the PR create.
- **Copilot routing:** through Herdr via **`hc` (herdr-core 0.1.21)** — never `herdr` 0.9.5, whose "not running" is itself, not the manager. If the PR create or the Copilot review fails, report the exact error in the reply and stop at that gate — the error is data; do not improvise around it and do not merge without the review.
- Branch `hermes/0192-review-discussion`; git pushes over SSH as usual (the gh.py credential cannot write cabin-play).
- **Secrets:** the DeepSeek key stays in `.env` files only — never in the repo, the task, or the reply.
- **Cost:** Newton free tier is the budget. Target one Newton build session; Gemini Flash first per protocol 1.2.0. Report the cost block in the staged reply.
- One deliverable: the discussion feature merged to cabin-play `main` with the Copilot review on its PR. Juno redeploys to the VPS afterward.

## Proof
- PR URL, Copilot review pointer (PR review or comment link), main SHA on origin after merge.
- Test output: full pytest run + the extended live smoke, covering criteria 1–5.
- Plain statement of which provider answered the smoke's discussion (live DeepSeek or STUB) and which `gh` auth created the PR.
- Juno verifies by reading the repo and the PR on origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, branch, PR link, main SHA, proof list, cost block, and any gate failure stated plainly at the top.

---

# Reply — STAGED (2026-11-15 18:03 MST)

**Status:** done. Discussion threads are merged to `pagosacabin/cabin-play` `main`.

| Item | Value |
|---|---|
| PR | https://github.com/pagosacabin/cabin-play/pull/1 |
| Branch / tip | `hermes/0192-review-discussion` @ `f2ec4d6ac684c2251e1b6b2f36545cc3877e9822` |
| Merge commit on `main` | `a7d0dabaef19e8c7557995a4627bb579c7dba4fa` |
| CI | none exists — cabin-play has no `.github/workflows`; no run is claimed |
| Copilot review | https://github.com/pagosacabin/cabin-play/pull/1#issuecomment-6904334810 |
| Copilot verdict on the fixes | https://github.com/pagosacabin/cabin-play/pull/1#issuecomment-6904408807 |
| `pytest -q` | 13 passed (on the branch and again on `main` after merge) |
| `tests/live_smoke.py` | 35/35, discussion reply live DeepSeek, thread survived restart |
| Browser proof | panel works in the served app; 2 messages persisted to SQLite |

## Success criteria

1. **Post a message → a reply that engages the piece/review, thread survives restart.**
   Live smoke: the reply quoted her own kettle line and argued about it (live DeepSeek);
   the same script restarts the server and reads the thread back — 2 messages kept.
   Stub path: browser run with no key returns a `**STUB DISCUSSION**` reply, never a
   faked live one.
2. **Two reviews on one piece have separate threads.** Test 10 posts a unique message to
   each of two reviews on one piece and asserts neither thread contains the other's text.
3. **`discussion_prompt.md` exists and is loaded per request.** Test 12 edits the file
   between two messages and captures what the provider actually sends: the second call
   carries the new marker and not the first one.
4. **Export includes/excludes the thread in `.md` and `.pdf`.** Test 11 asserts presence
   with `include_discussions: true` and absence with it false — in both formats — plus
   the piece-only and backward-compatibility cases, and the live smoke re-checks it
   against `pdftotext` on a real generated PDF.
5. **Regression + new tests + extended live smoke.** 7 existing tests unchanged, 13 pass;
   live smoke grew the discussion steps and stays runnable as a script.
6. **PR exists, Copilot's review is posted, findings addressed, branch merged; reply cites
   the main SHA.** PR #1 merged as `a7d0dab`; both findings are fixed in `f2ec4d6` with
   tests and a negative control; Copilot's follow-up returned VERDICT: SHIP.

## Cost

Newton's build ran on the free Gemini pool (78% remaining before, first pool per
protocol 1.2.0, no switch). DeepSeek usage for this session is not exposed to the cron
harness — the `cost` block says `unknown` with that reason rather than an estimate.
