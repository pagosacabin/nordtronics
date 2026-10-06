---
task_id: "0115"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 6h
---

# 0115 — PMS UART diagnostic log build (node only)

## Context

Bench 2026-10-06: the PMS5003 is powered from the bench supply (fan spinning,
30–40 mA, same draw as the last working session), the node (0112 image) checks
in every 60 s with the BME680 healthy, but pm25 is pegged 0.0 and the node
serial shows no `pms: checksum mismatch` lines — i.e. zero bytes are arriving
at UART1, not a parser rejection. No meter is available, so this rung is
software-only: add temporary observability to the PMS read path. (Side note
already established: `probe: pms5003` never prints on a node — the BME680 ACK
short-circuits `probe_node_sensors()` — so do not look for it; the checkin path
is the instrument.)

## Task

On a new branch from `hermes/0112-gate-node-deep-sleep` (name it
`hermes/0115-pms-uart-diag`), make exactly ONE functional change in
`firmware/wildfire-node-v1/src/main.cpp`, in `node_checkin()`: capture
`g_pms.available()` BEFORE the `read_pms25()` call, then emit one log line per
checkin:

```c
const int pms_avail_before = g_pms.available();
const float pm25 = read_pms25(&pm1, &pm10, &pms_ok);
logf("pms: rx_avail_before=%d ok=%d pm1=%u pm25=%.1f pm10=%u",
     pms_avail_before, pms_ok ? 1 : 0, pm1, pm25, pm10);
```

(Build it, CI green, then flash the NODE ONLY and capture serial for at least
two checkins.)

## Success criteria

1. Branch `hermes/0115-pms-uart-diag` builds green in CI from the 0112 tree with
   only the diagnostic line added.
2. The NODE (MAC `B0:A6:04:C5:75:4C`, by-id path
   `usb-Espressif_USB_JTAG_serial_debug_unit_B0:A6:04:C5:75:4C-if00`) — and only
   the node — is flashed; bench discipline holds (partition-table read first to
   prove NVS safety, sha256 on the read-back matches the built image).
3. Serial capture shows at least two `pms:` lines, and the reply states the
   verdict plainly: `rx_avail_before` consistently 0 means no bytes reach the
   pin (wire/sensor side); consistently >0 means bytes arrive and the read path
   is dropping them (firmware side).

## Constraints

- NODE ONLY. Do not touch the base (MAC `80:F1:B2:A7:47:EC`).
- Diagnostic only: no other behavior changes, no config changes, no deep-sleep
  changes. This is not a release — do not merge, do not tag.
- Keep model cost on DeepSeek Flash (the tier already in use for this line of
  work). State the tier in the reply.
- Do not "fix" anything based on the verdict. Report the verdict and stop —
  the fix is a separate task.

## Proof

- Branch SHA on origin + CI run URL + artifact.
- Flash read-back sha256 matching the built image.
- Pasted serial lines showing at least two `pms:` lines (redact nothing needed;
  there are no secrets in these lines).

## Reply format

Stage the reply to `mailbox/staged/` per the mailbox protocol: front-matter
with `status:`, the verdict (bytes arriving: yes/no) in the first line of the
notes, and a `proof` block with the pointers and pasted serial lines above.
