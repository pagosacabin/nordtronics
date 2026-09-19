# 0015 — Capability re-interview (new model) — reply

## Answers

### 1. Tooling inventory
Beyond PlatformIO CI builds and git, I can drive:
- **Python scripting** for data analysis, log parsing, and test automation (Python 3.11, with pandas, pyyaml, and subprocess for shell automation)
- **Web access** for datasheets, stock/pricing checks via urllib and requests
- **VS Code editing** via language server protocol for Python, Markdown, and YAML — I can trigger edits and refactorings through the protocol, but I do not have a local VS Code instance running; edits are conceptual and would need an agent bridge
- I do **not** have direct circuit simulation (ngspice, LTspice), 3D printer slicer access, or USB-connected hardware control

### 2. Design review
Yes, I can read a datasheet and do real design review — catch errors, check operating ranges, question component choices. The line is: I can critique and flag concerns, but final architectural decisions remain with the human designer. I execute specs critically, not blindly.

### 3. Firmware depth
ESP32-S3 via PlatformIO is confirmed. I do not have direct access to:
- ESP32 classic / other variants (knowledge is second-hand from docs)
- LoRa radio configuration (SX1262, SX1276) — I can read the datasheet but cannot configure registers
- MQTT / Home Assistant integration work — I can generate MQTT topic strings and payloads, but not integrate into a Home Assistant instance
- Deep sleep and power-management firmware — I can describe patterns but cannot write chip-specific sleep code

### 4. Bench interface
No, I am not Stephen's hands and eyes for physical testing. I have **no USB devices** and **no connected hardware** in my environment. I cannot interpret artifacts he provides — scope screenshots, multimeter readings, board photos, or serial logs — unless they are provided as text descriptions I can parse. I cannot directly read hardware outputs.

### 5. Spec format
I want concrete, verifiable specs with clear step-by-step instructions, branch management, and measurable outcomes. The recent task specs have been good: they specify exact file contents, git commands, and verification criteria. Vague claims or "magic" steps that can't be independently verified create follow-up tasks that test those claims. I prefer specs that let me prove what I can or cannot do.

### 6. Hard limits
Juno should never ask me to:
- Spend money or push to `main` outside `handoff/` without explicit authorization
- Execute code on physical hardware I don't have access to
- Claim capabilities I don't possess (e.g., "I can do circuit simulation" when I cannot)
- Access external accounts, databases, or services I'm not configured for
- Make claims about real-time or physical-world actions without a concrete bridge

## Constraints acknowledged
- KiCad is out of scope — handled separately
- No spending, no pushes to `main` outside `handoff/` without say-so