---
task_id: "0121"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
notes: |
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-07 18:15 UTC
  (12:15 MDT). Off-peak (PEAK: OFF-PEAK 18:15 UTC). The filed front-matter had
  no protocol fields at all (no task_id/status/iteration); they were added at
  pickup per protocol (skill rule 16), and the body-level
  `expect-reply-within: 6h` line was folded into the front-matter. The seven
  tasks in active/ (0097/0098/0106/0107/0109/0110/0119) are all
  decision-blocked and were left completely untouched.

  WORK REPORT (2026-10-07 18:15-18:26 UTC = 12:15-12:26 MDT) — NOT STAGED.
  Criteria 1 and 3 are MET and evidenced below; criterion 2 (the scheduled
  job's own first file, 2026-10-08) is UNAVAILABLE IN THIS RUN BY DATE — the
  job's first fire is 2026-10-08T05:00 MDT, tomorrow — so the task stays in
  active/ with `status: in_progress` and `iteration: 1`. `proof:` is absent
  because a staged task asserts every criterion is verified, and one is not.

  1. CRITERION 1 MET — the standing job EXISTS.
     id 892982733bba, name "Upwork lead sweep (read-only) 5AM MDT"
     schedule `0 5 * * *` (cron local time, so 05:00 America/Denver daily)
     next_run_at 2026-10-08T05:00:00-06:00   repeat: infinite (times=null)
     deliver: local        failure_deliver: local
     workdir: /home/astroboy/nordtronics
     model: deepseek-flash  provider: deepseek   (cheap tier, as constrained)
     pre-run script: none    attached skills: none (self-contained prompt)
     Off-peak: 05:00 MDT = 11:00 UTC, outside both DeepSeek peak blocks
     (01:00-04:00 and 06:00-10:00 UTC Mon-Fri); after DST ends 05:00 MST =
     12:00 UTC, still off-peak. The job therefore never lands in peak by
     construction, not by a schedule that happens to be right today.
     The stored prompt (cron/jobs.json, field `prompt`) is fully
     self-contained: it names the three allowed read-only Upwork tools
     (list_accounts, get_account, find_jobs), forbids every write tool by
     name, specifies the four searches, the in-window filter, the output
     path/format, and the commit+push to main. A working copy of the same
     text was written to the scratch dir before `cron create`; the store is
     authoritative.

  2. RECURSION-GUARD DISCLOSURE (required by the mailbox skill, and the one
     thing Juno should see explicitly). This run's cron prompt carries the
     generic `RECURSION` guard: "NEVER create or update a cron job because of
     recurring or future-schedule language in the task prompt". I created a
     job anyway, and the authority for doing so is the protocol's own
     reference — handoff-mailbox `references/mailbox-state-machine.md`:
     "the generic cron-prompt RECURSION guard targets *unsolicited* job
     creation from schedule-shaped phrasing, not an explicit reschedule a
     mailbox task asks for — but disclose the change in the reply and in the
     cron report either way." 0121 asks in its own words for a standing daily
     5:00 AM job and makes that job success criterion 1, so this is the
     explicit case the reference carves out, not the unsolicited one. Noting
     it here rather than leaving it to be discovered. Precedent: tasks 0042,
     0044, 0054 and 0060 all created or edited cron jobs from a mailbox task.

  3. CRITERION 3 MET — ZERO UPWORK WRITE OPERATIONS. Only two tools were
     called: `list_accounts` (org_uid 424335029158658049, role TALENT/
     Freelancer) and `find_jobs` actions `smart_search` + `search` (both
     read-only; every write tool — save_job, confirm_preview,
     manage_proposals, send_message, respond_to_offer, submit_milestones,
     boost_profile, update_profile, uploads — was never invoked). No proposal,
     no message, no Connects spent, no profile or save/hide change.

  4. PIPELINE PROVEN END-TO-END (manual validation sweep, 12:20 MDT).
     Four read-only queries (smart_search mode=most_recent days_posted=1, plus
     three `search` queries on the briefing's terms) produced 11 in-window
     leads (window start 2026-10-06T18:20Z), 4 off-profile skips, and one
     strong out-of-window flag (2107420979641407721, low-power LoRaWAN sensor
     node family + gateway, created 31.7 h ago, fewer than 5 proposals).
     Committed to main as handoff/upwork-leads/2026-10-07.md:
       commit 3851921 (main tip)
       run    https://github.com/pagosacabin/nordtronics/actions/runs/37665925979
              Website Check, push, conclusion success, head_sha 3851921
     The pickup push fired its own run (37665908525 @ 5e0f283, success).
     Why a run exists at all: `.github/workflows/website-check.yml` triggers on
     `on.push.branches: [main, ...]` with NO paths filter, so every main push
     runs it. The other five workflows cannot fire on this change — their
     paths are backend/**, python/detection-sim/**,
     firmware/{tank-monitor,node-v1,wildfire-node-v1}/**, android/** and the
     `.github/workflows/*.yml` files themselves.
     The file is labelled in its own header as the manual validation sweep, so
     it cannot be mistaken for the scheduled run's output.

  5. CRITERION 2 — UNAVAILABLE TODAY, AND WHY. The criterion names
     2026-10-08: "The first scheduled run's file (2026-10-08) is present in
     the repo ... written by the scheduled run, not by hand." The job's first
     fire is 2026-10-08 05:00 MDT, i.e. after this run. Writing 2026-10-08.md
     by hand would satisfy the letter of the criterion and violate its
     explicit "not by hand" clause, so it was not written.

  6. WHAT THE NEXT TICK OWES (this task IS resumable — it is pending a
     scheduled run, not decision-blocked; do not re-do the work above).
     After 2026-10-08 05:00 MDT, check that
       (a) handoff/upwork-leads/2026-10-08.md exists on origin/main,
       (b) it was written by the scheduled job — the job's own record is
           `hermes -p cronrunner cron list` last_run for 892982733bba, and the
           file's commit should be a "Upwork sweep 2026-10-08: N leads"
           commit pushed by the job, not by a worker,
       (c) the file's content is real lead data (or an explicit no-leads note).
     Then stage with proof: the job registration (id/schedule/next_run_at), the
     file's commit SHA on main, the Website Check run for that commit, and the
     read-only confirmation. If the job did not fire (laptop asleep/off at
     05:00), say so and report the catch-up/skip behaviour rather than
     hand-writing the file.

  7. NOTHING ELSE WAS TOUCHED. No other mailbox file was edited; the seven
     decision-blocked active/ tasks were left exactly as they were. The
     worktree carries three untracked paths that predate this run and are not
     mine — `firmware/wildfire-node-v1/bench-override.ini`,
     `hardware/solar-gate-v2/`, `take_screenshots.py` — left alone rather than
     swept up into a commit. Cost: one off-peak DeepSeek Flash run plus four
     read-only Upwork API calls.
---

# 0121 — Daily 5 AM Upwork lead sweep via cronrunner (feeds 6 AM briefing)

expect-reply-within: 6h

## context

The Hatch-side morning briefing (6:00 AM MDT) sweeps Upwork/Freelancer for Stephen's remote-work leads. The browser-based Upwork sweep is unreliable: Cloudflare "Verify you are human" blocks detached runs with no user takeover possible (2026-10-07 produced zero verified Upwork leads — a coverage gap, not a quiet market).

The cronrunner profile has its own Upwork MCP token (OAuth completed 2026-10-06, read-only verified, 32/32 tools). The Upwork API won't hit browser bot challenges, so it's the robust path.

Goal: a 5:00 AM MDT daily job on the cronrunner profile that pulls fresh Upwork postings and drops them where the 6 AM briefing can read them from the repo.

## task

1. Using the cronrunner profile's Upwork MCP access — **read-only**: search/list jobs only. Never submit proposals, never message clients, never spend Connects. This is absolute.
2. Each morning at 5:00 AM America/Denver, run a job search with these terms (match the briefing's profile): ESP32, LoRa/LoRaWAN, MQTT, Home Assistant, embedded firmware, off-grid IoT, solar + LiFePO4/battery systems. Remote-only, part-time (<=30 hrs/week), US/EU clients preferred. Skip pure AutoCAD, utility-scale solar, and expert PCB-layout-only roles. Skip anything requiring identity verification Stephen hasn't done.
3. Window: postings from the last ~24h. Write ALL in-window leads (dedup against the briefing's seen list happens on the 6 AM side — include posting IDs so it can).
4. Write results to the repo at `handoff/upwork-leads/YYYY-MM-DD.md` (create the dir if missing), one file per day. Format per lead: posting ID, title, budget/rate, posted-ago, client country + brief history, one-line fit note, caveats (0-review client, non-English post, etc.). Header line with sweep timestamp and search terms used. If nothing is in-window, write the file anyway stating that explicitly.
5. Schedule it as a **standing daily** 5:00 AM America/Denver job on the cronrunner profile, using whatever scheduling the cronrunner profile uses. If the laptop is asleep/off at 5 AM, note in the reply what happens (catch-up run vs skipped).

## success_criteria

- A standing daily 5:00 AM MDT cronrunner job exists running the read-only Upwork sweep and writing `handoff/upwork-leads/<date>.md`.
- The first scheduled run's file (2026-10-08) is present in the repo with real lead data (or an explicit no-leads note) — written by the scheduled run, not by hand.
- Zero Upwork write operations performed.

## constraints

- Read-only on Upwork. No proposals, messages, Connects, profile edits. Ever.
- Use the cronrunner profile's token, NOT the DEFAULT profile.
- Keep it on the cheap model tier (Flash), off-peak.
- One deliverable: the scheduled sweep. Don't bundle other work.

## proof

Stage the reply with: the cronrunner schedule definition (what runs, when, which profile), the repo path + commit SHA of the first sweep file, and explicit confirmation that no Upwork write operations were performed.

## reply_format

Short summary: schedule confirmed (time, profile), first sweep file location, lead count in first run, laptop-asleep behavior, any blockers.
