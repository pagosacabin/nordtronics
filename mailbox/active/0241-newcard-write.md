---
task_id: '0241'
status: in_progress
iteration: 2
proof: []
expect-reply-within: 6h
notes: |
  Filed 2026-11-22 07:15 MST at Stephen's direction in chat ("try to have
  him try this new card. I put a new card in."). Stephen seated a new card
  for the write test after 0240 stopped at driver init on the kit card.
  The format is authorized for the card he seated for this test — with one
  hard exception: the 512 GB Recalbox card is never scratch (step 1 gate).
---

# 0241 — New card: format, mount, write, read back

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
