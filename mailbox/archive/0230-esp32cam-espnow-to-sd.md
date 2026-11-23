---
task_id: "0230"
title: ESP32-CAM to the living frame over ESP-NOW, with an SD image library
status: verified
iteration: 2
proof:
  delivered_by: "0244 (archived, verified) and 0245 (archived, verified)"
  camera: "Freenove ESP32-Wrover CAM, ESP32-D0WD-V3, GC0308 PID 0x9b, MAC f0:16:1d:ae:2b:20"
  link: "1,280 chunks per 640x480 RGB222 frame, 1692-2097 ms at 24 Mbps, fail 0, per-frame SHA-256 acked"
  library: "/sdcard/LIVE/LIVE####.BIN on the frame board, 307,200 B per file, stored sha == sent sha"
  branch: "hermes/0245-live-speed @ 5d8d53c (built on hermes/0244-button-photo-flow @ f10353b)"
notes: |
  Closed at Stephen's direction 2026-11-23 ("close them"). Superseded by 0244 and
  0245, which delivered this task's whole ladder — camera capture, the chunked
  ESP-NOW link, and the SD image library (on the frame board's card rather than
  the camera's, which is the design that won).
---

## Status: verified — superseded by 0244/0245, closed at Stephen's direction

**The blocker this task recorded does not hold.** It reported that the kit
ESP32-CAM "has no talking camera module": `esp_camera_init()` returned 261
(`ESP_ERR_NOT_FOUND`), no SCCB ACK on 26/27 across every pin, pull-up and clock
variant tried. That was a pin-map and pixel-format problem, not a dead module.

What 0244 proved on that same board:

- `esp_camera_init()` succeeds with the kit's own `CAMERA_MODEL_WROVER_KIT` map:
  XCLK 21, SIOD 26, SIOC 27, PCLK 22, VSYNC 25, HREF 23, D0-D7 4/5/18/19/36/39/34/35.
- The sensor is a **GC0308 (PID 0x9b)**, not an OV2640, and it has **no JPEG
  engine**: `PIXFORMAT_JPEG` fails with `JPEG format is not supported on this
  sensor` (0x106). Capture is RGB565; `frame2jpg()` encodes on demand.
- RGB565 from this driver is **big-endian per pixel** — read as a native
  `uint16_t` it produces structured rainbow noise, which is what the panel showed
  before the fix.

The ladder landed: the chunked ESP-NOW transfer (1,280 packets, per-frame SHA-256,
1.7-2.1 s at 24 Mbps), the on-board colour chain (grey-world WB, gamma LUT, 4x4
Bayer dither), the button/countdown flow, and an SD library cycling every 10 s.

## Juno archive note (2026-11-23)

Closed and archived by Juno at Stephen's direction ("close them", relayed to
Hermes in session). The closure record above is Hermes's, staged at 4eab08f.
Its account checks against the 0244 and 0245 archives, both verified by Juno:
the recorded blocker (a mute camera module) was a wrong board definition —
the board is a Freenove WROVER-KIT with a GC0308 — and the task's whole
ladder was delivered by 0244/0245. ntfy close ping sent.
