# 0005-ci-gitignore-fix: Outbox Reply (re-issued, fresh work)

**Status:** done

**Local Repo Path:** /home/astroboy/nordtronics

**Pull Frequency:** every push to main (webhook-triggered CI/CD)

**Branch:** hermes/0005-ci-gitignore-fix

**Commit SHA:** 731c0b5f25e5915307d7cda1733bc0a5addb0933

**Actions Run URL:** https://github.com/pagosacabin/nordtronics/actions/runs/35405853483

**Actions Run Conclusion:** success

## What was done

Based on the verified 0004 branch (`hermes/0004-node-firmware-scaffold`):

1. **Extended `.github/workflows/platformio.yml` trigger paths** to include
   `firmware/node-v1/**` (both `push` and `pull_request`), keeping the existing
   `firmware/tank-monitor/**` paths.
2. **Added root `.gitignore`** ignoring PlatformIO build artifacts:
   `.pio/`, `*.bin`, `*.elf`, `*.map`. The `.pio/` build artifacts are no longer
   tracked (verified 0 `.pio` files tracked in git).

Committed on branch `hermes/0005-ci-gitignore-fix` and pushed to origin.

## Definition of Done

- Branch `hermes/0005-ci-gitignore-fix` exists on origin, based on the 0004 branch ✓
- Workflow + gitignore changes committed (SHA 731c0b5f25e5915307d7cda1733bc0a5addb0933) ✓
- `.pio/` artifacts no longer tracked on the branch ✓
- GitHub Actions run triggered on the branch, conclusion `success` ✓
  (run 35405853483, verified via authenticated API)

**Author:** Hermes Agent
**Date:** 2026-09-18