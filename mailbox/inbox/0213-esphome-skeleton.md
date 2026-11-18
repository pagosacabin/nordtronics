# 0213 — ESPHome device skeleton: three-layer base for every ESP32 build

## Assignment
- Hermes: build the skeleton and README on a branch in the nordtronics repo; drive the Copilot peer review via Herdr.
- Copilot: peer reviewer for this GitHub-bound work (reviewer only — Juno still verifies independently).
- Juno: verifies the branch, files, review, and compile proof; then files the follow-on skill task.
- Stephen: no action in this task. He provisions the first real board through the portal when the skeleton gets its first use.

## Context
- On 2026-11-18 the propane bench build (0208–0212) burned a morning on a staged-password hunt: WiFi credentials typed into a file, read by Hermes, flashed as secrets, ~190 failed join attempts before the cause was cornered. Stephen's call, same day: captive-portal provisioning from now on, and a reusable ESP32/ESPHome skeleton so every build starts from the same proven base instead of a blank page.
- This generalizes Stephen's 2026-10-03 wildfire rule: the portal configures WiFi credentials (and node settings there); infrastructure addresses are firmware defaults, never portal fields.
- Reference builds: `firmware/propane-sensor/propane-bench.yaml` (0208, ESPHome 2026.9.1) and the November C4002 presence build. Target the current ESPHome defaults.
- Stephen also wants this to become a skill Hermes holds (his words: "this needs to be a skill that Hermes learns"). That is a SEPARATE follow-on task Juno files after this skeleton is verified — per the teaching ladder, he encodes only verified procedures. Do not write the skill in this task.

## Task
Create `firmware/skeleton/` in the nordtronics repo with two files:

1. `firmware/skeleton/esphome-base.yaml` — the skeleton, organized in three clearly commented layers:
   - **Substitutions header** at the top: `device_name`, `friendly_name`, `board` (default `esp32-c3-devkitm-1`; comment noting S3 boards swap this line). Everything project-specific is added below the skeleton, never inside it.
   - **Layer 1 — Portal provisioning:** `wifi:` with a fallback AP plus the `captive_portal:` component. NO station credentials baked in — no house SSID/password in the config or any committed secrets file. The fallback AP gets its own setup password, defined in substitutions, plainly labeled in a comment as guarding only the setup page (not a real secret).
   - **Layer 2 — Firmware defaults (identical on every board):** native `api:` with encryption (the key is a per-device value supplied from a local, uncommitted secrets file at flash time — the YAML references it via `!secret`; commit only a `secrets.yaml.example` with placeholder values), `ota:` (ESPHome platform), `logger:`, SNTP `time:`, and the standard diagnostics: uptime sensor, WiFi signal sensor, IP address text sensor, restart button. Set the `esphome:` project name/version (nordtronics skeleton v1).
   - **Layer 3 — Per-device settings pattern:** a marked section with ONE worked example — a persisted template `number` with `restore_value: true` standing in as an example threshold — plus a comment block stating the rule: each project declares its own settings here (thresholds, offsets, intervals, toggles); all persisted on the board, all adjustable from Home Assistant after adoption. No other example entities.
2. `firmware/skeleton/README.md` — short: the three layers and why; how to start a new device (copy the base, set substitutions, add project code below the skeleton, generate the per-device API key into a local secrets file, flash, provision WiFi at the portal — the steps Stephen does on his phone — then adopt in Home Assistant); the rule that house credentials are never committed or staged in files again.

Open a PR with the branch and complete the Copilot peer-review step before staging the reply; address review comments or note why not in the reply.

## Success criteria
- `esphome config` validates the skeleton as committed (with the example secrets) and `esphome compile` succeeds for the default C3 target. Report the ESPHome version used and the compile result.
- No house WiFi credentials anywhere in the committed files; the only passwords present are the labeled setup-AP password and placeholder secrets.
- Copilot peer review completed, with its pointer in the reply.
- PR opened against main.

## Constraints
- Keep v1 minimal: no project sensors, no MQTT, no web_server, no example code beyond the one settings-pattern number. Start small, prove the base, extend later.
- Do NOT touch the propane board or `firmware/propane-sensor/` in this task (0212 is resolving that board separately). Do NOT retrofit the presence sensor.
- No real secrets in the repo, the task reply, or logs quoted in the reply.
- Cost: standard tier for build work; no premium models.

## Proof
- Branch name + head SHA on origin, PR URL, ESPHome compile output excerpt (version + success line), Copilot review pointer.

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, the proof pointers, notes field with anything Juno should know (deviations, review outcome).
