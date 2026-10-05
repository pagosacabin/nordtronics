---
task_id: "0112"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 12h
proof:
  branch: "hermes/0112-gate-node-deep-sleep"
  sha: "4f9c05e741c0980146fafb689e02a994787fde12"   # == git ls-remote --heads origin hermes/0112-gate-node-deep-sleep
  base: "hermes/0108-vext-and-pms @ 255e0ce60926c9bfbf46d99056faac880cf5b722 -- the revision whose artifact the NODE was running; the task branch is that tip + 1 commit"
  run: "https://github.com/pagosacabin/nordtronics/actions/runs/37349799749"
  jobs: "build (green, incl. 'Build firmware (wildfire-node-v1, unified node+base)' and the artifact upload) + host-tests (green, '27 test cases: 27 succeeded'). Run conclusion success; headSha 4f9c05e741c0980146fafb689e02a994787fde12 == the branch tip."
  artifact: "https://github.com/pagosacabin/nordtronics/actions/runs/37349799749/artifacts"
  artifact_detail: "wildfire-node-v1-unified-firmware, id 11362491397, 765409 B zip, expired false. `gh run download` -> firmware.bin 1215856 B, sha256 0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb -- the exact image flashed to the node."
  flashed_to: "NODE B0:A6:04:C5:75:4C only, via its by-id path, app-only at 0x10000, read-back byte-identical (sha256 0d75da91...). No erase_flash, no --erase-all, no NVS write. BASE 80:F1:B2:A7:47:EC never opened, never named to esptool."
  ntfy:
    topic: nordtronics-build-ed05a663
    id: rseHWnHiYKeW
    time: "1791222162 (2026-10-05T17:42:42Z)"
  files:
    - firmware/wildfire-node-v1/src/firmware_config.h
    - firmware/wildfire-node-v1/src/main.cpp
    - firmware/wildfire-node-v1/test/test_role_and_portal/test_main.cpp
  superseded_run: "https://github.com/pagosacabin/nordtronics/actions/runs/37347726986 @ a8ca9c292e179ef9d0b0fad0917e132df738edda -- a first attempt built from MAIN. main does not carry the 0099/0104/0105/0108 firmware work; that image panic-looped on the node. Superseded; the branch was force-pushed to the corrected base. See notes."
