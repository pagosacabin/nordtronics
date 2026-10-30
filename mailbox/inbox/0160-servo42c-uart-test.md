# 0160 — MKS SERVO42C UART protocol + ESP32 test sketch

## Context
Stephen has an MKS SERVO42C closed-loop stepper driver on a NEMA 17 (OLED live, showing shaft angle / following error / encoder clocks) wired to a classic ESP32 devkit: ESP32 TX2/RX2 to the driver's RX/TX, common GND, 12–24V VIN to the driver power stage. He is moving from step/dir pulse control to the driver's UART serial interface so the encoder feedback is actually used. The UART header is a JST-SH 1.0mm 4-pin (3V3, G, TX, RX). The MKS APT V1.0 adapter is NOT in the serial path — wire the sketch for direct ESP32-UART2 to driver-header connection.

## Task
1. Research the MKS SERVO42C UART protocol from authoritative sources (Makerbase/MKS official docs, wiki, or repo). Determine the default baud rate, the frame/command format, and the usable command set — at minimum: query shaft angle, query following error, absolute move, relative move, set-zero, motor enable/disable. Do not guess the baud rate or command bytes; every protocol claim must cite its source.
2. Write an ESP32 Arduino/PlatformIO test sketch on UART2 that runs a bench self-test on boot: open UART at the researched baud rate, query version/identity if the protocol supports it, read and print shaft angle + following error, command a +90 degree move, wait for settle, read back the angle and check it moved, command return to zero, and print a PASS/FAIL line per step over USB serial at 115200.
3. Compile-verify with PlatformIO (esp32dev board). Stephen runs the live motor test on the bench himself — no live hardware interaction in this task.

## Success criteria
- Sketch compiles clean under PlatformIO for esp32dev (no errors).
- Baud rate and all command bytes trace to cited MKS sources; PROTOCOL.md lists the command table with one source URL per row.
- Each test step prints an unambiguous PASS or FAIL line.

## Constraints
- Default model tier (keep the spend ordinary).
- Read-only with respect to everything except the deliverable branch. No credentials, no secrets, no network changes.
- 3V3 logic on the UART lines; nothing 5V touches them.

## Proof
- Branch `hermes/0160-servo42c-uart` pushed to origin containing the sketch and PROTOCOL.md.
- PlatformIO build log (paste the tail) or CI run URL proving a clean compile.

## Reply format
Staged reply with: branch name + tip SHA, build proof, the determined baud rate, and any protocol ambiguities found (note where sources disagree, if they do).

expect-reply-within: 6h
