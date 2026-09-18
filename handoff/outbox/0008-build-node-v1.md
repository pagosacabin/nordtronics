# 0008-build-node-v1: Outbox Reply (re-issued, fresh work)

**Status:** done

**Local Repo Path:** /home/astroboy/nordtronics

**Pull Frequency:** every push to main (webhook-triggered CI/CD)

**Branch:** hermes/0008-build-node-v1

**Commit SHA:** 83ccf2f3a7839959268e00ede3ad40dbd8b10797

**Actions Run URL:** https://github.com/pagosacabin/nordtronics/actions/runs/35405263857

**Actions Run Conclusion:** success

## What was done

Edited `.github/workflows/platformio.yml` so the build job actually compiles
`firmware/node-v1`, in addition to the existing `firmware/tank-monitor` build.
Added a second build step `pio run -d firmware/node-v1` (minimal change —
tank-monitor build is untouched).

CI run on branch `hermes/0008-build-node-v1` has a dedicated
`Build firmware (node-v1)` step that completed with `success` (step 8 of the
`build` job). This is a real node-v1 compile on GitHub Actions, not a local build.

## Definition of Done

- Branch `hermes/0008-build-node-v1` exists on origin with the workflow committed ✓
- Commit SHA 83ccf2f3a7839959268e00ede3ad40dbd8b10797 pushed ✓
- GitHub Actions run conclusion: success ✓
- Log shows node-v1 being compiled via the `Build firmware (node-v1)` step ✓

**Author:** Hermes Agent
**Date:** 2026-09-18