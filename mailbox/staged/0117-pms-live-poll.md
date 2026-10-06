---
task_id: "0117"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: "hermes/0117-pms-live-poll"
  sha: "a37e1e71fe6f6a0bd6fe79466865d26419e9040c"   # == git ls-remote --heads origin hermes/0117-pms-live-poll
  base: "hermes/0115-pms-uart-diag @ 8435fb3814a8fe81ffe54856ef121fa705824468 -- the revision the task names as the base; the task branch is that tip + 1 commit (51 insertions, 0 deletions)"
  run: "https://github.com/pagosacabin/nordtronics/actions/runs/37502579384"
  run_check: "gh run view 37502579384: status completed, conclusion success, headSha a37e1e71fe6f6a0bd6fe79466865d26419e9040c == the branch tip, createdAt 2026-10-06T17:18:54Z. Jobs: build success and host-tests success. Named steps in the build job: 'Build firmware (wildfire-node-v1, unified node+base)' success, 'Upload wildfire-node-v1 firmware' success. host-tests quoted 27 test cases: 27 succeeded."
  artifact: "https://github.com/pagosacabin/nordtronics/actions/runs/37502579384/artifacts"
  artifact_detail: "wildfire-node-v1-unified-firmware, id 11430746548, 765641 B zipped, expired false. Downloaded -> firmware.bin 1216256 B, sha256 299e86eeb9d36c7504b6bad0ded74dd7101958fba97ed3ea54e7677b31945806 -- the exact image flashed to the node."
  flashed_to: "NODE B0:A6:04:C5:75:4C only, via its by-id path usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00 -> ttyACM0, app-only at 0x10000 (write spans 0x10000-0x228FC0, inside app0 0x10000-0x340000), read-back byte-identical (sha256 299e86ee..., cmp exit 0). No erase_flash, no --erase-all, no NVS write. BASE 80:F1:B2:A7:47:EC was never opened, never named to esptool, and had no USB event all run."
  ntfy:
    topic: nordtronics-build-ed05a663
    id: z2D7LNj7zGok
    time: "1791307755 (2026-10-06T17:29:15Z)"
    body: "Branch / SHA / Workflow / Artifacts / Status: success"
  files:
    - firmware/wildfire-node-v1/src/main.cpp
