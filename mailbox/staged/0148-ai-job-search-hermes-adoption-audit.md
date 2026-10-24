---
task_id: "0148"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: none
  run: none
  files: []
  evidence: this staged file — the audit body below, with every check output quoted verbatim
  clone: /home/astroboy/audit-ai-job-search (local, outside every repo; HEAD e16acee3abdaae57f8026789338a3c6eebf4608b)
  ntfy: none — no compiled artifact, so the build topic does not apply
proof_note: |
  `branch: none` / `run: none` is earned by enumeration, not assumed (handoff-mailbox rule 18).
  This task changes no nordtronics repository file, so there is no branch to cite and no CI
  run that could cover the deliverable. The six workflows on main and what each can see:
  `website-check.yml` (on.push.branches: [main], no paths filter), `android-build.yml`
  (branches: [android-toolchain-setup]), `android-companion-v0.yml` (branch allow-list +
  `android/companion-v0/**`), `backend.yml` (`backend/**`), `platformio.yml` (`firmware/**`
  + `python/detection-sim/**`), `detection-sim.yml` (`python/detection-sim/**`). None
  compiles Python and none reads a document.
  One run did fire on this tick and is quoted only as context, not as proof: the pickup
  commit 0c0258cf3eb6586ab4aae9a7c093603a375acbab started Website Check run 40044103033
  (`conclusion: success`) — it builds `website/` and says nothing about this file or the
  audit clone. The evidence is the quoted command output in the reply body, which Juno can
  re-run against the public repo at the pinned SHA.
