---
task_id: "0218"
protocol_version: 1.3.0
status: staged
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T22:15:28+00:00"
lease_until: "2026-11-18T23:45:28+00:00"
expect-reply-within: 12h
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
  STAGED (documentation correction; no code change, no CI run). Run 2026-11-18
  15:15-15:35 MST (22:15-22:35 UTC), off-peak (PEAK: OFF-PEAK 22:15 UTC),
  PROTOCOL MATCH 1.3.0. 0218 arrived with no front-matter; the protocol fields were
  added at pickup/staging (rule 16).

  TASK CHOICE. 0142 and 0187 are `status: superseded`; 0159, 0209, 0213, 0214 and
  0216 are decision-blocked and were left completely untouched (no notes edit, no
  iteration change) — 0216 explicitly so, since 0218 is the correction that
  supersedes its verdict without re-opening it. Inbox 0176 is `status: superseded`
  ("do not pick up"). 0218 was the only actionable task.

  WHAT CHANGED. Crew vault commit 6db52d8 (bench/wifi-lessons.md + INDEX.md +
  log/2026-11-18.md): the 15:01 MST "MAC 9c:cc:01:d0:78:80 condemned" verdict is
  struck through and marked retracted in place, a new 15:30 MST section records
  Stephen's bare-board join and attributes the fault to the rig environment with the
  clean-USB power separator as the test that names the mechanism, the two
  forward-looking paragraphs that still implied a faulty board were corrected, and
  rules 9-10 were added (test a suspect outside its assembly; a capture window
  measures a placement, not a component). Branch hermes/0213-esphome-skeleton @
  bd0183d carries the same correction in firmware/skeleton/BUILD-AND-PROVISION.md,
  with the superseded paragraph kept as a marked block quote.

  VERIFIED HERE. git ls-remote --heads origin hermes/0213-esphome-skeleton equals
  bd0183d1f4765c31c07c53c5dbfeac4c30d4016e; the pushed commit is 1 file changed,
  32 insertions(+), 16 deletions(-); zero CI runs on the branch and zero check-runs
  on the commit (workflow triggers enumerated in proof.run_note); the vault working
  tree is clean at 6db52d8 (`git status` empty).

  NOT VERIFIED HERE, DECLARED. Stephen's bare-board join is quoted from 0218's own
  report; this worker cannot see the Starlink device list. The 15:15 MST vault
  correction (56beefa) predates this run and is not this run's work. Cost fields are
  `unknown` because the harness exposes no per-call usage to the cron session.

  NO SECRETS: no SSID or passphrase value appears in the reply or the diffs; only
  hashes and MAC addresses, as in 0216.
