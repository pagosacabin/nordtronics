---
task_id: "0098"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
---

# 0098 — Base firmware must connect to WPA2/WPA3 mixed-mode APs with zero router changes

# Context

Bench diagnosis 2026-10-02 (Hermes, verified by Juno): the base board cannot
join the cabin Starlink router. The AP advertises mixed WPA2/WPA3
(auth_mode=7). The firmware builds on Arduino core 2.x / ESP-IDF 4.4, which
has WPA3 SAE compiled in (CONFIG_ESP32_WIFI_ENABLE_WPA3_SAE=y) WITHOUT H2E
support — so the board elects SAE, and the AP rejects it at the auth stage
(reason 202 AUTH_FAIL). A wrong passphrase cannot produce this error; the
reason-code reading is confirmed.

Stephen's product call: telling customers to flip their router to WPA2-only
is not a shippable answer. The base MUST join default-config routers —
including WPA2/WPA3 mixed mode, which is the out-of-box default on Starlink
and most current ISP gateways — with no router-side changes whatsoever.

# Task

Fix it firmware-side. Two options, Hermes picks and documents why:

(a) Disable WPA3 SAE in the build config
    (CONFIG_ESP32_WIFI_ENABLE_WPA3_SAE=n). The board then uses pure
    WPA2-PSK, which mixed-mode APs accept. Smallest change; the base has no
    need for WPA3. Recommended.
(b) Upgrade the firmware to Arduino core 3.x / ESP-IDF 5.x with full WPA3
    (H2E) support. Larger change, more regression surface.

If the fix touches the repo firmware tree, do it on a branch
(hermes/0098-...) with a normal CI build; if it is purely a local bench
config, say so explicitly in the notes.

# Success criteria

- The cabin Starlink router stays in WPA2/WPA3 mixed mode — DO NOT change
  the router for this task. That is the test AP.
- The base joins it, gets an IP, sets time via NTP, and completes the MQTT
  publish leg from 0097 (broker mqtt.nordtronics.io:8883, TLS, user base-01).
- Serial shows: WiFi connected (with the AP still in mixed mode) + MQTT
  CONNECT + PUBLISH.

# Constraints

- No router changes, no credentials in the repo (WiFi/MQTT secrets live on
  Stephen's machine only).
- Do not break the working LoRa receive path.
- Worker cost: standard tier, off-peak preferred. State the tier used.

# Proof

- Staged reply with: the option chosen and why, the config diff or core
  version change, and quoted serial lines showing WiFi connect + MQTT
  CONNECT + PUBLISH against the still-mixed-mode AP.
- `reply_format`: staged file per `mailbox/README.md` with proof pointers,
  scope extensions, self-caught defects, and anything left undone.

# Reply format

Stage `mailbox/staged/0098-base-wifi-mixed-mode.md` per `mailbox/README.md`.
