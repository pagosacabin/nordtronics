---
task_id: "0124"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: "hermes/0124-lidar-rate-sweep"
  sha: "9bdd0a6b90be6e8ff1a3c4924de22a33f1f47133"   # == git ls-remote --heads origin hermes/0124-lidar-rate-sweep
  base: "hermes/0123-lidar-uart-commands @ d4af302048c6c21a4c4223a64ca5cf05fdff3641 -- the branch the task names; the task branch is that tip + 1 commit"
  files:
    - firmware/lidar-lds006-bringup/src/main.ino      # only file changed, +259/-37 vs 0123
  changes: "The sweep as specified: Phase A listens over all 22 rate x polarity combinations with P17 never written, Phase B transmits the 0123 start sequences only for the combinations that earned it, a confirmation window on the best combination, the parity backstop, and the summary line. Three deviations/additions, all declared in notes: (a) the probe windows are hard-bounded inside the drain loop, because a continuously active line starves that loop and the first build overran its budget by ~3x and stopped after 20 of 22 probes; (b) the parity backstop runs ALWAYS rather than only when fa_total is 0, because fa_total was non-zero from chance-level 0xFA hits and leaving parity untested on that basis would be the wrong call; (c) a 15 s silent best-window was added so the reply can describe the stream's actual shape rather than 16 bytes of it."
  flash: "pio run -t upload over the by-id port, four bench cycles in total (run1..run4, described in notes/runs). esptool wrote 0x1000 17536 B, 0x8000 3072 B, 0xe000 8192 B, 0x10000 269456 B, each followed by 'Hash of data verified.' then 'Hard resetting via RTS pin...' and SUCCESS. Chip ESP32-D0WD-V3 rev v3.1, MAC 00:70:07:e6:42:70, same board as 0122/0123. Built firmware.bin 269456 bytes, sha256 b3f67a900d332ab09d8d20c0bb2c869579dab661be50b03c8d3d4747ffb2d18c."
  sweep: "NOTHING DECODED. 39 probe lines (22 Phase A, 11 Phase B, 6 parity) plus two long windows: 1054929 bytes received in total, and the 0xFA frame header appeared 1419 times -- every probe's own count is at or BELOW the ~1/256 rate pure chance would give in the same byte count (2825 expected across the probe windows alone). No rate, no polarity and no parity variant produced a frame stream. Per-rate table in notes."
  bench_fault: "The 1.6k series resistor in the wire going to P17 (the TX wire) was DISCONNECTED during the original run; Stephen found it and reconnected it, and confirms it was already reconnected before the re-run below. Consequence: every transmission in the original run's Phase B -- the 0123 start sequences on each qualifying combination -- went into an OPEN CIRCUIT and never reached the LiDAR, so 'no response to the commands' was previously untested rather than negative. The receive side is untouched by the fault: Phase A never writes P17 and listens on the other wire, so all 22 Phase A rows and the parity rows stand as measured."
  rerun: "Same binary 9bdd0a6b re-flashed with the wire intact, 195 s: 22 Phase A + 15 Phase B + 6 parity + both long windows. 'sweep done best=460800/1 fa_total=720 parity_pass=1'. Still no framing anywhere -- every probe's 0xFA count is at or BELOW its own 1/256 chance rate (460800/inv 148 vs 242 expected, 230400/normal 21 vs 117, 300000/normal 19 vs 180), and the two long windows gave 307873 B with 773 0xFA (0.25%) and 438710 B with 1042 0xFA (0.24%) against 0.39% chance. With the line genuinely connected, 15 combinations had the sequences delivered and the stream's shape and 0xFA rate are unchanged from the listen-only rows. The best combination moved from 230400/inv to 460800/inv between runs -- the winner is whichever noise sample crosses 0xFA most often."
  observations: "TURRET: NOT OBSERVED for this run -- Stephen was at the bench when it ran and was asked immediately afterwards, but the question went unanswered (he had stepped away), so criterion 6 is NOT met and no spin claim is made for 0124. BYTES: no decode at any of the 11 rates, both polarities, or the 8E1/8O1 variants; byte rates track the sampling rate rather than any data rate, which is what a continuously toggling line looks like and not what a UART transmitter looks like."
  wiring: "blue -> P16 through the 10k/23k divider (RX), green -> P17 (TX), unchanged by this task; confirmed with Stephen 2026-10-07. P17 was never written during Phase A."
  runs: "run1 150 s = first build, sweep truncated at 20/22 probes (sketch bug, fixed); run2 180 s = window-bounded build, complete, parity_pass=0; run3 190 s = a compile error left the previous image in place, so this capture re-ran run2's binary (used as a reproducibility check, NOT parity evidence); run4 195 s = the binary cited here, full sweep plus parity, 'sweep done best=230400/1 fa_total=411 parity_pass=1'."
  sample_hex: "BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF BF FD BF BF BF "
