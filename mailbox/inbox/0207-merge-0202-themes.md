# 0207 — Merge 0202 themes (PR #6) and redeploy the writing app

## Assignment
- **Hermes:** the merge and the deploy. cabin-play writes go through your established SSH route from the laptop, as before.
- **Juno:** verifies the merge and the live deploy independently, then closes out 0202.

## Context
0202 (writing-app themes) is staged on branch `hermes/0202-themes` at `4c4121ff7eba53a55c53af860adaa2923d69f5e6`, PR #6 open on pagosacabin/cabin-play, Copilot SHIP. The spec gated the merge on Stephen's approval of the staged screenshots. **Stephen approved at the gate on 2026-11-17 ~09:12 MST — his words: "Looks warm — merge it."** This task executes that approval. Nothing about the branch changes.

## Task
1. Merge PR #6 into cabin-play `main` as-is at head `4c4121ff`. No edits, no extra commits on the branch. If the merge conflicts, STOP and report — do not resolve by editing code.
2. Redeploy the writing app on the VPS from the new main, the same way 0200 was deployed: update the app source, restart `writing-review`, run the test suite.
3. Smoke check on the live service: writing.nordtronics.io responds, the Settings page carries the theme picker with Warm as default, and the new CACHE_NAME is being served.

## Success criteria
- PR #6 shows merged; cabin-play main's new head is the merge of `4c4121ff` (state the merge SHA in the reply).
- VPS is running the new main: `writing-review` active, pytest passes (18 expected).
- Live smoke check passes (theme picker present, Warm default).

## Constraints
- Merge and deploy ONLY — no code changes in this task. Anything you notice goes in the reply as a note, not a fix.
- Cost: cronrunner profile's usual model.

## Proof
PR #6 merged state + merge SHA on origin; the VPS deploy evidence in the reply (service status, pytest result, smoke result) in reproducible form.

## Reply format
Staged reply in mailbox/staged/ per protocol 1.3.0. expect-reply-within: 6h.
