---
task_id: "0223"
status: inbox
iteration: 0
proof: []
notes: ""
---

# 0223 — Writing app: Develop mode (story-development conversation, story bible, merge) + Stuck? helper

## Assignment
- Hermes: route the build to Newton via Herdr (software goes to Newton first, protocol 1.3.0). You own integration, the backup, deployment to writing.nordtronics.io, and the live end-to-end proof.
- Copilot: peer review before deploy if the app code goes through GitHub. If it does not, say so in the reply — the step is skipped, never faked.
- Juno: verifies the staged reply against the live-site behavior and proof reported.

expect-reply-within: 48h

## Context
Bethany's feedback on her writing app (writing.nordtronics.io), relayed by Stephen 2026-11-19. She wants a conversational back-and-forth for developing a story and storyline — separate from the main writing portion — where she can argue back when she disagrees with a direction, where what gets developed can be merged into the work (Stephen's analogy: like merging code), and where the partner remembers the whole book or article she is working on. Full feedback note: Juno holds it at `workspace/goals/bethany-ai-peer-review-writing-app/files/dev-chat-feedback-2026-11-19.md` in her own workspace; the asks are restated in full below, so the task stands on its own.

Current app state, as Juno knows it: live at writing.nordtronics.io, Warm theme default (warmth is an explicit requirement, not polish), pieces with review, voice check, and delete-a-piece live. One open question this task must answer first: whether the review provider is a real configured DeepSeek key or still the stub Bethany saw on Nov 15 — a conversation cannot be built on a stub.

**Amendment, 2026-11-19 (Stephen's direction):** the "Stuck?" helper is merged into this task — build both in one pass, Develop mode first, then the helper, one deploy at the end. The helper was Stephen's idea on 2026-11-17 while Bethany was using the app: an in-editor button that looks at what she has already written and suggests a direction to take next, gated at about three uses an hour so it is not overused. Juno draft-spec'd it 2026-11-18 and it waited on Stephen's word; he gave it 2026-11-19 ("why are we waiting, let's do it").

## Task
Build Develop mode v1 for the writing app:

1. **Provider gate first.** Confirm the app's LLM provider is a real configured key, not the stub. If it is still the stub, stop and report that before building anything on top of it.
2. **Develop room.** Add a per-project Develop view alongside the existing writing view. A project can hold multiple conversation threads (each thread is a separate line of development she can return to).
3. **Whole-work memory.** Every Develop conversation is backed by (a) the project's full manuscript text and (b) a per-project **story bible**: characters, timeline, world facts, and decisions already made. The bible is visible to Bethany in plain language. Seed it from the manuscript where that is straightforward; an empty bible that fills as decisions are made is acceptable for v1.
4. **Pushback stance.** The partner's system prompt makes it a development editor, not a yes-machine: it defends a suggestion with craft reasons, asks why when Bethany disagrees, offers alternatives, and concedes when her argument is better. Settled points are treated as settled in later conversation.
5. **Decision capture.** A thread gets a plain-language action (e.g. "Save to story bible") that proposes the agreed facts/decisions from that thread as bible entries; Bethany sees them and confirms before anything is saved.
6. **Merge.** A thread gets an "Add to my book" action: a preview screen showing exactly what will change — bible entries to add, and/or a drafted passage to insert into the manuscript at a place Bethany picks — and nothing is applied until she confirms. Unmerged threads never touch the manuscript.
7. **Deploy** to writing.nordtronics.io and bump the PWA cache version so Bethany's existing install picks the change up on its next loads. Back up the app's data before any schema change.

Then, in the same pass:

8. **Stuck? helper.** Add a "Stuck?" button in the writing editor. On tap, it reads the piece in progress — what she has written and where it trails off — and returns **two or three short directions, not prose**: a possible next beat, a question the scene could answer, a thread from earlier in the piece to pick back up. Each direction is one short sentence or question. No ghostwritten sentences, paragraphs, or continuations, ever. Warm, peer-like tone, like a friend leaning over — not a teacher.
9. **Usage gate.** 3 uses per rolling 60-minute window, per piece, enforced server-side and persisted across service restarts. A 4th tap inside the window shows a gentle message only and makes no provider call — proposed copy: "that's your three for this hour, write a little and it comes back". The button works again as the window rolls.

## Success criteria
- Provider confirmed real (not stub) in the reply, or the task stopped at step 1 with that reported.
- On the live site, a test project has a Develop room and a conversation runs end-to-end against the real provider.
- The partner demonstrably uses manuscript context: a test question about a character or event that exists only in the test manuscript gets a correct answer.
- A decision saved from a thread appears in the bible, and a fresh thread afterward does not contradict it.
- Merge shows a preview before applying; after confirmation the bible/manuscript reflect it; content from an unmerged thread does not appear in the manuscript.
- Pushback is visible in a test transcript excerpt: when the tester disagrees with a suggestion, the partner answers with reasoning or an alternative, not instant capitulation.
- Bethany's existing pieces, reviews, and the Warm default are untouched.
- "Stuck?" button is visible in the editor on a piece with existing text; one tap returns exactly 2–3 directions, each a single short sentence or question, with no prose continuation, from the live provider.
- A 4th tap on the same piece within 60 minutes is refused with the gentle capped message and no provider call; the cap count survives a service restart; after the window rolls, the button works again.

## Constraints
- Model: Newton's normal tier for the build; your normal DeepSeek flash tier for routing and integration. No new paid services.
- Use the app's existing provider and key. The key never appears in the reply, a commit, or any file in the repo.
- No developer jargon in the UI: no "branch", "merge", "commit", or "diff" as labels — plain words only ("Add to my book", "Save to story bible"). Internal design may use those concepts; Bethany's screen may not.
- UI copy stays in the app's Warm voice.
- One deliverable: the writing app's next release containing Develop mode v1 and the Stuck? helper, built in that order. If the helper hits trouble, do not let it block Develop mode — deploy Develop on its own and report the helper's state separately.
- Do not break the existing review flow or lose any existing data.

## Proof
The live URL plus the test evidence in the reply body: provider status, the manuscript-context test exchange, the bible before/after a decision save, the merge preview and its result, the pushback transcript excerpt, and the Stuck? evidence (a live directions reply plus the 4th-tap cap refusal). Branch/commit pointers if the app code is in git, and the Copilot review outcome if it ran.

## Reply format
Status line first. Then: provider status, what was built, the end-to-end test evidence in the order of the success criteria, deploy and cache-version details. Deviations, if any.
