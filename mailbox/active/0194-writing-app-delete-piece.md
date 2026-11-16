---
task_id: "0194"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 24h
notes: |
  Front-matter added at pickup (the file shipped with none, per rule 16).
---

# 0194 — Writing app: delete a piece from history

## Assignment
- **Hermes:** mailbox owner. Route the build to Newton via Herdr, supervise, open the PR, drive the Copilot review, stage the reply.
- **Newton:** build the feature (software → Newton first, protocol 1.2.0).
- **Copilot:** peer review the PR, driven through Herdr via **`hc` (herdr-core)** — same route as 0192.
- **Juno:** verifies from origin, archives on green, and redeploys to the VPS after merge (the deploy is Juno's, not this task's).

## Context
Stephen's suggestion, 2026-11-16, after looking at the writing app (live at writing.nordtronics.io; cabin-play main has since advanced through 0195's merge at 9c109c0 — branch from current main at pickup): in the history, there is no way to delete one of your works. If you pressed the wrong button, or wrote a paragraph and decided not to go that direction, the piece sits in your history forever. He wants the ability to delete a single piece.

Current state of the app (verified by Juno against the deployed build, 2026-11-16):
- `app/db.py`: `pieces` table; `reviews.piece_id` references `pieces(id)` **ON DELETE CASCADE**; `messages.review_id` references `reviews(id)` **ON DELETE CASCADE**; connections set `PRAGMA foreign_keys = ON`. The schema already supports a clean cascade — one delete at the piece level takes its reviews and their discussion threads with it.
- `app/main.py`: pieces have POST (create), GET (list + single), PUT (update) — **no DELETE endpoint exists**.
- Portfolio export takes a list of piece ids and selects each by id.

## Task
Add **delete-a-piece**, end to end:

1. **Endpoint.** `DELETE /api/pieces/{piece_id}`: deletes the piece; its reviews and their discussion messages go with it (rely on the schema's cascades, and prove they fired). Deleting a piece id that does not exist returns 404, not a 500. Response confirms the deletion (shape consistent with the existing endpoints).
2. **UI.** In the **history list**, each piece gets a delete control. Tapping it asks for confirmation first, naming the piece's title ("Delete '<title>'? Its reviews and discussions go too — this can't be undone."). Confirming deletes and returns her to the history list with the piece gone, without a manual refresh. Cancelling changes nothing. Works in the installed PWA.
3. **No soft-delete, no trash, no undo** in this rung — hard delete behind the confirmation is the whole feature. (If you think the cascade does not actually fire in the app's runtime configuration, stop and report that instead of hand-rolling deletes table by table without saying so.)

**Out of scope:** deleting individual reviews or messages, editing history, bulk delete, auth changes, deploy work.

## Success criteria (falsifiable)
1. Create a piece, review it, post a discussion message → delete the piece via the endpoint → the piece, its review, and the message are all gone from the database; a second piece with its review is untouched.
2. `DELETE` on a non-existent piece id returns 404.
3. In the UI: the history list shows the delete control; the confirmation names the piece; cancelling leaves the piece in the list; confirming removes it from the list with no manual refresh.
4. A portfolio export requested after the deletion does not include the deleted piece and does not error when a stale id is in the request list — or, if the current export errors on a missing id, fix that as part of this task and say so in the reply.
5. Regression: the existing pytest suite passes, plus new tests for criteria 1 and 2; the live smoke is extended to exercise delete end-to-end.
6. The PR exists on cabin-play, Copilot's review is posted on it, its findings are addressed in code or answered in the staged reply, and the branch is merged to `main` after the review. The reply cites the **main** SHA.

## Constraints
- **PR creation:** use `gh` with the **keyring OAuth login** (pagosacabin, `repo` scope). **Do NOT source `gh-env.sh`** and do not export its fine-grained PAT as `GITHUB_TOKEN` — that override caused 0186's 403 (proven in 0189/0190, route proven working in 0192). State in the reply which auth `gh` used for the PR create.
- **Copilot routing:** through Herdr via **`hc` (herdr-core 0.1.21)** — never `herdr` 0.9.5. If the PR create or the Copilot review fails, report the exact error in the reply and stop at that gate — do not merge without the review.
- Branch `hermes/0194-delete-piece`; git pushes over SSH as usual (the gh.py credential cannot write cabin-play).
- **Secrets:** the DeepSeek key stays in `.env` files only — never in the repo, the task, or the reply.
- **Cost:** Newton free tier is the budget. Target one Newton build session; Gemini Flash first per protocol 1.2.0. Report the cost block in the staged reply.
- One deliverable: delete-a-piece merged to cabin-play `main` with the Copilot review on its PR. Juno redeploys to the VPS afterward.

## Proof
- PR URL, Copilot review pointer (PR review or comment link), main SHA on origin after merge.
- Test output: full pytest run + the extended live smoke, covering criteria 1–5.
- Juno verifies by reading the repo and the PR on origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, branch, PR link, main SHA, proof list, cost block, and any gate failure stated plainly at the top.
