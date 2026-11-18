---
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T22:15:28+00:00"
lease_until: "2026-11-18T23:45:28+00:00"
---
# 0218 — Correction to 0216: the board is GOOD — the fault is in the rig (not a task)

Stephen's bench report, 2026-11-18 ~15:08 MST, supersedes the board-branch reasoning in 0216:

- The hotspot test ran (0216 step 1): the board, seated in the rig, refused Stephen's phone hotspot with the same `Auth Expired` behavior. Per the tree, Hermes called the board bad.
- Stephen then pulled the ESP32-C3 Super Mini off the breadboard and tested it separately — **it joined immediately** (it is the `propane-bench` entry now visible in the Starlink device list).

Conclusions that follow:
- Do NOT condemn or retire the board (MAC 9c:cc:01:d0:78:80). It is proven good bare.
- The fault follows the rig/breadboard environment, not the board, not the router, not the credentials, not the build. (The router penalty-box theory is also dead: a fresh AP with no history with this board refused it identically while it was in the rig.)
- Next separator, when the rig is reassembled: power the board from clean USB while it is seated in the rig. Joins → the rig's power path (shared 5 V rail, sensor heaters, breadboard leads) is drooping under WiFi TX load. Still refuses on clean power → RF/antenna environment in the rig.
- The 0216 permanence deliverable (crew vault `bench/wifi-lessons.md`) must record THIS diagnosis — rig environment, mechanism to be confirmed by the power separator — not "bad board."
