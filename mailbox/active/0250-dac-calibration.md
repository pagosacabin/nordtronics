

### Rung 3 verified 2026-11-23 ~17:2x MST

Camera flashed with the measured table (clean build, `/dev/ttyUSB1`). One shot
through the live link: `sent : 1280 pkts, 4162 ms, ok 1282 fail 0`, `peer ack`
hash equal, frame stored, library 38 -> 39. So the new quant tables run the whole
pipeline end to end; nothing else changed.

Rung 3 is complete: `s_dac_v` carries the measured rows and the panel is now fed
frames quantised against the real ladder rather than the ideal thirds.
Remaining for the reply: the two-pass repeatability reading Stephen did not take
(one reading per cell), and a visual before/after on a photo if Juno wants one.
