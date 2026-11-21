---
task_id: "0234"
protocol_version: 1.3.0
status: pending
iteration: 1
proof: []
notes: |
  FILED 2026-11-21 07:18 MST. Stephen seated the new card in the frame board's
  holder this morning (~07:17 MST) and said so in chat — that was the filing
  trigger agreed Nov 20. Card brand/size unknown until probed (step 1 reads
  the CID). Stephen adds (07:19): it's a 512 MB card, one he hadn't used for
  a bit — expect SDSC; check the partition type before assuming FAT32.
  Scratch designation: NOT stated — treat the card as READ-ONLY;
  the step-5 write test stays locked unless Stephen designates it scratch,
  which Juno will annotate onto the task file if he does.
---

expect-reply-within: 6h

# 0234 — SD retest with the new card

## Assignment

Hermes does this task. Bench firmware work stays Hermes's (protocol 1.3.0). Copilot peer-reviews any committed code change, routed by Hermes via Herdr; Copilot is a reviewer, not the proof — Juno verifies independently on top.

## Context

- Task 0229 (branch `hermes/0229-sd-diagnosis` @ 82a76ce, cabin-play) found: Stephen's resolder fixed the wiring — the kit card answers at command level (CMD0 10/10, CMD8 R7=0x1AA, ACMD41 READY 12–19 ms, CSD 1 GB SDSC, FAT32 959 MB, root empty). But ESP-IDF's `sdmmc_card_init` fails with this card: the driver's SDIO CMD5 probe poisons it (CMD59 → 0x05; without CMD5, CMD59 → 0x01 accepted). Also: the first init pass after a reset failed 11 of 12 runs before succeeding, and the kit card's root was empty — the Testy/testy.txt from Stephen's laptop test is not on it.
- Stephen's call (2026-11-20 21:23): "Something is still not correct. I will put a new card in in the morning." One variable changes: the card. This task re-runs the 0229 probe against the new card and answers which faults followed the old card and which stay with the board.
- The frame board is the same wired N16R8 board (YD-ESP32-23, ESP32-S3, MAC c4:9e:7e:08:6d:f4); SD on SPI, SCK GPIO12 / MOSI GPIO11 / MISO GPIO13 / CS GPIO10. The accepted 800x600 gamma build is on the board now and must be restored after, byte-identical, verified from the board's own report (framebuffer sha256 204578cc…).
- Reuse the 0229 probe firmware (`living-frame/firmware/n16r8-sdprobe/` on the 0229 branch) — extend it, don't rewrite it.

## Task

With the new card seated, run in order and report each with numbers:

1. **Identify the card.** CMD9/CMD10: report CID (manufacturer, name, serial), CSD size, SDSC vs SDHC/SDXC. This establishes what card Stephen actually seated.
2. **Raw command level.** CMD0 success count over 10 tries, CMD8 R7, ACMD41 time-to-ready, CMD58 OCR — same measurements as 0229, so the two cards compare directly.
3. **First-pass behaviour.** Across at least 10 board resets, record whether the first init pass succeeds or fails, per attempt. 0229's old card failed the first pass 11 of 12 — does that follow the card or stay with the board?
4. **The stock driver.** `sdmmc_card_init` + mount at 400 kHz, then at the default clock. The question 0229 left open: does this card tolerate the driver's CMD5 probe so the unmodified ESP-IDF driver works? If the mount succeeds, list the root directory and read the first bytes of any file present.
5. **Write test — ONLY if Stephen designated this card scratch at seating** (Juno records the designation in the notes at filing; if the notes say read-only, skip this step and say so). Write a small test file, read it back, compare hashes.

## Success criteria

- The reply states, per measurement, whether the new card behaves like the old one or differently — card identity, raw-level numbers, first-pass rate, and the stock-driver verdict.
- If the driver mounts: the working configuration (mode, clock) is stated — task 0230's write path builds on it.
- If the driver still fails: the CMD5 replay control from 0229 is re-run against this card before any mechanism is claimed.

## Constraints

- The one wired board only. Do not reflash the Heltec.
- No format. No write to the card unless Stephen designated it scratch (see step 5).
- Preserve the accepted 800x600 display build: restore it after any probe flash, byte-identical, verified from the board's report partition as in 0229/0230.
- No new purchases.

## Proof

- Firmware changes + device logs committed to cabin-play under the living-frame path — branch SHA on origin. Push via SSH (gh.py writes to cabin-play return 403).
- The firmware's reported numbers in the reply via the flash read-back report, as in 0229.
- Copilot peer-review step on any committed code change; if the pane has signed out, say so in the reply rather than skipping silently.

## Reply format

Standard mailbox reply format: card identity, the ladder as actually walked with numbers per step, the old-card vs new-card comparison, the working configuration if one works, the repo pointer (branch SHA), and what blocks the next rung, if anything.
