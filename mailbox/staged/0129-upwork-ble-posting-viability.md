---
task_id: "0129"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof: []
proof_note: |
  Read-only status check: no branch and no CI run exist for it, and none is
  needed (mailbox/README.md read-only pattern, same as 0044 / 0053 / 0127).
  `proof: []` is real here, not an omission — the deliverable IS this staged
  file, and its evidence is the MCP call log quoted in the reply body below.
notes: |
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-08 19:15 UTC.
  Off-peak (PEAK: OFF-PEAK 19:15 UTC). Protocol MATCH (1.0.0). The filed
  front-matter carried no `protocol_version`; it was added at pickup.

  DONE, READ-ONLY. Posting is ALIVE (not dead). Verdict and every number are
  in the reply body below. Zero writes performed.

  ONE THING THE TASK GOT WRONG / COULD NOT BE CHECKED (declared). The task says
  the proposal "sits at DRAFT". A saved proposal draft is NOT readable through
  this MCP: `list_freelancer_proposals` exposes only the statuses
  Accepted/Offered/Hired/Activated/Pending/Declined/Withdrawn/Archived — there
  is no Draft status and no other tool returns a draft. So the DRAFT's
  existence, its $35/hr terms and its cover text could not be confirmed from
  the API and are taken from the task text. What WAS confirmed: `find_jobs
  action=get` on this job returns `applied: false`, and the newest of the 4
  submitted proposals is dated 2026-09-14 — so nothing has been submitted for
  this posting. The draft's own state is a UI-only object (not claimed here).

  PROPOSAL COUNT IS A TIER, NOT AN EXACT NUMBER (declared). `find_jobs
  action=get` on this job carries no proposal count at all, and `bid_stats_
  available: false`. The count comes from the `find_jobs action=search` row
  for the same job id, which on this account's plan (`Freelancer Basic`)
  returns only the tier `proposals_tier: "20 to 50"`. Reported as a tier; no
  exact number estimated, per the server's own note.
---

# 0129 — ESP32/BLE posting: read-only viability check

## Context

- Upwork posting ~022106710423569691901 ("Firmware with remote control" — ESP32/BLE remote-control pair, Canadian verified client, brand-new account, no rate listed). Stephen drafted a proposal at $35/hr on 2026-10-04; it sits at DRAFT, never submitted. It's now 2026-10-08, day 4.
- Stephen suspects the posting is dead. This is a READ-ONLY viability check before he decides whether to submit or drop it.

## Task

Check the posting's current state: still open or closed/expired? Has the client hired anyone or shortlisted? How many proposals are in? One-line verdict: worth submitting the DRAFT or dead — drop it.

## Success criteria

- Staged reply gives the posting status, hire/proposal counts, and the one-line verdict.
- No write action taken (state this explicitly).

## Constraints

- Read-only. Do not submit, message, bid, or modify anything on Upwork.
- Cost: use your cheapest model tier for this task. Do not use premium/pro models.

## Proof

Staged reply with the posting-state facts above; no branch or CI run needed (same pattern as 0127).

## Reply format

Stage with front-matter noting read-only MCP use. Body: posting status, numbers, verdict, explicit "no writes performed."

---

# Reply — STAGED 2026-10-08, read-only

Account: Stephen Nordlund, Freelancer, org_uid `424335029158658049` (from `list_accounts`).

## Posting state

