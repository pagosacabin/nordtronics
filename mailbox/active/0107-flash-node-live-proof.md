---
task_id: "0107"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 24h
proof: []
notes: |
  PICKED UP (iteration 1 -> 2) by the mailbox worker, 2026-10-04 10:15 UTC.
  Off-peak (PEAK: OFF-PEAK 10:15 UTC). Predecessors 0097/0098/0106 remain
  decision-blocked in active/ and were not touched.

  BLOCKED (iteration 2, 2026-10-04 10:16-10:35 UTC) — NO proof block, not staged.
  The FLASH HALF IS DONE AND VERIFIED; the task's own step 3 fired: the node
  resolved as anything but node, so per the task ("stop and report it as the
  finding (do not fix it with NVS writes)") the live-data half is unattainable.
  Exact state:

  1. DONE — the 0105 artifact is on the node, byte-identical.
     run 37167784900 (conclusion success, headSha
     951cbd38d152c0c3aeeb5ea0ec70c77ed009b280), artifact 11290351978
     "wildfire-node-v1-unified-firmware", re-downloaded fresh ->
     firmware.bin 1222672 B, sha256
     22874163dd4cc3dad2b3646b3092f928dda3ac3173d3a7fb6f957dada7f1a296
     (identical to 0106's recorded value for the same artifact).
     Identity confirmed BEFORE the write: by-id
     ..._B0:A6:04:C5:75:4C-if00 -> /dev/ttyACM1, esptool read_mac
     b0:a6:04:c5:75:4c (the NODE); the base is by-id
     ..._80:F1:B2:A7:47:EC-if00 -> /dev/ttyACM0, read_mac 80:f1:b2:a7:47:ec.
     (The ttyACM0/1 numbering is swapped relative to 0097's survey, so every
     step used the by-id path, never the bare number.)
     Partition table read from the node first (4096 B @ 0x8000): nvs
     0x9000/0x5000, otadata 0xe000/0x2000, app0 0x10000/0x330000, app1,
     spiffs, coredump — an 8 MB layout (see deviation 2). The app-only write
     0x10000 (ends 0x22A7D0) cannot reach NVS. Then
     `esptool --port /dev/ttyACM1 --after hard_reset write_flash
     --flash_size 16MB 0x10000 firmware.bin` -> "Wrote 1222672 bytes ...
     Hash of data verified." No --erase-all, no erase_flash.
     Read-back 0x10000 1222672 B: sha256 identical, `cmp` exit 0.

  2. THE FINDING — the node comes up as BASE because the artifact's sensor
     probe cannot see the I2C bus at all. Reproduced twice on the node:
       probe: i2c 0x77 -> no ACK
       probe: i2c 0x76 -> no ACK
       probe: pms5003 -> no frame
       ROLE: base (source=DEFAULT, sensors=absent, nvs_role=unset)
       firmware: wildfire-unified-v1 proto=1 build=Oct  4 2026 01:23:52
       oled: probe 0x3C -> no ACK
       oled: panel did not answer -- rendering disabled, bus left idle
       wifi: dhcp hostname [wildfire-base-01]  (portal AP fallback; DNS fails)
       -> 0 [tx]/telemetry lines in a 45 s steady window; the node never
          transmits, so hop 1 and the /v1/nodes criterion cannot be met.
     Ground truth on the SAME board minutes before the flash: the legacy bench
     image printed "Display : 0x3C -> init OK", "Sensor  : 0x77 -> bound OK",
     "Heltec V4.2 + BME680 tag v2.0-node", and "[tx] #N sent ... (26 bytes)"
     (0106 scratch node-console.log; my node-pre-flash.log shows the tx lines).
     The SAME binary on the untouched base board prints "oled: probe 0x3C ->
     ACK" — so the panel path works there; the difference is that the node
     board carries the BME680.
     PROBABLE MECHANISM (verified in source, NOT applied): the unified
     firmware never drives the Heltec Ve/Vext gate — GPIO36, ACTIVE LOW. The
     legacy bench sketch does (node.cpp: `#define PIN_VEXT 36` ...
     `pinMode(PIN_VEXT, OUTPUT); digitalWrite(PIN_VEXT, LOW); // Ve rail ON`),
     and this repo's own heltec-lora-board-bringup skill says GPIO36 is
     "Vext_Ctrl", that without driving it "the sensor's VIN sits at 0 V —
     unpowered exactly while the firmware runs", and that one unpowered chip
     clamps SDA and takes the whole bus down. `git show
     951cbd3:firmware/wildfire-node-v1/src/{firmware_config.h,main.cpp}` has no
     reference to 36 outside the frozen-net table and never drives it, so the
     BME680 on Ve is unpowered, the clamp kills the shared bus, `probe_node_sensors()`
     returns false, and resolve_role() falls through to Default = base.
     FIX OWED (named, not applied): enable the Ve rail (kOledVextPin 36,
     OUTPUT, LOW) before probe_node_sensors()/oled_probe_and_begin() in both
     roles, re-run the artifact, then re-flash. That is a firmware change =
     new branch + new artifact, outside this task's "no repo commits, no new
     branches" constraint and outside its "do not fix" instruction.

  3. /v1/nodes at 2026-10-04T10:25:23Z: bench-01 last_seen_utc
     2026-10-02T20:37:37Z, age_seconds 136066, status "stale"; node-01 stale.
     No fresh reading. Broker side (mosquitto journal on the VPS): zero
     telemetry publishes in the window; the only authenticated non-loopback
     client is the base board itself, 98.97.1.121 as wf-base-0 u'base-01',
     reconnecting about once a minute ("has exceeded timeout, disconnecting"),
     which is a separate pre-existing keepalive condition and not this task.

  4. DEVIATIONS DECLARED
     a. THE BASE BOARD WAS RESET DURING THIS RUN. Not reflashed — no write or
        flash command was ever issued to /dev/ttyACM0 or the 80:F1... by-id,
        and its boot banner still reports the unchanged artifact build string
        "build=Oct  4 2026 01:23:52". But my console-capture tooling (pyserial
        open) toggled the USB-JTAG modem lines and the base rebooted at the
        start of each capture. The criterion "the base board was never
        reflashed or reset for this task" is therefore NOT met as written.
        Corrected method for the record: `stty -F <port> 115200 raw -echo
        -hupcl` + `cat` — a 45 s window on each board with ZERO ESP-ROM
        banners, i.e. non-perturbing; the evidence windows quoted above use it.
     b. The node's on-device partition table is an 8 MB layout (app0
        0x10000/0x330000), not the default_16MB.csv the artifact was built
        with. The spec was followed literally — app-only at 0x10000, no erase
        flags, NVS untouched — and app0 still fits; no partition table was
        written, since step 2 says only firmware.bin.
     c. The node's previous app image was overwritten without a local backup
        (the task specified the write and forbade erase, and did not ask for a
        dump; in hindsight a read_flash of 0x10000 first would have been free).
        The legacy image exists only as its Arduino source tree at
        ~/Documents/PlatformIO/Projects/heltec-v3-bme680, outside this repo.

  Filed by Juno, 2026-10-04, at Stephen's explicit "Go ahead flash away"
  (04:13 MDT). Authorizes what 0106 forbade: flashing the NODE board.
  Context: 0106 proved the base runs the 0105 build and reaches the broker,
  but the node still transmits the legacy link.h waveform (BW250/SF11/CR8,
  26-byte LinkPacket) which the unified base (BW125/SF7/CR5, 27-byte v1
  frames) cannot demodulate. Hop 1 is down. The fix is the unified design's
  own answer: the same 0105 artifact on the node, which self-detects
  role=node from its plugged-in sensors (BME680 at 0x77, PMS5003 on UART).
---

# 0107 — Flash the 0105 artifact to the bench NODE and prove the stack

## Context

Base board (`80:F1:B2:A7:47:EC`) runs the 0105 build (run 37167784900,
artifact 11290351978) and reaches the broker. Node board
(`B0:A6:04:C5:75:4C`) still runs the legacy bench image and its LoRa
waveform is incompatible with the base. Flash the SAME 0105 artifact to
the node. The node carries the BME680 and PMS5003, so the 0104 sensor
probe must resolve it as node.

## Task

1. Confirm the node board by-id/MAC
   (`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`)
   BEFORE any write. Flashing the base again is the unforgivable error.
2. Download the 0105 artifact, verify sha256, flash `firmware.bin` to the
   NODE board only: app-only write at 0x10000, no erase flags, NVS
   untouched.
3. Capture the node boot log and report the ROLE line — expect
   `ROLE: node (source=PROBE, sensors=present, ...)`. If it resolves as
   anything else, stop and report it as the finding (do not "fix" it
   with NVS writes).
4. With both boards up, prove the stack end to end:
   - Node console: LoRa TX lines with the new params.
   - Base console: `rx:` lines (it hears the node).
   - Broker journal: the telemetry publish arrives.
   - API: `curl -s https://api.nordtronics.io/v1/nodes` shows a fresh
     reading (`last_seen_utc` < 15 min, not `stale`). Quote the row.
   - Note which node ID it publishes under (expect "0" — unprovisioned;
     that is acceptable for this proof, declared not fixed).

## Success criteria

- Flashed binary matches the 0105 artifact (sha256 before, read-back
  after).
- Node boot log shows `ROLE: node` from the sensor probe — quote it.
- `GET /v1/nodes` shows a reading fresher than 15 minutes — quote it.
- The base board was never reflashed or reset for this task.

## Constraints

- MAC/by-id identity check BEFORE flashing. Wrong board = failure.
- No NVS wipe on the node. No router changes. No credential reads/writes.
- No repo commits, no new branches — this is an apply task. One
  deliverable: the flashed node and the live-data proof. Nothing else.

## Proof

- sha256 of the flashed binary + read-back match.
- Quoted node boot ROLE line.
- Quoted base `rx:` line(s).
- Quoted `GET /v1/nodes` row with fresh `last_seen_utc`.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.

---

# Result — iteration 2 (2026-10-04 10:16-10:35 UTC)

**Status: BLOCKED — flash half done and verified; the role probe fired the
task's own step-3 branch, so there is NO proof block and the task is NOT
staged. It stays in `active/` with `status: in_progress` and `iteration: 2`.
`proof` is empty on purpose: this is an apply task with no branch and no CI
run, and two of its four success criteria are unattainable as written.**

## What is verified

1. **The node carries the 0105 artifact byte-for-byte.**
   - run `37167784900`, `conclusion: success`, `headSha:
     951cbd38d152c0c3aeeb5ea0ec70c77ed009b280`; artifact `11290351978`
     (`wildfire-node-v1-unified-firmware`, `expired: false`), re-downloaded
     fresh in this run.
   - `sha256(firmware.bin) = 22874163dd4cc3dad2b3646b3092f928dda3ac3173d3a7fb6f957dada7f1a296`
     (1222672 B) — the same value 0106 recorded for this artifact.
   - target identity twice before the write: by-id
     `..._B0:A6:04:C5:75:4C-if00 -> /dev/ttyACM1`, `read_mac` = `b0:a6:04:c5:75:4c`
     (NODE). The base's by-id `..._80:F1:B2:A7:47:EC-if00 -> /dev/ttyACM0`,
     `read_mac` = `80:f1:b2:a7:47:ec`.
   - partition table read first (nvs 0x9000/0x5000, app0 0x10000/0x330000);
     `write_flash --flash_size 16MB 0x10000 firmware.bin` → "Wrote 1222672
     bytes (745105 compressed) ... **Hash of data verified.**" No erase flags.
   - read-back of 1222672 B from 0x10000: **sha256 identical, `cmp` exit 0.**

2. **The node resolved as `base`, not `node`** (reproduced on two resets):
   ```
   probe: i2c 0x77 -> no ACK
   probe: i2c 0x76 -> no ACK
   probe: pms5003 -> no frame
   ROLE: base (source=DEFAULT, sensors=absent, nvs_role=unset)
   firmware: wildfire-unified-v1 proto=1 build=Oct  4 2026 01:23:52
   oled: probe 0x3C -> no ACK
   oled: panel did not answer -- rendering disabled, bus left idle
   ```
   and in a 45 s steady window afterwards: **0** `[tx]`/telemetry lines,
   88 DNS failures for `mqtt.nordtronics.io` (the node has no working wifi
   credentials on this bench — unrelated to the flash).

3. **The board's I2C devices are present and were working minutes earlier.**
   Before the flash the same `/dev/ttyACM1` ran the legacy bench image and
   printed `[tx] #4/#5/#6 sent ... bme=1 pms=1 (26 bytes)`; 0106's captured
   node console for that image reads
   `Heltec V4.2 + BME680   tag v2.0-node` / `Display : 0x3C -> init OK` /
   `Sensor  : 0x77 -> bound OK`. After the flash the same board sees neither
   device.

4. **The same binary behaves differently on the base**, which is the control
   that localises the cause: the untouched base prints `ROLE: base
   (source=NVS, sensors=absent, nvs_role=1)` **and `oled: probe 0x3C -> ACK`**.
   The panel path works there because that board has no BME680 holding the
   shared bus.

5. **No end-to-end reading exists.** `GET https://api.nordtronics.io/v1/nodes`
   at `2026-10-04T10:25:23Z`:
   `bench-01 ... last_seen_utc 2026-10-02T20:37:37Z, age_seconds 136066,
   status "stale"` (and `node-01` stale). Mosquitto journal: zero telemetry
   publishes; the only authenticated non-loopback client is the base itself
   (`98.97.1.121` as `wf-base-0`, user `base-01`, reconnecting ~1/min).

## The finding (Juno's decision to make)

The artifact's self-detecting role is **unattainable on this hardware**: the
probe resolves `sensors=absent` and `resolve_role()` (951cbd3 `role_detect.cpp`)
falls through `Probe` → `Default` → **base**, so a flashed node is a base
station and never transmits.

Probable cause, verified in source but deliberately not applied: the unified
firmware **never drives the Heltec Ve/Vext gate (GPIO36, active LOW)**. The
legacy bench sketch does — `~/Documents/PlatformIO/Projects/heltec-v3-bme680/src/node.cpp`:
`#define PIN_VEXT 36` … `pinMode(PIN_VEXT, OUTPUT); digitalWrite(PIN_VEXT, LOW);
// Ve rail ON - this is what powers the OLED` — and this repo's
`heltec-lora-board-bringup` skill states GPIO36 gates the Ve rail, that without
driving it "the sensor's VIN sits at 0 V — unpowered exactly while the firmware
runs", and that one unpowered chip clamps SDA and takes the whole bus down.
`git show 951cbd3:firmware/wildfire-node-v1/src/{firmware_config.h,main.cpp}`
contains no GPIO36 drive outside the frozen-net table. That accounts for both
the dead sensor probe and the failing panel probe on the node (BME680 on Ve,
clamping the shared bus) versus the ACK-ing panel on the sensor-less base.

**Fix owed, named and not applied:** add `kOledVextPin = 36`, drive it
`OUTPUT`/`LOW` at the top of `setup()` (both roles) before
`probe_node_sensors()` and `oled_probe_and_begin()`, rebuild, publish a new
artifact, then re-run this task. That is a firmware change — a new branch and
a new artifact — which this task's Constraints ("no repo commits, no new
branches") and step 3 ("do not fix it") both forbid from here.

## Deviations

- **The base board was RESET during this run** (not reflashed: no write command
  was ever issued to `/dev/ttyACM0` or the `80:F1…` by-id, and its banner still
  reports the unchanged `build=Oct  4 2026 01:23:52`). My first capture tooling
  drove the USB-JTAG modem lines on open and the base rebooted. This means the
  criterion "the base board was never reflashed or reset for this task" is
  **not** met as written. Replaced with `stty -F <port> 115200 raw -echo -hupcl`
  + `cat`, which produced two 45 s windows with **0** `ESP-ROM` banners — the
  windows quoted above are those.
- The node's on-device partition table is an **8 MB** layout (app0
  `0x10000`/`0x330000`), not the `default_16MB.csv` the artifact was built
  with. Spec followed literally (app-only at 0x10000, no erase flags, NVS
  untouched); app0 still fits the 1222672 B image. No partition table written.
- The node's previous app image was **overwritten with no local backup** — the
  task asked for the write and forbade erase and did not ask for a dump. A
  `read_flash 0x10000` beforehand would have been free; the legacy image now
  exists only as its out-of-repo Arduino tree.
- `ttyACM0`/`ttyACM1` are **swapped relative to 0097's survey**; every step used
  the by-id path rather than the bare number.

Cost: `deepseek-flash`, standard tier, off-peak (`PEAK: OFF-PEAK 10:15 UTC`).

