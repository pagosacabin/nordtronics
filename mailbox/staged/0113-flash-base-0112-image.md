---
task_id: "0113"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 12h
proof:
  branch: "hermes/0112-gate-node-deep-sleep"
  sha: "4f9c05e741c0980146fafb689e02a994787fde12"   # == git ls-remote --heads origin hermes/0112-gate-node-deep-sleep
  run: "https://github.com/pagosacabin/nordtronics/actions/runs/37349799749"
  run_check: "gh run view 37349799749: conclusion success, status completed, headSha 4f9c05e741c0980146fafb689e02a994787fde12 == the branch tip, createdAt 2026-10-05T17:37:20Z."
  artifact: "https://github.com/pagosacabin/nordtronics/actions/runs/37349799749/artifacts"
  artifact_detail: "wildfire-node-v1-unified-firmware, id 11362491397, 765409 B zip, expired false, created 2026-10-05T17:39:52Z. `gh run download` -> firmware.bin 1215856 B, sha256 0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb -- the exact image flashed to the BASE (matches the task's stated id + hash)."
  flashed_to: "BASE 80:F1:B2:A7:47:EC only, via its by-id path, app-only at 0x10000 (write spans 0x10000-0x138fff, well inside app0 0x10000-0x650000), read-back byte-identical (sha256 0d75da91..., cmp exit 0). No erase_flash, no --erase-all, no NVS write. NODE B0:A6:04:C5:75:4C never named to esptool and got no USB event all run."
  ntfy: "not applicable -- no compiled artifact this run. The 0112 artifact was built and CI-verified at 4f9c05e (run 37349799749); this task is flash/observe only and produced no build for the nordtronics-build-ed05a663 topic."
  files: []   # no repository file changed by this task; the mailbox transition is the only commit
  live_evidence: "backend api.nordtronics.io/v1/nodes/0/readings: 17 readings, first 2026-10-05T22:18:22Z, last 22:34:23Z, cadence exactly 60 s -- the BASE decoded the NODE's LoRa frames and republished them. Broker journal (deploy@89.117.21.105, Europe/Berlin +0200): wf-base-0's last 'exceeded timeout, disconnecting' at 2026-10-05T22:16:38Z (the OLD 0105 image), reconnect 22:18:16Z, and 0 timeouts since."
notes: |
  Filed by Juno, 2026-10-05 ~15:20 MDT. Stephen's call: "queue the new base
  code". The node is healthy on the 0112 build (verified, archived); the base
  still runs 0105 with the MQTT keepalive problem. Same unified image both
  ends eliminates protocol mismatch as a variable. No new build needed — the
  0112 artifact is CI-green and already flashed to the node.
  PICKED UP (iteration 0 -> 1) by the mailbox worker, 2026-10-05 22:15 UTC
  (PEAK: OFF-PEAK 22:15 UTC). Predecessors 0097/0098/0106/0107/0109/0110 remain
  decision-blocked in active/ and were not touched.
  STAGED (iteration 1) 2026-10-05 22:35 UTC — flash DONE and verified; base is
  live on the backend. One correction and several declared perturbations; all in
  the reply body. Headline: the base runs the 0112 image, holds a stable MQTT
  session (0 keepalive timeouts in 16+ min, vs a timeout every ~25 s on 0105),
  and the backend is now receiving the node's 60 s readings.
---

# 0113 — Flash the base to the 0112 unified image

## Context

The node (MAC B0:A6:04:C5:75:4C) is running the 0112 build: sleep gate off,
60 s checkin, LoRa TX every 60 s, stable on USB (0112 verified, archived).
The base (MAC 80:F1:B2:A7:47:EC) still runs the 0105 image: it connects to
mqtt.nordtronics.io:8883 then is disconnected for exceeding the keepalive
timeout ~30 s later, reconnects ~1/min, never publishes. 0109 also saw no
LoRa `rx:` lines on the base — but the node was not transmitting then.

Flash the base with the SAME 0112 image the node runs. The artifact is
already built and proven:
- Branch: hermes/0112-gate-node-deep-sleep @ 4f9c05e741c0980146fafb689e02a994787fde12
- Artifact: wildfire-node-v1-unified-firmware, id 11362491397
- firmware.bin sha256: 0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb
  (1215856 B)

