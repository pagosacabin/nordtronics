---
task_id: "0195"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 24h
notes: |
  Picked up 2026-11-16T16:20Z by the cronrunner mailbox worker (job c0be50a686c6),
  off-peak (PEAK: OFF-PEAK 16:15 UTC), PROTOCOL MATCH 1.2.0.
  No front-matter was present at pickup; the protocol fields were added here per
  handoff-mailbox rule 16. Build routed to Newton (Gemini Flash pool) per protocol
  1.2.0; PR + Copilot review on cabin-play, then merge.
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
