---
task_id: "0138"
protocol_version: 1.0.0
status: in_progress
expect-reply-within: 6h
iteration: 1
---

# 0138 — Living-frame rung 0 proof: VGA test pattern on the Magnavox

## context

Living-frame project (Christmas gift): Magnavox 22MD311B/F7 22" LCD TV driven
by ESP32 through its VGA/PC input. Stephen's ruling: proof first — nothing
else starts until the display path is proven.

Bench inventory (confirmed by Stephen 2026-10-15 with photos): classic ESP32
devkits on hand — ESP32-WROOM-32 modules and a DFRobot FireBeetle ESP32-E
(DFR0654). Both are classic ESP32 (not S3): the proven VGA path.

Play projects live in `pagosacabin/cabin-play`, never nordtronics. Push via
SSH from the laptop — the API credential is nordtronics-scoped (403 on
cabin-play), as learned on 0136.

## task

Build the rung-0 proof firmware: a classic ESP32 outputting a VGA test
pattern (color bars + a text label, 640x480) using bitluni's ESP32Lib
(or FabGL — your call, say which and why in the notes).

1. Write the firmware (PlatformIO project preferred; Arduino sketch
   acceptable if you say why). Target board: classic ESP32-WROOM-32 devkit.
2. Specify the EXACT wiring: which GPIO goes to which VGA pin (R, G, B,
   HSYNC, VSYNC, GND) including resistor values for the DAC ladder, per the
   library's wiring diagram. Stephen will wire this by hand — pin numbers
   must be unambiguous.
   NOTE 2026-10-15: Stephen has a VGA adapter for this — his
   PCB_0024_VGA_ADAPTER v1 schematic (270/470 ladders on R/G/B, direct
   HSYNC/VSYNC to a DB15). He has the physical module or can build from the
   schematic. Use the adapter's JP1 pin mapping for your wiring spec instead
   of a hand-wired ladder: the ESP32 plugs into the adapter, the adapter
   drives the VGA cable. If any JP1 pin assignment is unclear from the
   schematic, say so in the notes field rather than guessing.
3. Include flash instructions (one command or the exact IDE steps).
4. Push the project to `pagosacabin/cabin-play` under
   `living-frame/firmware/vga-proof/` via SSH.

## success_criteria

- Firmware builds cleanly for the classic ESP32 target.
- Wiring spec is complete and unambiguous (GPIO -> VGA pin + resistor).
- Stephen can flash, wire, and see a stable, correct-color test pattern on
  the Magnavox. (His photo of the TV is the final acceptance — he reports
  it, you don't need it to stage.)

## constraints

- Classic ESP32 only. No TV modifications, no panel work.
- One deliverable: proof firmware + wiring spec in cabin-play.
- No secrets in the task file or staged reply.

## proof

- Commit SHA on `pagosacabin/cabin-play` main containing
  `living-frame/firmware/vga-proof/`.
- Build log excerpt (successful build for the ESP32 target).
- The wiring table, quoted in the staged reply.

## reply_format

Stage the reply under `mailbox/staged/0138-vga-proof-rung.md` with front
matter (`task_id: 0138`, `status: staged`, `iteration: 1`), the proof
pointers above, and a `notes:` field (library choice + anything unexpected).
