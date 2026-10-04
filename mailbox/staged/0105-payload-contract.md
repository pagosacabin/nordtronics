---
task_id: "0105"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 72h
proof:
  - branch: hermes/0105-payload-contract
    sha: 951cbd38d152c0c3aeeb5ea0ec70c77ed009b280
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37167784900
  - jobs: "build (green, incl. \"Build firmware (wildfire-node-v1, unified node+base)\" = SUCCESS and the wildfire-node-v1-unified-firmware artifact upload); host-tests (green, 26 test cases / 26 succeeded, plus the scenarios_v02.h freshness guard)"
  - artifact:
      url: https://github.com/pagosacabin/nordtronics/actions/runs/37167784900/artifacts
      name: wildfire-node-v1-unified-firmware
      id: 11290351978
      size_bytes: 769746
  - host_tests: "pio test -d firmware/wildfire-node-v1 -e native -> 26 test cases: 26 succeeded (0 failed); the same count is in the CI host-tests log"
  - scratch_probe: "a non-committed host harness compiles the committed src/mqtt_payload.h and prints the exact byte strings; both shapes were then fed through the DEPLOYED backend modules on main (backend/ingest/ingest/validation.py, events.py) -> validate_payload ok=True, validate_event ok=True for all four kinds; transcript in the reply body"
  - ntfy:
      topic: nordtronics-build-ed05a663
      id: 4oohXL5dnzIS
      time: "1791077145 (2026-10-04T01:25:45Z), HTTP 200"
  - files:
      - firmware/wildfire-node-v1/src/mqtt_payload.h
      - firmware/wildfire-node-v1/src/main.cpp
      - firmware/wildfire-node-v1/README.md
notes: |
  Filed by Juno, 2026-10-03. From 0104's staged out-of-scope findings,
  independently verified by Juno against backend/ingest/ingest/validation.py
  and events.py on main: 0104 fixed the telemetry TOPIC, but the PAYLOAD
  does not satisfy the backend's strict validation, so no reading would be
  stored. Same story for events. Until this is fixed, the app cannot show
  live data no matter how healthy the MQTT session is. This is the last
  code item before the final flash.

  PICKED UP (iteration 1 -> 2) by the mailbox worker, 2026-10-04 01:1x UTC.
  Off-peak (PEAK: OFF-PEAK 01:15 UTC).

  STAGED 2026-10-04 01:2x UTC, same run. Deliverable: the firmware's
  telemetry payload and event leg now match the deployed contract, via one
  new portable header (src/mqtt_payload.h) that both main.cpp and a host
  harness compile. No consensus threshold, LoRa, or 0104 item was touched;
  the diff is one new file plus one function in main.cpp. Verified by the
  backend's own validators (see scratch_probe) and by CI at the branch tip
  (headSha 951cbd38 == branch tip; both jobs green). Cost: deepseek-flash,
  standard tier, off-peak.
---

# 0105 — wildfire-node-v1: payload + events match the backend contract

## Context

