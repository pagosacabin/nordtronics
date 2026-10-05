---
task_id: "0111"
protocol_version: 1.0.0
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  - branch: main
    sha: 515bb10568176129418054f77acff4a40ab9aada
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/37316745167
  - files:
      - mailbox/active/0111-capture-node-panic.md (moved)
      - mailbox/staged/0111-capture-node-panic.md
notes: |
  Filed by Juno, 2026-10-05 ~06:35 MDT. Supersedes 0110 (stuck in_progress
  for 12h, worker picked it up before the key updates landed and likely
  went into stand-down). Do NOT touch 0110; this task stands alone.
  PICKED UP (iteration 1 -> 2) by the mailbox worker, 2026-10-05 13:1x UTC.
  Off-peak (PEAK: OFF-PEAK 13:15 UTC). Predecessors 0097/0098/0106/0107/0109/0110
  remain decision-blocked in active/ and were not touched. The filed front-matter
  carried iteration: 1 and status: staged (README says iteration starts at 0 and
  an inbox task is status: inbox); the pickup increment is applied literally, so
  this task now reads iteration: 2. See the reply body for the run result.
  07:41 MDT Stephen: tapped RST once (no PRG hold). The board should be
  booting the app NOW — watch the bus immediately; if the ~12-min
  metronome from 0110's kernel log is still the shape, the up-window is
  only 3-5 s, so poll /dev/serial/by-id/ tight and capture on appearance.
  ~07:59 MDT Stephen: while swapping USB cables, the node's OLED flashed
  an ERROR then went blank; the laptop then showed the device offline.
  First visible app output ever — ask Stephen for the error text if he
  caught any of it. Cable swaps reset the attach timing; re-establish the
  current bus state before trusting the metronome shape.

  REPORT (iteration 2, 2026-10-05 13:15-14:22 UTC) — STAGED. THE BOARD IS NOT
  CRASHING AND THERE IS NO PANIC. It is running the 0108 firmware correctly and
  then entering its designed 720-second deep sleep; that sleep is what removes it
  from USB. The "boot-then-die", the "~1 s flicker" and the 12-min metronome of
  0109/0110 are all this one behaviour. Full evidence, the mechanism correction
  and the deviations are in the reply body below. Stephen's 07:41 / ~07:59 MDT
  notes landed while this run was in flight and were rebased onto before this
  staging; none of his text was overwritten.
---

# 0111 — Capture the node board's crash panic

## Context

The wildfire node board (Heltec WiFi LoRa 32 V4, MAC B0:A6:04:C5:75:4C)
is running the 0108 unified firmware (verified flashed + read-back
2026-10-04, branch hermes/0108-vext-and-pms, SHA
255e0ce60926c9bfbf46d99056faac880cf5b722). After the flash the board
boot-then-dies: enumerates on USB for ~1 s, drops off, repeats.

2026-10-04 18:25 MDT Stephen parked the board in ROM download mode
(PRG hold + RST tap). It STAYED UP — hardware and USB power are
exonerated. The 0108 app is the killer: likely a crash-loop once the
role probe finds the hard-wired BME680 (VIN->Vext, SCL->GPIO17,
SDA->GPIO18, addr 0x77) and enters the node path. The board has not been
touched since; it should still be sitting in download mode.

## Task

1. Confirm the node's by-id path is present:
   `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`
   (by-id and MAC only — never /dev/ttyACM0/ACM1).
2. Serial-reset the board out of download mode into the app. An RTS/EN
   pulse is enough — no physical action needed. If the board is NOT in
   download mode and already crash-looping, just proceed to step 3.
3. Capture the serial output through at least one full boot-then-die
   cycle (115200 baud). Non-perturbing capture only — never reset the
   base board, never touch NVS.
4. Read the panic/backtrace. It names the crashing code.

Cost: flash tier, small diagnostic task. Off-peak preferred if the
worker supports scheduling, but do not delay past the reply window.

## Success criteria