notes: |
  Pickup 2026-10-23 18:15 MDT (2026-10-24T00:15Z), off-peak (PEAK: OFF-PEAK 00:15 UTC),
  protocol MATCH 1.0.0. Read-only audit; nothing installed, nothing from the framework
  executed, no personal data entered. Juno's Context facts were verified against the clone
  and against the GitHub API; five discrepancies are listed at the end of the reply, and the
  two that change the recommendation are that `/scrape` is a skill rather than a command, and
  that criterion 1's `which lualatex`/`which xelatex` checks pass on a TeX install that
  cannot compile either stock template.
  Deviations, all declared: (1) I ran the two stock example compiles
  (`lualatex cv/main_example.tex`, `xelatex cover_letters/cover_example.tex`) inside the
  clone to test the LaTeX rung rather than trusting `which`. Both failed; the .aux/.log
  artifacts they left were deleted afterwards and `git status --porcelain` in the clone is
  empty, so the clone is byte-identical to the checkout. (2) Six `gh api` calls (gh
  porcelain via the github skill's token helper) confirmed star count, licence, default
  branch, commit count and the upstream redirect; no token value was printed. (3) One web
  fetch (DeepSeek pricing page) to ground the cost rate — 1 of the 5 allowed.
  Iteration 1 (pickup this run).
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

---

# Reply — 0148 adoption audit (2026-10-23 18:17 MDT)

**One line:** the framework is adoptable and cheap to run, but the three prerequisites
that actually matter — Bun, a complete LaTeX, and a permission model — are not in place
today, and the Claude Code command layer needs a mechanical port (37% of its in-file tool
references are `WebFetch`/`WebSearch`). Nothing about it is architecturally blocked.

Clone: `/home/astroboy/audit-ai-job-search`, HEAD `e16acee3abdaae57f8026789338a3c6eebf4608b`,
`master`, `git status --porcelain` empty. 6.1 MB, 417 commits, 11 tags.

## prereq_table

| Check (criterion 1) | Verbatim output | Verdict |
|---|---|---|
| `bun --version` | `/usr/sbin/bash: line 6: bun: command not found` (rc 127) | **not found** |
| `python3 --version` | `Python 3.14.7` | OK (repo wants 3.10+) |
| `which lualatex` | `/usr/sbin/lualatex` | path exists — **but see below** |
| `which xelatex` | `/usr/sbin/xelatex` | path exists — **but see below** |
| `which pdftotext` | `/usr/sbin/pdftotext` | OK — poppler `25.07.0`, `-bbox` verified working |

Supplementary checks I ran because the five above cannot decide the rungs:

| Extra check | Output | Consequence |
|---|---|---|
| `lualatex cv/main_example.tex` | `! LaTeX Error: File 'moderncv.cls' not found.` → `!  ==> Fatal error occurred, no output PDF file produced!` | CV cannot compile |
| `xelatex cover_letters/cover_example.tex` | `! LaTeX Error: File 'textpos.sty' not found.` → `! Emergency stop.` | cover letter cannot compile |
| `kpsewhich moderncv.cls` / `textpos.sty` | empty / empty | two packages missing from the TeX tree |
| `lualatex --version` | `This is LuaHBTeX, Version 1.17.0 (TeX Live 2023)`; `/usr/sbin/tlmgr` present | a real TeX Live, on a minimal scheme |
| `kpsewhich fontspec.sty` | `/usr/share/texlive/texmf-dist/tex/latex/fontspec/fontspec.sty` | `cover.cls`'s other dependency is present |
| `python3 -c 'import pypdf'` | `ModuleNotFoundError: No module named 'pypdf'` | `/apply` 5d falls back to `pdftotext`, which works — no action |
| `node --version` | `v26.8.2` | present, but not a Bun substitute (proof under blockers) |
| `which claude` | `not found` | expected; we are replacing it |
| `hermes --version` | `Hermes Agent v0.22.2 (2026.9.24) · stable · local d923a5eb487a` | host agent |

## component_map

Sizes are measured, not estimated: `.claude/commands` = 12 files/2,820 lines,
`.claude/skills` = 13 files/2,109 lines, `.claude/agents` = 3 files/44 lines, `tools/` =
16 Python files/4,784 lines (stdlib-only, with an optional `pypdf` for one check).

| Component | Repo path | Verdict | Reason (grounded in the file) |
|---|---|---|---|
| Profile setup | `.claude/commands/setup.md` (441 lines) | **port** (interactive only) | Three paths — documents folder / single CV / interview; writes the tracked `CLAUDE.md` profile block plus `01`,`02`,`04`,`05`,`06`,`07` and `cv/main_example.tex`. Pure read/ask/write, so the mechanics port cleanly, but it is a multi-turn Q&A: a gateway session, never a cron tick. |
| `/scrape` | `.claude/skills/job-scraper/SKILL.md` (310 lines, `name: scrape`) — **not** a `.claude/commands/` file | **port** | Orchestrates the portal CLIs, dedupes via `tools/scrape_state.py` + `tools/job_key.py`, web search as fallback. Its `allowed-tools` line already assumes `Bash(bun …)`, `Agent`, `AskUserQuestion` — exactly what needs replacing. |
| `/rank` | `.claude/commands/rank.md` (214 lines) + `tools/rank_state.py` (564 lines) | **port** | Candidate selection is a CLI query (`rank_state.py candidates|sweep`); scoring is batch `posting-scorer` agents, ~5 jobs each against a rubric passed inline. Python half ports as-is; agent half maps to `delegate_task`. |
| `/apply` pipeline | `.claude/commands/apply.md` (422 lines / 43.2 KB) | **port — the core deliverable** | Six steps, two agent hand-offs, an untrusted-input gate, then compile + ATS rungs. Every step is a file read, a web fetch or a shell command; only tool names and the Agent call are Claude-specific. Highest value, highest porting cost. |
| `/interview` | `.claude/commands/interview.md` (111 lines) | **port** | Wires `07-interview-prep.md` to the `/outcome` archive and emits a prep pack + optional mock; no exotic tools. |
| `/outcome` | `.claude/commands/outcome.md` (243 lines) | **adopt as-is** | Pure CSV + markdown writes (`job_search_tracker.csv`, `documents/applications/<company>_<role>/outcome.md`) plus a follow-up-note branch. Runs unchanged after the command → skill rename. |
| Portal CLIs | `.agents/skills/{linkedin,freehire,jobindex,jobnet,jobbank,jobdanmark}-search/cli/src/*.ts` | **adopt as-is (needs Bun)** | `linkedin-search` and `freehire-search` have `"dependencies": {}` and no Bun-specific imports; the four Danish portals each need 4 npm packages (`@bunli/core`, `@bunli/utils`, `node-html-parser`, `zod`). All are plain `fetch` with a browser `User-Agent`; no credentials — LinkedIn reads public `jobs-guest` endpoints. |
| `salary_lookup.py` | `salary_lookup.py` (root) | **adopt as-is** | Stdlib only (`json math sys re argparse unicodedata pathlib`); reads a user-supplied, gitignored `salary_data.json`, and `/apply` says to skip the benchmark when it is absent or errors — so it costs nothing to wire later. |
| Posting integrity gate | `tools/posting_guard.py` + `tools/page_decode.py` | **adopt as-is — port this first** | Stdlib-only, 480 lines that strip hidden blocks, instruction-like lines, hidden URLs and mixed-script words from untrusted postings. This is what makes the pipeline safe to run unattended. |
| PDF/ATS/layout checks | `tools/verify_pdf.py`, `tools/verify_layout.py` | **adopt as-is** | Stdlib + `subprocess`; `pypdf` optional with a working `pdftotext -bbox` fallback (verified). `verify_layout.py` encodes real page-break thresholds (hole >100pt, non-final page >25% early) that a visual read misses. |
| Fetch/robots helpers | `tools/fetch_page.py`, `tools/robots_check.py` | **adopt as-is** | Browser-header fallback for 403s; enforces `robots.txt` itself. Directly covers the gap left by `WebFetch`. |
| Agent definitions | `.claude/agents/{posting-fetcher,posting-scorer,application-reviewer}.md` (18/15/11 lines) | **port** | Tiny prompts whose entire safety property is the `tools:` line (fetcher: `Read, Glob, WebFetch, WebSearch, Bash`; reviewer: read-only). Maps to `delegate_task(toolsets=[...])` — see blocker 1 for how coarse that is. |
| Permission policy | `.claude/settings.json` | **skip — no equivalent; rewrite** | 26 allow rules, 9 ask rules, 24 deny rules (`curl -d`, `scp`, `ssh`, `nc`, `Read(.env)`, `Read(~/.ssh/**)`; ask on `git push`). Hermes has no declarative per-command policy — blocker 1. |
| Gmail sync | `.claude/commands/gmail-sync.md` (194 lines) | **skip for now** | Needs a Gmail search+read tool; nothing proven on this host, and its write path requires explicit batch approval a tick cannot give. |
| Notion sync | `.claude/commands/notion-sync.md` (152 lines) | **port later, via MCP** | The file itself says it "requires the Notion MCP server (OAuth)" and that only the two *(Notion binding)* sections are tool-specific. Hermes MCP is a proven rung. |
| HTML dashboard | `.claude/commands/html-report.md` (146 lines) | **adopt as-is** | Self-contained `.html` dashboard from the tracker CSV — "no server, no dependencies". Cheapest visible win in the repo. |
| `/upskill`, `/expand`, `/add-portal`, `/add-template`, `/reset` | `.claude/skills/upskill/SKILL.md` (256), `.claude/commands/{expand,add-portal,add-template,reset}.md` | **port later** | Additive profile enrichment and scaffolding; none is on the path to one correct application. |
| Thin-pointer contract | `AGENTS.md` (19 lines) | **adopt the pattern** | Already declares `.claude/` the single source of truth for non-Claude runtimes — which is Hermes's situation exactly: one pointer file plus skills, no duplicated rules. |

Unassessable read-only: `/setup` Path A's document parsing (needs Stephen's real CV and
diplomas) and `/apply`'s Step 5b PDF-geometry pass (needs a real compiled CV; the shipped
example cannot compile here at all).

