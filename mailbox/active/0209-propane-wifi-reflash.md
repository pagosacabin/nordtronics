---
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T15:15:30+00:00"
lease_until: "2026-11-18T16:45:30+00:00"
---
# 0209 — Propane bench: reflash with real WiFi credentials

## Assignment
- **Hermes:** rebuild + reflash + network proof. Bench firmware, stays with you (not Newton).
- **Copilot:** peer review only if you commit a repo change; if nothing in the repo changes, say so and skip it.
- **Juno:** independent verification on the staged reply.

## Context
Task 0208 is closed and archived (branch `hermes/0208-propane-bench-firmware` @ ccece52, verified GREEN). Its one open network item: the flashed unit has no WiFi credentials — the laptop's only secrets.yaml was a template — so the device loops on WiFi retries, never joins the network, and Home Assistant shows nothing to adopt. Stephen has now staged the real credentials for you: a file he placed in `/home/astroboy/.hermes/` on the laptop on 2026-11-17 at ~18:10 MST (the newest file there is his staging). Read the SSID and password from that file directly. The values never go in this task, your reply, or the repo.

## Task
1. Build a real secrets file for the 0208 config (`firmware/propane-sensor/propane-bench.yaml`): `wifi_ssid` and `wifi_password` from Stephen's staged file. For `api_encryption_key`, REUSE the key generated during 0208 (it lives in your scratch secrets file from that run) so the device identity doesn't change; only generate a new one if that file is truly gone, and say so if you do.
2. Reflash the bench board with the resulting image (the YAML itself does not change — secrets are external).
3. Prove the network side on the serial log: the device joins the WiFi, receives an IP address, and the ESPHome API server comes up. Capture the relevant log lines.
4. Confirm both ADS1115 channels are still reporting after the reflash, and state the alarm threshold value the board boots with (0208 left 2.5 V in NVS — leave it as-is; Stephen tunes it in Home Assistant).

## Success criteria
- Serial log excerpt in the reply shows WiFi connected and the assigned IP address.
- The reply states the device's IP address (an IP is fine to print — it is not a credential).
- The reply names the exact file path of the secrets file that holds the `api_encryption_key`, so Stephen can copy the key into Home Assistant's adoption prompt himself. Path only — never the key, never the WiFi password, in the reply or anywhere else.
- Both sensor channels reporting in the post-reflash log.

## Constraints
- Credentials come only from Stephen's staged file in `/home/astroboy/.hermes/`. Never commit them, never echo them.
- Do not change the YAML's logic, pins, or thresholds for this task. If a repo change turns out to be necessary, branch `hermes/0209-propane-wifi-reflash` and route it through Copilot review per the standing rule.
- Cost: standard tier only.
- If the board is not enumerated when you pick this up, stage the reply saying so plainly rather than waiting silently.

## Proof
- Serial log excerpt: WiFi join + IP assignment + API server up.
- The secrets-file path for the API key (path only).
- Post-reflash channel readings (a few lines are enough).

## Reply format
Status line, then: IP address, secrets-file path for the API key, WiFi/API log excerpt, channel check, and anything blocked stated plainly.

expect-reply-within: 6h
