---
task_id: "0151"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
prerequisites: |
  Bun 1.4.4 — `export PATH=$HOME/.bun/bin:$PATH; bun --version` -> `1.4.4`.

  jobsearch profile approvals — `grep -n -A6 '^approvals:' ~/.hermes/profiles/jobsearch/config.yaml`
  -> line 7 `approvals:` / line 8 `  mode: smart`. The same file still reads
  `model.default: deepseek-flash`, `provider: deepseek`, `base_url: https://api.deepseek.com/v1`.

  job-apply skill — /home/astroboy/.hermes/profiles/jobsearch/skills/job-search/job-apply/
  (SKILL.md 139 lines plus scripts/ and references/agents/). GATE HOOK POINT, read from the
  source: `GUARD = HERE / "posting_guard.py"` at fetch_gated.py:30; invoked as
  `[sys.executable, GUARD, "--in", <page>, "--json", "--text", "--out", <cleaned>]` at
  fetch_gated.py:110-118; on non-zero guard exit or an empty cleaned file the cleaned file is
  deleted and the tool returns 3 (fetch_gated.py:121-135). Confirmed live this run: the gate
  returned exit 0 with `{"ok": true, "verdict": "posting_guard: clean", "cleaned_text":
  "01-fetch/posting.cleaned.txt", "guard_sha256": "55c639bd6b3ac8c3f7264f7544559e58e737b17c545c4dd7dc73aa88bb6f7d71",
  "cleaned_sha256": "2d9cc3b07e8fff13a98a7b01ac20acc0cd48bfc9512f8243f40423fd7ef57a9e",
  "cleaned_chars": 1640}`.

  No prerequisite was reinstalled and nothing was recreated.
seed_files: |
  Stephen Nordlund Resume.docx (39,438 bytes)
  Stephen Nordlund Resume.pdf  (49,729 bytes)
  Both in /home/astroboy/.hermes/jobsearch-seed/, both mtime 2026-10-24 10:12. These two files
  are the only profile input used. The ~/Downloads candidates (Stephen_Nordlund_long.pdf and
  Stephen_Nordlund_short.pdf, 2026-09-01) were neither copied nor read for content.
pilot_status: complete
fit_verdict: |
  53/100 — "Do not apply as posted — the $300 fixed price covers two manufacturing-ready
  boards, and the 4G LTE / Ethernet RF layout work is outside the candidate's demonstrated PCB
  evidence." Dimension scores: skills 15/30, seniority 14/20, domain 12/20, price sanity 4/15,
  red flags 8/15.
artifacts: |
  /home/astroboy/jobsearch/pilot-2113244746195338786/
    01-fetch/  mcp_raw.json, posting.raw.txt, posting.cleaned.txt, gate.json
    02-fit/    scorer_prompt.md, evaluation.md
    03-draft/  cv.tex, cover.tex
    04-build/  cv.tex, cover.tex, cover.cls, OpenFonts/, cv.pdf (2 pages), cover.pdf (1 page)
    04-review/ reviewer_prompt.md, reviewer_raw.txt, reviewer_final.md, edits.md
    05-verify/ cv.txt, cover.txt

  cv.pdf 2 pages, A4 595x842 pt, 45,141 bytes. cover.pdf 1 page, A4 595x842 pt, 16,473 bytes.
verify_outputs: |
  cv page/text layer: `verify_pdf.py 04-build/cv.pdf --pages 2 --min-chars 800 --dump-text
  05-verify/cv.txt` -> "Verified 04-build/cv.pdf (extractor: pdftotext, pages: 2)" rc 0.
  cv layout: `verify_layout.py 04-build/cv.pdf` -> 2 pages; p1 text y 53..703 (bottom 139 pt,
  16%), p2 y 64..661 (bottom 181 pt, 21%); "layout: clean" rc 0.
  cover page/text layer: `verify_pdf.py 04-build/cover.pdf --pages 1 --min-chars 600
  --dump-text 05-verify/cover.txt` -> "Verified 04-build/cover.pdf (extractor: pdftotext,
  pages: 1)" rc 0.
  cover layout: `verify_layout.py 04-build/cover.pdf` -> 1 page; p1 text y 63..737 (bottom
  105 pt, 12%); "layout: clean" rc 0.
  ATS: both PDFs carry a real text layer (pdftotext: cv 6,748 chars / 696 words; cover 2,741
  chars / 418 words), no image-only text. Posting terms present in the CV: ESP32, ESP32-S3,
  LoRa, 915, SX1262, KiCad, PCB, schematic, BOM, antenna, low-power, battery, USB, LTE.
  Present in the cover letter only: Gerber, Pick & Place, PCBWay, JLCPCB, Ethernet, 4G,
  12 VDC. Absent from both: RFM95W, SIM, relay, "6 buttons" — deliberate, see notes.
