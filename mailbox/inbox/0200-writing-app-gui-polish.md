# 0200 — Writing app: full GUI polish pass — make it look really good

## Assignment
- **Hermes:** mailbox owner. Route the build to Newton via Herdr, supervise, open the PR, drive the Copilot review, stage the reply **with screenshots**. Do NOT merge until Juno relays Stephen's look approval (see the screenshot gate below).
- **Newton:** build the polish (software → Newton first, protocol 1.2.0).
- **Copilot:** peer review the PR, driven through Herdr via **`hc` (herdr-core)** — same route as 0192/0194/0195. Copilot reviews the code; the *look* is Stephen's call, not Copilot's.
- **Juno:** verifies from origin, shows Stephen the staged screenshots, relays his approval, archives on green after merge, and redeploys to the VPS (the deploy is Juno's, not this task's).

## Context
Stephen's call, 2026-11-16: rather than patch the one layout bug 0194 introduced (the History card crowding — title, date, word count, and badge all wrapping mid-phrase at phone width since the Delete button landed), do **one overall GUI pass** on Bethany's writing app and make it look really good, in one sequence. The history-card fix is part of this pass, not a separate task.

Current state: the app (cabin-play `writing-peer-review/`, live at writing.nordtronics.io, main @ 2b40e34) is functional and plain — serviceable CSS, no coherent visual system. Views: Deal Exercise, Write (editor), History (piece cards), Portfolio (export), Settings, plus the review display (five sections incl. Voice check) and the per-review discussion panel. All behavior is correct and must stay byte-for-byte equivalent in function; only presentation changes.

**Design direction:** set by Stephen at filing — see the line Juno fills in below before this task is picked up. If the line says "propose," Newton chooses a coherent direction fitting a writing app and the screenshots are where it is judged.
DIRECTION: **Propose, leaning warm / editorial — a writer's feel** (Stephen's steer, 2026-11-16 12:19: "you had it kind of right doing a kind of a warm or writer's feel to it"). Newton proposes within that lean; the look is judged at the screenshot gate, not pre-specified pixel by pixel.

## Task
A visual polish of the whole app — CSS and template structure only:

1. **Coherent visual system.** One spacing scale, consistent card treatment (padding, radius, border/shadow), consistent button styles (primary/subtle/destructive), consistent badges and tabs, applied across every view so the app reads as one designed thing.
2. **Typography.** Headings and piece titles in a readable display serif; body text with comfortable size and line-height; the Write editor gets prose-friendly typography and a comfortable measure (line length) — this is a writing app, the text is the product.
3. **History cards (the original bug, folded in).** At phone width (~360–400 px) nothing wraps mid-phrase: title full width on its own line, meta line (`Updated <date> · <N> words`) on one line, badge never wraps internally (`white-space: nowrap`), Delete in an actions row with the badge. Desktop keeps a sensible layout.
4. **Review display.** The five sections (What worked / Craft points / A question for you / Follow-up exercise / Voice check) are easy to scan: clear section headings, comfortable paragraph spacing, quoted passages visually distinct (indented/blockquoted), and the Voice check band line (`Reads human.` / `Mixed.` / `Reads machine-made.`) visually weighted as a verdict line, not buried in body text.
5. **Every view gets the pass:** Deal Exercise, Write, History, Portfolio, Settings, discussion panel, the peak/off-peak pill — no view left in the old styling.
6. **Responsive throughout:** every view checked at phone width and desktop width.
7. Bump `CACHE_NAME` in `static/sw.js` (v3 → v4) so installed copies pick the new look up.

**Out of scope:** any behavior or feature change; any endpoint, prompt (`review_prompt.md`, `discussion_prompt.md`), schema, or export-format change; dark mode (unless the direction line asks for it); new icon/logo work beyond restyling what exists.

## Screenshot gate (this task's one deviation from the usual flow)
"Pretty" is a human judgment. After Copilot's SHIP, Hermes stages the reply **with screenshots** (phone ~380 px + desktop ~1280 px for: History, a review with Voice check, Write, Deal Exercise, Portfolio) and leaves the PR **unmerged**. Juno shows them to Stephen; on his approval Hermes merges (Juno relays the go in a follow-up or Stephen prods via Herdr); on his rejection, his notes go back for one revision round on the same branch. Copilot's review still happens before the gate — code first, looks second.

## Success criteria (falsifiable)
1. Screenshots staged for the five views at both widths; at phone width no text wraps mid-phrase anywhere (the 0194 history-card defect is gone as part of the whole).
2. Function is unchanged: full pytest suite and live smoke pass at the branch tip (counts as they stand at pickup), including the delete flow and the five-section review rendering.
3. The PR exists on cabin-play, Copilot's review is posted, findings addressed in code or answered in the reply.
4. After Stephen's look approval: merged to `main`; the reply (updated or supplemented) cites the **main** SHA.

## Constraints
- **PR creation:** use `gh` with the **keyring OAuth login** (pagosacabin, `repo` scope). Do NOT source `gh-env.sh`; do not export its PAT. State which auth `gh` used.
- **Copilot routing:** through Herdr via **`hc` (herdr-core 0.1.21)** — never `herdr` (renamed to `herdr-legacy-0.9.5` in 0196; command-not-found now). If the PR create or the Copilot review fails, report the exact error and stop at that gate.
- Branch `hermes/0200-gui-polish`, based on cabin-play `main` as it stands at pickup. Git pushes over SSH as usual.
- **Cost:** Newton free tier is the budget — target one build session (one revision round is pre-authorized by the gate if Stephen asks for changes). Note at filing: the Gemini pool sat at ~14% remaining after 0194, below protocol 1.2.0's 20% switch point — route per protocol (pool two) and say which pool was used.
- **Secrets:** the DeepSeek key stays in `.env` files only.
- One deliverable: the polish merged to cabin-play `main` (after the screenshot gate) with the Copilot review on its PR. Juno redeploys afterward.

## Proof
- PR URL, Copilot review pointer, main SHA on origin after merge.
- The staged screenshots at both widths for the five views.
- Test output: full pytest run + live smoke at the branch tip.
- Juno verifies by reading the repo and the PR on origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, branch, PR link, proof list, cost block, screenshots, and any gate failure stated plainly at the top. State clearly that the PR is **awaiting Stephen's look approval** (the screenshot gate) rather than merged.
