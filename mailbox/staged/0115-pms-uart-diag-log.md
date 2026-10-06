---
task_id: "0115"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: hermes/0115-pms-uart-diag
  sha: 8435fb3814a8fe81ffe54856ef121fa705824468
  run: https://github.com/pagosacabin/nordtronics/actions/runs/37486051227
  files:
    - firmware/wildfire-node-v1/src/main.cpp
  artifact: wildfire-node-v1-unified-firmware (id 11422932942, 765510 B zipped, expired=false)
  base_revision: hermes/0112-gate-node-deep-sleep @ 4f9c05e741c0980146fafb689e02a994787fde12
  flash:
    board: node B0:A6:04:C5:75:4C (by-id usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00 -> ttyACM0)
    image_sha256: 9d5a075cfc55d6f1513b2f2a23e59c8ba21b36e2bd62e2d5d33d7a085ba59d4d
    readback_sha256: 9d5a075cfc55d6f1513b2f2a23e59c8ba21b36e2bd62e2d5d33d7a085ba59d4d
    image_bytes: 1215968
    write: "esptool 4.9.1 --chip esp32s3 --baud 921600 --after hard_reset write_flash --flash_size 16MB 0x10000 firmware.bin (no --erase-all, no erase_flash)"
  ntfy: none -- see notes (no ntfy step exists in the workflow that ran)
