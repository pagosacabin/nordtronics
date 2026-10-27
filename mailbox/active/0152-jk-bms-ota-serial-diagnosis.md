---
task_id: "0152"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0152 — Diagnose JK-BMS data not reaching Home Assistant (OTA serial line)

## Context

Stephen's home battery monitor chain, last known good 2026-10-16 midday: JK BMS -> ESPHome bridge -> Home Assistant -> Guition panel, live at 100% SoC, 55.86 V, pushing 19.8 A.

Known bridge from 2026-10-16: ESP32-S3 ESPHome device, device name `jk--bms`, syssi/esphome-jk-bms component, JK02_32S protocol, throttle 30 s, wifi_signal sensor, and a 5-minute data-staleness watchdog that cycles the BLE client switch. A complete jk.yaml with about 116 entities was in use in ESPHome Device Builder, and Stephen uploads to the bridge OTA.

Known failure history from 2026-10-16, do not re-litigate as fact without fresh evidence: the bridge repeatedly wedged and needed manual reboots; the root cause found that day was power, not BLE/WiFi contention — the bridge is battery-powered at its spot, and a sagging pack browned out the ESP32-S3 under WiFi + BLE load. A good-battery swap restored it. A separate, also-resolved that day, issue was Home Assistant deleting entity-registry entries after a truncated config was flashed, leaving a blank dashboard instead of grey entities; re-flashing the complete jk.yaml recreated them.

Stephen's current report, 2026-10-27: the JK-BMS is not sending data to Home Assistant, and he identifies the failing path as the OTA serial line. The exact link he means by that phrase, and when reporting last stopped, are not on file — establish both from the live system first, do not assume BLE, UART serial, WiFi/OTA, or the Home Assistant API is the broken link.

## Task

Diagnose, read-only, why JK-BMS data is not reaching Home Assistant. One deliverable: a staged reply that names the first broken link in the chain, with log evidence for each link, and a recommended fix Stephen can approve separately.

## Success criteria

- The reply states, each with evidence, whether the bridge is online over WiFi/OTA, whether the bridge's logs show BMS frames being decoded, and whether Home Assistant has the JK-BMS entities and is receiving state updates.
- The reply names exactly one first broken link: BMS serial/UART link, BMS BLE link, bridge WiFi/OTA link, or Home Assistant API/entity-registry link — or states, with evidence, that no link is broken and reporting is in fact current.
- Power is checked first and reported explicitly: bridge supply source, and any sign of brownout or supply sag under load, per the 2026-10-16 lesson. Power must not be listed as a suspect and then dropped.
- The reply quotes the decisive log lines (ESPHome logs, and Home Assistant entity state where available), not summaries alone, with timestamps.
- The recommended fix is a concrete sequence of steps, and it is marked as not yet applied, pending Stephen's approval.

## Constraints

- Read-only diagnosis only. Do not flash the bridge, do not edit jk.yaml or any ESPHome configuration, do not change Home Assistant entity registry entries, and do not reboot or power-cycle any device except to observe its normal startup logs if that is already safe and reversible.
- Include no WiFi, API, OTA, or fallback credentials in the staged reply or in any file written to the repo. Read secrets only from Stephen's local machine where they already live; never print them and never write them into a task file.
- Use your default model tier for this diagnosis. Do not use a premium/pro model.
- One deliverable per this task: the diagnosis. Do not bundle a firmware fix, a config rewrite, or any other job into this task.

## Proof

Host-local diagnosis by design. No nordtronics repo change is expected from the diagnosis itself, so branch: none and run: none are the honest proof pointers, with the staged reply's quoted log lines and named local log/config paths as the verifiable evidence. Pasted terminal output inside the reply is evidence for Juno to check against the live system, not a substitute for the quoted log lines themselves.

## Reply format

Fill in these fields when staging:

- current_state: bridge online or offline, last data seen in Home Assistant, and when reporting last stopped, if determinable
- link_chain: for each of bms_serial, bms_ble, bridge_wifi_ota, home_assistant_api — working, broken, or unknown, with one evidence line each
- power_check: bridge supply source and any brownout/sag evidence
- evidence: decisive quoted log lines with timestamps
- root_cause: the first broken link, and why the other links are ruled out
- recommended_fix: concrete steps in order, marked not yet applied, pending Stephen's approval
- proof: branch: none, run: none, plus the local log/config paths checked