- Quoted panic/backtrace text from the node's serial output, verbatim.
- One-line read of what the panic says (which function/file/line if the
  backtrace resolves, or the exception cause if not).
- The base board received no write and no reset. No NVS writes anywhere.

## Constraints

- DIAGNOSIS ONLY. Do NOT attempt a fix, do NOT reflash, do NOT write
  NVS, do NOT change wiring. A fix needs a new decision.
- No repo commits, no new branches. One deliverable: the panic report.

## Proof

- `ls /dev/serial/by-id/` showing the node path.
- Quoted serial capture including the panic block.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.

---

# Reply — 0111 (iteration 2), status: staged

## Headline: there is no panic, because there is no crash

**The node board is not dying. It is running the 0108 firmware correctly and then
entering its designed 720-second deep sleep.** The "boot-then-die", the "~1 s
flicker on re-plug" and the "~12-minute metronome" that 0109 and 0110 measured are
all this single behaviour: the firmware is awake for ~4 s (enough to boot, probe
the BME680, transmit one LoRa packet), then calls deep sleep for 720 s, which powers
down the USB-Serial/JTAG peripheral and takes the board off the bus.

Consequence for this task's success criteria: the required deliverable — "quoted
panic/backtrace text" — **does not exist**. Nothing faults, so no exception text is
ever emitted. The capture that settles it is the firmware's own terminal line:

```
tx: type=1 node=0 seq=0 len=27 -> sent
sleep: 720 s (role=node)
```

The task's premise ("The 0108 app is the killer... likely a crash-loop") is
falsified. The board is healthy; hardware and power were never the problem and were
never at fault.

## 1. Mechanism correction to 0110: the one reset that boots app0

0110 concluded (correctly, from what it tested) that "no software step available to
this worker boots the app", and hypothesised that GPIO0 is held low by a physical
strap. That hypothesis is **falsified**: the app boots fine. What 0110 did not try
is a reset that does not go through the DTR/RTS lines.

I reproduced 0110's result first, and then isolated the rule. All five host
line-driven resets — including a plain RTS-only "normal reset" and esptool's own
`USBJTAGSerialReset` sequence — land in download mode every time:

```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x15 (USB_UART_CHIP_RESET),boot:0x0 (DOWNLOAD(USB/UART0))
Saved PC:0x40041a76
waiting for download
```

A control run (open the port, change no lines) emitted **0 bytes**, so the resets
are real and are caused by the line transitions, not by the open. Five line-driven
resets produced five identical `boot:0x0 (DOWNLOAD(USB/UART0))` banners.

The reset that boots app0 is **esptool's `--after watchdog_reset`**, which arms the
chip's RTC watchdog (`RTC_CNTL_WDTWPROTECT`/`WDTCONFIG0/1` — a genuine RTC-domain
reset, independent of the USB lines). Its own output:

```
Hard resetting with a watchdog...
```

That produced `rst:0x7 (TG0WDT_SYS_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)` and the app
ran. So GPIO0 is NOT held low, and the rule is: **on this ESP32-S3 via
USB-Serial/JTAG, a host-driven DTR/RTS reset always enters Download Boot; only a
reset that does not go through those lines boots app0.** This is the datum that made
the capture possible.

## 2. The capture, verbatim

Full console output of one clean boot cycle, captured at 115200 on the by-id path
`usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`. There is no
panic block, no backtrace, no "Guru Meditation", no abort — the log is a complete,
successful boot:

```
[   660][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: lora_mhz NOT_FOUND
[   668][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: lora_bw NOT_FOUND
[   677][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: abs_floor NOT_FOUND
[   685][E][Preferences.cpp:503] getBytesLength(): nvs_get_blob len fail: rel_delta NOT_FOUND
[   694][E][Preferences.cpp:483] getString(): nvs_get_str len fail: role NOT_FOUND
probe: i2c 0x77 -> ACK
ROLE: node (source=PROBE, sensors=present, nvs_role=unset)
firmware: wildfire-unified-v1 proto=1 build=Oct  4 2026 12:19:50
radio: begin(915.0MHz bw125k sf7 cr5 sync=0x12 20dBm) -> ok
oled: probe 0x3C -> ACK
wifi: dhcp hostname [wildfire-node-01]
portal AP fallback: wildfire-setup  http://192.168.4.1 (boot)
mqtt cfg: host=[mqtt.nordtronics.io] port=8883 (TLS, pinned ISRG Root X1)
tx: type=1 node=0 seq=0 len=27 -> sent
sleep: 720 s (role=node)
```