The base's NVS is provisioned (0106: wifi connected, ip=192.168.1.71) —
an app-only flash preserves it. The unified firmware probes role at boot;
with no sensors the base resolves role=base and never sleeps.

## Task

1. Download the 0112 artifact (id 11362491397) and confirm firmware.bin
   sha256 = 0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb.
   If the artifact is expired or the hash mismatches, STOP and report —
   do not build a substitute without a new decision.
2. Read the base's partition table first (NVS safety, 0109's recipe).
3. Flash the BASE only (MAC 80:F1:B2:A7:47:EC, by-id path, never
   /dev/ttyACM0/ACM1): app-only at 0x10000, then read back and verify
   byte-for-byte against firmware.bin. No --erase-all, no erase_flash,
   NO NVS writes. Do NOT touch the node board in any way.
4. Observe the base serial for 10+ minutes, non-perturbing capture, and
   quote: the ROLE line (expect role=base), any `rx:` lines from the node
   (node TXes every ~60 s), and the MQTT connect/disconnect lines.

Cost: flash tier. Off-peak preferred; do not delay past the reply window.

## Success criteria

- Base running the 0112 image: quoted boot lines showing role=base and the
  0112 build string, read-back sha256 matching the artifact.
- Quoted `rx:` lines proving the base hears the node's LoRa packets, and
  quoted MQTT state lines (connected, or the keepalive disconnect if it
  persists — report what IS, not what should be).
- The node board received no write and no reset. No NVS writes anywhere.

## Constraints

- Base ONLY. App-only flash at 0x10000. No NVS writes, no base NVS wipe
  (its WiFi credentials must survive).
- If the base does not show `rx:` lines from the node within the observe
  window, report that honestly — do not reflash either board to chase it.

## Proof

- Artifact id + firmware.bin sha256 (pre-flash check).
- Read-back sha256 of the base's app0 matching firmware.bin.
- Quoted base serial: ROLE line, rx: lines (or their absence), MQTT lines.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.

# Reply — 0113 (iteration 1), status: staged

Run: 2026-10-05 22:15-22:36 UTC, off-peak (PEAK: OFF-PEAK 22:15 UTC), flash tier.
Host: the worker bench. Base = `usb-..._80:F1:B2:A7:47:EC-if00` (usb3/3-2),
node = `usb-..._B0:A6:04:C5:75:4C-if00` (usb1/1-2). Every command used a by-id
path; `/dev/ttyACM0`/`ACM1` were never named.

## Headline

The BASE now runs the 0112 image. It holds a stable MQTT session — **the 0105
keepalive defect is gone**, 0 `exceeded timeout, disconnecting` events in the
16.5 min since it came up, against one every ~25 s on 0105 — and the backend is
receiving the NODE's readings: **17 of them, first 22:18:22Z, last 22:34:23Z,
cadence exactly 60 s**. Those readings can only be produced by the BASE decoding
the node's LoRa frames and republishing them (see §5, correction 1).

## 1. Pre-flash artifact check — MET

`gh api /repos/pagosacabin/nordtronics/actions/artifacts/11362491397`:
`{"expired":false,"name":"wildfire-node-v1-unified-firmware","run":37349799749,
"size_in_bytes":765409,"created_at":"2026-10-05T17:39:52Z"}`.

`gh run download 37349799749 -n wildfire-node-v1-unified-firmware` ->
`firmware.bin`, 1215856 B,
`sha256 0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb`.

Both the id and the hash are byte-for-byte the values the task names, so no
substitute was needed and no decision is owed here.

## 2. Identity and NVS safety, both read BEFORE any write — MET

The run's own by-id topology, quoted from `flash-base.log`:

```
usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00 -> ../../ttyACM0
usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00 -> ../../ttyACM1
DEVPATH=/devices/pci0000:00/0000:00:08.1/0000:04:00.4/usb3/3-2/3-2:1.0/tty/ttyACM0
ID_SERIAL=Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC
```

