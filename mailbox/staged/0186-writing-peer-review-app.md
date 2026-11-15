---
task_id: "0186"
protocol_version: 1.2.0
status: staged
iteration: 1
expect-reply-within: 24h
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0186-writing-peer-review
  sha: 8da589792558fb548d2a382818348c562cfd6fa4
  sha_verified: "git ls-remote --heads origin hermes/0186-writing-peer-review -> 8da589792558fb548d2a382818348c562cfd6fa4"
  main_branch: main
  main_sha: 175ac491601dde2f07f49912414fd60e6e31a1f2
  base_sha: c26b60a0bdc88df896f03caadec4b6d583b8f86c
  run: "none - pagosacabin/cabin-play carries no CI at all"
  run_enumeration: 'ls .github -> No such file or directory; git ls-tree -r --name-only origin/main | grep -i "\.github/" -> NONE; gh run list -R pagosacabin/cabin-play -> empty (0 runs)'
  copilot_review: "NOT RUN - skipped under Stephen's explicit one-time waiver (2026-11-15 13:52 MST, relayed in mailbox task 0188). The 0186 run's own review findings substitute."
  smoke: "writing-peer-review/tests/live_smoke.py -> 31/31 checks passed (re-run on main 175ac49)"
  pytest: "writing-peer-review/tests -> 7 passed (re-run on main 175ac49)"
  provider: "live DeepSeek (provider=deepseek, 4.9 s per review) - not STUB"
  files:
    - writing-peer-review/.gitignore
    - writing-peer-review/README.md
    - writing-peer-review/app/__init__.py
    - writing-peer-review/app/config.py
    - writing-peer-review/app/db.py
    - writing-peer-review/app/exercises.py
    - writing-peer-review/app/export.py
    - writing-peer-review/app/main.py
    - writing-peer-review/app/peak.py
    - writing-peer-review/app/provider.py
    - writing-peer-review/requirements.txt
    - writing-peer-review/review_prompt.md
    - writing-peer-review/run.sh
    - writing-peer-review/static/app.js
    - writing-peer-review/static/icon.svg
    - writing-peer-review/static/index.html
    - writing-peer-review/static/manifest.webmanifest
    - writing-peer-review/static/styles.css
    - writing-peer-review/static/sw.js
    - writing-peer-review/tests/__init__.py
    - writing-peer-review/tests/conftest.py
    - writing-peer-review/tests/live_smoke.py
    - writing-peer-review/tests/test_smoke.py
  main_files_untouched_outside_dir: true
  no_ntfy_receipt: "spec names no ntfy topic; the deliverable is a repo tree on cabin-play main, not a device build artifact"
cost:
  model: deepseek-flash (worker) + gemini-3.8-flash-high (Newton build)
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
  notes: "Harness exposes no per-call DeepSeek usage to this cron session, so the token/cost fields are unknown (not estimated). Newton's own usage IS measured: see newton_usage below."
  newton_usage:
    model: gemini-3.8-flash-high
    duration_seconds: 1079.7
    input_tokens: 143216
    output_tokens: 72153
    thinking_tokens: 34249
    cache_read_tokens: 2137385
    weekly_pool_delta_points: 21.53
    weekly_pool_after: "78.47% remaining (Gemini), 100% (Claude/GPT)"
    quota_calls: "1 build session, ended status=interrupted (tree was already complete; re-verified from source)"
