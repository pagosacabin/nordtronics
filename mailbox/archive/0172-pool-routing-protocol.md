---
task_id: "0172"
protocol_version: 1.1.0
status: archived
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0172-pool-routing
    sha: 503e619219720efdfa45b8459a0948514128831c
  - run: none
    reason: >
      No workflow triggers on hermes/0172-pool-routing and no path filter covers
      mailbox/**; `gh run list --branch hermes/0172-pool-routing` is empty. The six
      workflows and their triggers are enumerated in notes (c).
  - files:
      - mailbox/README.md
cost:
  model: deepseek-flash
  api_calls: 31
  input_tokens: 2017383
  input_cache_hit_tokens: 1985664
  input_cache_miss_tokens: 31719
  output_tokens: 15108
  total_tokens: 2032491
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.019780
notes: |
  DONE — protocol 1.2.0 (model pool routing) on branch `hermes/0172-pool-routing`
  @ 503e619, one file changed (`mailbox/README.md`, +54/-1), NO merge to main.
  Run 2026-11-07 17:15-17:2x MST (2026-11-08 00:15-00:2x UTC), off-peak
  (PEAK: OFF-PEAK 00:15 UTC). PROTOCOL at start: MISMATCH local=1.0.0 remote=1.1.0;
  this task is the protocol-update task in inbox, so it was done before 0171.

  WHY THE MISMATCH LOOKED STALE. The 1.1.0 bump (0155) is already live on main
  (mailbox/README.md `protocol_version: 1.1.0`, archived task 0155) but the worker
  state file still said 1.0.0 — the `--record` step was never run for 0155. I did
  NOT patch anything for 1.1.0 as if it were missing (the README already carries
  it); after staging I ran `mailbox-protocol-check.py --record`, which stores the
  remote (main) version 1.1.0. The 1.2.0 bump exists only on the branch, as the
  task requires, so main stays at 1.1.0 until Stephen approves the merge.

  (a) README PATCH — `mailbox/README.md` is the ONLY changed file on the branch:
    protocol_version 1.1.0 -> 1.2.0; a 1.2.0 changelog entry; a new
    `## Model pool routing` section (placed after `## Cost format`, before
    `## Rules`) carrying every rule from the task: order of pools, model roles,
    coding reserve, read-both-gauges, quota stop, availability, headless, record
    switches, and the 20%-first-guess caveat. The 1.1.0 cost format section was
    not reworded — PROVEN byte-identical, not eyeballed:
      git show main:mailbox/README.md | awk '/^## Cost format/{f=1} f&&/^## Rules/{exit} f' | sha256sum
      awk '/^## Cost format/{f=1} f&&/^## Model pool routing/{exit} f' mailbox/README.md | sha256sum
      -> 7b17019036ab76ea911c58d070eaf28a22d46ee6068fce46cb728e75f9596558 (both)

  (b) SKILL PATCH IN LOCKSTEP — the handoff-mailbox skill
  (~/.hermes/profiles/cronrunner/skills/skills/handoff-mailbox/SKILL.md) now
  carries the 1.1.0 cost format AND the 1.2.0 routing section, both copied
  verbatim from the README (asserted by substring equality, not by hand). The
  whole patched block is quoted in the reply section below. Two structural fixes
  were needed and are declared: the block had no END marker (a second
  `<!-- BEGIN PROTOCOL-MANAGED` sat where the close belonged) and the 1.1.0 cost
  format had never been added — both are corrected, and the legacy
  `## Quick Start` tail is byte-identical to before the edit (checked by
  suffix comparison against a pre-edit copy). Skill version bumped 2.5.0 -> 2.6.0.

  (c) CI — `run: none`, EARNED by enumeration. `.github/workflows/` holds six
  files: android-build.yml (branches: [android-toolchain-setup]),
  android-companion-v0.yml (branches: nine named hermes/00xx branches + paths
  android/companion-v0/**), backend.yml (paths backend/**, workflow itself),
  detection-sim.yml (paths python/detection-sim/**, PRs to main),
  platformio.yml (paths firmware/tank-monitor/**, firmware/node-v1/**,
  firmware/wildfire-node-v1/**, python/detection-sim/**), website-check.yml
  (branches: main + three named hermes/006x-009x names). None lists
  hermes/0172-pool-routing and none has a path filter covering mailbox/**, so
  the push starts no run. Confirmed live: `gh run list -R pagosacabin/nordtronics
  --branch hermes/0172-pool-routing` returns empty (rc 0). I did not add the
  branch to any allow-list: that would edit a second file, which the task's
  "README as the only changed file" criterion forbids, and no workflow here can
  verify a markdown protocol change.

  (d) COST (1.1.0 rule demonstrated on this reply). Source: this profile's
  state.db `session_model_usage` for this run's session
  (local-8bca9e55...61aea), read read-only. Measured at the staging read
  (2026-11-08T00:18:01Z): 31 API calls, input 31,719 (all cache-miss; the
  harness reports cache reads separately), cache-read 1,985,664, output 15,108,
  reasoning 9,079. Arithmetic reproduced against the harness's own
  estimated_cost_usd: 0.15*31719/1e6 + 0.003*1985664/1e6 + 0.6*15108/1e6 =
  0.004758 + 0.005957 + 0.009065 = $0.019780, matching 0.019779642 (cost_status
  estimated, cost_source official_docs_snapshot). The calls that compose and
  commit this staged file land after that read and are not included — the
  harness keeps no per-call log, so a later figure could not be attributed to
  this task either.

  (e) PROTOCOL DEMONSTRATION (1.2.0). No Newton run was made and no pool switch
  occurred: this is a pure documentation patch, which the new section assigns to
  Hermes on DeepSeek — the floor — and explicitly not to Newton's coding
  reserve ("scripts, tools, code review, research, drafting" goes to Newton only
  when the task says so; 0172 says "Newton is not involved"). Because there was
  no Newton run, there was no `agy -p "/usage"` gauge read to make; the routing
  rule's operational half is 0171 (the Flash sweep measurement) and the 20%
  switch point stays a first guess until it lands. Recording this decision is
  the rule's "record any pool or model switch in the staged reply notes" clause
  exercised with the "no switch" answer stated rather than implied.

  (f) SCOPE — branch edits `mailbox/README.md` only; no merge to main; no task
  file touched on the branch (this file's transition happens on main). 0171 was
  NOT started: a protocol-update task outranks it, and one task per run. The
  three pre-existing untracked paths in the worktree
  (firmware/wildfire-node-v1/bench-override.ini, hardware/solar-gate-v2/,
  take_screenshots.py) are not mine and are untouched. The skill edit is outside
  the repo, so it does not appear in the diff; it is quoted below instead.

---


# 0172 — Pool routing protocol section (1.2.0)

## Assignment
- **Hermes does it all**: the README patch on the branch, the skill patch in lockstep, and the proof. Newton is not involved in this task.

## Context
Protocol 1.1.0 (cost reporting, task 0155) went live on main 2026-11-07 (PR #1, merge 64a4e05e). Newton's Antigravity CLI has two independent weekly quota pools — Gemini models and Claude/GPT models — and Stephen set the routing rule on 2026-11-07: use them in order, not all at once, with DeepSeek as the floor. That rule needs to live in the protocol so Hermes drives Newton the same way every time, instead of deciding per task. This task also closes the lockstep gap 0155 declared: the handoff-mailbox skill's protocol block was never patched for the 1.1.0 cost format. Patch it now for both 1.1.0 and 1.2.0.

Chain of command: Juno (boss) → Hermes → Newton. Hermes is the mailbox reader.

## Task
1. On branch `hermes/0172-pool-routing` (off current main @ `64a4e05e`), patch `mailbox/README.md`: `protocol_version` 1.1.0 → 1.2.0, changelog entry, and a new `## Model pool routing` section containing exactly the rules below, in Stephen's words where quoted. Do not touch anything else in the README — the 1.1.0 cost format section stays as merged.
2. Patch the handoff-mailbox skill's protocol block in lockstep: cost format (1.1.0) plus the new routing section (1.2.0), so the skill matches the README.
3. One deliverable: the branch with the README patch. No merge to main — Juno verifies, Stephen approves the merge.

Routing rules to write into the README (verbatim substance, tidy the wording only):
- Order of pools: Newton runs on Gemini Flash first for the daily sweep, light research, and light code. When the Gemini pool drops under 20% remaining, switch Newton to the second pool for the same jobs. Hermes on DeepSeek is the floor: paid, pennies per task, never runs out in practice, and the only one that can touch the bench.
- Model roles: Gemini Flash is the volume model. Claude Sonnet 4.6 is the preferred model on pool two and the pick for harder coding jobs. Claude Opus 4.6 thinking is for the hardest jobs only — the task names it, or Sonnet has already failed on the same job. GPT-OSS 120B is the light option on pool two. Lightest model that does the job. Per-model burn is unmeasured — 0171 measures the Flash sweep; revisit the roles if a later measurement shows otherwise.
- Coding reserve: Newton is the coding reserve for pure software — scripts, tools, code review, research, drafting. Bench firmware (ESP32/ESPHome/PlatformIO that must be flashed and measured) is Hermes's by default: he is the only hands, and Newton drafting firmware that Hermes then has to flash and prove is two steps where Hermes writing it himself is one. Newton may draft firmware only when the task explicitly says so, and Hermes proves it on the bench before it counts.
- Before every Newton run, read both gauges free with `agy -p "/usage"` (no agent turn, no quota spent) and pick the pool with room. Never guess which pool has room.
- Quota stop: on 429 / `RESOURCE_EXHAUSTED` / "Individual quota reached", do NOT retry in a loop — report the pool, the "Resets in" time, and stop. Note whether the other pool still has room. An exhausted pool does not fail fast: `agy` retries with backoff until the print timeout, so looping burns minutes, not quota. Read the timer: under 5h means the five-hour window (wait it out); over 5h means the weekly cap (switch pools). If neither pool has room, report both reset times and run on DeepSeek until the next reset.
- Availability: never assume a model is there or gone from memory or web reports — verify live with a probe call (`agy -p` with `--model`, one-word reply). The Nov 2 removal reports said Claude 4.6 and GPT-OSS were off the free tier; live calls on 2026-11-07 proved both work. A model list is not proof; a live reply is. If a model is gone, report it and fall through to the next pool — do not silently substitute a different model.
- Jobs run headless via Hermes, not interactively. Interactive Newton turns are for questions and checks — they burn the pool unmeasured (4 points in 15 minutes of Q&A on 2026-11-07).
- Record any pool or model switch in the staged reply notes, so the choice is visible.
- The 20% switch point is a first guess, revisited once 0171 measures the real per-sweep cost.

## Success criteria
- Branch `hermes/0172-pool-routing` exists with `mailbox/README.md` as the only changed file: version 1.2.0, changelog entry, routing section present, cost format section intact.
- The handoff-mailbox skill's protocol block matches (cost format + routing); the patched block is quoted in the staged reply.
- No merge to main.

## Constraints
- DeepSeek flash, off-peak preferred; this is a small patch task — keep the spend ordinary (a patch and a skill edit, not a build).
- README patch only: do not reword the 1.1.0 cost format section, do not touch task files, do not touch Hermes's cron or sweep schedule.
- If any step is blocked (branch won't check out, skill file not found), report exactly where it stopped — do not invent readings or patch a different file.

## Proof
- Branch name + SHA.
- Diff summary (files changed, insertions/deletions).
- The patched skill protocol block, quoted in the reply.

## Reply format
Staged reply per the mailbox README on main (protocol 1.1.0): summary, proof pointers, notes field.

## Reply — 0172 (staged)

Branch `hermes/0172-pool-routing` @ 503e619 — one file changed,
`mailbox/README.md` (+54/-1). Diff summary:
`mailbox/README.md | 55 ++++++++++++++++++++++++++++++++++++++++++++++++++++++-`
(54 insertions, 1 deletion: the version line). No merge to main.

Success criteria against the spec:
- version 1.2.0, changelog entry, routing section present, cost format intact
  (cost section sha256 identical to main, quoted in notes (a)).
- handoff-mailbox skill protocol block matches the README (cost format 1.1.0 +
  routing 1.2.0), quoted in full below.
- no merge to main.

### Patched skill protocol block (verbatim)

````markdown
<!-- BEGIN PROTOCOL-MANAGED (do not edit by hand — updated via mailbox protocol tasks) -->
## Protocol updates

- The canonical protocol is `mailbox/README.md` on origin/main. It carries
  `protocol_version: <semver>`.
- The worker keeps `last_seen_protocol_version` in its local state file.
  Every tick it fetches the README and compares. On mismatch, it picks up and
  completes the protocol-update task from `mailbox/inbox/` before any other
  task, then records the new version in its state file.
- Protocol-update tasks are always written in the previous protocol format.
  Each carries exact skill-patch text between these delimiters: replace this
  whole block verbatim. Never paraphrase — paraphrase is where drift enters.
- A protocol update is not complete until the new rule is demonstrated in the
  staged reply itself, not merely acknowledged.

## Cost format

Write `cost` in the staged front-matter, summed from the DeepSeek API
usage objects of every API call the task made — not from cumulative
dashboard totals:

```yaml
cost:
  model: deepseek-flash
  api_calls: 12
  input_tokens: 85000
  input_cache_hit_tokens: 40000
  input_cache_miss_tokens: 45000
  output_tokens: 12000
  total_tokens: 97000
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.014
```

- `model`: the model actually used (deepseek-flash by default,
  deepseek-v4-pro only when the task named it).
- `rate_window`: off-peak | peak | mixed, decided per call timestamp
  (peak = 01:00-04:00 and 06:00-10:00 UTC weekdays).
- `cost_usd`: input_miss x miss rate + input_hit x hit rate +
  output x output rate + search_requests x $0.00075. State the rates
  used; do not copy a rate card blindly, rates change.
- If the harness cannot see per-call usage, set the token and cost
  fields to `unknown` and state why in `notes`. Never estimate a
  cost and present it as API usage.
- Juno verifies the arithmetic (tokens x rates = cost_usd) before
  archive, the same as branch SHA / run URL verification.

## Model pool routing

Newton's Antigravity CLI has two independent weekly quota pools — Gemini
models and Claude/GPT models. Use them in order, not all at once, with
DeepSeek as the floor.

- **Order of pools.** Newton runs on Gemini Flash first: the daily sweep,
  light research, and light code. When the Gemini pool drops under 20%
  remaining, switch Newton to the second pool for the same jobs. Hermes on
  DeepSeek is the floor: paid, pennies per task, never runs out in practice,
  and the only one that can touch the bench.
- **Model roles.** Gemini Flash is the volume model. Claude Sonnet 4.6 is the
  preferred model on pool two and the pick for harder coding jobs. Claude
  Opus 4.6 thinking is for the hardest jobs only — the task names it, or
  Sonnet has already failed on the same job. GPT-OSS 120B is the light option
  on pool two. Lightest model that does the job. Per-model burn is unmeasured
  — 0171 measures the Flash sweep; revisit the roles if a later measurement
  shows otherwise.
- **Coding reserve.** Newton is the coding reserve for pure software —
  scripts, tools, code review, research, drafting. Bench firmware
  (ESP32/ESPHome/PlatformIO that must be flashed and measured) is Hermes's by
  default: he is the only hands, and Newton drafting firmware that Hermes then
  has to flash and prove is two steps where Hermes writing it himself is one.
  Newton may draft firmware only when the task explicitly says so, and Hermes
  proves it on the bench before it counts.
- **Read both gauges before every Newton run.** `agy -p "/usage"` costs no
  agent turn and no quota. Read both gauges and pick the pool with room; never
  guess which pool has room.
- **Quota stop.** On 429 / `RESOURCE_EXHAUSTED` / "Individual quota reached",
  do NOT retry in a loop — report the pool, the "Resets in" time, and stop.
  Note whether the other pool still has room. An exhausted pool does not fail
  fast: `agy` retries with backoff until the print timeout, so looping burns
  minutes, not quota. Read the timer: under 5h means the five-hour window
  (wait it out); over 5h means the weekly cap (switch pools). If neither pool
  has room, report both reset times and run on DeepSeek until the next reset.
- **Availability.** Never assume a model is there or gone from memory or web
  reports — verify live with a probe call (`agy -p` with `--model`, one-word
  reply). The Nov 2 removal reports said Claude 4.6 and GPT-OSS were off the
  free tier; live calls on 2026-11-07 proved both work. A model list is not
  proof; a live reply is. If a model is gone, report it and fall through to
  the next pool — do not silently substitute a different model.
- **Headless, not interactive.** Jobs run headless via Hermes, not
  interactively. Interactive Newton turns are for questions and checks — they
  burn the pool unmeasured (4 points in 15 minutes of Q&A on 2026-11-07).
- **Record switches.** Record any pool or model switch in the staged reply
  notes, so the choice is visible.
- The 20% switch point is a first guess, revisited once 0171 measures the real
  per-sweep cost.
<!-- END PROTOCOL-MANAGED -->
````

(The two numbered engineering rules 23/24 that follow the cost/routing sections
in the file are unchanged and omitted here for length; the delimiter pair is
the boundary the skill defines.)
