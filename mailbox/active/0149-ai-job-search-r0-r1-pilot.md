---
task_id: "0149"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0149 — ai-job-search R0+R1: unblock the prerequisites, port the apply pipeline, run one pilot

## Context

- 0148 (verified, archived) audited `ai-job-search-dev/ai-job-search` at clone
  HEAD `e16acee3abdaae57f8026789338a3c6eebf4608b`, local at
  `/home/astroboy/audit-ai-job-search`. Its rung ladder, which Stephen approved
  on 2026-10-23, starts: **R0** — install Bun, `tlmgr install moderncv textpos`,
  decide approvals mode; **R1 (the pilot)** — port `tools/posting_guard.py` +
  `tools/*` + the three `.claude/agents/` prompts into one Hermes skill and run
  one real posting end-to-end. This task executes R0 and R1 as one deliverable:
  the pilot result.
- Stephen approved the mitigation package from 0148's blocker 1 in the same
  go-ahead: `posting_guard.py` as a hard gate on every posting fetch, a
  dedicated job-search profile with `approvals.mode: smart`, and the
  reviewer/scorer subagents pinned to `["file","web"]` toolsets.
- 0148's three blockers, verbatim from the audit: (1) both existing profiles
  carry `approvals: mode: 'off'`, no declarative deny-list exists; (2) `bun` not
  found, Node cannot run the portal CLIs (`ERR_MODULE_NOT_FOUND` on
  extensionless TS imports); (3) `lualatex`/`xelatex` exist (TeX Live 2023) but
  `moderncv.cls` and `textpos.sty` are missing, so neither stock template
  compiles.
- Pilot posting (real, live, chosen by Juno): Upwork job
  **2113244746195338786** — ESP32 LoRa TX/RX boards, ~$300, Chile client.
  Dead-center Stephen's bench loop (Heltec LoRa 32, ESP32). He has NOT applied
  to it. Your Upwork MCP is a proven read-only rung (0114: 32 tools, zero
  writes) — read the posting through it.
- Pilot profile seed: Stephen stages his current resume/CV file at
  `/home/astroboy/.hermes/jobsearch-seed/` (file-drop convention, 2026-10-08).
  Check that directory first. It may not be there yet when you pick this up —
  see the blocked-path rule in the Task.
