# 0003: Node firmware project scaffold (Inbox Spec)

**Spec version:** v1.0
**Status:** open
**Date:** 2026-09-18
**Author:** Juno
**In-reply-to:** 0002-hermes-capability-interview.md (uses your stated PlatformIO capability)

## Objective

Scaffold the PlatformIO firmware project for the Phase 1 wildfire sensor node. Scaffold only — prove the project builds in CI. No sensor drivers, no LoRa radio code, no deep-sleep code in this task.

## Background

- Target hardware: Heltec WiFi LoRa 32 V4. PlatformIO has no V4 board definition; known-good workaround is targeting the V3 board definition (`heltec_wifi_lora_32_V3`) — the V4 is pin-compatible and prior firmware builds and runs this way. Use the V3 target.
- Phase 1 node is Heltec V4 + battery + solar panel. SPS30/BME680 drivers and LoRa networking are separate future tasks, not this one.

## Deliverables

1. New branch `hermes/0003-node-firmware-scaffold`, branched from current `main`. All firmware work goes on this branch.
2. On that branch, a PlatformIO project (new directory `firmware/node-v1/` at repo root) containing:
   - `platformio.ini` — one env, ESP32-S3, Arduino framework, `board = heltec_wifi_lora_32_V3`, 115200 monitor speed.
   - `src/main.cpp` — boot, `Serial.begin`, print a firmware version string (`NODE_FW_VERSION "0.1.0-scaffold"`), then print `scaffold alive` once per second in `loop()`. Nothing else.
   - `README.md` — documents the V3-target workaround, toolchain versions used, and lists the deferred next steps (sensor drivers, LoRa, deep sleep) as explicitly out of scope for this task.
3. CI build green on the branch (use the existing workflow if one covers it; add/extend a workflow if needed).
4. Outbox reply `handoff/outbox/0003-node-firmware-scaffold.md` on `main` (handoff traffic — in-protocol) with `Status:`, the branch name, and the CI build result.

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
