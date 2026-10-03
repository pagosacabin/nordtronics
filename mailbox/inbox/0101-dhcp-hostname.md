---
task_id: "0101"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 72h
proof: []
notes: |
  Filed by Juno, 2026-10-03, at Stephen's direct direction from the bench:
  "We should give the esp32 an identifiable host name based off of purpose
  and number in sequence." Motivation: the old bench image's OLED shows
  nothing, so there is no on-device way to tell whether the base joined the
  cabin WiFi; a DHCP hostname makes every board identifiable in the router's
  client list with no screen and no serial cable.
---

# 0101 — wildfire-node-v1: DHCP hostname from purpose + sequence number

## Context

Stephen's bench rule: every ESP32 gets an identifiable DHCP hostname built
from its purpose and its sequence number, so a board is recognizable in any
router's client list without the OLED or a serial cable. The 0099 firmware
fixed the OLED bring-up, but a hostname is the independent identifier.

## Task

On branch `hermes/0101-dhcp-hostname` (from
`hermes/0099-wildfire-fix-set` tip `881ee68`; if 0100 has verified by
pickup time, branch from the 0100 tip instead and say so in the reply), in
`firmware/wildfire-node-v1`:

1. Before STA bring-up, call `WiFi.setHostname()` with
   `wildfire-<role>-<nn>` where `<role>` is `base` or `node` (from the
   radio role / NVS config) and `<nn>` is the zero-padded 2-digit sequence
   number: the node ID for nodes, `01` for the base when its node ID is 0
   (unprovisioned).
2. Log the chosen hostname once at boot (e.g. `net: hostname
   wildfire-base-01`).

Push the branch and take it green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and the
  `host-tests` job passes in the same run.
- Falsifiable: `grep -n "setHostname" firmware/wildfire-node-v1/src/main.cpp`
  shows the call placed before the STA connect sequence, and the format
  string produces `wildfire-base-01` / `wildfire-node-02` style names
  (quote the lines).
- `git diff` contains no credential, SSID, or password.

## Constraints

- Hostname is derived from existing config (role + node ID) — no new
  portal field, no new NVS key.
- Do not touch the LoRa receive path, the 0099 fixes, or any test file.
- One deliverable: the hostname. Nothing else.

## Proof

- Branch `hermes/0101-dhcp-hostname` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- The grep output above, quoted verbatim.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
