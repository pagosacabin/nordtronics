---
task_id: "0097"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
---

# 0097 — Bench bring-up: base station publishes live telemetry to the backend

# Context

The wildfire stack is four hops; Juno verified each one live on 2026-10-02
~17:00 MDT and exactly one is broken:

1. Node → base over LoRa: WORKING (bench-proven; node sends, base receives).
2. Base → backend over MQTT: BROKEN — user `base-01` has not connected to
   mqtt.nordtronics.io:8883 in 2+ hours; the ingest log shows no arrivals and
   no rejections. Silence, not errors.
3. Backend → API: WORKING (Juno's VPS test reading stored and served).
4. API → app: WORKING (0096 build installed; it correctly labels the old
   readings "Stale").

The 0094 unified firmware (on main) implements the MQTT uplink, but the
physical bench base is still running the morning's bench test firmware, which
has no publish leg. This task closes hop 2 on the bench hardware.

# Task

1. On the bench base board, get MQTT publishing working — Hermes's call at
   the bench: either flash the 0094 unified firmware from main and configure
   via its captive portal, or graft the publish into the running bench
   firmware. Do NOT break the working LoRa receive path either way.
2. Configure (Stephen provides at the bench; credentials NEVER go in the
   repo, the task file, or any commit):
   - WiFi SSID/password for the cabin network
   - Broker: mqtt.nordtronics.io, port 8883, TLS (embed ISRG Root X1 —
     the server's chain.pem is the intermediate and will NOT validate)
   - MQTT user `base-01`, password held by Stephen (Juno issued 2026-10-02)
   - Topic per node: `nordtronics/wildfire/<node-id>/telemetry`
   - JSON payload: `pm25`, `temperature_c`, `humidity_pct`, `battery_v`
     (all required, backend rejects without them), `node_id` matching the
     topic, `observed_utc` ISO-8601 Z
3. Verify end to end: base serial shows MQTT CONNECT + PUBLISH;
   `https://api.nordtronics.io/v1/nodes/<node-id>/readings` shows a fresh
   reading; the installed app shows it WITHOUT the Stale label.

# Success criteria

- A reading less than 15 minutes old, originating from the physical bench
  node (not a VPS test publish), visible in the installed app as non-stale.
- The LoRa link still works after the change (node data keeps arriving).

# Constraints

- No credentials in the repo, ever. If a password is needed in a file for
  the bench, it lives on Stephen's machine only.
- Do not regress the 0094/0095/0096 trees on main; this task changes bench
  firmware state, not the repo, unless a repo fix is genuinely required —
  then say so in the notes.
- Worker cost: standard tier, off-peak preferred. State the tier used.

# Proof

- Staged reply with: base serial lines showing CONNECT + PUBLISH (quoted),
  the fresh `GET /v1/nodes/<id>/readings` response (quoted verbatim), and an
  app screenshot showing the reading without the Stale label.
- `reply_format`: staged file per `mailbox/README.md` with proof pointers,
  scope extensions, self-caught defects, and anything left undone.

# Reply format

Stage `mailbox/staged/0097-base-mqtt-bringup.md` per `mailbox/README.md`.
