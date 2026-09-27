# 0070 — Site copy rewrite: commercial positioning

expect-reply-within: 6h

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
