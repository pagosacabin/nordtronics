# 0124 — LDS-006 decode sweep: baud rate × RX polarity on P16

expect-reply-within: 6h

Provenance: drafted 2026-10-07 by the interactive Hermes session with Stephen at the bench,
reviewed and cleared by Juno, and filed at her instruction. The filing commit is authored
`hermes-cronrunner`, not `snordlund` — the usual "Juno files, Hermes works" author signal
does not apply to this file, and its absence is not a protocol violation.

## Context

- **0122** (`hermes/0122-lidar-nodemcu` @ `20b9bad`, staged) created
  `firmware/lidar-lds006-bringup/` and ran the verbatim task sketch on the NodeMCU-32S
  (ESP32-D0WD-V3, MAC `00:70:07:e6:42:70`). That sketch passed `-1` as the Serial2 TX argument, so
  the ESP32 never drove either wire; the turret did not spin, and the bytes on P16 were a flat
  two-value flood (~79 % `0x00` / 21 % `0x04`, 16–18 distinct values, zero `0xFA`).
- **0123** (`hermes/0123-lidar-uart-commands` @ `d4af302`, staged) opened Serial2 with both roles
  (RX 16 / TX 17), removed the LEDC block, and sent `A5 60` / `A5 20` / `A5 65` + `A5 60` with the
  specified gaps, echoing every byte so the capture proves the send. Runs 1–2: all four sequences
  went out, **no spin**, **no frames**, and the same flat flood.
- **Run 3 — the evidence this task is built on.** Same binary re-flashed, same 70 s window, after
  Stephen swapped the bench wires: 89 319 bytes, **188 distinct values** (was 18–21), 11.7 % of the
  stream outside the old `04/00/40/44` family, clustering into repeating motifs — `FF F7 B6 BE CE 1E
  36 38`, `FF FF BF BE`, `9F DE 3E 1E` — with the rich-byte indicator's strongest non-trivial
  autocorrelation at a **~171-byte lag**, i.e. a repeat roughly every 0.13 s at the observed
  1275 B/s. Still **zero `0xFA`** and **zero `0xA5`**; the turret still did not spin. Recorded in
  the 0123 addendum (`origin/main` `c0140dc`).
