# 0004: Node firmware scaffold — rework of 0003 (Inbox Spec)

**Spec version:** v1.0
**Status:** open
**Date:** 2026-09-18
**Author:** Juno
**In-reply-to:** 0003-node-firmware-scaffold.md (inbox), 0003 outbox reply (false done)

## Why this rework exists

The 0003 outbox reply claimed `Status: done`, but verification at 12:28 MDT showed no `hermes/0003-node-firmware-scaffold` branch on origin, no firmware committed, no CI run. The work was not performed. This spec re-issues the same task under a new number. Read it fully — do not copy the 0003 reply.

## Objective

Scaffold the PlatformIO firmware project for the Phase 1 wildfire sensor node. Scaffold only — prove the project builds in CI. No sensor drivers, no LoRa radio code, no deep-sleep code in this task.

## Background

- Target hardware: Heltec WiFi LoRa 32 V4. PlatformIO has no V4 board definition; known-good workaround is targeting the V3 board definition (`heltec_wifi_lora_32_V3`) — the V4 is pin-compatible and prior firmware builds and runs this way. Use the V3 target.
- Phase 1 node is Heltec V4 + battery + solar panel. SPS30/BME680 drivers and LoRa networking are separate future tasks, not this one.

## Deliverables

1. New branch `hermes/0004-node-firmware-scaffold`, branched from current `main`. All firmware work goes on this branch.
2. On that branch, a PlatformIO project (new directory `firmware/node-v1/` at repo root) containing:
   - `platformio.ini` — one env, ESP32-S3, Arduino framework, `board = heltec_wifi_lora_32_V3`, 115200 monitor speed.
   - `src/main.cpp` — boot, `Serial.begin`, print a firmware version string (`NODE_FW_VERSION "0.1.0-scaffold"`), then print `scaffold alive` once per second in `loop()`. Nothing else.
   - `README.md` — documents the V3-target workaround, toolchain versions used, and lists the deferred next steps (sensor drivers, LoRa, deep sleep) as explicitly out of scope for this task.
3. CI build green on the branch (use the existing workflow if one covers it; add/extend a workflow if needed).
4. Outbox reply `handoff/outbox/0004-node-firmware-scaffold.md` on `main` (handoff traffic — in-protocol) with `Status:`, the branch name, and the CI build result.

## Reply discipline

Write the outbox reply only after the branch is pushed to origin and CI is green. Verify both yourself before writing it. Do not mark done before then — a premature done will be caught on verification and the task re-issued again.

## Hard limits

- No pushes to `main` except under `handoff/`. Firmware code stays on the branch until Juno/Stephen review and approve a merge.
- No spending, no procurement.
- KiCad out of scope.
- Scaffold only — do not add sensor, LoRa, or power-management code.

## Definition of done

- Branch `hermes/0004-node-firmware-scaffold` exists on origin with the scaffold committed (verifiable via `git ls-remote`).
- CI build passes on that branch.
- Outbox reply exists with `Status: done`, branch name, and build result recorded.

## Spec changelog

- v1.0 (2026-09-18): initial spec. Re-issues 0003, whose outbox reply claimed done without the work being performed.
