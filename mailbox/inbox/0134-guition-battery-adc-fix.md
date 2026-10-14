# 0134 — Guition JC4827W543 battery voltage reads 3.29V with the battery at 4.1V

task_id: 0134
status: inbox
expect-reply-within: 6h

## context

The on-screen battery indicator shows `BAT 3% 3.29 V` while a meter on the
battery reads 4.1 V. The displayed value was 3.28 V last night and 3.29 V this
morning — essentially flat while the actual battery voltage moved from low to
4.1 V. A wrong divider factor would still track the battery; this does not.
3.29 V is the 3V3 rail, so the ADC input is almost certainly not seeing the
battery: wrong GPIO in firmware, or a pin with nothing wired to it.

Hardware facts for this board (Guition JC4827W543, ESP32-S3, 4.3" 480x272):

- The FITTED battery divider is 160K/220K, factor ~1.73. The schematic draws
  33K/100K (factor 1.33) — the fitted parts differ, use 1.73. Documented at
  https://github.com/78/xiaozhi-esp32/blob/HEAD/main/boards/guition/jc4827w543/README.md
- The ESP32-S3 has no factory ADC calibration; expect a few percent of error
  even when everything else is right.
- Use an ADC1 channel. ADC2 is unavailable while WiFi is up.
- Some board revisions may have no factory battery tap at all. If the divider
  is absent on this unit, say so explicitly instead of shipping a fake reading.

## task

Fix the battery voltage reading in the Guition firmware:

1. State which GPIO your battery code currently reads. Meter that pin with the
   battery at ~4.1 V and report the pin voltage.
2. If the pin is wrong or unconnected, move the reading to the GPIO the
   battery divider actually feeds (probe to find it; report which GPIO and the
   pin voltage you measured there).
3. Apply the 1.73 divider factor (Vbat = Vadc * 1.73), on an ADC1 channel.
4. Add a serial debug line printed on each battery read: GPIO number, raw ADC
   counts, computed pin voltage, computed battery voltage.

## success_criteria

- Displayed battery voltage within ±0.15 V of a meter reading at ~4.1 V.
- The reading visibly TRACKS the battery: with the battery unplugged the
  displayed value must move (not sit at ~3.29 V).
- If this board revision has no factory divider tap, the reply states that
  plainly with the probing evidence — no fabricated reading.

## constraints

- One deliverable: a truthful battery reading. Do not redesign the UI.
- ADC1 only. No secrets in the task file or staged reply.

## proof

- Branch name and commit SHA on origin with the fix.
- Serial log excerpt showing, per read: GPIO, raw ADC counts, pin voltage,
  battery voltage.
- The meter value you compared against, and the displayed value at that time.

## reply_format

Stage the reply under `mailbox/staged/0134-guition-battery-adc-fix.md` with
front matter (`task_id: 0134`, `status: staged`, `iteration: 1`), the proof
pointers above, and a `notes:` field describing anything unexpected found
while probing.
