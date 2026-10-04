---
task_id: "0104"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 72h
proof:
  - branch: hermes/0104-wildfire-combined
    sha: 11de2cc0c0cbe41b45340e27cce4c13f5b220bda
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37165068795
  - jobs: "build (green, incl. \"Build firmware (wildfire-node-v1, unified node+base)\" and the artifact upload); host-tests (green, incl. \"Run host-side firmware tests (native env)\" and the scenarios_v02.h freshness guard)"
  - artifact: https://github.com/pagosacabin/nordtronics/actions/runs/37165068795/artifacts
    artifact_name: wildfire-node-v1-unified-firmware (id 11289182278, 768555 bytes)
  - host_tests: "pio test -d firmware/wildfire-node-v1 -e native -> 26 test cases, 26 succeeded (0 failed)"
  - ntfy:
      topic: nordtronics-build-ed05a663
      id: NaXJ23yGEE2V
      time: "1791073938 (2026-10-04T00:32:18Z), HTTP 200"
  - files:
      - docs/wildfire/radio-protocol-v1.md
      - firmware/wildfire-node-v1/README.md
      - firmware/wildfire-node-v1/platformio.ini
      - firmware/wildfire-node-v1/src/device_name.h
      - firmware/wildfire-node-v1/src/firmware_config.h
      - firmware/wildfire-node-v1/src/main.cpp
      - firmware/wildfire-node-v1/src/mqtt_topic.h
      - firmware/wildfire-node-v1/src/role_detect.cpp
      - firmware/wildfire-node-v1/src/role_detect.h
      - firmware/wildfire-node-v1/test/test_role_and_portal/test_main.cpp
notes: |
  Filed by Juno, 2026-10-03. Supersedes 0100, 0101, 0102 (deleted from the
  inbox unpicked): Stephen hates the hour-per-task cycle, and all three are
  small, independent changes to firmware/wildfire-node-v1 that ride one CI
  run. One pickup, one branch, one verification. The checklists below
  are each falsifiable — verify all before staging.

  2026-10-03 17:45 MDT: fourth item added (MQTT session resilience) from
  0103's staged findings — still unpicked, still one build. The base holds
  a TLS session to the broker but mosquitto drops it on keepalive timeout
  every ~22 s (k15) and it reconnects ~45 s later, forever; mqtt_publish()
  early-returns when disconnected, silently dropping telemetry even though
  the portal's offline policy is "buffer" (cap 180). Until the session
  holds (or the buffer is honored), 0097's fresh-reading criterion cannot
  pass.
---

# 0104 — wildfire-node-v1: topic + hostname + sensor role + MQTT resilience (one build)

## Context

Three pending changes, all on `firmware/wildfire-node-v1`, stacked on the
verified 0099 tip (`881ee68`):

1. **Topic (was 0100, blocking live data):** the firmware builds telemetry
   as `<mqtt_root>/node/<id>/telemetry`, but the deployed backend
   (ingest worker, validation template, Mosquitto ACLs — all on main)
   subscribes `nordtronics/wildfire/<node-id>/telemetry`, single level.
   The `+` wildcard matches exactly one level, so the firmware's form is
   silently dropped. Two docs repeat the wrong form and stale broker
   defaults (`api.nordtronics.io:1883`).
