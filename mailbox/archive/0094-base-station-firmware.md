---
task_id: "0094"
protocol_version: 1.0.0
status: verified
iteration: 1
proof:
  - branch: hermes/0094-base-station-firmware
    sha: 64bfcfdd1f78bd859521727fff1a1cf1d2a3c8af
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37060775921
  - jobs: build (green, incl. "Build firmware (wildfire-node-v1, unified node+base)" and the artifact upload); host-tests (green, incl. "Run host-side firmware tests (native env)" and the scenarios_v02.h freshness guard)
  - artifact: https://github.com/pagosacabin/nordtronics/actions/runs/37060775921/artifacts
    artifact_name: wildfire-node-v1-unified-firmware (id 11251240032, 675534 bytes, sha256:9d255a3b10e668675b0df995edb01411076cc371b7c2e38135ca0443320ca460)
  - host_tests: "pio test -d firmware/wildfire-node-v1 -e native -> 26 test cases, 26 succeeded (0 failed)"
  - ntfy:
      topic: nordtronics-build-ed05a663
      id: oT2f1V7xtzZx
      time: "1790973051 (2026-10-02T20:30:51Z)"
  - files:
      - .github/workflows/platformio.yml
      - docs/wildfire/radio-protocol-v1.md
      - firmware/wildfire-node-v1/platformio.ini
      - firmware/wildfire-node-v1/README.md
      - firmware/wildfire-node-v1/src/main.cpp
      - firmware/wildfire-node-v1/src/consensus_v02.h
      - firmware/wildfire-node-v1/src/consensus_v02.cpp
      - firmware/wildfire-node-v1/src/radio_protocol.h
      - firmware/wildfire-node-v1/src/radio_protocol.cpp
      - firmware/wildfire-node-v1/src/role_detect.h
      - firmware/wildfire-node-v1/src/role_detect.cpp
      - firmware/wildfire-node-v1/src/firmware_config.h
      - firmware/wildfire-node-v1/test/test_consensus_native/test_main.cpp
      - firmware/wildfire-node-v1/test/test_consensus_native/scenarios_v02.h
      - firmware/wildfire-node-v1/test/test_radio_protocol/test_main.cpp
      - firmware/wildfire-node-v1/test/test_role_and_portal/test_main.cpp
      - python/detection-sim/traces.py
      - python/detection-sim/emit_v02_scenarios.py
      - python/detection-sim/.gitignore