## blockers

**1. Hermes has no per-agent tool gating and no declarative deny-list, and both profiles
currently approve everything.**
The repo's safety model is two-layer: `.claude/agents/*.md` give the fetcher and scorer
read/fetch-only tool sets, and `.claude/settings.json` denies `curl -d`, `scp`, `ssh`,
`nc`, `Read(.env)`, `Read(~/.ssh/**)` while requiring approval on `git push`. On this host:

- `~/.hermes/config.yaml` and `~/.hermes/profiles/cronrunner/config.yaml` both carry
  `approvals: mode: 'off'` — pure bypass. `command_allowlist` is an approval *bypass* list,
  not a deny list, and `tools/approval_detection.py` classifies command *shapes* (shell
  wrappers, `-c`/`-e`), not the repo's paths.
- `tools/delegate_tool.py` accepts `toolsets: [...]` per task, so a reviewer subagent can be
  pinned to `["file","web"]` — but that is a coarse toolset, not "these five tools and no
  writes", and the *executing* agent's shell still has no deny list.

Why it matters here specifically: `/apply` puts untrusted third-party posting text into the
same context as write and network tools — the precise threat `posting_guard.py` and the deny
list exist to contain. Mitigation, in order: `posting_guard.py` as a hard gate on every
fetch; `approvals.mode: smart` for a job-search profile; `["file","web"]` toolsets on the
reviewer/scorer subagents. Defensible, but it is not the repo's posture and must be a
decision, not an assumption.

**2. Bun is absent, and Node is not a drop-in — so `/scrape` cannot run at all.**
`bun --version` → command not found. I tested the obvious substitution instead of assuming:
`node --experimental-strip-types .agents/skills/linkedin-search/cli/src/cli.ts --help` →
`ERR_MODULE_NOT_FOUND … Cannot find module '…/src/commands/search.js'`, identical for
`freehire-search`. Those CLIs import extensionless TypeScript paths that only Bun resolves,
so there is no free rung: install Bun, or rewrite imports in six CLIs. `/rank` and `/apply`
do not depend on it (they take a URL), so this blocks discovery, not application drafting.

