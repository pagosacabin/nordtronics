---
task_id: "0232"
protocol_version: 1.3.0
status: staged
iteration: 1
proof: []
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
notes: |
  DONE — skill written and registered; the fresh-session retest produced a config that
  `esphome config` validates. Host-only task, `proof: []` per its own Proof block.

  SKILL. Name `esphome-device-skeleton`, path
  ~/.hermes/profiles/cronrunner/skills/software-development/esphome-device-skeleton/SKILL.md
  (168 lines). It is in the *cronrunner* profile's store because that is the profile this
  worker runs in; the default profile's skills tree (~/.hermes/skills/) is untouched —
  checked, no esphome-device-skeleton there. To install it for the default profile the
  command is `cp -a ~/.hermes/profiles/cronrunner/skills/software-development/
  esphome-device-skeleton ~/.hermes/skills/software-development/` (or re-create it with
  skill_manage from a default-profile session); I did not write into another profile.

  SOURCE READ IN FULL, then encoded in my own structure: esphome-base.yaml, README.md and
  BUILD-AND-PROVISION.md from origin/hermes/0213-esphome-skeleton @ bd0183d (the branch tip
  has moved past the 668f244 that 0213's notes cite; the code files are unchanged, the tip
  moved by 0216's addendum). Confirmed again that `main` carries **0** files under
  `firmware/skeleton/` — the skeleton is branch-only, so the skill names the branch as its
  source of truth.

  WHAT THE SKILL COVERS. The three layers with an explicit table of what belongs in each;
  the captive-portal-only WiFi rule with the 0208 history that earned it; the
  broker-as-firmware-default rule (mqtt.nordtronics.io:8883 named as the canonical case,
  declared in L2, never a portal field — v1 itself still carries no MQTT, per the card);
  the derive-a-new-device walkthrough (copy, substitutions, merge project code, generate the
  key into an uncommitted 600 secrets.yaml, local esphome build, portal provisioning, HA
  adoption); and the new-board bring-up procedure as its own 7-check section (identify chip
  via esptool read_mac/flash_id, flash, fallback AP appears, portal serves, joins with test
  credentials, one L3 setting changes, the setting survives a reboot) with pass/fail
  evidence per check.

  FRESH-SESSION RETEST — TWO RUNS, AND THE FIRST ONE FOUND A REAL GAP.
  The retest subject was Newton (`agy -p`, one-shot, a new process) reading ONLY the skill
  file — a fresh session by construction, and one that never saw the 0213 branch. Declared
  deviation: it was Newton's fresh session, not a second Hermes session (`hermes -z` hangs
  to timeout on any tool call in this environment, so a fresh Hermes turn was not available
  without burning the run). What that proves: the skill alone is sufficient to produce the
  pattern. What it does not prove: Hermes' own first-person recall of it.
    Run 1 produced a pattern-faithful config that FAILED validation:
      `esphome config` -> ERROR Error while reading config: Invalid YAML syntax:
      Duplicate key "sensor" in soil-moisture-sensor.yaml, line 80 (previous declaration
      line 47) — it had appended a second top-level `sensor:` block below layer 3 while
      layer 2 already declares one.
    That is a genuine omission in the skill (it said "append project code below the
    skeleton" without saying top-level keys must be merged), so the skill was patched with
    the rule, quoting the exact error. Run 2, on the patched skill, merged its project
    sensors into the existing `sensor:` list (its own comment: "Project sensors merged into
    existing sensor key to avoid duplicate key error") and validated:
      `esphome config soil-moisture-sensor-2.yaml` -> INFO Configuration is valid!
    on ESPHome 2026.9.1, with a generated 44-char secrets key. The config is reproduced
    verbatim in the reply body. `esphome compile` was NOT run for the retest config (the
    criterion is the pattern, and a first-time esp-idf platform build would not have
    finished inside this run); 0213 already carries the skeleton's own compile proof.

  NO BOARD WAS TOUCHED — nothing flashed, nothing generated on hardware. Scratch work stayed
  in ~/.hermes/profiles/cronrunner/cache/scratch/0232-test/.

  NO PROPOSED CHANGES TO THE 0213 PATTERN. The skill encodes it as written; the only
  additions are operational (the duplicate-key rule and the bring-up checklist), both of
  which are consistent with the branch's README rather than redesigns of it.

  Cost fields `unknown`: the cron harness records per-fire usage only at completion. Run
  2026-11-20 15:24-15:32 MST, off-peak.
---

# Response — 0232 the ESPHome skeleton, encoded as a skill

**Status: done — the skill exists, carries the new-board bring-up procedure as its own
section, and a fresh session using only the skill produced a config that `esphome config`
validates.** Host-only task, so `proof: []` per the card; the skill path and the retest
config below are the evidence.

## Skill

- **Name:** `esphome-device-skeleton`
- **Path:** `~/.hermes/profiles/cronrunner/skills/software-development/esphome-device-skeleton/SKILL.md`
- **Registered:** `skill_view` resolves it (`readiness_status: available`), 168 lines.
- **Profile:** written into the worker's own profile (cronrunner). The default profile's
  skills tree is untouched — verified, nothing of that name under `~/.hermes/skills/`. To
  install it there: `cp -a ~/.hermes/profiles/cronrunner/skills/software-development/esphome-device-skeleton ~/.hermes/skills/software-development/`.

## What it covers, in one paragraph

The three layers with a table of what belongs in each — substitutions header (device_name,
friendly_name, board, setup_ap_password, skeleton_version), **L1** `wifi: ap:` +
`captive_portal:` carrying WiFi credentials *only*, **L2** the identical firmware defaults
(USB_SERIAL_JTAG logger, encrypted `api:` with `!secret api_encryption_key`, ota, SNTP,
uptime/wifi_signal/IP/restart diagnostics, and infrastructure addresses such as
`mqtt.nordtronics.io:8883` declared here rather than in the portal), **L3** per-device
template entities with `restore_value` + `optimistic` so values persist in NVS and need no
re-flash. It restates the portal-only WiFi rule with the 0208 history that earned it (a
staged passphrase cost a morning and produced no diagnosis), gives the derive-a-new-device
walkthrough, and ends with the **new-board bring-up procedure**: identify the chip with
`esptool read_mac`/`flash_id`, flash with `esphome run`, confirm the `<device_name> setup`
AP appears, join it and confirm the portal serves at 192.168.4.1, enter test credentials and
confirm the join and IP, change one L3 setting, reboot, confirm the setting persisted — each
with its pass/fail evidence line.

## Step 3 — the fresh-session config, verbatim

Produced by a fresh `agy -p` session that read **only** the skill file, asked for a
hypothetical new ESP32-C3 sensor board with soil moisture on GPIO3 and battery voltage on
GPIO4. This is the second run (see the gap below):

```yaml
substitutions:
  device_name: "soil-moisture-sensor"
  friendly_name: "Soil Moisture Sensor"
  board: "esp32-c3-devkitm-1"
  setup_ap_password: "setup-password"  # Guards only the setup portal; not a secret
  skeleton_version: "1.0.0"

esphome:
  name: "${device_name}"
  friendly_name: "${friendly_name}"
  project:
    name: "nordtronics.skeleton"
    version: "${skeleton_version}"

esp32:
  board: "${board}"
  framework:
    type: esp-idf

# ==============================================================================
# Layer 1 — Portal Provisioning (WiFi credentials ONLY)
# ==============================================================================

wifi:
  ap:
    ssid: "${device_name} setup"
    password: ${setup_ap_password}

captive_portal:

# ==============================================================================
# Layer 2 — Firmware Defaults (identical on every board)
# ==============================================================================

logger:
  hardware_uart: USB_SERIAL_JTAG

api:
  encryption:
    key: !secret api_encryption_key

ota:
  - platform: esphome

time:
  - platform: sntp
    timezone: America/Denver

text_sensor:
  - platform: wifi_info
    ip_address:
      name: "IP Address"

button:
  - platform: restart
    name: "Restart"

sensor:
  - platform: uptime
    name: "Uptime"
  - platform: wifi_signal
    name: "WiFi Signal"
    update_interval: 60s

  # Project sensors merged into existing sensor key to avoid duplicate key error
  - platform: adc
    pin: GPIO3
    name: "Soil Moisture"
    update_interval: 60s

  - platform: adc
    pin: GPIO4
    name: "Battery Voltage"
    update_interval: 60s

# ==============================================================================
# Layer 3 — Per-Device Settings (the project's own values)
# ==============================================================================

number:
  - platform: template
    name: "Moisture Threshold"
    id: moisture_threshold
    optimistic: true
    restore_value: true
    min_value: 0
    max_value: 100
    step: 1
```

Against the three criteria: **portal carries WiFi only** (no station SSID/PSK anywhere;
the AP password is labelled non-secret) — yes. **Broker is a firmware default, not a portal
field** — yes; it placed no infrastructure address in the portal (this board needs no broker,
and the skill's L2 rule is where it would go). **Device settings persist via NVS** — yes,
`optimistic: true` + `restore_value: true` on the L3 template number.

Validated, not assumed:

```
$ esphome config soil-moisture-sensor-2.yaml
INFO ESPHome 2026.9.1
...
INFO Configuration is valid!
```

## The gap the retest found (and the fix)

Run 1, on the freshly written skill, produced a pattern-faithful config that **failed**
validation:

```
$ esphome config soil-moisture-sensor.yaml
ERROR Error while reading config: Invalid YAML syntax:

Duplicate key "sensor"
  in "soil-moisture-sensor.yaml", line 80, column 1
NOTE: Previous declaration here:
  in "soil-moisture-sensor.yaml", line 47, column 1
```

It had appended a second top-level `sensor:` block below layer 3 while layer 2 already
declares one. The skill said "append project code below the skeleton" without saying that
top-level keys must be **merged**, so this was the skill's omission, not the reader's
mistake. The skill now carries the rule and the exact error message; run 2 merged its
sensors and validated. Both runs are reported — the first is why the second is worth
anything.

## Deviations

1. The retest subject was **Newton** (`agy -p`, one-shot, new process, reading only the
   skill file), not a second Hermes session: `hermes -z` hangs to timeout on any tool call
   in this environment, so a fresh Hermes turn was not available inside this run. It proves
   the skill is self-sufficient; it does not prove Hermes' first-person recall.
2. `--dangerously-skip-permissions` for those print runs, so headless file reads would not
   block on a TTY permission prompt.
3. `esphome compile` was not run against the retest config (pattern was the criterion; a
   first esp-idf platform build would not fit the run). The skeleton's own compile proof
   stays 0213's.
4. Installed into the worker profile only; the default profile was not written to.

## Cost

`unknown` — per-call DeepSeek usage is not visible inside a cron run (the harness writes
usage at completion); not estimated. Rate window was off-peak (15:24–15:32 MST).
