---
task_id: "0247"
title: Make run_as_goal a standard — protocol bump, worker behaviour, live host-only proof
status: verified
iteration: 2
expect-reply-within: 24h
proof:
  - branch: hermes/0247-run-as-goal-standard
    sha: 318da84bae375394968a3a2ce055c82a4ccfa4a1
  - gate_log: mailbox/gate-logs/0248-t2-1-unittest.log
    turn: 2
    exit: 0
  - run: none
    reason: no workflow in .github/workflows/ triggers on a mailbox/README.md change on this branch (enumeration in notes)
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
notes: |
  STAGED. README bumped to protocol_version 1.4.0 on origin/main (merged fast-forward at
  318da84), skill patched to 2.6.0 with the claim-time behaviour, worker prompt paragraph
  added to all four worker jobs (next_run_at unchanged on each), Copilot peer review three
  passes to SHIP, and the live proof ran: 0248 was claimed by the standard helper, its goal
  card t_4b8e59ff (goal_mode=1, max_turns=8) was spawned by the gateway dispatcher, ran 27 s
  with exit 0, and the gate failed before (rc=1) and passed after (rc=0); 0248 staged at
  0c9be52 with its gate logs at 379eeba.

  DEVIATIONS AND OPEN ITEMS, all declared in the reply body:
  (a) a DUPLICATE TASK ID — an interactive session filed mailbox/active/0247-live-recovery.md
  at 12:09 MST into this same worktree (commit 318da84, author hermes-cronrunner, self-filed
  per its own notes). Two tasks now carry id 0247; I did not renumber or edit the other file,
  and it rode into main on the fast-forward merge because it sat on my branch tip.
  (b) the transition helper's fetch-reset discarded two unpushed local commits (0248's gate
  logs and a contract-path edit); both were regenerated and are declared inside 0248's staged
  notes.
  (c) the gate-failure -> continuation-card branch and goal.github_contract were NOT exercised
  live (the card succeeded on its first run; the task is host-only).
  (d) cost is unknown: the cron harness exposes no per-call usage objects.
  Protocol version recorded in the worker state file with --record after staging.
Juno verification (2026-11-23 12:3x MST): VERIFIED. mailbox/README.md read back from origin
main: protocol_version 1.4.0 (line 3), changelog entry (line 27), run_as_goal in the
front-matter allowlist (line 107), Goal mode section (line 121). Branch
hermes/0247-run-as-goal-standard read back at 318da84bae375394968a3a2ce055c82a4ccfa4a1,
exact match. 0248's lifecycle verified separately and closed by Juno earlier today (gate
logs read from origin; reconstruction deviation recorded there). Copilot three passes to
SHIP, dispositioned in the reply. Deviations (a)-(f) stand as declared, including the
duplicate task id (see the live-recovery archive note) and the helper staging the wrong
0247 file for one commit (72a6027, corrected 80783d4, nothing lost). Open item carried
forward from the reply: the Fri/Sat/Sun worker prompts are an older revision than Mon-Thu
(no transition helper, no drain) - Hermes added the goal paragraph only; reconciling the
drift is a separate task, not filed. Protocol 1.4.0 is the standing standard from here.

---

## Assignment

- **Hermes:** all of it — the README edit, the worker-skill change, the Copilot step, filing and reporting the proof task.
- **Copilot:** peer review of the README diff (GitHub-bound), driven by Hermes via Herdr. Reviewer, not the proof.
- **Newton:** not assigned.
- **Juno:** verifies independently — reads the README back on origin, checks 0248's lifecycle in the mailbox, and closes 0247 on the staged reply.

## Context

0246 (archived, verified) established that goal execution through the standard pickup is not available natively on Hermes v0.23.2: the mailbox worker is a cron job with no goal mode, and the kanban judge loop is unreachable from the pickup and carries no per-turn gate. The gap is a protocol field plus worker wording — not a version update. 0246's reply (section 5) drafted the standard and the README wording; the README was deliberately not edited there, because protocol changes are Stephen's call.

**Stephen has now made that call: option A is approved** (chat, 2026-11-23 11:54 MST). Option B (engine-level gates inside `run_kanban_goal_loop`) is NOT approved and stays a recorded alternative.

## Task

