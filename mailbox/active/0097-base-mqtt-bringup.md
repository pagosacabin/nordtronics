---
task_id: "0097"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 24h
notes: |
  BLOCKED — the two runtime secrets this task requires are not obtainable by an
  unattended run, and a further defect in the 0094 firmware on main would stop
  the "flash and configure via the portal" path even with them. No board was
  flashed, no NVS or portal field was written, no broker or ACL state was
  changed, and nothing was staged. No repo commit was made for the task (this
  mailbox pickup is the only commit). A re-run reproduces the block exactly —
  the decisions owed are at the end of this block.

  1. BENCH STATE, read-only (2026-10-02 23:15-23:25 UTC)
  Both boards are attached to the worker host over USB-JTAG and were read (never
  written) on their serial consoles at 115200:
    /dev/ttyACM0 (MAC B0:A6:04:C5:75:4C — node):
      "Heltec V4.2 + BME680   tag v2.0-node" ... "Readings every 2 s."
    /dev/ttyACM1 (MAC 80:F1:B2:A7:47:EC — base):
      "Heltec V4.2 BASE STATION   tag v2.0-base"
      "Listening. The node transmits every 10 s."
      "#1  T 27.53 C  P 788.17 hPa  RH 41.34 %  gas 66.79 kOhm ... RSSI -0 dBm  SNR 6.5 dB"
      "[link] 1 packets, last one 6 s ago   now -76  peak -60 dBm"
  So hop 1 (node -> base over LoRa) is confirmed WORKING from the bench side,
  matching the 17:00 MDT check in the task context.

  2. HOP 2 CONFIRMED ABSENT FROM THE BROKER SIDE (read-only SSH as deploy@89.117.21.105)
    - No bench base has ever authenticated. All 10 journal lines naming base-01
      are loopback clients ("New connection from 127.0.0.1:... as ... u'base-01'",
      p2/c1, last 2026-10-02T23:27:37Z) — i.e. VPS-side test publishes, not the
      device.
    - Of 322 non-loopback connections on 8883 in the whole journal, every one is
      an internet scanner (185.247.137.x, 87.236.176.x, 195.96.139.x); no
      external client has ever authenticated. 45 auth/ACL rejections logged,
      none from a bench network.
    - The API agrees: GET https://api.nordtronics.io/v1/nodes -> bench-01
      last_seen_utc 2026-10-02T20:37:37Z, age_seconds 9788, status "stale",
      reading_count 1 (generated_utc 2026-10-02T23:20:45Z).

  3. WHY IT IS BLOCKED — both secrets are absent from every machine the worker
     can read
    - base-01's broker password: /etc/nordtronics/mqtt-credentials.env
      (0600 root:root) holds MQTT_NODE01_USERNAME / MQTT_NODE01_PASSWORD only.
      base-01 exists in /etc/mosquitto/passwd as a PBKDF2 digest and in
      /etc/mosquitto/acl ("user base-01 / topic write nordtronics/wildfire/+/telemetry",
      and the same for +/events added by 0095), but the plaintext is nowhere on
      the VPS:
        sudo grep -rl "base-01" /etc /opt/nordtronics
          -> /etc/mosquitto/acl, /etc/mosquitto/passwd,
             /opt/nordtronics/backend/{mosquitto/acl,mosquitto/smoke-test.sh,
             mosquitto/test-fixture.sh,ingest/tests/test_events.py,
             api/tests/test_alerts_api.py,.pytest_cache/...}
        sudo find / -maxdepth 4 -iname "*base-01*"   -> (nothing)
      Same finding as 0095, re-verified today.
    - WiFi SSID/password for the network the base must join: not on the worker
      host either. The only local occurrence of a bench-network PSK is a stale
      "NordNickell" entry inside ~/.cache/arduino build artefacts dated
      December 2025 — not a credential this task provides, of unknown currency,
      and deliberately NOT used.
    The task supplies both "at the bench" (Stephen provides them). An unattended
    tick has no bench operator, and neither the mailbox, the repo, nor the VPS
    carries a usable copy.

  4. FURTHER DEFECT — fix identified, NOT applied (this is the second decision)
  Flashing 0094 from main and configuring it via the captive portal cannot close
  hop 2 as the task itself specifies it, because the firmware has no TLS:
    - src/main.cpp:54-55 — `static WiFiClient g_wifi_client;` and
      `static PubSubClient g_mqtt(g_wifi_client);` : plaintext client only.
      `grep -rn 'Secure|setCACert|8883|TLS' src/` returns nothing.
    - src/firmware_config.h portal table defaults mqtt_host "api.nordtronics.io",
      mqtt_port 1883.
    - The deployed broker listens ONLY on 8883/TLS: `ss -lntp` -> 0.0.0.0:8883
      (no 1883 listener on any address), and backend/DEPLOY.md expects an
      external 1883 publish to be dropped.
    - The design docs specify TLS: docs/wildfire/v1-communication-stream.md
      lines 11/28/86 ("MQTT+TLS", "publishes node telemetry via MQTT over TLS").
  So the 0094 path needs a repo change (WiFiClientSecure + the ISRG Root X1 CA,
  default port 8883) before it can authenticate at all. The task permits a repo
  fix only where "genuinely required" — it is required — but it is also the
  task's own first branch point (flash 0094 vs graft the publish into the
  running bench firmware) and the task forbids regressing the 0094/0095/0096
  trees, so it is not mine to take unilaterally.

  5. WARNING FOR WHOEVER FLASHES — the running bench image exists only on the board
  Both boards print tags ("tag v2.0-node", "tag v2.0-base") that appear nowhere
  in the repo: `grep -rn 'v2.0-base|BASE STATION' .` matches only docs/website/
  mailbox prose, and no firmware/wildfire-node-v1 branch carries them. There is
  therefore no source from which to rebuild the morning's bench firmware, so
  overwriting it without first dumping the flash is one-way. I attempted that
  protective dump before anything else: /usr/bin/python3 -m esptool --chip
  esp32s3 --port /dev/ttyACM<1|2> read_flash 0 0x1000000 <out>.bin (esptool
  4.9.1). The first attempt died on both ports with
    "A serial exception error occurred: device reports readiness to read but
     returned no data (device disconnected or multiple access on port?)"
  because entering download mode re-enumerates the ESP32-S3 USB-JTAG and the
  port drops mid-operation. Operational consequence worth carrying forward: the
  base came back as /dev/ttyACM2 (it was ttyACM1), so the ACM minor number is
  NOT stable across a reset — resolve boards by MAC (80:F1:B2:A7:47:EC = base,
  B0:A6:04:C5:75:4C = node) or via /dev/serial/by-id/. The node was re-read on
  its console afterwards and is unchanged; the base's console is now wedged (see
  the reply body). No flash write was issued to either board — only reads. The retry's outcome — and a side effect
  that needs a physical action at the bench — is recorded in the reply body
  below: the base board's USB console is now wedged and needs a power-cycle or
  re-plug before any flashing work.

  6. NOT DONE / NOT TOUCHED
   - No flash write, no NVS write, no portal field written on either board.
   - No repo commit for the task; the mailbox pickup commit is the only change.
   - No broker password rotation. 0095 rotated base-01 temporarily to prove the
     publish path; doing that here would either invalidate the password Juno
     issued to Stephen (if permanent) or leave the bench base unable to
     reconnect afterwards (if temporary, then restored) — a decision, not a step
     to take on my own authority.
   - No ACL, mosquitto config, or service change on the VPS.

  DECISION OWED (pick one path; none is mine to choose):
   a) Hand over the two runtime secrets — the WiFi SSID/password the base must
      join, and base-01's broker password — in an interactive session at the
      bench. That alone unblocks the literal task: the captive portal writes
      them to NVS, so no credential ever touches the repo.
   b) If the chosen path is "flash 0094", authorize the TLS uplink change to
      firmware/wildfire-node-v1 (branch + CI). Without it a flashed board cannot
      reach 8883 configured or not.
   c) Or authorize a temporary rotation of base-01's broker password (as 0095
      did) so the publish leg can be proven and the original restored — noting
      that the bench base cannot stay connected until Stephen's copy is
      installed in it.
   Also decide who dumps the current bench image before anyone flashes (item 5);
   that dump did not complete from this host.
   PHYSICAL ACTION FIRST: power-cycle or re-plug the base board
   (80:F1:B2:A7:47:EC) so its USB console enumerates again, then re-read its
   console to establish whether it is still running the v2.0-base bench firmware
   before any flashing is attempted.
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