notes: |
  MERGED TO MAIN AND STAGED 2026-11-15 21:15-21:3xZ (14:15 MST), off-peak
  (PEAK: OFF-PEAK 21:15 UTC), PROTOCOL MATCH 1.2.0.

  WHAT THIS RUN DID (the merge, not the build). The build and its verification
  were done in the previous 0186 tick; this tick acted on Stephen's decision
  relayed in 0188 and finished the task:
    git ls-remote --heads origin main            -> c26b60a (base)
    git merge-base --is-ancestor origin/main 8da5897 -> true (fast-forward possible)
    git diff --stat origin/main 8da5897          -> 23 files, 3581 insertions(+),
                                                    0 deletions, all under writing-peer-review/
    git merge --no-ff hermes/0186-writing-peer-review -> 175ac49
    git push origin main                         -> c26b60a..175ac49  main -> main
    git ls-remote --heads origin main            -> 175ac491601dde2f07f49912414fd60e6e31a1f2
  A --no-ff merge commit was used so the waiver is visible in the history rather
  than folded into a fast-forward.

  COPILOT REVIEW: SKIPPED, UNDER EXPLICIT ONE-TIME AUTHORIZATION, NOT BY DEFAULT.
  Criterion 7 asks for a Copilot peer review on the branch diff. It did not run:
  the fine-grained PAT in this profile has metadata/contents/issues read on
  cabin-play but no `Pull requests: write` (both PR paths still 403, re-probed
  under 0187), and the Copilot CLI is unauthenticated. 0187's instruction was to
  HOLD, and that is what the previous tick did. 0188 then relayed Stephen's
  decision of 2026-11-15 13:52 MST switching to option (c): merge this once
  without the review, with the 0186 run's own findings substituting. The standing
  rule (Copilot peer review on GitHub-bound work, 2026-11-13) is NOT repealed and
  option (a), the token grant, is deferred rather than cancelled. Under that
  waiver the merge proceeded; the substituted review is below.

  THE SUBSTITUTED REVIEW (what the 0186 run found and fixed):
   - DEFECT FOUND AND FIXED: the service worker was served only at /static/sw.js,
     so its scope was /static/ and the app shell at "/" was never cached - the PWA
     was not installable/offline-capable even though manifest and sw.js existed.
     Now served at /sw.js with Service-Worker-Allowed: / and registered with
     { scope: '/' }. The browser check confirmed getRegistration() -> scope
     http://127.0.0.1:8799/, active. Newton's own test asserted the old path and
     was updated rather than left certifying the defect.
   - SCOPE EXTENSIONS: tests/live_smoke.py (criterion 9 - Newton's pytest is
     in-process and provider-mocked, so a live script is what proves the real
     loop); run.sh bootstraps .venv on a fresh clone (criterion 1 promises one
     documented command); the /sw.js root route above.
   - DEVIATIONS: Herdr's server is not running on this host (`herdr status` ->
     server: not running), so Newton was driven headless via `agy` directly. The
     Newton session ended `status: ERROR, error: interrupted` after 1079 s having
     already written the complete tree; everything was re-verified from source
     instead of rebuilt.

  RE-VERIFIED THIS RUN, ON THE MERGED MAIN TREE (not quoted from the old tick):
    .venv/bin/python -m pytest tests -q
      -> 7 passed, 1 warning in 0.31s
    .venv/bin/python tests/live_smoke.py
      -> 31/31 checks passed, covering criteria 1-6 and 8, including:
         C1 /sw.js 200 with Service-Worker-Allowed=/
         C2 15 exercises, 5 types, random deal across 7 distinct ids
         C3 piece + review survive a server restart
         C4 four sections in order (positions 0/786/1914/2059), 6 verified
            quotes, provider=deepseek in 4.9 s  (LIVE, not STUB)
         C5 md + pdf carry the full text (pdf read back with pdftotext), and the
            piece-only toggle drops reviews
         C6 3-piece portfolio in chosen order in both md and pdf, title page
            carries the display name, unselected piece absent
         C8 all five fixed UTC cases plus /api/status agreeing with the clock
            (now=Sun 21:16Z -> Off-peak until 2026-11-16T01:00:00Z)

  PROVIDER: live DeepSeek, not the stub. The key lives only in the gitignored
  .env (mode 600) next to run.sh; `git grep 'sk-'` on the merged tree is clean and
  the key appears nowhere in this reply. .gitignore covers .env, .venv/, *.db,
  data/.

  NOT DONE, ON PURPOSE: no VPS deploy, no auth, no marketing - rung 1 is the
  local loop only, as the task specifies.

  SUPERSESSION: 0187 (option (a), token grant) is superseded by 0188 and was not
  acted on beyond the hold it instructed; no PR was created under it.
---

# 0186 — Bethany's writing peer-review app, rung 1

## Assignment
- **Hermes:** mailbox owner. Route the build to Newton via Herdr, supervise, run the smoke test on the laptop, drive the Copilot peer review, stage the reply.
- **Newton:** build the app (software → Newton first, protocol 1.2.0).
- **Copilot:** peer review the branch diff before merge (GitHub-bound work rule, 2026-11-13). Reviewer only — Juno verifies independently on top.
- **Juno:** verifies from origin (repo files + smoke output), archives on green.

