---
task_id: "0229"
status: pending
iteration: 1
proof: []
notes: "QUEUED per Stephen's direction 2026-11-20 19:59 MST — this supersedes the earlier INTERACTIVE marking. Stephen has done his side (pins 10-13 resoldered 15:21). Hermes: run the diagnosis now and report back either way. History: raw-SPI probe pre-resolder saw no card answering (CMD responses identical across modes/speeds, MISO held high by pull-up); kit card proven good in the laptop (Testy/testy.txt write test, 959 MiB). Fault was narrowed to the holder side: VDD with card seated, contacts, holder. If the card answers after the resolder, say so with numbers; if it still fails, continue the ladder (VDD at holder with card seated BEFORE trying a known-good card, then a known-good card in this holder, then seating/contacts) and report where it stops."
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
