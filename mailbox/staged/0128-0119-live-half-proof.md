---
task_id: "0128"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0119-battery-adc
    sha: d6fce0ec1710c8e5847a57df1cb40b5f8fcaf59c   # == git ls-remote --heads origin; the revision the LIVE NODE is running
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37533195529
  - jobs: "build success, host-tests success; conclusion success; headSha
      d6fce0ec1710c8e5847a57df1cb40b5f8fcaf59c == the branch tip; createdAt
      2026-10-06T21:20:36Z, updatedAt 2026-10-06T21:22:44Z"
  - artifact: wildfire-node-v1-unified-firmware (run 37533195529), firmware.bin
      1226608 B, sha256 c5004dd85f5a040efebd8e7c21f556ba1f5ff22f9db6db6c9f7cea4844548df4
      (unchanged from the sha256 0119 recorded) -- downloaded this run and its
      baked build stamp matched the live node's banner (see "The image on the
      node is the CI artifact", below)
  - files: []
    reason: "bench-only, read-only task -- NO repository file changed; this reply
      file is the only diff in the commit that stages it. The board was NOT
      flashed (constraint honoured), so there is no new firmware revision and no
      new CI run for this task; the pointers above are the revision the node is
      running and the run that produced it."
  - api: "GET https://api.nordtronics.io/v1/nodes -> node_id 0, status ok,
      battery_pct 100, latest.battery_v 4.125, reading_count 2407, age 51 s,
      generated_utc 2026-10-08T18:22:06Z; GET /v1/nodes/0/readings?limit=3 ->
      battery_v 4.125 / 4.125 / 4.106 V"
  - serial_probe: "node console @ /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00
      (ttyACM1), 115200 8N1, captured 2026-10-08 18:19:13-18:19:58 UTC, 6075 B;
      the three proof blocks are quoted verbatim in the reply body"
  - identity: "esptool --chip esp32s3 read_mac BEFORE and AFTER: b0:a6:04:c5:75:4c
      (node, == the constraint's B0:A6:04:C5:75:4C); base (80:F1:B2:A7:47:EC /
      ttyACM0) never opened, never addressed"
