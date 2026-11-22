---
task_id: '0238'
protocol_version: 1.3.0
status: staged
iteration: 2
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0238-cs-framing
  sha: e9f63aeefb8a9cb4c4593d741b98991e136d7936
  run: none
  run_note: >-
    cabin-play carries no .github/workflows directory (ls .github/workflows fails;
    git ls-tree -r --name-only origin/hermes/0238-cs-framing .github/workflows is
    empty), so no CI run exists or is expected for this branch. ntfy not
    applicable: the task names no topic, and nothing was published.
  files:
    - living-frame/firmware/n16r8-sdprobe/src/main.c
    - living-frame/firmware/n16r8-sdprobe/README.md
    - living-frame/firmware/n16r8-sdprobe/logs/14-0238-control-13resets.txt
    - living-frame/firmware/n16r8-sdprobe/logs/15-0238-sacrificial-frame-13resets.txt
    - living-frame/firmware/n16r8-sdprobe/logs/16-0238-control-table.txt
    - living-frame/firmware/n16r8-sdprobe/logs/17-0238-sacrificial-frame-table.txt
    - living-frame/firmware/n16r8-sdprobe/logs/18-0238-control-harness.log
    - living-frame/firmware/n16r8-sdprobe/logs/19-0238-variant1-harness.log
    - living-frame/firmware/n16r8-sdprobe/logs/20-0238-board-pre-state.txt
    - living-frame/firmware/n16r8-sdprobe/logs/21-0238-board-restored-800x600.txt
  board_restored: >-
    800x600 accepted build, report partition: mode 800x600, framebuffer sha256
    204578cc... == tool sha256, MATCH, 362 vsync in 5.998 s -> 60.35 Hz, 14473
    fills -> 40.0 per frame. esptool write_flash reported "Hash of data verified."
  peer_review: "Copilot (Herdr pane t17), 1 pass, no findings, Verdict: SHIP"
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-22T04:01:08+00:00"
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
  One bench session, 2026-11-21 21:00-22:05 MST (2026-11-22 04:00-05:05 UTC),
  off-peak (PEAK: OFF-PEAK 04:00 UTC), PROTOCOL MATCH 1.3.0. Picked up at
  39b832d (status in_progress, iteration 1 -> 2). Cost fields are `unknown`
  because this harness exposes no per-call usage to the cron session; they are
  not estimated.

  TWO DECLARED DEVIATIONS, both in the tooling, neither touching the card:

  (1) NVS namespace. The 0234/0236 table lived in the NVS namespace
  "sdprobe0234", keyed by a hash of the build stamp. On the first 0238 flash the
  new build reported FIRSTPASS_BOOT: 16 with 15 rows carrying the CONTROL's
  signatures, under a build stamp that did not match -- i.e. the table was
  handed over to a build it did not belong to, which would have reported the
  variant's rate as its predecessor's. 0238 now opens its own namespace
  "sdprobe0238" (one line, declared in the source comment). No NVS erase: the
  accepted 800x600 build's own settings are untouched. Both arms were re-run
  from empty tables after the change, and every number in the reply body is from
  those clean runs. The first (pre-change) pair of runs measured per-boot the
  same 13/13 for rung 1 and 0/14 for the control, but its table rows are not
  quoted because they were polluted -- see logs/19 for the harness log of the
  clean run.

  (2) A self-reboot burst after the first flash of the rung-1 image: the board
  rebooted itself 14 times inside ~45 s (each boot appending a slot with a
  FAILED first pass, 0x3f/0x47 -- the control's signatures), then settled into
  the 13 clean reset boots measured. It is not a host reset burst: each loop
  step makes one esptool connection and the esptool logs show one "Connecting"
  and no retries; the host issued one reset in that window. A re-flash of the
  IDENTICAL image afterwards produced exactly one boot, and that boot was clean
  (FIRSTPASS_BOOT: 25, first_pass_ok 13/24 -> 15/24 as the oldest row aged out).
  Mechanism undetermined, and it is the reason the verdict is scoped to
  host-driven resets rather than stated flatly: 14 of the 27 boots recorded for
  this build did not get the benefit. Recorded, not explained.

  Constraint compliance: nothing was written to the card -- the probe's SD
  access is raw init (CMD0/CMD8/CMD55/ACMD41/CMD58/CMD59/CMD13) plus CMD17/CMD9/
  CMD10 reads, and its only filesystem calls are opendir/readdir and
  f_open(..., "rb"); there is no f_write, f_unlink or f_mkfs anywhere in the
  source (grep quoted in the README). One honest qualification: the probe does
  mount the FAT32 volume through esp_vfs_fat_sdspi_mount with
  format_if_mount_failed=false, and that VFS mount is read-write by default --
  it is the base probe's (0234/0236) behaviour, unchanged by this task, and no
  write followed it. Making the mount itself read-only needs a FatFS
  FF_FS_READONLY change, which I did not make because it would break
  comparability with 0234/0236; declared rather than silently left out.
  Framing only: SPI clock unchanged at 400 kHz, SD_FIRSTPASS_DELAY_MS stayed 0,
  pull-ups unchanged. The 3.3 V rail was not metered (that is Stephen's task).

  Card in the holder: SanDisk SR512 512 GB, label RECALBOX (0234's card, the
  same one 0236 measured) -- confirmed from the CSD/CID the probe reads, not
  assumed. Rung 2 was not built or flashed because rung 1 cured the first pass.

  Peer review: the diff was put to Copilot (Herdr pane t17) before anything was
  flashed. One pass, "No findings.", Verdict: SHIP. It independently re-ran the
  range guard (-DSD_FRAMING_VARIANT=3 fails the build, exit 1) and re-read the
  CS framing of both variants. No GitHub PR exists for this branch (cabin-play
  has no workflows and the task asked for a pushed branch), so this section is
  the review record.
---

# 0238 — a sacrificial frame after the dummy clocks makes the first init pass reliable

| Variant | `-DSD_FRAMING_VARIANT=` | First pass | READY |
| --- | --- | --- | --- |
| Control (0236 arrangement) | 0 | **0/13** | 12/13 (one boot never answered at all) |
| Rung 1 — one sacrificial CMD0 after the dummy clocks | 1 | **13/13** | 13/13, every one on attempt 1 |
| Rung 2 — CS high >= 1 ms, then CMD0 | 2 | not run | rung 1 cured the first pass |

**Verdict: a single sacrificial CMD0 after the 74+ dummy clocks makes the first
init pass reliable — 13/13 against a 0/13 control — so the CS re-assert rung was
not needed.** Scoped honestly: 13/13 on host-driven resets; the caveat below
records 14 boots of the same build, right after its first flash, that did not get
the benefit.

## Control

0/13 first passes over 13 host resets (`esptool --after hard_reset`, 42 s settle,
report partition read back each time). 12 of the 13 reached READY on **attempt
2**; reset 1 never answered (`CMD0_first=0x00`, raw all-`ff`, `RAW=FAILED`,
`ok_attempt=0`). Every READY boot's first CMD0 was 0x3f or 0x47 and never 0x01.
Replicated in an earlier 13-reset run of the same configuration (0/13, 14 boots
including the flash boot). This reproduces 0236's ~0/13, so the setup is the same
setup: the control is live, not vacuous.

## Rung 1 — the sacrifice absorbs the shift, and the pairing is the evidence

Every one of the 13 resets: the throwaway frame comes back with the control's
broken shape, and the frame immediately after it is clean.

```
SACRIFICE CMD0  arg=0x00000000: R1=0x3f payload[0]=ff ff ff c1 3f ff ff ff
CMD0        arg=0x00000000: R1=0x01 payload[0]=ff 01 ff ff ff ff ff ff
CMD8        arg=0x000001aa: R1=0x01 payload[4]=ff 01 00 00 01 aa ff ff ff ff ff ff
```

(Second reset, the other control shape: `SACRIFICE CMD0 ... R1=0x47 payload[0]=
ff ff f8 47 ff ff ff ff`, then `CMD0 ... R1=0x01 payload[0]=ff 01 ...`.)

The control, same two positions, all 13 resets:

```
CMD0  arg=0x00000000: R1=0x3f payload[0]=ff ff ff c1 3f ff ff ff   (or 0x47: ff ff f8 47)
CMD8  arg=0x000001aa: R1=0x00 payload[4]=ff 00 00 00 01 aa ff ff ff ff ff ff
```

So the shift is real, it is consumed by exactly one throwaway transaction, and
the real attempt that follows is in frame: CMD0 answers `ff 01` (R1=0x01 in the
first Ncr byte) and CMD8's R7 arrives `ff 01 00 00 01 aa` with the echo in place.
`ok_attempt=1` in the persisted table for all 13 boots.

## Rung 2

Not built, not flashed: it is gated on rung 1 failing ("run rung 2 only if rung 1
does not cure the first pass"), and rung 1 cured it. The code path exists and is
range-guarded, but it carries no measured result and none is claimed.

## Board state found, and restored

Found on the accepted 800x600 build: report partition held `mode 800x600`,
framebuffer `sha256 204578cc…`, `MATCH`, `362 vsync in 5.998 s`
(`logs/20-0238-board-pre-state.txt`). Not the wrong-image state 0236 had to
repair.

Restored afterwards from `.pio/build/n16r8-photo-800x600` (bootloader 0x0,
partitions 0x8000, app 0x10000), `Hash of data verified.`, and verified from the
board's own report: `mode 800x600`, `sha256 204578cc… == tool 204578cc…`,
`MATCH`, `362 vsync in 5.998 s -> 60.35 Hz`, `14473 fills -> 40.0 per frame`
(`logs/21-0238-board-restored-800x600.txt`).

## Where the numbers live

`logs/14-0238-control-13resets.txt` and `logs/15-0238-sacrificial-frame-13resets.txt`
are per-reset read-backs: build stamp, framing variant, slot index, the first
transaction(s) of that boot with raw rx bytes, and the running summary.
`logs/16`/`logs/17` are the full persisted tables from the final read.
`logs/18`/`logs/19` are the harness logs the loop printed live. Every figure in
this reply is from those files, read back from the board's report partition.
