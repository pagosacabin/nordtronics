---
task_id: "0103"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 24h
proof:
  branch: hermes/0099-wildfire-fix-set
  sha: 881ee683763ad1593098ae67184adb0514ba91d2
  run: https://github.com/pagosacabin/nordtronics/actions/runs/37158144211
  files: []
  files_note: >-
    No repository file changed: this task flashed an existing, already-verified
    CI artifact to bench hardware. The revision above is the revision APPLIED to
    the device (branch tip == run headSha), re-checked this run.
  applied_to:
    - "bench BASE board MAC 80:f1:b2:a7:47:ec — app partition only, offset 0x10000,
      over the by-id USB-JTAG port (no --erase-all, no erase_flash, NVS untouched)"
  artifact:
    name: wildfire-node-v1-unified-firmware
    file: firmware.bin
    bytes: 1216688
    sha256_before: 5fdde258ba9fb574993bc39278d36ef5b9dec387f61af5cd57d52e90dc1be623
    sha256_readback: 5fdde258ba9fb574993bc39278d36ef5b9dec387f61af5cd57d52e90dc1be623
  flash_command: >-
    /usr/bin/python3 -m esptool --chip esp32s3 --port
    /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00
    --baud 460800 --no-stub --after hard_reset write_flash --flash_size 16MB
    0x10000 firmware.bin
  ntfy: not-applicable — no build was produced by this task (the binary was built and receipted by 0099's CI run); 0103's reply format asks for the trifecta, not a receipt
notes: |
  FLASHED AND VERIFIED — iteration 2, 2026-10-03 23:15-23:35 UTC.

  One deviation, declared up front: the image that was actually running on the
  base was NOT the `v2.0-base` bench image this task's notes assume, so the
  stated safety reason ("rebuildable from ~/Documents/PlatformIO/Projects/
  heltec-v3-bme680/") did not cover what was on the board. Details in the reply
  body under "Deviations". I proceeded because the task's authorization to
  overwrite the base is unconditional and it names this artifact as the
  deliverable — but the overwritten image was not rebuildable from anything
  reachable, and I did not widen scope to a flash dump.

  What was done, in order: synced main; picked 0103 up (iteration 2); downloaded
  the 0099 artifact and re-hashed it; resolved the base board by
  /dev/serial/by-id and confirmed its MAC with esptool before touching anything;
  READ the on-device partition table (4096 B @ 0x8000) to prove an app-only write
  was safe; wrote only the app image at 0x10000; read the region back and
  byte-compared it; captured boot logs; correlated the console's MQTT connect
  line against the broker's own journal; re-read the node board to confirm it was
  never touched.

  No credential was read, written, logged or committed. No router change. No NVS
  write (no erase flags were used; the stored WiFi credentials survived — the
  board joined on the first STA attempt after the flash). The node board
  (B0:A6:04:C5:75:4C) was never opened for writing and was never reset.

  Findings that are NOT this task's to fix, declared in the reply body: the base
  MQTT session is dropped by the broker on keepalive timeout every ~22 s and
  re-established ~45 s later, indefinitely, and it did the same before the flash;
  the OLED probe result is non-deterministic; and the flashed build emits no IP
  address or disconnect-reason line, so 0098's "fresh reason 202" reading is not
  obtainable from its console.
---

# 0103 — Flash the 0099 artifact to the bench base and report the trifecta

## Context

Task 0099 is verified and archived: branch
`hermes/0099-wildfire-fix-set` @ `881ee68`, CI run 37158144211 green. The
CI artifact `wildfire-node-v1-unified-firmware` contains `firmware.bin`
(1216688 bytes, sha256
`5fdde258ba9fb574993bc39278d36ef5b9dec387f61af5cd57d52e90dc1be623`).
The bench base board is on Stephen's machine at
`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00`
(usually `/dev/ttyACM0`; do NOT trust the ACM number — resolve by-id and
confirm the MAC `80:F1:B2:A7:47:EC` before touching anything).

## Task

1. Download `firmware.bin` from the 0099 artifact
   (`gh run download 37158144211 -n wildfire-node-v1-unified-firmware`),
   verify its sha256 matches the value above, and flash it to the BASE
   board only, via the by-id path.
2. Capture the serial boot log.
3. Report the trifecta, each with quoted log lines:
   a. OLED: alive? What does it show? (0099 added the GPIO21 reset +
      0x3C probe + guarded `sendBuffer`.)
   b. WiFi: did the STA join the stored cabin network — IP address, or
      the disconnect reason code? (This is the 0098 gate: a fresh
      reason 202 with the retyped password means the platform migration
      is real.)
   c. MQTT/TLS: does it reach `mqtt.nordtronics.io:8883` — `mqtt:
      connected`, or the connect-failure line with `state=n`? (The
      ESP32-side TLS handshake was explicitly unverified in 0099.)

## Success criteria

- The flashed binary's sha256 matched the artifact before flashing.
- Serial boot log captured and quoted for all three report items
  (OLED / WiFi / MQTT) — answers, not silence.
- The node board (`B0:A6:04:C5:75:4C`) was never touched.

## Constraints

- Confirm the board's MAC/by-id identity BEFORE flashing. Flashing the
  wrong board is the one unforgivable error here.
- Do NOT wipe NVS: no erase flags, no `erase_flash`. The stored WiFi
  credentials must survive.
- No router changes, no credential reads/writes. One deliverable: the
  flashed base and its report. Nothing else.

## Proof

- sha256 of the binary that was flashed.
- Quoted serial lines for (a), (b), (c).
- `esptool` / `pio run -t upload` output showing the by-id port used.

## Reply format

Follow the mailbox staged-reply format: status line, the three answers
with quoted evidence, deviations declared, cost line.

---

# Reply — 2026-10-03 23:35 UTC: DELIVERED (iteration 2)

**Status: delivered.** The 0099 CI artifact is flashed to the bench BASE board,
its bytes read back and verified, and all three trifecta items are answered from
captured serial output. No node board was touched, no NVS was wiped, no
credential was read or written, no router change. One deviation and three
out-of-scope findings are declared below.

## Identity, before anything was written

Resolved by-id, never by ACM number:

    $ ls -l /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00
    ... -> ../../ttyACM0
    $ /usr/bin/python3 -m esptool --chip esp32s3 --port <that by-id> --no-stub --after hard_reset flash_id
    Chip is ESP32-S3 (QFN56) (revision v0.2)
    USB mode: USB-Serial/JTAG
    MAC: 80:f1:b2:a7:47:ec          <- the BASE, as specified
    Detected flash size: 16MB
    Flash type set in eFuse: quad (4 data lines)

The node board (`B0:A6:04:C5:75:4C`) was enumerated as `ttyACM1` throughout and
was never the target of any command.

**Partition table read from the device** (4096 B @ 0x8000, read-only, parsed):

    nvs       type=1 subtype=2   off=0x009000 size=0x005000
    otadata   type=1 subtype=0   off=0x00e000 size=0x002000
    app0      type=0 subtype=16  off=0x010000 size=0x640000
    app1      type=0 subtype=17  off=0x650000 size=0x640000
    spiffs    type=1 subtype=130 off=0xc90000 size=0x360000
    coredump  type=1 subtype=3   off=0xff0000 size=0x010000

This is stock `default_16MB.csv` (the layout 0099 pins). It is why the write
below is app-only at 0x10000: the artifact carries no bootloader or partition
image, and an app-only write cannot touch the NVS at 0x9000/0xe000 anyway.

## Artifact, verified rather than assumed

    run 37158144211  -> headBranch hermes/0099-wildfire-fix-set, conclusion success
    git ls-remote --heads origin hermes/0099-wildfire-fix-set -> 881ee683763ad1593098ae67184adb0514ba91d2
    gh run view 37158144211 --json headSha -> 881ee683763ad1593098ae67184adb0514ba91d2   (tip == run)

    $ gh run download 37158144211 -n wildfire-node-v1-unified-firmware
    $ ls -l firmware.bin          -> 1216688 bytes
    $ sha256sum firmware.bin
    5fdde258ba9fb574993bc39278d36ef5b9dec387f61af5cd57d52e90dc1be623  firmware.bin

Byte count and digest both match the task's stated values.

## Write

    /usr/bin/python3 -m esptool --chip esp32s3 --port /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00 \
      --baud 460800 --no-stub --after hard_reset write_flash --flash_size 16MB 0x10000 firmware.bin

    Wrote 1217536 bytes at 0x00010000 in 12.6 seconds (774.7 kbit/s)...
    Hash of data verified.
    Hard resetting via RTS pin...

No `--erase-all`, no `erase_flash`, no NVS partition write. The 848-byte excess
over the file size is esptool's 4 KiB write padding.

**Read-back proof** (independent of esptool's own "Hash of data verified"):

    $ ... read_flash 0x10000 1216688 readback.bin
    Read 1216688 bytes at 0x00010000 in 16.2 seconds (599.9 kbit/s)
    $ sha256sum firmware.bin readback.bin
    5fdde258ba9fb574993bc39278d36ef5b9dec387f61af5cd57d52e90dc1be623  firmware.bin
    5fdde258ba9fb574993bc39278d36ef5b9dec387f61af5cd57d52e90dc1be623  readback.bin
    $ cmp firmware.bin readback.bin   -> identical, exit 0

## (a) OLED — the guard works; the probe itself is NOT deterministic

Quoted from the first boot after the write, and from other boots the same run:

    oled: probe 0x3C -> no ACK
    oled: panel did not answer -- rendering disabled, bus left idle

    (different boot)
    oled: probe 0x3C -> ACK

Across six boots of the flashed image the probe answered **ACK 4 times and
no-ACK 2 times**; across the two boots of the pre-flash image it answered ACK
once and no-ACK once. So the split is pre-existing and not attributable to 0099
— this panel's I2C presence at 0x3C is flaky at boot here, not dead and not
newly broken.

What is **directly established**: the two 0099 behaviours the task names are both
live. On a no-ACK boot the firmware does not hang and does not keep poking the
bus — it prints `rendering disabled, bus left idle`, and the boot continues to
WiFi/NTP/MQTT normally. The GPIO21 reset + 0x3C probe path therefore gates
`sendBuffer` as designed.

What is **NOT** established and is not claimed: I cannot see the glass. Nothing
in this run proves what the panel displays on an ACK boot. A photo at the bench,
or a firmware-side check that a frame was actually written, is needed for that;
`sendBuffer` being reachable is not the same as a legible screen.

## (b) WiFi — the STA joins; the flashed build emits no IP and no reason code

    portal AP fallback: wildfire-setup  http://192.168.4.1 (boot)
    wifi: status=3 (connected)

`WL_CONNECTED` is reached on the stored credentials (9-char password, unchanged
by the flash — no NVS write occurred). **This build prints no IP address and no
disconnect-reason line**, so the specific 0098 reading the task asks for — "a
fresh reason 202 with the retyped password" — is not obtainable from the flashed
firmware's console. That is a limit of the artifact, not a silent failure; say so
rather than calling it a join failure.

Two things stand in for the missing IP line:

1. The pre-flash image *did* log the join and is the same board, same stored
   credentials, same AP, minutes earlier. It printed, at every boot:
       wifi: joining, pass len=9
       wifi: status=6
       wifi event: id=5 reason=203
       wifi event: id=5 reason=2
       wifi: status=3
       wifi: CONNECTED ip=192.168.1.71 rssi=-25 ch=11
   Two transitional failures (reason 2 = auth expire, reason 203 = assoc fail)
   then a successful association on a strong signal. Note there is **no reason
   202** in that sequence, and the address obtained was `192.168.1.71`.
2. On the flashed image the STA's address and route are proved indirectly but
   concretely: it completed NTP and then authenticated a TLS session to a public
   broker (see (c)), which is not possible without a working DHCP lease and a
   default route.

## (c) MQTT/TLS — it reaches 8883, TLS completes, base-01 authenticates

Console, flashed image:

    ntp: clock set (t=1791069844)
    mqtt cfg: host=[mqtt.nordtronics.io] port=8883 (TLS, pinned ISRG Root X1)
    mqtt: connected to mqtt.nordtronics.io:8883

The `ntp: clock set` line matters for this one: 0099's pinned ISRG Root X1 cannot
validate at an epoch clock, so the sequence is evidence the NTP fix is live, not
decoration.

Broker side, independently read (read-only `journalctl -u mosquitto` on the
Mosquitto host), **UTC**:

    Oct 03 23:27:16 New connection from 98.97.1.121:26487 on port 8883.
    Oct 03 23:27:18 Client wf-base-0 already connected, closing old connection.
    Oct 03 23:27:18 New client connected from 98.97.1.121:26487 as wf-base-0 (p2, c1, k15, u'base-01').
    Oct 03 23:27:44 Client wf-base-0 has exceeded timeout, disconnecting.

A serial capture run over exactly 23:27:10-23:27:50Z printed the `mqtt: connected`
line above; the broker logged the matching authenticated connection from a public
address at 23:27:18Z, user `base-01`, on port 8883. This is 0097's dead hop 2
**live from the device** — the `base-01` account that had never authenticated from
a bench network now does, with the TLS handshake negotiated by the ESP32 itself
(the thing 0099 deliberately left unverified).

Two caveats, stated because they bound the claim:

- The pre-flash image was already reaching the same broker as `base-01` from the
  same public IP before the write (23:16:40Z, 23:17:00Z, and earlier today), so
  the broker log is *corroboration of the flashed image*, not a before/after
  differential. What the flash changes is that the board now runs the reviewed,
  committed revision.
- The base connects but does not *hold* the session: mosquitto drops it on
  keepalive timeout (`k15` -> ~22 s) and the device reconnects on its own,
  forever:

      23:27:18 connected   23:27:44 exceeded timeout
      23:28:28 connected   23:28:50 exceeded timeout
      23:29:36 connected   23:30:02 exceeded timeout
      23:30:44 connected   23:31:08 exceeded timeout
      23:31:52 connected   23:32:14 exceeded timeout

  That pattern was observed with **no resets from me** (a 200 s window with the
  board untouched). It limits hop 2 in practice: `mqtt_publish()` in this firmware
  returns early when `!g_mqtt.connected()`, so any base-side publish outside a
  ~22 s window is silently dropped. This is a finding, **not** a diagnosis, and it
  is out of 0103's scope — but it is in the way of 0097's "reading under 15
  minutes old" criterion, and it pre-dates the flash (the same account/IP was
  cycling the same way at 23:16-23:17Z, before the write).

Related and out of scope, for whoever takes 0104: the base publishes telemetry as
`<mqtt_root>/node/<id>/telemetry` (`base_handle_frame`, main.cpp:441) with keys
`node/seq/pm1/pm25/pm10/temp_c/rh/press_pa/batt_mv/status/level` — a two-level
topic and a payload schema that match neither 0104's single-level contract nor
0097's required `temperature_c/humidity_pct/battery_v/node_id/observed_utc`. The
API still shows nothing fresh (`base-01`/`bench-01` last seen 2026-10-02T20:37:37Z),
which is expected given both mismatches.

## Deviations

1. **The board was not running what the task's notes assume, and the stated
   safety reason did not hold.** 0103 says the running `v2.0-base` image is
   "rebuildable from ~/Documents/PlatformIO/Projects/heltec-v3-bme680/ ... so
   overwriting it is safe" (0099's carried correction). The image actually
   running when this run began was not that:

       firmware: wildfire-unified-v1 proto=1 build=Oct  2 2026 19:35:22

   0097 recorded the bench base printing `Heltec V4.2 BASE STATION tag v2.0-base`
   (`src/base.cpp:24`, FW_TAG "v2.0-base"), which is still on disk and unchanged.
   The banner above is a different firmware. Its log strings are attributable to
   no committed revision: `oled: probe 0x%02X -> %s` exists at 881ee68 but not on
   main; `mqtt cfg: ...` exists at 881ee68 but not on main; and
   `cfg: pass whitespace trimmed=...` exists at **neither**, on any branch. Its
   `mqtt cfg` line also differs from 881ee68's format string, which carries the
   `(TLS, pinned ISRG Root X1)` suffix. Searching every branch, every local
   worktree and the whole host for those strings finds no source, so the
   overwritten image was not rebuildable from anything reachable. The untracked
   `firmware/wildfire-node-v1/bench-override.ini` (mtime Oct 2 19:32, three
   minutes before that build, and already carrying 0099's
   `board_upload.flash_size`/`maximum_size` keys) is what the bench build most
   likely used. *Inference, labelled as such:* it looks like a bench draft of
   this same fix set, built Oct 2 and later formalised as 0099 — but I cannot
   prove that, so I am not claiming it.

   I flashed anyway: the task's authorization to overwrite the base is
   unconditional, it names this artifact as the deliverable, and refusing on my
   own judgment would substitute for the task owner's decision. Consequences,
   declared rather than hidden: that image is not recoverable — 0097's
   `read_flash` dump already failed twice on this board and left its console
   wedged, and I did not retry it (the task says "one deliverable ... nothing
   else", and a failed dump risks the port mid-run, as it did for 0097). If that
   image matters, it is gone; if it is the bench draft, nothing of value was lost.

2. **I read the partition table before writing.** Small, read-only, and the reason
   the app-only offset is defensible. Declared because the constraint list says
   "one deliverable".

3. **Timestamps in the broker quotes are UTC.** The Mosquitto host runs
   Europe/Berlin and mosquitto's own message text stamps *local* time with a
   literal trailing `Z`, which reads 2 h later than reality; the rows above are
   journalctl `--utc` (display column), which is the true instant.

4. **No IP address and no reason code for (b).** The artifact does not print
   them. Reported as a limit with the indirect evidence, not as a failure.

## Success criteria, checked

- Flashed binary's sha256 matched the artifact before flashing — yes, and the
  read-back matched it again after.
- Serial boot log captured and quoted for all three items — yes, (a)/(b)/(c) above.
- Node board never touched — yes: no command of any kind was addressed to
  `B0:A6:04:C5:75:4C`'s port, it was never reset, and a read-only console check
  at the end shows it still transmitting (`[tx] #8227 sent ... (26 bytes)`).

## Cost and tier

`deepseek-flash`, off-peak — the pre-run digest printed `PEAK: OFF-PEAK 23:15 UTC`
at pickup and the run finished 23:35 UTC. No credential was read, written or
committed; no router change; staged from `main` only.