`esptool --chip esp32s3 --port <base by-id> --before default_reset read_mac` ->
`MAC: 80:f1:b2:a7:47:ec` — the BASE, confirmed twice (once to park it in download
mode, once over the `no_reset` reconnect). The partition table was read first,
4096 B at 0x8000:

```
nvs        off=0x009000 size=0x005000  (end 0x00e000)
otadata    off=0x00e000 size=0x002000
app0       off=0x010000 size=0x640000
app1       off=0x065000 size=0x640000
spiffs     off=0x0c9000 size=0x360000
coredump   off=0x0ff0000 size=0x10000
```

The app-only write occupies 0x10000-0x138fff, i.e. roughly the first 0.4 MB of
app0's 6.4 MB — it cannot reach NVS at 0x9000-0xe000, nor otadata, nor spiffs.

## 3. Flash — app-only at 0x10000, read-back byte-identical — MET

```
--- app-only write: 0x10000, 1215856 bytes ---
Flash will be erased from 0x00010000 to 0x00138fff...
Wrote 1215856 bytes (740785 compressed) at 0x00010000 in 10.7 seconds
Hash of data verified.
--- verification ---
0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb  firmware.bin
0d75da91e32e0e45fccadcfaf454c50d99c47412d0ca0c2e3b027082c6045bfb  base-readback.bin
CMP OK: read-back is byte-identical to the artifact
```

No `--erase-all`, no `erase_flash`, no `nvs`/`nvs_partition_gen` call anywhere in
the run. NVS survived in practice as well as on paper: the board came up
`ROLE: base (source=NVS, sensors=absent, nvs_role=1)` and joined the stored
network without any credential being read or written.

## 4. Boot and observation — 13 min, 259/259 samples up

`esptool ... --before no_reset --after watchdog_reset read_mac` ->
`Hard resetting with a watchdog...` (the RTC-domain reset; the DTR/RTS reset used
first lands in ROM download mode on this chip, as 0110/0112 established).

