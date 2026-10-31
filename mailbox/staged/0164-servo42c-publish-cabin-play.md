---
task_id: "0164"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  repo: pagosacabin/cabin-play
  branch: main
  sha: 43d7c95767a167dcffbc2741ebfd6655fbc2c639
  commit_url: https://github.com/pagosacabin/cabin-play/commit/43d7c95767a167dcffbc2741ebfd6655fbc2c639
  run: none
  run_note: >-
    cabin-play carries no .github directory at all (verified: `ls .github` in a fresh
    checkout of the repo -> "no .github in cabin-play"), so a push to main starts no
    workflow. Nothing to quote, and no CI could verify this task: the deliverable is
    repository content (markdown + a sketch), not a compiled artifact of that repo.
  source_revision:
    repo: pagosacabin/nordtronics
    branch: hermes/0163-servo42c-move-reconcile
    sha: 1eec491dfe50e2394e2a39de4ff2841c3e2ff1cf
    run: https://github.com/pagosacabin/nordtronics/actions/runs/41160051576
    run_head_sha: 1eec491dfe50e2394e2a39de4ff2841c3e2ff1cf
    run_conclusion: success
  files:
    - servo42c-uart-test/README.md
    - servo42c-uart-test/PROTOCOL.md
    - servo42c-uart-test/platformio.ini
    - servo42c-uart-test/src/main.cpp
  no_ntfy_receipt: >-
    spec names no ntfy topic and this task produces no build artifact of its own; the
    firmware it publishes was built and flashed under 0163, whose receipt discipline is
    already recorded in that staged reply.
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
  PUBLISHED — cabin-play main 43d7c95, four new files under servo42c-uart-test/.
  0163 was already staged at pickup, so the published sketch is the final 0163-C
  (direction-corrected, 8/8 PASS on hardware twice), not an intermediate.

  WHAT WAS VERIFIED, AND HOW.
  1. Content identity, not "looks the same": the three files copied from nordtronics
     were hashed against the 0163 tip BLOBS, and the published git blobs on cabin-play
     origin/main have the SAME object ids as the nordtronics ones —
       src/main.cpp    fc2b96225f5fc346b99d0c9551aa3d1376e587f7
       PROTOCOL.md     ac6c0b69b16539b0f61598eb54a93bcb5efeeb35
       platformio.ini  a4b1eae4168923798fb163edcc1def411e2a31c4
     (also sha256-checked after extraction: all three IDENTICAL). So the published
     sketch is byte-for-byte the build that passed.
  2. The published tree BUILDS as published: `cd servo42c-uart-test && pio run` from the
     cabin-play copy -> [SUCCESS] Took 4.83 s, RAM 6.6% (21472/327680), Flash 20.8%
     (272673/1310720), esp32dev. The .pio/ build dir was removed before the commit, so
     the published tree is source only.
  3. Push path: SSH (git@github.com:pagosacabin/cabin-play.git) — the path 0136 proved.
     The gh.py API credential was NOT used. Verified afterwards from the remote, not the
     local repo: `git fetch origin` -> origin/main == HEAD == 43d7c957..., and
     `git ls-tree -r origin/main servo42c-uart-test` lists exactly the four files.

  README.md IS NEW, and it is the one file not transplanted from nordtronics. The
  nordtronics bench README was stale in three ways (build 0160, "UartBaud = 38400",
  "no board was attached while this was written"), so publishing it would have
  contradicted the sketch. The cabin-play README documents the settings that actually
  work: CtrMode=CR_UART, UartBaud=9600, UartAddr=e0, MStep=16, wiring TX(16)/RX(17)+
  common GND+12-24V VIN, build/flash/monitor commands, the 8 steps and their PASS
  meaning, and the step-1-fails triage.

  BAUD DISCREPANCY, DECLARED RATHER THAN SILENTLY RECONCILED. PROTOCOL.md §1 quotes
  38400 8N1 as the factory UartBaud (index 04) straight from the Makerbase wiki, and it
  was published VERBATIM — I did not edit a cited reference to match the bench. The
  driver on this bench answers at 9600 and returned zero bytes at 38400 (0162). The
  README states 9600 as the required OLED setting and says explicitly that 38400 is
  what PROTOCOL.md §1 lists and that DRIVER_BAUD in the sketch must be changed to match
  a unit left on 38400. If you would rather PROTOCOL.md carry a "bench-verified 9600"
  line, that is a one-line edit — but it belongs in the nordtronics source of truth, not
  only in the published copy.

  SCOPE, one deliberate extension: platformio.ini was published alongside the three
  files the spec names. Without it the sketch is not buildable from the directory
  ("how to run the self-test" would be unreachable), and it is 8 lines of board/framework
  config. Nothing else in cabin-play was touched: the commit adds servo42c-uart-test/
  only (4 files, 783 insertions, 0 deletions), and `git status` in the checkout is clean
  apart from the removed .pio dir.

  WHAT I DID NOT DO: I did not re-flash or re-run the board for this task. The 8/8 PASS
  and the +90.13 / +90.10 deg figures quoted in the README come from 0163's two captures
  (staged), and the README says "verified on hardware" on that basis — no new hardware
  evidence was produced by this run, and none was asked for.

  MAILBOX NOTES: 0142 skipped, not resumed — front-matter `status: superseded` plus
  Juno's 2026-10-19 resolution. 0159 skipped as decision-blocked (no controller address);
  its file was not touched. 0165 left for the next tick (one task per run). 0164 arrived
  with no front-matter and a body-level `expect-reply-within`; protocol fields were added
  at pickup, declared in the pickup commit 2869d58.
---