notes: |
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-06 15:15 UTC.
  Off-peak (PEAK: OFF-PEAK 15:15 UTC). Model tier: DeepSeek Flash. The filed
  front-matter carried no `iteration` field; it was added at pickup per the
  protocol. Predecessors 0097/0098/0106/0107/0109/0110 remain
  decision-blocked/parked in active/ and were not touched.

  VERDICT (first line): BYTES ARE ARRIVING AT UART1 -- rx_avail_before was
  consistently > 0 (256, 225, 193, 161 in window 1; 245, 213 in window 2) across
  six check-ins. The filing's premise ("zero bytes are arriving at UART1, not a
  parser rejection") is FALSIFIED. Not only do bytes sit in the UART1 RX ring
  buffer, every check-in assembled a complete 32-byte frame with a MATCHING
  checksum (ok=1), so the parser is not rejecting frames either; the frame's
  particle fields are the thing that is zero (pm1=0, pm25=0.0, pm10=0).
  Per the task's own criterion this places the suspect side on the FIRMWARE read
  path, not the wire/sensor side -- but see the observation below, which is
  stated as an observation and NOT acted on (the task forbids a fix).

  OBSERVATION, stated as an observation and not a diagnosis. The rx_avail_before
  series decreases by ~32 bytes (exactly one PMS frame) per check-in -- 256 ->
  225 -> 193 -> 161 (-31/-32/-32) -- with only erratic refill (161 -> 245 across
  the ~65 s gap between the two capture windows). A PMS5003 streaming at its
  nominal active-mode ~1 Hz would deliver ~32 B/s, which fills the 256-byte
  Arduino RX ring buffer in ~8 s and would therefore read exactly 256 at EVERY
  check-in; it does not. So bytes are present and parseable, but the supply is
  far below the nominal frame rate and bursty. No fix was applied, no config was
  changed, and this is not offered as a mechanism -- it is a second datum for
  the follow-up task the filing says will handle the fix.

  WHAT WAS BUILT (criterion 1 MET). Branch hermes/0115-pms-uart-diag from
  hermes/0112-gate-node-deep-sleep @ 4f9c05e. Exactly ONE functional change,
  the diagnostic line the spec asks for verbatim, in node_checkin() in
  firmware/wildfire-node-v1/src/main.cpp (+4/-1):
      const int pms_avail_before = g_pms.available();
      const float pm25 = read_pms25(&pm1, &pm10, &pms_ok);
      logf("pms: rx_avail_before=%d ok=%d pm1=%u pm25=%.1f pm10=%u",
           pms_avail_before, pms_ok ? 1 : 0, pm1, pm25, pm10);
  No other behavior, config or deep-sleep change. Not merged, not tagged.
  CI: workflow "PlatformIO Build" run 37486051227, conclusion success,
  headSha 8435fb3814a8fe81ffe54856ef121fa705824468 == the branch tip
  (`git ls-remote --heads origin hermes/0115-pms-uart-diag` returns the same
  sha, checked after the run). Run window 2026-10-06T15:16:20Z-15:18:33Z.
  The workflow fires on push with `paths: firmware/wildfire-node-v1/**` and NO
  branches filter, so it starts on a fresh hermes/NNNN-* branch (the
  website-check-style branch allow-list trap does not apply here). Artifact
  wildfire-node-v1-unified-firmware id 11422932942 downloaded fresh from that
  run -> firmware.bin 1215968 B, sha256
  9d5a075cfc55d6f1513b2f2a23e59c8ba21b36e2bd62e2d5d33d7a085ba59d4d.

  WHAT WAS FLASHED (criterion 2 MET). NODE ONLY. Identity confirmed with esptool
  BEFORE any write, on the by-id path the task mandates: read_mac ->
  "MAC: b0:a6:04:c5:75:4c" (the NODE). Base 80:F1:B2:A7:47:EC was never opened
  and no esptool command was ever pointed at it (kernel corroboration: usb port
  3-2 has had ZERO events today -- its last event is Oct 05 16:17:50 -- i.e.
  the base has not been reset or re-enumerated by this run).
  Partition table read from the device first (4096 B @ 0x8000, parsed):
    nvs       off=0x009000 size=0x005000 end=0x00e000
    otadata   off=0x00e000 size=0x002000 end=0x010000
    app0      off=0x010000 size=0x330000 end=0x340000
    app1      off=0x340000 size=0x330000 end=0x670000
    spiffs    off=0x670000 size=0x180000 end=0x7f0000
    coredump  off=0x7f0000 size=0x010000 end=0x800000
  The app-only write at 0x10000 occupies 0x010000-0x228D60 (1215968 B), inside
  app0 and far above NVS at 0x9000-0xE000, so NVS was not reachable by the
  write. No --erase-all, no erase_flash, no NVS write anywhere.
  Read-back of the same length: sha256
  9d5a075cfc55d6f1513b2f2a23e59c8ba21b36e2bd62e2d5d33d7a085ba59d4d == the
  built image, `cmp` exit 0 (identical). The board stayed enumerated through the
  flash and read-back (no USB drop this time).

  WHAT WAS CAPTURED (criterion 3 MET). Two non-perturbing windows on the node's
  console, recipe `stty -F /dev/ttyACM0 115200 raw -echo -hupcl` then
  `timeout <N> cat /dev/ttyACM0`: window 1 = 250 s (ended by the timeout, exit
  124, as intended), window 2 = 135 s, back to back. Pasted verbatim (CR
  inspected with cat -A; bytes unedited):
    pms: rx_avail_before=256 ok=1 pm1=0 pm25=0.0 pm10=0
    tx: type=1 node=0 seq=1 len=27 -> sent
    bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
    pms: rx_avail_before=225 ok=1 pm1=0 pm25=0.0 pm10=0
    tx: type=1 node=0 seq=2 len=27 -> sent
    bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
    pms: rx_avail_before=193 ok=1 pm1=0 pm25=0.0 pm10=0
    tx: type=1 node=0 seq=3 len=27 -> sent
    bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
    pms: rx_avail_before=161 ok=1 pm1=0 pm25=0.0 pm10=0
    tx: type=1 node=0 seq=4 len=27 -> sent
    bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
    pms: rx_avail_before=245 ok=1 pm1=0 pm25=0.0 pm10=0
    tx: type=1 node=0 seq=5 len=27 -> sent
    bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
    pms: rx_avail_before=213 ok=1 pm1=0 pm25=0.0 pm10=0
    tx: type=1 node=0 seq=6 len=27 -> sent
    bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
  That is six `pms:` lines, three times the two required.

  PERTURBATION DECLARED. Opening /dev/ttyACM0 can assert DTR/RTS on the
  USB-JTAG peripheral and reset the board, so it is declared rather than
  assumed harmless: it did NOT reset it here. Evidence, not assertion: the
  board's own `tx ... seq=` counter runs 1,2,3,4 -> 5,6 across the whole
  capture (monotonic, never restarting), no `ESP-ROM` banner appears anywhere
  in the log, and the 60 s check-in cadence is unbroken across the window
  boundary. The recipe opens the USB CDC console, which cannot inject bytes into
  UART1 -- the instrument is the firmware's own `g_pms.available()` on
  HardwareSerial(1) (main.cpp:70), called from node_checkin().

  NTFY RECEIPT: NONE, and this is earned rather than omitted (handoff-mailbox
  rule 18). Enumeration: `ls .github/workflows/` = android-build.yml,
  android-companion-v0.yml, backend.yml, detection-sim.yml, platformio.yml,
  website-check.yml. The run that fired is platformio.yml ("PlatformIO Build"),
  and it contains no ntfy publish step at all (its steps: checkout, set up
  python, cache PlatformIO, install PlatformIO Core, pio run/build, upload
  artifact, pio test -e native, scenario-header check). The ntfy topic named in
  the protocol (`nordtronics-build-ed05a663`) is the companion-APK build topic;
  no firmware workflow publishes to ntfy.

  BRANCH VS MAIN. This branch is based on hermes/0112-gate-node-deep-sleep
  (4f9c05e), not on main, exactly as the task specifies -- a verifier diffing
  the branch against main will see the 0112 bench-gate delta as well as the
  one-line 0115 delta. The 0115 delta is the only change this task made.

  DEVIATIONS: none. No fix was applied on the verdict, nothing was merged or
  tagged, the base board was not touched, no NVS write occurred, and the
  mailbox pickup commit (7bffec9) plus this staging commit are the only main
  commits. Cost: one off-peak run, DeepSeek Flash tier.
