---
task_id: "0232"
status: in_progress
iteration: 1
proof: []
notes: "Filed by Juno 2026-11-20 at Stephen's direction (block talk, issue 6). Prerequisite met: 0213 is verified. Stephen's shape for the live proof: the skeleton doubles as the standard new-board test — when a new board arrives he hooks it up, and Juno files a short run-through task for Hermes to flash and test the skeleton on it. That board task is filed at board-arrival time, not here; this task delivers the skill that makes it routine."
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-20T22:24:05+00:00"
lease_until: "2026-11-20T23:54:05+00:00"
---

# 0232 — Encode the ESPHome skeleton as your own skill, with the new-board bring-up procedure

## Assignment
- Hermes: every step below — your own skills on your own machine.
- Stephen: no input needed for this task. His part comes later and outside it: when a new board arrives, he hooks it up and tells Juno, and Juno files the run-through task.
- Juno: verifies the staged reply, including the fresh-session retest output against the 0213 pattern.

expect-reply-within: 24h

## Context
Task 0213 built and verified the reusable ESPHome skeleton, now on branch `hermes/0213-esphome-skeleton` in the nordtronics repo under `firmware/skeleton/`. Its pattern, settled by Stephen: three layers — L1 captive portal carries WiFi credentials ONLY (no staged password files, ever), L2 firmware defaults (including the MQTT broker `mqtt.nordtronics.io`/`8883`, which is a firmware default and never a portal field), L3 per-device settings persisted in NVS and editable through the portal. Stephen's direction when 0213 was verified: once Juno verified it, you encode the skeleton as your own skill so this pattern is your default way of building ESPHome configs. This is that task.

Stephen also set the live-proof shape (2026-11-20): the skeleton is the standard new-board test. When a new board comes in, he hooks it up and Juno files a short task for you to run through the bring-up and put the skeleton on it. For that to be routine, the bring-up procedure has to live inside the skill itself — step 2 below.

## Task
1. Read the 0213 skeleton on branch `hermes/0213-esphome-skeleton` (`firmware/skeleton/`) in full. Encode it as a skill in your own skill store, in your own words and structure, covering: the three-layer pattern and what belongs in each layer, the captive-portal-only WiFi rule, the broker-as-firmware-default rule, and how a new device config gets derived from the skeleton.
2. In the same skill, write the **new-board bring-up procedure**: given a freshly hooked-up, unflashed board, the exact run-through — identify the board/chip, flash the skeleton, confirm the fallback AP appears, join it and confirm the portal serves, enter test WiFi credentials, confirm the board joins the network, change one L3 setting in the portal, reboot, confirm the setting persisted from NVS, and report each check pass/fail. This procedure is what a future run-through task will point at.
3. Fresh-session retest: from a fresh session, using ONLY your new skill (not the 0213 branch), produce the skeleton config you would flash for a hypothetical new ESP32-C3 sensor board, and include it in your reply. Juno compares it against the 0213 pattern.

## Success criteria
- The skill exists in your store; name it and its path in the reply.
- The skill contains the new-board bring-up procedure as its own section.
- The fresh-session config from step 3 follows the three-layer pattern: portal carries WiFi only, broker is a firmware default (not a portal field), device settings persist via NVS.

## Constraints
- Encode the pattern, do not redesign it. If you think something in the 0213 skeleton should change, say so in the reply as a deviation — do not bake changes into the skill silently.
- No board work in this task: nothing gets flashed. The live run-through happens under a later task when Stephen designates a board.
- Model for this task: your normal DeepSeek flash tier.

## Proof
Host-only task: `proof: []` with the skill name/path and the step-3 config in the reply body.

## Reply format
Status line first. Then: skill name and path, a one-paragraph summary of what the skill covers, the step-3 fresh-session config, and any deviations or proposed changes to the pattern.
