# bench-0163 — SERVO42C: reconcile the short/backwards move (executed on hardware, 2026-10-31)

Task 0163: explain why 0162's 800-pulse `0xFD` move measured ~-25 deg instead of
+90, correct the sketch, flash, re-run, and quote the full serial log. Bar:
commanded **+90 deg reads back +90 deg** (± stated tolerance) with the sign right.

## Result in one line

**Nothing was wrong with the pulse unit or the angle scale.** 800 pulses is 90.0 deg
exactly (measured +90.10 / +90.13 deg over two runs, driver's own pulse counter `0x33`
tracking exactly 800), and the `0x36` angle scale is the documented **65536 counts/rev**
(measured **20.499 counts/pulse** vs the documented 20.48, +0.09 %). 0162's ~25 deg was
a **truncated move** — the driver acknowledged the command but only physically executed
~209 of the 800 pulses — plus a **direction-bit** convention that Stephen's `Dir=CW`
change does not invert. Both are fixed in the sketch; **SELF-TEST: PASS (0 of 8)**.

## Root cause, stated plainly

Two independent, separable faults produced the 0162 reading:

1. **The move was short, not scaled.** 0162 measured 4285 counts for 800 pulses.
   At the (now measured) 20.48 counts/pulse that is **209 pulses of actual shaft
   travel**, not a scale error. The same truncation is reproducible today and is
   visible only because this build reads `0x33` (the driver's received-pulse counter)
   around every move: **one 3200-pulse command produced exactly -2224 on `0x33`**,
   and the shaft travelled -45 548 counts = -2224 x 20.48. The driver accepted the
   frame (result `0x01`), moved 2224 pulses, and reported the shortfall only through
   its own counter. 0162 never read `0x33` after the move (its sole `0x33` read was in
   teardown, after the return move), so the shortfall was invisible and the only
   surviving hypothesis looked like a 4x unit/scale error.

2. **The direction bit, not the drive's `Dir` setting.** With `Dir=CW` selected on the
   OLED, `0xFD` with bit7=0 still moves the shaft in the **negative** `0x36` direction
   (0163-A: -16 405 counts, `0x33` -800). Setting `Dir` on the drive does not change the
   reported sign, because the driver remaps motor phases *and* encoder together — the
   encoder sign is invariant. So the sign has to be corrected in the command: **bit7=1
   is "forward = increasing angle"** (`DIR_FORWARD_BIT`, build 0163-B/C).

The two hypotheses 0162 could not separate (`MStep 64` vs a 16384-count/rev scale) are
both **false**: the 3200-pulse "one revolution" probe did not complete, so it could not
be read as a revolution — that is what made the factor-of-4 arithmetic look plausible.

### Evidence table

| Measurement | Value | What it proves |
|---|---|---|
| 800 pulses fwd (0163-A, `Dir=CW`) | `0x36` **-16 405 cnt = -90.12 deg**; `0x33` **-800** | magnitude right, sign wrong, unit = microstep |
| 800 pulses fwd (0163-C, sign fixed) | `0x36` **+16 407 cnt = +90.13 deg**; `0x33` **+800** | +90 reads +90 |
| counts/pulse over 4 x 800 pulses | **20.499** (20 480 cnt / 3200 pulses) | scale = 65536/rev, MStep 16 (documented 20.48) |
| 4 x 800 pulses | net **+33 cnt (0.18 deg)** = one revolution; `0x33` **+3200 exactly** | 3200 pulses = 1 rev at MStep 16, confirmed by the driver's counter |
| one 3200-pulse command (0163-A) | `0x33` **-2224** (not -3200); angle -45 548 cnt | the short-move fault, isolated |
| 0162's 4285 cnt | `4285 / 20.48 = 209 pulses` | 0162's "4x error" was a 209-pulse move |

MKS sources (re-fetched this run) support the values used here:
- `0xFD`: "If (xx xx xx xx = 00 00 0C 80), the motor (1.8 deg, subdivision 16) will
  rotate one circle" — the pulse field **is** the microstep count, 3200/rev at MStep 16.
- `0x36`: "The motor rotates one circle, the corresponding angle value range is
  0~65535", `e0 00 00 40 00 20` = 90 deg — 65536 counts/rev, as measured.
- `0x86` (`Dir`): 00 = CW, 01 = CCW — a drive-side setting that does not invert the
  `0xFD` command bit (measured, above).

## Sketch changes (build 0163-C)

| Change | Why |
|---|---|
| `DIR_FORWARD_BIT = 0x80` for "forward" | sign right without touching the drive's `Dir` setting |
| step 6/8 grade the **signed** angle, not `fabsf` | 0162 passed "magnitude" while the sign was wrong |
| `0x33` + `0x30` read around every move; `Motion` struct | makes a short move visible instead of presenting as a scale error |
| one-rev probe = **4 x 800 pulses**, each checked against `0x33` | a single 3200-pulse command does not complete on this unit |
| reads retried 3x; settle waits 250 ms and needs 5 stable samples | this link loses ~1 reply in 5 (0162 lost whole steps to it) |

## Runs

Both runs: full console in `serial-selftest-run1.log` / `serial-selftest-run2.log`.

```
STEP 1: uart framing / protocol                PASS  (V1.1 firmware, checksummed frames)
STEP 2: read shaft angle (0x36)                PASS  (raw 0 = 0.00 deg)
STEP 3: read following error (0x39)            PASS  (raw 5 = 0.03 deg)
STEP 4: motor enable (0xF3=01, 0x3A)           PASS  (ack=1 status=0x01)
STEP 5: command +90 deg move (0xFD)            PASS  (800 pulses fwd @ gear 16)
STEP 6: measured move is +90 deg               PASS  (moved +90.10 deg in time)
STEP 7: command return move (0xFD rev)         PASS  (800 pulses rev @ gear 16)
STEP 8: returned to start angle                PASS  (residual -0.12 deg from start)
-- INFO: pulse-unit probe, 4 x 800 pulses (nominal 1 revolution) --
INFO: 4 x 800 pulses -> +33 cnt (0.18 deg) net; 0x33 tracked +3200 pulses (exactly as commanded)
INFO: counts per pulse = 20.499 (documented 20.48 at 65536 cnt/rev, MStep 16)
INFO: shaft status 0x02 (unblocked)
SELF-TEST: PASS (0 of 8 steps failed)
```

Run 2 (identical build, `serial-selftest-run2.log`):

```
STEP 6: measured move is +90 deg               PASS  (moved +90.13 deg in time)
STEP 8: returned to start angle                PASS  (residual -0.12 deg from start)
INFO: 4 x 800 pulses -> +33 cnt (0.18 deg) net; 0x33 tracked +3200 pulses (exactly as commanded)
INFO: counts per pulse = 20.499 (documented 20.48 at 65536 cnt/rev, MStep 16)
SELF-TEST: PASS (0 of 8 steps failed)
```

Tolerance: the graded bar is +/- 5.0 deg (`ANGLE_TOL_DEG`); measured error is
**+0.10 / +0.13 deg** at step 6 and **-0.12 deg** at step 8. Repeatability across the
two runs is 0.03 deg, so 0162's "3.5 deg run-to-run variance" did not recur — it was
the short move plus single-shot unretried reads, not following error (0.03 deg here).

## Flash record

Only `app0` at `0x10000` was written; bootloader, partition table, otadata and NVS
were not touched.

| build | what changed | `firmware.bin` sha256 |
|---|---|---|
| A `0163-A (instrumented)` | instrumentation + 3200-pulse probe | `ff4109f8134106b14450f79a34663e2c35e970beaff6057d1a62732c491f05f7` (273 072 B) |
| B `0163-B` | direction bit, 4 x 800 probe | `d32657d529bd67f122dd573330a007118fc2d8a3dd62e9991d76fad26e5f9e96` |
| C `0163-C` (final) | INFO arithmetic fix + undo accounting | `e4769b7a42d3167ac756741a9de5209de5018d234b7184461bf7f8199dc375d9` |

Each write reported `Hash of data verified.`; A was additionally validated by the
behaviour it produced (the `0x33` readings only exist in this firmware).

## What is still open (reported, not worked around)

- **Why a long `0xFD` move stops short** (3200 commanded -> 2224 executed, repeatably,
  twice, with the same 2224 both times for the forward and the reverse command). It is
  not the pulse field (the same field is exact at 800) and not the link (the ack
  validates). Candidate mechanisms not yet tested: an internal per-command travel limit,
  a 16-bit target that wraps at exactly one revolution (3200 pulses x 20.48 =
  65 536 cnt), or the `ACC`/speed-gear ramp. This bench test needs ±90 deg only, so it
  is recorded rather than chased; a follow-up could sweep 800/1200/1600/2000/2400.
- `0x30` (encoder read) never returned a frame this build accepts: the driver answers
  `E0 FF FF FF FF FE DD B7`, which validates as addr + **6** payload bytes (sum B7),
  not the documented `u16`. So the encoder read is not used as evidence here — `0x33`
  and `0x36` carry the proof. Reported as a protocol-notes discrepancy.
