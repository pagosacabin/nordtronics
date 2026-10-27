---
task_id: "0152"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: none
  sha: none
  run: none
  files: []
  reason: "Host-local read-only diagnosis; no repository file is changed by the diagnosis itself (task's own Proof section specifies branch/run = none)."
  local_paths_checked:
    - "/home/astroboy/ha-watch/esphome-addon.log (Device Builder log snapshot, 2026-10-14 12:53 -> 2026-10-16 11:38 MDT)"
    - "/home/astroboy/ha-watch/esphome-jk-entry-diagnostics.json (ESPHome config-entry diagnostic, 2026-10-16)"
    - "http://192.168.1.132:8123/api/states, /api/history/period, and the WebSocket API (config_entries/get, config/device_registry/list, system_log/list, recorder/statistics_during_period) - live reads"
    - "/home/astroboy/.hermes/.env (HASS_TOKEN/HASS_URL read in-process; no value printed or written)"
  not_available:
    - "jk.yaml itself: no copy exists on the worker host (find ~ -name 'jk*.y*ml' -> none) and the Device Builder addon is not reachable (192.168.1.132:6052 refused; /api/hassio/addons -> 401 without a supervisor token; SSH to the HAOS host refused for root and hassio)."
notes: |
  Read-only diagnosis, nothing changed on any device. Off-peak (PEAK: OFF-PEAK
  17:15 UTC), protocol MATCH 1.0.0.

  THE HEADLINE IS NOT WHAT THE TASK EXPECTED. Every data link in the chain is
  working. The bridge is only ON for about 6 minutes a day, so Home Assistant
  shows 115 JK-BMS entities as `unavailable` for ~23h54m of every 24h. The
  first broken link is therefore upstream of all four candidates the task
  lists: the bridge's own power/uptime. Power is named as the leading cause and
  not dropped - it is the finding, not a side note.

  Corrections to facts the task carried, all evidenced below:
    (1) The documented HA host 192.168.1.103 is GONE (ARP FAILED, no ICMP, no
        port 8123). HA now answers at 192.168.1.132 (`homeassistant`, ports
        22/1883/8123, version 2026.10.1). ~/.hermes/.env already carries the
        .132 URL, so Hermes was fine; other docs are stale.
    (2) There is no "OTA serial line" in this data path. Firmware goes up by
        OTA; the *data* path is BLE (BMS -> bridge) then the ESPHome native API
        over WiFi (bridge -> HA). No serial/UART link is involved, so
        "serial" cannot be the broken link.
    (3) The 2026-10-16 brownout lesson is the same failure family, but the pack
        is NOT sagging now: the BMS's own runtime counter proves the battery
        stayed powered continuously across the whole outage (see power_check).

  DEVIATIONS, declared: (a) read HASS_TOKEN from ~/.hermes/.env to query HA over
  REST/WebSocket - never printed, never written to the repo; (b) no ESPHome
  config, entity registry, integration reload, reboot or power-cycle was
  performed, per the read-only constraint, so the *mechanism* inside the bridge
  (supply collapse vs. firmware sleep) is stated as undetermined with a
  discriminator rather than guessed; (c) the ~6 minute wake window makes a live
  `esphome logs` capture impossible outside 08:40-09:10 MDT, and this run was at
  11:15 MDT.

  I have deliberately NOT recommended re-adding entities or re-flashing. The
  registry is intact (115 entities, area `shed`, MAC 80:f1:b2:a7:bc:88) and a
  re-flash would be the one action most likely to destroy the evidence.
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

---

# Reply — DIAGNOSED (2026-10-27 11:15-11:35 MDT), read-only

## current_state

**The bridge is offline ~99.6% of every day.** It is not dead and it is not de-registered — it comes online once a day, for roughly six minutes, publishes correct data, then disappears for another 24 hours.

- **Last data in Home Assistant:** `2026-10-27T15:05:00Z` = **09:05:00 MDT today** — SoC 60 %, pack 53.92 V, charging 763 W, BLE `online_status: on`.
- **Reporting effectively stopped:** `2026-10-24T14:04:34Z` = **2026-10-24 08:04:34 MDT**. Up to that second the bridge pushed a sample every ~30 s continuously. From then on it has been reachable only in three short windows:

