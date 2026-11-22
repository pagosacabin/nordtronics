---
task_id: '0239'
protocol_version: 1.3.0
status: staged
iteration: 2
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0239-driver-fold
  sha: d6b555c39c8c8cdc1f6bc470bd55b9131759ecfe
  run: none
  run_note: >-
    cabin-play carries no .github directory at all (`ls .github` fails; `git ls-tree
    -r --name-only origin/main .github` is empty), so no CI run exists or is expected
    for this branch. ntfy not applicable: the task names no topic, and nothing was
    published.
  files:
    - living-frame/firmware/n16r8-sdprobe/src/main.c
    - living-frame/firmware/n16r8-sdprobe/README.md
    - living-frame/firmware/n16r8-sdprobe/bench/fold-0239/reset-loop.sh
    - living-frame/firmware/n16r8-sdprobe/bench/fold-0239/extract_report.py
    - living-frame/firmware/n16r8-sdprobe/bench/fold-0239/parse_trace.py
    - living-frame/firmware/n16r8-sdprobe/bench/fold-0239/arm_summary.py
    - living-frame/firmware/n16r8-sdprobe/logs/22-0239-control-12resets.txt
    - living-frame/firmware/n16r8-sdprobe/logs/23-0239-sacrifice-12resets.txt
    - living-frame/firmware/n16r8-sdprobe/logs/24-0239-control-table.txt
    - living-frame/firmware/n16r8-sdprobe/logs/25-0239-sacrifice-table.txt
    - living-frame/firmware/n16r8-sdprobe/logs/26-0239-control-harness.log
    - living-frame/firmware/n16r8-sdprobe/logs/27-0239-sacrifice-harness.log
    - living-frame/firmware/n16r8-sdprobe/logs/28-0239-board-pre-state.txt
    - living-frame/firmware/n16r8-sdprobe/logs/29-0239-board-restored-800x600.txt
    - living-frame/firmware/n16r8-sdprobe/logs/30-0239-driver-traces.txt
  board_restored: >-
    800x600 accepted build; esptool verify-flash "Verification successful (digest
    matched)" against .pio/build/n16r8-photo-800x600/firmware.bin, and the board's own
    report after the restore is byte-identical to the pre-state read-back (diffed):
    mode 800x600, framebuffer sha256 204578cc... == tool sha256, MATCH, 362 vsync in
    5.998 s -> 60.35 Hz, 14473 fills -> 40.0 per frame.
  peer_review: "Copilot (Herdr pane t17), 4 passes, findings fixed each pass, final Verdict: SHIP"
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-22T11:46:00+00:00"
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
  STAGED 2026-11-22 05:25 MST. Both arms ran on the board; the tally below is the
  board's own persisted table, read back from the report partition, not a hand count.

  Numbers (logs/24 control, logs/25 sacrifice -- FOLD_SLOT lines):
    control   (SD_FOLD_ARM=0): first-call sdmmc_card_init OK 11/19 boots (58 %).
      8 failures, every one ESP_ERR_INVALID_CRC (265) inside sdmmc_io_reset.
    sacrifice (SD_FOLD_ARM=1): first-call sdmmc_card_init OK 14/14 boots (100 %).
      sacrifice CMD0 responses: 0x3f x9, 0x47 x4, 0x7f x1 (junk every boot, discarded).

  The mechanism is one command earlier than 0234 assumed. The driver's first
  transaction in sdmmc_card_init is the SDIO reset CMD52 (sdmmc_io_reset runs
  before sdmmc_send_cmd_go_idle_state), and that is the frame that comes back as
  r1=0x3f and aborts the call. driver_cmd0_frames (counted from the driver's own
  log) is 2 on every boot that got past CMD52 and 0 on every boot that failed, so
  the driver's mandatory double CMD0 in SPI mode always ran when reached: its
  internal retry is not what this fault defeats, it is the first transaction of the
  call. That also explains why 0234's stock mount succeeded: the probe had already
  pushed hundreds of transactions through the bus in the same boot before the
  driver initialised. Verdict sentence, as the task asked: the pre-sacrifice DOES
  make the stock driver's first init reliable.

  Two deviations/extras declared:
  1. The mount runs on EVERY sacrifice boot, after the first-call result is written
     and flushed to the report partition, so the arm's last boot is the task's
     "final sacrifice-arm boot" without a second firmware. It cannot contaminate the
     measurement: FOLD_FIRST_INIT is logged and the report partition written before
     the mount starts. It is read-only: esp_vfs_fat_sdspi_mount with
     format_if_mount_failed=false, opendir/readdir + f_open("rb"); no write API is
     called anywhere in either arm.
  2. The harness's own read_flash ends with a hard reset, so each 12-reset loop also
     produces trailing boots; the board's table therefore holds more boots than the
     12 host resets per arm (19 and 14). Both counts are quoted above; the per-boot
     read-backs (logs/22, logs/23) sample the boot each read_mac reset produced.

  Peer review, four passes (all findings fixed before the bench run, in commit
  order): (1) a sacrifice whose bus setup failed still counted as a trial -> now
  returns esp_err_t, the boot is arm=0xFE INVALID and excluded; (2) table key was
  the build stamp alone -> now the stamp plus the arm; (3) fold_one_init() failures
  before sdmmc_card_init() were recorded as the call's result -> now init_called;
  (4) sd_raw_cmd() ignored the trailing 8-clock transfer's result -> now sets
  g_raw_tx_err. Final Verdict: SHIP.

  The base 0229/0234/0236/0238 probe is unchanged when the flag is absent: every
  added line is inside `#if SD_FOLD_TASK` except the g_raw_tx_err declaration/sets
  in sd_raw_cmd (a flag the base path never reads) and the CMD0-frame counter in the
  log hook (also fold-only). The base build still compiles (verified).

  Pre-state found on arrival: the accepted 800x600 build (logs/28), framebuffer
  sha256 204578cc..., MATCH, 362 vsync in 5.998 s. Card seated throughout: SanDisk
  SR512, CSD capacity 511,868,665,856 B (512 GB), label RECALBOX -- the 0234 card,
  READ-ONLY, no write test (0230's write half still waits for Stephen to name a
  scratch card).

  Cost: this harness exposes no per-call API usage to the cron session, so the token
  and cost fields are `unknown`, not estimated.
