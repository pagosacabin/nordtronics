---
task_id: "0176"
status: inbox
iteration: 0
proof: []
---

# 0176 — Living frame: 64-color VGA breakout board — schematic design

expect-reply-within: 48h

## Assignment

- **Hermes:** produce the KiCad schematic design (schematic only — no PCB layout in this task) plus the written pin map and BOM. KiCad capture is your proven rung; Newton is not involved.
- **Stephen:** review the schematic when it stages; layout, fab, and the physical build are his steps after review.
- **Juno:** verify the commit and the stated checks, then archive after Stephen's review.

## Context

- Living frame (white-elephant gift build): rung 0 (3-bit VGA color bars) and rung 1 (0175, one JPEG displayed) are both proven and archived. The 3-bit picture is too coarse — 8 colors reads as a dot puzzle, and Stephen's bar is a picture that reads as the photo.
- Decision (Stephen, 2026-11-12): go to **64 colors — 2 bits per channel — using FabGL's VGA 64-color controller** on the same classic ESP32-WROOM-32 devkit. This replaces the 14-bit bitluni option (parked, not cancelled — it stays available if 64 colors disappoints).
- Form factor (Stephen's call): the board IS the breakout — a proper PCB carrying a **standard female DB15 VGA socket** a normal VGA cable plugs into, the DAC resistors on the board, a header interface to the devkit, and **mounting holes so the board can be screwed down** inside the frame. Same role/form as his PCB_0024 v1 adapter, new guts. Not a point-to-point/bare-breakout hack.
- Stephen supplied a candidate pin/value table (FabGL defaults, pasted 2026-11-12): R MSB GPIO22 → 470 Ω, R LSB GPIO21 → 910 Ω, G MSB GPIO19 → 470 Ω, G LSB GPIO18 → 910 Ω, B MSB GPIO5 → 470 Ω, B LSB GPIO4 → 910 Ω, all joining at the VGA pin (1/2/3 respectively); HSYNC GPIO23, VSYNC GPIO15 (direct or ~100 Ω) → pins 13/14; GND → pins 5, 6, 7, 8, 10. **Treat this table as a proposal to verify against FabGL's documentation, not as gospel** — check the controller's actual default pins and reference resistor values in the FabGL docs/source and cite what you checked in the reply.

## Task

Design the schematic in KiCad and commit the project to `pagosacabin/cabin-play` at `living-frame/hardware/vga-64-breakout/`:

1. **Circuit:** ESP32 devkit header → 2-bit-per-channel resistor DAC (two resistors per channel joining at the VGA pin, ~2:1 MSB:LSB weighting, monitor's 75 Ω termination completes the DAC) → female DB15 VGA socket. HSYNC/VSYNC with optional ~100 Ω series resistors. Grounds per the VGA pinout.
2. **Pin selection, deliberate:** start from FabGL's VGA 64-color defaults, then review every pin. GPIO5 and GPIO15 in the pasted table are **strapping pins** — if FabGL's controller allows custom pins, move the affected signals to safer output GPIOs; prefer keeping HSYNC/VSYNC on the already-proven D32/D33 if the controller allows it. Avoid loading strapping pins (0, 2, 5, 12, 15) at boot unless you justify the specific pin in the notes. Every deviation from the FabGL defaults gets one line of reasoning in the README.
3. **Written pin map + BOM** in the project README: final GPIO → bit → resistor value → VGA pin table (this is the contract the firmware task will be written against), plus the parts list (DB15 female connector, resistors, devkit header, mounting holes).
4. **Review artifact:** a rendered schematic (PDF or PNG) committed alongside the KiCad files so Stephen can review without opening KiCad.
5. Schematic only. No layout/routing in this task — Stephen reviews the design first, layout is the next task.

## Success criteria

- KiCad project + rendered schematic committed on cabin-play `main` at the path above; reply carries the commit SHA.
- ERC runs clean, or every ERC warning is listed and explained in the reply.
- README contains the final pin-map table and BOM; every pin that differs from FabGL's defaults has its one-line reason.
- Resistor values and pin defaults in the design trace to FabGL documentation/source that the reply cites — not to the pasted table alone.

## Constraints

- Cabin-play writes go over SSH from the laptop (gh.py is nordtronics-scoped, 403 on cabin-play — 0136 precedent).
- No purchases, no fab order — design only.
- Classic ESP32-WROOM-32 devkit only; the board must not block the devkit's USB/boot/EN access when plugged in.
- One deliverable: the schematic design package. Layout, firmware, and any 14-bit work are out of scope.

## Proof

- Commit SHA on `pagosacabin/cabin-play` `main` covering `living-frame/hardware/vga-64-breakout`.
- cabin-play has no CI: `run: none` is the honest entry (0138/0175 precedent).

## Reply format

Staged reply in `mailbox/staged/` per protocol 1.2.0: front-matter with `status: staged`, proof list (`branch: main` + sha, `run: none` with the no-CI note), the cost block, and notes covering: the FabGL source you verified the pinout/values against, the final pin map (or a pointer to the README table), any strapping-pin decisions, and ERC results.