# 0164 — Publish the SERVO42C code to cabin-play

## Context
The SERVO42C UART thread (0160 research, 0162 bench retest at 9600, 0163 move reconciliation) produced a working ESP32 sketch + PROTOCOL.md, currently living on nordtronics branches (`hermes/0160-servo42c-uart`, `hermes/0162-servo42c-retest-9600`). Stephen wants it published to the **pagosacabin/cabin-play** repo on GitHub — play projects live there, never in nordtronics. The bench test is proven working (driver answers at 9600 8N1, addr 0xE0; closed-loop moves executing).

## Task
1. Take the final working version of the sketch + PROTOCOL.md (the 0163-corrected state once that task stages; if 0163 hasn't staged yet, wait for it — publish the corrected version, not an intermediate).
2. Push them to **pagosacabin/cabin-play** under `servo42c-uart-test/` (sketch, PROTOCOL.md, plus a short README.md: what it is, the wiring — ESP32 TX2/RX2 to driver RX/TX, common GND, 12–24V VIN — the OLED settings it needs — UART mode enabled, 9600 baud, addr 0xE0, MStep as configured — and how to run the self-test).
3. Push from your laptop using a GitHub write path you can prove reaches cabin-play. SSH is proven (task 0136 went that way). Do NOT use the gh.py API credential — it is nordtronics-scoped and its API writes to cabin-play return 403. If you have your own API token, verify it with a read against cabin-play before writing.

## Success criteria
- `servo42c-uart-test/` live on cabin-play main with the sketch, PROTOCOL.md, and README.md.
- README lets someone with the same hardware wire it up and run the self-test without asking questions.

## Constraints
- Default model tier.
- Use your own proven write path for the cabin-play push (not the gh.py credential). Don't restructure anything else in the repo.

## Proof
- Commit SHA on cabin-play main + file listing of the new directory.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.


# Reply — STAGED (picked up 2026-10-31 03:16 UTC, work complete)

**Task 0164 — SERVO42C code published to `pagosacabin/cabin-play`.**

**Outcome in one line:** `servo42c-uart-test/` is live on cabin-play **main** at
**`43d7c95767a167dcffbc2741ebfd6655fbc2c639`**, carrying the final 0163-C sketch,
`PROTOCOL.md`, a new README, and the `platformio.ini` that builds them.

```
servo42c-uart-test/PROTOCOL.md      ac6c0b6  (blob id == nordtronics 0163 tip)
servo42c-uart-test/README.md        d01e927  (new — this is the wiring/run guide)
servo42c-uart-test/platformio.ini   a4b1eae  (blob id == nordtronics 0163 tip)
servo42c-uart-test/src/main.cpp     fc2b962  (blob id == nordtronics 0163 tip)
```

| | |
|---|---|
| cabin-play main | `43d7c95767a167dcffbc2741ebfd6655fbc2c639` |
| Source revision | nordtronics `hermes/0163-servo42c-move-reconcile` @ `1eec491d…` (CI run 41160051576, headSha match, success) |
| Push path | SSH — `git@github.com:pagosacabin/cabin-play.git`; gh.py credential not used |
| CI | none — cabin-play has no `.github/`, verified, so the push starts no run |
| Build check | `pio run` in the published directory: SUCCESS, 4.83 s, Flash 20.8% |

## What was done

1. Waited for and took the **final** version: 0163 is staged, so the sketch published is
   build 0163-C (direction bit corrected, signed angle grading, `0x33`/`0x30`
   instrumentation, 4×800 probe, retried reads) — not the 0162 intermediate.
2. Pushed to cabin-play main under `servo42c-uart-test/` with a **new** README (the
   nordtronics bench README was stale: build 0160, 38400 baud, "no board attached"). The
   new one covers, as the spec asked: what it is · wiring (driver TX→GPIO16, driver RX→
   GPIO17, common GND, 12–24 V VIN, 3V3 left open) · the four OLED settings (CtrMode=
   CR_UART, UartBaud=9600, UartAddr=e0, MStep=16 matching `STEP_SUBDIVISION`) · build,
   flash and monitor commands · the 8 graded steps and what PASS means · a triage table
   for step 1 failing.
3. `platformio.ini` added beyond the three named files — the sketch is not buildable
   without it, so "run the self-test" would otherwise be unreachable.

## Verification performed (not assumed)

- **Content identity by git object id**: the published blobs on cabin-play `origin/main`
  are `fc2b962` / `ac6c0b6` / `a4b1eae` — identical to the nordtronics 0163 tip blobs.
  The published sketch *is* the one that passed 8/8.
- **Remote read-back, not local state**: after pushing, `git fetch origin` gave
  `origin/main == HEAD == 43d7c957…`, and `git ls-tree -r origin/main servo42c-uart-test`
  lists exactly the four files above.
- **Builds as published**: `cd servo42c-uart-test && pio run` from the cabin-play
  checkout compiled and linked (RAM 6.6 %, Flash 20.8 %, `esp32dev`); `.pio/` removed
  before the commit.
- **Not done**: no re-flash, no new bench capture. The "8/8 PASS, +90.13°/+90.10°"
  figures in the README are 0163's, cited as such.

## One thing to know (declared, not hidden)

`PROTOCOL.md` was published **verbatim**, and its §1 quotes the Makerbase factory
`UartBaud` default of **38400**. This driver answers at **9600** (0162: zero bytes at
38400). The README states 9600 as the required OLED setting and explains the
discrepancy in-line, so the two files do not contradict each other — but if you want
`PROTOCOL.md` itself to carry a "bench-verified 9600" note, that edit should land in the
nordtronics source of truth rather than only in the published copy.
