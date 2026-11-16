---
task_id: "0195"
protocol_version: 1.2.0
status: verified
iteration: 1
expect-reply-within: 24h
proof:
  - branch: hermes/0195-voice-band-line
    sha: eb26567e6a8f5266d7150ff51d1a4be10dace931
  - secondary_branch: hermes/0195-voice-check
    sha: e8d5f15c3626534923fc8ae5ff0d5f6940c0364d
  - pr: https://github.com/pagosacabin/cabin-play/pull/2
    pr_followup: https://github.com/pagosacabin/cabin-play/pull/3
  - main_sha: 9c109c053915cb7fafbccd2ac29e47cc2ebfbb47
  - copilot_review: https://github.com/pagosacabin/cabin-play/pull/2#issuecomment-6922788418
    copilot_review_followup: https://github.com/pagosacabin/cabin-play/pull/3#issuecomment-6922956289
    copilot_verdict: SHIP (both PRs; BLOCK first on each, then SHIP after the fixes)
  - run: none
    reason: >-
      cabin-play has no .github/workflows directory at all, so no workflow exists to
      run. Enumerated: `ls -la .github/workflows` -> "No such file or directory";
      `gh run list -R pagosacabin/cabin-play --limit 3` returns zero runs for the whole
      repo. The acceptance evidence here is the offline pytest suite + the live smoke +
      the two Copilot reviews, not a CI run.
  - files:
      - writing-peer-review/review_prompt.md
      - writing-peer-review/discussion_prompt.md
      - writing-peer-review/app/provider.py
      - writing-peer-review/tests/test_smoke.py
      - writing-peer-review/tests/live_smoke.py
      - writing-peer-review/README.md
  - ntfy: not applicable (no compiled artifact; this is a Python prompt/app rung and
      cabin-play publishes no build)
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
  notes: >-
    The harness exposes no per-call DeepSeek usage to this cron session, so the token and
    cost fields are unknown rather than estimated. Free-tier pools spent instead: the Newton
    (Gemini 3.8 Flash High) headless session took the Gemini weekly pool 43% -> 34% (-9
    points) before it stalled; the Copilot pane session went 3.55 -> 6.37 AIC across three
    review turns. DeepSeek calls made: 6 live reviews + 4 live discussion turns across the
    smoke runs (each smoke = 2 reviews + 2 discussion turns).
