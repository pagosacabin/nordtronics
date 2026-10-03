---
task_id: "0099"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 72h
notes: |
  Filed by: Hermes, from an interactive bench session, at Stephen's direction,
  2026-10-03 01:28 UTC. Number assigned locally: the highest task present on
  main at filing time is 0097, so this is the next free number — renumber if it
  collides with a task Juno has already authored.

  RENUMBERED 0098 -> 0099 by the worker at pickup (2026-10-03 02:4x UTC): Juno
  authored mailbox/inbox/0098-base-wifi-mixed-mode.md (origin/main 0b785df)
  after this file was committed (0907847), so this task took the next free
  number, per the paragraph above. Number only: the body heading, the branch
  name (hermes/0099-wildfire-fix-set) and the staged path were updated to
  match; no requirement, criterion or scope changed, and the fix set is
  untouched. It stays the next item in the queue behind 0098.

  This task is the code half of 0097's decision (b) ("authorize the TLS uplink
  change to firmware/wildfire-node-v1 (branch + CI)"), widened because the same
  bench session found six further defects that each independently stop this
  firmware from running or being configured on the physical base board. It
  deliberately contains NO bench leg: the hardware validation is already done
  and quoted below, so an unattended run can complete it in CI alone.

  Not in this task, and still 0097's to decide: the cabin WiFi credentials and
  the base-01 broker password, and who (if anyone) dumps the bench image.
---

# 0099 — wildfire-node-v1: apply the bench-proven fix set (TLS uplink + six defects)

## Context

Task 0097 is blocked, and its front-matter `notes:` item 4 records the first of
these defects — the 0094 unified firmware on `main` has **no TLS path**, so a
flashed board cannot reach the 8883-only broker however it is configured. It
lists the fix under the decisions owed and states plainly that it is not the
worker's to take unilaterally.

On 2026-10-02 (bench, base board MAC `80:F1:B2:A7:47:EC`) an interactive session
applied that fix set as a bench build and ran it on the physical board, after
which the firmware boots, serves its captive portal, holds a stable loop, and
is configured for the TLS broker. Six more defects surfaced in the process.
**Every one of them is invisible to CI: `main` is green today, and all seven
produce a board that either reset-loops, prints nothing, wedges, or cannot be
configured at all.**

Bench serial evidence from the fixed build (`Serial`, 115200):

    oled: probe 0x3C -> ACK
    cfg: ssid=[NordNickell] (11 chars), host=[mqtt.nordtronics.io]:1883
    wifi: status=4 (not connected)
    portal AP fallback: wildfire-setup  http://192.168.4.1
    bench: mqtt_port 1883 -> 8883 (TLS)
    mqtt cfg: host=[mqtt.nordtronics.io] port=8883
    hb loops=20 heap=263200

Reading of that evidence, which the fix set is designed to reproduce in the
repo: the panel answers on `0x3C` (it does not without a reset pulse); the
stored SSID arrives intact (before the trim it arrived as `nordnickell ` — all
lowercase **with a trailing space**, which is why the join previously failed
with `status=1` `WL_NO_SSID_AVAIL`); the network is now found and the remaining
`status=4` is `WL_CONNECT_FAILED`, i.e. an auth rejection on the password, not a
missing network; the fallback AP is up so the portal stays reachable; and the
loop heartbeats indefinitely with a healthy heap.

The seven defects, with their anchors on `main`:

1. **Upload size mismatch — boots to a reset loop.** `platformio.ini` sets
   `board_build.partitions = default_16MB.csv` but never `board_upload.flash_size`
   / `maximum_size`, so the uploader believes the 8 MB PlatformIO board JSON
   (`heltec_wifi_lora_32_V3`) while the image is laid out for 16 MB.
2. **No USB-CDC flag — silent console.** `build_flags` lacks
   `-DARDUINO_USB_CDC_ON_BOOT=1`, so on the ESP32-S3 `Serial` goes to UART0
   (GPIO43/44) and the native USB port reads as silence.
3. **Panel reset never driven** — `src/main.cpp:52`
   `U8G2_SSD1306_128X64_NONAME_F_HW_I2C g_oled(U8G2_R0, U8X8_PIN_NONE)` and
   nothing touches GPIO21: the panel does not ACK and U8g2's init spins. Drive
   RST (LOW, delay, HIGH, delay) before `Wire.begin`, and probe `0x3C`.
4. **`sendBuffer()` wedges the I2C bus** — `src/main.cpp:478` (and `:621`): the
   bulk 1024-byte write hangs `loop()` when the panel does answer, and that
   starvation is exactly why the captive portal never replied. Gate the render
   on a successful probe and do not bulk-write an unproven panel.
5. **Portal runs on an uninitialised TCP/IP stack** — `portal_setup()`
   (`src/main.cpp:481`, ending in `g_server.begin()` at `:500`) is called
   unconditionally at `:569`, while `WiFi.mode(WIFI_STA)`/`begin()` run only
   inside `if (g_cfg.wifi_ssid.length())` at `:562`. The result is
   `assert failed: tcpip_send_msg_wait_sem ... (Invalid mbox)` → `rst:0xc`,
   triggered from `g_server.handleClient()` at `:582`. Bring up
   `WiFi.mode(WIFI_AP)` + `softAP()` **before** the portal.