1. **README protocol bump.** Add the `run_as_goal` field to `mailbox/README.md`'s front-matter section, using the wording drafted in 0246's reply, adjusted only for fit with the surrounding text. Bump the protocol version per the README's own convention. Stephen's approval is the authority for this edit — cite it in the commit message.
2. **Worker behaviour.** Update the cronrunner handoff-mailbox skill (and the worker prompt wording, if that is where it lives) so that on claiming a task with `run_as_goal: true` the worker:
   - creates a kanban card with `--goal` and `--goal-max-turns` from `goal.max_turns`, adding `--completion-contract <goal.github_contract>` when that field is set, with `goal.contract` as the card body;
   - waits for the run to finish;
   - runs every command in `goal.gates` itself; on a non-zero exit, dispatches a continuation card quoting the gate's output tail, up to `max_turns`; on budget exhaustion, parks the task in `active/` with the last gate output;
   - stages the mailbox reply only when the card completed AND every gate exits 0.
   Quote the changed skill lines in the reply.
3. **Copilot peer review** of the README diff; address findings; then push. `git fetch` and read origin before any push.
4. **Live proof.** After steps 1–3 land, file task **0248** in the inbox as a normal task carrying `run_as_goal: true`: a small host-only scratch job whose initial state FAILS its gate — the 0246 section-4 shape (a two-file scratch job, `python3 -m unittest discover` as the gate, the subject or test deliberately broken), with a contract stating the outcome (tests pass) and the boundary (scratch only). Let the standard cycle claim it untouched by hand. Report 0248's full lifecycle in this task's reply: claimed by the tick, goal card run, the gate's failure fed back, the fix, the gate passing, the staged reply. If any link in that chain does not happen, report exactly where it stopped — a failed proof is a finding, not a licence to hand-run it and call it proved.

## Success criteria

- `mailbox/README.md` on origin carries the `run_as_goal` field and the behaviour paragraph (Juno reads it back).
- The cronrunner skill carries the claim-time behaviour, quoted in the reply.
- 0248's lifecycle ran through the untouched standard cycle with gate evidence quoted (fail, then pass) — or the reply names the exact link that failed.
- Copilot's review of the README diff is dispositioned in the reply.

## Constraints

- No engine changes. No v0.23.3 update, or any update. Option B stays parked.
- Tasks WITHOUT `run_as_goal` follow exactly the path they follow today — the default behaviour does not change.
- The proof job is host-only scratch: no hardware, no flashing, nothing changed outside the scratch directory, the mailbox, and the files named above.
- Secrets never in the task files, the proof task, or the reply.
- Model cost: flash tier is fine for this work.

## Proof

- The README as read back from origin after the push.
- The quoted skill lines implementing the claim-time behaviour.
- 0248's mailbox lifecycle (inbox → active → staged) plus the quoted gate outputs.

## Reply format

Status line first. Then per step: what changed, with pointers. Then the proof lifecycle in order. Deviations and failed links declared plainly.

## Reply

**Status: staged.** Protocol 1.4.0 is live on origin/main, the worker carries the claim-time
behaviour, Copilot signed off after three passes, and the standard ran end to end through the
untouched cycle on 0248 — one declared exception: task 0248's id collides with an
interactively filed task (deviation (a) below).

### Step 1 — README protocol bump (README on origin)

Read back from `origin/main` (`git show origin/main:mailbox/README.md`):

```
3:protocol_version: 1.4.0
27:- 1.4.0 — 2026-11-23: goal mode. Optional front-matter `run_as_goal: true` plus a
107:- `run_as_goal` (optional, boolean, default `false`): execute this task as a goal.
110:- `goal` (required when `run_as_goal: true`): the goal block —
121:## Goal mode (`run_as_goal`)
164:`mailbox/gate-logs/NNNN-t<turn>-<index>-<slug>.log`
216:- For a `run_as_goal` task, gate logs are additional `proof` pointers with their own canonical
```

What changed, in `mailbox/README.md` (+86/−1 on the branch):
- `protocol_version` 1.3.0 → **1.4.0** (MINOR: new field + rule, no state change), with a
  changelog entry citing Stephen's approval.
- Front-matter allowlist gains `run_as_goal` and `goal` (the README calls the allowlist
  closed, so the bump is the mechanism).
- New section **Goal mode (`run_as_goal`)**: field validation (`max_turns` > 0, non-empty
  `contract`, non-empty `gates`, optional `OWNER/REPO` `github_contract`), the five
  claim-time steps, gate evidence, and precedence/failure modes.
- **Gate evidence**: every gate run writes
  `mailbox/gate-logs/NNNN-t<turn>-<index>-<slug>.log` (command, cwd, workspace HEAD, exit,
  full output); green logs ride the staging commit, failing logs ride the continuation or
  park commit, and turns never overwrite each other.
- **Proof format** gains a canonical `gate_log: {gate_log, turn, exit}` entry resolved at the
  staging commit.
