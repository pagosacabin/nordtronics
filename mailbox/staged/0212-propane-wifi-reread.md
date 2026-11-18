---
task_id: "0212"
protocol_version: 1.3.0
status: staged
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T20:01:01+00:00"
lease_until: "2026-11-18T21:31:01+00:00"
expect-reply-within: 6h
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
proof:
  - evidence: mailbox/files/0212/capture-0212-redacted.log
    kind: serial-log
    note: "full 600 s console capture, ANSI-stripped, SSID masked; no branch and no CI run exist for this task (bench-only, no repository code change)"
notes: |
  STAGED (refused join, the task's documented either-way branch). Run
  2026-11-18 13:01-13:11 MST (20:01-20:11 UTC), off-peak (PEAK: OFF-PEAK 20:00
  UTC), PROTOCOL MATCH 1.3.0. One capture window, no reflash, no credential or
  config change.

  TASK CHOICE. 0212 was taken as the lowest actionable task: 0142 and 0187
  carry status: superseded, 0159 is decision-blocked, and 0209 is the
  decision-blocked task this one exists to unblock (its spec says leave 0209 in
  active/ either way - it is untouched, iteration and notes unchanged). 0176 in
  inbox/ is status: superseded (withdrawn 2026-11-12, "do not pick up").

  FRONT-MATTER ADDED (rule 16): 0212 arrived with no front-matter at all, so the
  claim had nowhere to record state. task_id / protocol_version /
  expect-reply-within were added at staging; the claim wrote status, iteration
  and claimed_*. Declared here rather than treated as a malformed task.

  THE THREE-WAY COMPARISON - ALL MATCH (values never printed; sha256 + length
  only):
    (a) file vs flashed   MATCH   both 9 chars, sha256 d2462d2f32d1...
    (b) file vs host PSK  MATCH   both 9 chars, sha256 d2462d2f32d1...
    (c) flashed vs host   MATCH   both 9 chars, sha256 d2462d2f32d1...
  No DIFFER anywhere, so no first-difference index and no case-only flag to
  report. The file was rewritten at 12:50 MST today (22 bytes, sha256
  96d7cdae...; mtime after 0211's 10:46 capture) and now holds the lowercase
  first-character value Stephen described. 0209's preserved "staged as written"
  copy (9 chars, sha256 eb6614b9be69...) differs from the current file at
  exactly index 0, case only - the old flub, still on disk only in 0209's
  scratch. Worth one line for the record: that old value is byte-identical to
  this host's saved PSK for a DIFFERENT network profile ("Nightrain"), which
  fits Stephen's copy-from-the-wrong-line account better than a typo.

  A SECOND SAME-BAND REFERENCE, new this run: the host also holds its own saved
  profile for the 2.4 GHz SSID itself (ssid 11 chars, sha256 5e7bb8150749...,
  identical to the SSID the board targets) and its stored PSK is
  d2462d2f32d1... - the same value as the 5 GHz sibling, the file and the
  board. So the reference truth used in 0211 (a 5 GHz sibling profile) is
  confirmed by a 2.4 GHz profile for the same SSID, not merely by Stephen's
  statement that both bands share one passphrase.

  NO REFLASH, per step 3 of the task: the flashed value already equals the host
  PSK, so the board already holds the correct credential and flashing from any
  file would only risk putting a stale value back. Image check from this
  capture's own boot banner: "ESPHome version 2026.9.1 compiled on 2026-11-18
  08:20:18 -0700" - byte-for-byte 0209's final image, so the board is not
  running a stale build. NVS untouched (threshold still 2.50 V).

  CAPTURE (600 s, 20:01:10-20:11:10 UTC, one window as constrained): 100
  connection attempts, every one in phase SCAN_CONNECTING; 99 deauthentications,
  ALL reason='Auth Expired' (no other reason code); 49 "Restarting adapter";
  0 'Got IP'; 0 API client connections (the api component only logs its normal
  "waiting for client connection" warning); 60 monitor lines with MQ-6
  1.510-1.542 V, Figaro 1.458-1.478 V, thr=2.50 V, alarm=off.

  FINDING. The same failure signature as 0211 - 99/100 attempts refused with
  Auth Expired - reproduced AFTER Stephen's ~12:20 MST router reboot and with
  the credential confirmed three ways including a same-band host profile. The
  credential question is therefore closed on every side that can be closed from
  this host: file, board and host references agree byte-for-byte, and the
  router's own reboot did not change the outcome. What remains is AP-side
  refusal of this client (9c:cc:01:d0:78:80): a device-level block/pause,
  access-control entry, client limit, or a 2.4 GHz radio/SSID profile that will
  not admit this client. PMF/WPA3 is not the explanation: 0209 already measured
  the 2.4 GHz BSSID's RSN IE as pair_ccmp/group_ccmp/psk/sae with no MFPR bit,
  and ESPHome sets pmf capable=true, required=false.

  ASK FOR STEPHEN, one check that costs nothing: in the Starlink router's device
  list, look for 9c:cc:01:d0:78:80 (or an "unnamed"/ESP device) and clear any
  paused/blocked entry, and confirm the 2.4 GHz SSID the board is joining is the
  main network rather than a guest/IoT-isolated one. Nothing on the board side
  needs to change; the next capture will show the join on its own.

  DEVIATION DECLARED. Opening the port resets the board over USB Serial/JTAG
  (rst:0x15 (USB_UART_CHIP_RESET), boot:0xd (SPI_FAST_FLASH_BOOT), app0 boots
  normally). This capture caused that reset; every refusal in the log is
  post-reset.

  NO REPO CODE CHANGE: files: [] and run: none are the honest shape here.
  `.github/workflows/` holds android-build.yml (push branches
  [android-toolchain-setup]), android-companion-v0.yml (branch allow-list +
  android/companion-v0 paths), backend.yml (backend/**), detection-sim.yml
  (python/detection-sim/**), platformio.yml (firmware/tank-monitor/**,
  firmware/node-v1/**, firmware/wildfire-node-v1/**, python/detection-sim/**)
  and website-check.yml (branch allow-list); none matches a mailbox-only change
  and no branch was pushed. The firmware source of record stays
  hermes/0208-propane-bench-firmware @ ccece52, unchanged.

  COST FIELDS ARE `unknown` because this harness exposes no per-call DeepSeek
  usage to the cron session - not estimated.
---

# 0212 — three-way password check: all MATCH; board still refused after the router reboot

## Status

**Verified on the credential, refused on the join.** Every value that can be
compared from this host agrees byte-for-byte, and no reflash was made. The board
still does not join.

## Device IP

**none.**

## The three comparisons

| Pair | Result | Detail |
|---|---|---|
| (a) file vs flashed | **MATCH** | 9 chars, sha256 `d2462d2f32d1…` both |
| (b) file vs host PSK | **MATCH** | 9 chars, sha256 `d2462d2f32d1…` both |
| (c) flashed vs host PSK | **MATCH** | 9 chars, sha256 `d2462d2f32d1…` both |

No DIFFER, so there is no first-difference index and no case-only flag to report.
`/home/astroboy/.hermes/wifi.txt` was rewritten at 12:50 MST today and now holds
the lowercase first character Stephen described. A second, same-band reference
was found this run: the host's own saved profile for the 2.4 GHz SSID (SSID
sha256 `5e7bb8150749…`, identical to the SSID in the board's log) carries the
same PSK `d2462d2f32d1…`.

## Reflash

**Not performed, deliberately.** The board already holds the host-PSK value, so
step 3's first case applies: rebuilding from any file could only risk putting a
stale value back. The capture's own boot banner reads
`ESPHome version 2026.9.1 compiled on 2026-11-18 08:20:18 -0700` — the same
image 0209 flashed. NVS untouched; the 2.50 V threshold survives.

## Capture outcome (600 s, 20:01:10–20:11:10 UTC)

```
[I][wifi:1145]: Connecting to '<ssid>' (BE:D9:D7:28:BA:46) (priority 0, attempt 1/2 in phase SCAN_CONNECTING)...
[W][wifi_esp32:869]: Disconnected ssid='<ssid>' bssid=BE:D9:D7:28:BA:46 reason='Auth Expired'
[W][wifi:1729]: Connecting to network failed (callback)
[I][wifi:1145]: Connecting to '<ssid>' (BE:D9:D7:28:BA:46) (priority 0, attempt 2/2 in phase SCAN_CONNECTING)...
[W][wifi_esp32:869]: Disconnected ssid='<ssid>' bssid=BE:D9:D7:28:BA:46 reason='Auth Expired'
[W][wifi:759]: Restarting adapter
```

**100 attempts, 99 deauthentications — all `reason='Auth Expired'`, no other
reason code — 49 adapter restarts, 0 `Got IP`, 0 API client connections.** The
sensors keep reporting throughout (60 monitor lines, MQ-6 1.510–1.542 V, Figaro
1.458–1.478 V, `thr=2.50 V`, `alarm=off`), so the rung's non-network half is
healthy.

Full log: `../files/0212/capture-0212-redacted.log` (ANSI stripped, SSID masked;
BSSID kept, as 0209 already published it).

## Finding

The credential is settled from four independent directions — the staged file as
Stephen rewrote it, the value on the board, the host's 5 GHz sibling profile and
the host's own 2.4 GHz profile for the same SSID — and the failure survived a
router reboot. So the refusal is the AP's treatment of this client
(`9c:cc:01:d0:78:80`): a paused/blocked device entry, an access-control or
client-limit rule, or a 2.4 GHz radio/SSID profile that will not admit it. It is
not a PMF/WPA3 mismatch: the 2.4 GHz BSSID advertises
`pair_ccmp group_ccmp psk sae` with no MFPR bit, and ESPHome sets
`pmf capable=true, required=false`.

One check for Stephen, no board work needed: in the router's device list, clear
any paused/blocked entry for that MAC, and confirm the board is joining the main
network rather than a guest/IoT-isolated SSID. The board retries forever, so the
next capture window will show the join without a reflash.

## Constraints honoured

No credential change, no config change, no reflash, one capture window, NVS
untouched. The capture's own port open resets the board over USB Serial/JTAG
(`rst:0x15`, app0 boots normally) — disclosed above, not presented as the board
restarting itself. No value or substring of the password appears in this file;
hashes, lengths and indices only.
