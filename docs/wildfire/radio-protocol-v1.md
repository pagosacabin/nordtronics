# Wildfire radio protocol v1

`protocol_version: 1` — the version byte is byte 0 of every frame.

Defines the LoRa link between a Wildfire Node v1 and a Wildfire base station
(Phase 1: another Heltec LoRa 32 V4, USB-powered, no sensors). The base runs
consensus/detection, the backend persists and serves, the app displays and
acknowledges. **915 MHz US (902–928 MHz) only** — the firmware refuses a
configured frequency outside that band, and there is no 433 MHz code path.

The normative implementations of everything below are
`firmware/wildfire-node-v1/src/radio_protocol.h` / `.cpp`. A test
(`firmware/wildfire-node-v1/test/test_radio_protocol`) asserts every offset, every
frame length and the golden frame hex in this document, so a field cannot be
added to the wire without changing this file too.

## 1. Frame layout

```
 offset  size  field                       notes
 ------  ----  --------------------------  -----------------------------------------
      0     1  version                     = 1; a receiver drops anything else first
      1     1  msg_type                    1 CHECKIN, 2 ALARM, 3 ACK, 4 JOIN_REQ, 5 JOIN_ACK
    2..3     2  node_id                     uint16 LE; 0 and 0xFFFF are invalid on the air
    4..5     2  seq                         uint16 LE, per-node, wraps at 65535
      6     1  payload_len                 must equal the type's fixed length (validated)
      7     1  flags                       header flags, below
    8..n     *  payload                     per type, below
   n..n+1     2  crc16                     CRC-16/CCITT (poly 0x1021, init 0xFFFF) over bytes 0..n-1
```

Little endian throughout; a `float` never goes on the wire (fixed-point/integer only).

### 1.1 Header `flags` (byte 7)

| bit | value | meaning |
|---|---|---|
| 0 | `0x01` | ALARM payload (`msg_type` = 2; both must agree) |
| 1 | `0x02` | ACK required — the sender will retransmit without one |
| 2 | `0x04` | battery below the `batt_low_mv` threshold |
| 3 | `0x08` | join pending (this node is not yet provisioned) |

### 1.2 `status` byte (last telemetry payload byte)

| bit | value | meaning |
|---|---|---|
| 0 | `0x01` | BME680 present |
| 1 | `0x02` | PMS5003 present |
| 2 | `0x04` | PMS5003 frame valid this cycle (checksum passed) |
| 3 | `0x08` | AS3935 lightning sensor present |
| 4 | `0x10` | lightning activity since the last check-in |
| 5 | `0x20` | a sensor read failed this cycle |

### 1.3 Telemetry payload — CHECKIN (1) and ALARM (2), 17 bytes

| off | size | field | units | one-line justification |
|---|---|---|---|---|
| 0 | 2 | `pm1_x10` | µg/m³ × 10 | PMS5003 gives PM1.0 in the same frame as PM2.5; free context for the backend, 0.1 µg/m³ resolution is finer than the sensor's |
| 2 | 2 | `pm25_x10` | µg/m³ × 10 | the detection quantity; the 25 µg/m³ rules need 0.1 resolution on a 16-bit field |
| 4 | 2 | `pm10_x10` | µg/m³ × 10 | distinguishes fine-particle smoke from coarse dust (the 0091 classification), same frame, no extra packet |
| 6 | 2 | `temp_c_x100` | °C × 100, signed | BME680 temperature; ×100 gives 0.01 °C, and signed covers a winter node |
| 8 | 2 | `rh_x100` | %RH × 100 | BME680 humidity; ×100 gives 0.01 %, 0..100 % fits in 16 bits |
| 10 | 4 | `press_pa` | Pa, unsigned | BME680 pressure in SI Pa (0..120000 fits u32); Pa with no scaling keeps the raw sensor value |
| 14 | 2 | `batt_mv` | mV | battery health; mV not volts so no float is needed, and 0 means "not measured" |
| 16 | 1 | `status` | bitfield | sensor presence/health without lengthening the frame |

### 1.4 ACK payload — ACK (3), 3 bytes

| off | size | field | justification |
|---|---|---|---|
| 0 | 2 | `acked_seq` | the node's sequence number being acknowledged, so a node with retries in flight can match the ACK to the right transmission |
| 2 | 1 | `ack_code` | 0 ok, 1 unknown node, 2 bad frame, 3 not permitted — a refusal is distinguishable from silence |

### 1.5 JOIN payloads — JOIN_REQ (4) / JOIN_ACK (5), 4 bytes each

| type | off | size | field | justification |
|---|---|---|---|---|
| 4 | 0 | 4 | `hw_hash` | 32-bit hash of the module MAC, so the base log can name an unprovisioned board without trusting a self-declared ID |
| 5 | 0 | 2 | `assigned_id` | the factory/bench-provisioned node ID the node must use |
| 5 | 2 | 2 | `base_seq` | the base's frame counter at provisioning, so a re-provisioned node can detect a base restart |

