---
task_id: "0070"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0070-site-rewrite
    sha: 235e2755b46443442bbe40542dd6b5ef883bafcc
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36297011367
  - files:
      - website/index.html
      - .github/workflows/website-check.yml
  - merge: 235e2755b46443442bbe40542dd6b5ef883bafcc (main fast-forwarded to the branch tip)
notes: |
  Delivered: blocks A-K of the approved copy rewritten in website/index.html and
  merged to main (fast-forward). Model: deepseek-flash (provider deepseek), the
  tier the spec named.

  Pointer verification, re-read from the live GitHub API while writing this:
  branch tip 235e2755b46443442bbe40542dd6b5ef883bafcc via git ls-remote; CI run
  36297011367 headSha == that tip, conclusion success, every step green (Validate
  HTML, Check links, Verify assets exist, Quick serve test) checked via gh run
  view --json jobs, not inferred from the run verdict. The merge to main was a
  fast-forward, so main == 235e2755; the push to main fired run 36297056731 on
  the same SHA, also success. grep -ri insurer website/ -> no output (exit 1,
  zero hits), quoted in the reply.

  Proof that the untouched parts really are untouched: the architecture <svg>
  block is byte-identical to main (sha256 72e25746ee6d18ea) and its RENDER is
  pixel-identical to main's (0 of 529,920 pixels differ once a 1px subpixel
  layout offset is corrected); #sensors, #power (the 18650 battery card),
  #pilot, the footer and #app's content are all byte-identical to 232564c
  (#app moved, content unchanged). Recording that because a vision pass flagged
  label overlaps inside the diagram - they are present on main too.

  Scope extension (one line, required by a success criterion): the branch was
  added to .github/workflows/website-check.yml on.push.branches. Without it a
  push to hermes/0070-site-rewrite starts no run, and the criterion asks for the
  website workflow to pass on the branch. Same pattern 0068 used.

  Deviations declared in the reply body: the contact section's bare
  hello@nordtronics.io pill was dropped (the spec enumerates three path cards
  plus "keep the GitHub link"; the address survives inside both mailto hrefs);
  the contact path grid minimum was reduced so the three cards sit on one row
  inside the 600px contact column; section titles are plain text rather than
  wrapped in the site's accent <span> so the approved wording greps verbatim.
  The task file arrived with no YAML front matter - task_id/status/iteration
  were added on pickup.

  Verification method: htmlhint 1.1.4 against the repo's .htmlhintrc and the
  workflow's exact linkchecker invocation, run locally on the branch file, both
  clean; served page rendered in headless Chromium at 1440px plus a 390px mobile
  render, with a delegated vision pass over the section screenshots
  (deepseek-flash) and DOM geometry measurements. Nothing in the new blocks
  overlaps, clips or is unreadable.

  No ntfy receipt: this is not a build task (no CI artifact is produced), so the
  build-green notification rule does not apply; the workflow URLs above are the
  CI pointers.

  Out-of-scope observations, unchanged and reported rather than fixed: (a) the
  privacy strip wraps to two rows inside its 900px max width - cosmetic, no text
  lost; (b) pre-existing, visible in the committed website/screenshots/desktop.png
  too - the global `header` rule gives every section's .section-header
  min-height:100vh, so each section opens with a ~900px empty band; (c) the
  architecture SVG's own labels overlap at 1440px on main as well.

  No blocker. No unverified claim in the proof block.
---


# 0070 — Site copy rewrite: commercial positioning

## Context

Stephen reviewed the live site and approved a full copy rewrite (draft
"site-rewrite-draft.md", approved 2026-09-26 ~22:40 MDT). Tonight Juno already
removed the false "insurer-supported pilot" claims (commit 800ca724) — grep
must confirm zero remaining "insurer" hits. The bio and battery card have
owner-open items (see constraints). Model: deepseek-flash. Report the model
used in your reply.

## Task

On branch `hermes/0070-site-rewrite`, cut from the tip of `origin/main`,
rewrite `website/index.html` copy per the approved blocks below. Implement
them verbatim unless a constraint says otherwise.

**A. Hero.** Subtitle becomes two lines: "Built for places the grid doesn't
reach." then "Off-grid power, energy storage, and intelligent sensing —
designed and built by one engineer, supported directly." Replace the single
"Request Pilot Pricing" button with two: primary "Explore Wildfire Detect"
→ `#wildfire`, outline "Discuss a Project" → `#contact`. Keep logo, eyebrow,
NORDTRONICS title.

**B. New featured section** `id="wildfire"`, first section after hero, before
`#architecture`. Label: "Featured Platform". Title: "Wildfire Detect". Lede:
"Know about a fire before you can see it." Body: "A network of solar-powered
sensor nodes watches your property line around the clock. When the air starts
changing — smoke particles rising, temperature climbing — the nodes compare
notes with each other and send an early warning to your phone. Not a confirmed
fire. An early warning, while there's still time to act." Then four steps:
1. **Detect** — Distributed nodes monitor PM2.5, temperature, and conditions
continuously. 2. **Relay** — LoRa sends events to the base station on your
property. No cellular, no subscription. 3. **Verify** — The system correlates
triggers across multiple nodes and looks for real change, not noise.
4. **Alert** — You get an early warning on your phone. Then the privacy strip:
"Your property. Your data. No cameras. No microphones. No GPS tracking. No
required cloud connection." Then small prototype honesty line: "Wildfire
Detect is a working prototype, not a finished product. Specs will change as
field testing teaches us things."

**C. Architecture section.** Directly above the diagram, below the section
title, add: "A network of autonomous sensors watches the environment
continuously and sends only meaningful changes back to the property gateway."
Diagram itself untouched.

**D. Detection card.** In the "Detection Radius & Logic" card, after the
consensus sentence, add: "The system reports environmental signatures
consistent with an emerging fire — an early warning, not a confirmed fire."

**E. Services → "What Nordtronics Builds".** Label: "What Nordtronics
Builds". Title: "Power, Sensing, and Custom Engineering". Three cards:
**Remote Power Systems** — "Solar + storage + battery systems for cabins,
remote sites, and backup power. Site assessment, design, sourcing,
commissioning." **Remote Sensing** — "LoRa + ESP32 environmental monitoring.
Sensors that run for years on sunshine and report back over miles, not feet."
**Custom Engineering** — "Hardware, firmware, telemetry, automation. If it
needs to work where nothing else does, that's the job." Under the grid add a
featured-platform callout: "**Wildfire Detect** is the proof of what
Nordtronics can engineer — not just another service. [Explore the platform
→](#wildfire)"

**F. New "Why Nordtronics" section**, after services, before pilot. Title:
"Why Nordtronics?" Body: "Built for places the grid doesn't reach.
Nordtronics designs power and sensing systems for remote environments — where
conventional infrastructure is expensive, unreliable, or just not there. Solar
becomes the power plant. Batteries become the grid. LoRa becomes the network.
One engineer designs it, builds it, and answers your email when you have
questions."

**G. Pilot section.** Keep as-is (heading already "Pilot Program", paragraph
already "looking for pilot partners").

**H. About bio.** Replace the background paragraph with: "Stephen Nordlund
is a systems engineer turned off-grid power and embedded-systems builder.
Background: systems engineering, Unix and VMware infrastructure, field service
— now solar installs, custom LiFePO4 batteries, and embedded electronics
(ESP32, LoRa, Home Assistant) from a shop in Pagosa Springs, Colorado."
Replace the one-person-shop paragraph with: "Nordtronics is a one-person
shop, and that's the point. Direct engineering. Direct support. The person
who designs your system is the person who answers your email — six months
later, too."

**I. Contact.** Title: "Have a remote-system problem?" Intro: "Talk directly
to the engineer building the solution. No sales pitch." Three path cards:
1. **Wildfire Detect** — Request a pilot → `#pilot` 2. **Off-Grid Power** —
Discuss a system → `mailto:hello@nordtronics.io?subject=Off-grid%20power%20inquiry`
3. **Custom Electronics** — Discuss your project →
`mailto:hello@nordtronics.io?subject=Custom%20electronics%20inquiry`. Keep the
GitHub link.

**J. Meta.** `<title>`: "Nordtronics — Engineering for places beyond the
grid". Meta description: "Nordtronics designs off-grid power and sensing
systems for remote environments. Featured platform: Wildfire Detect, an
autonomous early-warning sensor network. Pagosa Springs, Colorado."

**K. Section order** (move `#app` up): hero → #wildfire → #architecture →
#detection → #app → #services → why-nordtronics → #pilot → #about → #contact.
Privacy strip ("Your property. Your data.") should be a visual element, not
buried in a card. Detect → Relay → Verify → Alert steps get a simple visual
treatment reusing the site's existing diagram style.

## Success criteria

- All blocks A–K implemented; copy matches the approved wording above.
- `grep -ri insurer website/` returns zero hits.
- CI website workflow passes on the branch.
- Page renders correctly (no broken anchors: `#wildfire`, `#contact`,
  `#pilot` all resolve; mailto links well-formed).
- After CI is green, merge the branch into `main` (fast-forward) and push.

## Constraints

- Battery/power card: DO NOT CHANGE. Stephen has an open decision on the
  cell configuration; leave the 18650 card exactly as-is.
- Bio: use the exact wording in block H. Do not embellish or extend it.
- Sensor spec cards, power cards (other than untouched), architecture SVG,
  app screenshots, footer: unchanged.
- No new claims: nothing about pilots running, no field results, no pricing.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- Branch `hermes/0070-site-rewrite` pushed to origin; tip SHA.
- Actions run URL for the website workflow on that SHA, status successful.
- `grep -ri insurer website/` output (must be empty).
- Merge commit SHA on `main`.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: done`, the proof pointers above, and a notes field describing
anything you could not implement verbatim and why.

---

# Reply — Hermes → Juno

**Status: staged.** Branch `hermes/0070-site-rewrite` @
`235e2755b46443442bbe40542dd6b5ef883bafcc`; CI run
<https://github.com/pagosacabin/nordtronics/actions/runs/36297011367> —
`conclusion: success`, `headSha: 235e2755b46443442bbe40542dd6b5ef883bafcc`, all
steps green. Merged to `main` by fast-forward, so `main` == that SHA, and the
push to main fired run
<https://github.com/pagosacabin/nordtronics/actions/runs/36297056731> on the same
SHA — also `success`. **Model/tier: `deepseek-flash`** (provider `deepseek`).
Two files changed vs `232564c`: `website/index.html` (155 insertions, 47
deletions) and `.github/workflows/website-check.yml` (1 line).

## Block-by-block

| Block | Where | State |
|---|---|---|
| A Hero | `header` | Two-line subtitle exactly as approved; "Request Pilot Pricing" replaced by **Explore Wildfire Detect** (primary → `#wildfire`) and **Discuss a Project** (outline → `#contact`). Logo, eyebrow, NORDTRONICS wordmark untouched. |
| B Featured section | `#wildfire`, first section after the hero | Label *Featured Platform*, title *Wildfire Detect*, lede *Know about a fire before you can see it.*, the body paragraph verbatim, the four steps, the privacy strip, the prototype honesty line. |
| C Architecture lede | `#architecture`, below the section title, above the diagram | Added verbatim. **Diagram untouched** (see verification). |
| D Detection card | `#detection` → *Detection Radius & Logic* | Sentence appended after the consensus sentence. |
| E Services | `#services` | Label *What Nordtronics Builds*, title *Power, Sensing, and Custom Engineering*, the three renamed/reworded cards, and the callout under the grid with *Explore the platform →*. |
| F Why Nordtronics | new `#why-nordtronics`, after services, before pilot | Title + body verbatim. |
| G Pilot | `#pilot` | Unchanged (heading *Pilot Program*, "looking for pilot partners" paragraph). The "insurance carrier" mention in that paragraph also stays — it is the pilot section's own text, and the literal `insurer` grep is clean. |
| H About bio | `#about` | Both named paragraphs replaced with the exact wording. |
| I Contact | `#contact` | Title *Have a remote-system problem?*, intro *Talk directly to the engineer building the solution. No sales pitch.*, three path cards with the specified targets, GitHub link kept. |
| J Meta | `<head>` | `<title>` and meta description exactly as approved. |
| K Order + visuals | whole page | Order as requested; privacy strip is a standalone visual band, the four steps are cards with dashed-circle numerals in the diagram's own style. |

## Block K — how the two sections the ordering list omits were placed

The requested order names ten sections but the file has twelve: `#sensors` and
`#power` are not in the list, while the Constraints say the sensor spec cards and
the power cards stay unchanged. So I kept those two in their existing relative
position and satisfied every adjacency the list *does* state. Final DOM order,
read out of the served page:

```
wildfire → architecture → sensors → power → detection → app → services →
why-nordtronics → pilot → about → contact
```

Say the word if you want `#sensors`/`#power` moved or dropped and I will refile.

## Verification actually performed

1. **CI, on the branch tip and on main** — pointers above, each re-read from the
   API (`gh run view --json headSha,conclusion,jobs`) while this was written. The
   branch run is at the tip, not an earlier green run: the branch gained a second
   commit (contact-card layout) after the first green run, and the proof points
   at the tip run, `36297011367`.
2. **The workflow's own checks, run locally first** — `htmlhint@1.1.4` against the
   repo's `.htmlhintrc` → *"Scanned 1 files, no errors found"*; the workflow's
   exact `linkchecker` invocation (`--check-extern`, mailto and github ignored)
   → *"7 links in 7 URLs checked. 0 warnings found. 0 errors found."* So the green
   run is not the first time the file met those rules.
3. **Rendered, not just parsed** — the branch file served over a local HTTP server
   and rendered in headless Chromium: full page at 1440px and at 390px, plus
   element shots of every new or changed block. A vision pass over those shots
   (delegated, `deepseek-flash`) confirmed the hero (two buttons, two-line
   subtitle), the featured section (label, heading, lede, body, four numbered
   steps, green privacy strip, note), services (three cards + callout + link),
   the why section, and the contact section — no overlapping or clipped text, no
   unstyled block, no missing image. Text presence itself was checked
   deterministically against the file, not by eye.
4. **Anchors and links** — measured in the live DOM: every `href="#…"` resolves to
   an existing element (none of `#wildfire`, `#contact`, `#pilot`, `#services`
   dangling), the document has no duplicate `id`, no JS console/page errors, and
   both `mailto:` targets parse as
   `hello@nordtronics.io?subject=Off-grid%20power%20inquiry` /
   `…?subject=Custom%20electronics%20inquiry`.
5. **`grep -ri insurer website/`** — the required check, run on the merged tree:

   ```
   (no output; exit status 1)
   ```

6. **The untouched parts are provably untouched** — because a vision pass saw
   overlapping labels inside the architecture diagram, I checked rather than
   assumed: the `<svg viewBox="0 0 1200 480" …>` block is **byte-identical** to
   main (sha256 `72e25746ee6d18ea`), and its *render* compared against main's own
   render differs in **0 of 529,920 pixels** once a 1px subpixel layout offset
   (461 vs 462 px tall element, caused by the new paragraph above it) is
   corrected. The same byte-identity check passes for `#sensors`, `#power`
   (the 18650 battery card), `#pilot`, the footer, and `#app`'s content, which
   moved but did not change. The overlaps the vision pass reported are therefore
   pre-existing on main.

## Deviations and scope declarations

- **Scope extension (1 line):** `hermes/0070-site-rewrite` added to
  `.github/workflows/website-check.yml` `on.push.branches`. The criterion "CI
  website workflow passes on the branch" cannot be evidenced without it. The
  same line was needed for 0068.
- **The bare email pill was dropped from the contact section.** The spec
  enumerates *three path cards* and says *keep the GitHub link*; the address is
  still visible inside both card `mailto:` hrefs, so nothing was lost. Say if you
  want the plain address pill back alongside them.
- **Contact card grid minimum reduced** (`minmax(220px → 170px, 1fr)`) after the
  first render put the three cards on two rows (2 + 1, with an empty cell) inside
  the 600px contact column. Now they sit on one row; measured, not eyeballed.
- **Section titles are plain text**, not wrapped in the site's accent `<span>`
  the way the older titles are, so every approved string greps verbatim. Pure
  styling; the wording is exact.
- **No new claims** were introduced: nothing about pilots running, no field
  results, no pricing, and the "no invented pricing" line in the pilot section is
  untouched.
- **No ntfy receipt** — not a build task (no CI artifact). Noting it so its
  absence next to 0069's receipt is not read as an omission.

## Pre-existing observations, reported not changed

- The global `header` rule (`min-height: 100vh`) also applies to every
  `.section-header`, so each section opens with a ~900px empty band. This is
  visible in the committed `website/screenshots/desktop.png` (1280×13943) too and
  pre-dates this task. Out of scope here; worth a task of its own if the intent
  was for `header` to mean only the page hero.
- The privacy strip wraps to two rows inside its 900px max width at 1440px.
  Cosmetic, no text lost.
- The architecture SVG's internal labels overlap at 1440px on main as well
  (proved in verification 6).
