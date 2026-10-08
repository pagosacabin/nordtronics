---
task_id: "0131"
status: inbox
iteration: 0
expect-reply-within: 24h
---

# 0131 — Base MQTT bringup on 0120 firmware

## Context

- Re-file of 0097, which blocked on 2026-10-02 for two reasons: (a) two runtime secrets no unattended run could obtain, and (b) a defect in the 0094 firmware that would have stopped the flash-and-configure path. Reason (b) is now moot — 0120 (task 0125, staged 2026-10-08, CI green) supersedes 0094 entirely.
- The two secrets are still needed: the WiFi SSID/password the base must join, and base-01's broker password. Per the standing pattern (set 2026-10-08), Stephen stages them at `/home/astroboy/.hermes/` — read them from there, never commit them, never paste them into a staged reply or log.
- Bench state: base board on USB at Stephen's bench; 0120 firmware is built but the GPIO16/GPIO4 pin decision (from 0125's reply) is still open — do NOT flash until Stephen decides the pin. This task is the MQTT bringup leg only, after the pin decision lands.
- 0097's full blocked record (bench reads, secret-absence proof, flash-dump attempt) is preserved in `mailbox/archive/0097-base-mqtt-bringup.md`.

## Task

Bring up the base station's MQTT uplink on the 0120 firmware:

1. Read the WiFi SSID/password and base-01's broker password from `/home/astroboy/.hermes/` (Stephen stages them there).
2. Once Stephen has decided the 0125 pin question and the board is flashed with 0120, configure the base via its captive portal (portal writes credentials to NVS — no credential ever touches the repo).
3. Prove the uplink: base joins WiFi, connects to mqtt.nordtronics.io:8883, and publishes as base-01 (verify in the broker journal).
4. Do NOT change ACLs, mosquitto config, or services on the VPS. Do NOT rotate the broker password.

## Success criteria

- Staged reply shows the base joined WiFi, MQTT CONNECT succeeded as base-01, and a PUBLISH was observed in the broker journal.
- Proof: staged reply with the observed broker-journal lines and the portal-config evidence (no secret values).

## Constraints

- Secrets come only from `/home/astroboy/.hermes/`; never commit, log, or stage them.
- No VPS-side changes (no ACL, config, or service edits).
- No flash until Stephen decides the 0125 pin question.
- Cost: use your cheapest model tier for this task. Do not use premium/pro models.

## Proof

Staged reply per `mailbox/README.md` with the broker-journal evidence; no branch or CI run needed unless firmware changes are required (they should not be — 0120 is built).

## Reply format

Stage with front-matter. Body: WiFi join result, MQTT CONNECT/PUBLISH evidence from the broker journal, explicit "no secrets committed, logged, or staged" and "no VPS changes made."