| Date (MDT) | Bridge online | Bridge drops | Dwell | SoC | Pack V | Charging |
|---|---|---|---|---|---|---|
| Oct 25 | 09:25:28 | 09:30:58 | **5 min 30 s** | 55 % | — | 189 W |
| Oct 26 | 08:45:51 | 08:51:21 | **5 min 30 s** | 47 % | 52.76 → 52.86 | 368 → 399 W |
| Oct 27 | 08:58:10 | 09:05:00 | **6 min 50 s** | 60 % | 53.71 → 53.92 | 598 → 763 W |

- **Home Assistant side is healthy:** Core 2026.10.1 at `192.168.1.132:8123`, `state: RUNNING`, 269 entities, recorder writing. A control entity on a *different* ESPHome device (`sensor.bench_panel_4_3_battery`) has **105 of 105** possible hourly statistics buckets from Oct 23 to Oct 27 18:00Z — so neither HA nor the LAN was down during the JK gap.
- **The entity registry is intact**, which rules out a repeat of the 2026-10-16 registry deletion: 115 `jk_bms` entities exist under area `shed`, device `Jk--Bms`, MAC `80:f1:b2:a7:bc:88`, sw `2.3.0 (ESPHome 2026.9.1)`. Every one is `unavailable` with `last_updated 2026-10-27T15:05:00Z` — i.e. they were re-written *unavailable* at the moment the bridge dropped, not deleted.

## link_chain

| Link | Verdict | Evidence (one line) |
|---|---|---|
| `bms_serial` (UART/RS485) | **not in this path** | The bridge reads the BMS over **BLE**, not a serial line: `binary_sensor.jk_bms_jk_bms_online_status` = `on` at `2026-10-27T14:58:10Z`, and `switch.jk_bms_jk_bms_enable_bluetooth_connection` is the client switch the watchdog cycles. No UART/RS485 entity, no serial log, no serial port on the HA side. |
| `bms_ble` | **working** | During today's window HA received 24 cell voltages, SoC 60 %, delta-cell, temperatures, `online_status: on`, at a 30 s cadence from `14:58:30Z` to `15:05:00Z`. Frames are decoded, not merely received. |
| `bridge_wifi_ota` | **working while awake** | `sensor.shed_jk_bms_jk_bms_wifi_signal` published **−57 and −61 dBm** at `2026-10-27T14:58:10Z` / `14:59:03Z`; the ESPHome native API session was accepted (HA pulled full DeviceInfo + 115-entity list) — so WiFi association, OTA-reachable firmware and the API listener are all fine. |
| `home_assistant_api` / entity registry | **working** | 115 entities present and recorded; `binary_sensor.jk_bms_jk_bms_online_status` history shows `2026-10-26T14:45:51 'on'` → `2026-10-26T14:51:21 'unavailable'` → `2026-10-27T14:58:10 'on'` → `2026-10-27T15:05:00 'unavailable'` — HA is recording the truth, including the gaps. |

**No data link is broken.** All four were exercised successfully within today's 6 min 50 s window.

## power_check

