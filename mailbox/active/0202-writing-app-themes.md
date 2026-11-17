---
task_id: "0202"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0202 — Writing app: switchable themes — three looks plus a light customize

## Assignment
- **Hermes:** mailbox owner. Route the build to Newton via Herdr, supervise, open the PR, drive the Copilot review, stage the reply **with screenshots**. Do NOT merge until Juno relays Stephen's look approval (screenshot gate, same as 0200).
- **Newton:** build the feature (software → Newton first, protocol 1.2.0).
- **Copilot:** peer review the PR, driven through Herdr via **`hc` (herdr-core)** — same route as 0192/0194/0195/0200.
- **Juno:** verifies from origin, shows Stephen the staged screenshots, relays his approval, archives on green after merge, and redeploys to the VPS (the deploy is Juno's, not this task's).

## Context
Stephen's idea, 2026-11-16: instead of one fixed look, give the writing app **three main theme options plus a customize**. His sequencing call the same day: **warm design first (0200), themes as the next task** — because 0200's polish pass rebuilds the styling on named variables (colors, fonts, spacing as tokens), and themes are then "define more token sets plus a picker" instead of three separate designs.

**Filing dependency (Juno enforces):** this task is filed only *after* 0200 has merged to cabin-play `main`. Branch from main as it stands at pickup, which must include 0200's token-based styling. If the styling at pickup is still the pre-0200 CSS, stop and report — the base isn't ready.

Current state at drafting: app live at writing.nordtronics.io; 0194 (delete) and 0195 (voice check) merged; 0200 (GUI polish, warm/editorial) queued ahead of this task with its own screenshot gate.

**Stephen's verdict on 0200, 2026-11-16 — this is now part of THIS task.** He approved 0200's merge (structure and layout are right), but his words on the feel: *"it didn't really change the feel of the application, it just changed the layout slightly. I was looking for something like colors and warmth, and I didn't see any of that."* The 0200 palette (near-white warm ground, muted ink-green) reads as plain white on a phone — too restrained. His decision: merge 0200 as-is, and **the warmth lands here, in the themes task**. So the Warm theme below is not "0200 unchanged" — it is the warmed-up design he actually asked for, and it is judged at this task's screenshot gate by his standard: **colors and warmth you can see at a glance**, not on a spec sheet.

## Task
Add user-selectable themes to the writing app:

1. **Three built-in themes**, all defined as token sets over 0200's CSS variables:
   - **Warm** — the default, and the one that carries Stephen's brief: built on 0200's tokens but visibly warmed up — a ground that reads as cream/paper on a phone screen (not near-white), a stronger warm accent presence, colored surfaces where the 0200 design stayed neutral. If a screenshot of Warm could be mistaken for the 0200 render at a glance, it is not warm enough.
   - **Clean** — a light, minimal look (cooler neutrals, crisper contrast).
   - **Night** — a dark theme suited to evening writing (dark background, soft text contrast, no pure-black glare).
2. **Picker.** A theme control in the Settings view (and only there — no chrome added to other views) listing the three themes; switching applies immediately, no reload.
3. **Persistence.** The choice survives reloads and app restarts on that device. localStorage is the expected mechanism — no database schema change, no new endpoint, unless you report a concrete reason it can't work.
4. **Light customize.** Two user adjustments on top of any theme: an **accent color** chosen from a small curated set (4–6 swatches that work with all three themes), and a **text size** step (Small / Medium / Large) affecting reading/writing text. Both persist with the theme choice. Nothing more — no free-form color pickers, no font uploads.
5. **Every view, every theme.** Deal Exercise, Write, History, Portfolio, Settings, review display (incl. the Voice check band line), discussion panel, peak/off-peak pill — legible and coherent in all three themes at phone and desktop widths. The Voice check band stays readable as a verdict line in Night.
6. Bump `CACHE_NAME` in `static/sw.js` again (v4 → v5, or whatever the current version is at pickup +1).

**Out of scope:** behavior/feature changes beyond the picker and customize; per-piece or per-view themes (one app-wide choice); syncing the choice across devices; dark-mode auto-switching from the OS (manual picker only in this rung).

## Screenshot gate
After Copilot's SHIP, Hermes stages screenshots — for **each of the three themes**: History and a review with Voice check at phone width (~380 px), plus History at desktop width (~1280 px) — and leaves the PR **unmerged**. Juno shows them to Stephen; on his approval Hermes merges; on rejection, one revision round on the same branch.

## Success criteria (falsifiable)
1. Switching themes in Settings re-skins the whole app immediately; the choice and the customize settings persist across a full reload and a fresh browser session (prove in the browser proof, not just by reading localStorage).
2. Screenshots staged per the gate: all three themes legible in every shot, no mid-phrase wrapping at phone width, no element invisible against its background (check badges, the band line, buttons, and the pill in Night especially).
3. Customize works: changing accent visibly changes accent-colored elements in all three themes; text size steps visibly change body text; both persist.
4. Regression: full pytest suite and live smoke pass at the branch tip (counts as they stand at pickup); delete flow, review flow, exports untouched in behavior.
5. The PR exists on cabin-play, Copilot's review is posted, findings addressed in code or answered in the reply; after Stephen's approval, merged to `main` and the reply cites the **main** SHA.

## Constraints
- **PR creation:** use `gh` with the **keyring OAuth login** (pagosacabin, `repo` scope). Do NOT source `gh-env.sh`; do not export its PAT. State which auth `gh` used.
- **Copilot routing:** through Herdr via **`hc` (herdr-core 0.1.21)** — never `herdr` (renamed `herdr-legacy-0.9.5` in 0196; command-not-found). If the PR create or the Copilot review fails, report the exact error and stop at that gate.
- Branch `hermes/0202-themes`, based on cabin-play `main` as it stands at pickup (must include 0200). Git pushes over SSH as usual.
- **Cost:** Newton free tier is the budget — target one build session (one revision round pre-authorized by the gate). Route pools per protocol 1.2.0 and say which pool was used (Gemini sat at ~14% remaining on 2026-11-16, below the 20% switch point).
- **Secrets:** the DeepSeek key stays in `.env` files only.
- One deliverable: themes merged to cabin-play `main` (after the screenshot gate) with the Copilot review on its PR. Juno redeploys afterward.

## Proof
- PR URL, Copilot review pointer, main SHA on origin after merge.
- The staged screenshots (three themes × the gate's shot list).
- Test output: full pytest run + live smoke at the branch tip, plus the browser persistence proof.
- Juno verifies by reading the repo and the PR on origin herself; pasted output alone is a claim, not proof.

## Reply format
Standard staged reply: Status line, branch, PR link, proof list, cost block, screenshots, and any gate failure stated plainly at the top. State clearly that the PR is **awaiting Stephen's look approval** (the screenshot gate) rather than merged.
