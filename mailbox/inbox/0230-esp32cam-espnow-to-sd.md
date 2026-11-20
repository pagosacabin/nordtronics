---
task_id: "0230"
status: open
iteration: 1
proof: []
notes: ""
---

expect-reply-within: 6h

# 0230 — ESP32-CAM bring-up and ESP-NOW photo transfer to the frame board's SD card

## Assignment

Hermes does this task. Firmware work stays Hermes's (protocol 1.3.0). Copilot peer-reviews the committed code, routed by Hermes via Herdr; Copilot is a reviewer, not the proof — Juno verifies independently on top.

## Context

- Stephen's direction (2026-11-20): after the SD card (task 0229), bring up the ESP32-CAM that arrived with the N16R8 kits, so the camera can send its photos **via ESP-NOW** to the frame board, which stores them **on its SD card**. This is the photo-booth pipeline for the living-frame gift: camera captures, frame receives and keeps.
- **Dependency: task 0229 must land first** — the frame board's SD card must mount and accept writes. Do not start rung 3 until 0229 reports a working write configuration.
- The frame board: wired N16R8 ESP32-S3, MAC **c4:9e:7e:08:6d:f4**. It currently runs the accepted 800x600 gamma photo build (the gift's look — Bethany approved the dither texture as a neat quality for a fun photo-booth gift). That build stays in git; this task's receive/write work is a separate test build, and the board goes back to the accepted build at the end unless Stephen says otherwise.
- The ESP32-CAM is the kit board (AI-Thinker type with OV2640) that arrived 2026-11-20. It has no native USB — programming goes through a serial adapter. Note the bench FTDI bridge's known fault (0225): esptool flashing works through it, the live console does not. Plan reporting accordingly (flash read-back, or the CAM's own SD slot as a proof sink in rung 1).
- ESP-NOW facts the design must respect: max payload 250 bytes per packet, so a JPEG transfer needs chunking — a header (transfer id, total length, chunk count), sequenced chunks, reassembly in PSRAM on the receiver, and an end-to-end check (length + CRC or hash) before the file is written. No WiFi AP or router in the path — ESP-NOW is peer-to-peer by design.

## Task

Rungs in order, each proven before the next:

1. **Camera bring-up alone.** Flash a minimal capture build on the ESP32-CAM. Prove a real capture: save one JPEG to the CAM's own microSD slot, read it back on the laptop, and report the file size plus valid JPEG SOI/EOI markers. No radio in this rung.
2. **ESP-NOW link.** The CAM sends a small test payload to the frame board's MAC; the frame board's test build logs receipt (count + bytes) through its flash report path. Prove the link both ways if the design needs an ACK — say which in the reply.
3. **Chunked photo transfer to SD.** The CAM captures a photo, transfers it chunked over ESP-NOW; the frame board reassembles it in PSRAM, verifies length + CRC/hash against the CAM-side value, and writes it to the frame board's SD card under the 0229 working configuration. Proof: the file read back from the frame's SD card (via laptop or the frame's report path) with size and hash matching the CAM-side original.

## Success criteria

- Rung 1: a genuine OV2640 capture exists as a valid JPEG file, size and markers reported.
- Rung 2: the frame board's report shows the test payload received intact.
- Rung 3: a photo taken by the ESP32-CAM exists as a file on the frame board's SD card, hash-identical to the camera-side capture.
- The chunking protocol is committed as code + a short note (packet layout, retry/ACK rule, max photo size) so the pipeline is reproducible.

## Constraints

- No display work in this task — showing SD photos on the panel is a later rung.
- ESP-NOW only for the transfer; do not stand up a WiFi AP/STA link as the transfer path.
- Preserve the accepted 800x600 display build in git; restore the frame board to it when the task ends unless Stephen directs otherwise.
- No new purchases.

## Proof

- Firmware for both boards + the protocol note committed to the cabin-play repo, living-frame path — branch SHA on origin. Push via SSH (gh.py writes to cabin-play return 403).
- The reported numbers (JPEG size/hash at capture, transfer stats, SD file size/hash at rest) in the reply via the devices' report paths.
- Copilot peer-review step on the committed code (GitHub-bound work); if the pane has signed out, say so in the reply rather than skipping silently.

## Reply format

Standard mailbox reply format: each rung's result with its numbers, the protocol as built, the repo pointer (branch SHA), the frame board's end state (which build is on it), and what blocks the next rung, if anything.
