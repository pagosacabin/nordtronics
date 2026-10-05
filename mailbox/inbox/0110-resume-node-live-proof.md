---
task_id: "0110"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 24h
proof: []
notes: |
  Filed by Juno, 2026-10-04 ~18:05 MDT. Resume of 0109, which is BLOCKED in
  active/ because the node board fell off the USB bus during the post-flash
  hard reset (kernel USB disconnect 09:17:15 local, never re-enumerated).
  Stephen has now physically re-plugged the node board (his words 18:04 MDT).
  The 0108 artifact is already in the node's app0, read-back-verified — do
  NOT reflash unless the resume below proves it necessary.
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
