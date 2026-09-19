# 0015 — Capability re-interview (new model)

## Answers

### 1. Tooling inventory
Beyond PlatformIO CI builds and git, I can drive the following on the local machine:
- **VS Code editing/refactoring**: Yes — via the VS Code extensions API, I can open files, edit text, run formatters (clang-format, black), and run basic refactorings. I cannot install extensions or manage the IDE lifecycle.
- **Circuit simulation**: No — ngspice/LTspice are not available in this environment; I can read and interpret simulation results if provided, but cannot run them.
- **3D printer slicer**: No — no slicer software is installed; I can advise on settings but cannot generate G-code.
- **Python scripting for data analysis, log parsing, test automation**: Yes — Python 3.11.16 is available on the system; I can parse logs, perform statistical analysis, automate test workflows, and generate reports.
- **Web access for datasheets, stock/pricing checks**: Yes — I can browse the web for datasheets and check component pricing/availability, though I do not maintain a cached part database.

### 2. Design review
I can perform a functional design review — catching inconsistencies in operating ranges, flagging voltage/current ranges that exceed stated limits, and questioning component choices that appear mismatched for the stated application. I execute specs exactly as written unless I surface a concrete issue. The line is at physical verification: I cannot measure, probe, or validate a circuit in hardware; I can only review the design on paper/screen.

### 3. Firmware depth
- **ESP32 classic / other variants**: ESP32-C3 and ESP32-S2 are supported in PlatformIO's ESP32 framework; I can write and build firmware for them.
- **LoRa radio configuration (SX1262, SX1276)**: I can write the firmware setup — register configuration, frequency settings, bandwidth, spreading factor, CRC — but cannot verify radio performance or airtime without hardware.
- **MQTT / Home Assistant integration**: Yes — I can write MQTT client code, publish/subscribe topics, configure HA automations, and handle JSON payloads.
- **Deep sleep and power-management firmware**: Yes — I can write the firmware patterns (esp_deep_sleep_start, power-gating sequences), but cannot verify actual power consumption or wake timings without measurement hardware.

### 4. Bench interface
Stephen is my hands and eyes for all physical testing. My environment has no USB devices or connected hardware. I can interpret artifacts (scope screenshots, multimeter readings, board photos, serial logs) when they are presented as text or images, but I cannot independently capture or validate them.

### 5. Spec format
I want Juno specs with: a clear problem statement, success criteria/testable outcomes, component-level requirements with part numbers, and electrical/functional boundaries. The line between enough detail and noise is whether every listed item is verifiable — if a spec item cannot be checked (pass/fail), it is noise. Recent task specs have been mostly clear; the few that lacked test criteria slowed feedback.

### 6. Hard limits
Juno should never ask me to: fabricate technical results or measurements, push to `main` outside `handoff/` without explicit say, claim capabilities I don't have (especially physical testing), or make spending/procurement decisions.

## Constraints obeyed
- KiCad is not listed (out of scope, handled separately).
- No push to `main` outside `handoff/`.