**3. The LaTeX rung fails on a TeX install that looks complete to `which`.**
`which lualatex` and `which xelatex` both succeed — criterion 1 as literally written passes —
yet neither stock template compiles: `moderncv.cls` not found (CV) and `textpos.sty` not
found (cover letter) on TeX Live 2023. `/apply` Step 5 is explicitly mandatory and
non-skippable ("Never skip this step"), so without those two packages the pipeline cannot
reach Step 6. Fix: `tlmgr install moderncv textpos` (or `dnf install texlive-moderncv
texlive-textpos`), plus the in-tree `cover_letters/OpenFonts/{lato,raleway}` assets. Cheap,
but it is an install, and this audit was not permitted to run one.

## privacy_layout

```
1. Private GitHub repo `jobsearch` (no fork, no public mirror), seeded from this clone; the
   original stays a read-only upstream remote for /upstream-watch pulls only.
2. Identity lives in tracked files — CLAUDE.md, .claude/skills/…/01,02, cv/main_example.tex —
   so private visibility is the only acceptable setting. Never a public fork.
3. The existing .gitignore is already the right shape (verified in-tree): it denies
   salary_data.json, job_search_tracker.csv, documents/** (except .gitkeep), cv/main_*.*,
   cover_letters/cover_*.*, company_research/*.json, job_scraper/seen_jobs.json, .env*.
4. Keep security_guards.py's REQUIRED_IGNORE_RULES check in CI as the regression guard — a
   tidier .gitignore that drops one pattern is the realistic failure mode.
5. Run ported skills in a Hermes worktree of that private repo, never in nordtronics or
   cabin-play, so one `git add -A` cannot cross the boundary.
```

## cost_estimate

One full `/apply` run = **~17 model calls, ~450k billed input tokens, ~14.5k output tokens**
→ **≈$0.02 off-peak, ≈$0.04 peak** on `deepseek-flash` (DeepSeek-V4.1-Flash-1017).

Reasoning, per step (context sizes computed from the actual file byte counts at 3.6 bytes/token):

| Step | What calls the model | Calls | Context | Billed input |
|---|---|---|---|---|
| 0 posting-fetcher agent | fetch + `posting_guard`, return cleaned text | 2 | ~1.4k | ~3k |
| 1 evaluate fit | reads `apply.md` (43 KB) + `04` (15.7 KB) + `01` (2 KB) + posting | 2 | ~18.6k | ~40k |
| 2 draft CV + cover letter | adds `03` + `05` (27.5 KB) + `06` + master CV + both drafts | 2 | ~35.0k | ~75k |
| 3 reviewer agent (fresh context) | reads `01,02,03,04`, master CV, `CLAUDE.md`, both drafts + 4 web fetches | 5 | ~21.9k | ~142k |
| 4 revise | applies Part A edits + Part B narrative | 2 | ~32.7k | ~70k |
| 5 compile + inspect | 2 LaTeX compiles, `verify_pdf`/`verify_layout`, PDF page images | 3 | ~29.0k | ~100k |
| 6 verify + tracker + archive | final checklist, CSV row, posting archive | 1 | ~19.3k | ~19k |
| **total** | | **17** | peak ~35k | **~450k** |

Every call re-sends the accumulated context — that is why billed input (450k) is 13× the peak
context (35k), and why the workflow's own "never re-read a file" rules matter more than the
document sizes. At the published rates (input cache-hit $0.003/M, cache-miss $0.15/M, output
$0.60/M off-peak; peak doubles), assuming 85% cache-hit:

`0.85 × 0.45M × $0.003 + 0.15 × 0.45M × $0.15 + 0.0145M × $0.60 = $0.0011 + $0.0101 + $0.0087 = $0.0200`

Peak (01:00–04:00 and 06:00–10:00 UTC, Mon–Fri) doubles it to **$0.0399** — the cost lever is
*when* you run it, not which model. DeepSeek's Search/Fetch APIs are free until 2026-11-01 and
$0.75 per 1,000 requests after. For scale: at $0.02 a run, the ~$22.78 balance covers
>1,000 applications' worth of model spend — Stephen's time is the real constraint. A `/rank`
batch of 10 jobs is ~24 calls, ≈$0.03.

## rung_ladder