- Precedence made explicit: budget exhaustion **parks** (blocked) and can never also produce
  a `status: failed` staging or a second `iteration` bump; `run_as_goal: false` or absent is
  exactly the 1.3.0 path, and an orphan `goal:` block is ignored and noted.

Commit message cites the authority: *Protocol 1.4.0: run_as_goal standard (Stephen approved
2026-11-23 11:54 MST, task 0247)*.

### Step 2 — worker behaviour (skill + prompt)

`~/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md` — version 2.5.0 →
**2.6.0**, new section quoted:

> **## Goal mode (`run_as_goal`) — claim-time behaviour (protocol 1.4.0)**
> A task with `run_as_goal: true` in its front-matter is executed as a goal, and the staging
> boundary is hardened: the worker stages the reply only when the goal card completes **and**
> every gate exits 0. On claiming such a task … do all of this inside the same run:
> 1. Compose the card body from `goal.contract` verbatim and create the card:
>    `hermes kanban create "<task title>" --body-file <contract-file> --assignee cronrunner
>    --goal --goal-max-turns <goal.max_turns> [--completion-contract <goal.github_contract>]`
>    — `--completion-contract` is passed **only** when `goal.github_contract` is set.
> 2. Wait for the run to finish (`hermes kanban runs <id>` / `hermes kanban show <id>`); the
>    gateway dispatcher ticks about every 60 s, so wait in bounded polls.
> 3. Run every command in `goal.gates` **yourself** … Judge each by exit code only.
>    Non-zero → dispatch a continuation card quoting that tail, up to `goal.max_turns` turns;
>    budget exhausted → park in `active/` with `status: in_progress`, last gate output in
>    `notes`, no `proof`, sentinel deleted. That is *blocked*, not failed.
> 4. Stage the reply **only** when the card completed and the final full gate pass exited 0.
>    Re-run the whole ordered gate list from scratch after every completed turn … write each
>    run's command, cwd, workspace HEAD, exit status and full output to
>    `mailbox/gate-logs/NNNN-t<turn>-<index>-<slug>.log`, committed in the staging commit and
>    cited in `proof` as a `gate_log` entry. Commit the failing runs too … A stage with no
>    green gate log for the current turn is a fabricated pass.

The prompt is where the per-tick wording lives, so the same paragraph was added to all four
worker jobs (`hermes … cron update_job`, key `prompt`), anchored before `## Step 3`:

| job | prompt length | `next_run_at` |
|---|---|---|
| `c0be50a686c6` Mon-Thu | 8189 → 9588 | 2026-11-23T12:15:00-07:00 (unchanged) |
| `7aff6948c2c1` Fri | 6569 → 7968 | 2026-11-27T00:00:00-07:00 (unchanged) |
| `8b1c9e1323c5` Sat | 6569 → 7968 | 2026-11-28T00:00:00-07:00 (unchanged) |
| `5c1532977f15` Sun | 6569 → 7968 | 2026-11-29T00:00:00-07:00 (unchanged) |

Tasks without the field are untouched: the paragraph ends "A task WITHOUT `run_as_goal`
follows exactly the path it follows today — the default behaviour does not change."

Observation while editing: the Fri/Sat/Sun prompts are a *different, older* revision (6569
chars, no transition helper, no drain, no protocol step) than Mon-Thu (8189). I only added the
goal paragraph to them and did not reconcile the drift — that is a separate task.

### Step 3 — Copilot peer review (pane t17, `copilot-main`)

Three passes on the exact `git diff main...hermes/0247-run-as-goal-standard -- mailbox/README.md`:

1. **Verdict: FIX**, 8 findings — missing stage-bar statement; gate evidence not durable;
   gates not re-run after a continuation; continuation-card content undefined; park-vs-
   iteration-cap precedence; no schema validation; card-cancellation/spawn-failure modes;
   `run_as_goal: false` vs an orphan `goal` block.
2. **Verdict: FIX**, 2 remaining — gate-log filenames lacked a turn/attempt component and park
   commits did not commit the failing log; the `proof` schema had no `gate_log` entry shape.
3. **"No remaining findings" — Verdict: SHIP.**

All ten were addressed in the README text itself (gate-log naming with the turn, failing logs
committed, canonical `gate_log` proof entry, validation, precedence, failure modes), not just
argued.

### Step 4 — the live proof (0248)

Filed by me at `ca03fb2` as a normal inbox task carrying `run_as_goal: true`; from there it was
**untouched by hand**:

1. **Claimed by the standard helper** — `mailbox-transition.py --mode claim --task 0248`,
   `status: in_progress`, `iteration: 1`, sentinel created (`aff3a97`).