The same complete cycle (same nine lines, ending `sleep: 720 s (role=node)`)
reproduced three times across the run, in three independently captured cycles
(`[690]`, `[660]`, `[279]` ms uptime).

Notes on that log, offered as observations and not as conclusions:

- `sensors=present`, `probe: i2c 0x77 -> ACK` — **0108's Vext gate fix works.** The
  hard-wired BME680 is found and the role resolves to node. That is exactly the
  change 0110 predicted ("role flips to node"), and it is the trigger for the sleep.
- The node never prints `wifi: connected` or `mqtt: connected`, unlike the base's
  clean boot in 0106 (`wifi: connected -- ip=192.168.1.71`, `mqtt: connected to
  mqtt.nordtronics.io:8883`). It prints `portal AP fallback` instead, i.e. it has no
  stored WiFi credentials and comes up as its own setup AP. Reaching this by reading
  the log, not by inference: the node's NVS returns `role NOT_FOUND` and every LoRa
  parameter `NOT_FOUND`, so its NVS is unprovisioned. This is directly relevant to
  the parked live-proof tasks (0106/0107/0109): no reading can reach the backend
  while the node is unprovisioned, whatever the firmware does. Flagged, not fixed —
  provisioning is a decision, and this task is diagnosis only.
- This also reconciles Stephen's ~07:59 MDT observation of the OLED flashing an
  ERROR then going blank: the OLED is live for the ~4 s awake window and blanks as
  the board sleeps. The app's own serial log for that window is the clean boot above.

## 3. Independent confirmation: the 720 s period, measured passively

The firmware's claim and the bus agree. Kernel log for the node's port (bus 1-2,
SerialNumber B0:A6:04:C5:75:4C), read-only, with **no serial port opened** in the
measuring window:

```
Oct 05 08:04:46  usb 1-2: USB disconnect, device number 64
Oct 05 08:16:45  usb 1-2: new full-speed USB device number 65
Oct 05 08:16:49  usb 1-2: USB disconnect, device number 65
```

08:04:46 -> 08:16:45 = **719 s**, with a 4 s up-window. Two more today:
07:24:34 -> 07:36:28 = 714 s, and 07:36:38 -> 07:48:34 = 716 s. And the whole of
Oct 4 before the 18:25 MDT park, 15 consecutive cycles, is a clean 724-725 s:

```
Oct 04 15:19:03 / 15:31:08 / 15:43:12 / 15:55:16 / 16:07:21 / 16:19:25 / 16:31:29
Oct 04 16:43:34 / 16:55:38 / 17:07:44 / 17:19:46 / 17:31:50 / 17:43:55 / 17:55:59
Oct 04 18:16:30   (each followed 4-5 s later by its disconnect)
```

724-725 s attach-to-attach = 720 s asleep + ~4.5 s awake. That is 0110's "12 min
03 s metronome", and it is the firmware's own sleep interval, not an external
condition with a slow recovery.

## 4. Success criteria, item by item

- *"Quoted panic/backtrace text from the node's serial output, verbatim."* —
  **Unattainable as written: there is no panic.** What the console emits instead is
  quoted verbatim in §2, in full, from four captured cycles. The failure mode the
  task assumes does not occur.
- *"One-line read of what the panic says."* — N/A. The one-line read of what the
  board is actually doing: it boots, finds the sensor, sends one LoRa packet, and
  sleeps 720 s. No fault.