notes: |
  STAGED — delivered through two merged PRs on cabin-play. Ran at
  2026-11-16 16:15-17:00Z (09:15-10:00 MST), off-peak (PEAK: OFF-PEAK 16:15
  UTC), PROTOCOL MATCH 1.2.0.

  WHAT WAS BUILT. A fifth review section, `## Voice check`, appended after
  Follow-up exercise. Its first line is exactly one of `Reads human.` /
  `Mixed.` / `Reads machine-made.`; when passages read machine-made the review
  flags at most three, each as a bullet opening with her own quoted passage and
  a one-sentence peer-voice reason (stock phrasing, even rhythm, generic
  transition, summary tone). No detector API, no percentage, no gauge —
  `review_prompt.md` states plainly that this is an impression, never a
  measurement or an accusation.

  ROUTING. Newton first, per protocol 1.2.0: `agy -p` (gemini-3.8-flash-high,
  headless, --dangerously-skip-permissions) was given the build and produced the
  prompt/stub/discussion edits. It then stalled — no log line or tool call for 13
  minutes after its last response, `--print-timeout 13m` not triggering, no child
  process — so the session was terminated and the harness finished the tests and
  verification itself. Stated plainly: Newton wrote the prompt, stub and
  discussion changes; the tests, the live-smoke extensions, the band normaliser
  and all verification are mine.

  COPILOT GATE, BOTH ROUNDS BLOCKED FIRST AND SHIPPED AFTER FIXES.
  PR #2 (branch hermes/0195-voice-check): findings were (1) the stub's band was an
  input-independent verdict, (2) the machine-flavoured fixture accepted a human
  band and did not cap quotes, (3) the percentage grep missed "80 percent". All
  three fixed in e8d5f15 — the stub now says its band is a placeholder, not a live
  read; the fixture must band Mixed./Reads machine-made.; the grep catches spelled
  percent too. Follow-up verdict: SHIP.
  A verification run AT MERGED MAIN then exposed a defect neither PR had caught:
  the live model wrote the band and its explanation on ONE line ("Reads
  machine-made. A few passages carry ..."), so "first line" was no longer the
  band, and quote counts varied 3/4/5 across runs. That is what PR #3 fixes:
  `normalize_voice_check()` splits a band that continues into prose, matching the
  heading only as a standalone line and preserving markdown emphasis; the cap is
  restated as three *flagged passages* (one bullet each) rather than three
  quotation marks; `What worked` now says its two-quotation requirement is checked
  in that section. PR #3's Copilot round also blocked first (emphasis mangling from
  lstrip, `find()` matching a quoted mention of the heading, no minimum-flag
  assertion) and shipped after eb26567. Merged as 9c109c0.

  VERIFIED, NOT ASSUMED.
  - merged main 9c109c0 checked out and pulled; `pytest tests/test_smoke.py -q` ->
    15 passed (was 13 before this task; +test_4b stub shape, +test_4c normaliser).
  - `tests/live_smoke.py` at merged main -> 46/46 checks, 0 FAIL (rc 0). Also green
    twice at the PR #3 tip before merging. The smoke asserts, live: five sections in
    order; the band verbatim in both fixtures; human fixture bands Reads human. with
    5+ verified quotes; machine fixture bands Reads machine-made. with exactly 3
    flagged passages of 6-8 quotations; no `N%`, no spelled "percent", no
    "AI score/detector" in either review body; the voice check survives .md and .pdf
    export (pdftotext); a discussion question about the voice check gets a live reply
    that engages the flagged lines.
  - Sampled honestly: earlier runs on pre-normalisation trees failed on that merged
    band line (twice) and once on the pre-existing `C4 >=2 quoted lines` check. The
    greens quoted here are at the tip; the reds are why the tip changed.
  - Branch tips verified with `git ls-remote --heads origin`: hermes/0195-voice-band-line
    = eb26567 (PR #3 head, merged), hermes/0195-voice-check = e8d5f15 (PR #2 head,
    merged), main = 9c109c0.

  DEVIATIONS DECLARED.
  1. Newton's headless run stalled and was terminated; the harness completed the
     build. Recorded above and in the cost block (-9 Gemini points for a partial run).
  2. Two extra PRs rather than one, because the merged-main verification found a real
     defect after PR #2 merged. PR #3 is a follow-up to a merged PR, not a re-do.
  3. README updated (not in the task's file list): it documented "four exact markdown
     headers" and would have been stale. Also a short subsection stating there is no
     percentage/detector, so the omission is a decision on the record.
  4. `gh` used with the keyring OAuth login (pagosacabin, repo scope). `gh-env.sh` was
     not sourced and no PAT exported — checked: zero GH_TOKEN/GITHUB_TOKEN in env.
  5. The live smoke's machine-fixture assertions are stricter than the spec text in two
     places (band must not be "Reads human."; at least one bullet must open with the
     quote). Both exist because Copilot flagged the looser form as a test gap.
  6. `--delete-branch` was used on PR #2, then the branch was re-pushed at e8d5f15 so the
     proof pointer is verifiable; PR #3's branch was left in place.
---


# 0195 — Writing app: voice check — flag passages that read machine-made

## Assignment
- **Hermes:** mailbox owner. Route the build to Newton via Herdr, supervise, open the PR, drive the Copilot review, stage the reply.
- **Newton:** build the feature (software → Newton first, protocol 1.2.0).
- **Copilot:** peer review the PR, driven through Herdr via **`hc` (herdr-core)** — same route as 0192/0194.
- **Juno:** verifies from origin, archives on green, and redeploys to the VPS after merge (the deploy is Juno's, not this task's).

## Context
Stephen's suggestion, 2026-11-16: some kind of scoring that keeps AI content to a minimum — while the app reviews a piece, it should be able to say "this is too much AI" or flag how much of the piece reads as AI, because the wider writing world is putting up more and more guards against AI content, and a writer is better off hearing it from her own reviewer first.

**The honest constraint (Juno, verified position — do not engineer around it):** there is no reliable "% AI" measurement. AI-detector scores are statistical guesses with well-documented false positives on human writing; no detector can truthfully say a piece is "37% AI." This task therefore does **not** integrate a detector API and does **not** display a percentage. What a good peer reviewer *can* do — and what actually helps — is read with an ear for machine-made prose and point at it: stock phrases, uniform sentence rhythm, generic transitions, summary-tone passages. That is a craft judgment with quoted evidence, and it is what this task builds.

Current state (verified by Juno against the deployed build, 2026-11-16): reviews are generated from `review_prompt.md` (re-read from disk per request), which mandates EXACTLY four markdown headings in order — What worked / Craft points / A question for you / Follow-up exercise — and rule 7 already forbids rewriting her piece. The live smoke asserts the four sections in order. cabin-play main is at a7d0dab plus whatever 0194 (delete-a-piece) lands — branch from current main at pickup.

## Task
Add a **voice check** to every review:

1. **Prompt.** Extend `review_prompt.md` with a fifth section, appended after Follow-up exercise:
   - Heading: `## Voice check`
   - First line is exactly one band, verbatim one of: `Reads human.` / `Mixed.` / `Reads machine-made.` — the reviewer's overall ear for the piece.
   - Then: if any passage reads machine-made, quote up to 3 of her own passages and say briefly, in peer voice, what makes each one read that way (stock phrasing, even rhythm, generic transition, summary tone). If nothing reads machine-made, one line saying so plainly — no invented flags to fill the section.
   - Rules to state in the prompt: this is a peer's impression, never a measurement; never a percentage; never an accusation — she may have written every word, and the point is how it *reads* to an outside ear, because outside readers and platforms will judge the reading, not the process.
   - Update the prompt's "EXACTLY four headings" language and its rules list to match the five-section shape.
2. **Stub honesty.** Stub reviews include the Voice check section, clearly stub-consistent (the stub's existing marking conventions apply). Never fake a live read.
3. **Display.** The review renders all five sections as it does today (plain markdown rendering is fine — no special badge or gauge UI in this rung; a percentage-style gauge is explicitly out).
4. **Discussion awareness (small).** The discussion prompt (`discussion_prompt.md`) gains one line: if she asks about the voice check, discuss it the same way — point at her lines, concede plainly if she convinces you a passage reads human after all. Same concession rule as the rest of the discussion prompt.

**Out of scope:** any third-party AI-detector API or percentage score; statistical text analysis (perplexity/burstiness tooling); per-paragraph highlighting UI; export format changes (exports already carry the full review text, so the voice check rides along — verify and state that in the reply); auth changes; deploy work.

## Success criteria (falsifiable)
1. A live review of a normal human-style test piece contains all five sections in order, the Voice check's first line is one of the three bands verbatim, and any flagged passage is quoted from the piece.
2. A live review of a deliberately machine-flavoured test piece (generic, stock-phrased — fixture text in the smoke) flags at least one quoted passage in the Voice check. If the live model will not flag it, report that honestly rather than tuning the test to pass.
3. No percentage or numeric AI score appears anywhere in the review output or UI (grep the outputs in the smoke and say so).
4. A discussion message asking about the voice check gets a reply that engages the flagged passage (live) or is STUB-marked (stub).
5. Regression: existing pytest suite passes, updated for the five-section shape; the live smoke is extended to criteria 1–3 and passes. Exports still include the full review text (voice check included) in .md and .pdf.
6. The PR exists on cabin-play, Copilot's review is posted on it, its findings are addressed in code or answered in the staged reply, and the branch is merged to `main` after the review. The reply cites the **main** SHA.

## Constraints
- **PR creation:** use `gh` with the **keyring OAuth login** (pagosacabin, `repo` scope). **Do NOT source `gh-env.sh`** and do not export its fine-grained PAT as `GITHUB_TOKEN`. State in the reply which auth `gh` used.
- **Copilot routing:** through Herdr via **`hc` (herdr-core 0.1.21)** — never `herdr` 0.9.5. If the PR create or the Copilot review fails, report the exact error and stop at that gate — do not merge without the review.
- Branch `hermes/0195-voice-check`, based on cabin-play `main` as it stands at pickup (0194 may have landed first — rebase on current main, do not redo delete work). Git pushes over SSH as usual.
- **Secrets:** the DeepSeek key stays in `.env` files only — never in the repo, the task, or the reply.
- **Cost:** Newton free tier is the budget. Target one Newton build session; Gemini Flash first per protocol 1.2.0. Report the cost block in the staged reply.
- One deliverable: the voice check merged to cabin-play `main` with the Copilot review on its PR. Juno redeploys to the VPS afterward.

## Proof
- PR URL, Copilot review pointer, main SHA on origin after merge.
- Test output: full pytest run + the extended live smoke, covering criteria 1–5, including the two contrasting fixtures (human-style vs machine-flavoured).
- Juno verifies by reading the repo and the PR on origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, branch, PR link, main SHA, proof list, cost block, and any gate failure stated plainly at the top.

---

# Reply — STAGED (2026-11-16 09:58 MST)

**Status: done.** The voice check is merged to `cabin-play` `main` at **`9c109c0`**, with Copilot's review on both PRs and its SHIP verdict after the fixes.

## What ships

Every review now ends with a fifth section:

```
## Voice check
Reads machine-made.

- "In today's fast-paced world, it is important to note that change is a constant companion." — stock essay-opener phrasing; the frame and the hedge could open any piece on any topic.
- "serves as a testament to the resilience of the human spirit" — a ready-made cliché that summarises feeling instead of rendering it.
```

The band line is one of exactly three, verbatim: `Reads human.` / `Mixed.` / `Reads machine-made.` At most three passages are flagged, one bullet each, each opening with her own words. When nothing reads machine-made, the section says so in one line — no invented flags.

There is **no** AI-detector API and **no** percentage anywhere, and the prompt says why: this is a peer's impression, never a measurement and never an accusation — she may have written every word, and outside readers judge the reading, not the process.

## Deliverables on origin

| What | Where |
|---|---|
| PR #1 of this task | https://github.com/pagosacabin/cabin-play/pull/2 — merged, `5ff4c93` |
| PR #2 of this task (follow-up fix) | https://github.com/pagosacabin/cabin-play/pull/3 — merged, `9c109c0` |
| Branch tip (proof) | `hermes/0195-voice-band-line` @ `eb26567` |
| Main SHA | `9c109c0` (`git ls-remote --heads origin main`) |
| Copilot review | PR #2 comment `6922788418`, PR #3 comment `6922956289` — BLOCK then SHIP on each |
| CI | `run: none` — cabin-play has no `.github/workflows` at all (`ls` -> no such directory; `gh run list` returns zero runs for the repo) |

## Verification this run performed

- `pytest tests/test_smoke.py -q` on merged `main`: **15 passed** (13 before this task; `test_4b` covers the stub's five-section shape and placeholder band, `test_4c` covers the band normaliser).
- `tests/live_smoke.py` on merged `main`: **46/46 checks passed, 0 FAIL** — five sections in order and the band verbatim on both fixtures, the machine-flavoured fixture banding `Reads machine-made.` with 3 flagged passages of 6–8 quotations, the human fixture banding `Reads human.`, no percent sign / spelled "percent" / "AI score" in either body, the section surviving `.md` and `.pdf` export (via `pdftotext`), and a live discussion reply that engages the flagged lines.
- Both review rounds were BLOCK first; nothing merged until the findings were fixed in code and a follow-up check returned SHIP.

## Two things worth your attention

1. **Newton stalled.** `agy -p` on Gemini 3.8 Flash High produced the prompt, stub and discussion edits, then went silent for 13 minutes with no tool call and no exit; `--print-timeout 13m` did not fire. The session was killed and I finished the tests and verification myself. Gemini pool went 43% -> 34% for that partial run, so a headless long-form build is not reliably cheaper than doing it here.
2. **A defect only showed up after the first merge.** A verification run at merged main found the model writing the band and its explanation on one line, so "first line is the band" silently broke. That is now fixed in the app (`normalize_voice_check` splits it, matching the heading only as a standalone line and leaving markdown emphasis intact) rather than left to prompt luck — PR #3 exists for exactly that, and the smoke now asserts it.

Fetch notes are in the front-matter: `cost` is `unknown` because this harness exposes no per-call DeepSeek usage, and the six declared deviations (Newton stall, two PRs instead of one, README scope extension, keyring auth, stricter smoke assertions than the spec text, and the branch re-push) are listed there rather than buried.