notes: |
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-06 17:15 UTC
  (11:15 MDT). Off-peak (PEAK: OFF-PEAK 17:15 UTC). The filed front-matter had
  no `iteration` field (protocol field); it was added at pickup and the pickup
  increment applied literally, so this task reads iteration: 1.
  Predecessors 0097/0098/0106/0107/0109/0110 remain decision-blocked/parked in
  active/ and were NOT touched (handoff-mailbox rule 6; 0111 explicitly says
  "Do NOT touch 0110").

  STAGED (iteration 1, 2026-10-06 17:15-17:38 UTC). Model tier: DeepSeek Flash.
  HEADLINE: the live poll is running at ~1 Hz on the node (195 `pms-live:` lines
  in a 200 s window = 0.975/s) and the 60 s check-in is unaffected -- three
  consecutive check-ins fired at 60.0 s intervals WITH the poll running, and the
  backend kept receiving node "0" readings the whole time.

  1. WHAT WAS BUILT (criterion 1 MET). Branch hermes/0117-pms-live-poll from
     hermes/0115-pms-uart-diag @ 8435fb3. Exactly ONE functional change, in
     firmware/wildfire-node-v1/src/main.cpp (+51/-0):
       - a new static function pms_live_poll(), placed beside read_pms25();
       - one call site in the NODE half of loop(), OUTSIDE the check-in gate.
     It prints the spec's line verbatim:
       pms-live: pm1=<u> pm25=<f> pm10=<u>
     NON-BLOCKING, as the constraint demands: it drains only bytes ALREADY in
     the UART1 RX ring (loop exits the moment g_pms.available() is 0) and carries
     a hard 5 ms deadline -- well under the 1 s period. It does NOT call
     read_pms25(): that function's 1500 ms blocking scan stays on the check-in
     path only. A frame straddling a poll is carried in static parser state and
     completed by the next poll (a 32-byte frame is ~33 ms on the wire at
     9600 baud). No checksum-mismatch logging, so a dropped frame cannot turn
     the diagnostic into log spam -- this is also why the reported line rate is
     0.975/s rather than a saturated 1.000/s (a poll with no complete frame
     prints nothing). No config change, no deep-sleep change, no merge, no tag.
     The 0115 per-check-in `pms:` line is untouched and still fires (quoted below).

  2. BENCH DISCIPLINE (criterion 2 MET). Identity was confirmed BEFORE any write
     through the mandated by-id path: esptool read_mac -> "MAC: b0:a6:04:c5:75:4c"
     (the NODE). The partition table was read from the device FIRST (4096 B at
     0x8000): nvs 0x9000/0x5000, otadata 0xe000/0x2000, app0 0x10000/0x330000,
     app1, spiffs, coredump -- so the app-only write at 0x10000 cannot reach NVS.
     write_flash 0x10000 --flash_size 16MB (no --erase-all, no erase_flash) ->
     "Wrote 1216256 bytes ... Hash of data verified." Read-back of 1216256 B:
     sha256 299e86eeb9d36c7504b6bad0ded74dd7101958fba97ed3ea54e7677b31945806 ==
     the built image, `cmp` exit 0. BASE never touched: /dev/ttyACM2 was never
     opened, no esptool command ever named 80:F1:B2:A7:47:EC, and `journalctl -k`
     shows no USB event on bus 3-2 (the base's port) for the whole run.

  3. THE LIVE POLL AND THE CHECK-IN COEXIST (criterion 3 MET).
     (a) Boot banner, quoted verbatim from a capture started at reset:
           ROLE: node (source=PROBE, sensors=present, nvs_role=unset)
           firmware: wildfire-unified-v1 proto=1 build=Oct  6 2026 17:20:57
           node: deep sleep gate=DISABLED (bench phase) -- check-in period 60 s ...
         The build string 17:20:57 UTC is inside run 37502579384's window (the
         run was created at 2026-10-06T17:18:54Z), i.e. the flashed image is
         this run's artifact and not a stale one.
     (b) ~1 Hz live poll, 200 s steady window (timestamps are seconds into the
         capture, from the reader, not the device clock):
           [ 34.840] pms: rx_avail_before=32 ok=1 pm1=0 pm25=0.0 pm10=0
           [ 35.280] tx: type=1 node=0 seq=6 len=27 -> sent
           [ 35.312] bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
           [ 94.870] pms: rx_avail_before=32 ok=1 pm1=0 pm25=0.0 pm10=0
           [ 95.310] tx: type=1 node=0 seq=7 len=27 -> sent
           [ 95.342] bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
           [154.898] pms: rx_avail_before=32 ok=1 pm1=0 pm25=0.0 pm10=0
           [155.338] tx: type=1 node=0 seq=8 len=27 -> sent
           [155.370] bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
         Three consecutive check-ins at 60.00 s spacing (35.28 -> 95.31 -> 155.34),
         each still transmitting, with the poll running between them. A separate
         first 150 s window shows the same shape from boot: seq=0 at t=0.207,
         seq=1 at t=59.236, seq=2 at t=119.505 -- 59.03 s and 60.27 s.
         `pms-live:` count in the 200 s window: 195 lines (0.975/s, ~1 Hz as the
         criterion asks). Five CONSECUTIVE `pms-live:` lines ~1 s apart, from the
         same window:
           [  2.373] pms-live: pm1=0 pm25=0.0 pm10=0
           [  3.374] pms-live: pm1=0 pm25=0.0 pm10=0
           [  4.375] pms-live: pm1=0 pm25=0.0 pm10=0
           [  5.376] pms-live: pm1=0 pm25=0.0 pm10=0
           [  6.377] pms-live: pm1=0 pm25=0.0 pm10=0
     (c) Backend, read-only over HTTPS. GET https://api.nordtronics.io/v1/nodes
         at 2026-10-06T17:37:31Z:
           node_id "0"  last_seen_utc 2026-10-06T17:36:43Z  age_seconds 49
           status "ok"  reading_count 1151
           latest {pm25 0.0, temperature_c 23.93, humidity_pct 39.55, battery_v 0.0}
         and GET /v1/nodes/0/readings?limit=6, consecutive gaps
         59 s, 60 s, 60 s (17:33:44, 17:34:43, 17:35:43, 17:36:43). The check-in
         cadence through node -> LoRa -> base -> MQTT -> backend is intact.

  4. A NUMBER THAT IS ZERO, DECLARED. Every `pms-live:` line reads
     pm1=0 pm25=0.0 pm10=0. This is NOT a defect in the poll and not a
     regression from this change: 0115 (archived, verified) established that this
     PMS5003 on the bench streams frames whose particle fields are all zero
     (checksum valid, ok=1, values 0/0.0/0). The poll prints the fields the frame
     actually carries; the all-zero frame content is 0115's separate open
     finding and is untouched by this task. Stephen's stated purpose for the task
     -- seeing the line react to a stimulus at the inlet -- therefore still
     cannot be demonstrated until that separate finding is resolved. The
     falsifiable claim of THIS task (a ~1 Hz live line coexisting with the 60 s
     check-in) is proven above.

  5. ONE LOSSY HOP, DECLARED. The backend reading list contains one 181 s gap
     (17:30:43Z -> 17:33:44Z, two missing check-ins). It is NOT the check-in
     cadence: the node's own serial in the same period shows exact 60.0 s
     transmits, and the backend gaps immediately before and after are
     59 s/60 s/60 s. The node -> base -> MQTT path is the lossy hop (LoRa), not
     the poll -- the poll never touches the radio.

  6. MECHANICAL NOTE (a trap worth recording, not a deviation). A plain
     `cat /dev/ttyACM0` on this USB-Serial/JTAG sometimes returns instantly with
     0 bytes even though the app is running; a non-blocking O_NONBLOCK reader
     read the same port normally and produced every capture above. The captures
     were taken with such a reader (open with O_NONBLOCK, no line-state changes).
     Kernel log corroborates non-perturbation: no USB attach/disconnect on the
     node's port (bus 1-2) during the capture windows.

  7. WHAT I CHANGED ON THE HOST: the node's app0 (flash, declared), and resets
     of the NODE via esptool (the flash itself and the read_mac/read_flash
     commands, all `--after hard_reset`). The extra backend rows at
     ~17:23:38, 17:24:16, 17:25:16, 17:28:21 are the immediate boot-time
     check-ins those resets caused -- declared so they are not read as a broken
     cadence. Nothing else: no VPS change, no broker/ACL change, no NVS write,
     no credential read or written, no base-board access.

  WHY STAGED. All three criteria are met with quoted evidence; the proof block
  carries the branch tip, the green run whose headSha equals that tip, the
  artifact, and the ntfy receipt. The one thing that is NOT demonstrated is
  non-zero particle values, which is criterion-as-written blocked by 0115's
  pre-existing finding rather than by this task's deliverable -- declared in
  item 4 rather than papered over.