### 1.6 Encoded sizes

| type | payload | frame |
|---|---|---|
| CHECKIN | 17 | **27** |
| ALARM | 17 | **27** |
| ACK | 3 | **13** |
| JOIN_REQ | 4 | **14** |
| JOIN_ACK | 4 | **14** |

27 bytes at SF7 / BW125 / CR4-5 is ~62 ms of air time, which comfortably fits the
12-minute cadence and leaves the channel free for retries.

### 1.7 Golden frames

These exact byte strings are asserted by `test_radio_protocol`, so they are a
format check rather than an illustration.

```
CHECKIN  node=0x1234 seq=7 flags=0x02 (ACK required)
  pm1=1.2 pm25=13.7 pm10=42.1 temp=22.34C rh=45.12% press=101325Pa batt=3990mV status=0x07
  01013412070011020c008900a501ba08a011cd8b0100960f07e844      (27 bytes)

ACK      node=7 seq=9 acked_seq=0x0123 code=0 (ok)
  01030700090003002301009b25                                  (13 bytes)

JOIN_REQ node=7 seq=9 hw_hash=0xDEADBEEF
  0104070009000400efbeaddef261                                (14 bytes)

JOIN_ACK node=7 seq=9 assigned_id=0x0042 base_seq=0x0011
  01050700090004004200110066fe                                (14 bytes)
```

## 2. Radio parameters

All of these are captive-portal fields (firmware default + portal field + NVS),
stored in the single NVS namespace `wildfire`.

| field | NVS key | default | notes |
|---|---|---|---|
| frequency | `lora_mhz` | 915.0 | **902–928 MHz only**; the firmware prints a FATAL line and refuses any other band |
| bandwidth | `lora_bw` | 125.0 kHz | |
| spreading factor | `lora_sf` | 7 | |
| coding rate | `lora_cr` | 5 (= 4/5) | |
| sync word | `lora_sync` | 0x12 | private sync word; a public-network node cannot be received by accident |
| TX power | `lora_dbm` | 20 | |
| preamble | — | 8 | fixed; not exposed, it is a protocol constant |
| CRC on air | — | on | in addition to the in-frame CRC16 |

## 3. Join / provisioning flow

Node IDs are **bench-provisioned** (task 0094): the base never hands out an ID it
was not told about.

1. An unprovisioned node (`node_id` = 0 in NVS) may send `JOIN_REQ` with its
   hardware hash. Nothing is routed on the strength of it.
2. The base logs the hash and **ignores every frame from an ID that is not in its
   `prov_ids` allowlist**. A frame from an unprovisioned ID is counted and
   dropped before it reaches the consensus engine or MQTT.
3. Provisioning is a bench act: write `node_id` (and `prov_ids` on the base)
   through the portal, or `JOIN_ACK` for a node whose hash is already in the
   bench provisioning list.
4. An empty `prov_ids` allowlist means "bench open" — the base accepts any
   non-zero ID. This is only ever the state of a bench base; a deployed base
   carries the list.

## 4. ACK rules

- **Only alarm packets are acknowledged.** Routine CHECKINs are not ACKed — an
  ACK for every check-in would double the channel usage for no benefit, since a
  missing check-in is already detected by the offline rule.
- The node sets `flags |= ACK_REQUIRED (0x02)` and `msg_type = 2` on an alarm.
- ACK timeout: `ack_to_s` = **5 s** (portal field). A node waits 5 s for an ACK
  matching its `acked_seq`.
- Retries: `ack_tries` = **3** (portal field) further transmissions, i.e. up to 4
  attempts total, all in the same wake window. The node logs the outcome either
  way (`alarm seq=N ACKed` / `UNACKED (retries exhausted)`) — a delivery failure
  is never silent.
- The node raises an ALARM on its own hard threshold (PM2.5 ≥ 55 µg/m³ on a valid
  PMS frame). Consensus is the **base's** job; the node's alarm is a fast path,
  not a decision.
- An ACK with `ack_code != 0` is a terminal answer for that transmission.

## 5. Check-in cadence and the offline rule

- Routine check-in period: `chk_s` = **720 s = 12 min** (portal field). The 0092
  power budget closes at this cadence (219.3 mAh/day).
- A node deep-sleeps between check-ins. **A base station never deep-sleeps** — the
  firmware asserts `role == node` on every sleep path and refuses to sleep as base,
  logging a FATAL line instead, so a misdetected role fails loudly rather than
  silently deafening the property.
- **Offline rule: a node is marked offline after 3 missed check-ins (36 min).**
  The base publishes `node/<id>/state` `{"state":"offline"}` once, on the
  transition, and the OLED `last-seen` column stops advancing.

## 6. Detection thresholds (v0.2)

Fixed by the task and by 0091/0092:

