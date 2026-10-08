---
task_id: "0098"
protocol_version: 1.0.0
status: closed-killed
iteration: 1
expect-reply-within: 24h
notes: |
  CLOSURE (closed-killed), 2026-10-08: Killed per Stephen's decision 2026-10-08; WPA3 mixed-mode not implementable in this toolchain.

  PICKED UP (iteration 1) by the mailbox worker, 2026-10-03 02:50 UTC. The filed
  front-matter had no `iteration` field; it was added at pickup (protocol field).

  BLOCKED — the task produced no repo change and there is NO proof block. Three
  independent reasons, the first of which is a correction to the spec, not a
  detail of the bench:

  1. OPTION (a) AS WRITTEN IS NOT IMPLEMENTABLE IN THIS TOOLCHAIN (verified).
     - Pinned toolchain: firmware/wildfire-node-v1/platformio.ini sets
       `platform = espressif32` (unversioned) -> resolves to espressif32 6.12.0
       with framework-arduinoespressif32 3.20017.241212+sha.dcc1105b =
       arduino-esp32 2.0.17 (cores/esp32/esp_arduino_version.h: 2/0/17) on
       ESP-IDF 4.4.
     - `CONFIG_ESP32_WIFI_ENABLE_WPA3_SAE=y` is baked into the SDK config the
       framework package ships: tools/sdk/esp32s3/sdkconfig:1248 — the only
       WPA3/SAE line in that file.
     - The WiFi stack is precompiled: tools/sdk/esp32s3/lib/{libnet80211.a,
       libwpa_supplicant.a,libmbedtls.a,libmbedtls_2.a}.
     - PlatformIO's espressif32 builder exposes no sdkconfig hook:
       `grep -rn sdkconfig ~/.platformio/platforms/espressif32/builder/` -> 0
       matches (0 in builder/frameworks/arduino.py too). A
       `-DCONFIG_ESP32_WIFI_ENABLE_WPA3_SAE=0` in build_flags cannot reconfigure
       a prebuilt archive. Disabling SAE in this toolchain means rebuilding the
       SDK/framework package, which is not a build-config change.
     - A runtime override is not available either. arduino's
       WiFiSTA::begin() builds the STA config in wifi_sta_config()
       (libraries/WiFi/src/WiFiSTA.cpp:85-110) and calls esp_wifi_set_config:
       it sets `threshold.authmode = WIFI_AUTH_OPEN` and, only when a password
       is supplied, `= _minSecurity` (:96,:102), where `_minSecurity` defaults
       to `WIFI_AUTH_WPA2_PSK` (:118). setMinSecurity() documents
       threshold.authmode as the MINIMUM security "for AP to be considered
       connectable" (:418-426) — a floor, not a cap — so a transition-mode AP
       (authmode 7 = WPA2_WPA3_PSK) clears the floor and the PSK-vs-SAE
       election still happens inside the prebuilt supplicant. `sae_pwe_h2e`
       (wifi_sae_pwe_method_t) is a runtime field
       (tools/sdk/esp32s3/include/esp_wifi/include/esp_wifi_types.h:252-256,289)
       and the prebuilt supplicant does carry H2E code (strings: "SAE Hash to
       Element u1 P1", "esp_wifi_sta_get_use_h2e_internal"), but nothing in the
       Arduino/esp_wifi API caps the auth mode at WPA2 to force WPA2-PSK.
     => (a) is out; (b) (arduino-esp32 3.x / IDF 5.x) is the only reachable
        route, and it is a platform migration for BOTH roles in this tree.

  2. COMPETING CAUSE NOT DISCRIMINATED — reason 202 is also the wrong-PSK
     signature, and the spec's claim that "a wrong passphrase cannot produce
     this error" is not checkable from the worker side (the password lives only
     in NVS and is never printed). The stored credential's provenance is
     unverified: 0097 found a stale "NordNickell" PSK in ~/.cache/arduino
     artefacts dated 2025-12, of unknown currency, and deliberately did not use
     it. Cheap discriminating test, needs Stephen at the portal: type the
     known-good cabin PSK and retry the join. Still 202 => the SAE-without-H2E
     theory holds; joins => the stored PSK was the defect and (b) would be paid
     for nothing. This test is a bench action, not mine to guess.

  3. THE BENCH LEG IS NOT EXECUTABLE UNATTENDED. All three success criteria are
     bench-only (join the still-mixed-mode AP; NTP; MQTT CONNECT + PUBLISH as
     base-01). base-01's broker password is on no machine the worker can read —
     re-verified this run (read-only ssh deploy@89.117.21.105):
     /etc/nordtronics/mqtt-credentials.env (0600 root:root) holds
     MQTT_NODE01_USERNAME / MQTT_NODE01_PASSWORD only, base-01 exists in
     /etc/mosquitto/passwd as a digest and in /etc/mosquitto/acl, and its
     plaintext appears nowhere outside ACL/passwd/test fixtures. The WiFi
     password is stored in the base board's NVS but is not readable.

  LIVE BENCH STATE, read-only, this run (2026-10-03 02:3x-02:4x UTC; no write,
  no flash, no NVS/portal write):
    /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00
      -> ../../ttyACM0   (base)
    /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00
      -> ../../ttyACM1   (node)
    20 s read of ttyACM0 at 115200 (stty -F /dev/ttyACM0 115200 raw -echo;
    timeout 20 cat /dev/ttyACM0):
      hb loops=2440 heap=261412
      hb loops=2450 heap=261412
      wifi: retrying join to [NordNickell]
      wifi event: id=5 reason=202
    ttyACM1 (node) is printing sensor frames ("T 25.91 C  P 789.46 hPa
    RH 41.94 % ...", "[pm] frames=4996 badsum=22"), so the bench pair is alive.
    This reproduces the reported symptom live (SSID stored, auth-stage failure
    reason 202) and corrects 0097 item 5: the base console is readable again —
    it re-enumerated as ttyACM0 at 2026-10-02 19:14 MDT and is not wedged.

  DECISION OWED (none of it is mine to take unattended):
    a) Authorize or decline option (b) — the arduino-esp32 3.x / IDF 5.x
       platform migration. It is the only route that can fix this in the
       firmware, it changes the framework for both roles in the same tree, and
       its verification (join + the LoRa receive path intact) is bench-only.
    b) Whether to run the PSK discriminator in item 2 FIRST — it is the only
       step that separates the SAE defect from a stale stored credential, and
       it is cheap.
    c) Who runs the bench leg: the join test needs the WiFi password re-entered
       at the portal and the MQTT leg needs base-01's password; both are
       Stephen's.

  NOT DONE / NOT TOUCHED: no repo change, no branch, no build, no flash, no NVS
  or portal write, no router change, no credential written anywhere. The
  mailbox pickup commit is the only change made for this task.

  Cost context: deepseek-flash, standard tier; PEAK: OFF-PEAK 02:15 UTC from the
  pre-run digest.