## Context
Bethany (Stephen's partner) writes but has no writing circle — nobody who can peer review her work at a serious level. This app fills that gap: it deals her a writing exercise, she writes, and an AI reviews the piece the way a good writing-group peer would — specific, honest, craft-focused — then deals a follow-up exercise aimed at what the review found. It is a personal tool for one user (Bethany), not a product. Marketing is a maybe-later; do not build for it.

Personal/play project → it lives in **pagosacabin/cabin-play**, never nordtronics. New project directory: `writing-peer-review/`.

Engineering rule: start small, then go big. **Rung 1 proves the full loop running locally on the laptop.** Deployment to the VPS and Bethany's phone access are a later rung — do not deploy anywhere in this task.

## Task
Build `writing-peer-review/` in cabin-play:

**Stack (pinned for provability):** Python 3 + FastAPI + SQLite (one file DB), vanilla HTML/CSS/JS frontend with **no build step**, installable as a PWA (web app manifest + a service worker that caches the app shell). PDF export via a Python library (e.g. weasyprint or reportlab — Newton's pick, note it in the reply). Minimal dependencies; pin them in `requirements.txt`.

**Features — all of these, nothing more:**
1. **Exercise library.** Seed ≥12 exercises across ≥5 types (scene, dialogue, character, description, flash fiction minimum). Each: title, prompt text, suggested length. A "deal me one" random pick plus a browsable list, plus a free-write option (no prompt).
2. **Writing screen.** Title + body, autosave to the server as she types (debounced is fine), word count shown. Pieces persist in SQLite across server restarts.
3. **Peer review.** Submitting a piece calls the LLM and stores the returned review with the piece. The review prompt lives in the repo as a readable file (`review_prompt.md`) so Stephen and Bethany can tune the voice without touching code. Review structure, in order:
   - What worked — specific, quoting her own lines back at least twice.
   - Two or three craft points (voice, pacing, where a scene turns, dialogue, imagery) — focused, not a twenty-item correction dump.
   - At least one genuine question back to the writer.
   - One follow-up micro-exercise aimed at what the review found.
   Review rules baked into the prompt: peer, not teacher; no empty praise; suggestions only — never rewrite her piece for her.
4. **History.** List of her pieces with dates; reopen, revise, resubmit for a fresh review (old reviews kept, listed under the piece).
5. **Export.**
   - Single piece → `.md` and `.pdf`. Toggle: piece only, or piece + its reviews.
   - **Portfolio:** select multiple pieces, set their order, generate one portfolio as a single `.pdf` plus a single `.md`. Title page carries a display name from a settings field (default: "Bethany"). Reviews excluded by default; a toggle includes them.
6. **LLM provider behind an interface.** `provider.py` with one function that takes (piece text, exercise prompt if any) and returns review text. DeepSeek chat API is the first implementation. **Key source (Stephen's explicit call, 2026-11-15): use the DeepSeek API key from Hermes's own harness configuration on the laptop** — the key Hermes himself runs on, wherever his config keeps it (e.g. `~/.hermes/.env`). For this rung-1 test the app may read that same key via its own gitignored `.env` on the laptop. This authorization is for this local test only. The key is **never committed, never printed in logs or the reply, and never leaves the laptop**. Fallback only if the key genuinely cannot be located in Hermes's config: ship a stub provider returning a canned review clearly marked `STUB` at the top, and say so plainly in the staged reply — a stubbed rung is an honest rung, a silent stub is not.

7. **Peak / off-peak indicator (Stephen's addition, 2026-11-15).** The app will eventually be up 24/7, and DeepSeek review calls cost double during peak — so every screen carries a slim status strip in the header: a colored dot plus a text label (never color alone). **Green = "Off-peak"**, **red = "Peak"**, and the strip also names when the status next flips (e.g. "Off-peak until 6:00 PM"). Rules: peak is **01:00–04:00 and 06:00–10:00 UTC, Monday–Friday**; everything else, including all weekend hours, is off-peak. Compute from the **UTC clock**, not local time — the windows are fixed in UTC. The Chinese-holiday exception is **not** in rung 1; note that simplification in the README. The indicator is informational only — it never blocks or delays a review; Bethany writes when she writes.

**No auth in rung 1** (local-only on the laptop). Note in the project README that auth arrives with the VPS rung.

## Success criteria (falsifiable)
1. App starts on the laptop with one documented command; `GET /` serves the PWA (manifest + service worker present and linked).
2. Exercise library seeds ≥12 exercises across ≥5 types on first run; "deal me one" returns a random one.
3. A piece created in the UI is still there after the server is stopped and restarted (SQLite persistence).
4. Submitting a piece produces a stored review containing all four sections in order (worked / craft points / question / follow-up), with ≥2 quoted lines from the piece. If the provider is the stub, the review is marked `STUB` and the reply states it.
5. Single-piece export produces a `.md` and a `.pdf` that contain the piece's full text.
6. Portfolio export: given 3 selected pieces in a chosen order, the `.pdf` and `.md` each contain exactly those 3 pieces, in that order, with a title page showing the settings display name.
7. Copilot peer review ran on the branch diff; its findings are addressed in code or answered in the staged reply.
8. The peak/off-peak indicator is correct against fixed UTC test cases (compute from a clock the test can set): Sunday 15:00 UTC → off-peak; Tuesday 02:00 UTC → peak; Tuesday 05:00 UTC → off-peak (the gap between windows); Friday 09:00 UTC → peak; Friday 11:00 UTC → off-peak. The next-flip time shown matches the window boundary in each case.
9. A smoke script (pytest or curl-based, Newton's pick) exercises criteria 1–6 and 8 end-to-end; its output is in the staged reply or committed under `writing-peer-review/tests/`.

## Constraints
- **Repo: pagosacabin/cabin-play only.** The gh.py credential is nordtronics-scoped — cabin-play writes over the API return 403. Push via SSH from the laptop, as with 0136.
- Branch `hermes/0186-writing-peer-review`; Copilot review on the branch; merge to cabin-play `main` after review; staged reply cites the **main** SHA.
- **Secrets:** no API key in the repo, the task, or the reply. The one authorized key source for this test is Hermes's own DeepSeek key in his laptop config (Stephen, 2026-11-15) — it stays on the laptop, in the app's gitignored `.env` only. Do not pull keys from any other context.
- **Cost:** Newton free tier is the budget (~4.4 sweeps/week across all work). Target one Newton build session; Gemini Flash first per protocol 1.2.0. Report the cost block (model, tokens, quota) in the staged reply.
- One deliverable: the working rung-1 app on cabin-play main. No VPS deploy, no extra features, no marketing anything.

## Proof
- cabin-play `main` SHA on origin after merge, branch name, and the Copilot review summary.
- Smoke-test output covering criteria 1–6 and 8 (pasted or committed under `writing-peer-review/tests/`).
- Plain statement of which provider ran the review in the smoke test: live DeepSeek or `STUB`.
- Juno verifies by reading the repo on origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, branch, main SHA, proof list, cost block, and any blockers (e.g. API key not staged) stated plainly at the top.


# Reply — STAGED (merged 2026-11-15 21:2xZ, task 0188 waiver applied)

**Status: done.** `writing-peer-review/` is on `pagosacabin/cabin-play` **main** at
`175ac491601dde2f07f49912414fd60e6e31a1f2`, merged from branch
`hermes/0186-writing-peer-review` (tip `8da589792558fb548d2a382818348c562cfd6fa4`,
verified against `git ls-remote --heads origin`).

**Blocker stated plainly at the top:** the Copilot peer review (criterion 7) did **not**
run. The PAT in this profile has no `Pull requests: write` (both PR paths 403) and the
Copilot CLI is unauthenticated, so there is no PR to attach a review to. The merge went
ahead only under Stephen's explicit **one-time** waiver of 2026-11-15 13:52 MST, relayed
in task 0188. The 0186 run's own review stands in — including a real defect it found and
fixed (the service worker was served under `/static/`, so the app shell was never
cached and the PWA was not installable; now served at `/sw.js` with
`Service-Worker-Allowed: /` and `scope: '/'`). The standing Copilot rule is not repealed
and the token grant is deferred, not cancelled.

## Proof
- **Branch:** `hermes/0186-writing-peer-review` @ `8da589792558fb548d2a382818348c562cfd6fa4`
- **Main SHA:** `175ac491601dde2f07f49912414fd60e6e31a1f2` (merge commit, `--no-ff`)
- **CI run:** none — cabin-play has no CI at all. Enumerated, not asserted: `ls .github`
  → no such directory, `git ls-tree -r --name-only origin/main | grep -i '\.github/'` →
  NONE, `gh run list -R pagosacabin/cabin-play` → empty.
- **Copilot review:** skipped under the 0188 waiver (see above); substituted findings listed.
- **Smoke (re-run this tick on the merged main tree):**
  - `pytest tests -q` → **7 passed**
  - `tests/live_smoke.py` → **31/31 checks passed** (criteria 1–6 and 8 end-to-end,
    live uvicorn subprocess, throwaway SQLite, real network calls)
- **Provider:** live **DeepSeek** (`provider=deepseek`), 4.9 s per review — **not** `STUB`.
- **Files:** 23 files, 3581 insertions, 0 deletions, all under `writing-peer-review/`;
  nothing else in cabin-play main moved.
- **ntfy:** not applicable — the spec names no topic and the deliverable is a repo tree,
  not a device build artifact.

## Cost
`deepseek-flash`, off-peak. Per-call usage is not visible to this cron session, so token
and dollar fields are `unknown` rather than estimated. Newton's build usage is measured
and carried in front-matter `newton_usage` (gemini-3.8-flash-high, 1079.7 s, 143216 in /
72153 out / 34249 thinking, 21.53 pool points, 78.47 % of the Gemini pool left).

## Notes
The build was done in the previous tick; this tick did the merge and re-verified from
source. Full reasoning, scope extensions and deviations are in front-matter `notes:`.
