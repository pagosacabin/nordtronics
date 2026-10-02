---
task_id: "0094"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
---

# 0094 — Base-station firmware: LoRa gateway + v0.2 consensus + MQTT uplink

# Context

The bench validated the full v1 sensing stack on 2026-10-02: BME680 (I2C 0x77,
VIN on Vext, SCK→GPIO17 / SDI→GPIO18 — verified, do not flip) and PMS5003 (UART
TX→1kΩ→GPIO5, bench 5 V, checksummed frames) both live on one Heltec LoRa 32 V4,
with sane readings and a real solder-fume event detected and correctly
classified as fine-particle-dominated. Now the node needs something to talk to.

Architecture (decided by Stephen 2026-10-02, fixed): the **base station runs
consensus/detection**; the backend persists and serves alerts; the app displays
and acknowledges only, never decides. Phase 1 base hardware is another Heltec
LoRa 32 V4, USB-powered, no sensors — it bridges LoRa → WiFi/MQTT → VPS and
Home Assistant.

Evidence this firmware must encode (from 0091/0092, both verified and archived):
- 0091 sim: v0.1 rules miss slow-ramp burn piles (baseline chasing), flap with
  mid-plume auto-clears, and worst-case detection latency is ~43 min, not ≤25.
  v0.2 fixes: **freeze each node's baseline on first suspected rise**,
  **post-clear re-arm cooldown**, keep the 2-packet / 25 µg/m³ validation rule.
- 0092 budget: 12-minute cadence closes on energy (219.3 mAh/day); worst-case
  latency 45 min at 12-min cadence. Two independent analyses agree: publish
  ~45 min, never ≤25.
- Radio protocol was the open decision ("everything hangs off it"). This task
  closes it: **versioned packed-binary packets**, node IDs provisioned at the
  bench, ACKs for alarm packets only, 12-minute routine check-ins, node marked
  offline after 3 missed check-ins (36 min). All radio params are captive-portal
  fields (tank-monitor pattern: firmware default + portal field + NVS).

The single open parameter is the correlation window: 0091 validated 20 min and
recommended widening, with no number attached. Default it to 20 min and make it
portal-tunable (10–60 min); flag the widen question for Stephen, do not invent
a new default.

# Task

Write the base-station firmware (Heltec LoRa 32 V4, same PlatformIO target as
the node) and the protocol doc it implements:

1. `docs/wildfire/radio-protocol-v1.md` — versioned packed-binary packet
   layout (version byte first, then node ID, sequence, PM1/PM2.5/PM10,
   temp/humidity/pressure, battery mV, flags), join/provisioning flow
   (bench-provisioned node IDs; base ignores unprovisioned IDs), ACK rules
   (alarm packets only, retries + timeout values stated), 12-min check-in,
   3-missed offline rule. Every field justified in one line.
2. LoRa RX on 915 MHz US per that doc; validate + checksum every frame.
3. v0.2 consensus engine: per-node baseline with freeze-on-first-suspected-rise;
   alert when ≥2 nodes each show ≥2 packets ≥25 µg/m³ above their frozen
   baseline inside the correlation window; single-node elevation raises Watch
   only and NEVER auto-escalates; re-arm cooldown after any clear.
4. MQTT uplink: publish per-node telemetry and Watch/alert events; broker
   host/port/user/pass/topic-root all portal fields with defaults; state and
   implement the offline policy (buffer with cap + drop-oldest vs drop — pick
   one, document it, size the buffer).
5. Captive portal (tank-monitor pattern) exposing every item above.
6. OLED status screen: nodes seen / last-seen, last event, MQTT link state.

# Success criteria

- `platformio.yml` CI builds the base target green from the branch tip.
- `docs/wildfire/radio-protocol-v1.md` exists, versioned, and the firmware
  implements exactly what it says (no undocumented fields on the wire).
- A deterministic host-side test proves the consensus engine against the 0091
  scenario set (burn-pile slow ramp must NOT be missed via baseline freeze;
  single-node extreme must raise Watch and never alert; mid-plume clear must
  not re-arm inside the cooldown). Pasted serial output is a claim, not proof —
  the test must run in CI or as a one-command script like 0091's harness.
- Every portal field has a firmware default and persists to NVS.

# Constraints

- 915 MHz US variant only. No 433 MHz anywhere (Stephen corrected this twice).
- No backend/VPS changes, no app changes, no website changes in this task.
- BME680 on the bench is the 680; production is the 688 — the base does not
  care which Bosch sensor a node carries, so do not branch on it.
- Worker cost: standard tier, off-peak preferred. State the tier used in the reply.
- One deliverable: the base-station firmware + its protocol doc, on one branch.

# Proof

- Branch `hermes/0094-base-station-firmware`, pushed; SHA on origin.
- Actions run URL for the platformio build, green at the branch tip SHA.
- Consensus test: command to run + its PASS/FAIL summary lines verbatim, and
  the CI run if it executes there.
- `reply_format`: staged file with front-matter proof (branch, sha, run URLs,
  files changed), notes field carrying scope extensions, self-caught defects,
  and anything left undone — the 0091/0092/0093 notes sections are the bar.

# Reply format

Stage `mailbox/staged/0094-base-station-firmware.md` per the mailbox protocol
(rules in `mailbox/README.md`): front-matter with proof pointers, then the
Context/Task/Success/Constraints/Proof sections above answered with what was
actually built and where.