---

# 0098 — Base firmware must connect to WPA2/WPA3 mixed-mode APs with zero router changes

# Context

Bench diagnosis 2026-10-02 (Hermes, verified by Juno): the base board cannot
join the cabin Starlink router. The AP advertises mixed WPA2/WPA3
(auth_mode=7). The firmware builds on Arduino core 2.x / ESP-IDF 4.4, which
has WPA3 SAE compiled in (CONFIG_ESP32_WIFI_ENABLE_WPA3_SAE=y) WITHOUT H2E
support — so the board elects SAE, and the AP rejects it at the auth stage
(reason 202 AUTH_FAIL). A wrong passphrase cannot produce this error; the
reason-code reading is confirmed.

Stephen's product call: telling customers to flip their router to WPA2-only
is not a shippable answer. The base MUST join default-config routers —
including WPA2/WPA3 mixed mode, which is the out-of-box default on Starlink
and most current ISP gateways — with no router-side changes whatsoever.

# Task

Fix it firmware-side. Two options, Hermes picks and documents why:

(a) Disable WPA3 SAE in the build config
    (CONFIG_ESP32_WIFI_ENABLE_WPA3_SAE=n). The board then uses pure
    WPA2-PSK, which mixed-mode APs accept. Smallest change; the base has no
    need for WPA3. Recommended.
