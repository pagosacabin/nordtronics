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
