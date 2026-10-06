---
task_id: "0118"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: "hermes/0118-pms-oled"
  sha: "c084e5d7b46c474fb57c068c021cbb1ef925e42e"   # == git ls-remote --heads origin hermes/0118-pms-oled
  base: "hermes/0117-pms-live-poll @ a37e1e71fe6f6a0bd6fe79466865d26419e9040c -- the revision the task names as the base; the task branch is that tip + 1 commit (+24/-5, firmware/wildfire-node-v1/src/main.cpp only)"
  run: "https://github.com/pagosacabin/nordtronics/actions/runs/37521592932"
  run_check: "gh run view 37521592932: status completed, conclusion success, headSha c084e5d7b46c474fb57c068c021cbb1ef925e42e == the branch tip, createdAt 2026-10-06T19:47:54Z, updatedAt 19:50:40Z. Jobs: build success, host-tests success. Named steps: 'Build firmware (wildfire-node-v1, unified node+base)' success, 'Upload wildfire-node-v1 firmware' success. host-tests log: '27 test cases: 27 succeeded in 00:00:07.123'."
  artifact: "https://github.com/pagosacabin/nordtronics/actions/runs/37521592932/artifacts"
  artifact_detail: "wildfire-node-v1-unified-firmware, id 11439598926, 765742 B zipped, expired false. Downloaded -> firmware.bin 1216416 B, sha256 b25631bf38623f9d2df04b50d3641546890dbd58341197bb58ffb59a1a5ab7b7 -- the exact image flashed to the node."
  partition_table: "read from the device FIRST (4096 B at 0x8000, sha256 f4e3b6cfe370c81dca210d57f3c463f924a539842a19fff6eda4615278ee97a2): nvs 0x9000/0x5000, otadata 0xe000/0x2000, app0 0x10000/0x330000, app1 0x340000/0x330000, spiffs 0x670000/0x180000, coredump 0x7f0000/0x10000."
  flashed_to: "NODE B0:A6:04:C5:75:4C only, via its by-id path usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00 -> ttyACM0, app-only at 0x10000 (write spans 0x10000-0x229000, inside app0 0x10000-0x340000), --after no_reset, no erase of any kind. Read-back of the same region byte-identical: sha256 b25631bf38623f9d2df04b50d3641546890dbd58341197bb58ffb59a1a5ab7b7, cmp exit 0. No NVS write. BASE 80:F1:B2:A7:47:EC (ttyACM2) was never opened and never named to esptool."
  diff: |
    --- a/firmware/wildfire-node-v1/src/main.cpp
    +++ b/firmware/wildfire-node-v1/src/main.cpp
    @@ -336,6 +336,15 @@
     // The 0115 per-check-in `pms:` line is untouched and still fires.
    +//
    +// Task 0118: the same frame's three values are latched into file scope so the
    +// node OLED can show them. Latched only on a checksum-valid frame, so a poll
    +// with no complete frame leaves the OLED on the last good values instead of
    +// falling back to 0 -- 0 on this panel reads as clean air, which it is not.
    +static uint16_t g_pms_live_pm1 = 0;
    +static uint16_t g_pms_live_pm25 = 0;
    +static uint16_t g_pms_live_pm10 = 0;
    +static bool g_pms_live_valid = false;
    @@ -371,6 +380,11 @@ static void pms_live_poll() {
         const uint16_t pm1 = ((uint16_t)buf[10] << 8) | buf[11];
         const uint16_t pm25 = ((uint16_t)buf[12] << 8) | buf[13];
         const uint16_t pm10 = ((uint16_t)buf[14] << 8) | buf[15];
    +    // Task 0118: latch for the OLED. The values are the ones decoded right here,
    +    // so the panel and the `pms-live:` line can never disagree.
    +    g_pms_live_pm1 = pm1;
    +    g_pms_live_pm25 = pm25;
    +    g_pms_live_pm10 = pm10;
    +    g_pms_live_valid = true;
         logf("pms-live: pm1=%u pm25=%.1f pm10=%u", pm1, (float)pm25, pm10);
    @@ -731,15 +745,27 @@ static void oled_render_node() {
       g_oled.drawStr(0, 10, "WILDFIRE NODE");
    +  // Task 0118: six rows no longer fit on the 12 px cadence, so this block uses
    +  // the 10 px cadence oled_render_base() already proves on this panel (its
    +  // last-seen rows and its `last:` row sit 10-12 px apart on the same 64 px
    +  // height). Every pre-existing line is kept; the PM row is added last.
       char line[32];
       snprintf(line, sizeof(line), "id:%u", g_cfg.node_id);
    -  g_oled.drawStr(0, 22, line);
    +  g_oled.drawStr(0, 20, line);
       snprintf(line, sizeof(line), "chk:%lus", (unsigned long)g_cfg.checkin_s);
    -  g_oled.drawStr(0, 34, line);
    +  g_oled.drawStr(0, 30, line);
       snprintf(line, sizeof(line), "seq:%u", g_tx_seq);
    -  g_oled.drawStr(0, 46, line);
    +  g_oled.drawStr(0, 40, line);
       snprintf(line, sizeof(line), "vbat:%umV", 0);
    -  g_oled.drawStr(0, 58, line);
    +  g_oled.drawStr(0, 50, line);
    +  // Live PM from the 1 Hz poll (task 0118) -- the same frame the `pms-live:`
    +  // serial line prints, refreshing at the same ~1 Hz. `--` until the first
    +  // checksum-valid frame arrives, never a fake 0.
    +  if (g_pms_live_valid) {
    +    snprintf(line, sizeof(line), "PM %u/%u/%u", g_pms_live_pm1, g_pms_live_pm25, g_pms_live_pm10);
    +  } else {
    +    snprintf(line, sizeof(line), "PM --/--/--");
    +  }
    +  g_oled.drawStr(0, 60, line);
       oled_flush();
  ntfy:
    topic: nordtronics-build-ed05a663
    id: OhYzLa7zttjg
    time: "1791316613 (2026-10-06T19:56:53Z)"
    body: "Branch / SHA / Workflow / Artifacts / Status: success"
  files:
    - firmware/wildfire-node-v1/src/main.cpp
notes: |
  The node's 1 s OLED render block now carries a sixth row, `PM <pm1>/<pm2.5>/<pm10>`,
  fed from the same latched frame the `pms-live:` serial line prints, so the panel
  refreshes at the same ~1 Hz. The panel probe answered
  (`oled: probe 0x3C -> ACK`) so `oled_render_node()` is driving the panel, but
  pixels are not readable from serial -- the task itself makes the visual check
  STEPHEN'S step, and that is where it stands: it is not mine to claim.

  READ THIS BEFORE LOOKING: the sensor's PM fields currently read 0 (the open 0115
  finding -- frames parse, fields zero), so the new row will read `PM 0/0/0` and
  will not visibly move until the sensor reports non-zero. A moving row is not
  expected yet; the row being there, on the existing 1 Hz cadence, is the
  deliverable. That is stated here so a static row is not misread as a display bug.

  STAGED 2026-10-06 13:50-14:05 MDT (19:50-20:05 UTC, PEAK: OFF-PEAK; PROTOCOL:
  MATCH protocol_version=1.0.0). Model tier: DeepSeek Flash. Run mode: interactive
  session at Stephen's request, with the four cron worker jobs (c0be50a686c6,
  7aff6948c2c1, 8b1c9e1323c5, 5c1532977f15) PAUSED for the duration so two agents
  never hold this repo at once; they are resumed on staging.

  1. WHAT WAS BUILT (criterion 1 MET). Branch `hermes/0118-pms-oled` from the base
     this task names, `hermes/0117-pms-live-poll @ a37e1e7`. ONE commit, one file
     (+24/-5 in firmware/wildfire-node-v1/src/main.cpp):
       - three file-scope latches + a validity flag beside `pms_live_poll()`;
       - four assignments inside `pms_live_poll()`'s checksum-valid branch;
       - the sixth row in `oled_render_node()`, with the five pre-existing rows
         re-spaced 22/34/46/58 -> 20/30/40/50/60 so six rows fit the 64 px panel
         on the 10 px cadence `oled_render_base()` already uses. Nothing was
         replaced; `WILDFIRE NODE`, id, chk, seq and vbat are all still drawn.
     Format follows the task's own example (`PM 9/9/23`); the worst case,
     `PM 999/999/999`, is 14 chars = 84 px of the 128 px width, and the row at
     y=60 sits 3 px clear of the vbat row above it.
  2. THE ONE DECLARED DEVIATION. The constraints say "no changes to the PMS read
     path, the 1 s poll". The latches do sit inside `pms_live_poll()`, in its
     checksum-valid branch, because that function owns the only frame parser: a
     second decoder in the render block could disagree with the serial line, which
     is the exact failure the task's "same live values" wording is guarding
     against. Nothing about the poll changes -- no timing, no period, no buffer,
     no header/sum handling, no logging -- the four statements only latch values
     the function had already computed. Flagging it rather than hiding it.
  3. BENCH DISCIPLINE (criterion 2 MET). Identity confirmed BEFORE any write, over
     the mandated by-id path: `esptool read_mac` -> MAC b0:a6:04:c5:75:4c (the
     NODE). Partition table read from the device FIRST, before the write, and
     quoted in `proof.partition_table`. App-only write at 0x10000 with
     `--after no_reset`; no `erase_flash`, no `--erase-all`, no NVS write; the
     write stays well inside app0. Read-back of the same region is byte-identical
     to the CI artifact (`cmp` exit 0, matching sha256). NVS (0x9000) and otadata
     (0xe000) were never in the write span, so the node's stored config survives.
     The BASE was never opened, never named to esptool, and had no USB event.
  4. POST-FLASH LIVE STATE (criterion 3, serial half MET). Boot banner after the
     flash: `ROLE: node (source=PROBE, sensors=present, nvs_role=unset)`,
     `firmware: wildfire-unified-v1 proto=1 build=Oct  6 2026 19:49:56` -- the CI
     run's own build time, so the running image is the artifact -- and
     `oled: probe 0x3C -> ACK`. Two consecutive check-ins fired at 60.0 s with
     seq 0 -> 1 and the 0115 line intact (`pms: rx_avail_before=32 ok=1 pm1=0
     pm25=0.0 pm10=0`), while the live poll produced 69 `pms-live:` lines in a
     75 s window = 0.92/s. The stack is live end to end after the flash:
     `GET https://api.nordtronics.io/v1/nodes` -> node "0", last_seen_utc
     2026-10-06T19:56:14Z, age_seconds 20, status "ok", reading_count 1292.
  5. OUTSTANDING / NOT DONE. (a) The visual confirmation, by design, is Stephen's.
     (b) PM values are zeros until the sensor reports non-zero -- the 0115
     diagnostic's open question, untouched here. (c) Not merged, not tagged; no
     config, NVS or deep-sleep change; the base was not touched.