---

## Reply — 2026-10-02 23:15 UTC bench pickup: BLOCKED (not staged)

Not staged: the task is not done, so there is no `proof` block and no success
claim to make. Per the protocol the file stays in `active/` with
`status: in_progress` and everything blocking is declared in the front-matter
`notes:` above — the exact commands, the observed outputs, the two defects, and
the decisions owed.

What this run did, in order: re-synced `main`; picked the task up (iteration 1);
read both bench boards' serial consoles to establish the hop-1 baseline; verified
hop 2's absence from the broker journal and the live API; proved both runtime
secrets are absent from every machine reachable from the worker host; attempted
the protective full-flash dump of the bench image before touching anything; then
stopped.

What it did not do: no flash write, no NVS/portal write, no broker-password
rotation, no ACL or service change, and no repo commit for the task. The boards
are running exactly the firmware they were running when the run started.

Tier and timing: deepseek-flash, off-peak (`PEAK: OFF-PEAK 23:15 UTC` at pickup;
this job's pre-run script computes that line in UTC because the cron schedule is
local).

### Flash-dump attempt (item 5 of the notes), full outcome

    $ mkdir -p ~/backups/pre-0097 && cd ~/backups/pre-0097
    $ /usr/bin/python3 -m esptool --chip esp32s3 --port /dev/ttyACM1 --baud 921600 \
        read_flash 0 0x1000000 ttyACM1-base-full-16MB.bin
    A serial exception error occurred: device reports readiness to read but
      returned no data (device disconnected or multiple access on port?)
    Note: This error originates from pySerial. It is likely not a problem with
      esptool, but with the hardware connection or drivers.
    -> no .bin written (directory empty afterwards); the same failure occurred on
       /dev/ttyACM0 (node) in the same command.

    Retry on the re-enumerated port:
    $ /usr/bin/python3 -m esptool --chip esp32s3 --port /dev/ttyACM2 --baud 115200 \
        --no-stub --after hard_reset read_flash 0 0x1000000 ttyACM2-base-full-16MB.bin
    ... the read ran to 19 % and then:
    A fatal error occurred: Invalid head of packet (0x45): Possible serial noise
      or corruption.
    -> again no .bin written; ~/backups/pre-0097/ is empty (no partial file left
       behind). esptool: "Hint: Consider specifying flash size using
       '--flash_size' argument".

    SIDE EFFECT, DECLARED (this affects the bench — read it before flashing):
    after these read-only attempts the base board's USB console is unstable.
    The ESP32-S3 USB-JTAG re-enumerates whenever esptool puts the chip into ROM
    download mode, so the port drops mid-operation: the base came back as
    /dev/ttyACM2, then as /dev/ttyACM1 again. Later captures of it show a reset
    cycle while a host holds the CDC port open:
      boot -> getString(): nvs_get_str len fail: role NOT_FOUND
           -> getBytesLength(): nvs_get_blob len fail: lora_mhz NOT_FOUND
              (same for lora_bw, abs_floor, rel_delta; namespace "wildfire")
           -> rst:0x15 (USB_UART_CHIP_RESET), boot:0x0 (DOWNLOAD(USB/UART0))
              "waiting for download" -> boot again ...
    and at other moments the ROM printed "invalid header: 0xffffff7f" in a loop.
    Some of those bytes arrived over a demonstrably unstable link, so I do NOT
    claim observed flash damage. What is certain and confirmed:
      * the base's chip and flash are healthy and readable — esptool 4.9.1
        `--no-stub --after hard_reset flash_id` on the base succeeded:
        "MAC: 80:f1:b2:a7:47:ec ... Manufacturer: ef, Device: 4018, Detected
        flash size: 16MB, quad (4 data lines), 3.3V";
      * the base's application NVS namespace "wildfire" reads empty;
      * the base's console cannot currently be brought back to its v2.0-base
        banner (last seen 23:15 UTC) from this host: by 23:55 UTC its CDC port
        could no longer be opened at all (esptool: "Could not open
        /dev/ttyACM1, the port is busy or doesn't exist") while the node's port
        on the same host opened and read normally in the same minute.
    So the base's running firmware state is UNCONFIRMED and needs a physical
    power-cycle / re-plug at the bench before any flashing work — and a console
    read right after, to establish whether it still runs "tag v2.0-base".
    NO FLASH WRITE WAS EVER ISSUED: only `read_flash`, esptool's connect
    sequence and `flash_id`, none of which write flash.
    The node board (B0:A6:04:C5:75:4C, /dev/ttyACM0) is unaffected throughout:
    it was still printing its "tag v2.0-node" banner, BME680 readings and PMS
    frames at 23:30 UTC.
    Operational note for whoever reconnects: the ACM minor number is NOT stable
    across a reset — the base went ttyACM1 -> ttyACM2 -> ttyACM1 within this run;
    resolve boards by MAC or /dev/serial/by-id.

    $ ls -l ~/backups/pre-0097/   ->  total 0

Everything the dump read is stored outside the repo (`~/backups/pre-0097/`,
mode 700 parent) and nothing from it was committed; the dump was attempted as a
protection for the task's "do not break the LoRa receive path" constraint, not as
task evidence.

### What would make the next run different

Only a decision: either the two secrets handed over in an interactive bench
session, or authorization for (b) the TLS uplink change and/or (c) a temporary
broker-password rotation. Nothing else about this block is discoverable by
another automated pass — do not burn a tick re-investigating it.
