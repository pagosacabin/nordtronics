---
task_id: "0231"
status: pending
iteration: 0
proof: []
notes: "Filed by Juno 2026-11-20 at Stephen's direction during the block talk. This is the outstanding acceptance proof for the 0214 crew-vault pilot. Stephen notes Newton's token pools may be dry until the Friday reset — the task is written to wait for the pool rather than burn retries."
---

# 0231 — Crew-vault retest: fresh-session Newton answers from the vault alone

## Assignment
- Hermes: every step below. You route the questions to Newton via Herdr; Newton never reads the mailbox.
- Newton (via Hermes): answers the two questions using ONLY the crew vault at `/home/astroboy/crew-vault` — no other files, no web, no asking Hermes.
- Juno: verifies Newton's answers against ground truth she holds outside this spec. The answers are deliberately NOT in this file — do not put them in Newton's brief, and do not hint.

expect-reply-within: 72h

## Context
Task 0214 built the crew-vault pilot: a git-backed shared memory at `/home/astroboy/crew-vault` on this laptop, Hermes as librarian, four seed pages. The pilot's acceptance test was never run: a fresh Newton session, with no prior context, answering questions from the vault alone. If he can, the vault works as shared memory. If he can't, his misses show exactly what the vault is missing.

Stephen's note (2026-11-20): Newton's token pools may be dry until the Friday reset. This task waits for the pool — see step 1.

## Task
1. Check Newton's availability through Herdr as you normally would before routing work. If his pools are dry, do NOT force attempts and do NOT retry in a loop: stage a short reply saying the retest is waiting on the Newton pool reset, leave this task in active, and stop. Juno re-raises it after the reset.
2. When Newton is available: start a FRESH Newton session (new session, no carried context). Give him exactly this brief, nothing more: the vault path `/home/astroboy/crew-vault`, and the two questions in step 3. Do not include answers, hints, page names, or your own summary of the vault's contents.
3. The two questions, verbatim:
   a. "On the propane bench board (ESP32-C3 with an ADS1115): what is the ADS1115's I2C address, and which GPIOs are SDA and SCL?"
   b. "What is the rule for WiFi credentials when provisioning a new board?"
4. Record Newton's answers verbatim, including any "not in the vault" or uncertainty he states. Do not correct, complete, or annotate his answers in the reply — Juno verifies them as given.
5. Note which vault pages Newton cited or opened, if his session surfaces that.

## Success criteria
- Newton's two answers reproduced verbatim in the staged reply.
- A clear statement of whether each answer came from the vault, and which pages he used.
- If the pool was dry: the waiting reply described in step 1 instead — that is a valid outcome for this filing, not a failure.

## Constraints
- Newton gets the vault and the two questions only — no other sources, no hints, no second tries with rephrased questions.
- Read-only: nobody edits the vault during this task, whatever Newton finds or misses. Gaps get reported, not fixed, here.
- Model for your own work: your normal DeepSeek flash tier. Newton's usage is two questions — no extra exploration runs.

## Proof
Host-only task: `proof: []` with Newton's verbatim answers in the reply body.

## Reply format
Status line first. Then: Newton's answer to (a) verbatim, his answer to (b) verbatim, the pages he used, and any deviations. If waiting on the pool, the status line says so.
