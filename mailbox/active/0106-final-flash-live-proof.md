---
task_id: "0106"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 24h
proof: []
notes: |
  PICKED UP (iteration 1 -> 2) by the mailbox worker, 2026-10-04 02:2x UTC.
  Off-peak (PEAK: OFF-PEAK 02:15 UTC). Predecessor tasks 0097/0098 remain
  decision-blocked in active/ and were not touched.

  BLOCKED (iteration 2, 2026-10-04 02:17-02:34 UTC) — NO proof block, not
  staged. The flash half is DONE and verified; the live-data half is
  unattainable as specified. Exact state:

  1. DONE — the 0105 artifact was flashed to the BASE board and verified.
     run 37167784900 (artifact 11290351978, name
     wildfire-node-v1-unified-firmware, 769746 B zipped) -> firmware.bin
     1222672 B, sha256
     22874163dd4cc3dad2b3646b3092f928dda3ac3173d3a7fb6f957dada7f1a296.
     Board identity confirmed with esptool BEFORE any write: by-id
     ..._80:F1:B2:A7:47:EC-if00 -> ttyACM0, MAC 80:f1:b2:a7:47:ec (the BASE).
     Partition table read from the device first (4096 B @ 0x8000) = stock
     default_16MB.csv (nvs 0x9000/0x5000, app0 0x10000/0x640000, app1, spiffs,
     coredump), so the app-only write at 0x10000 cannot touch NVS.
     write_flash 0x10000 --flash_size 16MB, no --erase-all, no erase_flash.
     Read-back 1222672 B: sha256 identical, `cmp` exit 0.
     The board boots the new build:
       ROLE: base (source=NVS, sensors=absent, nvs_role=1)
       firmware: wildfire-unified-v1 proto=1 build=Oct  4 2026 01:23:52
         (run 37167784900 createdAt 2026-10-04T01:21:53Z / updatedAt
          01:24:34Z -- the build string is inside that window)
       wifi: connected -- ip=192.168.1.71 ... wifi: status=3 (connected)
       ntp: clock set (t=1791080929)
       mqtt: connected to mqtt.nordtronics.io:8883
     NVS survived (the board joined the stored network on the first STA
     attempt; no credential was read or written).

  2. BLOCKED — no reading can reach the backend, and it is NOT the flash.
     The base received NOTHING over LoRa: a 170 s console capture after the
     flash contains zero `rx:` lines while the node board transmitted
     (3 `[tx] #n sent ... (26 bytes)` frames in a 40 s node capture).
     Cause, verified on both sides:
       - the flashed unified firmware receives at 915.0 MHz / BW125 / SF7 /
         CR5 (console `radio: begin(915.0MHz bw125k sf7 cr5 sync=0x12 20dBm)`;
         firmware_config.h defaults lora_bw "125.0", lora_sf "7", lora_cr "5";
         docs/wildfire/radio-protocol-v1.md pins 125.0 kHz / SF7) and decodes
         ONLY the 27-byte v1 frame (radio_protocol.cpp:105 `decode_frame`:
         in[0]==kProtoVersion(1), in[6]==plen(17), len==27, CRC16-CCITT).
       - the node board still runs the legacy bench firmware
         (~/Documents/PlatformIO/Projects/heltec-v3-bme680/src/link.h):
         LINK_BW 250.0 kHz, LINK_SF 11, LINK_CR 8, and a 26-byte packed
         `struct LinkPacket { uint16_t seq; float tC,hPa,rhPct,gasK;
         uint16_t pm1,pm25,pm10; uint8_t haveBme,havePms; }`.
     Different bandwidth+SF => the base cannot demodulate the node's packets
     at all; and even a lucky demodulation fails decode_frame on length
     (26 != 27) before the CRC. There is no legacy receive path in the unified
     firmware (`git grep -i 'LinkPacket\|legacy' 951cbd38 --
     firmware/wildfire-node-v1/` -> nothing), so a config-only workaround
     (portal lora_bw/lora_sf) cannot bridge it either -- and NVS writes are
     forbidden by this task anyway.
     Consequence: 0106's premise ("the node board transmitting nearby (it
     already is)") does not hold for THIS node image. Hop 1 (node -> base) has
     been parameter-incompatible since 0103 put the unified firmware on the
     base; both boards ran the matching bench firmware when 0097 proved hop 1
     on 2026-10-02.

  3. Independent finding, out of 0106's scope: the MQTT session still does NOT
     hold. Broker journal (VPS local = CEST = UTC+2; the message text carries
     local time with a misleading Z), last 40 min, one cycle per line group:
       04:31:09 new client connected ... as wf-base-0 (p2, c1, k15, u'base-01')
       04:31:32 Client wf-base-0 has exceeded timeout, disconnecting.
       04:32:20 new client connected ...
       04:32:44 Client wf-base-0 has exceeded timeout, disconnecting.
     i.e. ~23 s sessions with ~48 s gaps, indefinitely -- the same ~22 s
     keepalive drop 0103 reported, so 0104's "MQTT session resilience" item is
     not effective on the device. `journalctl -u wildfire-ingest` had no
     entries in 40 min; the API still shows bench-01 stale
     (last_seen_utc 2026-10-02T20:37:37Z, age 107735 s at 02:33:12Z).

  4. NOT DONE / NOT TOUCHED: no repo commit for the task (the mailbox pickup
     commit is the only change); no branch, no build, no CI run (an apply task
     produces neither). The NODE board received no write of any kind -- but it
     WAS reset (an RTS/EN pulse, no flash, no NVS) so its console could be
     read; declared rather than omitted. No credential was read, written or
     logged; no router change; no VPS change (the broker journal and the API
     were read read-only over ssh/HTTPS).

  DECISION OWED (none of it is mine to take -- 0106 names the node as never
  the target, and the fix needs a repo change or a second flash):
   a) Authorize flashing the SAME 0105 artifact to the node board. The firmware
      is unified and self-detects role=node from the sensor probe, so this is
      the route the design implies and the only one that completes the stack
      with the reviewed revision. Caveat: the node's `node_id` default is 0
      ("unprovisioned"), so unless it is provisioned it would publish under
      node "0" (validation stores it; it would not be "bench-01"). The base's
      `prov_ids` is empty = bench-open, so it will not filter the frame.
   b) Or authorize a repo change (branch + CI) adding a legacy LinkPacket
      receive path to the unified base -- explicitly outside this task.
   c) Or authorize reverting the base to a link.h-compatible build, which
      discards 0099/0104/0105 on the device.
  Meanwhile the base is left running the 0105 build, connected to the broker.

  Filed by Juno, 2026-10-03. The final flash: 0105 is verified and archived
  (payload + events match the deployed backend contract — actual firmware
  bytes were fed through validation.py/events.py on main, ok=True). This
  task puts that build on the bench base and proves the whole stack:
  physical node -> physical base over LoRa -> MQTT -> API -> app, with a
  fresh reading and no "Stale". Stephen's explicit requirement.
