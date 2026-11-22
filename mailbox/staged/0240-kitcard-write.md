---
task_id: "0240"
protocol_version: 1.3.0
status: staged
iteration: 2
proof:
  branch: "hermes/0240-kitcard-write"
  repo: "pagosacabin/cabin-play"
  sha: "59ae567bb9d5bb5b7cc0dc5b111a2bfc76da5466"
  run: none
  run_note: |
    cabin-play carries no workflows at all: `ls .github/workflows` is "No such file or
    directory" and `git ls-tree -r --name-only origin/hermes/0240-kitcard-write
    .github/workflows` returns empty. No run exists or is expected for this branch.
  files:
    - living-frame/firmware/n16r8-sdprobe/src/main.c
    - living-frame/firmware/n16r8-sdprobe/bench/kitcard0240/expected_pattern.py
    - living-frame/firmware/n16r8-sdprobe/logs/31-0240-board-pre-state.txt
    - living-frame/firmware/n16r8-sdprobe/logs/32-0240-kitcard-gate-and-init.txt
    - living-frame/firmware/n16r8-sdprobe/logs/33-0240-board-restored-800x600.txt
    - living-frame/firmware/n16r8-sdprobe/README.md
  board_restored: "800x600 build, framebuffer sha256 204578cc...bd306 MATCH (logs/33)"
  ntfy: "not published -- this task names no topic and produces no compiled deliverable"
  peer_review: "Copilot (Herdr pane t17), 3 passes, findings fixed each pass, final Verdict: SHIP at f2fcbe9"
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-22T13:47:00+00:00"
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
  note: "This harness exposes no per-call usage to the cron session; token and cost fields are unknown rather than estimated."
notes: |
  OUTCOME: the chain stopped at the driver-init stage with the exact error, which is one
  of the two outcomes this task's success criteria name. The identity gate PASSED -- the
  seated card is the kit card -- and the stock driver then refused it at
  `sdmmc_init_spi_crc` with ESP_ERR_NOT_SUPPORTED (262). No format, no write: the firmware
  stops there by design and this run followed the constraint "no in-task workaround ladder".

  What the run measured (logs/32, full report read back from 0x310000):
    - card identity: CID 05 00 0c 61 73 64 66 67 22 00 00 13 a4 01 9a 00 (serial
      0x000013a4 -- the kit card 0229 recorded), CSD v1, 960 MB, OCR 80 ff 80 00, CCS=0
      byte addressing. Gate: PASS.
    - 0239 pre-sacrifice: ran, R1=0x01 (idle), discarded.
    - sdmmc_card_init 400 kHz: FAIL 262 at sdmmc_init_spi_crc. Driver's own log:
      CMD52 R1=0x05 (tolerated) -> CMD0 -> CMD8 R7 0xaa0100f0 -> CMD5 error 0x107
      ("not IO card") -> CMD59 R1=0x05 illegal -> `sdmmc_init_spi_crc returned 0x106`.
      That is exactly the CMD5-poisons-CMD59 shape 0229 named for this card.
    - mounts 1, 2 and 3 (plain, retry, format_if_mount_failed=true): all FAIL 262, same stage.
  So 0239's fold does NOT rescue this card: 0239's 14/14 first-init and its successful
  mount were measured on the 512 GB SR512 card. The failure follows the card.

  Board pre-state read before any flash (logs/31): the accepted 800x600 build, framebuffer
  sha256 204578cc...bd306 MATCH. Board restored afterwards and verified from its own report
  (logs/33): same build, same hash, MATCH, 362 vsync in 5.998 s.

  Peer review: Copilot pane t17, three passes, all against the pushed diff.
    pass 1 -> BLOCKER (gate admitted any ~1 GB SDSC card; no CID check) + MEDIUM
              (format_if_mount_failed reported as the format) + MEDIUM (unmount result
              ignored) -> Verdict: FIX.
    pass 2 -> BLOCKER: my first fix shifted the PSN parse to bytes 10..13 and would have
              made the kit card fail its own gate. It was right; reverted to cid[9..12],
              checked against 0229's recorded raw CID (00 00 13 a4 = 0x000013a4).
              -> Verdict: FIX.
    pass 3 -> all three findings RESOLVED, no new findings -> Verdict: SHIP.
  The self-caught mis-parse is declared here because it was fixed before the bench run and
  would otherwise be invisible in the diff history.

  One prior-run fact, declared: none of this work came from an earlier tick; the firmware,
  the review passes and the bench run all happened in this tick.

  Decision owed (not a blocker for this task, which is report-and-stop): the write path on
  THIS card is unavailable through the stock driver. Two real options -- (a) seat a card
  that implements CMD59 in idle state (any ordinary card) for 0230 rung 3, or (b) file the
  raw-sector write experiment (bypass sdmmc_init_spi_crc) as its own task. The kit card
  itself answers every read and is undamaged by this run: no format call was reached and no
  write command was issued.
---

# 0240 — Kit card: format, mount, write, read back

## Assignment

- **Hermes:** all of it — firmware change, bench run, report. (Bench firmware
  stays yours; nothing here goes to Newton.)
- **Copilot:** peer-review the probe change before the bench run, per the
  standing GitHub rule. Reviewer, not proof — Juno verifies the staged reply
  independently.
- **Juno:** verifies the reply (branch tip vs origin, numbers vs the logged
  report read back from the board), archives, pings Stephen.

## Context

The SD investigation chain is closed on the read side: 0236 killed the
delay hypothesis, 0238 proved a sacrificial CMD0 cures the first-frame
shift (13/13), and 0239 proved the pre-sacrifice carries over to the stock
ESP-IDF driver (first-call init 14/14 vs 11/19 control). What has never
been tested is a **write** — 0230's rung 3 (ESP-NOW chunked transfer →
file on the frame's SD card) needs one, and it waited for a scratch card.

Stephen has now designated one: **the 1 GB kit card** (the SDSC card from
0229 — CMD0 10/10 at raw level, OCR 0x80FF8000, CSD = 1 GB SDSC) and
approved erasing it. Two cautions from the record:

1. The kit card was never proven blank (0229's "root empty" was a probe
   bug). Erasure is approved anyway — but read and record its identity
   (CID/CSD) before formatting, for the record.
2. This card has its own known quirk, separate from the framing fault:
   in 0229 the stock driver's init failed on it — the driver's SDIO CMD5
   probe appears to poison this card's firmware, after which CMD59 is
   rejected. Whether the 0239 pre-sacrifice changes that is unknown. A
   refusal here damns this card, not the write path — report the exact
   failure stage and stop rather than inventing workarounds in-task.

Base branch: `hermes/0239-driver-fold`
@ d6b555c39c8c8cdc1f6bc470bd55b9131759ecfe in pagosacabin/cabin-play,
project `living-frame/firmware/n16r8-sdprobe/`. Board: YD-ESP32-23 /
N16R8, MAC c4:9e:7e:08:6d:f4. SD pins SCK 12 / MOSI 11 / MISO 13 / CS 10,
SPI mode 0, 400 kHz.

## Task

Read the board's pre-state from the report partition before flashing
anything, and say what was on the board when you arrived.

1. **Identity check (hard gate).** Read the seated card's CID/CSD via the
   raw path. It must be the ~1 GB kit SDSC card. **If it is the 512 GB
   SanDisk SR512 (Recalbox), or the holder is empty, STOP — do not format
   anything — and stage the reply saying what you found.** Nothing in this
   task authorizes touching any other card.
2. **Driver init with the fold.** Attempt stock-driver card init with the
   0239 pre-sacrifice in front of it. Record the first-call result and,
   on failure, the exact stage/error (especially whether it matches
   0229's CMD5/CMD59 shape). One retry via the driver's normal mount path
   is fine; record both outcomes.
3. **Format.** Format the card FAT32 through the ESP-IDF/FatFS stack
   (f_mkfs, or mount with format_if_mount_failed=true on the empty card —
   say which you used).
4. **Write + verify.** Mount read-write. Write one test file: a
   deterministic pattern of at least 64 KB (generate it in firmware;
   record its sha256 as computed by the board or the host tool — say
   which). Close it, reopen, read it back in full, sha256 the read-back.
   Then unmount, remount, and read it back a second time across the
   remount. Both hashes must match the written pattern's hash.
5. Restore the board to the accepted 800x600 build afterward and verify
   from the board (framebuffer sha256 204578cc…, MATCH), as before. The
   kit card stays in the holder, formatted, with the test file on it —
   Stephen will want to see it.

## Success criteria

- Card identity recorded, and the format/mount/write/read-back chain
  either completes with matching hashes (including across the remount),
  or stops at a named stage with the exact error logged.
- The full report read back from the board's report partition, committed
  as logs on the branch (same pattern as 0239's logs/22–30).

## Constraints

- **Kit card only.** The identity gate in step 1 is absolute: no format,
  no write, to any other card, under any reading of this task.
- If driver init fails on this card, report and stop at that stage — no
  in-task workaround ladder. (A raw-sector write experiment is a
  different task if Stephen wants one.)
- Clock speed, CS handling, pull-ups unchanged; delay stays 0.
- Push the branch via SSH (gh.py writes to cabin-play return 403). No PR
  needed; a pushed branch + the staged reply is the deliverable.
- Cost: DeepSeek off-peak rates; one bench session.

## Proof

- Branch `hermes/0240-kitcard-write` on pagosacabin/cabin-play at the
  pushed tip (Juno checks the SHA against origin).
- The stage-by-stage result with the written/read-back hashes, backed by
  the committed report-partition logs.
- Board-restored line (800x600, framebuffer hash MATCH).

## Reply format

Lead with the outcome in one sentence: the chain completed with matching
hashes, or it stopped at stage X with error Y. Then: card identity, init
result (and failure shape if any), format method, the three hashes
(written, read-back, post-remount), the board pre-state you found, the
Copilot pass count and verdict, and the board-restored proof.

## Reply 2026-11-22 07:05 MST (14:05 UTC) — stopped at driver init, gate PASSED

**Outcome in one sentence:** the identity gate confirmed the seated card is the ~1 GB kit
card, and the stock driver then refused it at `sdmmc_init_spi_crc` —
`ESP_ERR_NOT_SUPPORTED` (262) — so the chain stopped at stage 2 with no format and no write,
which this task's criteria accept as a named-stage stop.

### Card identity (the gate)

| Item | Measured |
| --- | --- |
| CID | `05 00 0c 61 73 64 66 67 22 00 00 13 a4 01 9a 00`, serial `0x000013a4` |
| CSD | v1 (SDSC), 960 MB (1,006,632,960 B) |
| OCR | `80 ff 80 00`, CCS=0 → byte addressing |
| Gate | **PASS** (identity + geometry) |

Read over the raw SPI path before anything else in the run; the firmware refuses to go
further unless the serial matches the kit card, so the 512 GB Recalbox card cannot be
formatted by this build even if it is the one seated.

### Init result and failure shape

```
V sdspi_transaction: r1 = 0x05 ... cmd=52          <- SDIO probe, tolerated
V sdspi_transaction: r1 = 0x00 ... cmd=0
V sdspi_transaction: r1 = 0x01 hw_cmd.r[0]=0xaa0100f0   <- CMD8 R7 echoes 0x1AA
D sdspi_host: sdspi_host_start_command: cmd=5 error=0x107   <- CMD5, "not IO card"
V sdspi_transaction: r1 = 0x05 ... cmd=59          <- CMD59 illegal after CMD5
E sdmmc_sd: sdmmc_init_spi_crc: sdmmc_send_cmd_crc_on_off returned 0x106
```

`sdmmc_card_init` / `fold_one_init` both report `ESP_ERR_NOT_SUPPORTED (262)`. All three
mount attempts (plain, retry, `format_if_mount_failed=true`) fail at the identical stage,
so there was no mount handle to format through and the firmware logged
`KIT_STOP ... No format, no write.`

### Format method

None ran. The build always uses the explicit `esp_vfs_fat_sdcard_format()` (never
`format_if_mount_failed` as the reported method), and it was never reached. The card is
untouched: no format call, no write command.

### Hashes

Not produced — there is no file to hash. The written pattern's expected hash, computed
independently on the host, is `9fdfcb84758db72e1c98ada8d10a47a7cb9945ebf3afd47d0a5fbe2914071dbd`
for the 128 KiB payload (`bench/kitcard0240/expected_pattern.py`); it is recorded so that a
later run on a working card has a reference value, but it is **not** evidence of a write.

### Board pre-state and restore

- Pre-state (before any flash): accepted 800x600 build, framebuffer
  `sha256 204578cc...bd306`, MATCH.
- Restored afterwards to the same build and verified from the board's own report: same
  hash, MATCH, `362 vsync in 5.998 s`. The kit card stays in the holder.

### Copilot review

Three passes on the pushed diff: BLOCKER + 2 MEDIUM → FIX; my first CID-parse fix was
itself off by one and the reviewer caught it → FIX; final pass "all three RESOLVED, no new
findings" → **SHIP** at `f2fcbe9`. The branch tip is `59ae567` (that verdict commit plus the
bench logs).

### What this means for 0230 rung 3

The pre-sacrifice does not rescue this card. 0239's 14/14 and its successful mount were
measured on the 512 GB SR512 card; the same firmware on the kit card dies at the same
CMD5→CMD59 step 0229 documented. Options are a card that tolerates the SDIO probe, or a
raw-sector writer that bypasses `sdmmc_init_spi_crc` — a separate task, not this one.