2. **Turn-1 gate, run by the worker:** exit 1, 2 failures
   (`'wildfire/bench-01/telemetry' != 'nordtronics/wildfire/bench-01/telemetry'`). Log:
   `mailbox/gate-logs/0248-t1-1-unittest.log`.
3. **Card created:** `t_4b8e59ff`, `--goal --goal-max-turns 8`; read back from the board DB
   `goal_mode=1`, `goal_max_turns=8`, completion contract `local-only`.
4. **Spawned for real:** events `created 12:12` → `claimed {lock: home:152866}` →
   `spawned {pid: 918374}` → `worker_bound {pid: 918416}`. Contrast the stale 0246 probe card
   `t_3c3bc243`, still `respawn_guarded {'reason': 'blocker_auth'}` every 60 s — our card was
   not guarded, it ran.
5. **Run #2 completed:** 27 s, `exit_code: 0`, summary: added the missing `nordtronics/`
   prefix in `pkg/topic.py`.
6. **Turn-2 gate, run by the worker:** exit 0, `Ran 2 tests … OK`. Log:
   `mailbox/gate-logs/0248-t2-1-unittest.log`.
7. **The fix came from the card, not from weakening the test:**
   `tests/test_topic.py` sha256 `3516a476…b472` is identical to filing; `pkg/topic.py` changed
   exactly one line.
8. **Staged** at `0c9be52` (sentinel deleted in the same commit), gate logs at `379eeba`.

### Deviations and failed links, plainly

- **(a) Duplicate task id — needs a decision.** At 12:09 MST, while this task was in progress,
  an interactive session filed `mailbox/active/0247-live-recovery.md` in this same worktree
  (commit `318da84`, author `hermes-cronrunner`, "self-filed" by its own notes). Two tasks now
  carry id 0247. I did not renumber, edit or stage that file; it reached `main` because it sat
  on the branch tip I fast-forwarded. Juno/Stephen should renumber it (0249 is free) or tell me
  to. Its task id also collides with 0247's branch namespace if it later pushes
  `hermes/0247-live-recovery`.
- **(b) Helper fetch-reset lost two unpushed commits.** `mailbox-transition.py --mode stage`
  resets `main` to `origin/main`, so the not-yet-pushed 0248 gate logs and a contract-path edit
  were discarded. Both were regenerated (the t1 log reproduces the pre-fix state in a copy
  whose `pkg/topic.py` hashes to `f8b4405…`, byte-identical to the seeded defect) and the
  recovery is declared inside 0248's staged notes. Lesson recorded for the skill.
- **(c) Not exercised live:** the gate-failure → continuation-card branch (the card succeeded
  on its first run, so `max_turns: 8` was never consumed) and `goal.github_contract` /
  `--completion-contract` (host-only task, no CI gate). Both are specified and reviewed, not
  proven.
- **(d) No CI run, by enumeration, not by assertion.** `run: none` is earned:
  `android-build.yml` pushes only on `android-toolchain-setup`; `android-companion-v0.yml` on
  `android/companion-v0/**`; `backend.yml` on `backend/**`; `detection-sim.yml` on
  `python/detection-sim/**`; `platformio.yml` on `firmware/tank-monitor/**` and
  `firmware/node-v1/**`; `website-check.yml` on pushes to `[main, hermes/0068…, hermes/0070…,
  hermes/0093…]` and PRs to main. A `mailbox/README.md`-only change on this branch matches
  none of them, and `gh run list --branch hermes/0247-run-as-goal-standard -R
  pagosacabin/nordtronics` returned empty.
- **(e) Cost unknown** — the cron harness exposes no per-call usage objects; stated, not
  estimated.
- **(f) The first staging commit staged the wrong file, and it is corrected on origin.** With two
  tasks carrying id 0247, `mailbox-transition.py --mode stage --task 0247` matched
  `mailbox/active/0247-live-recovery.md` (the other task) by numeric prefix, moved it to
  `staged/` and overwrote it with this reply. Commit `72a6027` did that; `80783d4` restored
  `0247-live-recovery.md` to `active/` byte-identically (from `318da84`) and staged *this* file
  by hand. Nothing was lost, but origin carried a wrong state for one commit — and it is the
  duplicate id, not the helper alone, that made the selection ambiguous.

- **Skill edit is host-side, not in the repo.** The skill lives in the cronrunner profile, so
  its "proof" is the quoted lines above plus the file itself; no branch pointer can cover it.

### Protocol state

`mailbox-protocol-check.py --record` was run after this staging so the next tick reads
`PROTOCOL: MATCH`. The pre-run gate had already flipped to MISMATCH as soon as 1.4.0 landed on
origin — recording here is what closes that window.