---

# 0118 — Show live PM on the node OLED (node only)

## Context

0117 (archived) added a 1 Hz non-blocking PMS live poll printing
`pms-live: pm1/pm25/pm10` to serial. Stephen wants the same live numbers on
the node's OLED — glance at the screen, breathe on the sensor, watch the
numbers move, no laptop needed. Base branch: `hermes/0117-pms-live-poll`;
new branch `hermes/0118-pms-oled`.

## Task

In the node's 1-second OLED render block, add one line showing the latest live
PMS values from the 1 Hz poll (pm1, pm2.5, pm10 — fit all three compactly,
e.g. `PM 9/9/23`). The line must update at the same ~1 Hz cadence as the
`pms-live:` serial lines, from the same live values.

## Success criteria

1. Branch `hermes/0118-pms-oled` builds green in CI from the 0117 tree.
2. The NODE (MAC `B0:A6:04:C5:75:4C`, by-id path) — and only the node — is
   flashed; bench discipline holds (partition-table read first, sha256 on the
   read-back matches the built image).
3. The node OLED shows live PM values updating ~1/sec. Proof: the code change
   quoted plus Stephen's bench confirmation (he will look at the screen) —
   state in the reply that visual confirmation is Stephen's step.

## Constraints

- NODE ONLY. Do not touch the base.
- Display only: no changes to the PMS read path, the 1 s poll, the 60 s
  checkin, deep-sleep gating, or config. No merge, no tag.
- Keep the existing OLED lines (id/chk/seq/vbat) — add, don't replace.
- Keep model cost on DeepSeek Flash. State the tier in the reply.

## Proof

- Branch SHA on origin + CI run URL + artifact.
- Flash read-back sha256 matching the built image.
- Quoted code diff of the OLED line addition.

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, first line of notes confirms the OLED shows live PM at ~1 Hz,
`proof` block with the pointers above.