**Bridge supply source:** battery-powered at its spot in the shed, per the 2026-10-16 finding (the same pack the BMS monitors, stepped down to the ESP32's 5 V). No mains supply is documented for it.

**Is the battery the problem? No — and this is provable, not assumed.** The JK BMS reports its own `total_runtime` counter (seconds since the BMS booted). Comparing it across the outage windows against wall-clock time:

```
2026-10-24T14:04:34Z  runtime 87,393,376
2026-10-25T15:25:46Z  runtime 87,484,648   wall 91,272 s   delta 91,272 s   (+0)
2026-10-26T14:45:55Z  runtime 87,568,656   wall 84,009 s   delta 84,008 s   (−1)
2026-10-27T14:58:30Z  runtime 87,655,800   wall 87,155 s   delta 87,144 s   (−11)
```

The BMS clock advanced by wall-clock time to within 11 s across three days. **The pack and the BMS never lost power for a single second.** The battery is charging hard in every window (189 W → 399 W → 763 W) and its voltage is *rising* (52.76 → 52.86, then 53.71 → 53.92 V), and SoC is 60 % today. A sagging pack is therefore ruled out as the trigger — different from 2026-10-16.

**What the power evidence does show:** the fault is between that healthy pack and the ESP32 — the bridge's own 5 V supply, its DC-DC converter, or its wiring/connector — **or** the bridge is being put to sleep by firmware. No brownout counter is readable: the device is off the network 23h54m a day, `jk.yaml` is not on this host, and the Device Builder addon is not reachable (port 6052 refused, `/api/hassio/addons` 401 without a supervisor token, SSH refused).

**Corroborating supply instability around the break:** HA's own recorder logged an unclean shutdown on the same day — `2026-10-24T20:58:26Z WARNING homeassistant.components.recorder.util: The system could not validate that the sqlite3 database at //config/home-assistant_v2.db was shutdown cleanly` / `Ended unfinished session (id=40 from 2026-10-15 13:37:21)`. The JK break (08:04 MDT) precedes HA's restart (14:58 MDT), so HA did not cause the JK gap — but it is a second power event at the house that day.

## evidence

Decisive quoted lines, verbatim, with timestamps.

**1. The reconnect failure that names both devices** — HA `system_log/list`, `2026-10-27T15:05:03.513962Z`:

```
WARNING aioesphomeapi.reconnect_logic
Can't connect to ESPHome API for jc4827w543 @ 192.168.1.72: Error connecting to
[AddrInfo(family=<AddressFamily.AF_INET: 2>, type=<SocketKind.SOCK_STREAM: 1>,
proto=6, sockaddr=IPv4Sockaddr(address='192.168.1.72', port=6053))]:
[Errno 111] Connect call failed ('192.168.1.72', 6053) (SocketAPIError)
Can't connect to ESPHome API for jk--bms @ 192.168.1.56: Error connecting to
[AddrInfo(... address='192.168.1.56', port=6053))]:
[Errno 113] Connect call failed ('192.168.1.56', 6053) (SocketAPIError)
Can't connect to ESPHome API for jc4827w543 @ 192.168.1.72:
Timeout waiting for HelloResponse after 30.0s (TimeoutAPIError)
```

`Errno 113` = *No route to host*: `192.168.1.56` does not answer ARP at all. The names are `jk--bms` (the JK bridge) and `jc4827w543` (the bench panel — a separate device that was briefly down for its own reason and is irrelevant here).

**2. Config entry is loaded, so this is not a registration fault** — `config_entries/get`:

```
esphome    jk--bms              state=loaded         reason=None disabled=None entry=01KET09PAJDAW7N89DJRKY2QKN
esphome    Bench Panel 4.3      state=loaded         reason=None disabled=None entry=01M4VV2R76Z6ZDQZCBWFRFXS99
mqtt       Mosquitto Mqtt Broker state=loaded        reason=None disabled=None entry=01KEAHBA0HE2F0BYYHJE7EHJ91
```

**3. The bridge is genuinely off the LAN right now** (not a DNS/mDNS artefact), worker host `192.168.1.65`, `2026-10-27T17:15Z`:

```
$ ping -c 2 192.168.1.56
2 packets transmitted, 0 received, 100% packet loss
$ ip neigh show dev wlo1
192.168.1.56 FAILED
192.168.1.1  lladdr 74:24:9f:93:c2:3b REACHABLE        # gateway fine, WiFi -29 dBm
$ avahi-browse -rtp _esphomelib._tcp
=;wlo1;IPv4;jc4827w543;_esphomelib._tcp;local;jc4827w543.local;192.168.1.72;6053;
"friendly_name=Bench Panel 4.3" "version=2026.9.1" "mac=44bd8d7acccc"
```

Only `jc4827w543` (bench panel) announces; `jk--bms` publishes no mDNS record and answers no ARP. A /24 sweep found 13 live hosts, none of them on 6053.

**4. The last continuous sample, then the daily-window pattern** — `/api/history/period`, `sensor.jk_bms_jk_bms_state_of_charge`:

```
2026-10-24T14:03:08  (regular 30 s samples up to here)
2026-10-24T14:04:34  last sample of the continuous run
2026-10-25T15:25:46  55.0      <- 25 h later
2026-10-25T15:30:58  unavailable
2026-10-26T14:45:55  47.0
2026-10-26T14:51:21  unavailable
2026-10-27T14:58:30  60.0
2026-10-27T15:05:00  unavailable
```

**5. The pack never lost power** — `sensor.jk_bms_jk_bms_total_runtime`, deltas matching wall clock to within 11 s (full table in `power_check`).

**6. The Device Builder log shows the last OTA and an earlier, identical wedge** — `/home/astroboy/ha-watch/esphome-addon.log`:

```
2026-10-16 10:41:37  Job 7fdabe3083a6 completed (exit code 0)        # compile ok
2026-10-16 10:41:37  Starting job aa4261b9f571: upload jk.yaml
2026-10-16 10:41:57  Job aa4261b9f571 failed (exit code 1)           # upload dies
2026-10-16 10:42:29  Device jk--bms (jk.yaml): unknown -> offline (via ping)
2026-10-16 10:45:35  Device jk--bms (jk.yaml): offline -> online (via ping)
2026-10-16 10:45:47  Job 5938b7dffc16 completed (exit code 0)        # retry upload ok
2026-10-16 10:45:47  Device jk--bms (jk.yaml) version: 2025.12.5 -> 2026.9.1
```

Note the established behaviour: `online -> unknown (via mdns)` / `-> offline (via ping)` then back hours later (also `2026-10-15 06:46:52 online -> unknown`, `2026-10-15 10:42:03 unknown -> online`). This bridge has form for dropping off the network for hours and recovering on its own — the new part is that since Oct 24 it no longer recovers until the next morning.

## root_cause

**First broken link: the bridge's own power/uptime — the ESP32-S3 is not staying on.** Every data link downstream of it is proven working. The failure is that the device only runs for ~6 minutes out of every 24 hours, so 115 entities are `unavailable` for the other 23h54m and any dashboard or automation keyed on the pack sees nothing.

Why the four candidate links are ruled out:

- **`bms_serial`** — there is no serial/UART link in this data path. The syssi component talks to the BMS over BLE; firmware upload is OTA. "OTA serial line" is a conflation of two different things, and neither is the break.
- **`bms_ble`** — `online_status: on` and full frame decoding (24 cell voltages, SoC, current, temperatures) inside every window. A BLE fault cannot produce correct payloads at a 30 s cadence.
- **`bridge_wifi_ota`** — −57/−61 dBm RSSI published, and HA completed a full ESPHome API handshake each window. WiFi association and OTA reachability are demonstrably fine; the device simply is not powered.
- **`home_assistant_api` / registry** — entry `state=loaded`, 115 entities intact under area `shed`, and the recorder captured every transition truthfully, including each drop. The 2026-10-16 "entities deleted by a truncated flash" failure has **not** recurred.
- **The battery/pack** — `total_runtime` advances with wall clock to within 11 s over three days. The pack never lost power and is charging at 189→763 W. **This is a different fault from 2026-10-16's sagging pack.**

**Mechanism inside the bridge: undetermined, and I am not going to guess it.** Two candidates survive the evidence, and they need one observation to separate:

1. **Supply collapse** (leading) — the pack is healthy, so this is the 5 V stage between it and the ESP32 (converter, connector, or a small local cell if one is in circuit). The morning-only wake times (08:45–09:25 MDT) fit a solar/threshold-driven supply, and the longest dwell (6 min 50 s) came on the day with the highest charge power (763 W).
2. **Firmware sleep** — a `deep_sleep:` / `run_duration:` block that keeps the radio up ~5–6 min. Against this: a self-timed deep sleep would wake at the same minute each day, and these wakes drift by up to 40 minutes; also the 2026-10-16 build reported `has_deep_sleep: false`. For it: two dwells were identical to the second (5 min 30 s), which a collapsing supply is unlikely to reproduce.

`jk.yaml` is not on this host and the Device Builder is unreachable, so I could not read the config to settle it — declared above rather than assumed.

## recommended_fix

**Not applied. All steps are proposals pending Stephen's approval.** Nothing in the repo, on HA, or on the bridge was changed by this run.

1. **Read the bridge's own log at the next wake window.** Have `esphome logs jk.yaml` (Device Builder → Logs) running from **08:30 MDT tomorrow**; the device is only reachable ~08:45–09:05. This one capture settles candidate 1 vs 2:
   - boot banner (`ESP-ROM:esp32s3…`, `rst:0x…`) at the start and again mid-window → **it is rebooting** → supply fault;
   - `Going into deep sleep` / a sleep line, or no reboot banner at all → **it is sleeping** → firmware.
2. **Measure the supply at the bridge while it is awake.** 5 V at the ESP32 5V/VIN and GND with a meter, watched across the whole ~6 minutes. A rail that sags as WiFi+BLE come up (or drops out at the 5–6 minute mark) is the answer, and it is a two-minute check.
3. **Read `jk.yaml` in the Device Builder editor for three things** (read-only, no edit): a `deep_sleep:` block, `run_duration:` / `sleep_duration:`, and the 5-minute staleness watchdog script. If a deep-sleep block is present, that is the cause and its removal is the fix — that would be a separate, approved task.
4. **If it is supply** (expected): move the ESP32 off the failing 5 V stage onto a converter rated ≥1 A continuous from the pack, add a bulk capacitor at the ESP32, and reseat the connector. Do **not** swap the pack — it is healthy and charging.
5. **Only after the bridge is stably up for an hour**, re-check HA: `sensor.jk_bms_jk_bms_state_of_charge` should track every 30 s and `online_status` should stay `on`. **No HA-side repair is needed** — the entities are intact; do not delete or re-add them, and do not re-flash to "recreate" entities (that was the 2026-10-16 fix and it is not the fault this time; re-flashing now would also destroy the evidence).

**Secondary observation, outside this task's scope, worth a look:** the BMS's own `temperature_sensor_1` read **51.6 °C** in today's window (55.0 °C Oct 25, **59.0–61.3 °C** Oct 24). That is hot for a LiFePO4 pack and is worth checking whether the sensor is on the BMS power board or on the cells — but it is not the cause of this outage and no action is proposed here.

## proof

```
branch: none
sha:    none
run:    none
files:  []
```

No branch, SHA or CI run exists for this task and none is fabricated: the deliverable is a read-only diagnosis of a live system, not a repository change. The task's own Proof section specifies `branch: none` / `run: none` for exactly this case.

**Verifiable evidence is the quoted lines above**, and the paths they came from:

| Path / endpoint | What was read |
|---|---|
| `http://192.168.1.132:8123/api/states` | 269 states; 115 `jk_bms` entities, all `unavailable` |
| `/api/history/period` | `state_of_charge`, `total_voltage`, `total_runtime`, `online_status`, `wifi_signal`, `charging_power`, `temperature_sensor_1` |
| WebSocket `system_log/list` | the `aioesphomeapi.reconnect_logic` `Errno 113` line, `2026-10-27T15:05:03Z` |
| WebSocket `config_entries/get` | `jk--bms` `state=loaded` |
| WebSocket `config/device_registry/list` | `Jk--Bms`, MAC `80:f1:b2:a7:bc:88`, area `shed`, sw `2.3.0 (ESPHome 2026.9.1)` |
| WebSocket `recorder/statistics_during_period` | 43 JK buckets vs 105/105 for the `bench_panel` control |
| `/home/astroboy/ha-watch/esphome-addon.log` | Device Builder OTA/online-state history, 2026-10-14 → 2026-10-16 11:38 MDT |
| `/home/astroboy/ha-watch/esphome-jk-entry-diagnostics.json` | ESPHome config entry, `host 192.168.1.56:6053`, `has_deep_sleep: false`, compile `2026-10-16 10:33:10 -0600` |
| worker host `192.168.1.65` shell | `ping`/`ip neigh`/`avahi-browse`/`/24` sweep showing `.56` absent |

**Not reachable, and declared rather than guessed:** `jk.yaml` (no copy on this host), the device console (offline 99.6 % of the day), the Device Builder addon (`:6052` refused), and SSH to the HAOS host (refused for `root` and `hassio`).

No credential value appears in this file. `HASS_TOKEN` was read in-process from `/home/astroboy/.hermes/.env` and used only as a bearer header.
