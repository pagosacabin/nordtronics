---
task_id: "0148"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0148 — Audit: can Hermes adopt the ai-job-search framework?

## Context

Stephen found this repo and asked whether Hermes can adopt it. Juno read the
repo on 2026-10-23; facts below are from that read, verify them against your
clone.

- Repo: https://github.com/ai-job-search-dev/ai-job-search (upstream:
  MadsLorentzen/ai-job-search). MIT license. ~64k stars, 417 commits, default
  branch `master`, commit e16acee3abdaae57f8026789338a3c6eebf4608b at read time.
- What it is: a job-application framework built on Claude Code. Core commands:
  `/setup` (builds a profile from documents/CV), `/scrape` (search job portals),
  `/rank` (batch fit-scoring), `/apply <url>` (evaluate posting → draft tailored
  CV + cover letter in LaTeX → second-agent review → revise), plus `/interview`,
  `/outcome`, `/gmail-sync`, `/notion-sync`, `/upskill`, `/expand`.
- Structure: `.claude/` (Claude Code commands), `.agents/skills/` (portal search
  skills with Bun CLIs under `.agents/skills/*/cli/`), `AGENTS.md` (the route
  for non-Claude agent tools — README says the portal skills work there out of
  the box), `salary_lookup.py`, LaTeX templates in `templates/` and `cv/`.
- Stated prerequisites: Claude Code (no free tier), Python 3.10+, Bun, a LaTeX
  distribution with `lualatex` (CV) and `xelatex` (cover letter); optional
  `pypdf` / Poppler `pdftotext` for the ATS parseability check.
- Portal skills target the Danish market (Jobindex, Jobnet, Akademikernes
  Jobbank); `linkedin-search` and `freehire-search` are the exceptions and have
  zero runtime dependencies (plain Bun).
- Privacy fact from the README itself: a GitHub fork of a public repo is always
  public, and `/setup` writes personal data (name, contact details, employment
  history, salary expectations) into tracked files. The README's own fix is a
  private repository with the original as upstream.
- Stephen's situation (why this matters): remote-work search targeting
  $1,200–$2,000/month, ~20 hrs/week, mainly Upwork / Indeed / Handshake /
  Freelancer — not Danish boards. His registry-removal case makes the privacy
  point above a hard constraint, not a preference: his profile data never goes
  in a public repo.
- Your proven rungs Juno is relying on: real `hermes` CLI + gateway, `plugins
  install`, `config get/set`, skill encoding, git commit/push from the laptop,
  DeepSeek V4.1-Flash with web search/fetch (0147, cronrunner profile).
- Cost constraint, stated explicitly per Stephen's standing rule: run this on
  the cheap/flash tier you already use. Stephen's DeepSeek balance is ~$22.78
  topped-up with ~$21.46 total spend over the last 30 days. This audit is
  local reading plus a handful of version checks — keep any web searches ≤5,
  prefer reading the local clone.

## Task

One deliverable: an adoption audit, staged as the reply to this task file.
Clone the repo read-only to `/home/astroboy/audit-ai-job-search/` (outside
every git repo — inspection copy only), then produce the audit in the Reply
format below: a prerequisites check against your actual machine, a
component-by-component adopt/port/skip map, the top blockers, a privacy-safe
repo layout recommendation, a DeepSeek cost estimate for one `/apply` run, and
a proposed adoption rung ladder.

## Success criteria

1. Prerequisites table reports the actual outputs from your machine for:
   `bun --version`, `python3 --version`, `which lualatex`, `which xelatex`,
   `which pdftotext` — each either a version/path or an explicit "not found".
2. Component map covers at least: profile setup (`/setup`), `/scrape`,
   `/rank`, the `/apply` pipeline (evaluate → draft → review → revise),
   `/interview`, `/outcome`, the portal CLIs under `.agents/skills/`,
   `salary_lookup.py`. Each row: repo path, verdict (adopt as-is / port to a
   Hermes skill / skip), one-line reason grounded in what the file actually
   contains.
3. Top blockers section names the 3 biggest adoption risks specific to your
   install (or fewer, stated as such) — e.g. anything in the Claude Code
   command layer that has no Hermes equivalent you have proven.
4. Cost estimate states the expected number of model calls and rough token
   volume for one full `/apply` run on DeepSeek V4.1-Flash, with the reasoning
   shown (which steps call the model).
5. Attestation line confirms: nothing installed, nothing pushed, no personal
   data of Stephen's entered anywhere, clone left local at the path above.

## Constraints

- Read-only audit. Install nothing: no `bun install`, no pip, no LaTeX install,
  no `hermes plugins install`. Version checks and file reading only.
- Run no framework command (`/setup`, `/scrape`, `/apply`) — this task audits,
  it does not execute the workflow.
- Enter none of Stephen's personal data anywhere. If a component can only be
  assessed by running it with real profile data, mark it "unassessable
  read-only" in the map instead.
- Push nothing. The clone stays local and outside the nordtronics and
  cabin-play repos. If you recommend a private GitHub mirror layout, that is a
  recommendation in the report only.
- Use the cheap/flash model tier for this task. Web searches ≤5 if used at all.
- Where Juno's Context facts disagree with what you find in the clone, trust
  the clone and flag the discrepancy in `notes`.

## Proof

Host/local-only audit: no nordtronics repo file changes, so `branch: none`,
`run: none` — honest non-proof per the 0147 precedent. The evidence is the
staged reply carrying the exact check outputs and the component map with repo
paths Juno can spot-check against the public repo.

## Reply format

Fill in exactly these fields when staging:

- `prereq_table`: the five checks from criterion 1, verbatim outputs
- `component_map`: table — component | repo path | verdict | reason
- `blockers`: top adoption blockers (≤3), each with the repo path or local
  fact it rests on
- `privacy_layout`: recommended private-repo layout, ≤5 lines
- `cost_estimate`: model calls + rough tokens for one `/apply` run, reasoning
  shown
- `rung_ladder`: proposed adoption rungs in order, first pilot named (one real
  posting through a ported `/apply`)
- `attestation`: installed / pushed / personal-data-entered — each "none"
- `notes`: discrepancies with Juno's Context, anything unexpected
