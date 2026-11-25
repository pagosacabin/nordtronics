---
task_id: "0253"
status: verified
iteration: 1
proof:
  kind: host-only
  port: "/dev/ttyUSB0 — 1a86:7523 CH340 (driver ch341), the only serial port present"
  ltchiptool: "v4.15.0, installed with uv tool install ltchiptool"
  handshake: "no-answer — TimeoutError: Timed out attempting to link with chip (90 s wall, 20 s chip timeout)"
  port_state: "opens cleanly, 0 bytes received in 4 s at 115200 — link is alive, chip is silent"
  chip_writes: "none — no dump taken, no write attempted, chip untouched"
notes: |
  NO-ANSWER, STOPPED AT STEP 3 EXACTLY AS THE TASK INSTRUCTS — Hermes (laptop),
  2026-11-25 09:36-09:41 MST. Nothing below step 3 was attempted: no dump, no
  flash, no write of any kind. The chip is untouched.

  Deviation from the filing, declared: the task names a Freenove CP2102 serial
  port. The only port on the laptop is /dev/ttyUSB0 = 1a86:7523 CH340, driver
  ch341 (from udev and lsusb, not assumed). ltchiptool used it without
  complaint; the identity came from the device, not the filing.

  Workers: all four mailbox jobs were paused at pickup and are RESUMED on this
  staging (c0be50a686c6, 7aff6948c2c1, 8b1c9e1323c5, 5c1532977f15), verified
  with cron list.
expect-reply-within: 6h
---

# 0253 — NAS-CW01: dump stock firmware, flash OpenBeken (BK7231N)

## Assignment

- **Hermes — laptop side (this task):** install ltchiptool, find the serial port, verify the chip answers, dump the full stock flash and verify it, then flash OpenBeken and verify the write. Every step below is yours.
- **Stephen — bench side (already done):** wiring, bench supply, CEN download-mode dance, all pre-flight checks passed (see Context). If the chip is no longer in download mode when you pick this up, stage a reply saying so and Stephen re-stages the board — do not improvise bench steps.
- **Newton:** not involved. Bench firmware stays with Hermes.

## Context

- Device: NEO Coolcam NAS-CW01 4-in-1 sensor (temp, humidity, lux, clock/alarm), WiFi variant. Board silkscreen **NAS-CW01W6 -V3 2021.11.30**. WiFi module is a **BK7231N** (U1). Helper MCU (U2, silkscreen "MCU_TCD") drives the LCD and sensors over UART — classic TuyaMCU split. **The BK7231N is 3.3 V only — 5 V kills it.**
- Board is fully apart: LCD disconnected, batteries out. Board does NOT need to come out of the case for flashing.
- P1 through-hole row, measured by Stephen with a meter (holes do not physically line up with the pads, but map in order): hole 1 = GND, 2 = WIFI_3V3, 3 = TDO, 4 = TDI, 5 = TMS, 6 = TCK, 7 = TXD, 8 = RXD, 9 = CEN, 10 = CSN. MCU_TCK is pad-only, not in the hole row.
- Serial adapter is a **Freenove ESP32-WROVER CAM board** standing in as a USB-serial adapter: its EN pin is jumpered to GND (ESP32 held in reset, camera asleep), TX0 → sensor RXD (hole 8), RX0 → sensor TXD (hole 7), GND common with the bench supply. Freenove USB goes to the laptop.
- Power is a **bench supply at 3.3 V** into WIFI_3V3 (hole 2). The adapter's 3V3 pin is NOT used.
- Bench state as of filing (2026-11-25 ~09:35 MST): board wired, powered, put in download mode (CEN held to GND at power-on, then released). Stephen's pre-flight passed: 3.3 V on the rail, Freenove port enumerates, adapter loopback echoes, serial terminal at 115200 stays silent at power-on (download mode, not a normal boot), TXD idles at ~3.3 V.

## Task

Flash OpenBeken onto the NAS-CW01's BK7231N, with the stock firmware dumped and verified first as unbrick insurance. One deliverable: the board running OpenBeken with a verified stock dump preserved.

1. Install ltchiptool if not present (`pip install ltchiptool` or equivalent on your machine) and read `ltchiptool --help` for the documented commands.
2. Identify the Freenove CP2102 serial port (`ls /dev/ttyUSB*`, `dmesg`). Do not assume a port name.
3. Connect and confirm the BK7231N answers the bootloader handshake. **If it does not answer, stop here and stage a reply saying so** — the board likely dropped out of download mode and Stephen will re-stage it. Do not guess, do not retry blindly.
4. **Full stock flash dump FIRST, before any write.** Read the entire flash (2 MB) and save it on the laptop at `~/nas-cw01-stock-dump.bin` (keep this file — it is the unbrick backup). Verify the dump: report its size and SHA256, and confirm it is real firmware (not all 0xFF / all 0x00).
5. Fetch the latest OpenBeken release binary for the BK7231N (OpenBK7231N QIO **.bin** — the serial-flash binary, NOT the .ug.bin OTA variant) from the OpenBeken GitHub releases. Record the release version, download URL, and binary SHA.
6. Flash the OpenBeken binary with ltchiptool, then read back and verify the write.
7. Stage the reply (see Reply format) including the power-cycle instruction for Stephen. Boot verification happens after he power-cycles — it is a follow-up, not part of this task.

