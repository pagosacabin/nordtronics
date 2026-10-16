---
task_id: "0139"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
proof: []
notes: |
  PICKED UP by the mailbox worker, 2026-10-16 15:15 UTC (off-peak; PEAK: OFF-PEAK
  15:15 UTC). Protocol MATCH (1.0.0). The filed file carried no front-matter at
  all; the protocol fields were added at pickup and `expect-reply-within: 6h`
  was folded into them.
---

# 0139 — Submit Upwork proposal: Solar LED Prototype

## Context

Stephen approved a proposal for the Upwork posting "Electronics Engineer for Solar LED Prototype" ($1,000 fixed, job `~022110739515688940235`, https://www.upwork.com/jobs/~022110739515688940235). Juno drafted it; Juno cannot submit (Upwork Cloudflare-blocks Juno's browser). This task is the explicit write authorization Stephen's 2026-10-01 Upwork-MCP fence requires: until now Hermes's Upwork MCP work was read-only by rule ("no proposals, no messages, no Connects spent — any write action needs a separate explicit task"). This is that task.

Cost: submitting spends 15 Connects from Stephen's balance of 42. Stephen approved the spend.

## Task

1. Verify write scope FIRST on the Upwork MCP DEFAULT profile (the interactive-OAuth profile on this laptop — never the cronrunner profile). Confirm the token can submit a proposal. If writes are not available, STOP: do not attempt workarounds, do not try another profile, do not touch the browser. Report back that write scope is unavailable.
2. If writes are verified: submit the proposal text below verbatim to the job URL above, as a fixed-price proposal with milestones 300 / 400 / 300 and the durations given in the text. Confirm the 15-Connect charge.
3. Do not edit the proposal wording. If the posting is closed, filled, or the seat is gone before you submit, report that instead of submitting.

## Proposal text (submit verbatim)

Hi — this is right in my wheelhouse. I build off-grid solar + LiFePO4 systems through my own shop (Nordtronics): custom LiFePO4 packs with JK BMS, solar charge management, and ESP32 firmware with real deep-sleep power budgets. I've got a solar-powered LoRa sensor running in the field right now.

Your screening questions:

**Standby current:** design target under 50 µA in sleep — MCU in deep sleep (~5 µA) plus charger quiescent and divider leakage. I'll put a meter on the built unit and report the actual number before it ships.

**Sub-freezing charge block:** NTC thermistor on the pack wired to the charger's temp-sense input, so charging is inhibited in hardware below 0°C, with a second check in firmware from the MCU's own temperature reading as backup. A frozen LiFePO4 charge is the one thing you never want to get wrong.

**Milestones:** I'd split it 300 / 400 / 300 — (1) schematic review + design sign-off, about a week; (2) working prototype on my bench, dusk-to-dawn cycle and logging verified, about a week and a half; (3) unit in your hands and verified, with wiring diagram, code, and recommendations.

**Shipping:** I'm in Colorado — USPS/UPS to Connecticut, on a truck in time for your Nov 20 date.

One honest note: my Upwork history here is thin (a few proposals, no contracts yet), so I'm leaning on the bench, not the profile. Happy to walk through any of the above on a call.

— Stephen

## Success criteria

- Either: proposal submitted to the exact job URL above, with 15 Connects spent (balance 42 → 27) confirmed from the account.
- Or: a clear report that write scope is unavailable on the DEFAULT profile, or that the posting closed/filled before submission. Both are acceptable outcomes — do not force a submit.

## Constraints

- DEFAULT Upwork MCP profile only. Never cronrunner, never browser automation.
- No edits to the proposal text.
- This authorization covers this one submission (15 Connects) only. It does not open general write access.

## Proof

- API response or screenshot showing the submitted proposal (proposal ID/URL), plus Connects balance before and after. Pasted terminal output is a claim, not proof — show the account-side numbers.

## Reply format

Staged reply `mailbox/staged/0139-upwork-submit-solar-led-proposal.md` with: submitted (yes/no), proposal URL or ID if yes, Connects before/after, and if not submitted, the exact reason. Include the notes field.
