---
task_id: "0108"
protocol_version: 1.0.0
status: verified
iteration: 2
expect-reply-within: 72h
proof:
  branch: "hermes/0108-vext-and-pms"
  sha: "255e0ce60926c9bfbf46d99056faac880cf5b722"   # == git ls-remote --heads origin hermes/0108-vext-and-pms
  run: "https://github.com/pagosacabin/nordtronics/actions/runs/37201631922"
  jobs: "build (green, incl. 'Build firmware (wildfire-node-v1, unified node+base)' and the artifact upload); host-tests (green, incl. 'Run host-side firmware tests (native env)' and the scenarios_v02.h freshness guard). Run conclusion: success; headSha 255e0ce60926c9bfbf46d99056faac880cf5b722 == the branch tip."
  artifact: "https://github.com/pagosacabin/nordtronics/actions/runs/37201631922/artifacts"
  artifact_detail: "wildfire-node-v1-unified-firmware, id 11302294566, 769803 B zip, zip sha256 627f871da94a02d7e6611dd85932cc6906edabdf9b71f4fd2eb6606b63df7432 (from the run log); re-downloaded with `gh run download`: firmware.bin 1222688 B, sha256 36dd5ddf9bd3cecef667e67cee93f544a746cc89bfa3925d18bbb887cc440a95 (a fresh build of the same source is never byte-identical -- firmware.bin embeds __DATE__/__TIME__)"
  host_tests: "pio test -d firmware/wildfire-node-v1 -e native -> '27 test cases: 27 succeeded in 00:00:08.666' (0 failed). 26 before this branch; test_vext_gate_pin is the new case, and it is listed as [PASSED] at test/test_role_and_portal/test_main.cpp:230 in the CI log."
  ntfy:
    topic: nordtronics-build-ed05a663
    id: RjrWwBnF16HE
    time: "1791116476 (2026-10-04T12:21:16Z)"
  files:
    - firmware/wildfire-node-v1/src/main.cpp
    - firmware/wildfire-node-v1/src/firmware_config.h
    - firmware/wildfire-node-v1/test/test_role_and_portal/test_main.cpp