6. **Failed join locks you out of the portal** — once an SSID is stored and the
   join fails, the board is on neither network and `192.168.4.1` is gone. Fall
   back to the AP on a failed join.
7. **Portal corrupts stored values, and the uplink is plaintext** —
   (a) the form emits `value='...'` into a **single-quoted attribute with no
   escaping**, so any value containing `'`, `"`, `<` or `&` is truncated or
   mangled on the round trip; that is the mechanism behind the leading case
   above, and it can silently corrupt a password. Escape `& < " '` (String
   overloads — `String::replace` has no `(char, const char*)` form, a compile
   error worth knowing up front).
   (b) `src/main.cpp:54-55` are `WiFiClient` + `PubSubClient` — plaintext only;
   `src/firmware_config.h:102-103` default to `api.nordtronics.io:1883`, and
   that host is Cloudflare with nothing on 1883 *or* 8883. Use
   `WiFiClientSecure` with the ISRG Root X1 CA (per 0097 — pin the root, do not
   ship `setInsecure()`), default host `mqtt.nordtronics.io`, default port 8883.

## Task

Apply all seven fixes to `firmware/wildfire-node-v1` on branch
`hermes/0099-wildfire-fix-set`, push it, and take it green through
`.github/workflows/platformio.yml`. No hardware is required and none should be
touched: this is a source change against a fix set whose behaviour on the real
board is already recorded above.

## Success criteria

- `pio run -d firmware/wildfire-node-v1 -e heltec_v4` builds in CI, and the run
  publishes the `wildfire-node-v1-unified-firmware` artifact
  (`firmware/wildfire-node-v1/.pio/build/heltec_v4/firmware.bin`).
- The `host-tests` job passes in the same run (`pio test -e native`, plus the
  generated-scenario-header check).
- Falsifiable, per fix: `platformio.ini` carries `board_upload.flash_size` and
  `board_upload.maximum_size` = 16777216; `build_flags` carries
  `-DARDUINO_USB_CDC_ON_BOOT=1`; the firmware drives GPIO21 before
  `Wire.begin()` and probes `0x3C`; no `sendBuffer()` can run unless the probe
  succeeded; `WIFI_AP` + `softAP()` precede `portal_setup()`; a failed STA join
  brings the AP back up; the portal escapes `& < " '`; and
  `firmware_config.h` defaults read `mqtt.nordtronics.io` / `8883` behind a
  `WiFiClientSecure`.
- `git diff` on the branch contains no credential, no SSID, and no password.

## Constraints

- One deliverable: the `firmware/wildfire-node-v1` fix set on that branch.
  Leave `main` alone, and do not touch the 0095/0096 trees or the 0094 task file.
- **No credentials in the repo, ever.** The cabin WiFi SSID/password and the
  base-01 broker password stay with Stephen and are written to NVS by the portal
  at the bench. Per 0097 they exist on no machine the worker can read, so do not
  go looking and do not substitute a value found in build caches.
- Keep the `default_16MB.csv` partition table, the LoRa receive path, and the
  native tests exactly as they are.
- Crash-loop safety is a criterion, not a nicety: this firmware must never
  bulk-write an unproven panel and must never run the portal on an
  uninitialised stack. Both failures are bench-reproduced above.
- Cost: standard tier, off-peak. State the tier and the `PEAK:` line used.
- If any item cannot be applied without a decision, leave the task in `active/`
  with `status: in_progress` and declare it in `notes:` — do not improvise
  around it.

## Proof

- `branch` + `sha`: the branch tip on origin.
- `run`: the Actions run URL for `platformio.yml`, whose head SHA equals that tip.
- `artifact`: `wildfire-node-v1-unified-firmware` from that run, with its size.
- `ntfy`: the build-green publish receipt (topic, id, timestamp) — this task's
  deliverable is a CI-built firmware image, so per `mailbox/README.md` the
  notification is part of the proof.
- `files`: the changed paths.

## Reply format

Stage `mailbox/staged/0099-wildfire-fix-set.md` per `mailbox/README.md`, with
the proof pointers above, any scope extensions, self-caught defects, the tier
and `PEAK:` line used, and anything left undone.

## Correction owed to 0097 (carry this into the staged reply)

0097's `notes:` item 5 warns that the running bench image "exists only on the
board" and that overwriting it without a flash dump is one-way. That is not
accurate and the warning should not keep anyone from flashing: both bench images
have source outside the repo, in the local bench project
`~/Documents/PlatformIO/Projects/heltec-v3-bme680/` —
`src/base.cpp:24` `#define FW_TAG "v2.0-base"` and `src/node.cpp:46`
`#define FW_TAG "v2.0-node"`, with `platformio.ini` building `-e base` / `-e node`.
The bench firmware is therefore rebuildable, and the failed `read_flash` dump is
not the only copy. Worth folding into the repo (or at least recording), since
that project is not under version control.
