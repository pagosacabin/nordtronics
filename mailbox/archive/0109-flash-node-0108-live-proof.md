---
task_id: "0109"
protocol_version: 1.0.0
status: closed-superseded
iteration: 2
expect-reply-within: 24h
proof: []
notes: |
  CLOSURE (closed-superseded), 2026-10-08: Superseded by the 0119 recovery (Stephen's option A); closed 2026-10-08 per Stephen's decision.

  Filed by Juno, 2026-10-04, at Stephen's explicit "Send it" (07:21 MDT).
  Supersedes 0107's flash half (0105 artifact, BLOCKED on the Vext root
  cause): flash the 0108 artifact (Vext gate driven at boot) to the NODE
  board and prove the whole stack live. 0107's diagnostic notes stay in
  active/ as the honest record of the finding.
  PICKED UP (filed iteration 1 -> 2) by the mailbox worker, 2026-10-04 15:15 UTC.
  Off-peak (PEAK: OFF-PEAK 15:15 UTC). Predecessors 0097/0098/0106/0107 remain
  decision-blocked in active/ and were not touched. The filed front-matter
  carried iteration: 1 (README says iteration starts at 0); the pickup
  increment is applied literally, so this task now reads iteration: 2.

  BLOCKED (iteration 2, 2026-10-04 15:15-15:30 UTC) — NO proof block, not
  staged. The FLASH HALF IS DONE AND VERIFIED; the live-proof half is
  unattainable this run because the NODE BOARD FELL OFF THE USB BUS during the
  post-write hard reset and has not re-enumerated. Exact state:

  1. DONE — identity confirmed BEFORE any write, via the by-id path the task
     mandates: /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00
     -> esptool read_mac -> "MAC: b0:a6:04:c5:75:4c" (the NODE). The base was
     left alone and independently re-confirmed as 80:f1:b2:a7:47:ec (udevadm
     ID_SERIAL on /dev/ttyACM0).

  2. DONE — the 0108 artifact was fetched fresh and verified against the spec.
     gh run view 37201631922: conclusion success, headSha
     255e0ce60926c9bfbf46d99056faac880cf5b722 (== the SHA in the task text),
     workflow "PlatformIO Build", createdAt 2026-10-04T12:18:10Z.
     artifact 11302294566 "wildfire-node-v1-unified-firmware", 769803 B zipped,
     expired false -> firmware.bin 1222688 B, sha256
     36dd5ddf9bd3cecef667e67cee93f544a746cc89bfa3925d18bbb887cc440a95.
     Partition table read from the NODE FIRST (4096 B @ 0x8000, by-id):
       nvs 0x009000/0x005000 (end 0x00e000)  <- the app write cannot reach it
       otadata 0x00e000/0x002000
       app0 0x010000/0x330000 (end 0x340000)
       app1 0x340000/0x330000  spiffs 0x670000/0x180000  coredump 0x7f0000/0x10000
     (the 8 MB layout 0107 found; 16 MB flash detected). App-only write at
     0x10000 of 1222688 B occupies 0x010000-0x22a820, inside app0, far above
     NVS. `esptool --chip esp32s3 --baud 921600 --after hard_reset write_flash
     --flash_size 16MB 0x10000 firmware.bin` -> "Wrote 1222688 bytes (745182
     compressed) at 0x00010000 in 10.2 seconds ... Hash of data verified."
     No --erase-all, no erase_flash. Read-back 0x10000 1222688 B -> sha256
     identical (36dd5ddf...) and `cmp` exit 0. Success criterion 1 is met.

  3. BLOCKED — NO NODE BOOT LOG, NO ROLE LINE. The node's USB device
     disconnected during the read-back's hard reset and never came back:
       kernel: Oct 04 09:17:15 home kernel: usb 1-2: USB disconnect, device
               number 7        (= 15:17:15 UTC)
     It is the LAST usb event in the log; no attach/reset has followed in the
     ~13 minutes since. `/sys/bus/usb/devices/1-2` no longer exists,
     `/sys/bus/usb/drivers/usb/` lists only 1-3, 3-2, 3-3, 3-4, and `lsusb`
     shows a single 303a:1001 device (the base, ID_SERIAL 80:F1:B2:A7:47:EC).
     No ttyUSB*, and /dev/serial/by-id/ holds only the base. Software
     recovery was checked and is not available to this account: no uhubctl
     anywhere on the host, no /etc/sudoers.d entry, no polkit rule, and
     `sudo -n` fails ("a password is required"), so the port cannot be
     re-authorized/rebound. This needs a PHYSICAL re-plug or power-cycle of
     the node board (B0:A6:04:C5:75:4C). Success criterion 2 is NOT met.

  4. BLOCKED — the stack is NOT proven live, and the evidence says it is not
     live. Read-only, non-perturbing (the task's own recipe: `stty -F <port>
     115200 raw -echo -hupcl` + `cat`), three captures of the BASE console
     totalling 370 s produced ZERO `rx:` lines — the only output was
     "mqtt: connected to mqtt.nordtronics.io:8883", once per ~60 s.
     Broker (read-only ssh deploy@89.117.21.105, journalctl -u mosquitto):
     from 17:09 to 17:26 the base authenticates repeatedly as
       "New client connected from 98.97.1.121:PORT as wf-base-0
        (p2, c1, k15, u'base-01')"
     and every session is then killed with
       "Client wf-base-0 has exceeded timeout, disconnecting."
     (~24-35 s after each connect: keepalive 15 is not being serviced).
     There is no publish line for any nordtronics/wildfire/... topic anywhere
     in the window. API agrees: GET https://api.nordtronics.io/v1/nodes
     (2026-10-04T15:26:35Z) -> bench-01 last_seen_utc 2026-10-02T20:37:37Z,
     age_seconds 154138 (~42.8 h), status "stale", reading_count 1. Success
     criterion 3 is NOT met. No fresh reading exists, so there is nothing to
     screenshot as non-stale either.

  5. BASE BOARD — NOT written, NOT reset (success criterion 4 met).
     No esptool/unlock/write was ever run against 80:F1:B2:A7:47:EC. The only
     access was the task's prescribed non-perturbing console read. Confirming
     evidence that no reset occurred: (a) none of the three captures contains
     a boot banner (a reset would print the full `ROLE: ... firmware: ...`
     sequence; the captures begin mid-run), and (b) the broker journal shows
     an unbroken ~70 s reconnect cadence across the whole window with no
     reboot gap. Per the "your own capture tooling can reset the board" rule
     I captured twice more and grepped for `ESP-ROM`/banner text: none.

  6. MECHANISM OF THE NODE DROP — UNDETERMINED, and the flash is not shown to
     be innocent. What is established: the chip was fully functional AFTER the
     write (the 1222688 B read-back matched byte-for-byte). The disconnect
     happened at/after the read-back's `--after hard_reset`, i.e. while the
     newly-written app was taking over. So I can neither confirm nor exclude
     that the 0108 image (which drives GPIO36 at the top of setup()) leaves the
     node in a state where the USB-JTAG peripheral does not re-enumerate, or
     that the host port itself latched off. This is NOT the 0097 "ACM minor
     renumbers" case (that board always came back); the port is gone from the
     USB stack entirely. Deliberately NOT fixed by guessing: no alternate
     image, no NVS write, no erase, nothing flashed a second time.

  7. NOT DONE / NOT TOUCHED. No NVS write or wipe on either board; no repo
     commit and no branch for the task (apply task; the mailbox pickup commit
     is the only repo change); no credential read or written; no router change;
     no defect hunt beyond the evidence above. Raw captures and the binary are
     outside the repo in
     ~/.hermes/profiles/cronrunner/cache/scratch/0109/ (firmware.bin,
     node-readback.bin, node-parttable.bin, base-console{,2,3}.txt).

  DECISION OWED (and the one thing the next tick cannot do for itself):
    a) PHYSICAL ACTION FIRST — re-plug or power-cycle the NODE
       (B0:A6:04:C5:75:4C) so it re-enumerates, then read its console. The
       0108 app is already in app0 and read-back-verified; a re-read of the
       ROLE line is all that stands between this task and staging.
    b) If the node comes back and re-flashing 0108 reproduces the drop, decide
       whether to (i) treat the USB re-enumeration failure as a hardware/port
       issue and re-flash once more, or (ii) gate the GPIO36/role change behind
       a delay so the USB-JTAG peripheral settles before setup() touches the
       rail. That is a repo change and therefore not mine to take on an
       "apply task only" brief.
    c) The base's MQTT session cycling (connect -> keepalive timeout, no
       publish) is a second, independent open question that 0109 does not ask
       me to fix; it blocks the end-to-end proof even once the node is back.
    Cost context: deepseek-flash, standard tier; PEAK: OFF-PEAK 15:15 UTC.
---

# 0109 — Flash the 0108 artifact to the bench NODE and prove live data

## Context

0108 (branch `hermes/0108-vext-and-pms` @
`255e0ce60926c9bfbf46d99056faac880cf5b722`, CI run 37201631922, artifact
id 11302294566) drives GPIO36 (Vext, active LOW) at the top of `setup()`
in both roles, before role detection. Root cause it fixes: the BME680 is
wired VIN->Vext, sat unpowered, clamped I2C, and the node probed as base.
Base board (`80:F1:B2:A7:47:EC`) already runs the 0105 build and reaches
the broker — do not touch it.

## Task

1. Confirm the NODE board by-id/MAC
   (`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`,
   esptool read_mac `b0:a6:04:c5:75:4c`) BEFORE any write. Use the by-id
   path for every step — never the bare ttyACM number (they swap).
2. Download the 0108 artifact, verify sha256, flash `firmware.bin` to the
   NODE only: app-only write at 0x10000, no erase flags, NVS untouched.
   Read the node's partition table first to confirm the app write cannot
   reach NVS (0107 found an 8 MB layout on this board).
3. Capture the node boot log and report the ROLE line — expect
   `ROLE: node (source=PROBE, sensors=present, ...)` with the BME680 now
   visible. Quote it. Also quote the BME680 probe line and, if the
   PMS5003's external power is on, its frame line; if that power is off,
   declare it (Stephen: it was off during 0107's testing) — no defect
   hunt either way.
4. Prove the stack end to end, quoting each hop:
   - Node console: LoRa TX lines with the new params (915/125/SF7/CR5,
     27-byte v1 frames).
   - Base console: `rx:` lines — it hears the node. (Non-perturbing
     capture: `stty -F <port> 115200 raw -echo -hupcl` + `cat`; do NOT
     reset the base — 0107's pyserial opens rebooted it.)
   - Broker journal: the telemetry publish arrives.
   - API: `curl -s https://api.nordtronics.io/v1/nodes` shows a fresh
     reading (`last_seen_utc` < 15 min, not `stale`). Quote the row and
     name the node ID it publishes under (expect "0" — unprovisioned;
     acceptable, declared not fixed).

## Success criteria

- Flashed binary matches the 0108 artifact (sha256 before, read-back
  after, `cmp` clean).
- Node boot log shows `ROLE: node` from the sensor probe — quoted.
- `GET /v1/nodes` shows a reading fresher than 15 minutes — quoted.
- The base board received no write and no reset.

## Constraints

- MAC/by-id identity check BEFORE flashing. Wrong board = failure.
- No NVS wipe on either board. No router changes. No credential
  reads/writes. No repo commits, no new branches — apply task only.

## Proof

- sha256 of the flashed binary + read-back match.
- Quoted node boot ROLE line and BME680 probe line.
- Quoted base `rx:` line(s).
- Quoted `GET /v1/nodes` row with fresh `last_seen_utc`.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.

---

## Reply — BLOCKED (iteration 2), no `proof` block, not staged

**Status: `in_progress` in `active/`.** There is no `proof` block and no staged
file: 2 of the 4 success criteria are met, and the two that are not are blocked
by a hardware event this run cannot work around. Per `mailbox/README.md` the
file stays in `active/` with everything declared in the front-matter `notes:`
above — this body is the reader-facing summary.

### Criterion-by-criterion

| # | Criterion | Result |
|---|---|---|
| 1 | Flashed binary matches the 0108 artifact (sha256 before, read-back after, `cmp` clean) | **MET** |
| 2 | Node boot log shows `ROLE: node` from the sensor probe — quoted | **NOT MET** — no node console exists |
| 3 | `GET /v1/nodes` shows a reading fresher than 15 minutes — quoted | **NOT MET** — `bench-01` is 42.8 h stale |
| 4 | The base board received no write and no reset | **MET** |

### 1. Identity, before any write (the task's own hard gate)

    $ ls -l /dev/serial/by-id/
    ..._80:F1:B2:A7:47:EC-if00 -> ../../ttyACM0      (BASE, untouched)
    ..._B0:A6:04:C5:75:4C-if00 -> ../../ttyACM1      (NODE)
    $ esptool --chip esp32s3 --port /dev/serial/by-id/...B0:A6:04:C5:75:4C-if00 read_mac
    MAC: b0:a6:04:c5:75:4c

The by-id path was used for every step, never a bare `ttyACM` number (they swap:
0097 saw `ttyACM1 -> ttyACM2 -> ttyACM1` on the base). The base's identity was
re-confirmed independently: `udevadm info -n /dev/ttyACM0` ->
`ID_SERIAL=Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC`.

### 2. Artifact fetched fresh and verified

    $ gh run view 37201631922 -R pagosacabin/nordtronics --json headSha,conclusion,workflowName,createdAt
    {"conclusion":"success","createdAt":"2026-10-04T12:18:10Z",
     "headSha":"255e0ce60926c9bfbf46d99056faac880cf5b722",
     "workflowName":"PlatformIO Build"}
    $ gh api .../actions/artifacts/11302294566/zip > artifact.zip     # 769803 B
    $ unzip artifact.zip
      inflating: firmware.bin                                        # 1222688 B
    $ sha256sum firmware.bin
    36dd5ddf9bd3cecef667e67cee93f544a746cc89bfa3925d18bbb887cc440a95

The run's `headSha` equals the SHA the task names, and the run is green — both
re-read live, not taken from the task text.

### 3. Partition table read from the device FIRST, then the app-only write

    nvs       0x009000/0x005000   (end 0x00e000)   <- unreachable from 0x10000
    otadata   0x00e000/0x002000
    app0      0x010000/0x330000   (end 0x340000)
    app1      0x340000/0x330000
    spiffs    0x670000/0x180000
    coredump  0x7f0000/0x010000

`firmware.bin` at `0x10000` spans `0x010000`-`0x22a820`, entirely inside `app0`
and entirely above `nvs` (which ends at `0x00e000`). So the write structurally
cannot touch NVS — the criterion is satisfied by layout, not by hope.

    $ esptool --chip esp32s3 --port /dev/serial/by-id/...B0:A6:04:C5:75:4C-if00 \
        --baud 921600 --after hard_reset write_flash --flash_size 16MB 0x10000 firmware.bin
    Wrote 1222688 bytes (745182 compressed) at 0x00010000 in 10.2 seconds (effective 955.8 kbit/s)...
    Hash of data verified.
    $ esptool ... read_flash 0x10000 1222688 node-readback.bin
    Read 1222688 bytes at 0x00010000 in 15.1 seconds
    $ sha256sum firmware.bin node-readback.bin
    36dd5ddf9bd3cecef667e67cee93f544a746cc89bfa3925d18bbb887cc440a95  firmware.bin
    36dd5ddf9bd3cecef667e67cee93f544a746cc89bfa3925d18bbb887cc440a95  node-readback.bin
    $ cmp firmware.bin node-readback.bin ; echo $?
    0

No `--erase-all`, no `erase_flash`.

### 4. The blocker: the NODE fell off the USB bus and did not come back

Immediately after the read-back's `--after hard_reset`:

    Oct 04 09:17:15 home kernel: usb 1-2: USB disconnect, device number 7

That is still the most recent USB event. In the ~13 minutes since:
`/sys/bus/usb/devices/1-2` does not exist, `/sys/bus/usb/drivers/usb/` lists
only `1-3 3-2 3-3 3-4`, `/dev/serial/by-id/` holds only the base's symlink, and
`lsusb` shows exactly one `303a:1001` (the base). Both boards were enumerated
before the flash; the node is simply gone from the stack.

I looked for a way to recover it without hands and found none available to this
account: no `uhubctl` binary anywhere on the host, no `/etc/sudoers.d/`, no
polkit rule, and `sudo -n` refuses ("a password is required"). The port cannot
be re-bound or re-authorized from here. **This needs a physical re-plug or
power-cycle of the node (B0:A6:04:C5:75:4C).**

### 5. The stack is not live — and the evidence says so, not "unverified"

Three non-perturbing captures of the BASE console (the task's own recipe,
`stty -F <port> 115200 raw -echo -hupcl` + `cat`), 370 s total: **zero `rx:`
lines.** The only line was:

    mqtt: connected to mqtt.nordtronics.io:8883

Broker side (read-only, `deploy@89.117.21.105`), 17:09-17:26:

    New client connected from 98.97.1.121:PORT as wf-base-0 (p2, c1, k15, u'base-01')
    Client wf-base-0 has exceeded timeout, disconnecting.

The base authenticates cleanly and then is dropped ~24-35 s later, every cycle,
because keepalive 15 is never serviced; no `nordtronics/wildfire/...` publish
appears anywhere in the window. The API agrees — `GET /v1/nodes` at
`2026-10-04T15:26:35Z`:

    {"node_id":"bench-01","last_seen_utc":"2026-10-02T20:37:37Z","age_seconds":154138,
     "status":"stale","reading_count":1,"latest":{"pm25":9.2,"temperature_c":23.0,
     "humidity_pct":45.1,"battery_v":4.05}}

### Deviations and self-caught items

- **The node reset is mine and was expected** — a flash task resets the target.
  What was not expected is that it did not re-enumerate. Declared, not hidden.
- **The flash is not proven innocent.** The read-back matched byte-for-byte, so
  the chip was healthy *after* the write; the drop coincided with the new app
  taking over. I neither confirm nor exclude that the 0108 image (GPIO36 driven
  at the top of `setup()`) leaves the USB-JTAG peripheral unable to re-enumerate.
  I did **not** guess a fix: no second flash, no alternate image, no erase, no
  NVS write.
- **No base reset** (success criterion 4). No `esptool` was ever pointed at the
  base; only the prescribed console read, which with `-hupcl` asserts nothing on
  close. Grepped all three captures for boot/`ESP-ROM` banner text — none, and
  the broker's reconnect cadence is unbroken across the window, so the base did
  not reboot under me.
- **Filing iteration.** The task was filed with `iteration: 1` (README says
  iteration starts at 0); the literal pickup increment makes this `iteration: 2`.
- **No defect hunt** on the PMS path was performed (its external power state
  could not be observed at all — the node console is gone), so the task's
  "declare it either way" clause is unresolvable this run and is folded into the
  blocker rather than guessed at.

### What would unblock this

A physical re-plug of the node, then a console read for the `ROLE:` line. The
0108 app is already in `app0` and read-back-verified; nothing else needs to be
flashed. The base's MQTT session cycling is a separate open question that blocks
the end-to-end proof even after the node returns.

### Cost

`deepseek-flash`, standard tier; `PEAK: OFF-PEAK 15:15 UTC` at pickup.
`PROTOCOL: MATCH protocol_version=1.0.0`. Raw captures (outside the repo):
`~/.hermes/profiles/cronrunner/cache/scratch/0109/`.