- **Job** `~022106710423569691901` — "Firmware with remote control" — `workFlowState.status: **ACTIVE**` (research live, **not** closed or expired).
- Published 2026-10-04T11:39:23Z (created 2026-10-04T11:38:00Z) — **4 days old** as of this run.
- Contract: **HOURLY**, experience level **INTERMEDIATE**, engagement **PART_TIME**, `hourlyBudgetType: **NOT_PROVIDED**` (no rate listed — matches the task's description).
- Category: Web, Mobile & Software Dev → Other - Software Development. 20 skill tags (Firmware, ESP32, BLE, PCB Design, …).
- Client: **Canada** (timezone America/Halifax), payment method **verified**.
- Apply path open: `can_apply: true`, `applied: false`, `connects_cost: **9**`, `connects_balance: **11**`.

## Hire / shortlist state — nobody hired, but the client is already interviewing

`activityStat.jobActivity` (this posting's own hiring funnel):

| Metric | Value |
|---|---|
| `totalHired` | **0** |
| `totalOffered` | **0** |
| `invitesSent` | **2** |
| `totalInvitedToInterview` | **2** |
| `totalUnansweredInvites` | 0 |

So: **no hire, no offer** — but the client has **sent 2 invites and both invitees are already at interview stage**. The client is actively working their own pipeline.

Client record (`client_record`, read live for this job):

| Metric | Value |
|---|---|
| `jobs_posted` | **1** (this posting is the client's only job) |
| `jobs_with_hires` | **0** |
| `contracts_total` / `contracts_active` | 0 / 0 |
| `hire_rate_percent` | **0** |
| `feedback_count` / `feedback_score` | 0 / 0 |
| `hours_total` | 0 |
| `open_jobs` | 1 |

Brand-new client: **has never hired anyone on Upwork.**

## Proposal count

**`proposals_tier: "20 to 50"`** — that is the whole answer this account can get.

Why it is a tier and not a number: this is a **Freelancer Basic** account, so Upwork returns only the proposal tier; `find_jobs action=get` on the posting carries no proposal count field at all, and `bid_stats_available: false` with the server's note that competing-bid data is a Freelancer Plus feature. **No exact count is estimated** here (per the server's own instruction: never estimate the competing applicants' numbers).

## The DRAFT (could not be read — declared)

- `find_jobs action=get` → `applied: **false**`: no proposal has been submitted for this posting.
- `list_freelancer_proposals list` (newest first, `Accepted`) → **4** proposals, newest **2026-09-14**; nothing from 2026-10-04.
- **A saved draft is not exposed by this MCP.** `list_freelancer_proposals` supports only Accepted / Offered / Hired / Activated / Pending / Declined / Withdrawn / Archived — there is no `Draft` status, and no other tool returns one. So the draft's continued existence, its $35/hr terms and its cover text are **taken from the task text, not verified here**. Nothing in this reply depends on the draft's contents.

## One-line verdict

**Alive, not dead — but drop it:** the posting is ACTIVE and the client is interviewing, yet it is a brand-new client who has **never hired anyone** (`jobs_with_hires: 0`, hire rate 0%), it lists **no rate**, it already sits at **20–50 proposals** after 4 days with **2 invitees already in interview**, and submitting costs **9 of Stephen's 11 remaining Connects** — a bad price for those odds.

## MCP calls made (the read path — this is the proof)

All via `mcp__upwork__*` in this session, all read-only:

| # | Tool | Action / params | Read-only result |
|---|---|---|---|
| 1 | `list_accounts` | `{}` | Stephen Nordlund, TALENT, org_uid `424335029158658049` |
| 2 | `find_jobs` | `get id=~022106710423569691901` | ACTIVE, hourly/no rate, `applied:false`, `can_apply:true`, connects 9, client_record + jobActivity |
| 3 | `find_jobs` | `search title="remote control"` | the same job id with `proposals_tier:"20 to 50"`, published 2026-10-04 |
| 4 | `get_freelancer_dashboard` | `check` | messages 0 rooms, offers 0, 0 contracts, connects balance 11 |
| 5 | `find_saved_jobs` | `list` | 3 saved jobs — **this posting is not among them** |
| 6 | `list_freelancer_proposals` | `list` (default Accepted) | 4 submitted, newest 2026-09-14 (no draft status exists) |
| 7 | `get_tool_help` | `find_jobs`, `list_freelancer_proposals` | confirmed param/status vocabularies used above |

## No writes performed

Every call above is a read (`list_accounts`, `find_jobs` actions `get`/`search`, `get_freelancer_dashboard` action `check`, `find_saved_jobs` action `list`, `list_freelancer_proposals` action `list`, `get_tool_help`). No `manage_proposals`, `send_message`, `update_profile`, `save_job`, `boost_profile`, `respond_to_offer` or `submit_milestones` was invoked. Nothing was submitted, edited, withdrawn, messaged, saved or un-saved, and **no Connects were spent** (balance unchanged at 11).