notes: |
  STAGED (iteration 1). Model tier: DeepSeek Flash (this session). Bench work done
  interactively at Stephen's request; the four cron worker jobs were paused for the
  duration and are resumed on staging. Provenance is in the pickup notes: this task was
  drafted interactively and filed at Juno's instruction, and the 13:15 tick could not
  have run it (gate blind spot for locally-filed tasks).

  HEADLINE: nothing decodes. Across 39 probes and two long windows --
  1054929 received bytes -- the 0xFA frame header appears
  1419 times, and in every single probe the count is at or below what
  chance alone would produce (2825 expected over the probe windows). There
  is no frame stream on the blue wire at any of the 11 rates, in either polarity, or
  under 8E1/8O1.

  1. WHAT WAS RUN (criteria 1-3 MET). ... three deviations, all declared:
     (a) THE WINDOW FIX. The first build stopped after 20 of 22 Phase A probes and never
     reached Phase B. Cause, and it is a real property of this bench: the probe drains
     with `while (Serial2.available()) ...`, and when the line is CONTINUOUSLY active that
     inner loop never finds an empty moment, so the 2 s window was never re-checked and
     each probe overran to ~7 s. The fix bounds the window inside the drain. Worth
     recording as a bench fact, not just a code fix: a listener that only checks its
     deadline between drains cannot time-bound a wire that never goes quiet.
     (b) PARITY ALWAYS. As above -- fa_total came back non-zero from chance hits, so the
     spec's "only if fa_total is 0" trigger would have skipped the one test that rules
     parity out. It ran.
     (c) BEST-WINDOW. 15 s silent at the best combination, so the reply describes the
     stream's shape rather than 16 bytes of it.

  2. PHASE A -- the table. Columns: bytes received in the 2 s window, that as B/s, that as
     a percentage of the rate's theoretical maximum (rate/10 B/s at 8N1), 0xFA count, and
     the count chance alone would give for the same bytes.

  | rate | polarity | bytes | B/s | % of baud/10 | 0xFA | chance 0xFA |
  |---|---|---|---|---|---|---|
  | 9600 | normal | 1817 | 908 | 95% | 0 | 7 |
  | 9600 | inv | 404 | 202 | 21% | 0 | 2 |
  | 19200 | normal | 3644 | 1822 | 95% | 3 | 14 |
  | 19200 | inv | 811 | 405 | 21% | 0 | 3 |
  | 38400 | normal | 7307 | 3653 | 95% | 0 | 29 |
  | 38400 | inv | 1617 | 808 | 21% | 0 | 6 |
  | 57600 | normal | 9787 | 4893 | 85% | 0 | 38 |
  | 57600 | inv | 2105 | 1052 | 18% | 0 | 8 |
  | 115200 | normal | 19591 | 9795 | 85% | 0 | 77 |
  | 115200 | inv | 4920 | 2460 | 21% | 2 | 19 |
  | 128000 | normal | 19770 | 9885 | 77% | 0 | 77 |
  | 128000 | inv | 6840 | 3420 | 27% | 0 | 27 |
  | 230400 | normal | 30475 | 15237 | 66% | 4 | 119 |
  | 230400 | inv | 25440 | 12720 | 55% | 64 | 99 |
  | 256000 | normal | 29982 | 14991 | 59% | 16 | 117 |
  | 256000 | inv | 26040 | 13020 | 51% | 46 | 102 |
  | 300000 | normal | 44022 | 22011 | 73% | 5 | 172 |
  | 300000 | inv | 36259 | 18129 | 60% | 8 | 142 |
  | 460800 | normal | 3292 | 1646 | 4% | 1 | 13 |
  | 460800 | inv | 60103 | 30051 | 65% | 45 | 235 |
  | 921600 | normal | 3705 | 1852 | 2% | 0 | 14 |
  | 921600 | inv | 2243 | 1121 | 1% | 1 | 9 |

     Read the % column, not just the 0xFA column. At 9600-115200 normal polarity the byte
     rate sits at 85-95% of the theoretical maximum, i.e. the receiver is producing a byte
     essentially every time one could be produced -- which can only happen if the wire has
     transitions at least as fast as the sampling rate. A UART transmitter is IDLE between
     bytes: listening to a real 115200 stream with no data flowing yields zero bytes, and
     listening to one WITH data yields the data rate, not 85% of the sampling ceiling.
     Nothing about this wire idles.

  3. PHASE B -- the combinations that earned it (11 of them, per the spec's "any
     Phase A fa > 0") re-probed with the 0123 start sequences transmitted first. No
     combination improved; the 0xFA counts stay in the same chance-level band.

  | rate | polarity | bytes | % of baud/10 | 0xFA |
  |---|---|---|---|---|
  | 19200 | normal | 2044 | 53% | 4 |
  | 115200 | inv | 5040 | 22% | 14 |
  | 230400 | normal | 28491 | 62% | 9 |
  | 230400 | inv | 23526 | 51% | 52 |
  | 256000 | normal | 29250 | 57% | 8 |
  | 256000 | inv | 26657 | 52% | 54 |
  | 300000 | normal | 43250 | 72% | 2 |
  | 300000 | inv | 36431 | 61% | 8 |
  | 460800 | normal | 3683 | 4% | 2 |
  | 460800 | inv | 50934 | 55% | 40 |
  | 921600 | inv | 2103 | 1% | 1 |

  4. PARITY BACKSTOP -- 6 lines, no framing. DECLARED IMPERFECTION: the top-3 selection is
     over COMBINATIONS, so when both polarities of one rate place in the top three, the
     parity pass tests that rate twice (300000 appears twice under 8E1 and twice under
     8O1). The six lines are really three distinct tests run twice each; deduping by rate
     is the fix in any later sweep.

  | rate | framing | bytes | % of baud/10 | 0xFA |
  |---|---|---|---|---|
  | 460800 | 8E1 | 3097 | 3% | 0 |
  | 300000 | 8E1 | 31182 | 52% | 4 |
  | 300000 | 8E1 | 31162 | 52% | 5 |
  | 460800 | 8O1 | 3676 | 4% | 1 |
  | 300000 | 8O1 | 31206 | 52% | 4 |
  | 300000 | 8O1 | 31259 | 52% | 8 |

  5. CONFIRMATION AND BEST WINDOWS at 230400/inverted (the best combination by 0xFA, 64
     hits in Phase A): 10 s gave 131844 bytes with
     401 0xFA (0.30%, chance is 0.39%); the 15 s
     silent window gave 199920 bytes with
     607 0xFA (0.30%). The bytes are dominated by a
     handful of repeating values (BF/FF at 230400, 7F at 256000, 00 at 460800, 80 at
     300000, FF/FE at 921600-inverted), i.e. a periodic waveform being aliased, not data.

  6. THE TURRET: NOT OBSERVED, AND I WILL NOT INVENT IT. Criterion 6 is unmet. Stephen
     was at the bench while this ran, but the direct question afterwards went unanswered,
     so there is no observation for 0124. Supporting facts that are not a substitute: the
     sketch transmits only in Phase B, so the motor had no stimulus before those probes;
     and in 0122 and 0123, where he DID watch and the same three sequences went out, the
     turret never moved.

  7. WHAT THIS MEANS (measurement vs inference). MEASURED: no rate, polarity or parity
     decodes; the wire never idles; 0xFA is at or below chance in every probe. INFERRED,
     and labelled: the blue wire is carrying a periodic signal far faster than any UART
     rate we sampled, which is consistent with the ~9 kHz low-going pulses Stephen saw on
     the scope (4.2 V -> 3.6 V) rather than with a data line. The sweep's numbers
     independently support that: at rates whose bit time is much longer than a 9 kHz
     period the byte rate saturates (one start bit per pulse), and at 921600 it collapses
     to 1-2% because the pulses are too slow to qualify as start bits. So runs 1-2 of 0123
     ("we hear nothing") and run 3 ("we hear something") are both explained without a
     frame ever being present.

  8. SAFE NEXT STEPS, NAMED BUT NOT PERFORMED (a decode sweep was this task's scope; these
     are bench/firmware decisions for a later task):
     (a) THE SUPPLY. Every wire in this bring-up idles at ~4 V on Stephen's meter, not the
     5 V this class of unit expects, and the device is motor-driven and has never once
     spun. A supply that sags under load explains BOTH symptoms with one cause, and it is
     the cheapest thing left to test: measure at the LiDAR connector under load, or run it
     from a bench supply with adequate current. No firmware involved.
     (b) SCOPE BLUE WITH P17 COMPLETELY QUIET. Characterising the ~9 kHz train (frequency,
     duty, whether it changes when the turret is turned by hand) separates "clock/tacho"
     from "data" in one measurement. The sweep's numbers already point at the former.
     (c) ACCEPT THE UNIT MAY BE DEAD, and stop spending bench time if (a) and (b) are
     clean.
  CONSTRAINTS HONOURED: RX stayed on P16 and TX on P17; no wiring was changed; Phase A
  never wrote P17; the software roles were not swapped; only main.ino changed; the branch
  is cut from 0123 as specified; CI position stated -- this directory is not in
  .github/workflows/platformio.yml, so the proof is branch + SHA + flash log, and there is
  deliberately no run URL.

  OUTSTANDING: criterion 6 (turret observation) -- a 3 minute re-run of this same binary
  with Stephen watching captures it, and the sketch is unchanged. 0122 and 0123 remain
  staged awaiting verification; this task does not depend on either.

  ADDENDUM -- THE BENCH FAULT, AND THE RE-RUN WITH IT FIXED. Recorded 2026-10-07 while
  this reply was still staged and unverified; everything above is left exactly as taken.
    - THE FAULT. The 1.6k series resistor in the wire to P17 (TX) had been DISCONNECTED.
      Stephen found it, reconnected it, and confirms it was already reconnected when the
      re-run below started. Nothing transmitted in the original run's Phase B reached the
      LiDAR. This also reaches back: 0122's 5 kHz PWM on blue and 0123's three command
      sequences went through the same resistor, so their "no spin / no response" results
      were also measured through an open circuit. Both files carry a pointer here.
    - THE RE-RUN WITH IT CONNECTED (same binary 9bdd0a6b, re-flashed, 195 s): 22 Phase A
      + 15 Phase B + 6 parity lines + both long windows, 'sweep done best=460800/1
      fa_total=720 parity_pass=1'. Every probe's 0xFA count is again at or below the
      ~1/256 chance rate for its own byte count, and the long windows sit at 0.25% and
      0.24% against 0.39%. My working threshold: a real frame stream has 0xFA at the
      packet rate, i.e. tens of percent, not tenths.
    - WHAT THE FAULT DID AND DID NOT CHANGE. It invalidates the TRANSMIT half of the
      original run and of 0122/0123, so "the unit ignores us" was previously untested.
      It does not touch the RECEIVE half: no rate, polarity or parity decoded, before or
      after. With the wire intact the transmit half has now been tested and the answer is
      still no response -- 15 combinations had the 0123 sequences delivered at the
      specified baud over a connected line, and the byte stream is unchanged from the
      listen-only rows. So the conclusion is now trustworthy rather than suspect, and it
      is the same conclusion.
    - STILL NOT OBSERVED: the turret, for either 0124 run. Criterion 6 stays unmet and no
      spin claim is made. A 3 minute re-run of this binary with Stephen watching captures
      it; the sketch is unchanged and the bench is currently in the state that run
      would need.
---
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