## Success criteria

- ltchiptool connects to the BK7231N bootloader on the first attempt (or the no-answer stop above is followed exactly).
- Stock dump saved at `~/nas-cw01-stock-dump.bin`, size 2,097,152 bytes, SHA256 reported, content verified non-trivial.
- OpenBeken BK7231N QIO binary flashed, read-back verify passes.
- Staged reply carries the dump hash, the OpenBeken version/URL/binary SHA, and the power-cycle instruction.

## Constraints

- Power the sensor board at 3.3 V only, from the bench supply. Never apply 5 V to any board pin.
- Dump the stock firmware and verify it before any write to the chip. No write happens before the dump is verified.
- Flash ONLY the sensor board's BK7231N. Do not flash the Freenove board — it is the adapter, not the target.
- Do not touch the P3 header (3V3/TCD/TCK/GND) — that is the helper MCU's debug port and is off limits.
- If ltchiptool cannot connect, stop and report. Do not work around a dead link by guessing at ports or baud rates.
- No cost: ltchiptool and OpenBeken are free. No model or provider choice is involved in this task.

## Proof

- Stock dump SHA256 + size + laptop path (`~/nas-cw01-stock-dump.bin`), quoted in the staged reply.
- OpenBeken release version + download URL + binary SHA, quoted in the staged reply.
- ltchiptool log lines for the handshake, the dump, the flash, and the read-back verify (relevant lines pasted in the reply — pasted output is a claim, so the hashes above are the verification anchors).

## Reply format

Stage the reply in this file with these fields filled in:

- `port`: serial port used (e.g. /dev/ttyUSB0)
- `ltchiptool_version`: installed version
- `handshake`: ok / no-answer (if no-answer, stop here — everything below is n/a)
- `stock_dump_path`: ~/nas-cw01-stock-dump.bin
- `stock_dump_size_bytes`: number
- `stock_dump_sha256`: hex
- `stock_dump_verified`: yes / no + one line on how
- `openbeken_version`: release version flashed
- `openbeken_url`: download URL
- `openbeken_binary_sha256`: hex
- `flash_verify`: pass / fail + one line
- `power_cycle_instruction`: exact instruction for Stephen (bench supply off/on) so Juno can hand him the next step
- `notes`: anything unexpected (port name surprise, ltchiptool quirk, retry needed)

---

# Reply — 0253: handshake no-answer, nothing written

**Status: blocked at step 3, staged per the task's own instruction. The BK7231N did not answer the
bootloader, so no dump was taken and no write was made. The chip is untouched.**

Everything below the handshake field is n/a by the task's reply format; the fields are filled in
that shape rather than omitted, so the reply parses the same way a successful one would.

| Field | Value |
|---|---|
| `port` | `/dev/ttyUSB0` — `1a86:7523` CH340, driver `ch341` |
| `ltchiptool_version` | `v4.15.0` |
| `handshake` | **no-answer** |
| `stock_dump_path` | n/a — nothing read |
| `stock_dump_size_bytes` | n/a |
| `stock_dump_sha256` | n/a |
| `stock_dump_verified` | n/a |
| `openbeken_version` | n/a — not fetched, not flashed |
| `openbeken_url` | n/a |
| `openbeken_binary_sha256` | n/a |
| `flash_verify` | n/a |
| `power_cycle_instruction` | n/a — no write occurred, so there is nothing to boot. Stephen's next action is the re-stage below, not a power cycle |

## The failure, verbatim

```
I: Connecting to 'Beken 7231N' on /dev/ttyUSB0 @ 115200
E: TimeoutError: Timed out attempting to link with chip
E: |-- File ".../bk7231tools/serial/linking.py", line 23, in connect
```

## What was checked before stopping

1. **Port identity** — from udev and `lsusb`: `1a86:7523 QinHeng CH340`, driver `ch341`,
   `/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0`. This contradicts the filing's
   "Freenove CP2102"; the filing is wrong about the bridge chip, and it is declared here rather
   than smoothed over.
2. **The port is free and the link is alive** — opened at 115200 with only one process, then 4 s
   of read: **0 bytes**, no exception. So the adapter enumerates and the tty accepts data; what is
   missing is a chip that answers.
3. **No baud-rate guessing and no retry loop** — one link attempt, the documented default. The task
   forbids working around a dead link by guessing at ports or rates, so that is where it stopped.

## Why it is out of download mode, and the exact re-stage needed

This is the mechanism ltchiptool itself prints, and it is more specific than the filing's
pre-flight note: *"To enter download mode, the chip has to be rebooted while the flashing program
is trying to establish communication. In order to do that, you need to bridge CEN pin to GND with
a wire."*

The filing's pre-flight says CEN was held to GND **at power-on, then released**. That satisfies
the *boot* condition but not the *link* condition: once CEN is released the stock application runs,
and the bootloader is no longer listening — which is exactly what a silent port and a link timeout
look like.

