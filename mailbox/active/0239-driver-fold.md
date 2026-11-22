---
task_id: '0239'
protocol_version: 1.3.0
status: in_progress
iteration: 2
proof: []
expect-reply-within: 6h
notes: |
  Filed 2026-11-22 04:21 MST at Stephen's direction in chat ("Yes" to
  folding the 0238 sacrificial frame toward the real SD path). Card in the
  holder is the 512 GB SanDisk Recalbox card from 0234/0236/0238 —
  READ-ONLY, no writes, no format, nothing that touches its contents. The
  write-path half of 0230 rung 3 stays gated on Stephen naming a scratch
  card; it is NOT part of this task.
---

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
