---
task_id: "0250"
title: Per-channel DAC calibration — measure the real ladder levels, build per-channel quant tables
status: in_progress
iteration: 2
expect-reply-within: 24h
proof: []
notes: Filed by Juno 2026-11-23 on Stephen's word ("Go ahead", 12:32 MST) after Juno's assessment of the external Claude review he shared. The 24h window is because rung 2 needs human hands at the bench. Follow-on already agreed in direction: the error-diffusion A/B comes after this, using the calibrated levels — it is NOT part of this task.
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-23T19:46:00+00:00"
lease_until: "2026-11-23T21:16:00+00:00"
---

## Assignment

- **Hermes:** rung 1 (test frames + code restructure), rung 3 (build the measured tables in, before/after panel photos), the Copilot step, and the reply.
- **Stephen:** the rung-2 meter readings — his hands on the probes, or Hermes's with Stephen at the bench. If nobody is at the bench when the worker picks this up, do rung 1, park in active/ with the procedure written and the test frames ready, and say so plainly. Never estimate or fabricate a voltage.
- **Copilot:** peer review of the cabin-play diff (GitHub-bound), driven by Hermes via Herdr. Reviewer, not the proof.
- **Juno:** verifies independently — branch SHA against origin, the voltage table against the reply, before/after photos present.

## Context

The living frame's colour pipeline (on the camera board, `living-frame/firmware/freenove-cam`) quantises through tables built from assumed ideal DAC levels: `L = {0, 0.0891, 0.4097, 1.0}`, i.e. `(v/3)^2.2` for codes 0–3, and all three channels get identical tables. The real resistor ladder on the frame board (2-bit DAC: R GPIO 4/5, G 6/7, B 15/16) will be off those ideals by a few percent, the GPIO output resistance shifts the steps further, and full scale may not be 0.7 V. A per-channel level error is a plausible mechanism for the cool cast seen on the panel (separate layer from white balance — 0249 shipped WB per-shot; the WB-strength knob is not touched here).

This task comes from an external review Stephen shared and Juno assessed in chat 2026-11-23; Stephen approved filing the calibration rung first. The display side itself is settled (0243 accepted; 800×600 at exact 2× on the SyncMaster 213T) and is not reopened here.

## Task

**Rung 1 — prepare (no bench hands needed).**
- Generate 12 test frames as .BIN files in the accepted packed format: for each channel (R, G, B) and each code (0–3), a solid frame with that code on that channel and 0 on the other two.
- Back up the frame SD's `/sdcard/FRAMES` set to the laptop before touching it; the test frames go in only for the measurement session and the original set is restored byte-for-byte afterwards (verify by file count and, where available, hashes). The LIVE library (36 frames at last count) is not touched.
- Restructure the quant-table build in freenove-cam so each channel's table is built from that channel's own level array, with the ideal `{0, 1/3, 2/3, 1}` voltage ratios as the shipped defaults — behaviour-identical until rung 3 supplies measured numbers.

**Rung 2 — measure (bench hands).**
- With the monitor connected (its termination is part of the divider), display each test frame and read the channel's voltage at the VGA connector pin (or DAC output node) with a DC multimeter. Take each reading at least twice; record both.
- A DC meter averages over blanking, which scales every reading by the same active-frame fraction — that is acceptable because the tables need only the ratios `V[v]/V[3]` within each channel. Record code 0 as a sanity floor (expect ~0 V).
- The deliverable of this rung is the 3×4 voltage table, recorded in the reply, with full scale per channel stated.

**Rung 3 — build and prove.**
- Build each channel's quant table as `L_c[v] = (V_c[v] / V_c[3])^γ` with γ = 2.2 (γ is not a variable in this task). Flash the camera board, shoot the same scene used for the recent panel photos, and attach before/after panel photos under `mailbox/files/0250/`.
- Confirm nothing else moved: transfer timings and stored-frame hashes behave as in 0249 (this change is camera-side processing only; the .BIN format and the link are untouched).

## Success criteria

- The reply carries the full 3×4 measured voltage table (two readings per cell) with the monitor connected, and full scale per channel.
- The three per-channel tables in the shipped firmware differ from each other exactly as the measurements dictate; γ = 2.2 stated.
- Before/after panel photos of the same scene are attached, and the FRAMES restore is verified.
- A null result is a valid result: if the measured ratios are within meter noise of ideal on every channel, ship the code restructure with the ideal defaults, report the table, and say plainly that the ladder is not the cast's cause.

## Constraints

- Camera firmware only (`living-frame/firmware/freenove-cam`) for the table change. Frame firmware, the .BIN format, and the ESP-NOW link are untouched.
- White balance behaviour and strength are not changed in this task — one variable at a time.
- Cabin-play branch `hermes/0250-dac-calibration`; Copilot peer review before the push is called done; `git fetch` before any push.
- No fabricated measurements: an unmeasured rung is reported as unmeasured.
- Secrets never in files. Flash tier is fine.

## Proof

- Branch SHA on origin (Juno reads it back).
- The voltage table in the reply.
- Before/after panel photos at `mailbox/files/0250/`.

## Reply format

Status line first. The voltage table. What the tables became. The photos. Deviations plainly.
