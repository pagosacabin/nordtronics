---
task_id: '0240'
status: in_progress
iteration: 2
proof: []
expect-reply-within: 6h
notes: |
  Filed 2026-11-22 06:30 MST at Stephen's direction in chat: he designated
  the 1 GB kit card as scratch and confirmed "the erase is fine." This is
  the write-path test 0230 rung 3 has been waiting on. DESTRUCTIVE to the
  kit card only — the identity check in step 1 is the guard that keeps the
  512 GB Recalbox card safe.
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