Window `2026-10-05T22:17:50Z -> 22:30:54Z` (780 s). `monitor.log`:
`samples=260 present=259 absent=0`; capture 1063 B. Kernel side,
`journalctl -k | grep 'usb 3-2'` sampled every 5 s for the whole window: **zero
events** — the base never re-enumerated or dropped (its only two events of the
run are the pair at 22:17:49-50 caused by this run's own resets, device 14 -> 15).

The complete serial capture (19 lines, CR-stripped, verbatim):

```
probe: pms5003 -> no frame
ROLE: base (source=NVS, sensors=absent, nvs_role=1)
firmware: wildfire-unified-v1 proto=1 build=Oct  5 2026 17:39:20
base: deep sleep is disabled for this role (asserted on every sleep path)
radio: begin(915.0MHz bw125k sf7 cr5 sync=0x12 20dBm) -> ok
oled: probe 0x3C -> ACK
wifi: dhcp hostname [wildfire-base-01]
portal AP fallback: wildfire-setup  http://192.168.4.1 (boot)
wifi: disconnected -- reason=2 (sta down)
wifi: connected -- ip=192.168.1.71 gw=192.168.1.1 rssi=-23 dBm
wifi: status=3 (connected) ip=192.168.1.71
wifi: disconnected -- reason=33 (sta down)
ntp: no time yet (t=22) -- TLS verification will fail until it syncs
mqtt cfg: host=[mqtt.nordtronics.io] port=8883 (TLS, pinned ISRG Root X1)
portal AP fallback: wildfire-setup  http://192.168.4.1 (join failed)
wifi: retrying join to [NordNickell] (status=6)
[ 22958][E][WiFiGeneric.cpp:1583] hostByName(): DNS Failed for mqtt.nordtronics.io
wifi: connected -- ip=192.168.1.71 gw=192.168.1.1 rssi=-23 dBm
mqtt: connected to mqtt.nordtronics.io:8883
```

`build=Oct  5 2026 17:39:20` sits inside run 37349799749's window
(createdAt 17:37:20Z, artifact 17:39:52Z) — the base is running the 0112 image,
not 0105. The two `wifi: disconnected` lines and the DNS failure are boot
transients before NTP/DNS settle; the base ends connected at 192.168.1.71 with
the MQTT session up.

## 5. Success criteria, item by item

**"Base running the 0112 image: quoted boot lines showing role=base and the 0112
build string, read-back sha256 matching the artifact."** — MET. All three are in
§3/§4: `ROLE: base (source=NVS, sensors=absent, nvs_role=1)`,
`build=Oct  5 2026 17:39:20`, and the read-back hash equal to the artifact's.

**"Quoted `rx:` lines proving the base hears the node's LoRa packets, and quoted
MQTT state lines (connected, or the keepalive disconnect if it persists — report
what IS, not what should be)."** — SPLIT. The `rx:` half is UNATTAINABLE AS
WRITTEN and is reported as such (correction 1 below); the reception itself is
MET, by better evidence:

* `https://api.nordtronics.io/v1/nodes/0/readings` returns 17 readings,
  `first 2026-10-05T22:18:22Z`, `last 2026-10-05T22:34:23Z`, every interval
  exactly 60 s. `/v1/nodes` lists node `"0"` with `first_seen_utc
  2026-10-05T22:18:22Z`, `reading_count 17`, `status "ok"`, latest
  `pm25 0.0, temperature_c 26.76, humidity_pct 40.81, battery_v 0.0`.
* That is the base's own telemetry path and nothing else can produce it: the
  0112 firmware publishes telemetry from exactly one call site,
  `main.cpp:535` inside `base_handle_frame`, reached only from
  `base_radio_poll` after `g_radio.receive()` + `wf::decode_frame()` return
  successfully (`main.cpp:568-582`). The loop itself has no periodic telemetry
  publish.
* Corroboration on the broker, `ssh deploy@89.117.21.105`, mosquitto journal
  (VPS is Europe/Berlin, +0200 — `00:18:16+02:00` is `22:18:16Z`):

  ```
  2026-10-06T00:16:15+02:00 ... New client connected ... as wf-base-0 (p2, c1, k15, u'base-01').
  2026-10-06T00:16:38+02:00 ... Client wf-base-0 has exceeded timeout, disconnecting.
  2026-10-06T00:18:16+02:00 ... New client connected ... as wf-base-0 (p2, c1, k15, u'base-01').
  ```

  The `00:16:38` timeout is the last one in the journal, i.e. the OLD 0105 image.
  `journalctl -u mosquitto --since 2026-10-05T22:18:30Z | grep -c
  "exceeded timeout"` -> `0`, and no `wf-base` line at all after 22:18:16Z:
  the 0112 image's session has held for 16+ minutes without a single keepalive
  drop, where 0105 lost it every ~25 s.

  **MQTT state lines, reported as they are**: the base's console shows exactly one
  `mqtt: connected to mqtt.nordtronics.io:8883` for the whole window — and that
  singular line is the correct shape, since the firmware logs `mqtt: connected`
  on *every* successful (re)connect. On 0105 the same window produced ~13 of
  them; here the broker independently confirms one connection and no disconnect.

**"The node board received no write and no reset. No NVS writes anywhere."** —
MET.

* No esptool invocation in this run named the node: `grep -rn
  'B0:A6:04:C5:75:4C|ttyACM1|b0:a6'` over the run's scripts and logs hits only
  (a) the read-only `ls -l /dev/serial/by-id/` listing and (b) the *historical*
  kernel lines for 07:54 (when the node was still cabled to 3-2). Nothing else.
* `journalctl -k`, node port `usb 1-2`: its last event of any kind is
  `11:40:59 USB disconnect ... / new full-speed USB device number 83` (the 0112
  flash's own reset) and there are **zero** `usb 1-2` events between then and
  22:36 UTC — 10 h 55 min of continuous enumeration across this whole run.
* The only flash write was app-only at 0x10000 on the base; the base's partition
  table, read before the write, puts NVS at 0x9000-0xe000.

## 6. Deviations, corrections and perturbations — declared

1. **CORRECTION (mechanism the spec asserts, not its facts): a successful LoRa
   reception in the 0112 firmware logs NOTHING, so "quoted `rx:` lines" is
   unattainable as a criterion and its absence is not evidence of deafness.**
   `main.cpp:568-599`: the only two `rx:` lines are
   `rx: dropped frame (%s)` (decode failure) and
   `rx: node %u is not provisioned -- ignored` (ACL/provisioning miss); a frame
   that decodes, is provisioned, and is not an alarm or ACK-required goes
   straight into `base_handle_frame()` and out to MQTT with no serial line. An
   ACK-required frame would log `tx: ACK seq=... for node ...`. So the criterion
   is satisfiable only by accident (a defect), and the honest substitute for
   "the base hears the node" is the delivered payload — §5, 17 readings at an
   exact 60 s cadence, which no other code path can produce. I did not reflash
   anything to chase the missing line, per the task's own constraint.
2. **PERTURBATION — the base was reset twice, deliberately.** A DTR/RTS reset
   (`--before default_reset`) to park it in ROM download mode, and the RTC
   watchdog reset (`--after watchdog_reset`) to boot app0. Step 4 needs a running
   app, and on this chip family only the watchdog reset boots app0. The base is
   the board the task asked me to flash, so this is in scope; recorded because
   the "no reset" clause in the criteria is about the NODE.
3. **PERTURBATION — the base re-enumerated from `/dev/ttyACM0` to
   `/dev/ttyACM2`** (`ls -l /dev/serial/by-id/` after the run). Harmless: the
   whole run used the by-id path, which re-pointed itself at 16:17:50 local.
4. **Capture method and its limit.** Non-perturbing `stty -F <by-id> 115200 raw
   -echo -hupcl` + `timeout 3 cat`, looped (so the reader is closed and reopened
   roughly every 3 s). Small reader gaps are possible in principle; `present=259
   absent=0` and the single-connect agreement with the broker's independent
   journal are the check that the capture was complete for the events that
   matter.
5. **`ntfy` receipt: not applicable this run.** No compiled artifact was
   produced — the 0112 image was already built and CI-verified at 4f9c05e (run
   37349799749), and the task's own text says "No new build needed". Nothing to
   publish to `nordtronics-build-ed05a663`.
6. **No repository file changed.** This is a flash/observe task; `proof.files` is
   empty by construction and the mailbox transition is the run's only commit.
   There is therefore no task branch and no CI run for 0113 itself — the run
   cited in `proof` is the one that built and green-verified the revision that
   was flashed, with `headSha` re-checked against the branch tip.
7. **The task's premise about the base is CONFIRMED, not corrected** — worth
   stating because it is now the falsifiable record: on 0105 the base "connects
   then is disconnected for exceeding the keepalive ~30 s later, reconnects
   ~1/min" is exactly what the broker journal shows up to 22:16:38Z, and the
   0112 image ends it (§5).

## 7. What this does NOT prove

* The backend now holds readings for node `"0"` with `first_seen_utc
  2026-10-05T22:18:22Z` — this appears to be the first time the node's data has
  ever reached the backend, which touches the blocker 0106/0107/0109/0110
  recorded ("no reading can reach the backend"). I did not investigate, reopen or
  edit any of those tasks (rule 6); flagging the observation for Juno, who owns
  whether their blockers are now discharged. `0097`/`0098` are untouched and
  unrelated to this.
* `battery_v 0.0` and `pm25 0.0` in every reading: the node's battery ADC is not
  wired and the PMS5003 is absent (`probe: pms5003 -> no frame`), which matches
  0112's note that this bench node has no PMS. Nothing here validates those two
  fields.
* Nothing in this run proves the node's own WiFi/uplink: the node has no stored
  WiFi credentials, so its readings reach the backend via the base's LoRa ->
  MQTT relay, not from the node itself.

Raw captures outside the repo, in
`~/.hermes/profiles/cronrunner/cache/scratch/0113/`: `dl/firmware.bin`,
`base-readback.bin`, `base-parttable.bin`, `base-console.bin`, `flash-base.log`,
`monitor.log`, `usb-events.log`, plus the two scripts.

## Cost

One off-peak run (PEAK: OFF-PEAK 22:15 UTC), flash tier. Zero CI runs started,
one artifact downloaded, one flash of the base, one 13-minute capture window.