So the re-stage is not "power cycle the board". It is:

1. Keep the 3.3 V bench supply on WIFI_3V3 (hole 2). **Never 5 V.**
2. Have a wire or jumper ready on **CEN (hole 9) to GND**.
3. Say the word — I start `ltchiptool flash info bk7231n -d /dev/ttyUSB0` in one pane and tell you
   "bridge now" the moment the link window opens; CEN goes to GND then, and comes off ~1 s later.
   The chip must see its reset inside that window, not before it.

If that still does not answer, the next suspects are the two data wires (adapter TX0 → hole 8, RX0
→ hole 7) and the common ground — all bench-side, and I will not churn them from here.

## What is ready for the retry

`ltchiptool v4.15.0` is installed and working (`uv tool install ltchiptool`); it finds the port and
opens the link. The moment the chip answers, the remaining steps are mechanical and already
specified: full 2 MB dump to `~/nas-cw01-stock-dump.bin`, size + SHA256 + non-trivial-content check,
then the OpenBeken QIO serial binary and a read-back verify.



---

# Verification — Juno, 2026-11-25 ~09:45 MST

Verified and archived. What I checked: the staged file on origin/main (file present in
mailbox/staged/, active/ clean of 0253); the reply table is complete with everything below
the handshake honestly marked n/a — no fabricated hash, URL, or version anywhere; the stop
rule in the task (step 3, stop and report on no-answer) was followed exactly; the verbatim
TimeoutError is quoted; the CH340 deviation is declared, not smoothed over. Stephen's
screenshot of Hermes's session output corroborates the same reply text.

Two filing errors, mine, corrected here for the record:
- The task names the adapter as a Freenove CP2102. The laptop's only serial port is
  /dev/ttyUSB0 = 1a86:7523 CH340 (driver ch341). Stephen's Freenove board carries a CH340,
  not a CP2102. Future filings must say CH340.
- The filing's CEN procedure (hold at power-on, release, chip sits in download mode) is
  wrong for the BK7231N: the bootloader listens only briefly at boot, then the stock app
  runs. The silence pre-flight could not prove download mode. Correct re-stage: start the
  ltchiptool link attempt first, bridge CEN to GND inside the link window, release ~1 s
  later.

Outcome: handshake no-answer, no dump taken, no write made, chip untouched. Next step is a
re-stage (live interactive finish offered by Hermes in-session, or a new mailbox task).
Nothing is bricked and nothing needs unwinding.


---

# Addendum — live-session outcome, 2026-11-25 ~10:42 MST (Juno)

The "handshake no-answer" verdict above was accurate when staged at 09:41 and wrong within
the hour. In a live session at the bench, Hermes got the chip to link after three measured
fixes. This addendum supersedes the no-answer verdict; the stop-rule outcome is preserved
above as history.

What the rig proved (Hermes, live session):
- The chip IS a BK7231N and it links: chip-info TLV blocks intact across three runs.
- Read path works: a 64 KiB slice read clean (sha256 ace03d80...), 256 distinct byte values
  (real firmware); a 32 KiB slice starting where the previous run died also read clean
  (rules out a bad flash region).
- No full dump exists: best full-dump attempt 12,288 of 2,097,152 bytes — NOT a backup.
  No write was ever attempted; the chip is untouched. Power-cycle the 3.3 V rail and it is
  back on stock firmware.

Three filing errors, all measured at the bench (all mine):
1. TX/RX backwards for this adapter. The filing said Freenove TX0 -> hole 8, RX0 -> hole 7.
   Measured correct: TX0 -> hole 7, RX0 -> hole 8. The Freenove header labels are
   ESP-centric: the TX0 pin is the CH340's RX input and the RX0 pin is the CH340's TX
   output. Loopback across 7/8 proved both wires fine — it was direction.
2. "Common ground" wasn't. 50 ohms measured between the Freenove's GND and the supply
   negative. One dedicated jumper from Freenove GND to hole 1 brought it under 1 ohm and
   is what made reads work.
3. Transfer ceiling (time-based, not address-based): ~28 KB at 115200, ~200 KB at 921600,
   fails at block 0 at 2 Mbaud.
   (Plus the two errors already noted in the verification above: CP2102 vs CH340, and the
   CEN hold-at-power-on procedure vs bridge-during-link.)

Knowledge preserved outside the mailbox (host-only, on the laptop): ~/crew-vault/bench/
nas-cw01-bk7231n.md (wiring, ground fault, transfer ceiling, hashes, "no write until a
full dump exists" rule) and log/2026-11-25.md; committed at 8587fbd, tree clean. Hermes
also created and corrected the bk7231n-programming skill (flag order for -s; 0x32000 is
200 KB, not 2 MiB).

Next move (Stephen's call, per Hermes): (a) a dedicated CP2102/FT232 adapter — most
likely fix for the transfer ceiling; (b) supervised slice-wise dump, 32 KiB slices with
Stephen tapping CEN between them (tens of resets, every block hash-checked); (c) park the
board and leave OpenBeken for another day. Re-filing the result as a new mailbox task
needs Stephen's word.
