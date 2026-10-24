---
task_id: "0151"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP 2026-10-24 17:15 UTC (PEAK: OFF-PEAK, PROTOCOL MATCH 1.0.0).
  No front-matter was present on arrival; protocol fields added at pickup.
---
# 0151 — ai-job-search pilot run (0149 follow-up, seed now in place)

expect-reply-within: 6h

## Context

- 0149 (verified, archived; staged copy at `mailbox/staged/0149-ai-job-search-r0-r1-pilot.md`) completed steps 1–3 and stopped on its own blocked path: `pilot_status: blocked-on-profile-seed`. The pilot itself did NOT run, and no Upwork MCP call was made.
- What 0149 already proved, do not redo: Bun 1.4.4 installed; TeX packages installed user-local in `~/texmf` and both stock examples compile (CV 2 pages, cover 1 page, evidence in `/home/astroboy/jobsearch/r0-compile/`); Hermes profile `jobsearch` created with `approvals.mode: smart`, default and cronrunner verified byte-identical; one skill `job-apply` built at `/home/astroboy/.hermes/profiles/jobsearch/skills/job-search/job-apply/`, with `scripts/fetch_gated.py` as the ONLY fetch path invoking `scripts/posting_guard.py` (fail-closed: non-zero guard exit or empty output deletes the cleaned file and exits 3, posting text never printed to stdout).
- The unblock has now happened, by Stephen himself on 2026-10-24 morning: he placed the two files Juno emailed him on 2026-10-23 (the Sep 28, 2026 resume, DOCX + PDF) into `/home/astroboy/.hermes/jobsearch-seed/`.
- Pilot posting (real, live, chosen by Juno, Stephen has NOT applied to it): Upwork job **2113244746195338786** — ESP32 LoRa TX/RX boards, ~$300, Chile client. Read it through the proven read-only Upwork MCP (0114: 32 tools, zero writes).
- Privacy (Stephen's hard constraint, from 0148): all pilot artifacts stay LOCAL on this machine under `/home/astroboy/jobsearch/`. Nothing of Stephen's — profile, CV, cover letter, posting archive — is committed to nordtronics, cabin-play, or any remote. The private GitHub repo from the 0148 layout is a later rung and is NOT part of this task.
- Cost constraint, explicit: DeepSeek spend for this task ≤ **$0.50**. Use the `jobsearch` profile's model, deepseek-flash, only — never deepseek-v4-pro. 0149 spent $0.00 of its $0.50 bound, so this bound covers the pilot run.

## Task

One deliverable: **a completed pilot application package for Upwork posting 2113244746195338786, produced by the already-ported pipeline on this machine** — fit evaluation, tailored CV (PDF), cover letter (PDF).

Steps:

1. **Verify prerequisites, do not reinstall.** Confirm Bun 1.4.4, the `jobsearch` profile with `approvals.mode: smart`, and the `job-apply` skill at the path above with its gated fetch path. If any prerequisite is missing, stop and stage with `pilot_status: blocked-on-prerequisite` naming exactly what is missing — do not reinstall or recreate anything silently.
2. **Check the seed first.** List `/home/astroboy/.hermes/jobsearch-seed/` and use ONLY the files Stephen placed there (the emailed Sep 28 DOCX + PDF). Do NOT copy the `~/Downloads` candidates (`Stephen_Nordlund_long.pdf`, `Stephen_Nordlund_short.pdf`, 2026-09-01) into the seed — choosing the authoritative CV was Stephen's call and he has now made it. If the seed directory is empty or missing when you reach this step, stop and stage with `pilot_status: blocked-on-profile-seed` and exactly what file you need. Invent no profile content — no filler name, no template CV.
3. **Pilot run.** Read posting 2113244746195338786 via the Upwork MCP (read-only), through the gated fetch path only. Run the pipeline end-to-end: gated fetch → evaluate fit → draft tailored CV + cover letter from the profile seed → reviewer pass → revise → compile both PDFs → `verify_pdf` / `verify_layout` + ATS check (`pdftotext` fallback is fine; `pypdf` is absent per 0148). Artifacts land in `/home/astroboy/jobsearch/pilot-2113244746195338786/`.

Blocked paths (report, don't improvise):
- If the Upwork MCP cannot read the posting: stage with `pilot_status: blocked-on-posting-text`. Juno will supply the text in a follow-up; do not scrape Upwork with the browser instead.
- If the pilot needs a second full model run that would push spend past $0.50: stop and stage with the reason instead of spending past the bound.

## Success criteria

1. Prerequisites verified: Bun version, `jobsearch` approvals mode, and `job-apply` skill path + posting_guard hook point, each stated with how you verified it.
2. Seed directory listing stated (filenames only in the staged reply); the tailored drafts are built from the seed files, not from the Downloads candidates and not from invented content.
3. The artifact directory holds the cleaned posting text, the fit evaluation, `cv.pdf` (page count from `verify_pdf`), `cover.pdf`, and the reviewer's Part A edit list with each edit marked applied or rejected-with-reason.
4. `verify_pdf` and `verify_layout` outputs quoted for both PDFs; ATS check result stated.
5. Reply reports the DeepSeek call count for the pilot and the estimated spend against the $0.50 bound.
6. Attestation: nothing submitted to Upwork (no proposal, no message — the MCP stayed read-only); nothing of Stephen's committed or pushed anywhere.

## Constraints

- The pilot produces a **draft package for Stephen's review**. Submit nothing, send nothing, apply nowhere. Stephen submits applications himself.
- Scope the approvals change to nothing: this task changes no profile. The `jobsearch` profile already exists with smart approvals; default and cronrunner stay untouched.
- All personal-data artifacts stay under `/home/astroboy/jobsearch/` and `/home/astroboy/.hermes/jobsearch-seed/`. The staged reply carries metadata only (paths, page counts, verdicts, check outputs) — include the fit evaluation's one-line verdict and score, and leave the full cover-letter text and CV text in the local files only.
- Stay inside the $0.50 DeepSeek bound for this task, flash tier only.
- Where the framework's Claude-specific tool names appear in the ported prompts, use the proven mappings recorded in 0149's reply; invent no capability you have not proven on this machine.

## Proof

Mixed task, host-local by design (per 0147/0148 precedent): exact command outputs in the reply are the evidence for the prerequisite checks; the pilot artifacts are local files on Stephen's machine — paths + check outputs in the reply; Stephen is the eyes on the PDFs themselves. No nordtronics repo change, so `branch: none`, `run: none`.

## Reply format

Fill in exactly these fields when staging:

- `prerequisites`: Bun version, jobsearch approvals value, job-apply skill path + posting_guard hook point, how each was verified
- `seed_files`: filenames found in `/home/astroboy/.hermes/jobsearch-seed/`
- `pilot_status`: complete | blocked-on-profile-seed | blocked-on-posting-text | blocked-on-prerequisite
- `fit_verdict`: one line + score from the evaluation (skip if blocked)
- `artifacts`: artifact directory path + file list with page counts
- `verify_outputs`: verify_pdf / verify_layout / ATS results for both PDFs
- `review_edits`: count of Part A edits, applied vs rejected
- `model_calls`: count + estimated spend vs the $0.50 bound
- `attestation`: submitted nothing / pushed nothing personal — each stated plainly
- `notes`: anything unexpected, verbatim errors where a step failed