(b) Upgrade the firmware to Arduino core 3.x / ESP-IDF 5.x with full WPA3
    (H2E) support. Larger change, more regression surface.

If the fix touches the repo firmware tree, do it on a branch
(hermes/0098-...) with a normal CI build; if it is purely a local bench
config, say so explicitly in the notes.

# Success criteria

- The cabin Starlink router stays in WPA2/WPA3 mixed mode — DO NOT change
  the router for this task. That is the test AP.
- The base joins it, gets an IP, sets time via NTP, and completes the MQTT
  publish leg from 0097 (broker mqtt.nordtronics.io:8883, TLS, user base-01).
- Serial shows: WiFi connected (with the AP still in mixed mode) + MQTT
  CONNECT + PUBLISH.

# Constraints

- No router changes, no credentials in the repo (WiFi/MQTT secrets live on
  Stephen's machine only).
- Do not break the working LoRa receive path.
- Worker cost: standard tier, off-peak preferred. State the tier used.

# Proof

- Staged reply with: the option chosen and why, the config diff or core
  version change, and quoted serial lines showing WiFi connect + MQTT
  CONNECT + PUBLISH against the still-mixed-mode AP.
- `reply_format`: staged file per `mailbox/README.md` with proof pointers,
  scope extensions, self-caught defects, and anything left undone.

# Reply format

Stage `mailbox/staged/0098-base-wifi-mixed-mode.md` per `mailbox/README.md`.

---

# Reply — BLOCKED, no proof block (iteration 1)

There is no `proof` block and no staged file for this task: nothing was built,
pushed or flashed, and every success criterion is a bench observation an
unattended run cannot make. The file stays in `active/` with
`status: in_progress`, per `mailbox/README.md`. Exact commands are in the
front-matter `notes:`.

1. **The recommended option (a) is not a build-config change in this toolchain.**
   `CONFIG_ESP32_WIFI_ENABLE_WPA3_SAE=y` is baked into the prebuilt framework
   package (espressif32 6.12.0 / arduino-esp32 2.0.17 / IDF 4.4), whose WiFi and
   supplicant stacks ship as static archives, and PlatformIO's espressif32
   builder has no sdkconfig hook — so no `build_flags` value can reach it.
   Arduino's `threshold.authmode` is a minimum (a floor), not a cap, and there
   is no runtime knob that forces WPA2-PSK over SAE. Option (b) — arduino-esp32
   3.x / IDF 5.x — is therefore the only reachable route, and it is a platform
   migration for both roles, which is why it is a decision, not a patch applied
   on my authority.
2. **The symptom is live-reproduced, and a second cause fits it equally well.**
   Read-only 20 s capture of the base board (ttyACM0, MAC 80:F1:B2:A7:47:EC) at
   115200:

       wifi: retrying join to [NordNickell]
       wifi event: id=5 reason=202

   Reason 202 is also exactly what a wrong or stale PSK produces at the auth
   stage, and the stored credential's provenance is unverified (0097 found a
   stale "NordNickell" PSK in build-cache artefacts dated 2025-12). Typing the
   known-good PSK at the portal and retrying is the cheap test that separates
   the two diagnoses, and it needs Stephen at the bench.
3. **The bench leg's credentials are unreachable.** Re-verified read-only this
   run: base-01's broker password exists only as a digest in
   `/etc/mosquitto/passwd` plus the ACL — its plaintext is on no machine the
   worker can read — and the WiFi password is only inside the board's NVS.

Correction carried for 0097: its item 5 warning that the base console is wedged
no longer holds. The board re-enumerated as `/dev/ttyACM0` at 2026-10-02 19:14
MDT and is printing again (heartbeat plus join retries), and the node (ttyACM1)
is printing sensor frames, so the bench pair is alive.

Nothing else was touched: no repo commit for the task, no branch, no build, no
flash, no NVS or portal write, no router change, no credential anywhere. Cost
context: deepseek-flash, standard tier; `PEAK: OFF-PEAK 02:15 UTC`.
