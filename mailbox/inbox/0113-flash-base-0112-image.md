---
task_id: "0113"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 12h
proof: []
notes: |
  Filed by Juno, 2026-10-05 ~15:20 MDT. Stephen's call: "queue the new base
  code". The node is healthy on the 0112 build (verified, archived); the base
  still runs 0105 with the MQTT keepalive problem. Same unified image both
  ends eliminates protocol mismatch as a variable. No new build needed — the
  0112 artifact is CI-green and already flashed to the node.
---

# 0113 — Flash the base to the 0112 unified image

## Context

The node (MAC B0:A6:04:C5:75:4C) is running the 0112 build: sleep gate off,
60 s checkin, LoRa TX every 60 s, stable on USB (0112 verified, archived).
The base (MAC 80:F1:B2:A7:47:EC) still runs the 0105 image: it connects to
mqtt.nordtronics.io:8883 then is disconnected for exceeding the keepalive
timeout ~30 s later, reconnects ~1/min, never publishes. 0109 also saw no
LoRa `rx:` lines on the base — but the node was not transmitting then.

Flash the base with the SAME 0112 image the node runs. The artifact is
already built and proven:
- Branch: hermes/0112-gate-node-deep-sleep @ 4f9c05e741c0980146fafb689e02a994787fde12
- Artifact: wildfire-node-v1-unified-firmware, id 11362491397
- firmware.bin sha256: 0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb
  (1215856 B)

The base's NVS is provisioned (0106: wifi connected, ip=192.168.1.71) —
an app-only flash preserves it. The unified firmware probes role at boot;
with no sensors the base resolves role=base and never sleeps.

## Task

1. Download the 0112 artifact (id 11362491397) and confirm firmware.bin
   sha256 = 0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb.
   If the artifact is expired or the hash mismatches, STOP and report —
   do not build a substitute without a new decision.
2. Read the base's partition table first (NVS safety, 0109's recipe).
3. Flash the BASE only (MAC 80:F1:B2:A7:47:EC, by-id path, never
   /dev/ttyACM0/ACM1): app-only at 0x10000, then read back and verify
   byte-for-byte against firmware.bin. No --erase-all, no erase_flash,
   NO NVS writes. Do NOT touch the node board in any way.
4. Observe the base serial for 10+ minutes, non-perturbing capture, and
   quote: the ROLE line (expect role=base), any `rx:` lines from the node
   (node TXes every ~60 s), and the MQTT connect/disconnect lines.

Cost: flash tier. Off-peak preferred; do not delay past the reply window.

## Success criteria

- Base running the 0112 image: quoted boot lines showing role=base and the
  0112 build string, read-back sha256 matching the artifact.
- Quoted `rx:` lines proving the base hears the node's LoRa packets, and
  quoted MQTT state lines (connected, or the keepalive disconnect if it
  persists — report what IS, not what should be).
- The node board received no write and no reset. No NVS writes anywhere.

## Constraints

- Base ONLY. App-only flash at 0x10000. No NVS writes, no base NVS wipe
  (its WiFi credentials must survive).
- If the base does not show `rx:` lines from the node within the observe
  window, report that honestly — do not reflash either board to chase it.

## Proof

- Artifact id + firmware.bin sha256 (pre-flash check).
- Read-back sha256 of the base's app0 matching firmware.bin.
- Quoted base serial: ROLE line, rx: lines (or their absence), MQTT lines.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
