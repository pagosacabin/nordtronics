# 0005: CI coverage + gitignore for node firmware (Inbox Spec)

**Spec version:** v1.0
**Status:** open
**Date:** 2026-09-18
**Author:** Juno
**In-reply-to:** 0004-node-firmware-scaffold.md (inbox), 0004 outbox reply (verified, with nits)

## Why this task exists

0004's scaffold is real and verified, but two nits remain: (1) the PlatformIO workflow only triggers on `firmware/tank-monitor/**`, so no GitHub Actions run ever fired for `firmware/node-v1/` — the "CI passes" claim was a local build only; (2) the `.pio/` build artifacts were committed to the branch and should never be tracked.

## Objective

Give the node firmware project real CI coverage and stop tracking build artifacts.

## Deliverables

1. New branch `hermes/0005-ci-gitignore-fix`, branched from `hermes/0004-node-firmware-scaffold`.
2. On that branch:
   - Extend `.github/workflows/platformio.yml` trigger paths to include `firmware/node-v1/**` (keep the existing `firmware/tank-monitor/**` paths).
   - Add a `.gitignore` at repo root (or extend the existing one) ignoring `.pio/` (and `*.bin`, `*.elf` build outputs if not already covered).
   - Untrack the committed build artifacts: `git rm -r --cached firmware/node-v1/.pio` (and the stray `.pio/build/project.checksum` at repo root if present), then commit.
3. Push the branch and confirm a real GitHub Actions run triggers on it and goes green.
4. Outbox reply `handoff/outbox/0005-ci-gitignore-fix.md` on `main` (handoff traffic — in-protocol) with `Status:`, the branch name, and the Actions run URL + conclusion.

## Reply discipline

Write the outbox reply only after the branch is pushed and the Actions run shows green. Verify both yourself before writing it. Juno will independently verify the Actions run via the API.

## Hard limits

- No pushes to `main` except under `handoff/`. Fix stays on the branch until Juno/Stephen review and approve a merge.
- No spending, no procurement.
- KiCad out of scope.

## Definition of done

- Branch `hermes/0005-ci-gitignore-fix` exists on origin, based on the 0004 branch, with the workflow + gitignore changes committed.
- `.pio/` artifacts no longer tracked on the branch.
- A GitHub Actions run on that branch concludes `success` (verifiable via API — not a local build).
- Outbox reply exists with `Status: done`, branch name, and Actions run URL + conclusion.

## Spec changelog

- v1.0 (2026-09-18): initial spec. Follow-up to verified 0004.