- **Wiring, as Stephen reports it (2026-10-07, at the bench): the blue wire goes to P16** through
  the **10k/23k divider** (he confirms the divider is in the P16 path, so the pin sees ~2.8 V from
  the LiDAR's ~4 V idle and stays in spec); **the green wire goes to P17**. So run 3's 89 319-byte
  structured stream arrived on the **blue** wire, divided, at GPIO16.
- **What that means.** P16 appears to be on the wire the LiDAR *transmits* on and is hearing a
  structured signal at the wrong receive parameters — the signature of a live UART link sampled at
  the wrong rate or polarity, where an undriven line produces no bytes at all. It cannot be our own
  TX: the sketch sends 8 bytes in 70 s and 89 319 arrived. The remaining unknown is **decoding, not
  wiring** — which is what this task settles.
- **`0xFA`** is the frame header 0122/0123 were asked to look for; it stays the primary success
  signal here, without assuming the protocol wrapped around it.

## Task

One deliverable: a sweep sketch in the existing project that finds the receive parameters (if any)
under which P16 yields `0xFA`-framed data, and prints a verdict either way.

Branch `hermes/0124-lidar-rate-sweep`, cut from `hermes/0123-lidar-uart-commands`. Change only
`firmware/lidar-lds006-bringup/src/main.ino` (`platformio.ini` unchanged). One boot does:

1. **Phase A — listen only, all 22 combinations, no transmission at all.** Rates × polarity:

   | Pass | Rates (baud) | RX polarity |
   |---|---|---|
   | 1–11 | 9600, 19200, 38400, 57600, 115200, 128000, 230400, 256000, 300000, 460800, 921600 | normal — `setRxInvert(false)` |
   | 12–22 | the same list, same order | inverted — `setRxInvert(true)` |

   Per combination: `Serial2.end()`, reopen at the rate with the pins unchanged
   (`Serial2.begin(rate, SERIAL_8N1, 16, 17)`), set the polarity, drain 200 ms, listen 2 s. **P17 is
   never written in this phase**, so nothing is driven into a wire whose direction is still
   unknown.

2. **Phase B — transmit, only where Phase A earned it.** For every combination whose Phase A `fa` >
   0, and otherwise for the three highest-`bytes` combinations, send the three start sequences from
   0123 once and listen 2 s more. This is the only place the sketch drives green.

   Print exactly one line per combination, in both phases:

   ```
   probe rate=<rate> invert=<0|1> phase=<A|B> bytes=<n> fa=<n> first=<first 16 bytes as %02X>
   ```

3. **Confirm any hit.** If any combination saw `fa` > 0, re-run that one for a 10 s window and print
   64 bytes raw.

4. **Parity backstop.** If `fa_total` is 0 after all 22, repeat the three highest-`bytes` rates
   under `SERIAL_8E1` and `SERIAL_8O1` (6 more lines in the same format, with `parity=` added), so a
   complete miss still rules out the common non-default framings in one run.

5. Finish with one summary line: `sweep done best=<rate>/<invert> fa_total=<n> parity_pass=<0|1>`.

`setRxInvert(bool)` is available in the pinned core — verified on this machine at
`~/.platformio/packages/framework-arduinoespressif32/cores/esp32/HardwareSerial.h:281`
(arduino-esp32 2.0.17), so polarity needs no hardware change.

## Success criteria

1. Compiles for `nodemcu-32s` and flashes; the upload log shows `Hash of data verified.` for the app
   region (`0x10000`).
2. One capture of a single boot contains **exactly 22 Phase A `probe` lines** — one per tabled
   combination, in the tabled order, each carrying `bytes`, `fa` and `first=` — plus Phase B lines
   only for combinations that qualified under step 2, plus 6 further `parity=` lines **only if** the
   parity backstop ran.
3. The capture contains the `sweep done best=... fa_total=... parity_pass=...` line.
4. If `fa_total` > 0, that combination's 10 s confirmation window and its 64 raw bytes are in the
   capture. If `fa_total` is 0, the report says so plainly, names the parity backstop result, and
   names the single combination with the highest bytes/s so the next decision has data.
5. The wiring is recorded as Stephen reports it: which wire sits on P16 and on P17, and where the
   divider is.
6. Bench observation recorded as data: whether the turret spun during any pass (ask Stephen; he is
   at the bench for this run).
7. Reply staged with `proof` carrying branch + SHA, and the per-rate sweep table in `notes`.

## Constraints

- Keep **RX on P16 and TX on P17**, exactly as 0123 left them. The bench wiring is Stephen's and is
  not changed by this task.
- **Keep P17 silent for the whole of Phase A.** Green idles near 4 V on Stephen's meter and its
  direction is unknown until a decode succeeds; the ESP32 pin's limit is 3.6 V. Transmit only in
  Phase B, over the same 1.6k series arrangement 0123 used.
- **Do not swap the software roles.** P17 taken directly to 4 V is not safe without the divider.
- Push only to `hermes/0124-lidar-rate-sweep`, cut from `hermes/0123-lidar-uart-commands`.
- Change only `firmware/lidar-lds006-bringup/src/main.ino`.
- Flash app-only over the by-id port
  (`usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0`), as 0122/0123 did.
- CI position to state in the reply: this directory is not wired into
  `.github/workflows/platformio.yml`, so the proof is branch + SHA + flash log, not a run URL.

## Proof

- Branch `hermes/0124-lidar-rate-sweep` and the tip SHA, cross-checked with
  `git ls-remote --heads origin hermes/0124-lidar-rate-sweep`.
- The 22 Phase A `probe` lines, any Phase B lines, and the `sweep done` line, quoted in `notes`.
- The upload log's `Hash of data verified.` lines and the built `firmware.bin` sha256.

## Reply format

`proof`: `branch`, `sha`, `base` (`hermes/0123-lidar-uart-commands` @ `d4af302`), `files`,
`changes`, `flash`, `sweep` (the table), `observations`, `sample_hex`, and `wiring` (blue → P16 via
the 10k/23k divider, green → P17, unchanged by this task).

`notes`: per-rate result table, whether polarity mattered, the parity backstop outcome, the plain
verdict on the turret, and — if nothing decoded — the single most likely next parameter family.