The backend validates strictly: a reading that fails is not stored at all
(`backend/ingest/ingest/validation.py`: "a reading that cannot be trusted
is not stored at all"). The firmware's telemetry payload does not satisfy
it, and its event topics/payloads do not satisfy `events.py`. Read both
files on main before writing code — they are the contract.

## Task

On branch `hermes/0105-payload-contract` (from the 0104 tip `11de2cc`),
in `firmware/wildfire-node-v1`:

1. **Telemetry payload** — emit exactly what `validation.py` requires:
   - `pm25` (µg/m³, already present)
   - `temperature_c` (not `temp_c`)
   - `humidity_pct` (not `rh`)
   - `battery_v` in VOLTS (not `batt_mv` — millivolts would fail the
     0–30 V range check; divide by 1000)
   - `node_id` as a STRING (not `node`), matching the node ID in the
     publish topic (the worker checks this; mismatch is rejected)
   - `observed_utc` ISO-8601 Z (the NTP clock is live since 0099, so the
     board can stamp it)
   Extra keys are ignored by validation (never fatal), so keep or drop
   the rest on your judgment — but the required keys must be exact.
2. **Events** — read `backend/ingest/ingest/events.py` first:
   - Publish to `<root>/<base-id>/events` (the backend subscribes
     `nordtronics/wildfire/+/events`; the firmware's `<root>/event/alert`
     shape is dropped).
   - Use the backend's event kinds: `watch_raised`, `watch_cleared`,
     `alert_raised`, `alert_cleared` (not `{"event":"alert",…}`).
3. **Offline notice** — `<root>/node/<id>/state` is ACL-denied (the ACL
   grants write only on `+/telemetry` and `+/events`). Either move it
   onto a permitted topic or remove it; your call, declared in the reply.

Push the branch and take it green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and the
  `host-tests` job passes in the same run. If a host test asserts the old
  payload shape, you MAY update it (behavior change) — declare it.
- Falsifiable: a scratch host harness (same tactic as 0104's probe)
  prints a sample telemetry JSON containing exactly `pm25`,
  `temperature_c`, `humidity_pct`, `battery_v` (volts, e.g. 4.05 not
  4050), `node_id` as a string, and `observed_utc` matching
  `YYYY-MM-DDTHH:MM:SSZ` — quote the output.
- Falsifiable: the event topic builder emits
  `<root>/<base-id>/events` and the kinds are the four `*_raised` /
  `*_cleared` strings — quote them.
- `git diff` contains no credential, SSID, or password.

## Constraints

- The backend is the authority. Do not "fix" validation.py or events.py
  to match the firmware — the backend is deployed and the app renders
  from it.
- Do not touch the LoRa receive path, the consensus thresholds, or the
  0104 items. One deliverable: the contract. Nothing else.

## Proof

- Branch `hermes/0105-payload-contract` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- The two quoted outputs above.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.

---

# Reply — STAGED (iteration 2): payload + events match the deployed contract

One deliverable, one new file: `firmware/wildfire-node-v1/src/mqtt_payload.h`
holds both payload builders and the events-topic builder; `main.cpp` calls them
and no longer hand-writes the JSON. The contract was read from `main` before any
code was written (`backend/ingest/ingest/validation.py`, `events.py`).

Branch `hermes/0105-payload-contract` from the 0104 tip `11de2cc`, tip
`951cbd38d152c0c3aeeb5ea0ec70c77ed009b280`.

## Falsifiable check 1 — the telemetry JSON the firmware emits

A non-committed host harness (`harness.cpp`, in the worker's scratch dir, not in
the repo) `#include`s the committed header and prints the exact bytes
(`g++ -std=gnu++17 -Wall -Wextra -I src harness.cpp consensus_v02.cpp`):

```
telemetry_topic   = nordtronics/wildfire/3/telemetry
telemetry_payload = {"pm25":12.3,"temperature_c":22.34,"humidity_pct":41.00,"battery_v":0.000,"node_id":"3","observed_utc":"2026-10-04T01:23:45Z"}
```

Six keys, exactly the required set (no extras), `battery_v` in volts,
`node_id` a string, `observed_utc` `YYYY-MM-DDTHH:MM:SSZ`.

## Falsifiable check 2 — the event topic and kinds

Same harness:

```
events_topic      = nordtronics/wildfire/2/events
event_payload[0]  = {"event":"watch_raised","observed_utc":"2026-10-04T01:23:45Z","node_id":"3","nodes":["3"]}
event_payload[1]  = {"event":"watch_cleared","observed_utc":"2026-10-04T01:23:45Z","node_id":"3","nodes":["3"]}
event_payload[2]  = {"event":"alert_raised","observed_utc":"2026-10-04T01:23:45Z","node_id":"network","nodes":["3","4"]}
event_payload[3]  = {"event":"alert_cleared","observed_utc":"2026-10-04T01:23:45Z","node_id":"network","nodes":["3","4"]}
event_kind[4]     = (none -- internal, not published)
```

The four kinds are exactly `events.py`'s `EVENT_KINDS`. `event_kind[4]` is the
engine's `NodeRiseConfirmed`: an internal per-node confirmation with no backend
counterpart, so it has no wire name and is not published (declared below).

## The check that actually matters — fed through the deployed backend

The harness output above was piped straight into the backend's own validators on
`main` (not a re-implementation), which is the authority the task names:

```
  validation.TOPIC_TEMPLATE  = nordtronics/wildfire/+/telemetry
  events.EVENT_TOPIC_TEMPLATE = nordtronics/wildfire/+/events
  events.EVENT_KINDS = ['alert_cleared', 'alert_raised', 'watch_cleared', 'watch_raised']

telemetry topic nordtronics/wildfire/3/telemetry  matches nordtronics/wildfire/+/telemetry -> True
events topic    nordtronics/wildfire/2/events     matches nordtronics/wildfire/+/events    -> True

[telemetry] ok=True reading={'pm25': 12.3, 'temperature_c': 22.34, 'humidity_pct': 41.0, 'battery_v': 0.0, 'observed_utc': '2026-10-04T01:23:45Z'} ignored=[] errors=
[event] ok=True event=watch_raised  type=watch  severity=watch    state=active  title='PM2.5 elevated on 3, above its frozen baseline.'
[event] ok=True event=watch_cleared type=watch  severity=watch    state=cleared title='Watch on 3 cleared.'
[event] ok=True event=alert_raised  type=smoke  severity=warning  state=active  title='PM2.5 elevated confirmed on 2 nodes.'
[event] ok=True event=alert_cleared type=smoke  severity=warning  state=cleared title='Alert cleared for the network.'
ALL PAYLOADS ACCEPTED BY THE DEPLOYED BACKEND: True
```

`ignored=[]` on the telemetry reading: no field is discarded, i.e. the emitted
set is exactly the required set. Before this change the same validator rejected
the payload on `pm25`'s neighbours (`temperature_c`/`humidity_pct`/`battery_v`
absent, `battery_v` in millivolts) and on `node_id`.

## CI — green at the branch tip

Run https://github.com/pagosacabin/nordtronics/actions/runs/37167784900,
`headSha = 951cbd38d152c0c3aeeb5ea0ec70c77ed009b280` = the branch tip
(`git ls-remote --heads origin hermes/0105-payload-contract`), conclusion
`success`, both jobs green:

- `build`: `Build firmware (wildfire-node-v1, unified node+base)` = SUCCESS, and
  artifact `wildfire-node-v1-unified-firmware` (id 11290351978, 769746 bytes)
  uploaded.
- `host-tests`: `26 test cases: 26 succeeded` — quoted from the CI log, not from
  the green tick (PlatformIO can exit 0 having run zero cases; the count is the
  evidence). The `scenarios_v02.h` freshness guard passed too.

ntfy receipt `nordtronics-build-ed05a663`, id `4oohXL5dnzIS`,
2026-10-04T01:25:45Z: Branch / SHA / Workflow / Artifacts / Status, HTTP 200.

## Scope, deviations and choices declared

- **`battery_v` is 0.0.** Rev C has no fuel gauge; the frame carries `batt_mv=0`
  (unchanged from 0104, TODO'd in the README). 0.0 is inside `validation.py`'s
  `[0, 30]` volt range so the reading is stored; it will simply read 0 V until
  the sense divider exists. Not a defect introduced here.
- **Extras dropped.** The old payload also carried `seq`, `pm1`, `pm10`,
  `press_pa`, `status`, `level`. `validation.py` ignores unknown keys, but the
  criterion says the sample must contain *exactly* the required set, so the
  builder emits exactly those six. Nothing on the backend consumed the extras,
  and the OLED still shows them locally.
- **`NodeRiseConfirmed` is not published.** It is the engine's per-node
  confirmation signal, not one of `events.py`'s four kinds; publishing it would
  have required inventing a mapping (e.g. calling it a `watch_raised`), which
  would double-report the Watch the engine raises in the same packet. The Watch
  is published; the per-node confirmation is logged only.
- **`alert_cleared` vs `watch_cleared`.** The engine's `Cleared` event does not
  record which level was cleared, and `feed()` has already reset the level by
  the time the event is returned, so `main.cpp` captures `g_engine.level()`
  *before* `feed()` and passes it to `wf::wire_event_kind(kind, was_alert)`.
  The engine itself is untouched (constraint: do not touch the consensus
  thresholds).
- **The `<root>/node/<id>/state` offline notice is removed, not re-routed**
  (item 3). The ACL grants write only on `+/telemetry` and `+/events`, so the
  notice was broker-denied and reached nothing; and it cannot go onto `/events`
  because `events.py` has no offline kind (it would be rejected) nor onto
  `/telemetry` because that requires a full valid reading. The backend already
  derives staleness itself (`/v1/nodes` → `last_seen_utc` / `status: "stale"`),
  and the base still shows node-offline on its OLED. Chose removal over
  relocation; the reasoning is in the code comment at the call site.
- **Event `pm25`/`baseline`/`window_min` are omitted.** `events.py` treats them
  as optional and the engine's `Event` does not carry them structurally (they
  exist only inside the free-text `detail` string), so the alert card falls back
  to its "PM2.5 elevated" title rather than a measured value. The events are
  valid and stored; surfacing the numbers would need a structured field on
  `Event`, which is a consensus-engine change this task's constraint excludes.
  Flagged as a possible follow-up, not claimed as done.
- **No host test asserts the payload shape**, and none was added: the criterion
  asks for a scratch harness, and `test/` has no test that compiles `main.cpp`.
  Nothing in `test/` needed updating (no stale-payload assertion exists).
- **`platformio.yml` needed no branch-list edit** — its `on.push` has no branch
  filter, only a path filter, which `firmware/wildfire-node-v1/**` satisfies, so
  the push started the run directly.
- **Not touched:** consensus thresholds, the LoRa receive path, the 0104 items,
  and the untracked `firmware/wildfire-node-v1/bench-override.ini` (left
  uncommitted). `git diff` contains no credential, SSID or password.

Cost: `deepseek-flash`, standard tier, off-peak (`PEAK: OFF-PEAK 01:15 UTC`).