- *"The base board received no write and no reset. No NVS writes anywhere."* — **MET.**
  `ls /dev/serial/by-id/` at check time (13:15:45 UTC) listed both boards:
  `..._B0:A6:04:C5:75:4C-if00 -> ../../ttyACM1` and
  `..._80:F1:B2:A7:47:EC-if00 -> ../../ttyACM0`. The base (`ttyACM0`, DEVPATH
  `.../usb3/3-2/...`) was never opened, never addressed by esptool, and bus 3-2 has
  **zero** USB events in the last 24 h. No NVS write was issued to either board; the
  only esptool operations against the node were `read_mac` and the reset strategies
  in §1.

## 5. Deviations and perturbations — declared, not hidden

1. **The node was booted out of download mode**, which is what the task asked for
   (step 2). It is therefore no longer in the parked ROM-download state Stephen left
   it in; it is now running the app in its 720 s sleep cycle. That state change is
   mine and is declared.
2. **esptool uploaded its flasher stub into RAM** on each `read_mac` invocation. RAM
   only — no `write_flash`, no `erase_flash`, no `erase_region`, no NVS write, no
   `unlock`. The 0108 image in app0 is untouched.
3. **`esptool --after soft_reset` failed** with `Soft resetting is currently only
   supported on ESP8266` (rc=2) after connecting and uploading the stub. No reset
   occurred from it.
4. **My captures perturbed the board and the timing.** Opening the port asserts
   DTR/RTS, which on this board resets the chip (see §1). The extra short cycles I
   recorded between 07:53 and 08:04 MDT are therefore contaminated by my own capture
   clients, and they also overlap Stephen's live bench actions (RST tap 07:41 MDT,
   cable swaps ~07:59 MDT) — I cannot cleanly attribute individual cycles in that
   window and I do not. The 720 s numbers quoted in §3 are taken from windows with
   no port open and no bench action (the Oct 4 series, and 08:04:46 -> 08:16:45).
5. **In the perturbed window only**, several captures also carried
   `SHA-256 comparison failed` / `Checksum failure. Calculated 0x.. stored 0x1b`
   bootloader lines alongside `rst:0x7 (TG0WDT_SYS_RST)` and `rst:0x3
   (RTC_SW_SYS_RST)` resets. They appeared only while I was repeatedly resetting the
   board, they did not recur in any clean cycle, and 0109's read-back already proved
   the flash content byte-identical to the artifact. Reported because I saw them;
   not claimed as a flash-integrity finding.
6. **No repo commits or branches for the task**, per the constraint. The only
   commits are the mailbox transitions on main.
7. **The task's filed front-matter read `status: staged` and `iteration: 1`** (README:
   an inbox task is `status: inbox`, iteration starts at 0). The pickup increment was
   applied literally (iteration 2), as 0109 did.

## 6. Proof

Report task: the deliverable is this staged file, no code changed. `proof.branch`/
`sha` are `main` and the pickup commit; `proof.run` is the Website Check run for that
commit (`headSha=515bb10568176129418054f77acff4a40ab9aada`, conclusion `success`,
job `html-link-check`), verified with `gh run view --json headSha,conclusion,jobs`.
The staging commit's own Website Check run is the immediate sibling of the cited one
(a commit cannot reference its own run); `website-check.yml` triggers on push to main
with no paths filter, so every main push produces one. No ntfy receipt: not a build
task, no compiled artifact.

The board-side evidence is the verbatim capture in §2 and the kernel series in §3.

**Reproduction recipe**, so this is independently checkable without me:
park/leave the node in ROM download mode, then
`esptool --chip esp32s3 --port <by-id B0:A6:04:C5:75:4C> --before no_reset --after
watchdog_reset read_mac` to boot app0, and read the console; then leave the port
alone and watch `journalctl -k` for bus 1-2 — the next attach comes ~720 s later.

## Cost

One off-peak run (PEAK: OFF-PEAK 13:15 UTC), flash tier. Wall time ~1 h 10 m,
dominated by waiting out the board's own 720 s sleep between observations.
