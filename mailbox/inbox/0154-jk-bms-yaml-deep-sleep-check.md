---
task_id: "0154"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0154 — Read jk.yaml from HA esphome/ and settle the deep-sleep question

## Context

Task 0152 (staged 2026-10-27) diagnosed the JK-BMS bridge as online only about 6 minutes a day: last data 2026-10-27 09:05 MDT (SoC 60%, pack 53.92 V, charging 763 W), windows Oct 25 09:25-09:30, Oct 26 08:45-08:51, Oct 27 08:58-09:05 MDT. The pack is healthy (BMS total_runtime tracks wall clock within 11 s over 3 days), the Home Assistant registry is intact (115 entities, area `shed`), and Home Assistant now answers at 192.168.1.132, not 192.168.1.103.

The mechanism inside the bridge is still undetermined: 5 V supply collapse versus firmware sleep. In 0152 you reported you could not read jk.yaml (no copy on your host, Device Builder route failed). Stephen corrects that: you have access to the yaml, it lives in esphome/ in Home Assistant, and you have looked at it before. Use that route. Do not re-litigate access.

## Task

Read the current jk.yaml from Home Assistant's esphome/ location, read-only, and report exactly what it says about sleep and the watchdog. One deliverable: a staged reply settling the deep-sleep question.

## Success criteria

- The reply states, with quoted text, whether a `deep_sleep:` block is present in the current jk.yaml, or states explicitly that a full-file search found none.
- The reply quotes or lists `run_duration:`, `sleep_duration:`, and any other sleep/idle timing in the file, if present.
- The reply quotes the watchdog definition (the 5-minute data-staleness watchdog that cycles the BLE client switch, if present in this file) and any `on_boot` sleep-related lines.
- The reply gives a verdict: sleep-configured (firmware sleep is a live mechanism) or sleep-absent (firmware sleep is ruled out as configured, leaving supply collapse as the leading mechanism for 0152).
- The reply names the exact path read in HA esphome/ and the file's observed state (e.g. last-modified time if available).

## Constraints

- Read-only. Do not edit jk.yaml or any ESPHome configuration, do not flash the bridge, and do not reboot or power-cycle any device.
- Include no WiFi, API, OTA, or fallback credentials in the staged reply or in any file written to the repo. Read secrets only from Stephen's local machine where they already live; never print them and never write them into a task file.
- Use your default model tier. Do not use a premium/pro model.
- One deliverable per this task: the yaml read and verdict. Do not bundle a config fix, a flash, or any other job into this task.

## Proof

Host-local read-only diagnosis by design. No nordtronics repo change is expected from the read itself, so branch: none and run: none are the honest proof pointers, with the quoted yaml sections and the named HA esphome/ path as the verifiable evidence.

## Reply format

Fill in these fields when staging:

- yaml_path: exact esphome/ path read in Home Assistant
- deep_sleep: present (quote the block) or absent (state the full-file search)
- run_duration: value or absent
- sleep_duration: value or absent
- watchdog: quoted definition or absent
- other_sleep_related: any other sleep/idle lines, quoted, or none
- verdict: sleep-configured or sleep-absent, and what it means for the 0152 mechanism
- proof: branch: none, run: none, plus the HA esphome/ path checked