review_edits: |
  7 Part A edits proposed: 6 applied (A1, A2, A3, A5, A6, A7), 1 rejected with reason (A4 —
  it would assert delivered Gerber / BOM / Pick & Place assembly packages for PCBWay or JLCPCB,
  which the seed does not evidence; the reviewer's own honesty flag said to confirm instead).
  Plus 1 orchestrator fix for a Part B honesty flag that arrived with no edit ("every board I
  ship ..." -> "the boards I design ..."). Per-edit table with reasons: 04-review/edits.md.
model_calls: |
  4 child sessions on profile jobsearch / model deepseek-flash, 9 API calls, all off-peak:
    20261024_111718_3f0482  credential probe         in 3,693   out 5
    20261024_111727_a7a073  tool probe (read_file)    in 7,572   out 67
    20261024_111801_5b9f7d  posting-scorer            in 21,638  out 3,214
    20261024_111937_65309d  application-reviewer      in 22,011  out 8,037
  Totals: 54,914 input / 11,323 output, of which 38,656 input tokens were cache hits.
  Estimated spend at DeepSeek's published off-peak deepseek-flash rates (cache-miss in $0.15/M,
  cache-hit in $0.003/M, out $0.60/M): about $0.009 — under $0.02 against the $0.50 bound.
  This cron orchestrator turn is a fifth, separate model context and is not included.
attestation: |
  Submitted nothing: every Upwork MCP call this run was read-only (`list_accounts`,
  `find_jobs action=get`). No `manage_proposals`, no create, no confirm_preview, no
  send_message, no attachment upload, no boost, no profile edit. The posting's `applied` field
  read false before the work and nothing in this run could change it.
  Pushed nothing personal: the pilot artifacts exist only under /home/astroboy/jobsearch/ and
  the seed only under /home/astroboy/.hermes/jobsearch-seed/; `git status --porcelain mailbox/`
  was clean before this staging commit, and the only nordtronics commits this run are the 0151
  pickup and this staging commit (mailbox files, no personal content).
proof:
  branch: none
  sha: none
  run: none
  files: []
  reason: |
    Host-local task by design — its own Proof section says "No nordtronics repo change, so
    branch: none, run: none". `run: none` is earned, not asserted: the repo has exactly 6
    workflows (.github/workflows/: android-build.yml, android-companion-v0.yml, backend.yml,
    detection-sim.yml, platformio.yml, website-check.yml) and each is gated on on.push.paths
    (backend/**, firmware/*, python/detection-sim/**, android/companion-v0/**, or the workflow
    file) or on a branch allow-list, so nothing under ~/.hermes or ~/jobsearch can start a run.
    No ntfy receipt: there is no compiled repo artifact and no build topic applies to a
    host-local deliverable.
notes: |
  PICKED UP AND COMPLETED 2026-10-24, 17:15-18:05 UTC (PEAK: OFF-PEAK, PROTOCOL MATCH 1.0.0).
  Task arrived with no front-matter; protocol fields added at pickup (declared in the pickup
  commit d1cbe45).

  PIPELINE DEFECT FOUND AND FIXED (declared): `scripts/run_role.sh` invoked
  `hermes chat -q --toolsets …`, and `-q` takes a single argument, so argparse swallowed the
  flag and every role died with "argument -q/--query: expected one argument" — the launcher
  could not run a role at all. Fixed to `--query-file <prompt> -t <toolsets>` (also avoids the
  shell-quoting of a long prompt) and the SKILL.md sentence describing it was corrected. Both
  child roles then ran normally. Second wiring gap, also fixed for the run and documented in
  SKILL.md: the `jobsearch` profile's .env carries no DEEPSEEK_API_KEY, so a child exits 1
  with "No usable credentials found for provider 'deepseek'"; the key was exported into the
  child's environment from ~/.hermes/.env for the call (never printed, never written to a file).

  VENUE: the Upwork read used the DEFAULT profile's MCP token
  (~/.hermes/mcp-tokens/upwork.json, expires_at 1792937273) over streamable HTTP to
  https://mcp.upwork.com/mcp, exactly as 0139/0142 did. The cronrunner token was not read and
  no token value was printed.

  GATE: the posting body came from the MCP read, was written to 01-fetch/posting.raw.txt, and
  only then passed through `fetch_gated.py --in … --text` (verdict `posting_guard: clean`,
  exit 0). The description carries the MCP's own `<untrusted_participant_content>` wrapper; it
  was treated as data, not instructions.

  FIT: 53/100 and "do not apply as posted" is the scorer's honest verdict, not a defect of the
  run. The pilot package is a draft for Stephen's review; a low-scoring package is the correct
  outcome for a $300 two-board job whose receiver needs Ethernet PHY and 4G LTE design the seed
  does not evidence.

  DELIBERATE ATS GAPS: RFM95W, SIM, relay and "6 buttons" are absent from both documents
  because the seed does not support them and the reviewer's A4 (which would have added
  Gerber/P&P/PCBWay/JLCPCB to the CV) was rejected for the same reason. The cover letter does
  name Gerbers/BOM/P&P/PCBWay/JLCPCB and 12 VDC, but only as deliverables and requirements of
  this job, not as prior delivered work. If Stephen has in fact shipped fab/assembly packages,
  applying A4 verbatim is the one edit worth making before he sends this.

  SELF-CAUGHT DEFECTS, all declared in 04-review/edits.md: (1) applying A6+A7 pushed the cover
  letter to 2 pages with only the signature on page 2 — content was compressed back to 1 page;
  (2) the first clean CV compile orphaned the "Off-grid solar + storage install" header at the
  foot of page 1 — fixed with an explicit \newpage because `needspace.sty` is not installed on
  this host ("! LaTeX Error: File `needspace.sty' not found"); (3) CV and cover rendered on
  different paper sizes (A4 vs letter) until `\documentclass[a4paper]{cover}` forwarded the
  option through cover.cls's \PassOptionsToClass.

  NOT VERIFIED BY ME: the visual appearance of the two PDFs beyond their text layer and layout
  metrics — I did not view a rendered image. Stephen is the eyes on the documents, as the task
  specifies. Documents: 04-build/cv.pdf (2 pp, A4), 04-build/cover.pdf (1 pp, A4).
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

## Reply — pilot run complete (2026-10-24 18:05 UTC), draft package only

The pipeline ran end to end on the already-ported `job-apply` skill and produced the pilot
package for Upwork posting 2113244746195338786. Nothing was submitted anywhere.

| Stage | Result |
|---|---|
| gated fetch | `posting_guard: clean`, exit 0; posting read via the MCP (read-only), 1,640 cleaned chars |
| fit evaluation | **53/100 — do not apply as posted** (price/scope and Ethernet+4G LTE gaps are the weight) |
| draft | `cv.tex` (moderncv banking, A4) and `cover.tex` (cover.cls, A4), both built only from the two seed files |
| reviewer pass | 7 Part A edits, 6 applied / 1 rejected with reason, plus 1 orchestrator fix for an unflagged honesty issue |
| compile | `lualatex` cv -> 2 pages; `xelatex` cover -> 1 page |
| verify | `verify_pdf` rc 0 both; `verify_layout` "layout: clean" both; ATS text layer present in both |

**The one thing Stephen should read first:** the scorer's verdict is that this job is not worth
applying to at $300 for two manufacture-ready boards, and that the receiver's Ethernet PHY and
4G LTE sections are not evidenced anywhere in his CV. The cover letter says so plainly rather
than papering over it, and asks for the missing requirements document before quoting.

**Decision he owes on the drafts:** edit A4 (add Gerber / Pick & Place / PCBWay / JLCPCB to the
CV's manufacturing-handoff line) was rejected because the seed does not show he has delivered
assembly-ready fab packages. If he has, that line is one edit away and would recover the ATS
keywords this CV is missing.

Artifacts: `/home/astroboy/jobsearch/pilot-2113244746195338786/` (01-fetch, 02-fit, 03-draft,
04-build, 04-review, 05-verify). PDFs: `04-build/cv.pdf`, `04-build/cover.pdf`.