proof:
  branch: hermes/0213-esphome-skeleton
  sha: bd0183d1f4765c31c07c53c5dbfeac4c30d4016e
  run: none
  run_note: |
    No CI run exists to cite and none is expected. Enumerated, not asserted: all six
    workflows in the repo were read for their triggers — android-build.yml
    (branches: android-toolchain-setup), android-companion-v0.yml (branch allow-list +
    paths android/companion-v0/**), backend.yml (paths backend/**), detection-sim.yml
    (paths python/detection-sim/**), platformio.yml (paths firmware/tank-monitor/**,
    firmware/node-v1/**, firmware/wildfire-node-v1/**, python/detection-sim/**),
    website-check.yml (branches main + three named hermes/0068,0070,0093) — none matches
    firmware/skeleton/** or the branch hermes/0213-esphome-skeleton. Confirmed live:
    `gh run list -R pagosacabin/nordtronics --branch hermes/0213-esphome-skeleton`
    returns zero runs, and `gh api repos/pagosacabin/nordtronics/commits/bd0183d/check-runs
    --jq .total_count` returns 0. No ntfy receipt either: the change is documentation-only,
    no compiled artifact was produced, so the build topic does not apply.
  files:
  - firmware/skeleton/BUILD-AND-PROVISION.md
  files_reason: |
    One repository file: the 0213 skeleton addendum, where the earlier "board is the fault
    / MAC condemned" paragraph lived. firmware/skeleton/README.md and the two yaml files are
    untouched (verified with `git show --stat bd0183d`: 1 file changed, 32 insertions,
    16 deletions).
  vault:
    repo: ~/crew-vault (local-only, no remote)
    commit: 6db52d812297ac1d45d44416aa1bdb3ab551c2c7
    files:
    - bench/wifi-lessons.md
    - INDEX.md
    - log/2026-11-18.md
    note: |
      Local-only repo, so the pointer is the commit SHA, as in 0216's precedent. Nothing
      about the vault is pushed anywhere.
  unverified_by_this_run: |
    Stephen's bare-board join is quoted from 0218's own report, not measured here: this
    worker cannot see the Starlink app's device list. What this run *did* verify is that
    no surviving document still tells a reader to condemn the board, and that the branch
    and vault bytes are as quoted.
---

# 0218 — Correction to 0216: the board is GOOD, the fault is in the rig

**Done and staged.** The board is not condemned; the documents that said so now say
otherwise, and 0216's replacement step is withdrawn in writing.

## What was wrong, and what changed

The 14:50-15:00 MST separator test (0216 step 1) ended with "MAC 9c:cc:01:d0:78:80
condemned" in two places: the crew vault page `bench/wifi-lessons.md` and the skeleton
addendum `firmware/skeleton/BUILD-AND-PROVISION.md`. Stephen's report of ~15:08 MST
(task 0218) supersedes that: pulled off the breadboard, the same board on the same image
and credential **joined immediately** and is the `propane-bench` client in the Starlink
device list. The fault therefore follows the rig, not the board.

Two documents, both corrected in place with the retraction left visible rather than
deleted (so the wrong inference keeps its provenance):

1. **Crew vault `bench/wifi-lessons.md`** — vault commit `6db52d8` (3 files: the page,
   `INDEX.md`, `log/2026-11-18.md`).
   - A new dated section, "2026-11-18 15:30 MST — the board is GOOD; the fault follows the
     rig (task 0218)", carries the bare-board join, the "do not condemn / do not replace"
     instruction, and the next separator: **clean USB power while the board is seated in
     the rig** (join → rig power path drooping under WiFi TX load; still refused on clean
     power → RF/antenna environment in the rig).
   - The 15:01 MST verdict section is retitled "(verdict RETRACTED at 15:15/15:30 MST)";
     its "Condemned: MAC …" line is struck through and marked retracted, and its
     "next step is Stephen's: swap the rig onto a spare … mark the condemned board
     physically" paragraph is replaced with "do not swap, do not mark".
   - The page's forward-looking "What remains — two suspects" paragraph (both suspects now
     dead) and the "propane bench board does not join" bullet were corrected too, and the
     header now sends the reader to the two dated corrections first.
   - Two rules earned, added as 9 and 10: a part that fails only inside its assembly has
     been tested as a *system*, not a part; and a capture window measures a *placement*,
     not a component.
2. **Skeleton addendum** — `firmware/skeleton/BUILD-AND-PROVISION.md`, branch
   `hermes/0213-esphome-skeleton` @ `bd0183d` (pushed; `git ls-remote --heads origin`
   returns exactly this SHA). The header gained a one-line 0218 correction and "Where the
   propane board stands" was rewritten: board not at fault, no replacement step, the
   clean-power separator named, the rail-sag theory labelled a hypothesis rather than a
   finding, and the old paragraph preserved as an explicit block quote marked "do not act
   on it". The build and portal rules above it are untouched.

## What was deliberately NOT touched

- **`mailbox/active/0216-propane-wifi-end-the-fight.md` is untouched** — no notes edit, no
  iteration change, still `status: in_progress`, still parked on Stephen's hands. It is a
  decision-blocked task and the protocol forbids re-opening it. **Its notes still carry the
  "MAC … CONDENNED" wording and point at vault commit `63a48d8`; read this staged file, not
  0216's notes, for the verdict.** Its step 3 (swap to a spare board) is withdrawn by this
  correction — recorded in both corrected documents.
- 0142 and 0187 (`status: superseded`), 0159/0209/0213/0214 (decision-blocked) and inbox
  0176 (`status: superseded`, "do not pick up") were all left untouched.
- **`mailbox/active/0216`'s board is not physically marked, and nothing was reflashed, no
  credential staged, no threshold or sensor config moved.** This task is documentation.

## Provenance and limits — read this before crediting the correction

- The 15:15 MST correction already in the vault (commit `56beefa`, "board joins
  intermittently … adopted in HA at .130") was **not written by this run**; it was in the
  repo when this tick started. This run's contribution is the 15:30 section, the retraction
  markers, the two new rules, and the correction of the forward-looking paragraphs that
  still called the board faulty.
- **The bare-board join is Stephen's report, relayed by 0218** — this worker cannot read the
  Starlink device list, so it is quoted, not independently measured. What was verified here
  is documentary: the vault page's post-edit state, the branch tip SHA, and the absence of
  any CI run (enumerated above).
- 0218 arrived with **no front-matter at all**; `task_id`, `protocol_version` and
  `expect-reply-within` were added at staging, and the claim wrote `status`, `iteration`
  and `claimed_*` (rule 16). Its own header calls it "not a task": it was nevertheless the
  lowest actionable inbox item and the only thing that corrects 0216's verdict, so it was
  taken through the normal flow rather than left sitting.
- Cost fields are `unknown` because this harness exposes no per-call DeepSeek usage to the
  cron session (not estimated).

## What is left for Stephen

**When the rig is reassembled: power the board from clean USB while it is still seated in
the rig, and capture serial.** A join names the rig's power path (shared 5 V rail, both
sensor heaters, breadboard leads) as the mechanism; a refusal on clean power points at the
rig's RF/antenna environment. That is the one measurement that closes 0216 step 1's
mechanism — and it does not involve condemning or replacing any board.