notes: |
  STAGED (iteration 2, 2026-10-04 12:15-12:30 UTC, off-peak). The firmware half
  is DONE and CI-green; the PMS5003 half is reported as the verification it is,
  with the bench-side frame confirmation NOT performed and why (below). No
  hardware was written, flashed, reset or provisioned; no credential was read,
  written or logged.

  WHAT CHANGED (one deliverable: sensor power):
    src/firmware_config.h  + `constexpr int kOledVextPin = 36;` in the fixed-wiring
                             block, with the active-LOW / frozen-net rationale.
    src/main.cpp           + STEP -1 at the top of setup(): `pinMode(wf::kOledVextPin,
                             OUTPUT); digitalWrite(wf::kOledVextPin, LOW); delay(10);`
                             in BOTH roles (the gate is before the role is known
                             on purpose: the probe cannot read the bus it powers),
                             immediately before the STEP 0 role-detection block.
    test/test_role_and_portal/test_main.cpp  + test_vext_gate_pin (new case).

  FALSIFIABLE CHECK 1 -- the grep, quoted with order (criterion: `grep -n "36"`
  shows the Vext drive before the first I2C/Sensor/OLED init):

    $ grep -n "36" firmware/wildfire-node-v1/src/main.cpp
    805:  // GPIO36 gates Vext, the switched 3.3 V rail the BME680/OLED sit on, active

    and the drive statements that follow it, in file order (read from the branch
    tip with `git show 255e0ce:firmware/wildfire-node-v1/src/main.cpp`):

      805  // GPIO36 gates Vext, ... (comment)
      812  pinMode(wf::kOledVextPin, OUTPUT);
      813  digitalWrite(wf::kOledVextPin, LOW);
      814  delay(10);  // let the rail settle before the probe's first I2C transaction
      828  const bool sensors_present = probe_node_sensors();     // <- first I2C/Sensor init
      860  g_oled_ready = oled_probe_and_begin();                 // <- first OLED init
      863  g_pms.begin(...);                                     // <- node-only sensor UART

    So the drive is at 812-814, ahead of both init sites. Note the literal-form
    caveat (handoff-mailbox rule 24 family): the GPIO *number* is not in main.cpp
    -- the call sites use the named constant, and "36" appears in main.cpp only in
    the explanatory comment. The number itself is in the header, quoted to satisfy
    the criterion as written:

      $ grep -n "36" firmware/wildfire-node-v1/src/firmware_config.h
      32:    "GPIO36", "GPIO17", "GPIO18", "GPIO4", "GPIO5", "GPIO6",
      38:inline constexpr int kFrozenGpioPins[] = {36, 17, 18, 4, 5, 6, 33, 47, 48, 34};
      69:// family. Heltec LoRa 32 V4 gates it on GPIO36, ACTIVE LOW (U1 pin 5 -- already
      74:// legacy bench firmware did the same: node.cpp PIN_VEXT 36, OUTPUT, LOW).
      75:constexpr int kOledVextPin = 36;

    GPIO36 is U1 pin 5, already on the 0079 frozen net list, so driving it adds no
    new net to the freeze -- that is now machine-checked rather than asserted (see
    scope extension 1).

  FALSIFIABLE CHECK 2 -- `git diff` contains no credential, SSID or password:
    $ git diff 951cbd3..255e0ce | grep -niE "^\+.*(ssid|password|passwd|secret|token|psk|api[_-]?key)"
    (no output; exit 1)
    The diff is 3 files, +48/-1, all of it source, comments and one test.

  FALSIFIABLE CHECK 3 -- item 3 preserved (the probe's OR logic and the panel
    exclusion are untouched): `git show --stat 255e0ce` lists exactly the three
    files above; probe_node_sensors() still returns true on a BME680/688 ACK
    (line 334) or on a valid PMS5003 frame (line 341), and the SSD1306 at 0x3C is
    still absent from the probe's address list. No LoRa receive path, consensus,
    MQTT or 0104/0105 code was touched.

  PMS5003 ITEM -- REPORTED, NOT CONFIRMED ON HARDWARE. The task splits it:
  "Stephen confirms its external power was OFF during the 'no frame' observation,
  so no firmware defect is suspected ... verify the probe sees valid 32-byte
  frames with the sensor powered". The verification half cannot be produced from
  this worker, for two independent reasons, and I did not manufacture it:
    (a) The board that has to run the probe (node, B0:A6:04:C5:75:4C, /dev/ttyACM1)
        still carries the 0105 image, which does not assert Vext, so it still
        resolves as base and never executes the node-role sensor path (0107's
        finding, re-observed read-only this run -- see the deviation below). No
        node-role `probe: pms5003 ->` line can exist on the bench until this
        branch's artifact is flashed, which is a hardware write this task forbids.
    (b) The PMS5003's 5 V is an external bench supply (PMS VCC->5V); the task
        constrains the run to "no hardware touched" and "do not rewire the bench",
        so the worker cannot establish "with the sensor powered" itself.
  What the verification will have to show, read from the source so the next run
  knows the exact strings to look for:

    - 0108 tip, src/main.cpp:338-343: the probe opens the PMS UART at
      `g_cfg.pms_baud` (portal default 9600, firmware_config.h:145), then calls
      read_pms25() with a 1500 ms window (line 290) and logs exactly one of
      `probe: pms5003 -> frame valid` / `probe: pms5003 -> no frame`.
    - read_pms25 (lines 285-316) requires the 0x42 0x4D header and an exact
      16-bit sum over frame[0..29] (line 307 logs a mismatch and drops the
      frame), so a "no frame" line with no checksum line above it means no bytes
      arrived, not a parse rejection -- that distinction is the whole diagnosis.
    - Ordering caveat for whoever captures it: the BME680/688 ACK check returns
      TRUE before the PMS read (line 334), so on a board with a working BME680
      the `pms5003` probe line is never printed at all. A capture that shows the
      I2C ACK and no PMS line confirms the OR logic, not a PMS fault.
    - 0107's "no frame" was observed on a board where the BME also failed and
      Vext was undriven; with Vext now driven the BME branch will normally
      short-circuit first, which is the intended behaviour (item 3).

  BENCH STATE, read-only (deviation declared): I opened both CDC ports with
  `stty -F /dev/ttyACM{0,1} 115200 raw -echo -hupcl` + `timeout 20 cat`, which is
  the non-perturbing recipe from 0107 (no DTR/RTS drive). 0 `ESP-ROM` banners in
  either window, i.e. neither board was reset. The node board (B0:A6:04:C5:75:4C,
  /dev/ttyACM1) is still running the 0105 image in the BASE role and is logging
  `[E][WiFiGeneric.cpp:1583] hostByName(): DNS Failed for mqtt.nordtronics.io`
  followed by `mqtt: connect to mqtt.nordtronics.io:8883 failed (state=-2)` every
  ~0.5 s; the base board (80:F1:B2:A7:47:EC, /dev/ttyACM0) printed 0 bytes in its
  20 s window. That is the live "node resolves as base" symptom this task fixes,
  plus an unreported bench-side DNS/WiFi condition on the node board that is
  outside this task's scope and was not investigated further.

  DEVIATIONS
    1. Scope extension, declared: test_vext_gate_pin is a new host test case in
       test/test_role_and_portal/test_main.cpp (the criterion is that the
       host-tests job passes, and the pin choice -- GPIO36 == a 0079 frozen net,
       no S3 special-function collision -- was worth machine-checking rather than
       asserting in prose; the file already carries the same check for the probe
       pins). It is the 27th case (26 before this branch).
    2. The header comment on the STEP 0 block in setup() was amended by one
       sentence, to record the Vext gate as the deliberate exception to
       "nothing power-related before the role is known". Behaviour unchanged.
    3. Read-only console capture described above (no write, no reset, 0 banners).
    4. Not done, deliberately: no flash, no NVS/portal write, no router or VPS
       change, no broker/ACL change, no credential read or logged. No re-run of
       the PMS diagnosis beyond the source reading quoted above.
    5. Correction carried from the task context, not re-verified here: the
       legacy bench firmware's pin name is `PIN_VEXT` (node.cpp), i.e. Vext in
       this tree is the *sensor/panel* rail gate, not an OLED-only reset line --
       the constant is named kOledVextPin because 0107's fix text named it that
       and the panel shares the rail, but its purpose here is the BME680's VIN.

  COST: deepseek-flash, standard tier, off-peak (PEAK: OFF-PEAK 12:15 UTC).
