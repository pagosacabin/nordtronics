---
task_id: "0134"
protocol_version: 1.0.0
status: verified
expect-reply-within: 6h
iteration: 1
proof:
  - branch: hermes/0134-guition-battery-adc
    repo: pagosacabin/nordtronics
    sha: 7be846568424ab5f505115b84126c69b18de8f41
    note: "Branch and SHA are on the origin of THIS repo. The panel's firmware is not
      a PlatformIO tree in this repo (and its own git checkout's remote,
      pagosacabin/JC4827W543-ESPHome, returns 404 from the GitHub API -- that repo
      does not exist), so the fixed ESPHome config is mirrored here on the task
      branch. The mirrored jc4827w543.yaml is byte-identical to the file flashed to
      the panel: sha256 d355ded833659ec972442612df107e42b8f978e49438c037226e607083d965cc,
      checked against the bench-host copy after the flash."
  - run: none
    note: "Earned by enumeration, not asserted. .github/workflows/ holds android-build.yml
      (on.push.branches [android-toolchain-setup] only), android-companion-v0.yml
      (branch allow-list + paths android/companion-v0/**), backend.yml (paths
      backend/**), detection-sim.yml (paths python/detection-sim/**), platformio.yml
      (paths firmware/tank-monitor/**, firmware/node-v1/**, firmware/wildfire-node-v1/**,
      python/detection-sim/**), website-check.yml (branches [main, hermes/0068-*,
      hermes/0070-*, hermes/0093-*]). This branch changes firmware/guition-jc4827w543/**
      only, which is in none of those path filters and is not in platformio.yml's
      firmware allow-list, so its push starts zero runs. Confirmed: gh run list
      --branch hermes/0134-guition-battery-adc returns empty."
  - files:
      - firmware/guition-jc4827w543/jc4827w543.yaml
      - firmware/guition-jc4827w543/README.md
      - firmware/guition-jc4827w543/0134-battery-log.txt
    note: "The config carries the fix; the log file is the durable before/after console
      evidence (the reply quotes from it); the README states the mirror's provenance
      and the sha256."
  - device_evidence:
      device: "jc4827w543 (Bench Panel 4.3), 192.168.1.72, MAC 44:bd:8d:7a:cc:cc"
      build_running: "ESPHome 2026.9.1 compiled on 2026-10-15 09:22:58 -0600 (the fixed build)"
      readings: "raw=2975..2982 counts, pin=2.4580..2.4587 V, batt=4.2456..4.2479 V, Battery=100 %"
      capture: "esphome logs jc4827w543.yaml --device 192.168.1.72 (ESPHome API stream --
        reads over the network, opens no serial port, toggles no DTR/RTS; nothing here
        reset the board). The OTA upload itself reboots the device, as flashing does."
      quoted_in: firmware/guition-jc4827w543/0134-battery-log.txt
  - ntfy: none
    note: "No ntfy receipt applies: the task's deliverable is a behaviour fix on one
      bench board, not a CI-built artifact, and it names no build topic. The device
      evidence above is the receipt."
notes: |
  STAGED (iteration 1), 2026-10-15 09:15-09:30 MDT. Off-peak (PEAK: OFF-PEAK 15:15 UTC).
  PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md).

  THE TASK'S CAUSAL STORY IS WRONG, AND THE EVIDENCE SAYS SO. The task expected the ADC
  to be on the 3V3 rail ("wrong GPIO, or a pin with nothing wired to it"). It is not:
  the node sits at 2.4572 V (pre-fix: 3.268 V displayed / 1.32998), which a 3V3 tap
  cannot produce, and GPIO5 is exactly the pin the vendor pin table calls "Battery
  divider to ADC1_CH4". No pin move was needed. The defect is the SCALE FACTOR: the
  config used the schematic's unfitted 33K/100K (x1.32998) while the board is fitted
  with 160K/220K (x1/0.57895 = x1.72727) -- the same fitted-vs-drawn discrepancy the
  task's own vendor reference documents. And the flatness was never a dead pin: the
  node is stable because USB is connected and the TP4056 holds BAT at the charge
  voltage. 3.268 x (1.72727/1.32998) = 4.2447, i.e. exactly the value the pack net
  reads. Two decimals of arithmetic, no pin swap.

  WHAT CHANGED. multiply 1.32998 -> 1.72727; attenuation auto -> fixed 12db (the node
  peaks at 2.43 V on a full pack, so nothing clips -- auto existed only to chase the
  clipping the WRONG divider implied, and it cannot be combined with a raw read); a
  per-read console line `[I][battery] GPIO5 (ADC1_CH4) raw=<counts> pin=<V> V batt=<V> V`.
  The UI is untouched, ADC1 only.

  WHAT THIS RUN DID NOT VERIFY -- stated plainly, no proof pointer attached:
    1. NO METER READING. I have no multimeter. The pin voltage reported is the device's
       own curve-fitted calibrated ADC, not an independent instrument.
    2. THE UNPLUG TEST WAS NOT PERFORMED. The panel's USB is plugged into the worker
       host (its USB-Serial/JTAG shows in /dev/serial/by-id as 44:BD:8D:7A:CC:CC) and I
       have no physical access; I cannot pull the cable or disconnect the pack. So
       "with the battery unplugged the displayed value must move" is UNPROVEN by this
       run. It needs one bench minute: pull the USB cable and read `Battery voltage` --
       it should fall from the charge voltage to the pack's resting volts (and the
       reading is only a pack voltage with the cable out; the config has said so since
       before this task).
    3. Criterion 1 is therefore also qualified: displayed 4.246 V vs the task's meter
       4.1 V is |0.146 V|, inside +-0.15 V, but it is a charger-held comparison -- the
       TP4056 CV target is ~4.2 V, so with the cable out the two numbers would be
       compared like for like.
  What does stand without hands: the node is measurably NOT the 3V3 rail, it is stable
  at exactly the value the fitted divider predicts for the net, and the same node
  produced both the old 3.268 V and the new 4.246 V -- so the fix changed the scale,
  not the measurement.

  UNEXPECTED FINDINGS.
    - The firmware's own repo has no origin. The bench host's checkout
      (~/Documents/esphome/JC4827W543, remote github.com/pagosacabin/JC4827W543-ESPHome.git)
      is not on GitHub: gh api repos/pagosacabin/JC4827W543-ESPHome -> 404, and `gh repo
      list pagosacabin` shows only nordtronics, hermes-agent, pagosacabin, Pi-projects.
      Without an origin there is no branch tip to cite, so the fix is mirrored into this
      repo on the task branch (byte-identical, sha256 in the README and above) and is also
      committed in the bench checkout on its own local branch hermes/0134-battery-divider-factor
      (commit c6ec129, local only -- nothing to push to). Creating that GitHub repo is a
      decision for Stephen/Juno, not a side effect I took on my own authority.
    - ESPHome rejects a second adc sensor on the same pin ("Pin 5 is used in multiple
      places" -- no allow_other_uses on the adc pin schema), so the raw-count debug line
      samples through the sensor's public output_raw switch instead: one extra ADC read
      per logged line, then back to volts.
    - The panel's own text was not photographed (no camera on the bench host). I verified
      the sensor states that drive the display lambda ('Battery voltage' 4.246 V,
      'Battery' 100 %), not the pixels.
    - Battery % moved 3 -> 100 because the same sensor feeds it; that is the expected
      consequence of the scale fix, not a separate change.

  VERIFIED BY JUNO 2026-10-15 ~10:00 MDT (bench proof, Stephen's hands):
    - Branch hermes/0134-guition-battery-adc @ 7be846568424ab5f505115b84126c69b18de8f41
      confirmed on origin via branch-sha (matches staged proof pointer exactly).
    - Meter check: Stephen's meter 4.17 V vs display 4.25 V = |0.08 V|, inside the
      +-0.15 V acceptance bar (better than the reply's own 4.246-vs-4.1 comparison).
    - Unplug test (the run's stated gap #2): Stephen pulled USB; display fell from
      4.25 V / 100% (charger-held) to 4.17 V / 97% (resting) -- photo evidence in
      chat. The reading moves with the pack, not the rail. Acceptance criteria met.
    - Repo decision (run's unexpected finding): Stephen ruled 2026-10-15 that play
      projects go to a DIFFERENT repo, never nordtronics. Relocating the mirrored
      firmware/guition-jc4827w543/ out of the product repo is pending his choice of
      destination repo; the mirror stays on the task branch until then.
---

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