- Privacy (0148 layout, Stephen's hard constraint): all pilot artifacts stay
  LOCAL on this machine under `/home/astroboy/jobsearch/`. Nothing of
  Stephen's — profile, CV, cover letter, posting archive — is committed to
  nordtronics, cabin-play, or any remote. The private GitHub repo from the
  0148 layout is a later rung and is NOT part of this task.
- Cost constraint, explicit: DeepSeek spend for this whole task ≤ **$0.50**
  (0148 estimated ~$0.02–0.04 for one `/apply` run; the bound covers retries).
  Cheap/flash tier only. Report the actual call count in the reply.

## Task

One deliverable: **a completed pilot application package for the Upwork
posting above, produced by the ported pipeline on this machine** — fit
evaluation, tailored CV (PDF), cover letter (PDF) — with the three R0 blockers
cleared on the way. Steps:

1. **R0 installs.** Install Bun by its official route. Run
   `tlmgr install moderncv textpos` (or the dnf equivalents from 0148). Verify:
   `bun --version` prints a version, and both stock examples from the audit
   clone (`cv/main_example.tex` under lualatex, `cover_letters/cover_example.tex`
   under xelatex) compile to PDF clean.
2. **Dedicated profile.** Create a Hermes profile named `jobsearch` with
   `approvals.mode: smart`. Leave the default and cronrunner profiles
   byte-identical — verify and report that you did.
3. **Port.** Into the `jobsearch` profile, build one skill, `job-apply`,
   carrying: `tools/posting_guard.py` + the `tools/*.py` set it needs
   (stdlib-only per 0148), and the three agent prompts
   (`.claude/agents/posting-fetcher.md`, `posting-scorer.md`,
   `application-reviewer.md`) mapped to your subagent mechanism with the
   reviewer/scorer pinned to `["file","web"]`. Every posting fetch passes
   through `posting_guard.py` before any model sees the text — no bypass path.
4. **Pilot run.** Read posting 2113244746195338786 via the Upwork MCP
   (read-only). Run the pipeline end-to-end on DeepSeek: gated fetch →
   evaluate fit → draft tailored CV + cover letter from the profile seed →
   reviewer pass → revise → compile both PDFs → `verify_pdf` / `verify_layout`
   + ATS check (`pdftotext` fallback is fine; `pypdf` is absent per 0148).
   Artifacts land in `/home/astroboy/jobsearch/pilot-2113244746195338786/`.

Blocked paths (report, don't improvise):
- If `/home/astroboy/.hermes/jobsearch-seed/` is empty or missing when you
  reach step 4: complete steps 1–3, then stage this task with
  `pilot_status: blocked-on-profile-seed` and exactly what file you need.
  Invent no profile content — no filler name, no template CV.
- If the Upwork MCP cannot read the posting: stage with
  `pilot_status: blocked-on-posting-text`. Juno will supply the text in a
  follow-up; do not scrape Upwork with the browser instead.

## Success criteria

1. `bun --version` prints; both stock example .tex files compile to PDF with
   no fatal error (quote the final line of each compile log).
2. A `jobsearch` profile exists with `approvals.mode: smart`; default and
   cronrunner configs unchanged (state how you verified).
3. The `job-apply` skill exists in the jobsearch profile, and its fetch path
   invokes `posting_guard.py` — name the file and line/mechanism.
4. Pilot (unless a blocked path fired, stated as such): the artifact directory
   holds the cleaned posting text, the fit evaluation, `cv.pdf` (page count
   from `verify_pdf`), `cover.pdf`, and the reviewer's Part A edit list with
   each edit marked applied or rejected-with-reason.
5. `verify_pdf` and `verify_layout` outputs quoted for both PDFs; ATS check
   result stated.
6. Reply reports the DeepSeek call count for the pilot and the estimated spend
   against the $0.50 bound.
7. Attestation: nothing submitted to Upwork (no proposal, no message — the MCP
   stayed read-only); nothing of Stephen's committed or pushed anywhere;
   installs limited to Bun + the two TeX packages.

## Constraints

- The pilot produces a **draft package for Stephen's review**. Submit nothing,
  send nothing, apply nowhere. Stephen submits applications himself.
- Scope the approvals change to the new `jobsearch` profile only. Default and
  cronrunner keep `approvals: mode: 'off'` untouched.
- Installs are exactly: Bun, `moderncv`, `textpos`. If a step wants more
  (e.g. `pypdf`), skip it and note it — 0148 verified the `pdftotext` fallback
  works.
- All personal-data artifacts stay under `/home/astroboy/jobsearch/` and
  `/home/astroboy/.hermes/jobsearch-seed/`. The staged reply carries metadata
  (paths, page counts, verdicts, check outputs) — include the fit evaluation's
  one-line verdict and score, and leave the full cover-letter text in the
  local file only.
- Stay inside the $0.50 DeepSeek bound for the task. If the pilot needs a
  second full run, stop and stage with the reason instead of spending past it.
- Where the framework's Claude-specific tool names appear in the ported
  prompts, map them to your proven equivalents and list each mapping in the
  reply; invent no capability you have not proven on this machine.

## Proof

Mixed task. Installs and the port are host-local (honest non-proof for those,
per 0147/0148: exact command outputs in the reply are the evidence). The pilot
artifacts are local files on Stephen's machine — paths + check outputs in the
reply; Stephen is the eyes on the PDFs themselves. No nordtronics repo change,
so `branch: none`, `run: none`.

## Reply format

Fill in exactly these fields when staging:

- `bun_version`: verbatim output
- `tex_compiles`: final log line for each stock example (lualatex CV, xelatex
  cover)
- `profile_state`: jobsearch profile created + approvals value; how default
  and cronrunner were verified unchanged
- `skill_path`: where `job-apply` lives; the posting_guard hook point
- `tool_mappings`: each Claude tool name in the ported prompts → the Hermes
  equivalent used
- `pilot_status`: complete | blocked-on-profile-seed | blocked-on-posting-text
- `fit_verdict`: one line + score from the evaluation (skip if blocked)
- `artifacts`: artifact directory path + file list with page counts
- `verify_outputs`: verify_pdf / verify_layout / ATS results for both PDFs
- `review_edits`: count of Part A edits, applied vs rejected
- `model_calls`: count + estimated spend vs the $0.50 bound
- `attestation`: submitted nothing / pushed nothing personal / installs limited
  to Bun + moderncv + textpos — each stated plainly
- `notes`: anything unexpected, verbatim errors where a step failed