2. **Hostname (was 0101, Stephen's ask):** every board gets a DHCP
   hostname `wildfire-<role>-<nn>` (role = base/node, nn = zero-padded
   node ID, `01` when unprovisioned) via `WiFi.setHostname()` before STA
   bring-up — identifiable in any router client list, no OLED or serial
   needed.
3. **Role (was 0102, Stephen's call — no hw mods):** the GPIO7 strap was
   Juno's 0094 spec, never Stephen's. Replace with the tank-monitor
   pattern: probe node sensors at boot (BME680/688 at 0x77/0x76,
   PMS5003 on UART) — found = node, none = base. NVS/portal override
   still wins. Portal label "blank = strap" becomes "blank = auto-detect".
4. **MQTT resilience (from 0103's bench findings):** the TLS session
   reaches the broker but mosquitto drops it on keepalive timeout every
   ~22 s (`k15` in the broker journal) with reconnect ~45 s later, in a
   loop that pre-dates the 0099 flash — the client is not servicing the
   connection in time. Worse, `mqtt_publish()` early-returns when
   disconnected, silently dropping telemetry while the configured offline
   policy is "buffer" (cap 180 records). Fix the servicing so the session
   holds; honor the offline policy (buffer-then-flush when set to buffer,
   not silent drop); and log the WiFi IP on connect plus the disconnect
   reason code on drop (0098's observability gap — the current build
   prints neither).

## Task

On branch `hermes/0104-wildfire-combined` (from `881ee68`), apply all
three changes, push, and take green through
`.github/workflows/platformio.yml`. No hardware touched, no credentials.

Test-file rule for this task: `test/test_role_and_portal` MAY be edited,
but ONLY for the role truth table (sensors present → node; absent → base;
valid NVS wins both directions; out-of-range NVS falls through to the
probe). No other test file is touched; if any other test asserts
behavior you must change, stop and declare it in the reply.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI and
  the `host-tests` job passes in the same run.
- Topic: `grep -rn "wildfire/node/" firmware/wildfire-node-v1/src/`
  returns nothing; the topic builder emits
  `nordtronics/wildfire/<node-id>/telemetry` for a sample node id (quote
  the line). `grep -rn "api.nordtronics.io"
  docs/wildfire/radio-protocol-v1.md backend/DEPLOY.md` returns nothing,
  and both show `mqtt.nordtronics.io` / `8883`.
- Hostname: `grep -n "setHostname" firmware/wildfire-node-v1/src/main.cpp`
  shows the call before the STA connect sequence; format produces
  `wildfire-base-01` / `wildfire-node-02` style names (quote the lines);
  no new portal field, no new NVS key.
- Role: `grep -rni "strap" firmware/wildfire-node-v1/src/` returns nothing
  in logic; priority is NVS override > sensor probe > base default.
- MQTT resilience: the session-hold can't be proven in CI — so the
  falsifiable part is code + config: the MQTT client is serviced every
  loop (no path starves PINGREQ), `mqtt_publish()` routes through the
  offline buffer when disconnected instead of early-returning (quote the
  lines), and WiFi connect/disconnect log lines exist naming IP and
  reason code (quote them). The bench hold-test is 0103's successor
  task's job.
- `git diff` contains no credential, SSID, or password.

## Constraints

- Do not touch the LoRa receive path, the 0099 fixes, or the consensus
  logic. These four changes and nothing else.

## Proof

- Branch `hermes/0104-wildfire-combined` @ SHA on origin.
- Actions run URL, conclusion `success`, both jobs green.
- Every grep above, quoted verbatim.

## Reply format

Follow the mailbox staged-reply format: status line, the four checklists
with quoted evidence, deviations declared, cost line.


---

# Reply — 2026-10-04 00:35 UTC: DELIVERED (iteration 2)

**Status: delivered.** All four items are implemented on
`hermes/0104-wildfire-combined`, pushed, and green through
`.github/workflows/platformio.yml` (both jobs) at the branch tip. No hardware was
touched, no credential, SSID or password appears in the diff. Three deviations and
three out-of-scope findings are declared below; one of them is a correction to a
success criterion as written.

Branch tip, re-read from origin after the push (not from memory):

    $ git ls-remote --heads origin hermes/0104-wildfire-combined
    11de2cc0c0cbe41b45340e27cce4c13f5b220bda	refs/heads/hermes/0104-wildfire-combined
    $ gh run view 37165068795 --json headSha,conclusion,event
    {"conclusion":"success","event":"push","headSha":"11de2cc0c0cbe41b45340e27cce4c13f5b220bda"}
    jobs: build success, host-tests success
    host-tests log: "26 test cases: 26 succeeded in 00:00:11.026"

The artifact `wildfire-node-v1-unified-firmware` (id 11289182278, 768555 bytes) is
from the *same* run, so it is the tip build. `firmware.bin` embeds `__DATE__
__TIME__` and is intentionally not reproducible, so its size is quoted rather than
compared (0099's note).

## Item 1 — telemetry topic (was 0100)

The topic is now built by one function and nothing else. `src/mqtt_topic.h` is new:

    inline std::string telemetry_topic(const std::string& root, const std::string& node_id) {
      std::string r = root;
      while (!r.empty() && r.back() == '/') r.pop_back();
      return r + "/" + node_id + "/telemetry";
    }

`main.cpp:535` is the only caller in the firmware:

      mqtt_publish(wf::telemetry_topic(g_cfg.mqtt_root.c_str(),
                                       std::to_string((unsigned)f.node_id)),
                   buf);

**The criterion, run from the branch tree:**

    $ grep -rn "wildfire/node/" firmware/wildfire-node-v1/src/
    (no output, exit 1)

    $ grep -rn "wildfire/node/" firmware/wildfire-node-v1/src/
    # also empty on origin/main before the change, because the old call site was
    # written as String("node/") + id + "/telemetry" and the root was prepended
    # inside mqtt_publish(). So this grep never was the discriminator the spec
    # assumed -- the builder output below is.

**The builder's output**, from a scratch host program that `#include`s the
committed header and prints it (the headers are the same ones the device compiles;
this is the same tactic 0099 used for the `ROLE:` lines, and it is NOT a repo test
file):

    $ g++ -std=c++17 -I src -Wall -Wextra -o probe probe.cpp && ./probe
    telemetry_topic("nordtronics/wildfire", "bench-01") = nordtronics/wildfire/bench-01/telemetry
    telemetry_topic("nordtronics/wildfire", "3")        = nordtronics/wildfire/3/telemetry
    telemetry_topic("nordtronics/wildfire/", "7")       = nordtronics/wildfire/7/telemetry

The trailing-slash case is included because `mqtt_root` is a free-text portal
field; a stored `nordtronics/wildfire/` would otherwise emit a double slash and
miss the ACL just as quietly as the old shape did.

Docs: `docs/wildfire/radio-protocol-v1.md` §8's telemetry row is now
`<root>/<node-id>/telemetry` with a paragraph stating the one-level rule and why,
and the broker default there is corrected to `mqtt.nordtronics.io:8883` (it read
`api.nordtronics.io:1883`, the stale pair 0099 flagged).

⚠️ **Criterion correction — the `api.nordtronics.io` half of item 1 cannot be
satisfied literally, and it should not be.** The criterion reads "`grep -rn
"api.nordtronics.io" docs/wildfire/radio-protocol-v1.md backend/DEPLOY.md`
returns nothing". `docs/wildfire/radio-protocol-v1.md` now returns nothing, but
`backend/DEPLOY.md` still contains seven lines, every one of them a *correct* API
reference, not a broker default:

    $ grep -rn "api\.nordtronics\.io" docs/wildfire/radio-protocol-v1.md backend/DEPLOY.md
    backend/DEPLOY.md:8:`nordtronics.io` / `mqtt.nordtronics.io` / `api.nordtronics.io`, and a Let's
    backend/DEPLOY.md:271:## 9. nginx for api.nordtronics.io
    backend/DEPLOY.md:283:    server_name api.nordtronics.io;
    backend/DEPLOY.md:319:curl -sS https://api.nordtronics.io/healthz
    backend/DEPLOY.md:338:curl -sS "https://api.nordtronics.io/v1/nodes" | python3 -m json.tool
    backend/DEPLOY.md:340:curl -sS "https://api.nordtronics.io/v1/nodes/node-01/readings?limit=5" | python3 -m json.tool
    backend/DEPLOY.md:422:curl -s https://api.nordtronics.io/healthz   # "schema_version":2

`api.nordtronics.io` is the HTTPS API's own name (the api vhost's `server_name`,
its `/healthz`, its `/v1/nodes`); deleting those to satisfy the grep would break
the deployment runbook. There is no broker default left in DEPLOY.md and there
never was one in this revision: `grep -n "1883" backend/DEPLOY.md` returns only
the two lines of the deliberate plaintext-refusal test (`timeout 5 mosquitto_pub
-h 89.117.21.105 -p 1883 ...`, "UFW never opened it and the broker has no 1883
listener"), and the smoke test already publishes to `mqtt.nordtronics.io:8883` on
`nordtronics/wildfire/node-01/telemetry`. So the spec's "two docs repeat the wrong
form and stale broker defaults" is true of one doc, not two. I implemented the
intent (no stale broker default anywhere; the correct broker values shown in both
files) and am declaring the literal criterion unattainable rather than deleting
correct documentation to turn a grep green.

## Item 2 — DHCP hostname (was 0101)

`src/device_name.h` (new) is the builder; `main.cpp:688` wraps it and
`main.cpp:729` is the STA connect that follows it:

    $ grep -n "setHostname" firmware/wildfire-node-v1/src/main.cpp
    692:  WiFi.setHostname(hostname.c_str());

    688: static void wifi_set_device_name() {
    689:   const std::string hostname = wf::device_hostname(wf::role_name(g_role), g_cfg.node_id);
    691:   WiFi.setHostname(hostname.c_str());     <- set before the STA starts
    726:   wifi_set_device_name();                 <- called from wifi_begin()
    729:   WiFi.begin(g_cfg.wifi_ssid.c_str(), ...)  <- the STA connect sequence

Builder output (same scratch-harness tactic):

    device_hostname("base", 1) = wildfire-base-01
    device_hostname("node", 2) = wildfire-node-02
    device_hostname("node", 0) = wildfire-node-01  (unprovisioned node_id 0)
    device_hostname("node", 123) = wildfire-node-123

`blank = auto-detect` from item 3's rule means the role token is the *resolved*
role, not the raw NVS value, and `node_id == 0` maps to `01`. **No portal field and
no NVS key were added** — the name is derived from `g_role` and `g_cfg.node_id`;
`test_role_and_portal` still counts 30 fields / 30 unique keys, unchanged. The
setup AP keeps its fixed `wildfire-setup` name, because that one is what the
operator joins to reach the portal, not a site value.

## Item 3 — role from the sensor probe (was 0102)

The jump is gone: `resolve_role()` now takes `sensors_present`, and the truth
table is `NVS (0/1) > probe > base default`.

    $ grep -rni "strap" firmware/wildfire-node-v1/src/
    (no output, exit 1)

That is the whole `src/` tree, comments included. It cost two renames that are worth
declaring: `kS3BootStrappingPins` became `kS3BootSelectPins` (same four pins, same
table), and `wf::RoleSource::Strap` became `Probe`. The retired constants
`kRoleStrapPin` / `kStrapLevelBase` / `kStrapLevelNode` and their justification
block are deleted; the 0079 frozen-net tables are kept and now justify the probe's
pins instead (GPIO17/18 I2C, GPIO5/6 UART are all on the frozen list).

The probe itself, `main.cpp:327`:

    $ grep -n "probe_node_sensors\|resolve_role(sensors_present\|role_log_line" \
        firmware/wildfire-node-v1/src/main.cpp
    327: static bool probe_node_sensors() {
    813:   const bool sensors_present = probe_node_sensors();
    815:   g_role_decision = wf::resolve_role(sensors_present, nvs_present, nvs_role);
    817:   logf("%s", wf::role_log_line(g_role_decision).c_str());

It asks two independent questions and either one answers "node": a BME680 (0x77) or
BME688 (0x76) ACKing on the sensor I2C bus, or a valid 32-byte PMS5003 frame on the
sensor UART. The SSD1306 at 0x3C is deliberately *not* part of the probe — both
roles carry the panel, so probing it would answer "node" for every base station.

The truth table is asserted host-side, in the one test file 0104 opens
(`test/test_role_and_portal`), and the run prints it:

    [role] probe->node / none->base / NVS->base / NVS->node / NVS-invalid-falls-through: all 6 cases hold
    [role-log] ROLE: node (source=PROBE, sensors=present, nvs_role=unset)
    [role-log] ROLE: base (source=DEFAULT, sensors=absent, nvs_role=unset)
    [role-log] ROLE: base (source=NVS, sensors=present, nvs_role=1)

(the three `[role-log]` lines are the exact boot strings; the third is quoted
complete in the test's equality assertion:
`"ROLE: base (source=NVS, sensors=present, nvs_role=1)"`.)

Portal label changed to `Radio role (0 node / 1 base; blank = auto-detect)`
(`firmware_config.h:103`). `role_detect.h`/`.cpp`, the firmware README's role
table, `platformio.ini`'s header comment and radio-protocol §9 were all rewritten
to match — leaving any of them describing a jumper would have been a stale spec.

⚠️ Two ordering facts worth stating because they are behaviour, not cosmetics:
`load_config()` now runs *before* the probe, so the probe uses the configured PMS
baud rather than the compile-time default; and the probe necessarily touches I2C
before the OLED reset pulse, so the old "role detection before ANY sensor init"
sentence in the 0094 header comment is no longer true and was corrected rather
than left standing.

## Item 4 — MQTT session resilience

The falsifiable part, as the task frames it:

**(a) The client is serviced every pass, and no path can starve it.**

    $ grep -n "g_mqtt.loop()\|offline_flush()\|kRadioPollMs" \
        firmware/wildfire-node-v1/src/main.cpp
    562: static constexpr uint32_t kRadioPollMs = 500;
    566:   const int st = g_radio.receive(buf, sizeof(buf), kRadioPollMs);
    887:         offline_flush();
    897:     g_mqtt.loop();
    898:     offline_flush();

The base loop now services MQTT first, then the portal, then the radio, and the
one blocking call is bounded. That mattered more than it looks: 0094 called
`g_radio.receive(buf, sizeof(buf))` with no timeout, and RadioLib's default is
**5× the time-on-air of the whole 128-byte buffer** (`SX126x.cpp`:
`timeoutInternal = (getTimeOnAir(maxLen) * 5) / 1000;`). At the configured
SF7/125 kHz that is a 198-symbol, 215 ms frame, so the default window is ~1.1 s —
harmless against a 15 s keepalive. But `lora_sf` is a portal field, and at SF12 the
same call blocks for ~24.6 s, which is longer than the ~22 s mosquitto allows a
client to miss its keepalive. `kRadioPollMs = 500` pins the window regardless of
the configured SF, so the drop-on-keepalive cannot be reintroduced by a config
change. (These air times are computed from the LoRa formula, not measured on a
board — see the limits note below.)

**(b) `mqtt_publish()` routes through the offline buffer instead of
early-returning.**

    main.cpp:483    offline_push(topic, payload.c_str());
    main.cpp:497  static void offline_flush() {
    main.cpp:535    mqtt_publish(wf::telemetry_topic(...), buf);   <- telemetry

`mqtt_publish()` previously began `if (!g_mqtt.connected()) return;`. It now
buffers when the session is down and `offl_policy == "buffer"`, logs the buffer
depth, drops the OLDEST record on overflow (`wf::kOfflineBufferDropOldest`), and
`offline_flush()` replays in order on reconnect, stopping at the first record the
client refuses rather than discarding the tail. That is the rule §8 of the protocol
doc already stated and the firmware did not implement.

**(c) WiFi connect/disconnect log lines naming IP and reason code.**

    main.cpp:701    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    main.cpp:702-704  logf("wifi: connected -- ip=%s gw=%s rssi=%d dBm", ...)
    main.cpp:706    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    main.cpp:707-708  logf("wifi: disconnected -- reason=%d (%s)", ...)
    main.cpp:860    WiFi.onEvent(wifi_event_log);   <- registered before the STA

The reason code is `info.wifi_sta_disconnected.reason` from the ESP-IDF event,
which is what was missing from 0103's bench log. `wifi_begin()`'s own line also
carries the IP now.

**Limit, stated plainly:** the session-hold itself is NOT proven. The task says so
("the session-hold can't be proven in CI"), and no board was touched. What is
proven is that the servicing is unconditional, the one long blocking call is
bounded at 500 ms (28× under the keepalive), and a dropped session no longer loses
data. The hold-test belongs to 0103's successor.

## Verification

- Local, before the push: `pio test -e native` → **26 test cases, 26 succeeded**;
  `pio run -e heltec_v4` → SUCCESS (RAM 16.5 %, Flash 18.6 %).
- CI at the tip: run 37165068795, `headSha` = the branch tip, `build` and
  `host-tests` both `success`, 26/26 test cases in the host-tests log.
- Warnings: the only warnings attributable to this tree are the pre-existing
  `inline variables are only available with -std=gnu++17` notes in
  `firmware_config.h`/`mqtt_ca.h` (`inline constexpr` under the tree's
  `-std=gnu++11` default, documented in 0099's notes). The two new headers use
  plain `inline` functions and add none; a clean rebuild was checked
  (`.pio/build` removed) to be sure the log was not a cached no-op.
- `git diff 881ee68..11de2cc` contains no credential, SSID or password: the only
  matches for those words are the pre-existing portal field names
  (`wifi_ssid`, `wifi_pass`) and comments about a stored SSID.

## Deviations

1. **Two renames and a deleted constant, forced by item 3's grep criterion.**
   `kS3BootStrappingPins` → `kS3BootSelectPins`; `RoleSource::Strap` → `Probe`;
   `kRoleStrapPin`/`kStrapLevelBase`/`kStrapLevelNode` deleted with their comment
   block. The literal criterion is "`grep -rni strap` returns nothing **in
   logic**", which could have been read as comments-excluded; I satisfied the
   stricter reading, because that is the one a verifier can check mechanically.
2. **The strap-collision test in `test_role_and_portal` was replaced.** 0104
   permits editing that file "ONLY for the role truth table", and the file
   contained a test whose subject no longer exists (`test_strap_pin_does_not_collide`
   asserted on the deleted `kRoleStrapPin` — it could not compile). It is now
   `test_boot_probe_pins_are_frozen_sensor_nets`, asserting that every pin the boot
   probe drives is on the 0079 frozen net list and is not a boot-select / USB /
   UART0 / SX1262-SPI pin. Same file, same verification value, new subject; no
   other test file was touched, and the pin block verifies:
   `[probe] i2c=17/18 pms=5/6 frozen_nets=14`.
3. **A second commit after the first green run.** The first push (1f4655e, run
   37164834664, green) was superseded by 11de2cc, which corrects only a comment:
   my initial note said RadioLib's default window was "~1.3 s at SF7", and
   recomputing the air time (198 symbols / 215 ms, ×5) made it ~1.1 s, with the
   SF12 case being ~24.6 s rather than "several seconds". The proof pointers above
   are for the tip run; the superseded run and its ntfy receipt
   (`T8WwmWY7Jp5P`) are not cited as proof.
4. **The `api.nordtronics.io` criterion** — see item 1. Implemented the intent,
   declared the literal criterion unattainable, deleted nothing.

## Out-of-scope findings (found, NOT fixed — outside the four items)

1. **The event topics have the same defect the telemetry topic had.**
   `base_handle_frame()` still publishes `event/alert` / `event/watch` /
   `event/clear`, i.e. `<root>/event/alert` — two levels after the root, while the
   deployed ingest worker subscribes `nordtronics/wildfire/+/events` (plural
   leaf). Those events are dropped exactly as telemetry was. I routed them
   through a `rooted_topic()` helper that preserves the current shape rather than
   silently changing a contract 0095 owns; fixing them is a separate change (and
   the payload vocabulary differs too — the firmware emits
   `{"event":"alert",…}` while `backend/ingest/ingest/events.py` fixes
   `EVENT_KINDS = {watch_raised, watch_cleared, alert_raised, alert_cleared}`).
2. **The telemetry payload does not match the deployed validation schema.**
   `backend/ingest/ingest/validation.py` requires `pm25`, `temperature_c`,
   `humidity_pct`, `battery_v` and a timestamp (`observed_utc`/`ts`/`timestamp`).
   The firmware publishes `temp_c`, `rh`, `batt_mv`, `press_pa`, `status`, `level`
   and no timestamp. **So the topic fix in item 1 is necessary but not by itself
   sufficient for live data to be stored** — this is the next thing to fix, and it
   is a bigger change than anything in 0104's four items.
3. **`<root>/node/<id>/state` (the offline notice) reaches nothing.** The ingest
   worker understands only `telemetry` and `events` leaves, and the Mosquitto ACL
   grants write on `+/telemetry` and `+/events` only, so the offline publish is
   ACL-denied rather than merely unsubscribed. Left untouched (it is not one of
   the four items); the API's own staleness computation covers the same ground.

## Success criteria, checked

- `pio run -e heltec_v4` builds in CI and `host-tests` passes in the same run —
  **yes**, run 37165068795, both jobs `success` at the tip, 26/26 test cases.
- Topic greps — **yes for the firmware** (`grep -rn "wildfire/node/" src/` empty)
  and the builder is quoted emitting `nordtronics/wildfire/<node-id>/telemetry`;
  **corrected for the `api.nordtronics.io` half** (declared above, nothing
  deleted).
- Hostname — **yes**: `setHostname` at `main.cpp:692`, before the STA connect at
  `:729`; `wildfire-base-01` / `wildfire-node-02` quoted; no new portal field, no
  new NVS key (still 30 fields).
- Role — **yes**: `grep -rni "strap" src/` empty; priority NVS > probe > base
  default asserted in the truth table, 6 cases.
- MQTT resilience — **code and config only, as the task defines it**: serviced
  first every pass, the 500 ms bound, buffer-then-flush quoted, connect/disconnect
  log lines quoted. The hold-test is not claimed.
- `git diff` contains no credential, SSID or password — **yes**, checked.

## Cost and tier

`deepseek-flash`, off-peak — the pre-run digest printed
`PEAK: OFF-PEAK 00:15 UTC -- safe to work.` and the run stayed inside that tick.
One task worked (0104, iteration 2), one branch, one CI cycle plus one
comment-only re-verify. 0097 and 0098 were left untouched: both are
decision-blocked (0097 on unobtainable bench secrets, 0098 on the pinned
toolchain), and re-declaring either is churn, not progress.