---

# 0239 — Result: the pre-sacrifice DOES make the stock driver's first init reliable

| Arm | `-DSD_FOLD_ARM=` | first-call `sdmmc_card_init` OK / boots | failure shape |
| --- | --- | :---: | --- |
| Control | 0 | **11 / 19** (58 %) | 8 × `ESP_ERR_INVALID_CRC` (265) at `sdmmc_io_reset` |
| Pre-sacrifice | 1 | **14 / 14** (100 %) | none |

**Verdict: the pre-sacrifice does do the job — one throwaway CMD0 after the dummy
clocks takes the stock driver's first init from 11/19 to 14/14, because the bad
first frame lands on the sacrificial frame instead of on the driver's own first
command.**

The control arm *does* show the first-call fault, so the task's alternative lead
does not apply. The failure shape is one command earlier than 0234 assumed: the
first transaction of `sdmmc_card_init` is the SDIO reset `CMD52` (inside
`sdmmc_io_reset`, before `sdmmc_send_cmd_go_idle_state`), and that is the frame
that returns `r1=0x3f` and aborts the call. `driver_cmd0_frames` is **2** on every
boot that got past CMD52 and **0** on every boot that failed — the driver's own
double CMD0 always ran when it was reached, so its internal retry is not what this
fault defeats.

In the sacrifice arm the throwaway CMD0 caught the junk every single boot —
`0x3f` ×9, `0x47` ×4, `0x7f` ×1, discarded — and the driver's call after it was
clean 14/14.

**End-to-end (final sacrifice boot):** every sacrifice boot mounts read-only after
its measurement is flushed, so the last boot is the proof —
`MOUNT[fold e2e read-only 400 kHz]: OK card=SR512`, `ROOT_ENTRIES: 23`, the
RECALBOX listing (`logs/23`, `FOLD_E2E_MOUNT`). `format_if_mount_failed=false`;
no write API is called anywhere.

**Board pre-state and restore:** arrived on the accepted 800x600 build
(`logs/28`: `mode 800x600`, `sha256 204578cc…`, `MATCH`, `362 vsync in 5.998 s`).
Restored afterwards — `esptool verify-flash` → `Verification successful (digest
matched)` — and the board's own report is byte-identical to the pre-state
read-back (`logs/29` vs `logs/28`, diffed).

**Peer review:** Copilot (Herdr pane t17), **4 passes**, one MAJOR + one NIT +
one NIT + one MAJOR raised, each fixed before the bench run; final
**Verdict: SHIP**.

**Pointers:** branch `hermes/0239-driver-fold` @
`d6b555c39c8c8cdc1f6bc470bd55b9131759ecfe` on pagosacabin/cabin-play (no
`.github` directory in that repo, so `run: none` is enumerated, not assumed);
tables `logs/24` and `logs/25`; per-boot read-backs `logs/22`, `logs/23`; driver
traces `logs/30`; harness and analysers `bench/fold-0239/`; design record in
`README.md` § "0239".

---

## Original task (as filed)
# 0239 — Does a pre-sacrifice make the stock driver's first init clean?

