---
task_id: "0251"
title: OpenRouter backup test — install the funded key, run one real Hermes task on Inkling
status: in_progress
iteration: 2
expect-reply-within: 6h
proof: []
notes: |
  BLOCKED on the funded key -- it authenticates but cannot run inference.
  DECISION OWED: an inference-scoped OpenRouter key.  No proof block; nothing staged.

  What was done:
   1. Key installed from /home/astroboy/.hermes/openrouter-api-key.txt (sha prefix
      73eb8c5e, length 73, mode 600 files) into BOTH .env files, replacing the
      old key line.  It authenticates: GET /api/v1/key returns 200, limit null
      (not $0), usage 0, credits endpoint reports total_credits 10,
      free_model_daily_requests 0/1000, so the account is funded.
   2. It cannot infer.  POST /api/v1/chat/completions with that key, on both
      nvidia/nemotron-3-ultra-550b-a55b:free and openrouter/free, returns
      HTTP 401 {"error":{"message":"User not found.","code":401}}.  The key is
      flagged is_management_key: true and is_provisioning_key: true -- a
      provisioning key, which manages the account but is not an inference
      credential.  Hermes says the same thing in its own words:
      "OpenRouter rejected your API key ... Provider said: HTTP 401: User not
      found."
   3. Fallback chain entry 3 changed from openrouter/free to the specific free
      model nvidia/nemotron-3-ultra-550b-a55b:free (one-line config edit,
      comments 36 -> 36).  `hermes -p cronrunner fallback list` reads the chain
      back with that model at entry 3; `fallback check` reports all three
      entries config ok / auth ok.  Primary and job pinning untouched.
   4. Inkling substitution is in force: thinkingmachines/inkling:free is NOT in
      OpenRouter's live 439-model list.  Paid variants exist (thinkingmachines/
      inkling, inkling-small, inkling-2-mini, inkling-2-small) but no :free id,
      so the card's stated substitute (nvidia/nemotron-3-ultra-550b-a55b:free,
      1M ctx, tools supported, $0) is what the chain names.
   5. End state of the .env files: the WORKING key (sha prefix fc9595ea, the
      previous one) is back in place, because it does serve free models --
      a direct chat completion on nvidia/nemotron-3-ultra-550b-a55b:free
      returned HTTP 200 with that key -- and leaving a key that 401s on every
      inference call would have made the backup path strictly worse than it
      was.  The funded key is untouched at its staged path
      /home/astroboy/.hermes/openrouter-api-key.txt; this is a declared
      deviation from "install the funded key", taken because the funded key
      cannot be used for the thing the task exists to test.
   6. The live tool-calling test was attempted and did not run: with the funded
      key `hermes -z` failed immediately with the 401 above; with the working
      key the same tool-using one-shot ran past 180 s with an empty log, which
      is the known `hermes -z` hang on tool calls in this environment, not a
      model fault.  So there is no served-model ID, no timing and no vault
      answer to report -- none is invented here.

  Pickup and park are the same commit (iteration 1 -> 2).

  Decision owed: mint an inference-scoped key (OpenRouter -> Keys -> create a
  normal key, not a provisioning key; the account has $10 of credit) and stage
  it at the same path.  Then this task is one tick: install, run the test, report
  the served model.  Alternatively Stephen can re-issue the existing key's
  permission, but a provisioning key cannot be promoted -- a new key is needed.
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

## Status: parked blocked — the funded key authenticates but cannot infer

No `proof` block. The chain is configured and reads back correctly; the key the
task is built around is the wrong kind of key, so the live test cannot run.

| step | state |
|---|---|
| 1. install the funded key | DONE then REVERTED — it authenticates (200 on /api/v1/key, limit null, $10 credits) but returns `HTTP 401 {"error":{"message":"User not found.","code":401}}` on every chat completion, on both the free model and the `openrouter/free` router. `is_management_key: true, is_provisioning_key: true` — a provisioning key, not an inference credential. Both `.env` files now hold the previous working key (prefix fc9595ea) so the fallback path is not left broken. |
| 2. chain names the model | DONE — entry 3 is `openrouter / nvidia/nemotron-3-ultra-550b-a55b:free`. `hermes -p cronrunner fallback list` reads it back; `fallback check` says config ok, auth ok for all three entries. Primary and job pinning untouched. |
| 3. live tool-calling test | NOT RUN — funded key 401s on the first call; with the working key `hermes -z` hung past 180 s with an empty log (the known tool-call hang in this environment). No served model ID, no timing, no vault answers. |
| 4. report what served | NOTHING SERVED — reporting an ID here would be a fabrication. |

**Substitution** stated as the card requires: `thinkingmachines/inkling:free` is
not in OpenRouter's live model list (439 models; paid `thinkingmachines/inkling*`
variants exist, no `:free` id), so the card's named substitute
`nvidia/nemotron-3-ultra-550b-a55b:free` (1M context, tool calling, $0) is what
the chain carries.

**Why the key is the blocker, in one line:** a provisioning key manages the
account (`/api/v1/key`, `/api/v1/credits`, `/api/v1/models/user` all answer 200;
`total_credits: 10`) but cannot be used as a model credential — OpenRouter
answers `User not found` for inference because there is no inference user behind
it. A new, normal key is the fix; nothing can be done to the existing one.

**Blast radius of the two failed attempts, declared:** the auth pool now records
`openrouter OPENROUTER_API_KEY auth failed (401)` from the funded-key attempt
(`hermes -p cronrunner auth list`). It will clear when the pool re-probes a key
that works; the previous key is back in `.env` in both profiles.