| Rung | What | Why this order | Gate to advance |
|---|---|---|---|
| **R0 — unblock (≈30 min, no porting)** | install Bun; `tlmgr install moderncv textpos`; decide `approvals.mode` for a job-search profile | converts all three blockers into known-good or known-bad before any porting effort | `bun --version` prints; both `*_example.tex` compile clean |
| **R1 — pilot (the named first pilot)** | port `posting_guard.py` + `tools/*` + the three agent prompts into one Hermes skill `job-apply`, then run **one real posting** end-to-end on DeepSeek: fetch → gate → evaluate → draft → review → revise → compile → ATS check | proves the pipeline on real content without `/scrape`'s discovery half; ~17 calls, ~$0.02 | a real posting yields a 2-page CV + 1-page letter with `verify_pdf --pages` and `verify_layout` clean, Part A edits applied |
| **R2 — discovery** | port `/rank` + the `job-scraper` skill over `freehire-search` and `linkedin-search` only (both zero-dep) | a weekly sweep without four Danish boards Stephen does not use; LinkedIn reads public endpoints — keep volume low, its own ToS note says personal use | a `/rank` batch of 10 real postings returns a shortlist, dedupe proven against the tracker |
| **R3 — record + review (cheap wins)** | `/outcome` as-is, `/interview`, `/html-report` | all pure-local file writes, no Claude-specific tools, immediate value | tracker row round-trips through `/outcome` → `/html-report` |
| **R4 — profile build** | port `/setup` Path C (interview mode) as a gateway session | one-time, interactive, writes identity into tracked files — needs `privacy_layout` first | `CLAUDE.md` + `01`/`02` populated, zero placeholders left |
| **R5 — integrations** | `/notion-sync` via Hermes MCP; `/gmail-sync` only if a Gmail tool is proven | OAuth grants and a write path into permanent records; last for a reason | one pending approval batch written correctly |

Deferred deliberately: `/add-portal`, `/add-template`, `/expand`, `/upskill`, `/reset` — none
is needed to produce one correct application, and each adds surface.

## attestation

- **installed:** none. No `bun install`, no pip, no `tlmgr`, no LaTeX package, no
  `hermes plugins install`.
- **pushed:** none to the audited repo — the clone has no write remote and no commit was made
  in it (`git status --porcelain` empty). The only push this run was the protocol's own
  mailbox commit to `pagosacabin/nordtronics` main.
- **personal-data-entered:** none. No name, contact detail, employment history, salary
  expectation or document of Stephen's was read or written; the clone's profile files are
  still the shipped templates (verified: `CLAUDE.md` line 1 reads
  `# Job Application Assistant for [YOUR_NAME]`).
- **clone:** left local at `/home/astroboy/audit-ai-job-search`, outside every repo, `master`
  @ `e16acee3abdaae57f8026789338a3c6eebf4608b`.
- **framework commands run:** none. No `/setup`, `/scrape`, `/rank` or `/apply`; the only
  framework-adjacent commands were the two stock `*_example.tex` compiles declared above.

## notes — discrepancies with Juno's Context

1. **`/scrape` is not a command.** `.claude/commands/` holds 12 files and none is
   `scrape.md`; the scraper is `.claude/skills/job-scraper/SKILL.md` (310 lines, `name:
   scrape`), which `AGENTS.md` confirms ("the `/scrape` workflow in
   `.claude/skills/job-scraper/`"). Same for `/upskill`. Effect: those two port as *skills*,
   not command ports — slightly less work, not blocked.
2. **There is no separate upstream.** `gh api repos/MadsLorentzen/ai-job-search` resolves to
   `ai-job-search-dev/ai-job-search` (`fork: false`, 64,096 stars, MIT, `master`, last push
   2026-10-21T05:29:11Z) — a GitHub rename redirect, not a fork relationship. "Upstream:
   MadsLorentzen/ai-job-search" should be read as "the same repo, renamed". Commit count 417
   confirmed independently (`Link: …page=417; rel="last"`); 11 tags exist, so a versioned pin
   is possible.
3. **Criterion 1's LaTeX checks pass on a broken rung.** `which lualatex` and `which xelatex`
   both return paths, yet both stock templates fail to compile for missing `moderncv.cls` and
   `textpos.sty`. I compiled rather than trusted `which`; flagged as blocker 3. The criterion
   as written would have reported "OK".
4. **Bun is a hard dependency, not just "for the CLIs".** The README's "portal skills work in
   non-Claude agents out of the box" is true of the *skill format* (agent-agnostic
   `SKILL.md`), not the runtime: without Bun the CLIs do not start, and Node's TypeScript
   stripping does not rescue them (proved). Read "out of the box" as "no Claude needed", not
   "no Bun needed".
5. Two facts that make adoption cheaper than it looks: `tools/*.py` is stdlib-only end to
   end (no pip rung at all), and `.gitignore` already encodes the personal-data exclusions the
   README only describes — so the privacy work is the repo choice, not a policy rewrite.