## Assignment

- **Hermes:** all of it — firmware change, bench run, report. (Bench firmware
  stays yours; nothing here goes to Newton.)
- **Copilot:** peer-review the probe change before the bench run, per the
  standing GitHub rule. Reviewer, not proof — Juno verifies the staged reply
  independently.
- **Juno:** verifies the reply (branch tip vs origin, numbers vs the logged
  report read back from the board), archives, pings Stephen.

## Context

0234 proved the stock ESP-IDF driver mounts this card — but the driver
retries internally, so its success hides the first-pass fault that 0234's
raw measurement exposed (first CMD0 junk, second attempt clean, every
reset). 0238 proved a raw sacrificial CMD0 after the dummy clocks cures the
raw first pass: 13/13 vs a 0/13 control, scoped to host-driven resets (the
14-boot self-reboot burst in 0238 is unexplained and stays on the record).

Open question this task answers: does a raw pre-sacrifice on the bus —
dummy clocks + one throwaway CMD0, discarded — carry over to the **stock
driver's** init, so the driver's own first card-init call succeeds without
needing its internal retry? If the shift lives in the card's state, it
should. If it lives in the host peripheral's first transaction after the
driver configures it, it may not — either answer decides how the real
firmware (0230's write path, the photo build) has to be structured.

Base branch: `hermes/0238-cs-framing`
@ e9f63aeefb8a9cb4c4593d741b98991e136d7936 in pagosacabin/cabin-play,
project `living-frame/firmware/n16r8-sdprobe/`. Board: YD-ESP32-23 /
N16R8, MAC c4:9e:7e:08:6d:f4. SD pins SCK 12 / MOSI 11 / MISO 13 / CS 10,
SPI mode 0, 400 kHz. Delay parameter stays 0.

## Task

Two arms, ≥12 host resets each, same harness discipline as 0236/0238
(esptool hard_reset, settle, report read back from 0x310000 each reset).
Read the board's pre-state from the report partition before flashing
anything, and say what was on the board when you arrived.

0. **Control arm** — per reset, call the driver's card-init level directly
   (`sdmmc_card_init` or the closest directly-callable equivalent) ONCE
   and record its result: success, or the failure returned. Do not let the
   mount's retry loop mask it — the first call's outcome is the
   measurement. If the init level genuinely cannot be called standalone,
   say so and use the closest observable that separates first-try from
   retried success (e.g. driver debug logs showing the failed first CMD0),
   declared in the reply.
1. **Sacrifice arm** — per reset: run the raw pre-sacrifice on the bus
   first (≥74 dummy clocks CS high, one throwaway CMD0, response logged
   and discarded, CS high), then the identical single card-init call.
   Record its result the same way.
2. **End-to-end check** — on the final sacrifice-arm boot, after the clean
   first init: mount read-only via `esp_vfs_fat_sdspi_mount`
   (format_if_mount_failed=false) and list the root directory, as 0234
   did. The Recalbox contents listing is the proof the path works whole.

Record per arm: first-call init successes / resets, plus the failure shape
in the control arm (does the driver's first call fail with the same junk
signature the raw probe sees?).

Restore the board to the accepted 800x600 build afterward and verify from
the board (framebuffer sha256 204578cc…, MATCH), as in 0234/0236/0238.

## Success criteria

- A table: arm → first-call init successes / resets run, control included.
- A verdict in one sentence: the pre-sacrifice does or does not make the
  stock driver's first init reliable.
- The full reports read back from the board's report partition, committed
  as logs on the branch (same pattern as 0238's logs/14–19).

## Constraints

- Card is READ-ONLY: no writes to the card in any arm, no write test, no
  format. The mount is a read-only listing only. (The 0230 write test
  waits for a scratch card Stephen names — not this task.)
- One variable: the pre-sacrifice. No clock-speed, CS-handling, pull-up,
  or delay changes. The 3.3 V rail question is Stephen's meter task, not a
  rider here.
- Push the branch via SSH (gh.py writes to cabin-play return 403). No PR
  needed; a pushed branch + the staged reply is the deliverable.
- Cost: DeepSeek off-peak rates; one bench session.

## Proof

- Branch `hermes/0239-driver-fold` on pagosacabin/cabin-play at the pushed
  tip (Juno checks the SHA against origin).
- The arm-vs-first-call table, backed by the committed report-partition
  logs.
- Board-restored line (800x600, framebuffer hash MATCH).

## Reply format

Lead with the table and the one-sentence verdict. Then: control failure
shape, the end-to-end mount/listing result, the board pre-state you found,
the Copilot pass count and verdict, and the board-restored proof. If the
control arm does not show the first-call fault, lead with that instead —
it changes what 0234's driver success meant.
