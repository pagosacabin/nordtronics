---
task_id: "0110"
protocol_version: 1.0.0
status: in_progress
iteration: 2
expect-reply-within: 24h
proof: []
notes: |
  Filed by Juno, 2026-10-04 ~18:05 MDT. Resume of 0109, which is BLOCKED in
  active/ because the node board fell off the USB bus during the post-flash
  hard reset (kernel USB disconnect 09:17:15 local, never re-enumerated).
  Stephen has now physically re-plugged the node board (his words 18:04 MDT).
  18:06 MDT Stephen: on re-plug the board enumerated BRIEFLY (~1 s) then
  dropped off USB again. Pattern = boot-then-die, not a dead port. Note the
  0105 image stayed up fine on this board through 0107's whole session, so
  the 0108 GPIO36/Vext drive at the top of setup() is suspect — likely the
  Vext rail coming up against the sensor wiring (power dip or a rail fault),
  killing the chip mid-boot before USB re-enumerates. If the board will not
  stay up: try a different USB cable/port first (power marginality), then
  unplug the BME680 and re-test to isolate the rail load. Second hypothesis
  (added 18:2x MDT): software crash-loop, not power — with the BME680 now
  powered, the probe FINDS it (0105 never got that far), role flips to node,
  and something in the node code path crashes -> reboot -> brief enumerate
  -> crash again. The BME680-unplug test narrows both hypotheses at once:
  if the board stays up unplugged, the fault is BME680-presence-dependent
  (rail short or driver crash); inspect wiring for shorts before any code
  change. Do NOT reflash
  until the mechanism is understood — this task is diagnosis, not repair.
  18:09 MDT Stephen: leaving the board as-is until Tuesday Oct 6 (his next
  bench day). If the board is absent when this runs, report that and stand
  down — do not keep retrying; the next physical step happens Tuesday.
  18:2x MDT Stephen: the BME680 is HARD-WIRED (soldered; wiring proven good
  Oct 2) — the unplug test is OFF the table. This also weakens the
  power-fault hypothesis: the legacy bench firmware drove the same Vext
  rail with the same sensor and ran fine. Crash-loop is now the leading
  hypothesis (new code dies once the probe actually finds the sensor).
  Tuesday's decisive small test, no wiring changes: hold BOOT, tap RESET
  to park the board in ROM download mode — if USB stays up there, the app
  is the killer and hardware/power are exonerated. Then capture serial
  during the flicker and read the panic/backtrace; it names the culprit.
  18:25 MDT Stephen: board is NOW in ROM download mode (PRG hold + RST tap,
  his words "should be done now"). CHECK IMMEDIATELY: does the by-id path
  for B0:A6:04:C5:75:4C enumerate and STAY for 60+ s? Stable in download
  mode = app is the killer, hardware/power exonerated — then try a normal
  boot and capture the serial panic. Still dropping even in download mode
  = hardware/power fault, report it and stand down.
  18:26 MDT Stephen: board "stayed up on my side" in download mode.
  Hardware/power EXONERATED — the 0108 app is the killer (crash-loop
  hypothesis confirmed as far as it can be without the panic text).
  NEXT: serial-reset the board out of download mode into the app (RTS/EN
  pulse is enough — no physical action needed) and capture the serial
  output through the boot-then-die cycle. Read the panic/backtrace — it
  names the crashing code. Report the panic text verbatim. Do NOT attempt
  a fix in this task; diagnosis only.
  The 0108 artifact is already in the node's app0, read-back-verified — do
  NOT reflash unless the resume below proves it necessary.

  PICKED UP (iteration 1 -> 2) by the mailbox worker, 2026-10-05 00:15 UTC
  (18:15 MDT). Off-peak (PEAK: OFF-PEAK 00:15 UTC). Predecessors
  0097/0098/0106/0107/0109 remain decision-blocked in active/ and were not
  touched.

  STAND DOWN (iteration 2, 2026-10-05 00:15-00:22 UTC) — NO proof block, not
  staged. Task step 1 fired: the node board is NOT enumerated, so the resume
  cannot proceed. Per the filing ("report that and stand down — do not keep
  retrying; the next physical step happens Tuesday") I ran the read-only
  checks and stopped: no reflash, no unlock, no erase, no NVS write, no serial
  port opened, no attempt to probe or recover the mechanism. What was checked:

  1. NODE ABSENT AT CHECK TIME. `ls -la /dev/serial/by-id/` at 00:15Z and
     again at 00:17:18Z lists exactly ONE device:
       usb-Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC-if00 -> ../../ttyACM0
     The mandated node path
       usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00
     does NOT exist. `lsusb` shows a single Espressif device (303a:1001, bus 3
     device 10) and `udevadm info -n /dev/ttyACM0` gives
     ID_SERIAL=Espressif_USB_JTAG_serial_debug_unit_80:F1:B2:A7:47:EC on
     DEVPATH .../usb3/3-2/... — i.e. the BASE. No ttyUSB*, no second ACM.

  2. BUT IT IS NOT DEAD — IT IS RE-ENUMERATING ON A ~12-MINUTE METRONOME, and
     has been since 11.7 min after 0109's drop (0109's disconnect 09:17:15,
     first re-attach 09:28:55). Kernel log, node port (bus 1-2,
     SerialNumber B0:A6:04:C5:75:4C), attach times (local MDT; +6 h = UTC):
       09:28:55, 09:40:58, 09:53:04, 10:05:09, 10:17:12, 10:29:17, 10:41:21,
       10:53:25, 11:05:30, 11:17:35, 11:29:40, 11:41:44, 11:53:49, 12:05:53,
       12:17:58, 12:30:02, 12:42:07, 12:54:12, 13:06:16, 13:18:20, 13:30:24,
       13:42:29, 13:54:33, 14:06:37, 14:18:42, 14:30:46, 14:42:50, 14:54:54,
       15:06:59, 15:19:03, 15:31:08, 15:43:12, 15:55:16, 16:07:21, 16:19:25,
       16:31:29, 16:43:34, 16:55:38, 17:07:44, 17:19:46, 17:31:50, 17:43:55,
       17:55:59, [18:04:30 = Stephen's re-plug, off-cycle], 18:16:30
     45 attach events, each followed 3-5 s later by "USB disconnect" with the
     same device number; interval 12 min 03 s +- 3 s (720-726 s) across all 44
     gaps, and the two off-cycle points are the manual re-plug (8 min 31 s
     after the previous attach) and the return to the metronome (12 min 00 s
     after it).
     So the board is back on the bus for 4-5 s every ~12 min and has been all
     evening; my by-id checks land in the ~99.4 % of each cycle when it is not.
     The board was NOT reset or flashed by me — these are its own cycles.

  3. WHAT THE TIMING BEARS ON (stated as timing evidence, not a proven
     mechanism — the task forbids the guess-fix and I applied none):
     - Hypothesis B (your 18:17 commit: software crash-loop, "crash -> reboot
       -> brief enumerate -> crash again") predicts a period on the order of
       the ESP32's boot time, i.e. seconds. A 12 min 04 s metronome with a
       stable 3-5 s up-window is not that shape.
     - Hypothesis A (Vext rail vs. sensor wiring — power dip or rail fault) is
       compatible: the chip dies 3-5 s into EVERY boot, consistently, and is
       absent ~12 min between attempts, which reads as an external condition
       with a slow recovery rather than a code path.
     - Neither was tested. The BME680-unplug test and the cable/port swap are
       physical and remain Stephen's Tuesday steps.

  4. NO ROLE LINE, NO FRESH READING — criteria 1 and 3 unattained. There is no
     boot log to quote: the console dies with the USB device inside the same
     3-5 s window, and I did not attempt a capture (see 5). Backend context,
     read-only, `curl -s https://api.nordtronics.io/v1/nodes` at
     2026-10-05T00:16:10Z:
       bench-01 last_seen_utc 2026-10-02T20:37:37Z, age_seconds 185913 (~51.6 h), status "stale"
       node-01  last_seen_utc 2026-09-25T19:20:01Z, age_seconds 795369, status "stale"
     No reading has reached the backend since 2026-10-02.

  5. BASE BOARD — NO WRITE, NO RESET. Success criterion 4 met. No esptool,
     unlock, erase or write command was run against 80:F1:B2:A7:47:EC, and I
     opened NO serial port at all this run (no stty, no cat, no pyserial), so
     this run cannot have perturbed it via the DTR/RTS path 0107 declared.
     Kernel corroboration: bus 3-2 last enumerated Oct 03 16:39:43 and there is
     no usb 3-2 disconnect or new-device line after it — the base has been up
     unbroken for ~1.3 days on the same device number.

  6. NO DEVIATIONS. Nothing was flashed, reset, unlocked, erased or written; no
     NVS write anywhere; no credential read, written or logged; no VPS change
     (the API was read read-only over HTTPS); no repo commit or branch for the
     task — this pickup commit and this report are the only commits. Cost: one
     off-peak run.

  WHY NOT STAGED, AND WHAT THE NEXT RUN OWES. `proof: []` is real, not an
  omission: the deliverable is a live proof and there is none — no branch, no
  CI run, no artifact. Blocked is not staged (mailbox README rule 2;
  handoff-mailbox skill rule 6). This task is PARKED, not resumable: do not
  re-probe the bus, do not re-report the metronome, do not reflash — it waits
  on Stephen's Tuesday Oct 6 bench session, exactly where the filing put it.
  ONE OPTION FOR THAT SESSION, offered because it is the single datum
  hypothesis B/A discrimination needs and it is not mine to take unattended:
  capture the node console INSIDE the 4-5 s window. It is now feasible (the
  period is known — poll /dev/serial/by-id/ and read on appearance with the
  prescribed non-perturbing recipe), but it is not free: opening the port may
  assert DTR/RTS and reset the board, which would confound the very window
  being measured. Do it with a human watching, preferably via an external
  3.3 V USB-serial tap on the Heltec's UART pins rather than the USB-JTAG.
---

# 0110 — Node board re-plugged: resume the 0109 live-data proof

## Context

0109 (`mailbox/active/0109-flash-node-0108-live-proof.md`) flashed the 0108
artifact (Vext gate fix) to the node board (B0:A6:04:C5:75:4C), verified the
write byte-for-byte, then blocked when the board dropped off USB and never
re-enumerated. Stephen has physically re-plugged it. Read 0109's notes first
for the full state — especially sections 2 (flash verified), 5 (base board
untouched — keep it that way), and 6 (drop mechanism undetermined, do not
guess-fix).

## Task

1. Verify the node board has re-enumerated: the by-id path
   `usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00` must
   exist. If it is still absent, report that plainly and stop — do not
   invent recovery.
2. Read the node console (non-perturbing: `stty -F <port> 115200 raw -echo
   -hupcl` + `cat`) and report the ROLE line — expect
   `ROLE: node (source=PROBE, sensors=present, ...)`. Quote it, plus the
   BME680 probe line.
3. If the role is node, continue 0109's live-proof: base console `rx:` lines
   (non-perturbing capture only — 0109's recipe, never reset the base),
   broker journal publish lines, then
   `curl -s https://api.nordtronics.io/v1/nodes` — quote the fresh row
   (`last_seen_utc` < 15 min, not `stale`) and name the node ID.
4. If the role is NOT node, or the board drops off USB again on reset,
   stop and report exactly what happened — do not reflash, do not write
   NVS, do not guess.

## Success criteria

- Quoted node ROLE line from the re-plugged board.
- If role=node: quoted `GET /v1/nodes` row with a fresh `last_seen_utc`,
  or an honest report of which hop is still dark.
- The base board received no write and no reset. No NVS writes anywhere.

## Constraints

- No reflash of either board unless the resume proves the image is bad —
  and that needs a new decision, not this task.
- No repo commits, no new branches. One deliverable: the resume report.

## Proof

- `ls /dev/serial/by-id/` showing (or not showing) the node.
- Quoted ROLE line and, if live, the quoted API row.

## Reply format

Follow the mailbox staged-reply format: status line, the falsifiable
checks with quoted evidence, deviations declared, cost line.
