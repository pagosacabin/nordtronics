---
task_id: '0234'
protocol_version: 1.3.0
status: staged
iteration: 2
claimed_by: cronrunner/8b1c9e1323c5
claimed_at: '2026-11-21T14:30:39+00:00'
completed_at: '2026-11-21T15:49:16+00:00'
proof:
  branch: hermes/0234-sd-card-retest
  sha: d17efa70e2fbe13a9b43525530b36c7f47705aa1
  run: none (no .github/workflows in cabin-play)
  files:
  - living-frame/firmware/n16r8-sdprobe/src/main.c
  - living-frame/firmware/n16r8-sdprobe/README.md
  - living-frame/firmware/n16r8-sdprobe/bench/reset-loop.sh
  - living-frame/firmware/n16r8-sdprobe/logs/06-0234-new-card-report.txt
  - living-frame/firmware/n16r8-sdprobe/logs/06-0234-new-card-report.bin
  - living-frame/firmware/n16r8-sdprobe/logs/07-frame-board-restored-800x600-0234.txt
  - living-frame/firmware/n16r8-sdprobe/logs/08-first-pass-table-14-resets.txt
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
  rates_used: 'DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output'
  cost_usd: unknown
notes: "FILED 2026-11-21 07:18 MST. Stephen seated the new card in the frame board's\nholder this morning (~07:17 MST) and said so in chat — that was the filing\ntrigger agreed Nov 20. Card brand/size unknown until probed (step 1 reads\nthe CID). Stephen adds (07:19): it's a 512 MB card, one he hadn't used for\na bit — expect SDSC; check the partition type before assuming FAT32.\nScratch designation: NOT stated — treat the card as READ-ONLY;\nthe step-5 write test stays locked unless Stephen designates it scratch,\nwhich Juno will annotate onto the task file if he does.\n\nPICKED UP 2026-11-21 07:30 MST by cronrunner (rule 16: the file arrived with\n`status: pending`, which is not one of the README's values\n(inbox|in_progress|staged|verified|failed); normalized to `in_progress` at\npickup so the state machine has a valid state, and declared here).\nLowest-actionable check: 0176 (inbox) is `status: superseded` (\"Do not pick\nup\"); 0216 and 0230 in active/ are both decision-blocked (0216:\
  \ DECISION OWED\n+ step 3 needs Stephen's hands; 0230: no talking camera module, unwritten\ncard) and were left completely untouched. 0234 is therefore the lowest\nactionable task (0235 is higher and is prep for it).\n\n  STAGED 2026-11-21 08:47 MST. Card = SanDisk 512 GB (not the 512 MB the filing\n  note expected): CSD v2 488 GiB, PNM SR512, PSN e4845e78, mfg 2024/9, carrying a\n  Raspberry-Pi Recalbox image (3 partitions, FAT32 RECALBOX volume, 23 root\n  entries). Stock ESP-IDF driver now initialises (400k/100k/20 MHz) AND mounts\n  (esp_vfs_fat_sdspi_mount at 400 kHz, read-only) where the old card failed at\n  sdmmc_init_spi_crc -- so the CMD59/CMD5 quirk followed the old card. The\n  first-pass flakiness did NOT follow it: 14/14 boots on the shipped build (and\n  13/13 on the intermediate build) failed the first raw init pass, with a junk\n  first CMD0 (0x3f/0x47) and CMD8 R1=0x00, READY on attempt 2 every time --\n  board-side. Step 5 (write test) skipped: card not designated scratch;\
  \ nothing\n  written to the card, mount used format_if_mount_failed=false and only read\n  APIs. Correction to 0229 filed with the reply: its FAT32 root walk omitted the\n  partition base LBA, so \"root empty\" on the kit card was a probe bug, not\n  evidence. Board restored byte-identical (read-back sha256 a2829d61...cdf0fc =\n  the accepted 800x600 firmware.bin and the pre-flash read-back), and its own\n  report shows mode 800x600, framebuffer sha256 204578cc..., MATCH, 362 vsync in\n  5.998 s. CI: cabin-play has no .github/workflows at all (ls -> No such file or\n  directory; git ls-tree over the branch -> empty), so no run exists or is\n  expected, and there is no ntfy receipt because no workflow builds or publishes\n  anything from this repo. Copilot peer review (Herdr t17) took three passes:\n  3 findings (MEDIUM missing base LBA in the new MBR diagnostic; LOW CID\n  alignment/MDT/PSN; LOW capacity-probe underflow) -> fixed -> pass 2 confirmed 2\n  resolved and finding 3 incomplete\
  \ -> fixed -> pass 3 verdict SHIP. Cost fields\n  are unknown: this harness exposes no per-call usage to the cron session (not\n  estimated). Re-probing note: the first-pass counter lives in the nvs partition\n  (namespace sdprobe0234) and is keyed by build stamp, so the accepted build's\n  own NVS settings were never erased.\n"
