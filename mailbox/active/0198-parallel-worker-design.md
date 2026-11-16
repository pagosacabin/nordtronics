---
task_id: "0198"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 12h
---

# 0198 — Design: cutting mailbox queue latency — drain mode, parallel pickup, or both

## Assignment
- **Hermes**: the design and the scratch-clone proof. This is your machinery — you built the worker, and your 0193 report is the evidence base.
- **Newton:** not used — no build. **Copilot:** not used for the design itself; if the design later advances to a protocol PR, Copilot reviews that PR under the standing rule.
- **Juno:** reviews the design against 0193's findings and against her own outbox-watch/staleness tooling (which reads mailbox state and must keep working), then puts the decision to Stephen. **Nothing in this task implements anything** — design + proof only; implementation is a separate task on Stephen's word.

## Context
Established by 0193 (verified, archived 2026-11-16), restated so this task stands alone:
- The mailbox worker (cronrunner job `c0be50a686c6`, Mon–Thu, fires at :15 past hours 4–17 and 21–22) is **one prompt-driven agent session per fire**. Its selector: resume an in_progress task in `active/` first, else take the lowest-numbered inbox task, **"Work ONE task per run,"** iteration cap 5. The scheduler refuses a second concurrent fire of the same job.
- State transitions (inbox → active → staged → archive) are `git mv` + commit + push on `main` in the shared nordtronics checkout. The task *work* can already be isolated: per-task worktrees are proven (`git worktree add .worktrees/NNNN -b hermes/NNNN-<slug> origin/main` + per-task workdir), and `delegation.worktree_isolation: true` is on for subagents.
- Your 0193 verdict on parallel pickup: what breaks first is **the mailbox itself** — two workers' `git mv` + commit + push on main collide (non-fast-forward, double-moves, resurrections); there is no claim/lock protocol. Then the shared checkout, then model contention (minor).
- Parallelism already exists *inside* a task: Newton builds (Herdr pane t19, separate Antigravity process), you supervise, Copilot reviews (single pane t17).

Why Stephen wants this (2026-11-16, his words in substance): with three tasks queued, serial pickup meant a 3–4 hour wait for work that totals well under an hour — hourly fires, one per run, and a long run silently eats the next slot. He is often out of reach of the laptop; the queue should drain itself without him prodding via Herdr. He asked for the parallel mechanism to be designed. Juno's framing note: his underlying want is **latency**, and the honest design compares the cheap fix against the real one.

## Task
1. **Design three options, in writing, in the staged reply:**
   - **Option A — Drain mode (serial, no hourly gaps).** One worker session keeps claiming the next task until the inbox is empty (or a stated cap: max tasks per session, max wall-clock), instead of one task per fire. Specify: what changes in the selector and job (exact prompt diff in substance), how the scheduler's no-concurrent-fire rule interacts, what happens when a task in the drain hangs or hits its iteration cap, and how Juno's staleness/watch tooling sees it (unchanged state shapes?).
   - **Option B — True parallel pickup.** N≥2 tasks in flight at once. Specify the full mechanism: (a) **claim** — how a worker atomically takes a task so two workers can never take the same one (e.g., the transition push itself as the claim, a claim file, the GitHub API — evaluate what fits the existing git-only tooling), including recovery when a worker dies holding a claim; (b) **transition serialization** — how inbox/active/staged/archive moves stop colliding (a lock, a single serialized transition helper with pull-retry, API-based moves — your call, justified), mapped against each 0193 failure mode: non-fast-forward, double-move, resurrection; (c) **work isolation** — per-task worktrees as proven, and what still must never run concurrently (same-repo builds? tasks that both need the Copilot pane? Newton quota pools — parallel builds burn the Gemini pool ~2× as fast; state the routing consequence); (d) **concurrency cap** — propose a number and the reasoning (DeepSeek rate limits as you know them, pane capacity, quota burn).
   - **Option C — Hybrid.** Drain mode as the default, with parallelism only where it is provably safe (name the class: e.g., host-local/report tasks in parallel, repo builds serial — or your better cut). Specify the classification rule in one sentence a worker can apply mechanically.
2. **Compare and recommend.** A short table: option × (latency bought for a 3-small-task queue, complexity added, new failure modes, quota/cost effect). Then **one recommendation**, with the reasoning — including when the honest answer is "A buys 90% of it; B isn't worth it yet," if that's what the evidence says. Stephen decides; your job is a decision he can make from the reply alone.
3. **Prove the riskiest mechanism on a scratch clone.** Whichever option you recommend, take its riskiest moving part and prove it in a **throwaway clone of the nordtronics repo** (scratch directory, never the live checkout, never pushed to origin): for B or C, run two concurrent worker-style sessions claiming two different synthetic tasks and performing transitions through your proposed mechanism, and show the claim race resolves to exactly one winner per task and the final state is correct for both; for A, show one session draining two synthetic tasks back-to-back with correct transitions and a clean stop at the cap. Synthetic tasks = trivial file edits in disjoint paths, created by you in the scratch clone. Quote the commands and outputs. Delete the scratch clone when done and say so.
4. **Name the follow-on implementation exactly.** If Stephen approves the recommended option, what precisely changes: the worker job prompt diff (quoted), any new helper script (name, location, one-line purpose), the protocol README version bump and the sections it touches, and what Juno must update on her side (watch/staleness assumptions) — or "nothing" if state shapes are unchanged. This section is the implementation task in embryo; Juno will file it from your text on Stephen's word.

## Success criteria (falsifiable)
- All three options are specified at the level above — mechanisms named, not gestured at — and each 0193 failure mode (non-fast-forward, double-move, resurrection, eaten slot) is addressed by name in the option(s) it applies to.
- The comparison table and a single recommendation are present, with the quota-burn consequence of parallel Newton work stated in numbers or plainly as unknown-with-reason.
- The scratch proof's raw outputs show the recommended option's riskiest mechanism working (exactly one claim winner per task / clean drain), in a scratch clone that was never pushed and is deleted after.
- No live state changed in this task (see constraints) — the reply's own staged file is the only mailbox write.

## Constraints
- **Design + scratch proof only.** Do not change the live worker job, its prompt, its schedule, the protocol README, or any live mailbox state. Do not pick up or alter the queued tasks (0195, 0196-bundled) in this task.
- All proof work in a scratch clone under a scratch directory; never push scratch branches/commits to origin; delete the clone afterward.
- Do not use the live Copilot or Newton panes for the proof — synthetic tasks are shell/git work you do yourself.
- One run. If the proof fails or the design hits a genuine unknown (e.g., a DeepSeek rate limit you cannot establish), report it as a finding with the evidence — do not iterate silently past the cap, and do not paper over it.
- Dependency note: 0196 (queued ahead of you) commits `.worktrees/` to both repos' `.gitignore`. If it has landed by the time you run, say so; if not, note where your design assumes it.
- **Cost:** standard tier (DeepSeek Flash) for the design; the proof is shell/git. No Newton session, no model escalation. State token/cost in the reply's cost block as usual, or declare it unknown with the reason.

## Proof
Design task: the deliverable is the staged reply itself (design + comparison + recommendation + implementation sketch) plus the quoted scratch-clone outputs. `branch: none` / `run: none` is the honest state for the proof clone — it is local and deleted; the quoted command/output transcript is the record, and Juno checks the design against 0193's archived findings line by line.

## Reply format
One staged reply:
- Recommendation at the top, one paragraph
- Option A / Option B / Option C — mechanism specs
- Comparison table + recommendation reasoning
- Scratch proof: commands and outputs, clone deleted confirmation
- Follow-on implementation, exactly as it would be filed
- Cost block
