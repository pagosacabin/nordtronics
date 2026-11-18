---
task_id: "0211"
protocol_version: 1.3.0
status: staged
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T17:45:40+00:00"
lease_until: "2026-11-18T19:15:40+00:00"
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
  - evidence: mailbox/files/0211/capture-0211-redacted.log
    kind: serial-log
    note: "full 600 s console capture, ANSI-stripped, SSID masked; no branch and no CI run exist for this task (bench-only, no repository change)"
notes: |
  REFUSED — the task's documented "still refuses" branch, staged as a reply. Run
  2026-11-18 10:45-11:00 MST (17:45-18:00 UTC), off-peak (PEAK: OFF-PEAK 17:45
  UTC), PROTOCOL MATCH 1.3.0. One capture window only, as constrained; no reflash,
  no credential change.

  RESULT. 100 deauthentications in 600 s, all `reason='Auth Expired'`, all at the
  2.4 GHz BSSID BE:D9:D7:28:BA:46 (channel 6); 99 connection attempts, every one
  still in phase SCAN_CONNECTING; zero `Got IP`; zero API client connections. The
  board never joins, so 0209's step 3 remains unmet and 0209 stays in active/
  untouched.

  THE CREDENTIAL ON THE BOARD IS PROVEN CORRECT, HOST-SIDE. The flashed
  wifi_password (9 chars, sha256 d2462d2f32d1...) is byte-identical to this
  host's stored PSK for the sibling 5 GHz BSSID of the same network
  (nmcli -s -g 802-11-wireless-security.psk for the 5 GHz connection; equal
  strings, not merely equal under .lower() as in 0209). Stephen confirmed today
  that both bands share one passphrase, so 0209's candidate (a) — a different
  2.4 GHz passphrase — is closed. Candidate (b) is what remains: the router is
  refusing this client (MAC access control, client limit, or a blacklist left
  from this morning's retries).

  AP SIDE, READ-ONLY. A scan at 10:46 MST saw the 2.4 GHz BSSID at 100 % signal,
  WPA2/WPA3, channel 6 — nothing about the advertised parameters changed since
  0209. This host's own 5 GHz link never dropped during the window (282
  NetworkManager journal lines in the 45-min span, zero wlo1 disconnect/roam
  events), so no router reboot was observed from this side.

  DEVIATIONS. (1) Opening the port resets the board over USB Serial/JTAG
  (`rst:0x15 (USB_UART_CHIP_RESET), boot:0xd (SPI_FAST_FLASH_BOOT)`, the app
  boots normally, not download mode) — the board therefore began this window
  fresh at 10:46:16 and every refusal in the log is post-reset. (2) The boot
  banner is also the image check: ESPHome 2026.9.1, compiled 2026-11-18
  08:20:18 -0700 — byte-for-byte the build 0209 flashed, so no reflash was
  needed or done. (3) The raw log carries ANSI escapes and the SSID; the
  committed copy has escapes stripped and the SSID replaced with `<ssid>`, per
  0209's masking policy. BSSID is left in, as 0209 already published it.

  COST FIELDS ARE `unknown` because this harness exposes no per-call DeepSeek
  usage to the cron session — not estimated.

  NO REPO CHANGE beyond the evidence log and this reply: `branch: none`,
  `run: none` are honest here, not omissions. `.github/workflows/` has no
  workflow that runs for a mailbox-only change.
---

# 0211 — still refused with a confirmed-correct credential: no IP

## Status

**Refused.** The board does not join. This is the task's second branch — a staged
reply documenting the continued refusal — and 0209 stays in `mailbox/active/`
untouched, pending Stephen's router findings.

## Device IP

**none.**

## Finding

One fresh 10-minute capture (2026-11-18 10:46:16–10:56:20 MST) shows the board
attempting the 2.4 GHz BSSID 99 times and being deauthenticated **100 times, every
one with `reason='Auth Expired'`**, never leaving `SCAN_CONNECTING`, with no
`Got IP` and no API client connection. The passphrase now on the board is not a
guess: it is byte-identical (sha256 `d2462d2f32d1…`, 9 characters) to the PSK this
laptop uses to associate to the sibling 5 GHz BSSID of the same network, and
Stephen confirmed today that both bands share it — so this is a network-side
refusal of this client, not a credential defect.

## Log excerpt (full capture: `mailbox/files/0211/capture-0211-redacted.log`)

Boot — the image check, and the reset this capture itself caused:

```
ESP-ROM:esp32c3-api1-20210207
rst:0x15 (USB_UART_CHIP_RESET),boot:0xd (SPI_FAST_FLASH_BOOT)
[I][app:151]: ESPHome version 2026.9.1 compiled on 2026-11-18 08:20:18 -0700
[I][app:158]: ESP32 Chip: ESP32-C3 rev0.4, 1 core(s)
```

One refusal cycle (representative; `<ssid>` masks the network name):

```
[I][wifi:1145]: Connecting to '<ssid>' (BE:D9:D7:28:BA:46) (priority -49, attempt 1/2 in phase SCAN_CONNECTING)...
[I][monitor:161]: MQ-6=1.415 V  Figaro=1.356 V  thr=2.50 V  alarm=off
[W][wifi_esp32:869]: Disconnected ssid='<ssid>' bssid=BE:D9:D7:28:BA:46 reason='Auth Expired'
[W][wifi:1729]: Connecting to network failed (callback)
[I][wifi:1145]: Connecting to '<ssid>' (BE:D9:D7:28:BA:46) (priority -49, attempt 2/2 in phase SCAN_CONNECTING)...
[W][wifi_esp32:869]: Disconnected ssid='<ssid>' bssid=BE:D9:D7:28:BA:46 reason='Auth Expired'
[W][wifi:1729]: Connecting to network failed (callback)
[W][wifi:759]: Restarting adapter
```

Counts over the 600 s window: 100 × `reason='Auth Expired'` (no other reason code),
99 attempts (50 at 1/2, 49 at 2/2), 0 × `Got IP`, 0 client connections, 60 monitor
lines (MQ-6 1.394–1.431 V, Figaro 1.328–1.391 V, threshold 2.50 V from NVS, alarm off).

## What this settles, and what is still open

- **Settled:** the credential. 0209's candidate (a) — a different 2.4 GHz
  passphrase — is closed by Stephen's answer plus a byte-for-byte match against
  this host's working PSK. The board is also running exactly 0209's final image,
  so the refusal is not a stale firmware artifact.
- **Open:** the router's treatment of this client. The board's MAC is
  `9c:cc:01:d0:78:80`; the ask is an access-control / client-limit / block-list
  check for it, and a reboot if anything is found. Nothing else in the
  deliverable changes — one reflash is not even needed; a permitted join should
  show up in the next capture window on its own, since the board retries forever.

## Constraints honoured

No credential change, no reflash, one capture window only. The capture does reset
the board over USB Serial/JTAG (`rst:0x15`, booting app0 normally); that reset is
disclosed above rather than presented as the board restarting itself. Secrets stay
out of the repo and out of this reply.
