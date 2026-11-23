---
task_id: '0241'
protocol_version: 1.3.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  branch: hermes/0241-newcard-write
  sha: ceb31e1006d45187efb3aadbcefe49cefbf06545
  run: none — pagosacabin/cabin-play has no .github/workflows directory (`ls .github/workflows` fails; `git ls-tree -r --name-only origin/hermes/0241-newcard-write .github` is empty), so no workflow run exists or is expected for this branch.
  files:
    - living-frame/firmware/n16r8-sdprobe/src/main.c
    - living-frame/firmware/n16r8-sdprobe/README.md
    - living-frame/firmware/n16r8-sdprobe/logs/34-0241-board-pre-state.txt
    - living-frame/firmware/n16r8-sdprobe/logs/35-0241-newcard-run.txt
    - living-frame/firmware/n16r8-sdprobe/logs/36-0241-first-partial-run.txt
    - living-frame/firmware/n16r8-sdprobe/logs/37-0241-board-restored-800x600.txt
    - living-frame/firmware/n16r8-sdprobe/logs/38-0241-newcard-run-fixed.txt
    - living-frame/firmware/n16r8-sdprobe/logs/39-0241-board-restored-800x600.txt
    - living-frame/firmware/n16r8-sdprobe/logs/40-0241-newcard-run-review3.txt
    - living-frame/firmware/n16r8-sdprobe/logs/41-0241-board-restored-800x600.txt
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
notes: |
  Chain PASSES on the new card: format -> mount read-write -> 128 KiB write ->
  read back -> unmount -> remount -> read back, all three SHA-256s equal to the
  host-computed pattern and a byte-exact compare of 131072/131072 on both read
  passes. Card: MID 0x03 SanDisk "SD32G", CID PSN 0xc6bd0f92, CSD v2, 30,436 MB
  (31.9 GB), SDHC/SDXC — not the kit card (0x000013a4), not the Recalbox SR512
  (0xe4845e78, and 31.9 GB < 400 GiB), so the format was authorized and ran.

  WHAT THE PREVIOUS RUN LOOKED LIKE AND WHY. The 08:44 build's run reached
  KIT_FORMAT_START and its report read back the same content for eight hours.
  It was not stalled: report_vprintf() copies every esp_log line into a 60 kB
  RAM buffer, fold_driver_log_levels() turns the sdspi driver up to DEBUG/
  VERBOSE for the init measurement, and at 400 kHz that is ~1,000 lines of
  "spi_master: device5 locked the bus" per mount. The buffer was FULL before
  the format began, so every later app_logf — format result, all three hashes,
  the verdict — was silently dropped while the run itself went on to finish.
  Fixed: DEBUG/VERBOSE lines now go to the UART only (INFO+ still reach the
  report), and each stage logs its elapsed time. That build's report is 6.8 kB
  of the 60 kB buffer.

  MEASURED NUMBERS (passing run, log 40). Identity: CID and CSD read three
  times byte-identical, CRC7 and SPI CRC16 both verified (0x63/0x831d,
  0xc3/0xa35b). Pre-sacrifice ran (R1=0x01); one stock sdmmc_card_init -> OK.
  Mount attempt 1 (no format flag) OK. Format esp_vfs_fat_sdcard_format ->
  OK, allocation unit 65536 B (IDF's own log line), 190,950 ms. Volume
  31,910,985,728 B total. Write 128 KiB sha256 9fdfcb84…071dbd == expected,
  3,397 ms. Read back in place same hash, 131072/131072 bytes, 4,126 ms.
  Unmount ok, remount ok, read again same hash, 131072/131072, unmount ok.
  KIT_VERDICT: PASS.

  BOARD PRE-STATE AND END STATE. Report read before any flash (log 34): the
  accepted 800x600 photo build, framebuffer sha256 204578cc…bd306 MATCH.
  Board restored to that build after every run and verified from its own
  report each time (logs 37/39/41: mode 800x600, runtime == tool == 204578cc…,
  MATCH, 60.35 Hz, 40.0 fills/frame). The new card stays in the holder,
  formatted, with /NEWCARD.BIN (128 KiB) on it.

  PEER REVIEW (Copilot pane t17, three passes on this task's probe, all
  findings fixed). Pass 1: MAJOR the KATs and expected-hash comparisons only
  printed MATCH/MISMATCH and could not fail the run; MEDIUM the byte
  comparator counted mismatches but never failed the pass. Pass 2: MAJOR the
  per-pass *_VS_EXPECTED strcmp still bypassed that gate; MEDIUM a skipped
  KAT (buffer allocation failure) left the verdict flag set. Pass 3 on
  35a0490: "No findings... Verdict: SHIP". The bench run cited here is from
  the 35a0490 build, i.e. after every review finding was fixed.

  DEVIATIONS, DECLARED. (1) The probe changed beyond the task's literal
  scope: DEBUG/VERBOSE excluded from the report, stage timings, a heartbeat
  task, a 12-attempt raw-init ladder (the earlier 5-attempt ladder failed the
  gate on boots that came straight after a host reset), software SHA-256 with
  KATs, and a byte-exact read-back compare — every one of them a fix for an
  observed fault or a review finding, none of them touching clock speed, CS
  handling, pull-ups or the delay. (2) Three builds were flashed in total on
  this board before the passing one; each intermediate run is in the logs
  (36 is the one cut before its read-back). (3) Restoring the 800x600 build
  requires regenerating the raster from data/hummingbird.jpg with
  tools/convert_photo64.py --mode 800x600 --space gamma (hash 204578cc…);
  the generated headers were reverted afterwards and the committed tree is
  untouched. (4) One host read_flash during a run resets the chip and
  restarts the flow — every report read costs a fresh 190 s format, which is
  why the passing runs were read once each, after they had finished.

  COST: unknown. This harness does not expose per-call DeepSeek usage to the
  worker, so the token fields are 'unknown' rather than estimated.


# 0241 — New card: format, mount, write, read back
---

## Assignment

- **Hermes:** all of it — firmware change, bench run, report. (Bench firmware
  stays yours; nothing here goes to Newton.)
- **Copilot:** peer-review the probe change before the bench run, per the
  standing GitHub rule. Reviewer, not proof — Juno verifies the staged reply
  independently.
- **Juno:** verifies the reply (branch tip vs origin, numbers vs the logged
  report read back from the board), archives, pings Stephen.

## Context

0240 ran the write chain on the 1 GB kit card and stopped at driver init:
the gate passed (kit card, serial 0x000013a4), the pre-sacrifice ran clean,
and `sdmmc_card_init` then failed ESP_ERR_NOT_SUPPORTED (262) at
`sdmmc_init_spi_crc` — the card's CMD5→CMD59 quirk from 0229, which the
0239 fold does not cure. The failure follows that card.

Stephen has now seated a **new card** for this test. Same chain as 0240:
format → mount read-write → write → read back → hash, including across a
remount. This is the storage half of 0230 rung 3 (the ESP-NOW chunked
transfer itself is 0230's other half, not this task).

Base branch: `hermes/0240-kitcard-write`
@ 59ae567bb9d5bb5b7cc0dc5b111a2bfc76da5466 in pagosacabin/cabin-play,
project `living-frame/firmware/n16r8-sdprobe/` (identity-gate code, the
explicit format path, and the pattern generator are all there). Board:
YD-ESP32-23 / N16R8, MAC c4:9e:7e:08:6d:f4. SD pins SCK 12 / MOSI 11 /
MISO 13 / CS 10, SPI mode 0, 400 kHz.

## Task

Read the board's pre-state from the report partition before flashing
anything, and say what was on the board when you arrived.

1. **Identity check (hard gate).** Read the seated card's CID/CSD via the
   raw path and record what it is (manufacturer, product, serial,
   capacity). **If it is the 512 GB SanDisk SR512 (Recalbox), STOP — do
   not format anything — and stage the reply saying so.** That card is
   never scratch. If it is the 1 GB kit card again (serial 0x000013a4),
   say so and stop too — 0240 already measured it. Any other card is the
   new one Stephen seated for this test, and the format below is
   authorized for it.
2. **Driver init with the fold.** Stock-driver card init with the 0239
   pre-sacrifice in front of it. Record the first-call result and, on
   failure, the exact stage/error.
3. **Format.** Format the card FAT32 through the ESP-IDF/FatFS stack —
   the explicit `esp_vfs_fat_sdcard_format()` path 0240's build uses,
   not format_if_mount_failed. Say so in the reply.
4. **Write + verify.** Mount read-write. Write the same deterministic
   test pattern 0240 prepared (`bench/kitcard0240/expected_pattern.py`,
   128 KiB, host-computed sha256
   9fdfcb84758db72e1c98ada8d10a47a7cb9945ebf3afd47d0a5fbe2914071dbd).
   Close it, reopen, read it back in full, sha256 the read-back. Then
   unmount, remount, and read it back a second time across the remount.
   Both read-back hashes must match the expected hash.
5. Restore the board to the accepted 800x600 build afterward and verify
   from the board (framebuffer sha256 204578cc…, MATCH), as before. The
   new card stays in the holder, formatted, with the test file on it.

## Success criteria

- Card identity recorded, and the format/mount/write/read-back chain
  either completes with matching hashes (including across the remount),
  or stops at a named stage with the exact error logged.
- The full report read back from the board's report partition, committed
  as logs on the branch (same pattern as 0240's logs/31–33).

## Constraints

- **The Recalbox card is never touched.** The identity gate in step 1 is
  absolute: no format, no write, to the 512 GB SR512 under any reading of
  this task.
- If driver init fails on this card too, report the exact stage and stop
  — no in-task workaround ladder.
- Clock speed, CS handling, pull-ups unchanged; delay stays 0.
- Push the branch via SSH (gh.py writes to cabin-play return 403). No PR
  needed; a pushed branch + the staged reply is the deliverable.
- Cost: DeepSeek off-peak rates; one bench session.

## Proof

- Branch `hermes/0241-newcard-write` on pagosacabin/cabin-play at the
  pushed tip (Juno checks the SHA against origin).
- The stage-by-stage result with the written/read-back hashes, backed by
  the committed report-partition logs.
- Board-restored line (800x600, framebuffer hash MATCH).

## Reply format

Lead with the outcome in one sentence: the chain completed with matching
hashes, or it stopped at stage X with error Y. Then: card identity, init
result (and failure shape if any), format confirmation, the hashes
(expected, read-back, post-remount), the board pre-state you found, the
Copilot pass count and verdict, and the board-restored proof.


---

## Reply — 2026-11-22 18:10 MST (01:10 UTC 2026-11-23)

**The chain completed with matching hashes.** Card formatted FAT32 through
`esp_vfs_fat_sdcard_format()` (allocation unit 65536 B, 190.95 s), mounted
read-write, 128 KiB of the 0240 pattern written, read back in place, unmounted,
remounted and read back again — all three SHA-256s `9fdfcb84…071dbd` equal to the
host-computed value, and a byte-exact compare of 131072/131072 on both read
passes. `KIT_VERDICT: PASS`.

**Card identity.** MID 0x03 SanDisk, name `SD32G`, CID PSN `0xc6bd0f92`, CID
`03 53 44 53 44 33 32 47 85 c6 bd 0f 92 01 97 63`, CSD v2, 30,436 MB
(31,916,983,424 B), OCR `c0 ff 80 00` CCS=1. CID and CSD were each read three
times byte-identical with CRC7 and SPI CRC16 both verified. It is neither the kit
card (`0x000013a4`) nor the Recalbox SR512 (`0xe4845e78`, and 31.9 GB < 400 GiB),
so the format was authorized by the gate and nothing else was touched.

**Init result.** Pre-sacrifice ran (R1=0x01); one stock `sdmmc_card_init` at
400 kHz returned OK; the plain mount (attempt 1, no format flag) succeeded.

**Hashes.** expected `9fdfcb84758db72e1c98ada8d10a47a7cb9945ebf3afd47d0a5fbe2914071dbd`;
written, read-back and post-remount read are all that same value, with
131072/131072 bytes matching on both read passes (first mismatch offset -1).

**Pre-state found.** Report read before any flash: the accepted 800x600 photo
build, framebuffer sha256 `204578cc…bd306` MATCH.

**Board restored.** 800x600 build back on it and verified from its own report
after every run — `mode 800x600`, runtime == tool == `204578cc…bd306`, MATCH,
60.35 Hz, 40.0 fills/frame. The new card stays in the holder, formatted, with
`/NEWCARD.BIN` (128 KiB) on it.

**Copilot peer review: three passes, final "No findings. Verdict: SHIP" on
`35a0490`.** Pass 1 (MAJOR: KATs and expected-hash checks could not fail the run;
MEDIUM: the byte comparator never failed the pass) and pass 2 (MAJOR: the
per-pass `_VS_EXPECTED` compare bypassed that gate; MEDIUM: a skipped KAT left
the flag set) were both fixed, and the bench run cited above is from the build
after those fixes.

No `proof.run`: cabin-play has no `.github/workflows` directory, so no workflow
exists or is expected for this branch. The full stage-by-stage report read back
from the board is committed as `logs/40-0241-newcard-run-review3.txt`.