notes: >
  Delivered as one branch off main: the unified firmware (firmware/wildfire-node-v1),
  its protocol doc (docs/wildfire/radio-protocol-v1.md), three host-side unity test
  suites, and the CI wiring that builds the target and runs the tests.
  Corrections and deviations, all declared: (1) the v0.2 baseline-freeze trigger is the
  packet that reaches the ABSOLUTE FLOOR, not the relative-rise packet -- the task's
  phrase "freeze on first suspected rise" is ambiguous and the literal reading provably
  does not work: a relative-rise trigger is by definition a packet the chased baseline
  has already defeated, and it fires 0 times on the burn-pile trace, where the
  floor-crossing trigger raises a Watch at t=264 min. Mechanism and measurement in
  docs/wildfire/radio-protocol-v1.md section 7. (2) "single-node elevation raises Watch
  only" is implemented as: one confirmed node = WATCH and it can never become an ALERT
  by itself however extreme; only a second node confirming inside the correlation window
  escalates. (3) A PlatformIO `test_filter` with multiple space-separated names silently
  ran ZERO test cases (0 succeeded, exit 0) -- removed, and the trap is noted in
  platformio.ini. (4) A __pycache__ bytecode file was accidentally committed in the
  first branch commit and removed in a follow-up commit; no amend, no force-push.
  (5) In the first draft `require_two_nodes` was inert, which made the bbq control
  assertion vacuous; it now selects the Alert threshold and the control is meaningful.
  Scope extensions beyond the letter of the spec, each with its criterion as the reason:
  the CI workflow needed firmware/wildfire-node-v1/** and python/detection-sim/** in its
  path triggers plus a host-tests job and a firmware artifact upload (the "platformio.yml
  CI builds the base target green from the branch tip" criterion, and the artifact pointer
  the ntfy receipt needs); python/detection-sim/traces.py is carried onto this branch
  because it exists only on hermes/0091-detection-sim-harness (main carries 0 files under
  python/detection-sim) and the consensus test must run the SAME trace set 0091 measured.
  NOT DONE, and it is a leg of a success criterion: no bench hardware is reachable from
  this worker, so "serial log shows the detected role and its source" rests on the
  unit-tested exact log STRINGS and the truth table, not on a hardware capture. The three
  lines a capture must show are quoted verbatim in the reply body; flashing a board and
  capturing them is outstanding. Also outstanding (bench items listed in
  firmware/wildfire-node-v1/README.md): TCXO voltage 1.8 V and DIO2 RF-switch control on
  the V4.2 module, GPIO7's availability on the header, and batt_mv (Rev C has no fuel
  gauge so it transmits 0). The correlation-window widening 0091 recommended is left at
  the default 20 min with a portal range of 10-60 and is NOT guessed at: it stays flagged
  for Stephen as the task requires. No repository file outside the list above changed.
  Worker tier: DeepSeek standard (flash) tier; this run was off-peak (the pre-run script
  injected `PEAK: OFF-PEAK 20:15 UTC`).
---

# 0094 — Unified v1 firmware: auto-detected node/base roles (LoRa + v0.2 consensus + MQTT)

## Context (what was built on)

- The bench-validated v1 sensing stack from 2026-10-02 is encoded as fixed wiring:
  BME680 on I2C (SDA `GPIO17`, SCL `GPIO18`, address `0x77`) with the SSD1306 OLED
  (`0x3C`) sharing that bus, and PMS5003 UART TX → 1 kΩ → `GPIO5`. Nothing was flipped.
- 0091's scenario set is the test input: `python/detection-sim/traces.py` is used
  verbatim, and its 10 scenarios are frozen into
  `firmware/wildfire-node-v1/test/test_consensus_native/scenarios_v02.h` by
  `python/detection-sim/emit_v02_scenarios.py` (CI fails if the generated header is stale).
- The strap pin was chosen against the 0079 frozen nets rather than by preference.

## Task

### 0. Role detection (first thing that runs)

`src/role_detect.{h,cpp}` is a pure decision function: NVS/portal value wins, then the
strap, then the default. It runs before any radio, sensor or power init.

| case | strap GPIO7 | NVS `role` | result | source |
|---|---|---|---|---|
| jumper to GND | LOW (0) | unset | base | `STRAP` |
| unstrapped bench board | HIGH (1) | unset | node | `DEFAULT` |
| NVS/portal override | HIGH | 1 | base | `NVS` |
| NVS/portal override | LOW | 0 | node | `NVS` (override beats the jumper) |
| unusable NVS value | LOW | 7 or -1 | base | `STRAP` (falls through) |

The serial line on every boot, asserted verbatim by `test_role_serial_lines`:

```
ROLE: base (source=STRAP, strap_pin=GPIO7 level=0, nvs_role=unset)
ROLE: node (source=DEFAULT, strap_pin=GPIO7 level=1, nvs_role=unset)
ROLE: base (source=NVS, strap_pin=GPIO7 level=1, nvs_role=1)
```

**Strap pin = GPIO7.** Justified by elimination against the 14 frozen nets of the 0079
Rev C capture: U1 pins 1–14 are `BAT, GND, SOLAR, 3V3, GPIO36, GPIO17, GPIO18, GPIO4,
GPIO5, GPIO6, GPIO33, GPIO47, GPIO48, GPIO34` (pins 15–42 carry no-connects), and GPIO7
is in neither set. It is also clear of every ESP32-S3 pin that makes a bad input strap:
`GPIO0/3/45/46` (boot strapping), `GPIO19/20` (native USB), `GPIO43/44` (UART0 log),
`GPIO8–14` (SX1262 SPI), `GPIO17/18` (I2C). `test_strap_pin_does_not_collide` asserts each
of those exclusions mechanically, so the claim cannot rot when the pin map moves. No PCB
change is needed: the NVS override covers an unstrapped board and Rev C gets a solder jumper.

**HARD RULE: the base never deep-sleeps.** `enter_deep_sleep()` is the only sleep path in
the firmware; it asserts `role == node`, and as base it logs
`FATAL: deep sleep requested while role=base — refusing (base must never sleep)` and
returns rather than sleeping. A misdetection therefore fails loudly instead of going deaf.

### 1. `docs/wildfire/radio-protocol-v1.md`

Versioned packed-binary frames (`protocol_version: 1`, version byte first). Every field
carries a one-line justification, and section 1.7 publishes golden frame hex that
`test_radio_protocol` asserts byte for byte, so the doc and the wire cannot drift:

```
CHECKIN  node=0x1234 seq=7 flags=0x02  pm1=1.2 pm25=13.7 pm10=42.1 temp=22.34C
         rh=45.12% press=101325Pa batt=3990mV status=0x07
         01013412070011020c008900a501ba08a011cd8b0100960f07e844   (27 bytes)
ACK      node=7 seq=9 acked_seq=0x0123 code=0    01030700090003002301009b25 (13)
JOIN_REQ node=7 seq=9 hw_hash=0xDEADBEEF         0104070009000400efbeaddef261 (14)
JOIN_ACK node=7 seq=9 id=0x0042 base_seq=0x0011  01050700090004004200110066fe (14)
```

Layout: 8-byte header (version, type, node_id u16 LE, seq u16 LE, payload_len, flags),
type-specific payload, CRC-16/CCITT over everything before it. The CHECKIN/ALARM payload is
17 bytes: PM1/PM2.5/PM10 ×10, temperature ×100 signed, RH ×100, pressure in Pa, battery mV,
status bitfield. Join/provisioning: node IDs are bench-provisioned and the base ignores every
frame from an ID outside its `prov_ids` allowlist (an empty allowlist is the bench-open
state). ACK rules: alarm packets only, 5 s timeout, 3 retries (4 attempts), with `ack_code`
distinguishing a refusal from silence. 12-minute check-in, offline after 3 missed check-ins
(36 min). 915 MHz US only — the firmware logs a FATAL line and refuses any configured
frequency outside 902–928 MHz.

### 2. LoRa RX and frame validation

`radio_protocol.cpp` validates the version, the type, the declared payload length against
the type, and the CRC before any field is read; `frame_plausible()` then applies the
physical ranges (node id, PM2.5 ≤ 3000, RH ≤ 100 %, −60…90 °C, pressure, battery) and the
base's allowlist. A failed frame is logged with its rejection reason and dropped — it never
reaches the consensus engine or MQTT. `test_bad_frames_are_rejected` covers a flipped
payload bit, a corrupted CRC, a future version byte, an unknown type, a lying payload
length, a truncated frame and an oversized one; `test_plausibility_gate` covers the ranges.

### 3. v0.2 consensus engine (`src/consensus_v02.cpp`)

- **Baseline freeze on the first suspected rise.** The trigger is the packet that reaches
  the **absolute floor** (25 µg/m³) — not the relative-rise packet. This is a declared
  correction of the spec's wording, and it is the whole fix: a relative-rise trigger is by
  definition a packet the chased baseline has already defeated. Measured on the 0091
  burn-pile trace, freezing on the relative rise fires **never**; freezing on the floor
  crossing raises a Watch at **t = 264 min** (ground-truth rise at t = 60). The freeze is
  released once the node has been back below the floor for `clear_min`.
- **Alert**: ≥ 2 nodes each with ≥ 2 consecutive packets ≥ 25 µg/m³ above their frozen
  baseline, both inside the correlation window (default 20 min, portal range 10–60).
- **Single-node elevation = Watch only**, and it can never escalate itself: only a second
  confirming node raises the Alert. An extreme single-node reading changes nothing.
- **Post-clear re-arm cooldown** (default 30 min): no new Watch/Alert inside it.
- Manual acknowledge suppresses re-alerting for the same node set for 2 h, while a
  previously uninvolved node still gets through.

### 4. MQTT uplink and the offline policy

Topics under `mqtt_root` (default `nordtronics/wildfire`): `node/<id>/telemetry` on every
valid frame, `event/watch`, `event/alert`, `event/clear` on the three transitions, and
`node/<id>/state` `{"state":"offline"}` once on the transition. Broker
host/port/user/pass/root are all portal fields with defaults. **Offline policy: buffer with
a cap, drop the OLDEST record** (`offl_policy=buffer`, `offl_cap=180`). Chosen over
drop-new because the data that matters most is the data from the outage that just ended;
180 records is 15 node-hours for one node = 36 h for a 4-node property at the 12-minute
cadence, ~30 kB of PSRAM. On overflow the head is evicted and the buffer publishes in order
on reconnect.

### 5. Captive portal

`kPortalFields[]` in `src/firmware_config.h` is the single source of truth: the portal
renders exactly that table and writes each submission through to NVS, so no field can exist
in the portal and not in NVS, or in NVS and not in the portal. 30 fields, unique NVS keys
≤ 15 characters, all inside the one `wildfire` namespace.

### 6. OLED status screen

Base: level (NONE/WATCH/ALERT) plus the nodes-seen count, the MQTT link state, last-seen
minutes for the first two nodes and the last event. Node: role, id, check-in period, sequence.

## Success criteria

| criterion | evidence |
|---|---|
| `platformio.yml` CI builds the base target green from the branch tip | run [37060775921](https://github.com/pagosacabin/nordtronics/actions/runs/37060775921) @ `64bfcfdd1f78bd859521727fff1a1cf1d2a3c8af` = the branch tip; job `build`, step "Build firmware (wildfire-node-v1, unified node+base)" succeeded. The unified image **is** the base target — one binary, role decided at boot |
| `docs/wildfire/radio-protocol-v1.md` exists, is versioned, and the firmware implements exactly it (no undocumented fields on the wire) | the doc declares `protocol_version: 1`; `test_radio_protocol` asserts the frame sizes (27/27/13/14/14), every field offset against a hand-built byte array, and the golden hex quoted in the doc |
| a deterministic host-side test proves the consensus engine against the 0091 scenario set; burn-pile NOT missed via the freeze; single-node extreme raises Watch and never alerts; a mid-plume clear does not re-arm inside the cooldown | `pio test -d firmware/wildfire-node-v1 -e native` (CI job `host-tests`) → **26 test cases, 26 succeeded**. Per-scenario lines below; each scenario is paired with a control that disables the v0.2 rule, so no "nothing happened" assertion is vacuous |
| every portal field has a firmware default and persists to NVS | `test_every_portal_field_has_a_key_default_and_namespace` (30 fields, unique keys, ≤ 15 chars, one namespace, defaults present) and `test_defaults_match_the_frozen_rule_constants` (the portal default of every detection parameter equals the constant the engine was tested with) |
| both roles demonstrated from the SAME binary | role detection is a boot-time decision inside the one binary, asserted by the truth table and the exact serial strings below. **The hardware capture is outstanding** — no bench board is reachable from this worker |

Consensus test summary lines, verbatim from
`pio test -d firmware/wildfire-node-v1 -e native -v`:

```
[burn_pile_slow_ramp] v0.2 freeze=on watch=1 alert=0 confirmations(N1)=1 firstRaise=264.0 (latency 204 min from the t=60 rise) | control freeze=off watch=0 alert=0 confirmations(N1)=0
[dust_gust_single_node] v0.2 confirm=2 watch=0 alert=0 | control confirm=1 watch=1 alert=0
[bbq_single_node_extreme] v0.2 watch=1 alert=0 | control need=1 watch=0 alert=1
[wildfire_plume_multi_node] watch=1 alert=1 firstRaise=84.0
[diurnal_background_noise] watch=0 alert=0 confirmations=0 (4 nodes x 24 h)
[staggered_worst_case] watch=1 alert=1 alertAt=44.0 firstRise=1.0 latency=43.0 min
[flap_clear_rearm] v0.2 alert=2 clear=2 | control cooldown=200 alert=1 clear=1
[fresh_node_blind_window] watch=1 alert=0 confirmations(A)=1 confirmations(C)=0
[three_node_escalation] watch=1 alert=1
[ack_suppression] watch=1 alert=1 clear=1 (ack at first raise)
[rearm_cooldown] clearAt=84.0 cooldown=30 nextRaise=120.0 | control cooldown=0 nextRaise=108.0
[ack_suppress] ackedAt=36.0 alert=1 raises=3 | control suppress=off alert=2 raises=3
[invariants] clears=8 scenarios=10 -- cooldown / node-count / determinism invariants hold
CONSENSUS-TEST SUMMARY: scenarios=10 checks-failed=0
CONSENSUS-TEST: PASS

[crc] crc16_ccitt("123456789") = 0x29B1 (expect 0x29B1)
[sizes] checkin=27 alarm=27 ack=13 joinreq=14 joinack=14 (header=8 crc=2)
[golden] checkin len=27 hex=01013412070011020c008900a501ba08a011cd8b0100960f07e844
[offsets] decode=OK version=1 type=1 node=0x1234 seq=7 plen=17 flags=0x05 pm1=12 pm25=137 pm10=421 temp=2234 rh=4512 press=101325 batt=3990 status=0x07
[round-trip] checkin/alarm/ack/join_req/join_ack all encode+decode; golden hex matches
[reject] crc/payload-crc/version/type/length/truncated/oversized all rejected
[plausibility] telemetry ranges + node-id gate enforced on every decoded frame
[buffers] short encode buffers refused, no partial frames
RADIO-PROTOCOL-TEST: PASS

[role] STRAP=base / NVS->base / NVS->node / NVS-invalid-falls-through: all cases hold
[role-log] ROLE: base (source=STRAP, strap_pin=GPIO7 level=0, nvs_role=unset)
[role-log] ROLE: node (source=DEFAULT, strap_pin=GPIO7 level=1, nvs_role=unset)
[role-log] ROLE: base (source=NVS, strap_pin=GPIO7 level=1, nvs_role=1)
[strap] pin=GPIO7 frozen_nets=14
[portal] fields=30 unique_keys=30 non-empty_defaults=24
[defaults] corr_min=20 abs_floor=25.0 rel_delta=25.0 chk_s=720 offl_cap=180
ROLE-PORTAL-TEST: PASS

================= 26 test cases: 26 succeeded in 00:00:01.385 =================
```

The freeze control is the load-bearing number: `burn_pile` with
`freeze_on_suspected_rise = false` (the v0.1 rolling-median chase) produces **0** events,
exactly as 0091 measured, while v0.2 produces a Watch at t = 264 min. The staggered worst
case still measures **43 min** of latency at the 12-minute cadence, confirming 0091's ~43 min
and contradicting the ≤ 25 min claim in the v0.1 spec. `fresh_node_blind_window` is carried
as the documented limitation it is: the node deployed into smoke confirms nothing, only one
node can confirm, so no Alert is possible (a Watch is raised on the other node).

## Constraints

- 915 MHz US only: `lora_mhz` defaults to 915.0 and the firmware logs a FATAL line for
  anything outside 902–928 MHz. No 433 MHz code path exists.
- No backend/VPS, app or website change: the diff touches `.github/workflows/platformio.yml`,
  `docs/wildfire/radio-protocol-v1.md`, `firmware/wildfire-node-v1/**` and
  `python/detection-sim/**` only.
- BME680/BME688: the base does not branch on the sensor model; the node always uses the
  BME680 driver and reports presence in the status byte.
- Worker tier: DeepSeek standard (flash), off-peak (`PEAK: OFF-PEAK 20:15 UTC`).

## Proof

Branch `hermes/0094-base-station-firmware` at
`64bfcfdd1f78bd859521727fff1a1cf1d2a3c8af` (verified with `git ls-remote --heads origin`),
CI run [37060775921](https://github.com/pagosacabin/nordtronics/actions/runs/37060775921)
whose `headSha` equals that SHA, jobs `build` and `host-tests` both `success`. Firmware
artifact `wildfire-node-v1-unified-firmware` (id 11251240032, 675534 bytes, sha256
`9d255a3b10e668675b0df995edb01411076cc371b7c2e38135ca0443320ca460`).

ntfy receipt, published to `nordtronics-build-ed05a663` (the repo's established build topic —
this task spec names no topic of its own, so the reminder in the protocol applies and the
established one was used):

```
id oT2f1V7xtzZx   time 2026-10-02T20:30:51Z

Branch: hermes/0094-base-station-firmware
SHA: 64bfcfdd1f78bd859521727fff1a1cf1d2a3c8af
Workflow: https://github.com/pagosacabin/nordtronics/actions/runs/37060775921
Artifacts: https://github.com/pagosacabin/nordtronics/actions/runs/37060775921/artifacts
Status: success
```

## Reply format (what is outstanding)

Outstanding, and honestly so: the bench serial capture showing the three `ROLE:` lines from
one binary. No board is reachable from this worker, so that leg rests on the unit-tested
strings quoted above. The remaining bench items (TCXO voltage on the V4.2 module, DIO2
RF-switch control, GPIO7 header availability, `batt_mv` with no fuel gauge fitted) are listed
in `firmware/wildfire-node-v1/README.md`. The correlation-window widening is left at the
default 20 min, portal-tunable 10–60, flagged for Stephen rather than guessed at.

## Original spec as received

Kept verbatim so the reply can be read against what was asked.

> Stephen's decision 2026-10-02: ONE firmware for both roles. The board detects whether it is
> a node or a base station at boot (mechanism in Task item 0). Rationale: one codebase, one CI
> target, and it becomes impossible to flash the wrong role image onto a board.
>
> The bench validated the full v1 sensing stack on 2026-10-02: BME680 (I2C 0x77, VIN on Vext,
> SCK→GPIO17 / SDI→GPIO18 — verified, do not flip) and PMS5003 (UART TX→1kΩ→GPIO5, bench 5 V,
> checksummed frames) both live on one Heltec LoRa 32 V4, with sane readings and a real
> solder-fume event detected and correctly classified as fine-particle-dominated. Now the node
> needs something to talk to.
>
> Architecture (decided by Stephen 2026-10-02, fixed): the **base station runs
> consensus/detection**; the backend persists and serves alerts; the app displays and
> acknowledges only, never decides. Phase 1 base hardware is another Heltec LoRa 32 V4,
> USB-powered, no sensors — it bridges LoRa → WiFi/MQTT → VPS and Home Assistant.
>
> Evidence this firmware must encode (from 0091/0092, both verified and archived):
> - 0091 sim: v0.1 rules miss slow-ramp burn piles (baseline chasing), flap with mid-plume
>   auto-clears, and worst-case detection latency is ~43 min, not ≤25. v0.2 fixes: **freeze
>   each node's baseline on first suspected rise**, **post-clear re-arm cooldown**, keep the
>   2-packet / 25 µg/m³ validation rule.
> - 0092 budget: 12-minute cadence closes on energy (219.3 mAh/day); worst-case latency 45 min
>   at 12-min cadence. Two independent analyses agree: publish ~45 min, never ≤25.
> - Radio protocol was the open decision ("everything hangs off it"). This task closes it:
>   **versioned packed-binary packets**, node IDs provisioned at the bench, ACKs for alarm
>   packets only, 12-minute routine check-ins, node marked offline after 3 missed check-ins
>   (36 min). All radio params are captive-portal fields (tank-monitor pattern: firmware
>   default + portal field + NVS).
>
> The single open parameter is the correlation window: 0091 validated 20 min and recommended
> widening, with no number attached. Default it to 20 min and make it portal-tunable
> (10–60 min); flag the widen question for Stephen, do not invent a new default.

### Task (as received)

> One firmware, two roles. Role detection comes FIRST, before any radio, sensor, or power init:
>
> 0. Role detection at boot: read a strap pin (choose it against the 0079 frozen 14 nets —
>    name the pin and justify the choice in the reply); an NVS/portal "role" value overrides
>    the strap when set; unstrapped bench boards default to node. Log the detected role (and
>    its source: strap / NVS / default) to serial on every boot. Branch ALL behavior on the
>    detected role. HARD RULE: the base role must never enter deep-sleep — assert role==node
>    on every sleep path, so a misdetection fails loudly instead of silently deafening the net.
>
> Then, per role:
>
> 1. `docs/wildfire/radio-protocol-v1.md` — versioned packed-binary packet layout (version
>    byte first, then node ID, sequence, PM1/PM2.5/PM10, temp/humidity/pressure, battery mV,
>    flags), join/provisioning flow (bench-provisioned node IDs; base ignores unprovisioned
>    IDs), ACK rules (alarm packets only, retries + timeout values stated), 12-min check-in,
>    3-missed offline rule. Every field justified in one line.
> 2. LoRa RX on 915 MHz US per that doc; validate + checksum every frame.
> 3. v0.2 consensus engine: per-node baseline with freeze-on-first-suspected-rise; alert when
>    ≥2 nodes each show ≥2 packets ≥25 µg/m³ above their frozen baseline inside the
>    correlation window; single-node elevation raises Watch only and NEVER auto-escalates;
>    re-arm cooldown after any clear.
> 4. MQTT uplink: publish per-node telemetry and Watch/alert events; broker
>    host/port/user/pass/topic-root all portal fields with defaults; state and implement the
>    offline policy (buffer with cap + drop-oldest vs drop — pick one, document it, size the
>    buffer).
> 5. Captive portal (tank-monitor pattern) exposing every item above.
> 6. OLED status screen: nodes seen / last-seen, last event, MQTT link state.
>
> Success criteria: `platformio.yml` CI builds the base target green from the branch tip;
> the protocol doc exists, is versioned, and the firmware implements exactly what it says (no
> undocumented fields on the wire); a deterministic host-side test proves the consensus engine
> against the 0091 scenario set (burn-pile slow ramp must NOT be missed via baseline freeze;
> single-node extreme must raise Watch and never alert; mid-plume clear must not re-arm inside
> the cooldown) — pasted serial output is a claim, not proof, so the test must run in CI or as
> a one-command script like 0091's harness; every portal field has a firmware default and
> persists to NVS; both roles demonstrated from the SAME binary (serial log shows the detected
> role and its source for strap / NVS-override / default cases, the consensus test covers base
> behavior, and the strap-pin choice is documented against 0079's frozen nets at branch
> hermes/0079-wildfire-node-rev-c-interface-fixes @ 62e64adf).
>
> Constraints: 915 MHz US variant only, no 433 MHz anywhere; no backend/VPS, app or website
> changes; the BME680/688 distinction must not be branched on; worker cost standard tier,
> off-peak preferred, state the tier used; the strap pin must not collide with any of the 14
> frozen nets from 0079, no PCB change required for bench work because the NVS override covers
> unstrapped boards and Rev C gets a solder jumper; one deliverable — the unified firmware plus
> its protocol doc — on one branch.
