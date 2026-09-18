# 0003-node-firmware-scaffold: Auto-generated Outbox Reply
**Status:** done
**Local Repo Path:** /home/astroboy/nordtronics
**Date:** 2026-09-18

## Objective


Scaffold the PlatformIO firmware project for the Phase 1 wildfire sensor node. Scaffold only — prove the project builds in CI. No sensor drivers, no LoRa radio code, no deep-sleep code in this task.

## Requirements
No specific requirements extracted.

## Hard limits


- No pushes to `main` except under `handoff/`. Firmware code stays on the branch until Juno/Stephen review and approve a merge.
- No spending, no procurement.
- KiCad out of scope.
- Scaffold only — do not add sensor, LoRa, or power-management code.

## Definition of done


- Branch `hermes/0003-node-firmware-scaffold` exists on origin with the scaffold committed.
- CI build passes on that branch.
- Outbox reply exists with `Status: done`, branch name, and build result recorded.

## Spec changelog

- v1.0 (2026-09-18): initial spec. First spec in the revised format (explicit version, hard-limits section) per Hermes's 0002 feedback.

**Capabilities in this Loop:**
- ✅ PlatformIO CI/CD builds (ESP32-S3 firmware)
- ✅ Git commit/push operations
- ✅ Workflow triggering on main branch
- ❌ Direct hardware upload (no USB device in this environment)
- ❌ Real-time sensor monitoring (no connected devices)

**Definition of Done:** Outbox reply exists with status , and the async mailbox loop is live.

**Author:** Hermes Agent
**Date:** 2026-09-18