---

# 0108 — wildfire-node-v1: drive Vext at boot, verify PMS5003 framing

## Status: STAGED. Firmware deliverable complete and CI-verified; PMS5003 hardware confirmation reported as unperformed (reasons above).

## What was asked and what was done

1. **Drive Vext (GPIO36, active LOW) early in `setup()`, both roles** — DONE.
   `setup()` now drives the rail in a new STEP -1 block at lines 803-814, ahead of
   `probe_node_sensors()` (828) and `oled_probe_and_begin()` (860), i.e. ahead of
   the first `Wire.begin()` in the sketch. The pin is named
   `wf::kOledVextPin = 36` in `firmware_config.h` and the comment records the why
   (an unpowered chip on the bus clamps SDA, which is what blinded the probe).

2. **PMS5003** — no firmware defect suspected or found; reported as verification
   pending on the bench (see the notes block). The parser's header + 30-byte-sum
   contract and the probe's 1500 ms window are quoted there so the bench check has
   exact expected strings and a way to tell "no bytes" from "bad bytes".

3. **Probe OR logic and panel exclusion** — unchanged. `probe_node_sensors()` is
   byte-identical; only the pin constant and `setup()`'s first three statements
   changed.

## Evidence

- Branch `hermes/0108-vext-and-pms` @ `255e0ce60926c9bfbf46d99056faac880cf5b722`
  (`git ls-remote --heads origin hermes/0108-vext-and-pms`).
- Actions run 37201631922, conclusion `success`, both jobs green,
  `headSha == 255e0ce` (checked with `gh run view --json headSha,conclusion`):
  https://github.com/pagosacabin/nordtronics/actions/runs/37201631922
  - `build`: wildfire-node-v1 unified firmware built (`Flash: 18.7% (used 1222277
    bytes from 6553600 bytes)`), artifact
    `wildfire-node-v1-unified-firmware` id 11302294566, 769803 B.
  - `host-tests`: `27 test cases: 27 succeeded in 00:00:08.666`, including
    `test/test_role_and_portal/test_main.cpp:230: test_vext_gate_pin [PASSED]`.
- Local pre-flight (not evidence for the tip, just the fast feedback loop):
  `pio run -d firmware/wildfire-node-v1 -e heltec_v4` SUCCESS and
  `pio test -e native` `27 test cases: 27 succeeded`.
- ntfy receipt: topic `nordtronics-build-ed05a663`, id `RjrWwBnF16HE`,
  2026-10-04T12:21:16Z.

## Cost line

`deepseek-flash`, standard tier, off-peak (`PEAK: OFF-PEAK 12:15 UTC`). One task
worked this run; 0097/0098/0106/0107 remain parked and untouched.
