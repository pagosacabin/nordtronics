---
task_id: "0251"
title: OpenRouter backup test — install the funded key, run one real Hermes task on Inkling
status: pending
iteration: 1
expect-reply-within: 6h
proof: []
notes: Filed by Juno 2026-11-23 on Stephen's word ("Let's go ahead and do a test of inkling as a backup model", 21:19 MST). Follows 0221, which added openrouter/free as fallback entry 3 but left two blockers: the funded key was never installed (that task forbade writing it) and every worker/sweep job is pinned. This task removes the first blocker and proves the model end to end; it does NOT unpin the production jobs — that decision comes after this test reads out.
---

## Assignment

- **Hermes:** all of it — key install, chain configuration, the live test run, and the reply. Host-only work on Stephen's laptop; no repo code changes, so no Copilot step.
- **Stephen:** nothing. The funded key is already staged where you left it (see Constraints).
- **Juno:** verifies the reply — model ID as served, test output, chain state — and reports to Stephen.

## Context

0221 (verified 2026-11-20) added `openrouter/free` as fallback entry 3 in the cronrunner profile and probed it: the router answered with a free NVIDIA model (flagged model_mismatch, working as designed). But the fallback has never carried a real run, for two reasons 0221 recorded: (1) both `.env` files (~/.hermes/.env and the cronrunner profile .env) still hold the OLD OpenRouter key (sha prefix fc9595ea, per-key limit $0.00 — authenticates, cannot spend); the funded key Stephen staged was read for a probe but not installed, because 0221's spec forbade writing it. (2) Every worker/sweep job is pinned to DeepSeek, so pinned work never falls back. Stephen has now decided: test Inkling (`thinkingmachines/inkling:free`) as the backup model — 1M-token context, built for coding/agent harnesses, currently the largest free model on OpenRouter.

## Task

1. **Install the funded key.** Read the key from Stephen's staged file at `/home/astroboy/.hermes/openrouter-api-key.txt` and set it as OPENROUTER_API_KEY in both `.env` files (default profile and cronrunner profile). Confirm the installed key authenticates and report only its sha prefix (expected 73eb8c5e) and that its limit is not $0 — never print the key itself anywhere.
2. **Point the fallback at Inkling.** Configure the cronrunner fallback chain so the OpenRouter entry names the specific model `thinkingmachines/inkling:free` (not just the openrouter/free router) for this test. Leave the rest of the chain order as it is. Do NOT change any job's pinning.
3. **Run one real end-to-end test on Inkling.** In a fresh session forced onto the Inkling fallback (unpinned test run, not a production job), complete this small real task using tools, and keep the full output:
   - Read the crew vault at ~/crew-vault/ and answer, from the vault alone: (a) the ADS1115 I2C address and its SDA/SCL GPIOs for the propane board; (b) the WiFi provisioning rule for new ESPHome devices.
   - Then make one read-only git call against the nordtronics repo (e.g. report the current main SHA and the three most recent commit subjects).
   The point is proof that tool calling, file reads, and a shell/git call all work through the OpenRouter/Inkling path — not just a chat completion.
4. **Report what actually served.** From the API response metadata, state the exact model ID that served the test, the round-trip time for the run, any rate-limit or error responses, and the token/request counts if the response reports them.

## Success criteria

- Both .env files hold the funded key (sha prefix reported, authenticates, limit not $0).
- The fallback chain's OpenRouter entry names `thinkingmachines/inkling:free`.
- The test run completes both parts (vault answers + git read) through the Inkling path, with the served model ID and timing in the reply.
- If `thinkingmachines/inkling:free` is no longer available on OpenRouter (the free list churns weekly), run the identical test on `nvidia/nemotron-3-ultra-550b-a55b:free` instead and state the substitution plainly at the top of the reply.

## Constraints

- Secrets: the key is read from the staged file on this machine only. It never appears in the reply, in any commit, or in any log you quote. Sha prefix only, as in 0221.
- No unpinning, no changes to production worker/sweep job configuration, no changes to the primary model. This is a test of the backup path, not a cutover.
- Cost: the test model is free ($0/token). Do not call any paid model for this task. Free-tier limits are 20 requests/min and 1,000/day on Stephen's funded account — stay well inside them.
- If the key install or the test fails, park blocked with the exact error rather than improvising a workaround.

## Proof

- Host-only task: the reply itself is the deliverable (served model ID, test output, timings, chain state). Juno cross-checks the vault answers against the known ground truth (ADS1115 0x48, SDA GPIO7, SCL GPIO6; captive-portal-only WiFi provisioning).

## Reply format

Status line first. Then: key install result (prefix + auth), the chain as configured, the test — what was asked, the served model ID, timing, the vault answers and git output verbatim — then any errors or deviations, plainly.
