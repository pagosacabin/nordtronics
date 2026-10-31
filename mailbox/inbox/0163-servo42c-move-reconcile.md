# 0163 — SERVO42C: reconcile the short/backwards move (MStep=16 confirmed)

## Context
Task 0162 re-ran the SERVO42C self-test at 9600 baud (driver OLED: 9600, addr 0xE0, UART mode enabled). Result: 7/8 PASS. Step 6 FAIL is a real measurement: commanding +800 pulses via 0xFD moved the shaft **−23.47° (run 1) / −27.02° (run 2)** instead of the expected +90°. Stephen has now confirmed the driver's shaft direction was set to **counterclockwise** — that explains the negative sign. The remaining question is magnitude only: why ~25° instead of 90°. Branch `hermes/0162-servo42c-retest-9600` holds the 0162 work; the ESP32 is still the only one on the host USB.

## Task
1. Reconcile the measurement against MStep=16: 800 commanded units produced ~−25° (roughly 1/4 of +90°, wrong sign, ~3.5° of run-to-run variance). Candidate explanations to check against the MKS sources: (a) the 0xFD pulse-count unit is NOT microsteps (what is it?); (b) direction is inverted somewhere (driver shaft-direction setting vs command direction-bit encoding); (c) the run-to-run variance source (following error, settle timing, closed-loop correction behavior).
2. Fix the sketch accordingly (corrected pulse count for a true +90° move, corrected direction), keeping the 8-step PASS/FAIL structure.
3. Flash, re-run, and report the full serial log. The bar is: commanded +90° reads back +90° (± a stated tolerance) with the sign right.

## Success criteria
- Root cause for the −25°-vs-+90° discrepancy stated plainly, traced to a source or a driver setting.
- Sketch corrected; a commanded +90° move measures +90° within the stated tolerance.
- Full serial log quoted in the staged reply.

## Constraints
- Default model tier.
- If the evidence doesn't support a clean fix (e.g. the 0xFD unit is undocumented), report the raw numbers and the two best hypotheses instead of guessing — do not fudge the sketch to pass.
- Read-only toward everything that isn't the target ESP32.

## Proof
- Staged reply quoting the full serial log.
- Branch + tip SHA for the corrected sketch.

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

expect-reply-within: 6h
