---
task_id: "0230"
status: in_progress
iteration: 2
proof: []
notes: |
  Stephen plugged in the ESP32-CAM on 2026-11-20 ~14:55 MST — the board is on the bench and available. Rungs that do not touch the frame board's SD card (CAM capture to its own slot; ESP-NOW link to the frame board) may proceed. The SD-dependent rungs wait on the interactive 0229 diagnosis, which is being worked by Stephen and Hermes directly.
  BLOCKED 2026-11-20 16:23 MST (23:23 UTC) — no proof block, nothing staged. Rung 1 is
  unattainable this run: THE KIT ESP32-CAM HAS NO TALKING CAMERA MODULE.
    Board, proven this run (probe firmware flashed over the CH340 uplink):
      chip ESP32-D0WD-V3 rev3, MAC f0:16:1d:ae:2b:20, flash 4,194,304 B, PSRAM 4,192,123 B.
      `esptool write_flash` -> `Hash of data verified.`, and the app's console came back
      end-to-end over /dev/serial/.../usb-1a86_USB_Serial-if00-port0 at 115200. Note the
      0225 "live console dead" fault belongs to the frame board's FTDI bridge, not to this
      CH340 path — the CAM console works.
    Camera fault: `esp_camera_init()` = 261 (ESP_ERR_NOT_FOUND) — the sensor probe gets no
    ACK on SCCB. The diag build (XCLK stopped / 20 MHz / 10 MHz, PWDN high and low, both
    candidate pin pairs, internal pull-ups forced on) reports:
      idle_sda=1 idle_scl=1 at every scan; i2c_found_count=0 on 26/27 in all five scans;
      ov2640_pid_20mhz=0x00 (expect 0x26); alt pair 21/22 also 0 devices.
    Zero devices on two pin pairs with the bus idling high is not a pin-map or XCLK fault;
    it is an absent or unpowered module (ribbon not seated / module not fitted / dead
    module). Unfixable from firmware — Stephen's hands.
    CAM's own microSD slot: `sd_begin=0 card_type=0 (NONE)` — no card in the CAM slot. The
    probe is read-only by design (card choice is 0229's), so nothing was written to any card.
    Rung 2/3 not attempted: they need a working capture and the frame board, which is under
    the interactive 0229 diagnosis with Stephen's re-solder retest pending — flashing a test
    build over it now would disturb that session. Frame board end state: untouched (no write);
    one `esptool read_mac` identification reset was run on its port, after which its flashed
    app resumes.
    DELIVERED AND PUSHED (not the task's proof, which needs device numbers): cabin-play branch
    hermes/0230-esp32cam-capture @ e672d11 carries living-frame/firmware/esp32cam-capture/ —
    three envs (esp32cam capture, esp32cam-probe identity+SD, esp32cam-diag SCCB scan), the
    pyserial bench harness (bench/capture.py, flash + console + base64->JPEG decode), the raw
    console logs of this run, README.md with the measured numbers, and PROTOCOL.md (chunked
    ESP-NOW layout, ACK/NACK/retry rule, the three-point SHA-256 chain). The retest after the
    ribbon and card are seated is one flash per env.
    CI: cabin-play has NO .github/workflows directory at all (`ls .github/workflows/` empty,
    `git ls-tree -r --name-only origin/hermes/0230-esp32cam-capture .github/workflows` empty),
    so no run exists or is expected for this branch — `run: none` is enumerated, not assumed.
    ntfy: not applicable — this task's proof section names branch SHA + device report paths,
    no topic, and nothing was published.
    Copilot peer review: no PR was opened (task's proof asks for review on GitHub-bound work,
    and the deliverable is blocked anyway); the review step is owed on the retest branch.
    DECISION OWED: (i) seat/replace the OV2640 module (ribbon contacts toward the board, latch
    closed) and seat a card in the CAM slot, then a tick re-runs `esp32cam-diag` (expect
    i2c_found=0x30, ov2640_pid=0x26) and `esp32cam`; (ii) name the bench window for rung 2 so
    the frame board can take a test build without colliding with 0229's SD retest.
    Run facts: PEAK OFF-PEAK 23:15Z, PROTOCOL MATCH 1.3.0; pickup commit 7bf9bd2 (iteration 2);
    active/ held 0213 and 0216, both decision-blocked and left untouched; inbox 0176 is
    `superseded` and 0229 is interactive per Stephen, so 0230 was the lowest actionable task.
  Update 2026-11-20 17:02 MST (Stephen, via Juno): the OV2640 module has been reseated. Re-run authorized now — diag first (expect i2c_found=0x30, ov2640_pid=0x26), then the capture rung. Rung 2 window is GRANTED: Stephen says the between-boards ESP-NOW test can go ahead, so flash the frame-board test build as needed; restore the accepted 800x600 build when the rung ends. CAM slot card state was not stated — report what you find in the slot.
  Reseat detail (Stephen, 17:05 MST): full reseat — board unplugged, camera module unplugged, camera replugged, board replugged, all to the SAME slots. So the module connection was broken and remade and the board power-cycled; no port or slot changed.
  RUN 2026-11-20 17:15-17:45 MST (00:15-00:45 UTC) — resumed after Stephen's reseat; rung 1 still
  fails identically, RUNG 2 PROVEN on the bench, nothing staged (rung 1 is the task's first
  criterion and it is a hardware condition). Frame board is back on the accepted 800x600 build.
  Branch hermes/0230-esp32cam-capture @ 6394c11 (origin, verified by ls-remote). Copilot reviewed
  four passes (fb336ec -> 99960cd -> f1cca1b -> 03c8713): 7 findings, all fixed, final verdict
  SHIP at 03c8713. Full numbers in the Reply section at the end of this file. iteration unchanged
  (this is a resume of iteration 2, not a new pickup).
  Power fact (Stephen, 18:25 MST): the CAM board is powered over USB 5 V. The fed-at-3.3 V candidate is therefore CLOSED by the owner — do not re-raise it. Remaining camera candidates: dead module/ribbon, FFC orientation. Stephen notes a replacement via Amazon would not arrive in time for the build; no decision taken yet.
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-20T23:16:25+00:00"
lease_until: "2026-11-21T00:46:25+00:00"
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

## Reply 2026-11-20 23:23 UTC (16:23 MST) — BLOCKED at rung 1, no proof block

Rung 1 cannot be completed this run, so there is no `proof` block and the task is not
staged. The board is on the bench and reachable; its **camera module is not**: the OV2640
does not answer on SCCB at any XCLK/PWDN state or pin pair, and the CAM's own microSD slot
holds no card. Both are hands-at-the-bench conditions.

**Rung 1 — camera bring-up: FAILED, cause isolated.** `esp_camera_init()` returns
`261 / ESP_ERR_NOT_FOUND`. The diagnostic build scanned the bus with XCLK stopped, at
20 MHz and at 10 MHz, with PWDN high and low, with internal pull-ups forced on, and on the
alternate 21/22 pin pair: **zero devices at every address**, `ov2640_pid_20mhz=0x00`
(expected `0x26`). Lines idle high throughout, so the bus is not being held — nothing is
driving it. That is an absent/unseated module, not a firmware fault.

**Board identity (proven, flashed and read back):** ESP32-D0WD-V3 rev 3,
MAC `f0:16:1d:ae:2b:20`, 4 MB flash, 4 MB PSRAM; flashing verifies (`Hash of data
verified.`) and the console is live over the CH340 uplink at 115200.

**Rung 1 — SD sink: no card.** `sd_begin=0 card_type=0 (NONE)`. The probe is read-only
(0229 owns the card decision); nothing was written.

**Rungs 2-3: not started, two dependencies.** They need a working capture, and they need
the frame board, which is inside the interactive 0229 diagnosis with Stephen's pin-10..13
re-solder retest pending. Flashing a test build over it now would disturb that bench
session. The frame board was not written to (one read-only `esptool read_mac` reset).

**What is committed:** cabin-play `hermes/0230-esp32cam-capture` @ `e672d11`
(`living-frame/firmware/esp32cam-capture/`) — capture/probe/diag firmware, the bench
capture harness, this run's raw console logs, a README carrying the measured numbers, and
`PROTOCOL.md` with the chunked ESP-NOW design (16-byte header, 234-byte payload, in-order
chunks, NACK-on-gap, SHA-256 equality at capture → reassembly → SD-rest). No CI exists in
that repo (`ls .github/workflows/` is empty), so `run: none`; ntfy not applicable (the
task names no topic and nothing was published).

**Decision owed:** (i) reseat/replace the OV2640 module and seat a card in the CAM slot —
the next tick then re-runs `esp32cam-diag` (expect `i2c_found=0x30`, `ov2640_pid=0x26`) and
`esp32cam`; (ii) name a frame-board bench window for rung 2 that does not collide with
0229's retest.

## Reply 2026-11-21 00:45 UTC (17:45 MST) — rung 1 still blocked, rung 2 PROVEN

No `proof` block: the task opens with rung 1, and rung 1 is a hardware condition the bench cannot
clear from firmware. Rung 2 — which the 17:02 update explicitly opened — **is complete and proven**,
with the numbers below. The task stays in `active/`.

### Rung 1 — camera bring-up: FAILED again after the reseat, cause narrowed

Stephen's full reseat (board unplugged, camera module unplugged, replugged, board replugged, same
slots) changed nothing measurable. Both bring-up builds were re-flashed and captured (17:16 and
17:18 MST); `POWERON_RESET`, no brownout, no `ESP-ROM` banner in either capture:

| check | before reseat (16:15) | after reseat (17:16) |
|---|---|---|
| SCCB scan 26/27, PWDN high, XCLK off | 0 devices | 0 devices |
| … PWDN low, XCLK off / 20 MHz / 10 MHz | 0 / 0 / 0 | 0 / 0 / 0 |
| alternate pair 21/22 | 0 devices | 0 devices |
| `ov2640_pid_20mhz` | `0x00` (expect `0x26`) | `0x00` |
| SCCB lines | SDA 1 / SCL 1 | SDA 1 / SCL 1 |
| board identity | D0WD-V3 rev3, MAC f0:16:1d:ae:2b:20, 4 MB flash, 4 MB PSRAM | identical |
| CAM microSD slot | `sd_begin=0 card_type=0 (NONE)` | `card_type=0 (NONE)` — **still no card**, as asked |

A connection that was broken and remade with zero change in bus state is no longer a seating fault.
The three candidates left are all hands-and-multimeter: (a) dead module or ribbon, (b) **the camera's
3.3 V rail** — the AI-Thinker feeds the OV2640 from the board's regulator, so a supply fed at 3.3 V
instead of 5 V would run the ESP32 while starving the camera, and (c) the FFC contacts facing the
wrong way. Nothing in firmware can separate them; a second known-good module is the fastest
discriminator.

### Rung 2 — ESP-NOW link: PROVEN, both directions

Design: 32 × 240 B packets, header `u16 seq | u16 expect | u32 transfer_id`, payload
`(seq*7 + i*3 + 0x5a) & 0xff`. Sender ESP32-CAM (f0:16:1d:ae:2b:20) → frame board
(c4:9e:7e:08:6d:f4), channel 1, `11B|11G|11N` on both ends.

- CAM: `pass=1 send_ok=31 seq_acked=31`, `pass=2 send_ok=32 seq_acked=32 of 32`,
  `result=espnow_tx_sent`, `sha256_tx=74bb840392a757f743926df1e363626405bb5416482b85e30e817766afaa2f28`
- Frame board (its own report partition, read back with esptool):
  `rx_packets=63 distinct=32 rx_bytes=15120`, `bad_pattern=0 dup=31 out_of_range=0`,
  `tfer_id=0x0230beef first_seq=0 last_seq=31`,
  `sha256_rx=74bb8403…aa2f28`, `result=espnow_rx_complete`, `report_write_errors=0`

**Sender hash == receiver hash**, so the 7,680 bytes on the air are the 7,680 bytes reassembled in
order — that is the rung-2 criterion (a report showing the payload received intact), not an ACK
count. One frame of pass 1 was genuinely lost and pass 2 closed it; the receiver still ended with 32
of 32 distinct sequences. Cause found and fixed: each report write erased/rewrote a 64 kB partition
per packet (2 of 32 lost) — now throttled to a 5 s heartbeat, and the residual single-frame loss is
what rung 3's NACK/retry rule has to absorb.

### Peer review (Copilot, Herdr pane t17)

Four passes on the committed code, seven findings, **all fixed**, final **Verdict: SHIP** at
`03c8713`. Two mattered: the sender's success criterion counted ACKs in aggregate (a duplicate pass
could mask a missing sequence), and the per-sequence state was not cleared between passes. Fixing
the criterion is what exposed the receiver's flash-write frame loss above. No GitHub PR exists in
cabin-play (the task's own note: writes there return 403), so the review record is the README
section, as 0227/0228 did.

### Frame board end state — restored, verified from the board

Accepted 800x600 gamma build back on it, report read back at 17:43 MST: `mode 800x600`,
`sha256 runtime 204578cc…bd306` **==** `tool`, `MATCH`, `361 vsync → 60.18 Hz`, `14475 fills →
40.1/frame` — the frozen 0228 proof reproduced. **Deviation declared:** my first restore attempt
flashed `env:n16r8-photo-800x600` straight from the committed tree and the app aborted at boot
(`FATAL: embedded image is 307200 B, framebuffer needs 480000 B`) because `src/photo64_data.h` in
git is the 640x480 raster — the env's raster comes from the converter, not from build flags. I
regenerated the 800x600 raster from the committed source (`--mode 800x600 --space gamma`, hash
`204578cc…`, i.e. byte-identical to the accepted build), rebuilt, flashed, and reverted the generated
headers afterwards so the committed tree is untouched. The frame board carried a non-rendering build
for about one minute during that; it is fine now and the numbers above prove which build is on it.

### Committed

cabin-play `hermes/0230-esp32cam-capture` @ **6394c11** (origin; `git ls-remote` matches):
`living-frame/firmware/n16r8-espnow-rx/` (ESP-IDF receiver, partitions, report dumps, README with the
trap above), `living-frame/firmware/esp32cam-capture/` (`esp32cam-espnow-tx` env + source,
`bench/run-2026-11-20-reseat/` console logs for diag/probe/transfer, README retest table,
`PROTOCOL.md` measured-rung-2 section). CI: cabin-play has **no** `.github/workflows` directory
(`ls` fails, `git ls-tree origin/<branch> .github` empty) — `run: none` enumerated, not assumed.
ntfy: not applicable — the task names no topic and no artifact was published.

### Decisions owed

(i) Camera: swap in a known-good OV2640 module, or measure the 3.3 V rail at the camera connector and
confirm how the board is being fed (5 V vs 3.3 V); check FFC contact orientation last. (ii) A card in
the CAM slot if rung 1's SD sink is still wanted (rung 3 writes to the **frame** board's card under
0229, so this is optional). (iii) Rung 3 still needs 0229's working SD configuration on the frame
board; the ESP-NOW half it depends on is now proven.