---

## Result — 2026-11-21, new card seated, probe re-run on the frame board

Board: YD-ESP32-23 / N16R8, ESP32-S3, MAC `c4:9e:7e:08:6d:f4`, currently on the
accepted 800x600 build (restored and verified, below). One flash of the extended
0229 probe, ~14 boots, one restore.

### 1. Card identity — it is not a 512 MB card

Read from the card itself (CMD9/CMD10), CID at the spec alignment (all five PNM
bytes printable; the other alignment yields a non-printable PNM):

- `MID=0x03` (**SanDisk**), `OID="SD"`, `PNM="SR512"`, `PRV=8.6`,
  `PSN=e4845e78`, manufacture date **2024/9**.
- CSD v2 → SDHC/SDXC class, **capacity 511,868,665,856 B = 488 GiB ≈ 512 GB**.
  Stephen's "512 MB" was most likely the last three digits of the model name.
- MBR, three partitions: `0x0c` **FAT32 `RECALBOX`** at LBA 2048
  (6,301,696 sectors ≈ 3.0 GiB), `0x07` at LBA 6,305,792 (991,338,497 sectors),
  `0x83` at LBA 997,646,336 (2,097,152 sectors). This is a **Raspberry-Pi
  Recalbox image** written for a 512 GB card — not a blank card.
- Root directory: **23 entries** (`CRT`, `START4.ELF`, `FIXUP4.DAT`,
  `config.txt`, `overlays`, `BOOT`, `CMDLINE.TXT`, …).
- First file read: `/sdcard/START4.ELF`, 2,258,496 B, first 32 bytes
  `7f 45 4c 46 01 01 01 00 …` (ELF magic, so the content is real data).
- Read stability: the root sector read **identical on 6 consecutive reads**;
  LBA 1,000,000 and the CSD's last LBA (999,743,487) both read.

### 2. Raw command ladder

| Measurement | New card |
| --- | --- |
| CMD0 repeatability | **10/10** `R1=0x01` (idle), on both probes |
| CMD8 | R7 echoes `0x1AA` (v2.00+), `R1=0x01` on the clean attempt |
| ACMD41 | READY in **38–59 ms** |
| CMD58 OCR | `c0 ff 80 00`, **CCS=1 → block addressing** (SDHC/SDXC) |
| CSD | v2, 488 GiB |
| CID | MID 0x03 / "SR512" / PSN e4845e78 / 2024-09 |

### 3. First-pass behaviour — 14 resets, board-driven counter

The probe now keeps a first-pass slot table in NVS (one slot per boot: the very
first CMD0/CMD8 responses before any retry, the attempt that reached READY, and
the ACMD41 time). `bench/reset-loop.sh` drove 12 resets and read the report
after each; with the two boot-time runs that is **14 boots on the shipped
build, and 13 on the intermediate build — 27 of 27 first passes failed**:

```
FIRSTPASS_SLOTS: 14 (of 24 kept), build_id=3473dcd9
  boot  1: CMD0_first=0x3f CMD8_r1=0x00 RAW=READY ok_attempt=2 acmd41=58 ms
  ...
  boot 14: CMD0_first=0x47 CMD8_r1=0x00 RAW=READY ok_attempt=2 acmd41=38 ms
FIRSTPASS_SUMMARY: raw_init_ready=14/14 first_pass_ok=0/14
```

The signature is different from 0229's old card: there the first CMD8 answered
`R1=0x05`; here the first CMD0 answers **`0x3f`/`0x47` (junk, not a valid R1)**
and the first CMD8 answers `R1=0x00`, then attempt 2 is clean every time. Same
shape — the first SPI exchange after a reset is not understood and the second is
— so **the flakiness stayed with the board**; it did not follow the card.

### 4. The stock driver — the question 0229 left open