| parameter | portal key | default | meaning |
|---|---|---|---|
| absolute floor | `abs_floor` | 25.0 µg/m³ | a packet can only be part of a rise at or above this |
| rise above baseline | `rel_delta` | 25.0 µg/m³ | `pm25 − frozen_baseline ≥ rel_delta` |
| confirmation | — | 2 packets | consecutive packets, both meeting the two conditions |
| correlation window | `corr_min` | **20 min** (range 10–60) | the sliding window that must hold ≥ 2 confirmed nodes |
| live age | `live_min` | 30 min | a node with no packet for this long does not participate |
| auto-clear | `clear_min` | 30 min | all participating nodes below threshold for this long clears the event |
| re-arm cooldown | `rearm_min` | 30 min | **v0.2** — no new Watch/Alert for this long after a clear |
| baseline window | `baseline_n` | 8 readings | the rolling median used until the freeze engages |
| ack suppression | — | 120 min | a manual acknowledge suppresses the same node set for 2 h |

The correlation window is the one parameter 0091 left open (it validated 20 min and
recommended widening with no number attached). **Default 20; the widen question is
flagged for Stephen and not guessed at.**

## 7. Baseline freeze (the v0.2 fix)

0091 measured that a v0.1 rolling-median baseline *chases* a slow incursion: the
burn-pile scenario's 2.0 µg/m³-per-packet ramp pulls the median up with it, the
relative term never fires, and the event is missed entirely.

v0.2 freezes each node's baseline **on the first packet at or above the absolute
floor** — the earliest point at which an incursion is even possible, and a point
the baseline cannot have chased, because the node's own history at that moment is
still clean. The frozen value holds while the node stays in the excursion, and is
released once the node has been back below the floor for `clear_min`.

Why the threshold for the freeze is the floor and not the relative rise: a
relative-rise trigger is by definition a packet the chased baseline has already
defeated. Measured on the 0091 burn-pile trace, freezing on the relative rise
never happens at all; freezing on the floor crossing raises a Watch at t = 264 min
against a ground-truth first rise at t = 60 (`test_consensus_native`).

## 8. MQTT uplink and the offline policy

Topics (root = `mqtt_root`, default `nordtronics/wildfire`):

| topic | payload | when |
|---|---|---|
| `<root>/<node-id>/telemetry` | JSON: node, seq, pm1, pm25, pm10, temp_c, rh, press_pa, batt_mv, status, level | every valid frame from a provisioned node |
| `<root>/event/watch` | `{"event":"WATCH","t":…,"nodes":"…"}` | a single node confirms |
| `<root>/event/alert` | `{"event":"ALERT",…}` | ≥ 2 nodes confirm inside the window |
| `<root>/event/clear` | `{"event":"CLEARED",…}` | the event auto-clears |
| `<root>/node/<id>/state` | `{"state":"offline"}` | 3 missed check-ins |

Broker host/port/user/pass/root are portal fields with defaults
(`mqtt.nordtronics.io:8883` — TLS only; the deployed broker has no 1883 listener,
and the uplink pins the ISRG Root X1 anchor in `src/mqtt_ca.h`).

**The telemetry topic carries exactly ONE level between the root and the leaf**
(`<root>/<node-id>/telemetry`), because the deployed ingest worker and the
Mosquitto ACL both subscribe `nordtronics/wildfire/+/telemetry` and `+` matches
exactly one level. It is built by `src/mqtt_topic.h` and nothing else, so no call
site can drift back to a two-level `<root>/node/<id>/...` shape that the broker
drops silently (task 0104 item 1).

**Offline policy: buffer with a cap, drop the OLDEST record** (`offl_policy` =
`buffer`, `offl_cap` = 180 records). Chosen over drop-new because the value of a
node's data is highest when the link has just been restored — the event that
matters is the one that happened while the broker was unreachable — and because
the oldest records in a rolling window are the least informative. 180 records =
15 node-hours for one node = **36 h for a 4-node property** at the 12-minute
cadence, and ~30 kB of the ESP32-S3's 2 MB PSRAM. On overflow the buffer evicts
from the head and publishes in order on reconnect.

## 9. Captive portal (tank-monitor pattern)

The firmware default lives in the `kPortalFields` table
(`src/firmware_config.h`); the portal renders that table and writes through to
NVS, so there is no field that exists in one and not the other. Every field has an
NVS key ≤ 15 characters inside the single `wildfire` namespace. `role` is the
override described in `src/role_detect.h`; blank means "auto-detect from the
sensor probe" (sensors found ⇒ node, none ⇒ base).

## 10. What is deliberately left open

- **Correlation window widening** (0091 recommended it; no number). Default 20,
  portal-tunable 10–60; the decision is Stephen's.
- `prov_ids` allowlist content for the first deployed base — a bench provisioning
  act, not a firmware default.
- A base→node alert fan-out (so nodes could show an event locally) is out of scope
  for v1: the app is the display and the acknowledge surface.