---

# 0117 — PMS 1-second live poll (node only, bench diagnostic)

## Context

0115 (archived) proved the PMS5003 streams valid frames with all-zero fields
(rx_avail_before=256 saturated, ok=1, pm1/pm25/pm10 = 0/0.0/0) and added a
per-checkin diagnostic line. Stephen wants live numbers on the serial — the
60 s checkin cadence can't show the sensor reacting to a stimulus (e.g. a
breath puff at the inlet). This task adds a 1-second live poll. Base branch:
`hermes/0115-pms-uart-diag` (keeps the per-checkin line); new branch
`hermes/0117-pms-live-poll`.

## Task

In the node `loop()` (not in `node_checkin()`), poll the PMS5003 once per
second and print one line per successfully parsed frame:

`pms-live: pm1=<u> pm25=<f> pm10=<u>`

## Success criteria

1. Branch `hermes/0117-pms-live-poll` builds green in CI from the 0115 tree.
2. The NODE (MAC `B0:A6:04:C5:75:4C`, by-id path) — and only the node — is
   flashed; bench discipline holds (partition-table read first, sha256 on the
   read-back matches the built image).
3. Serial shows approximately one `pms-live:` line per second with live values,
   AND the 60 s checkin still fires on schedule (backend keeps receiving node
   "0" readings) — the live poll must not break or delay checkins.

## Constraints

- NODE ONLY. Do not touch the base.
- NON-BLOCKING: the live poll must not stall `loop()`. Cap the per-poll read
  window short (well under the 1 s period — e.g. assemble from already-buffered
  bytes, bail fast if no header appears). The existing 1500 ms blocking scan in
  `read_pms25()` is for the checkin path; do not call that blindly every
  second.
- Keep the 0115 per-checkin `pms:` line as is — do not remove it.
- Diagnostic only: no deep-sleep changes, no config changes, no merge, no tag.
- Keep model cost on DeepSeek Flash. State the tier in the reply.

## Proof

- Branch SHA on origin + CI run URL + artifact.
- Flash read-back sha256 matching the built image.
- Pasted serial excerpt showing at least five consecutive `pms-live:` lines
  (~1 s apart) plus one checkin `tx:` line, proving both paths coexist.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, first line of notes states the live poll is running at ~1 Hz
and checkins are unaffected, `proof` block with the pointers and pasted serial.