| Measurement | Old card (0229) | New card (0234) |
| --- | --- | --- |
| CMD59 in idle state | `R1=0x05` **rejected** after CMD5 | `R1=0x01` **ACCEPTED** (idle bit only) |
| CMD59 after CMD52+CMD5 replay | 0x05 (poisoned) | **0x01** (CMD5 itself answers 0x05 = no response; harmless here) |
| CMD59 with no CMD5 (control) | 0x01 | 0x01 |
| `sdmmc_card_init` (SPI) | FAIL `ESP_ERR_NOT_SUPPORTED` at 400 kHz / 100 kHz / 20 MHz | **OK at 400 kHz, 100 kHz and 20 MHz** |
| `esp_vfs_fat_sdspi_mount` | not reachable | **OK at 400 kHz** — 23 root entries listed, `START4.ELF` opened `"rb"` |

The CMD59-rejected-in-idle-state quirk and the resulting `sdmmc_init_spi_crc`
failure **followed the old card**. On this card the unmodified ESP-IDF driver
both initialises and mounts.

Step 5 (write test) **skipped**: the notes designate the card read-only unless
Stephen names it scratch, and he has not. Nothing was written to the card — the
mount was made with `format_if_mount_failed=false` and only `opendir`/`readdir`
and `fopen(..., "rb")` were called.

### Working configuration for 0230's write path

SPI mode (`sdspi`/`sdmmc` SPI host), CS GPIO10, SCK 12 / MOSI 11 / MISO 13,
400 kHz through `esp_vfs_fat_sdspi_mount`; 20 MHz `sdmmc_card_init` also
verified. What 0230 still needs is Stephen's designation of a scratch card —
writing to this one would overwrite a Recalbox install on a 512 GB card.

### Correction to 0229 (this changes one of its conclusions)

`raw_inspect()` computed the FAT32 root LBA from partition-relative offsets and
**never added the partition's base LBA**, so on a card whose partition starts at
2048 it read the root 2048 sectors too low (12320 instead of 14368) and got
zeros — reported by 0229 as "Root directory … first sector all `0x00`: empty".
With `part_lba` added the same read lists the directory, and the raw path now
agrees with the driver path (23 entries, ELF magic). **0229's "root empty" line
is a probe bug, not evidence that the kit card was blank.** Declared here
because it is a correction to a filed finding, not a new measurement.

### Board restored

The accepted 800x600 build is back on the board, byte-identical, verified twice:
a read-back of `0x10000..0xB5000` is
`a2829d610cd2be6fa7455b18cc3e1b378248864f975b370e2eb949e862cdf0fc`, equal to
`.pio/build/n16r8-photo-800x600/firmware.bin` and to the pre-flash read-back; and
the board's own report says `mode 800x600`, framebuffer `sha256 204578cc…`,
`MATCH`, `362 vsync in 5.998 s` (60.36 Hz) — the 0228 proof reproduced.

### Repo pointer and review

- Branch `hermes/0234-sd-card-retest` @ `d17efa70e2fbe13a9b43525530b36c7f47705aa1` in pagosacabin/cabin-play
  (pushed over SSH; `gh.py` writes to cabin-play 403 as before).
- Files: `living-frame/firmware/n16r8-sdprobe/src/main.c` (extended probe),
  `README.md` (0234 section incl. the 0229 correction), `bench/reset-loop.sh`,
  `logs/06-0234-new-card-report.{txt,bin}` (full report read back from the
  board), `logs/07-frame-board-restored-800x600-0234.txt`,
  `logs/08-first-pass-table-14-resets.txt`.
- **CI: no run exists or is expected.** cabin-play has no `.github/workflows`
  directory at all: `ls .github/workflows/` → "No such file or directory", and
  `git ls-tree -r --name-only origin/hermes/0234-sd-card-retest .github/workflows`
  → empty. Enumerated, not assumed. No ntfy receipt: nothing is built or
  published by this repo (no workflow), and the deliverable is bench evidence on
  a local board.
- Copilot peer review (Herdr pane t17), three passes: pass 1 found 3 findings
  (MEDIUM: the new MBR diagnostic repeated the missing-base-LBA bug; LOW: CID
  alignment chosen from one byte + wrong MDT/PSN offsets; LOW: capacity probe
  could underflow on an unparsed CSD). All three fixed; pass 2 confirmed 1 and 2
  RESOLVED and found the third incomplete (a non-0xFF CSD still sets the
  identified flag) — fixed by requiring a decoded capacity of at least 1 KiB;
  pass 3 verdict: SHIP.

### What blocks the next rung

Nothing on the card/driver path — 0230 can mount and read. Two things remain
Stephen's: (a) name a scratch card before any write path is exercised (this
card is a 512 GB Recalbox install, not the 512 MB the filing note assumed), and
(b) 0229's rung 2 meter readings are still owed, now with a sharper target: the
first-exchange-after-reset failure is board-side and identical on both cards.
