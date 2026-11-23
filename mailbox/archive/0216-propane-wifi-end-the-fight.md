---
task_id: "0216"
title: Propane bench WiFi — end the fight
status: verified
iteration: 1
proof:
  device_ip: 192.168.1.130
  mac: 9c:cc:01:d0:78:80
  backend: "propane-bench.local, :6053 open, 7 HA entities live at 23:36:41Z, threshold 2.50 V"
  root_cause: "breadboard power/contact path under the ~250 mA association burst, plus a measurement artefact: every serial capture hard-reset the C3 (rst:0x15 USB_UART_CHIP_RESET on port open), so each window was a fresh boot racing association"
notes: |
  Closed at Stephen's direction 2026-11-23 ("close them"). No branch, no flash
  and no config change was needed: the board came back online as soon as it left
  the breadboard, on the same firmware and the same credential, and has stayed
  online since.
---

## Status: verified — resolved, closed at Stephen's direction

Two faults were stacked, and only one was real.

1. **The 5 V rail was genuinely overloaded.** The two sensor heaters drew ~150 mA
   on the module rail; moving them to the supplemental rail fixed the ADC
   (31 samples, MQ-6 0.949-0.99 V, Figaro 1.143-1.149 V, zero NaN, zero faults).
2. **The "WiFi refuses to associate" evidence was self-inflicted.** Every capture
   opened `/dev/ttyACM0`, which hard-resets the C3: each window began with
   `rst:0x15 (USB_UART_CHIP_RESET)` and was therefore a fresh boot racing
   association. Measured from the network side, the board joined on its own about
   two minutes after the captures stopped.

Stephen's A/B settled it: same board, same firmware, same credential — joined out
of the breadboard, refused in it. Spring-contact resistance under the association
burst corrupts the handshake without tripping the brownout detector, which is
exactly the signature (Auth Expired, no reset, calm ADC).

Recorded in the `esp32-starlink-wifi-setup` skill as rule 6: never measure
association with the serial port open.

Board at closure: `192.168.1.130`, `:6053` open, `propane-bench.local` resolving,
seven entities live, threshold 2.50 V untouched.

## Juno archive note (2026-11-23)

Closed and archived by Juno at Stephen's direction ("close them", relayed to
Hermes in session). The closure record above is Hermes's, staged at 4eab08f,
and it is the final word on this task: two stacked faults — a genuinely
overloaded 5 V rail, and WiFi-refusal evidence that was an artefact of the
measurement method (every serial capture hard-reset the C3 on port open, so
each window was a fresh boot racing association). The marginal-transmitter
theory is dead. Board live at 192.168.1.130, seven entities, threshold 2.50 V.
ntfy close ping sent.