---

# 0106 — Flash the 0105 artifact to the bench base and prove live data

## Context

The code chain is complete and verified: 0099 (TLS + fixes, flashed in
0103 — hop 2 live), 0104 (topic + hostname + sensor role + MQTT
resilience), 0105 (payload + events match the backend contract). The CI
artifact `wildfire-node-v1-unified-firmware` from run 37167784900
(artifact id 11290351978, 769746 bytes zipped) is the build to flash.
The bench base board is at
`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00`
(MAC `80:F1:B2:A7:47:EC` — confirm by-id/MAC before touching anything;
the node `B0:A6:04:C5:75:4C` is never the target).

## Task

1. Download the 0105 artifact (`gh run download 37167784900 -n
   wildfire-node-v1-unified-firmware`), flash `firmware.bin` to the BASE
   board only via the by-id path. App-only write at 0x10000, no erase
   flags, NVS untouched (stored WiFi credentials must survive).
2. With the node board transmitting nearby (it already is), watch for
   the whole stack to complete:
   - Serial: base logs LoRa receive, then `mqtt: connected`, then a
     telemetry publish to `nordtronics/wildfire/<node-id>/telemetry`.
   - Broker: `journalctl -u mosquitto` shows the publish arriving.
   - API: `curl -s https://api.nordtronics.io/v1/nodes` shows a node
     with `last_seen_utc` within the last 15 minutes and
     `status` anything but `stale`.
   - Session: does the MQTT session hold now (0104's keepalive fix),
     or does it still cycle on keepalive timeout? Quote the broker
     journal.
3. Report which node ID appeared (the topic's node ID), the fresh
   `last_seen_utc`, and the session-hold observation.

## Success criteria

- The flashed binary matches the 0105 artifact (sha256 before flashing,
  read-back after).
- `GET /v1/nodes` shows a reading with `last_seen_utc` less than 15
  minutes old — the app will flip off "Stale" on its own from this.
  Quote the API row.
- The node board was never touched; NVS was not wiped.

## Constraints

- Confirm the board's MAC/by-id identity BEFORE flashing.
- No NVS wipe, no router changes, no credential reads/writes. One
  deliverable: the flashed base and the live-data proof. Nothing else.

## Proof

- sha256 of the flashed binary + read-back match.
- Quoted serial lines (LoRa receive, MQTT connect, publish).
- Quoted `GET /v1/nodes` row with fresh `last_seen_utc`.
- Broker journal lines showing session hold or continued cycling.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.

---

# Reply — BLOCKED, no proof block (iteration 2)

There is **no `proof` block** and no staged file for this task. The 0105
artifact **is** flashed to the base and verified byte-for-byte; the live-data
success criterion is **not met** and is unattainable under this task's own
constraints, because the base cannot receive anything from the node board's
current image. The file stays in `active/` with `status: in_progress`. Full
detail is in the front-matter `notes:`; the evidence is below.

## What is DONE — the flash (success criterion 1 and 3)

Identity first, before any write (0103's rule, and this task's):

    $ ls -l /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00
    ... -> ../../ttyACM0
    $ /usr/bin/python3 -m esptool --chip esp32s3 --port <that by-id> --no-stub --after hard_reset flash_id
    Chip is ESP32-S3 (QFN56) (revision v0.2)
    USB mode: USB-Serial/JTAG
    MAC: 80:f1:b2:a7:47:ec          <- the BASE, as specified
    Detected flash size: 16MB

The node (`B0:A6:04:C5:75:4C`, `ttyACM1`) was never the target of a write.

Partition table read from the device (4096 B @ 0x8000, read-only) before
flashing — stock `default_16MB.csv`, so an app-only write is safe and cannot
reach NVS:

    nvs       type=1 subtype=2   off=0x009000 size=0x005000
    otadata   type=1 subtype=0   off=0x00e000 size=0x002000
    app0      type=0 subtype=16  off=0x010000 size=0x640000
    app1      type=0 subtype=17  off=0x650000 size=0x640000
    spiffs    type=1 subtype=130 off=0xc90000 size=0x360000
    coredump  type=1 subtype=3   off=0xff0000 size=0x010000

Artifact, re-derived rather than assumed:

    gh run view 37167784900 -> headBranch hermes/0105-payload-contract,
                               conclusion success, createdAt 2026-10-04T01:21:53Z
    git ls-remote --heads origin hermes/0105-payload-contract
        -> 951cbd38d152c0c3aeeb5ea0ec70c77ed009b280   (run headSha == branch tip)
    gh <run>/artifacts -> wildfire-node-v1-unified-firmware id 11290351978, 769746 B, not expired
    gh run download 37167784900 -n wildfire-node-v1-unified-firmware
    sha256sum firmware.bin -> 22874163dd4cc3dad2b3646b3092f928dda3ac3173d3a7fb6f957dada7f1a296
                              (1222672 bytes)

Write — app partition only, no erase flags (this is the command 0103 proved):

    /usr/bin/python3 -m esptool --chip esp32s3 --port <by-id> --baud 460800 --no-stub \
      --after hard_reset write_flash --flash_size 16MB 0x10000 firmware.bin
    Wrote 1223680 bytes at 0x00010000 in 12.6 seconds (775.8 kbit/s)...
    Hash of data verified.
    Hard resetting via RTS pin...

Read-back proof, independent of esptool's own hash line:

    read_flash 0x10000 1222672 readback.bin
    sha256sum firmware.bin readback.bin
    22874163dd4cc3dad2b3646b3092f928dda3ac3173d3a7fb6f957dada7f1a296  firmware.bin
    22874163dd4cc3dad2b3646b3092f928dda3ac3173d3a7fb6f957dada7f1a296  readback.bin
    cmp firmware.bin readback.bin -> identical, exit 0

The board boots the new build (serial capture, 170 s):

    [   308][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: abs_floor NOT_FOUND
    probe: i2c 0x77 -> no ACK
    probe: i2c 0x76 -> no ACK
    probe: pms5003 -> no frame
    ROLE: base (source=NVS, sensors=absent, nvs_role=1)
    firmware: wildfire-unified-v1 proto=1 build=Oct  4 2026 01:23:52
    base: deep sleep is disabled for this role (asserted on every sleep path)
    radio: begin(915.0MHz bw125k sf7 cr5 sync=0x12 20dBm) -> ok
    oled: probe 0x3C -> ACK
    wifi: dhcp hostname [wildfire-base-01]
    portal AP fallback: wildfire-setup  http://192.168.4.1 (boot)
    wifi: disconnected -- reason=203 (sta down)
    wifi: connected -- ip=192.168.1.71 gw=192.168.1.1 rssi=-39 dBm
    wifi: status=3 (connected) ip=192.168.1.71
    ntp: clock set (t=1791080929)
    mqtt cfg: host=[mqtt.nordtronics.io] port=8883 (TLS, pinned ISRG Root X1)
    mqtt: connected to mqtt.nordtronics.io:8883

`build=Oct  4 2026 01:23:52` falls inside run 37167784900's 01:21:53–01:24:34Z
window, so the image on the board is the artifact under test. NVS survived: the
board joined the stored network on its first STA attempt and `ROLE:` came from
NVS (`nvs_role=1`), neither credential nor role was re-entered.

## What is BLOCKED — the live-data criterion

**The base received nothing over LoRa.** The 170 s post-flash capture contains
zero `rx:` lines (grep over the whole capture: only the three `mqtt: connected`
lines). The node board was transmitting throughout — a 40 s node capture:

    Heltec V4.2 + BME680   tag v2.0-node
    Radio   : 915.0 MHz  SF11  BW250  14 dBm  -> ok
    [tx] #1 sent  T 26.63 C  P 789.60 hPa  RH 44.46 %  gas 49.92 kOhm  PM1 0  PM2.5 0  PM10 0  bme=1 pms=1  (26 bytes)
    [tx] #2 sent  ...
    [tx] #3 sent  ...

The two ends are on different LoRa parameters and different frame formats:

| | flashed base (0105 artifact) | node board (running image) |
|---|---|---|
| bandwidth | 125.0 kHz | 250.0 kHz |
| spreading factor | 7 | 11 |
| coding rate | 5 (4/5) | 8 (4/8) |
| frame | 27-byte v1 packed (`version,type,node_id,seq,…`, CRC16-CCITT) | 26-byte packed `LinkPacket` (`seq, tC, hPa, rhPct, gasK, pm1, pm25, pm10, haveBme, havePms`) |

Sources, both read from the repo/host rather than inferred:

- `firmware/wildfire-node-v1/src/firmware_config.h` @ 951cbd38 defaults
  `lora_bw "125.0"`, `lora_sf "7"`, `lora_cr "5"`; the board prints the same.
  `docs/wildfire/radio-protocol-v1.md:92,123,124` pins 125.0 kHz / SF7.
  `radio_protocol.cpp:105 decode_frame` requires `in[0] == kProtoVersion(1)`,
  `in[6] == plen(17)`, `len == 27`, then CRC16 — the 26-byte LinkPacket fails on
  length before the CRC is even reached.
- `~/Documents/PlatformIO/Projects/heltec-v3-bme680/src/link.h`: `LINK_BW 250.0`,
  `LINK_SF 11`, `LINK_CR 8`, and `struct LinkPacket { … } __attribute__((packed))`
  (2 + 4×4 + 3×2 + 2 = 26 bytes) — the "… (26 bytes)" the node prints.

Wrong SF/bandwidth means the base cannot demodulate the node's packets at all;
even a lucky demodulation is rejected on length. The unified firmware has **no**
legacy receive path (`git grep -i 'LinkPacket\|legacy' 951cbd38 --
firmware/wildfire-node-v1/` returns nothing), so this cannot be bridged by
configuration — and NVS writes are forbidden by this task in any case.

**The premise is the defect, not the flash.** 0106 says "the node board
transmitting nearby (it already is)". It is transmitting — with the legacy bench
firmware. Hop 1 (node → base) stopped being parameter-compatible when 0103 put
the unified firmware on the base; both boards were still on the matching bench
firmware when 0097 proved hop 1 working on 2026-10-02. Nothing since 0103 has
ever produced a stored reading, which is consistent with this.

## Independent finding, out of scope — the MQTT session still does not hold

Task item 2 asks whether the session holds after 0104's keepalive fix. It does
not. Broker journal, last 40 minutes (VPS local time is CEST = UTC+2; the
message text prints local time with a misleading `Z`):

    Oct 04 04:31:09 ... 2026-10-04T04:31:09Z: New client connected from 98.97.1.121:4552 as wf-base-0 (p2, c1, k15, u'base-01').
    Oct 04 04:31:32 ... 2026-10-04T04:31:32Z: Client wf-base-0 has exceeded timeout, disconnecting.
    Oct 04 04:32:20 ... 2026-10-04T04:32:20Z: New client connected from 98.97.1.121:42361 as wf-base-0 (p2, c1, k15, u'base-01').
    Oct 04 04:32:44 ... 2026-10-04T04:32:44Z: Client wf-base-0 has exceeded timeout, disconnecting.

~23 s sessions with ~48 s gaps, indefinitely — the same ~22 s keepalive drop
0103 reported. (`k15` is PubSubClient's keepalive; mosquitto times out at 1.5×
it.) `journalctl -u wildfire-ingest` had **no entries** in 40 minutes, and the
API at 2026-10-04T02:33:12Z still reads:

    bench-01  last_seen_utc 2026-10-02T20:37:37Z  age_seconds 107735  status "stale"
    node-01   last_seen_utc 2026-09-25T19:20:01Z  age_seconds 717191  status "stale"

Since the base buffers when the session is down and replays on reconnect, this
cycling alone would not stop telemetry — which is how we know the missing
reading is the LoRa layer, not the session.

## Deviations and things left undone

1. **The node board was reset.** Reading its console needs an EN pulse, so an
   RTS/EN reset was issued to `ttyACM1` (`B0:A6:04:C5:75:4C`) — no flash, no
   NVS write, no other write of any kind. It rebooted and resumed transmitting
   (`[tx] #1 sent` onwards). Declared rather than left implicit.
2. **No proof block, no branch, no CI run.** This is an apply task; it produces
   neither, and the criterion that matters failed. Not staged.
3. **The task's own no-touch constraint blocked the only fix.** Flashing the
   node with the same artifact is the reachable fix; 0106 names the node as
   never the target and forbids anything beyond the base, so it was not done.
4. Not touched: no repo file, no router, no VPS configuration, no credential
   read/written/logged.

## Decision owed

- (a) **Authorize flashing the same 0105 artifact to the node board.** The
  firmware is unified and self-detects `role=node` from the sensor probe, so
  this completes the stack on the reviewed revision. Caveat: the node's
  `node_id` default is `0` (unprovisioned), so without provisioning it would
  publish under node `"0"` — a valid stored reading, but not `bench-01`. The
  base's `prov_ids` is empty (= bench-open), so it will not filter the frame.
- (b) Or authorize a repo change (branch + CI, outside this task) adding a
  legacy `LinkPacket` receive path to the unified base.
- (c) Or authorize reverting the base to a `link.h`-compatible build, which
  discards 0099/0104/0105 on the device.

The base is left running the 0105 build and connected to the broker in the
meantime.

Cost: `deepseek-flash`, standard tier, off-peak (`PEAK: OFF-PEAK 02:15 UTC`).

