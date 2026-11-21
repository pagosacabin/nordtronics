---
task_id: "0229"
protocol_version: 1.3.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - branch: hermes/0229-sd-diagnosis
    sha: 82a76ce37b0098a532624efa22b4d81c21652c2e
  - files:
      - living-frame/firmware/n16r8-sdprobe/.gitignore
      - living-frame/firmware/n16r8-sdprobe/CMakeLists.txt
      - living-frame/firmware/n16r8-sdprobe/README.md
      - living-frame/firmware/n16r8-sdprobe/logs/01-first-run-card-answers-driver-refuses.txt
      - living-frame/firmware/n16r8-sdprobe/logs/02-driver-log-names-the-fatal-step.txt
      - living-frame/firmware/n16r8-sdprobe/logs/03-instrumented-partial.txt
      - living-frame/firmware/n16r8-sdprobe/logs/04-final-full-diagnosis.txt
      - living-frame/firmware/n16r8-sdprobe/logs/05-frame-board-restored-800x600.txt
      - living-frame/firmware/n16r8-sdprobe/partitions.csv
      - living-frame/firmware/n16r8-sdprobe/platformio.ini
      - living-frame/firmware/n16r8-sdprobe/sdkconfig.defaults
      - living-frame/firmware/n16r8-sdprobe/src/CMakeLists.txt
      - living-frame/firmware/n16r8-sdprobe/src/main.c
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
  STAGED 2026-11-20 21:05 MST (04:05 UTC, off-peak; PEAK: OFF-PEAK 03:00 UTC) — the reply body below is the report.

  STEPHEN'S ASK IS ANSWERED: the card works after his resolder. CMD0 10/10, CMD8 R7
  echoes 0x1AA, ACMD41 READY in 12-19 ms, CSD read gives a 1 GB SDSC card
  (1,006,632,960 B), FAT32 959 MB, partition 0x0c at LBA 2048, root directory reads
  empty. All of it read-only: no mount with write, no format, no write to any card.

  STILL FAILING, MECHANISM ISOLATED: sdmmc_card_init (ESP-IDF's own driver) dies at
  sdmmc_init_spi_crc with CMD59 = 0x05 at 400 kHz, 100 kHz and 20 MHz. A replay with
  controls shows why: this card rejects commands after an SDIO CMD5 probe
  (CMD5-only -> CMD59 0x05; CMD52-only -> CMD59 0x01; no probe -> CMD59 0x01). The
  driver always probes SDIO first, so the driver cannot be used with this card
  unmodified. This is a card-firmware quirk, not a wiring fault.

  CORRECTION I MADE MID-RUN: I first read the idle-state CMD59 answer R1=0x01 as a
  rejection. R1 bit0 is the "in idle state" bit, not an error — 0x01 means ACCEPTED.
  Copilot's review caught the overclaim; the code, the verdict text and the README now
  say only what the data supports, and the CMD5 mechanism was re-derived by replay
  rather than asserted.

  NOT RUN, AND SAID SO: rung 2 (meter: 3.3 V at the holder's VCC with the card seated,
  continuity pad-to-pin, shorts between adjacent lines) needs Stephen's hands.
  Rung 4 (SDMMC 1-bit) ran but is INCONCLUSIVE — a card stays in SPI mode until power
  is cycled, so the test could not be fair; it is reported as such, not as a fault.
  Rung 3's software half only: internal pull-ups change nothing and are not needed for
  these reads.

  OPEN FLAG FOR STEPHEN: the kit card's root directory is empty — the
  Testy/testy.txt from the laptop test is not on this card. Either that test was on a
  different card or this one has been reformatted.

  DECISION OWED (0230's write path): (a) use a card that tolerates CMD5, (b) bypass
  sdmmc_init_spi_crc (own block I/O + FAT writer, or Arduino SD which ignores a failed
  CMD59), or (c) keep this card read-only. No write test was run because no scratch
  card was designated.

  PROOF: branch hermes/0229-sd-diagnosis in pagosacabin/cabin-play at
  82a76ce37b0098a532624efa22b4d81c21652c2e (git ls-remote --heads origin equals it).
  run: none — ENUMERATED, not assumed: cabin-play has no .github/workflows directory at
  all (`git ls-tree -r --name-only 82a76ce .github | wc -l` = 0), so no workflow can
  trigger on any branch. No ntfy receipt for the same reason: nothing builds in CI, and
  the task's proof section names a branch SHA plus device report paths, not a topic.
  The branch is based on the 0230 tip (6394c11) because main carries 0 files under
  living-frame/firmware/n16r8-*; the reply's file list is the branch diff against that
  base, so a verifier diffing against main will see the whole n16r8 tree as branch-only.

  COPILOT: five review passes on pane t17 (ea58165 -> 638fa87 -> 82a76ce), nine findings
  all fixed, final "No findings — Verdict: SHIP" at 82a76ce. The findings list is in the
  reply body.

  BOARD STATE: accepted 800x600 gamma build restored and verified from the board's own
  report (mode 800x600, framebuffer sha256 204578cc… MATCH, 362 vsync in 5.998 s). Six
  probe flashes, four restores; the last restore is the one quoted.

  Cost: this harness exposes no per-call usage to the cron session, so the token and
  cost fields are `unknown` rather than estimated.
---


expect-reply-within: 6h

# 0229 — SD card diagnosis on the N16R8 frame board

## Assignment

Hermes does this task. Bench firmware work stays Hermes's (protocol 1.3.0); the meter work is Stephen's hands at the bench with Hermes directing and recording. Copilot peer-reviews any committed code change, routed by Hermes via Herdr; Copilot is a reviewer, not the proof — Juno verifies independently on top.

## Context

- Task 0225 probed the wired N16R8 board (YD-ESP32-23, ESP32-S3, MAC c4:9e:7e:08:6d:f4). Flash and PSRAM are proven good. **The SD card failed**: `sdmmc_card_init` returned ESP_ERR_NOT_SUPPORTED at the default clock and at 400 kHz; GPIO14 (card-detect) read high. No mount, format, or writes were attempted — the card is untouched.
- The SD holder is wired on SPI per the pin map Stephen agreed and wired himself: **SCK GPIO12, MOSI GPIO11, MISO GPIO13, CS GPIO10, GPIO14 spare/card-detect**. Copilot's 0225 caveat applies: the probe proved those pads are working GPIOs, NOT that the silkscreen labels sit on the right header positions — wire identity is unproven until metered.
- The card in the holder came with the kit; its format and contents are unknown. Stephen has designated no scratch card yet — ask him at the bench which card is the known-good/scratch one before any write test.
- The FTDI bridge console is dead (0225 finding), so firmware reporting goes through the `report` flash partition + esptool read-back, as in 0225–0228.
- Stephen's call (2026-11-20): the display work is closed — the 800x600 gamma build is accepted as the gift's look (Bethany's judgment: the dither texture gives it a neat quality; a fun photo-booth gift, not a high-end piece). SD is the next rung because everything after it (ESP32-CAM photo transfer, task 0230) needs a working card on this board.

## Task

Work the ladder in order; stop at the rung that fixes it and report which one it was:

1. **Known-good card baseline.** With a card Stephen designates as known-good/scratch (FAT32), seated firmly, retry card init at 400 kHz and at the default clock. Report the exact error codes.
2. **Power and continuity (meter, Stephen's hands).** 3.3 V measured at the holder's VCC pin with the card inserted and init attempted. Continuity from each holder pad to its ESP32 pin (SCK/MOSI/MISO/CS), and a shorts check between adjacent lines.
3. **Pull-ups.** SD in SPI mode needs pull-ups (~10 kΩ) on CS and MISO at minimum. If the holder/breadboard lacks them, add them and retry init.
4. **Alternate host mode.** If SPI still refuses, try SDMMC 1-bit mode on the same pins (the S3's GPIO matrix allows it) as a fork — a card that answers in 1-bit SDMMC but not SPI points at the SPI path/wiring; a card that answers in neither points at power, the holder, or the card.
5. **Swap isolation.** If a second card or second holder is on hand, swap one variable at a time.

If a mount succeeds at any rung: list the root directory, read the first bytes of a file, and report the card's size and CID from the firmware report path. A write test happens ONLY on the card Stephen designated scratch: write a small test file, read it back, compare.

## Success criteria

- The root cause is named with evidence (which rung, what measurement), OR a working mount is demonstrated (directory listing + file read via the report path).
- If the fault is a component (holder, card, wiring), it is isolated to that component by a swap or a measurement, not by elimination alone.
- The reply states the final working configuration (mode, clock, pull-ups) if one works — task 0230 builds on it.

## Constraints

- The one wired board only. Do not reflash the Heltec.
- Do not format or write to any card Stephen has not designated as scratch. The unknown kit card stays read-only unless he says otherwise.
- Preserve the accepted 800x600 display build: if it is not yet committed, commit it before flashing any diagnostic build over the board.
- No new purchases. If a part is condemned and a replacement is needed, name it and stop — Stephen decides.

## Proof

- Diagnostic firmware + report committed to the cabin-play repo, living-frame path — branch SHA on origin. Push via SSH (gh.py writes to cabin-play return 403).
- The firmware's reported numbers (init results per rung, measurements Hermes recorded, mount output if reached) in the reply via the flash read-back report.
- Copilot peer-review step on any committed code change; if the pane has signed out, say so in the reply rather than skipping silently.

## Reply format

Standard mailbox reply format: the ladder as actually walked (each rung's result, not just the winner), the root cause or the isolated component, the working configuration if any, the repo pointer (branch SHA), and what blocks the next rung, if anything.


## Update 2026-11-20 19:59 MST — Stephen's direction: run it now

The interactive marking is lifted. Stephen's words: he has redone everything he needs to do (the resolder); Hermes is to go look at the frame board's SD, find out whether it works now, and report back — that is the whole ask.

- Re-run the SD probe/mount test on the frame board with the kit card seated. Report the result either way, with numbers.
- If it still fails, work the ladder in the original task (VDD at the holder with the card seated first) and report where it stops.
- You may flash the probe build on the frame board for this; restore the accepted 800x600 gamma build afterward, byte-identical, as in 0230.

## Reply — 2026-11-20 20:00-21:05 MST (03:00-04:05 UTC, off-peak)

**The card answers now. Stephen's resolder fixed it.** It initialises at command
level, is identified, and its contents read back — all read-only. What still
fails is ESP-IDF's own driver, and the mechanism is isolated below. No mount,
no format, no write was attempted: the kit card stays untouched, as the task
requires.

### The ladder as actually walked

| Rung | Result |
| --- | --- |
| 1 — card init, 400 kHz and default clock | **WORKS.** CMD0 answers 10/10, CMD8 returns R7 echoing `0x1AA`, ACMD41 goes READY in 12-19 ms, CMD58 OCR `0x80FF8000` (CCS=0 → byte-addressed SDSC). `sdmmc_card_init` — the library — still fails at 400 kHz, 100 kHz and 20 MHz. |
| 1b — contents (read-only, CMD17/CMD9/CMD10) | **WORKS.** SDSC 1 GB (CSD v1: 1,006,632,960 B, CID serial `0x000013a4`), MBR type `0x0c` at LBA 2048, FAT32 with 512 B sectors / 4096 B clusters / 245,020 clusters, 959 MB, label `NO NAME`. Root directory cluster 2 → LBA 3872 reads all `0x00`: **the root is empty.** The `Testy/testy.txt` from the laptop test is not on this card. |
| 2 — power and continuity (meter) | **NOT RUN — needs your hands.** 3.3 V at the holder's VCC with the card seated, continuity pad-to-pin, shorts between adjacent lines. This is the one measurement firmware cannot substitute for, and the first-pass flakiness below is exactly what it would settle. |
| 3 — pull-ups | **Software half only.** Forcing internal pull-ups on MISO/CS/MOSI changes nothing (identical responses); reads work at 400 kHz without them. Whether an external 10 kΩ is fitted needs eyes or a meter. |
| 4 — alternate host mode (SDMMC 1-bit) | **INCONCLUSIVE, and not evidence of a fault.** It fails with `ESP_ERR_TIMEOUT`, but the card had already been put into SPI mode by the steps above, and a card stays in SPI mode until its power is cycled. A meaningful 1-bit test needs a power cycle before the first command. |

### Root cause of the driver failure (isolated, not guessed)

`sdmmc_card_init` dies at `sdmmc_init_spi_crc`: CMD59 comes back `0x05` (illegal
command). The driver's own log names the step, and a replay with controls shows
what poisons it:

```
with CMD52+CMD5 first: CMD52=0x05  CMD5=0xff  CMD59=0x05   <- poisoned
without CMD5 (control):                        CMD59=0x01   <- accepted
CMD52 only,   then CMD59=0x01                               <- CMD52 is harmless
CMD5 only,    then CMD59=0x05                               <- CMD5 is the poison
```

ESP-IDF probes SDIO with CMD5 before it enables CRC. This card does not tolerate
CMD5 — after one, it answers `0x05` to everything until a CMD0 reset. That is a
card-firmware quirk, **not** a wiring fault: the card reads perfectly; the
driver's own probe locks it out of the step that follows. Note `R1=0x01` is the
idle bit, not an error, so an idle-state CMD59 answering `0x01` is an accepted
command — an earlier reading of that byte as "rejected" was wrong and is
corrected in the code and the README.

### First-pass flakiness worth naming

The first init pass after a reset fails at CMD8 with `R1=0x05` in **11 of 12**
completed runs; the next pass succeeds **12 of 12**. That is the shape of a
marginal contact or a card that wants longer after power-up, and it is what rung
2's meter readings are for. It is not a firmware bug: the probe retries and
records the attempt number.

### Working configuration for 0230 to build on

- **SPI, 400 kHz, mode 0**, CS driven manually; pins SCK 12 / MOSI 11 / MISO 13 / CS 10.
- **Byte addressing** (SDSC): CMD17 argument is `LBA × 512`.
- **Block length 512**, single-block reads (CMD17); CMD9/CMD10 return 16-byte blocks.
- Pull-ups: none needed for these reads; internal ones make no difference.
- Reads work; **writes are unproven and the vendor driver cannot be used** —
  see the decision owed below.

### Restore of the accepted display build

The accepted 800x600 gamma build was restored after each probe flash (four
restores; the last at 21:02 MST) and each restore was verified **from the board's
own report**, not from a flash command exiting 0 — `logs/05-frame-board-restored-800x600.txt`:

```
mode       : 800x600 raster from the converter (800x600)
sha256     : runtime 204578cc3339baf944345090680c7178f653be7fa79f8fcb83cf82b8ff3bd306
sha256     : MATCH - streamed framebuffer is the converted photograph
measured   : 362 vsync in 5.998 s -> 60.36 Hz refresh
```

The frozen 0228 proof reproduced, hash included (the vsync count moves by one
between boots and is measurement grain, as 0230's restore record already notes).

### Peer review (Copilot via Herdr, pane `t17`)

Five passes, nine findings, all fixed before the branch was final; the review
record is the pane's, and the findings were real each time:

1. raw-ready card could still be reported as "does not answer" — verdict split into RAW/DRIVER/CARD/READ.
2. "inspection OK" could follow a failed read — every read failure now clears it.
3. first-file read reused the directory buffer — its own buffer.
4. CMD59 diagnostic asserted a mechanism the probe had not observed — now gated on the probe.
5. root-sector read failures did not clear the status — they do.
6. the aliasing fix had landed in the wrong lister — the duplicate driver-path lister was deleted.
7. `0xFF` (no response) was counted as a rejection — now a distinct state.
8. `R1=0x01` was read as a rejection though bit0 is the idle bit — corrected, and the mechanism re-derived by replay.
9. the CMD5 isolation claim was not gated on clean controls — both controls must now be accepted responses.

**Final verdict at `82a76ce`: No findings — SHIP.**

### DECISION OWED (write path, task 0230 rung 3)

No write test was run: no scratch card was designated and the kit card is
read-only by this task. The options, cheapest first:

1. **Use a different card** that tolerates CMD5 — the driver then works unmodified.
2. **Bypass `sdmmc_init_spi_crc`** for this card (own block reads + a FAT writer,
   or the Arduino `SD` library, which ignores a failed CMD59).
3. Keep this card read-only.

Also owed: rung 2's meter readings, and Stephen naming the scratch card if a
write test is to happen at all.

### Deviations declared

- The frame board was flashed six times with probe builds and restored after each
  (last restore verified above). Each `esptool` connect resets the board; that is
  the only disturbance.
- `read_flash` of the report partition is read-only; nothing was written to the card.
- Copilot was driven through Herdr's `t17` pane, not a GitHub PR — cabin-play has
  no pull-request flow in use for this branch.
