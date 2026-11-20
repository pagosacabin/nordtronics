---
task_id: "0224"
status: in_progress
iteration: 1
proof: []
notes: ""
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-20T04:00:30+00:00"
lease_until: "2026-11-20T05:30:30+00:00"
---

# 0224 — Writing app: show a version number, and make the test piece a permanent "Example Story"

## Assignment
- Hermes: you own this one — build it yourself or route per protocol 1.3.0 as the pools allow (Newton's Claude/GPT pool hit its weekly cap during 0223; do not wait on it if another route serves). You own deployment to writing.nordtronics.io and the live proof.
- Copilot: peer review only if pane t17 is signed in by the time you run; if it still answers /login, declare the deviation exactly as 0223 did — never fabricate a review.
- Juno: verifies the staged reply (version displayed, delete-protection, existing data untouched).

expect-reply-within: 48h

## Context
Stephen's feedback, 2026-11-19, from a screenshot of Bethany's phone showing the app's History view after the 0223 deploy. Two things:

1. **No version number anywhere.** His words: "we should have a version number on the app so we can refer to the version number that is displayed for each one of us so we know we're referring to the right version." When Bethany, Stephen, and Juno talk about the app, there is currently no shared reference for which build anyone is looking at.
2. **The "0223 E2E" piece** (the end-to-end test project Hermes left live for verification) sits at the top of Bethany's archive with a task-number title. Stephen's call: the concept of a built-in example is good — "have it there to show off what you can and can't do" — but it should be called **"Example Story"**, in plain words for Bethany, and it should **not be deletable**, so it is always there as the safe place to look at what the features do.

Note on the existing pieces, Stephen 2026-11-19 evening: Bethany's three pieces are short play pieces (about 30 words each) and are disposable — she is just playing with the app so far. Preserve them in the normal course, but they are not precious data: do not let protecting them complicate this build, and do not treat them as the archive that matters. Her real writing comes later.

## Task
1. **Display a version number in the app.** One version string, defined in a single place in the code, shown somewhere Bethany can always reach without hunting (the Settings view is the natural home; a small footer line is also acceptable — pick one and keep it there). Bump it with each deploy alongside the service-worker cache version. State the new displayed version in your reply so Stephen and Juno can quote it back.
2. **Turn the "0223 E2E" piece into the built-in Example Story.**
   - Rename it to exactly **"Example Story"**.
   - Make it permanent: the Delete action is not offered for it in the UI, and the delete endpoint refuses it server-side too (not UI-only). Every other piece stays deletable as today.
   - Make it a real showcase: it should contain a short piece of writing with named characters, at least one review, a story bible with character/decision entries, and a Develop thread — so Bethany can open it and see what each feature does without experimenting on her own work. Reuse and tidy the existing E2E material (Marta/Elias) rather than inventing a new cast, unless its content is confusing as an example; plain words throughout.
   - It must survive restarts and deploys as a fixture of the app, not a one-off database edit that the next seed/migration could lose. Implement it as a protected, seeded piece (flag or reserved identity in the data model — your call on mechanism, stated in the reply).
3. **Deploy** to writing.nordtronics.io with the usual backup first and the cache-version bump, so Bethany's install picks it up.

## Success criteria
- The app displays a version number in the UI; a screenshot or UI check shows it, and it matches the version stated in the reply.
- "Example Story" appears in the archive under exactly that title; its Delete is absent in the UI and a delete attempt against its endpoint is refused.
- The Example Story contains writing, at least one review, bible entries, and a Develop thread.
- Bethany's three real pieces ("First piece", "The unexpected guard", "Second piece") and their reviews are untouched, and still deletable.
- The existing pytest suite passes, plus new coverage for the delete-protection.

## Constraints
- Model: your normal DeepSeek flash tier for your own work; routing per the Assignment note.
- No developer jargon in any user-visible copy; Warm remains the default theme.
- Additive and protective only: do not alter Bethany's existing pieces, reviews, or bible data.
- The Example Story's protection is a product feature, not a hack: future deploys must not silently make it deletable again.

## Proof
Branch and commit SHA in pagosacabin/cabin-play, test counts (local and VPS), the displayed version string, live evidence for the delete refusal (UI state plus endpoint response), and confirmation that the three real pieces are unchanged. Screenshots under mailbox/files/0224/ as in 0223.

## Reply format
Status line first. Then: the version (where it is shown, what it now reads), the Example Story mechanism and contents, delete-refusal evidence, test counts, deploy details. Deviations, if any.