---

# 0115 — PMS UART diagnostic log build (node only)

## Context

Bench 2026-10-06: the PMS5003 is powered from the bench supply (fan spinning,
30–40 mA, same draw as the last working session), the node (0112 image) checks
in every 60 s with the BME680 healthy, but pm25 is pegged 0.0 and the node
serial shows no `pms: checksum mismatch` lines — i.e. zero bytes are arriving
at UART1, not a parser rejection. No meter is available, so this rung is
software-only: add temporary observability to the PMS read path. (Side note
already established: `probe: pms5003` never prints on a node — the BME680 ACK
short-circuits `probe_node_sensors()` — so do not look for it; the checkin path
is the instrument.)

## Task

On a new branch from `hermes/0112-gate-node-deep-sleep` (name it
`hermes/0115-pms-uart-diag`), make exactly ONE functional change in
`firmware/wildfire-node-v1/src/main.cpp`, in `node_checkin()`: capture
`g_pms.available()` BEFORE the `read_pms25()` call, then emit one log line per
checkin:

```c
const int pms_avail_before = g_pms.available();
const float pm25 = read_pms25(&pm1, &pm10, &pms_ok);
logf("pms: rx_avail_before=%d ok=%d pm1=%u pm25=%.1f pm10=%u",
     pms_avail_before, pms_ok ? 1 : 0, pm1, pm25, pm10);
```

(Build it, CI green, then flash the NODE ONLY and capture serial for at least
two checkins.)

## Success criteria

1. Branch `hermes/0115-pms-uart-diag` builds green in CI from the 0112 tree with
   only the diagnostic line added.
2. The NODE (MAC `B0:A6:04:C5:75:4C`, by-id path
   `usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`) — and only
   the node — is flashed; bench discipline holds (partition-table read first to
   prove NVS safety, sha256 on the read-back matches the built image).
3. Serial capture shows at least two `pms:` lines, and the reply states the
   verdict plainly: `rx_avail_before` consistently 0 means no bytes reach the
   pin (wire/sensor side); consistently >0 means bytes arrive and the read path
   is dropping them (firmware side).

## Constraints

- NODE ONLY. Do not touch the base (MAC `80:F1:B2:A7:47:EC`).
- Diagnostic only: no other behavior changes, no config changes, no deep-sleep
  changes. This is not a release — do not merge, do not tag.
- Keep model cost on DeepSeek Flash (the tier already in use for this line of
  work). State the tier in the reply.
- Do not "fix" anything based on the verdict. Report the verdict and stop —
  the fix is a separate task.

## Proof

- Branch SHA on origin + CI run URL + artifact.
- Flash read-back sha256 matching the built image.
- Pasted serial lines showing at least two `pms:` lines (redact nothing needed;
  there are no secrets in these lines).

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, the verdict (bytes arriving: yes/no) in the first line of the
notes, and a `proof` block with the pointers and pasted serial lines above.