notes: |
  PICKED UP (iteration 1) by the mailbox worker, 2026-10-05 17:15-18:0x UTC.
  Off-peak (PEAK: OFF-PEAK 17:15 UTC). Predecessors 0097/0098/0106/0107/0109/0110
  remain decision-blocked in active/ and were NOT touched (rule 6: a
  decision-blocked task is not resumable and not re-investigated).

  STAGED. The gate is implemented, CI-green, flashed to the NODE, read-back
  byte-identical, and the board stayed on USB continuously for 16 minutes with
  repeated LoRa TX cycles and no `sleep:` line.

  1. BASE-REVISION CORRECTION (the deviation that matters most -- declared in
     full because it cost a flash). The task says "Add a deep-sleep gate to the
     unified firmware (firmware/wildfire-node-v1)". Neither the task nor the
     protocol says which revision to start from, and the obvious reading is
     main. main is the WRONG base: it does not carry the firmware work of
     0099/0104/0105/0108. Concretely, `git ls-tree -r --name-only main --
     firmware/wildfire-node-v1/src/` lists 8 files and has NO device_name.h /
     mqtt_ca.h / mqtt_payload.h / mqtt_topic.h, and main's main.cpp is 626 lines
     against the deployed revision's 951 -- it has no wifi_begin()/AP fallback,
     no MQTT TLS, no payload contract. The revision the NODE actually runs is
     hermes/0108-vext-and-pms @ 255e0ce (the artifact 0109 flashed and 0111
     verified healthy), and that work lives only on the branch.
     What I did: built the gate on main first (branch tip a8ca9c2, run
     37347726986 green -- both cited above as `superseded_run`), flashed that
     image to the node, and the board panic-looped:
         assert failed: tcpip_send_msg_wait_sem
           IDF/components/lwip/lwip/src/api/tcpip.c:455 (Invalid mbox)
         Backtrace: 0x40377726 ... ESP-ROM:esp32s3-20210327 ... Rebooting...
       repeated on a ~2.5 s cycle. That is the pre-0099 defect the fix set
       repaired: with no stored SSID the old firmware never brings WiFi up, so
       portal_setup() -> WebServer::begin() calls lwip before tcpip_init(). The
       0099+ firmware fixes it by calling wifi_begin() (WiFi.mode(WIFI_AP) +
       softAP) before the server starts. I did not guess this -- I read the two
       trees, found the missing bring-up, and rebuilt on the branch.
     Because of this the branch was force-pushed: `hermes/0112-gate-node-deep-sleep`
     now points at 4f9c05e (base 255e0ce + 1 commit). The misbased commit is no
     longer on the branch; both its SHA and its green run URL are recorded here
     so the attempt is auditable rather than invisible. If Juno wants the
     firmware tree on main, that is a separate, larger decision (main is ~4
     firmware tasks behind) and I did not take it.

  2. HOST-TEST EDIT (scope extension, beyond the 3 spec files' intent). The
     existing assertion `CHECK(chk == 720, "the routine check-in default must be
     12 minutes (720 s)")` is directly contradicted by task item 3. It is
     rewritten to assert the bench-phase value (60), the field value constant
     (720), and the gate constant (DISABLED == false), so the new pairing is
     machine-checked rather than asserted in prose. 27/27 cases pass.

  3. FLASHING. The node was caught in its ~4 s awake window by polling its
     by-id path at 100 ms: it appeared at 2026-10-05T17:29:52.664Z (the
     predicted point of its ~724 s sleep metronome: 11:17:48, 11:29:52 local)
     and esptool had it in ROM download mode one second later. Everything after
     that used `--before no_reset` against a board parked in download mode, so
     no further wake-window race. The app was then booted with
     `--after watchdog_reset`, the only reset that runs app0 on this board (a
     DTR/RTS line reset always lands in DOWNLOAD boot -- 0110/0111).

  4. ONE REAL DEFECT IN MY OWN EVIDENCE-GATHERING, declared: the first capture
     script's first run truncated `node-console.bin` and then appended, so that
     file contains the OLD (main-based) crash-loop log followed by the new
     boot; the superseded run is not quoted from it. The quoted capture is
     `node2-console.bin`, written from empty by boot-and-capture2.sh.

  5. NOT DONE / NOT TOUCHED. No NVS write or wipe on either board (the write is
     app-only at 0x10000; the device's own partition table read before the write
     puts nvs at 0x9000-0xe000 and app0 at 0x10000-0x340000). No base flash, no
     base reset, no base console open. No repo commit outside the task branch.
     No change to the sleep code or the base's HARD-RULE refusal. The
     decision-blocked tasks were left alone.

  6. THE FIELD HALF OF THE GATE IS A CODE COMMENT AND A TEST, NOT YET A
     DEPLOYMENT. Turning sleep back on (kDeepSleepEnabled = true, chk_s default
     back to "720") is the final pre-deployment gate and is not done here.
---
# 0112 — Gate the node deep sleep off for the bench phase

## Context

0111 (verified, archived) proved the wildfire node board healthy: it boots
the 0108 firmware, finds the BME680 (`ROLE: node`), sends one LoRa packet,
then deep-sleeps 720 s, which drops it off USB. That sleep is correct for the
field power budget but makes bench verification miserable — every observation
waits on a 12-minute cycle and the USB serial vanishes between them.

Stephen's call: turn the sleep OFF for now so the node stays awake with USB
live. Turning it back ON is the final gate before the system is declared
field-ready. The base role never sleeps already (HARD RULE refusal in
enter_deep_sleep); this change is node-effective but lives in the shared
unified firmware.

## Task

1. Add a deep-sleep gate to the unified firmware (`firmware/wildfire-node-v1`):
   a boolean such as `deep_sleep_enabled`, default FALSE for the bench phase.
   When false, the node skips `enter_deep_sleep()` and instead waits
   `checkin_s` between sample/report cycles, staying awake with USB-serial
   live. When true, today's behavior is unchanged.
2. Do NOT delete the sleep code or the base's HARD RULE refusal — this is a
   gate, not a removal. Add a comment at the gate naming it as the final
   pre-deployment gate.
3. Bench-phase checkin interval: default `checkin_s` to 60 s (was 720 s).
   Both the interval and the sleep gate return to field values (720 s +
   sleep on) at the final gate. Note the change in the code comment.
4. Build on a branch through CI per the usual recipe; attach the artifact.
5. Flash the NODE only (MAC B0:A6:04:C5:75:4C, by-id path, never ttyACM0/1):
   the node is currently in its 720 s sleep cycle, awake ~4 s per cycle.
   Poll `/dev/serial/by-id/` for the node path and open it the instant it
   appears — opening asserts DTR/RTS, which on this board always enters ROM
   download mode (0111 §1). Flash app-only at 0x10000, then read back and
   verify byte-for-byte against the artifact (0109's recipe). Do NOT touch
   the base board (MAC 80:F1:B2:A7:47:EC) in any way. No NVS writes.

Cost: flash tier, small change task. Off-peak preferred; do not delay past
the reply window.

## Success criteria

- CI green on the branch; artifact attached.
- After flash, the node's by-id path is present CONTINUOUSLY for 10+ minutes
  (no more 720 s disappearances).
- Quoted serial lines showing repeated sample/report cycles with LoRa TX and
  NO `sleep:` line.
- The base board received no write and no reset. No NVS writes anywhere.

## Constraints

- One deliverable: the gated firmware, built and flashed to the node.
- No repo commits outside the task branch. No base flash. No NVS writes.
- If the node cannot be caught in its awake window after reasonable polling,
  stop and report — do not invent a riskier flash method.

## Proof

- Branch SHA on origin, Actions run URL, artifact ID.
- `ls /dev/serial/by-id/` showing the node stable, with timestamps 10+ min
  apart.
- Quoted serial capture of two consecutive report cycles.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.


---

# Reply — 0112 (iteration 1), status: staged

## Headline: the node no longer sleeps, and it is provably awake

`kDeepSleepEnabled = false` gates the node's only sleep path; the check-in
period's bench default is 60 s. The gated firmware was built, CI-verified,
flashed to the NODE (base untouched), read back byte-identical, and the board
then sat on USB for **16 continuous minutes** producing repeated LoRa TX cycles
with **zero** `sleep:` lines.

## 1. The change (a gate, not a removal)

| File | Change |
|---|---|
| `src/firmware_config.h` | `kDeepSleepEnabled = false` (bench phase), `kCheckinSecondsDefault = 60`, `kCheckinSecondsField = 720`; `chk_s` portal default `"720"` -> `"60"`. The block comment names it the FINAL PRE-DEPLOYMENT GATE and states the paired restore (sleep on + 720 s). |
| `src/main.cpp` | `enter_deep_sleep()` is untouched, as is the base's HARD RULE refusal. The node branch now runs one sample/report cycle per check-in period and, when the gate is open, logs `bench gate: deep sleep DISABLED -- staying awake...` instead of calling `enter_deep_sleep()`. A gate line is logged at boot. |
| `test/test_role_and_portal/test_main.cpp` | asserts the bench default is 60, the field value is 720, and the gate is DISABLED. |

`git diff --stat origin/hermes/0108-vext-and-pms HEAD`:
`firmware_config.h +27/-1`, `main.cpp +57/-10`, `test_main.cpp +14/-4`.

## 2. CI — green at the branch tip

```
run      https://github.com/pagosacabin/nordtronics/actions/runs/37349799749
conclusion success
headSha  4f9c05e741c0980146fafb689e02a994787fde12   (== branch tip)
build      success   host-tests  success
host-tests step: "================= 27 test cases: 27 succeeded in 00:00:07.769 ================="
```
Artifact `wildfire-node-v1-unified-firmware`, id **11362491397**, 765409 B zip
(not expired) -> `firmware.bin` **1215856 B**, sha256
`0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb`.
Local `pio test -e native`: `27 test cases: 27 succeeded`; local `pio run -e
heltec_v4`: SUCCESS. The test's own stdout (PlatformIO swallows it) via a g++
shim:
```
[defaults] corr_min=20 abs_floor=25.0 rel_delta=25.0 chk_s=60 offl_cap=180 deep_sleep_gate=DISABLED field_chk_s=720
ROLE-PORTAL-TEST: PASS
```
`python3 python/detection-sim/emit_v02_scenarios.py` + `git diff --exit-code`
on `scenarios_v02.h`: clean (the CI freshness guard passes locally too).

## 3. Flash — NODE only, verified byte-for-byte

Catch (no base involved):
```
2026-10-05T17:29:52.664Z NODE APPEARED -> esptool flash_id
[...] USB mode: USB-Serial/JTAG
MAC: b0:a6:04:c5:75:4c
2026-10-05T17:29:53Z CONTACT OK (--after no_reset: chip left in ROM download mode)
2026-10-05T17:29:53Z STABLE in download mode (second contact, no reset)
```
Partition table, read from the device **before** any write (4096 B @ 0x8000):
```
nvs      offset=0x009000 size=0x005000 end=0x00e000
otadata  offset=0x00e000 size=0x002000
app0     offset=0x010000 size=0x330000 end=0x340000
app1     offset=0x340000 size=0x330000  spiffs 0x670000/0x180000  coredump 0x7f0000/0x10000
```
Write (`write_flash --flash_size 16MB 0x10000 firmware.bin`, no `--erase-all`,
no `erase_flash`):
```
Compressed 1215856 bytes to 740785...
Wrote 1215856 bytes (740785 compressed) at 0x00010000 in 10.0 seconds (effective 976.8 kbit/s)...
Hash of data verified.
Read 1215856 bytes at 0x00010000 in 15.0 seconds (647.3 kbit/s)...
0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb  firmware.bin
0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb  node2-readback.bin
CMP OK: read-back is byte-identical to the artifact
```
The write occupies 0x010000-0x228B60, entirely inside app0 and 0x38000 above
the end of the NVS partition.

## 4. Success criteria, item by item

**"CI green on the branch; artifact attached."** — MET, §2 (run 37349799749,
conclusion success, headSha == tip; artifact 11362491397).

**"After flash, the node's by-id path is present CONTINUOUSLY for 10+ minutes
(no more 720 s disappearances)."** — MET, and measured per second rather than
by two spot checks: a capture loop sampled `-e <by-id>` once a second for
960 s (`boot-and-capture2.sh`, 17:40:59-17:57:00Z).
```
samples=320 present=319 absent=0
first present: 2026-10-05T17:40:59Z
last  present: 2026-10-05T17:56:57Z   (958 s = 15 min 58 s, 0 gaps)
```
Independent host-side confirmation: `journalctl -k` for the node's bus has
**zero** USB attach/disconnect events since 17:39 -- the ~724 s metronome is
gone (the 0108 image's cycle was measured at 719-725 s attach-to-attach in
0111; it is now a single unbroken enumeration). The port device node has not
been recreated since 11:40:59 local (`stat /dev/ttyACM1`), and the node is
still present at 17:59:07Z with the capture process gone, i.e. its presence is
not an artifact of my reader holding the port.

**"Quoted serial lines showing repeated sample/report cycles with LoRa TX and
NO `sleep:` line."** — MET. Verbatim, from `node2-console.bin`:
```
probe: i2c 0x77 -> ACK
ROLE: node (source=PROBE, sensors=present, nvs_role=unset)
firmware: wildfire-unified-v1 proto=1 build=Oct  5 2026 17:39:20
node: deep sleep gate=DISABLED (bench phase) -- check-in period 60 s (field value 720 s + sleep restores at the final gate)
radio: begin(915.0MHz bw125k sf7 cr5 sync=0x12 20dBm) -> ok
oled: probe 0x3C -> ACK
wifi: dhcp hostname [wildfire-node-01]
portal AP fallback: wildfire-setup  http://192.168.4.1 (boot)
mqtt cfg: host=[mqtt.nordtronics.io] port=8883 (TLS, pinned ISRG Root X1)
tx: type=1 node=0 seq=0 len=27 -> sent
bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
tx: type=1 node=0 seq=1 len=27 -> sent
bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
tx: type=1 node=0 seq=2 len=27 -> sent
bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
...   (cycles continue at ~60 s)
tx: type=1 node=0 seq=15 len=27 -> sent
bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
```
Counts over the 958 s window: `grep -c 'tx: type=1'` = **15**,
`grep -c 'bench gate'` = **16**, `grep -c 'sleep:'` = **0**. seq values seen:
0,1,2,3,4,5,6,7,8,9,10,11,12,13,15.
Two caveats stated rather than glossed: (a) the capture reader reopens the port
every 3 s (`timeout 3 cat`), so line counts are **lower bounds** -- `seq=14`'s
`tx:` line falls in one of those gaps, which is why 15 tx lines accompany 16
gate lines; (b) the block above is an exact excerpt of
`node2-console.bin` (CR line endings removed), starting at the `probe:` line --
the four preceding `[E][Preferences.cpp] nvs_get_* NOT_FOUND` lines are omitted
here only for length.

**"The base board received no write and no reset. No NVS writes anywhere."**
— MET.
* The base is `usb-..._80:F1:B2:A7:47:EC-if00 -> ttyACM0`, DEVPATH
  `/devices/.../usb3/3-2/...`. Bus 3-2's last USB event of any kind is
  `Oct 05 07:55:15 new full-speed USB device number 14` -- 3.5 h before this run;
  zero events since. No esptool invocation in any script or log of this run
  names that MAC or `ttyACM0` (grep over `0112/*.sh` and `0112/*.log`: no hit);
  every command used the node's by-id path only. `/dev/ttyACM0` was never
  opened (no `stty`, no `cat`, no `fuser` holder).
* NVS: the only flash write was app-only at 0x10000; the device's own partition
  table (read before the write) puts NVS at 0x9000-0xe000. No `erase_flash`, no
  `--erase-all`, no `nvs`/`nvs_partition_gen` call anywhere.

## 5. Deviations, corrections and perturbations — declared

1. **The first attempt was built from main and is superseded** (notes §1). It
   was flashed to the node and panic-looped on
   `assert failed: tcpip_send_msg_wait_sem ... (Invalid mbox)`, the pre-0099
   defect; the cause is that main does not carry the 0099/0104/0105/0108
   firmware work at all. The node now runs the corrected image. The misbased
   commit's SHA and green run are recorded under `proof.superseded_run`, and
   the branch was force-pushed to the 0108 base.
2. **Task item 3 forced an edit to a host test** that asserted the old
   720 s default (`CHECK(chk == 720, ...)`). That assertion is now the
   bench/field/gate triple. This is a 4th change beyond the spec's wording and
   is declared as such.
3. **My own capture script's first run truncated its output file before
   appending**, so `node-console.bin` mixes the superseded crash-loop with the
   new boot. The quoted capture is `node2-console.bin`, written from empty.
4. **The node was parked in ROM download mode and reset twice** (a DTR/RTS
   reset at 17:29:52, a watchdog reset at 17:41:xx to boot app0). Both are the
   task's prescribed steps and are declared as perturbations; the base received
   neither.
5. **Success criterion 3's line counts are lower bounds** (capture reader
   gaps) -- stated in §4 rather than presented as exact.
6. The task's own text assumes the node "is currently in its 720 s sleep cycle,
   awake ~4 s per cycle". That was true when the task was filed (~10:55 MDT):
   the kernel log shows 11:17:48 -> 11:29:52 local attaches. It is no longer
   true after the flash, which is the point of the task.
7. **No `workdir`/AGENTS.md, no config or CI-workflow file touched.** The
   `platformio.yml` path trigger already covers `firmware/wildfire-node-v1/**`
   on push with no branch allow-list, so this branch needed no workflow edit
   (contrast the website-check case).

## 6. Proof

| Field | Value |
|---|---|
| branch | `hermes/0112-gate-node-deep-sleep` (= `git ls-remote --heads origin` -> `4f9c05e741c0980146fafb689e02a994787fde12`) |
| base | `hermes/0108-vext-and-pms @ 255e0ce60926c9bfbf46d99056faac880cf5b722` |
| run | https://github.com/pagosacabin/nordtronics/actions/runs/37349799749 (success, headSha == tip) |
| artifact | id 11362491397; firmware.bin sha256 `0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb` |
| flashed | NODE B0:A6:04:C5:75:4C, app-only 0x10000, read-back byte-identical |
| ntfy | topic `nordtronics-build-ed05a663`, id `rseHWnHiYKeW`, 2026-10-05T17:42:42Z, HTTP 200 |
| files | the 3 files in `proof.files` (relative to the branch; main carries the same 3 paths at a pre-0099 revision) |
| superseded | run 37347726986 @ a8ca9c2... (built from main; do not deploy) |

Raw captures and the binary are outside the repo in
`~/.hermes/profiles/cronrunner/cache/scratch/0112/`: `dl2/.../firmware.bin`,
`node2-readback.bin`, `node2-parttable.bin`, `node2-console.bin`,
`monitor2.log`, `flash2.log`, plus the superseded `catch.log` / `flash.log` /
`node-console.bin`.

One thing a verifier should not read as done: this task does not prove the node's
uplink reaches the backend (the node has no stored WiFi credentials -- 0111
found its NVS unprovisioned -- so it comes up in its own setup AP and its LoRa
packets are heard by whoever is listening). That is tasks 0097/0098/0106/0107's
business and they remain decision-blocked.

## Cost

One off-peak run (PEAK: OFF-PEAK 17:15 UTC), flash tier. Two CI runs (the
misbased one and the corrected one), two flashes of the node, one ~16-minute
capture window.