notes: |
  STAGED (iteration 1), 2026-10-08 18:15-18:30 UTC. Off-peak (PEAK: OFF-PEAK
  18:15 UTC). PROTOCOL: MATCH protocol_version=1.0.0. Model tier: DeepSeek Flash
  (deepseek-flash / provider deepseek) -- the cron worker's own cheapest tier, as
  the Constraints section requires.

  THE NODE IS ALIVE AND RUNNING THE 0119 CODE. It was already awake and emitting
  its 1 Hz `pms-live` line on the first non-perturbing read, before anything
  touched it, so the 0119 recovery image survived the discharge run. Three proofs
  are in the body: (1) boot banner text, (2) the GPIO1/GPIO2 ADC pin probe, and
  (3) the live API battery_v. Read the body for the verbatim captures.

  WHAT THE PROBE SETTLED. `batt-probe: gpio1 raw=3484 adc_mv=847` and
  `batt-probe: gpio2 raw=44 adc_mv=10`. GPIO1 carries the divider (847 mV at the
  tap = 4150 mV implied at the cell); GPIO2 reads 10 mV, i.e. nothing is connected
  to it -- the 0119 correction (the spec named GPIO2; the V4.2 datasheet and the
  0079 frozen set say GPIO1/ADC1_CH0) is now bench-confirmed rather than
  datasheet-derived. That probe had never run before: 0119 could not reach a single
  application line on a board that was in a ROM boot loop.

  THE METER LEG IS NOT PERFORMED AND IS NOT CLAIMED. The success criterion lists a
  "meter comparison"; there is no reading from Stephen's meter anywhere in the task
  text, the bench record, or the repo/cron history (checked), and a meter is a
  physical instrument at the bench. 0119's own criterion 4 assigns this leg to
  Stephen as his step and asks only that the reply state it. This reply states it:
  the meter comparison is outstanding. What IS performed is the agreement check
  between the three values this run could obtain -- probe-implied 4150 mV,
  OLED/checkin latched 4116-4125 mV, API 4.106-4.125 V -- a max spread of 34 mV,
  comfortably inside the +/-100 mV the 0119 bench note asks for. Stephen's meter on
  the battery terminals is the remaining leg.

  THE OLED READING IS THE FIRMWARE'S OWN VALUE, NOT A PHOTOGRAPH. I did not
  visually read the panel -- there is no camera on this host and the firmware does
  not echo the OLED frame to serial. What is true: `oled: probe 0x3C -> ACK` (panel
  present, rendering enabled) and the panel's `vbat:%umV` string is built from
  `g_batt_mv` (main.cpp:819), the same variable the checkin's `batt:` line logs
  (`batt: pin=1 raw=3465 adc_mv=840 vbatt=4116 mV`, then 4125 mV at the next two
  checkins). So the OLED shows the checkin figure; I did not read the glass.

  THE IMAGE ON THE NODE IS THE CI ARTIFACT. The banner's build stamp is `__DATE__
  __TIME__` baked at compile time (main.cpp:989-990), and the string pair
  `21:22:14\0Oct  6 2026\0` sits immediately before the banner format string inside
  firmware.bin downloaded from run 37533195529 (offsets 6679/6688, banner format at
  6710), with the artifact sha256 equal to 0119's recorded value. The run's window
  (21:20:36-21:22:44Z) contains that stamp. LIMIT: this is a string match against
  the CI artifact, not a full hash of the chip's flash -- the task is read-only and
  0119 found this board's flash read-back path unreliable, so I did not read the
  flash to compare it. "The node runs the CI image" is evidenced by the baked build
  stamp, not by a flash read-back, and I want that read as a limit.

  ONE DELIBERATE RESET, DECLARED (the only board-side side effect). The board was
  NOT flashed, NOT erased, and NO NVS key/partition/portal field was written -- the
  read-only constraint is honoured. But a boot banner cannot be captured without a
  boot, so the board was reset ONCE via esptool's RTC-watchdog reset
  (`--before default_reset --after watchdog_reset read_mac`, 18:19:13 UTC). That
  restarts uptime and fires an immediate checkin (the capture shows `tx: type=1
  node=0 seq=0`), no more. Two earlier `--before no_reset` probes failed to connect
  (rc 1 and rc 2) and touched nothing -- expected, since the running app owns the
  USB-Serial/JTAG channel; `--before default_reset` is what connects on this board.
  A DTR/RTS-style reset lands in ROM download boot on this part, so the watchdog
  variant was used because it boots app0.

  THE ROM-LEVEL BANNER IS NOT IN THE CAPTURE. `cat` opened the port ~0.5 s after
  the watchdog reset, so the capture starts at the app's own t=509 ms and the
  `ESP-ROM:esp32s3-...` / `rst:...` lines at t~0-100 ms are before the window. The
  "boot banner" quoted below is the firmware's full boot sequence, banner line
  included. I could not have the port open during the reset: esptool needs
  exclusive access to it. I am saying this rather than presenting a partial capture
  as the whole ROM header.

  OBSERVATIONS THAT DID NOT MATCH / ARE NOT MINE TO CLAIM. (a) Every PM reading in
  the capture is `pm1=0 pm25=0.0 pm10=0`; that is the carried-over PMS state from
  0117/0118, unchanged by anything in this run, and not part of 0128 -- flagged so
  it is not read as a new defect. (b) The API's post-boot reading is 4.106 V while
  the boot line prints vbatt=4116 mV; both are within 10 mV and both are inside the
  +/-100 mV band, but they are not bit-identical because the probe, the checkin
  average and the API record are three different samples at three different instants.
  (c) Nothing else deviated from expectation.

  NO NTFY RECEIPT: 0128 is not a build task -- nothing was compiled or packaged and
  there is no new artifact to announce, so the README's build-green ntfy rule does
  not apply. Publishing a "Status: success" build receipt for a read-only bench run
  would misreport it.

  VERIFICATION METHOD, STATED PLAINLY. Model tier: DeepSeek Flash. Every number in
  this reply was produced by a command in this run on this host: the serial capture
  (`stty -F <by-id> 115200 raw -echo -hupcl` + `cat`), `esptool read_mac`, `curl`
  against the live API, `gh run view/download`, `git ls-remote`, and `grep`/Python
  against the downloaded artifact. Pasted output is a claim; the durable pointers in
  `proof:` are the branch/SHA/run/artifact/API the verifier can re-check itself.
---

# 0128 — 0119 live-half proof: node is back on USB

## Reply — STAGED

The node is on USB, running the 0119 image, and the API is serving its battery_v.
All three proofs follow, verbatim from the capture of
2026-10-08 18:19:13-18:19:58 UTC on `ttyACM1`
(`/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`).

### Proof 1 — boot banner text

Full app boot sequence (verbatim, from the capture; the `[ 509]` prefix is the
firmware's own millis() timestamp, so the first line here is ~0.5 s after the reset):

```
[   509][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: lora_mhz NOT_FOUND
[   517][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: lora_bw NOT_FOUND
[   526][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: abs_floor NOT_FOUND
[   534][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: rel_delta NOT_FOUND
[   543][E][Preferences.cpp:483] getString(): nvs_get_str len fail: role NOT_FOUND
probe: i2c 0x77 -> ACK
ROLE: node (source=PROBE, sensors=present, nvs_role=unset)
firmware: wildfire-unified-v1 proto=1 build=Oct  6 2026 21:22:14
node: deep sleep gate=DISABLED (bench phase) -- check-in period 60 s (field value 720 s + sleep restores at the final gate)
radio: begin(915.0MHz bw125k sf7 cr5 sync=0x12 20dBm) -> ok
oled: probe 0x3C -> ACK
batt-probe: gpio1 raw=3484 adc_mv=847
batt-probe: gpio2 raw=44 adc_mv=10
wifi: dhcp hostname [wildfire-node-01]
portal AP fallback: wildfire-setup  http://192.168.4.1 (boot)
mqtt cfg: host=[mqtt.nordtronics.io] port=8883 (TLS, pinned ISRG Root X1)
pms: rx_avail_before=32 ok=1 pm1=0 pm25=0.0 pm10=0
batt: pin=1 raw=3465 adc_mv=840 vbatt=4116 mV
tx: type=1 node=0 seq=0 len=27 -> sent
bench gate: deep sleep DISABLED -- staying awake, next check-in in 60 s
```

The banner line is `firmware: wildfire-unified-v1 proto=1 build=Oct  6 2026 21:22:14`.
The ROM-level header (`ESP-ROM:esp32s3-...`) is not in the window; see `notes:`.

### Proof 2 — GPIO1 ADC pin probe (and the OLED/API comparison)

```
batt-probe: gpio1 raw=3484 adc_mv=847
batt-probe: gpio2 raw=44 adc_mv=10
batt: pin=1 raw=3465 adc_mv=840 vbatt=4116 mV
...
batt: pin=1 raw=3461 adc_mv=842 vbatt=4125 mV
batt: pin=1 raw=3456 adc_mv=842 vbatt=4125 mV
```

| Quantity | Value | Source |
|---|---|---|
| GPIO1 (ADC1_CH0) probe, calibrated | **847 mV** (raw 3484) | `batt_adc_pin_probe()` at boot |
| GPIO1 implied cell voltage (÷ 0.2041) | **4150 mV** | computed |
| GPIO2 (ADC1_CH1) probe, calibrated | **10 mV** (raw 44) | `batt_adc_pin_probe()` at boot |
| GPIO2 implied cell voltage (÷ 0.2041) | 49 mV | computed |
| OLED `vbat:` source value (checkin latch) | **4116 mV** at boot, 4125 mV at the next two checkins | `g_batt_mv`, the variable the panel renders |
| API `battery_v` | **4.106 → 4.125 V** | `GET /v1/nodes`, `/v1/nodes/0/readings` |
| Stephen's meter | **not performed** | see below |

- **The GPIO1-vs-GPIO2 question is now bench-settled.** The 0119 spec named GPIO2;
  the correction to GPIO1 (ADC1_CH0, per the V4.2 datasheet and the 0079 frozen
  connected set) was datasheet-derived because the board never reached an
  application line. With the divider live, GPIO1 reads 847 mV (a real cell tap) and
  GPIO2 reads 10 mV (an open pin). GPIO1 is the divider's tap.
- **Agreement among the three obtainable values: 34 mV spread** (4150 / 4125 / 4125
  mV; 4116 at boot), inside the ±100 mV the 0119 bench note asks for.
- **The OLED figure is the firmware's value, not a photograph.** `oled: probe 0x3C
  -> ACK` shows the panel is present and rendering; the `vbat:%umV` string is built
  from `g_batt_mv` (main.cpp:819), the same variable the `batt:` line logs. I did
  not read the glass — no camera on this host.
- **The meter comparison is outstanding, not performed.** No meter reading exists in
  the task, the bench record, or the repo/cron history, and a meter is a physical
  instrument at the bench. 0119 criterion 4 assigns this leg to Stephen and asks only
  that the reply state it. It is stated here as the remaining leg.

### Proof 3 — live API `battery_v`

`GET https://api.nordtronics.io/v1/nodes` (2026-10-08T18:22:06Z):

```json
{"nodes":[{"node_id":"0","first_seen_utc":"2026-10-05T22:18:22Z",
"last_seen_utc":"2026-10-08T18:21:15Z","reading_count":2407,"age_seconds":51,
"status":"ok","battery_pct":100,"latest":{"pm25":0.0,"temperature_c":24.5,
"humidity_pct":31.06,"battery_v":4.125}}],"count":1,"stale_after_seconds":900,
"generated_utc":"2026-10-08T18:22:06Z"}
```

`GET https://api.nordtronics.io/v1/nodes/0/readings?limit=3`:

```
2026-10-08T18:21:15Z  battery_v 4.125
2026-10-08T18:20:15Z  battery_v 4.125
2026-10-08T18:19:09Z  battery_v 4.106
```

`GET https://api.nordtronics.io/healthz` → `{"status":"ok","database":"ok","schema_version":2,...}`.
Node `"0"`, `status: ok`, `battery_pct: 100`, `battery_v: 4.125` — the API is
serving a live, non-zero `battery_v` for this node, and the reading count (2407)
continues past the 2241 the discharge run ended on.

### The image on the node is the CI artifact

The banner's `build=` stamp is the compiler's `__DATE__ __TIME__`
(`firmware/wildfire-node-v1/src/main.cpp:989-990`), baked into the image:

```
$ grep -aob "wildfire-unified-v1 proto=" firmware.bin     # run 37533195529 artifact
6710:wildfire-unified-v1 proto=
$ # bytes just before that banner format string, offset 6688:
b'02X -> %s\x00probe: pms5003 -> %s\x0021:22:14\x00Oct  6 2026\x00firmware: wildfire-unified-v1 proto=%u build=%s %s'
$ sha256sum firmware.bin
c5004dd85f5a040efebd8e7c21f556ba1f5ff22f9db6db6c9f7cea4844548df4
```

The live node printed `build=Oct  6 2026 21:22:14`; the artifact carries
`21:22:14` + `Oct  6 2026` as a byte pair; its sha256 equals the sha256 0119
recorded; and run 37533195529's window (created 21:20:36Z, updated 21:22:44Z,
`headSha` `d6fce0e` == the branch tip) contains that compile timestamp. **Limit:
this is a string match against the CI artifact, not a hash of the chip's flash** —
0128 is read-only and 0119 found this board's flash read-back unreliable, so no
flash read was used to compare. The node is evidenced to be running the CI image by
its baked build stamp, not by a read-back.

### Bench discipline and the one declared side effect

- **Identity.** `esptool ... read_mac` printed `b0:a6:04:c5:75:4c` both before and
  after the session, matching the constraint. The base
  (`80:F1:B2:A7:47:EC` → ttyACM0) was never opened or addressed.
- **No flash, no erase, no NVS write.** The read-only constraint is honoured. Two
  `--before no_reset` connect attempts failed (rc 1, rc 2) and wrote nothing;
  `--before default_reset` is what connects on this board.
- **One reset, declared.** A boot banner needs a boot, so the board was reset once
  via esptool's RTC-watchdog reset (`--after watchdog_reset`, 18:19:13 UTC) — a
  DTR/RTS reset lands in ROM download boot on this part, so the watchdog variant,
  which boots app0, was used. Effect: uptime restarted and one immediate checkin
  (`tx: type=1 node=0 seq=0`) went out; nothing else.

### What did not match expectations

1. **The ROM-level `ESP-ROM:` / `rst:` header is not in the capture** — the reader
   opened ~0.5 s after the reset (esptool needs exclusive port access, so the reader
   cannot be open during it). The banner quoted above is the firmware's full boot
   sequence.
2. **The meter comparison could not be produced** — it is Stephen's physical step
   and no reading was relayed with the task.
3. **The OLED figure is the firmware's rendered value, not a photograph of the
   panel** — no camera on this host, and the firmware does not echo the frame to
   serial.
4. **PM readings are all zero** (`pm1=0 pm25=0.0 pm10=0`) — carried over from
   0117/0118, unchanged by this run, flagged so it is not read as new.
