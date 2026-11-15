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
6. **LLM provider behind an interface.** `provider.py` with one function that takes (piece text, exercise prompt if any) and returns review text. DeepSeek chat API is the first implementation. The key comes from the `DEEPSEEK_API_KEY` environment variable or a gitignored `.env` — **never committed, never printed in logs or the reply**. If no key is reachable in the build environment, ship a stub provider that returns a canned review clearly marked `STUB` at the top, and say so plainly in the staged reply — a stubbed rung is an honest rung, a silent stub is not.

**No auth in rung 1** (local-only on the laptop). Note in the project README that auth arrives with the VPS rung.

## Success criteria (falsifiable)
1. App starts on the laptop with one documented command; `GET /` serves the PWA (manifest + service worker present and linked).
2. Exercise library seeds ≥12 exercises across ≥5 types on first run; "deal me one" returns a random one.
3. A piece created in the UI is still there after the server is stopped and restarted (SQLite persistence).
4. Submitting a piece produces a stored review containing all four sections in order (worked / craft points / question / follow-up), with ≥2 quoted lines from the piece. If the provider is the stub, the review is marked `STUB` and the reply states it.
5. Single-piece export produces a `.md` and a `.pdf` that contain the piece's full text.
6. Portfolio export: given 3 selected pieces in a chosen order, the `.pdf` and `.md` each contain exactly those 3 pieces, in that order, with a title page showing the settings display name.
7. Copilot peer review ran on the branch diff; its findings are addressed in code or answered in the staged reply.
8. A smoke script (pytest or curl-based, Newton's pick) exercises criteria 1–6 end-to-end; its output is in the staged reply or committed under `writing-peer-review/tests/`.

## Constraints
- **Repo: pagosacabin/cabin-play only.** The gh.py credential is nordtronics-scoped — cabin-play writes over the API return 403. Push via SSH from the laptop, as with 0136.
- Branch `hermes/0186-writing-peer-review`; Copilot review on the branch; merge to cabin-play `main` after review; staged reply cites the **main** SHA.
- **Secrets:** no API key in the repo, the task, or the reply. If a key must come from Stephen, it will be staged at `/home/astroboy/.hermes/` and the reply names that as the blocker — do not invent or reuse a key from another context.
- **Cost:** Newton free tier is the budget (~4.4 sweeps/week across all work). Target one Newton build session; Gemini Flash first per protocol 1.2.0. Report the cost block (model, tokens, quota) in the staged reply.
- One deliverable: the working rung-1 app on cabin-play main. No VPS deploy, no extra features, no marketing anything.

## Proof
- cabin-play `main` SHA on origin after merge, branch name, and the Copilot review summary.
- Smoke-test output covering criteria 1–6 (pasted or committed under `writing-peer-review/tests/`).
- Plain statement of which provider ran the review in the smoke test: live DeepSeek or `STUB`.
- Juno verifies by reading the repo on origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, branch, main SHA, proof list, cost block, and any blockers (e.g. API key not staged) stated plainly at the top.
