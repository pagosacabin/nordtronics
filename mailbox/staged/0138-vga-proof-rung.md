---
task_id: "0138"
protocol_version: 1.0.0
status: staged
expect-reply-within: 6h
iteration: 1
proof:
  branch: main (pagosacabin/cabin-play)
  sha: "eac708539842a8f82deb75ca51b6ed3fc392e0ef"
  branch_note: "The deliverable is a cross-repo commit, so the proof points at the
    DESTINATION repo's main, not at a branch of nordtronics -- the same shape as
    0136 and 0137. Verified on origin, not just locally: `git ls-remote origin
    refs/heads/main` = eac708539842a8f82deb75ca51b6ed3fc392e0ef and
    `gh api repos/pagosacabin/cabin-play/commits/main --jq .sha` returns the same
    sha, committer date 2026-10-15T19:24:20Z, 149 files, +16591/-0. Pushed over
    SSH (`git@github.com:pagosacabin/cabin-play.git`) because the github skill's
    API credential is nordtronics-scoped and API writes to cabin-play 403, exactly
    as 0136 and 0137 recorded (0136 note 1)."
  run: none
  run_note: "Earned by enumeration, not asserted. cabin-play carries NO CI at all:
    `gh api repos/pagosacabin/cabin-play/contents/.github` -> HTTP 404, the
    checkout has no .github directory, `gh run list -R pagosacabin/cabin-play`
    returns no rows, and the tree at ref=main contains no workflows. No
    nordtronics workflow can cover it either -- the deliverable is not in
    nordtronics at all (main carries 0 files under living-frame/firmware), and the
    only nordtronics runs this invocation produced are for its own mailbox
    transitions (pickup 0409726, this staging commit), which check the website,
    not this task. The build evidence for this task is therefore a local
    PlatformIO run, quoted below and committed to the repo, not a CI run URL."
  files:
    - living-frame/firmware/vga-proof/README.md
    - living-frame/firmware/vga-proof/WIRING.md
    - living-frame/firmware/vga-proof/platformio.ini
    - living-frame/firmware/vga-proof/src/main.cpp
    - living-frame/firmware/vga-proof/build-esp32dev.log.txt
    - living-frame/firmware/vga-proof/.gitignore
    - "living-frame/firmware/vga-proof/lib/ESP32Lib/** (143 files: vendored bitluni
      ESP32Lib src/ + library.json + library.properties + README.md + NOTICE.md)"
  build_log: |
    $ cd living-frame/firmware/vga-proof && pio run
    Processing esp32dev (platform: espressif32@6.12.0; board: esp32dev; framework: arduino)
    PLATFORM: Espressif 32 (6.12.0) > Espressif ESP32 Dev Module
    HARDWARE: ESP32 240MHz, 320KB RAM, 4MB Flash
     - framework-arduinoespressif32 @ 3.20017.241212+sha.dcc1105b
     - tool-esptoolpy @ 2.40900.250804 (4.9.0)
     - toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5
    Compiling .pio/build/esp32dev/src/main.cpp.o
    Linking .pio/build/esp32dev/firmware.elf
    RAM:   [=         ]   7.1% (used 23128 bytes from 327680 bytes)
    Flash: [==        ]  24.0% (used 315009 bytes from 1310720 bytes)
    Successfully created esp32 image.
    ========================= [SUCCESS] Took 6.28 seconds =========================
  build_note: "Run FROM THE DELIVERABLE TREE (cabin-play working copy) after the
    files were staged for commit and BEFORE the push, so the bytes that were built
    are the bytes that were pushed; the origin re-hash below confirms they are. 12
    warnings, every one of them the vendored ESP32Lib's pre-existing
    '-Wpointer-arith' notes inside Composite/VGAI2SEngine headers; zero warnings
    and zero errors from src/main.cpp. The 166-line log is committed as
    build-esp32dev.log.txt."
  origin_rehash: "Byte-level check after the push: six files re-downloaded from
    raw.githubusercontent.com at the pushed commit and re-hashed --
    src/main.cpp 7c376601f41c76675a4b36bb96abc761b4f658a03047f675fa1ec82e40bd7941,
    lib/ESP32Lib/src/I2S/I2S_ESP32.cpp
    33e49b650d382f164ad2822c50c388a0667c42a2098f05ba069484666a67eb6e,
    WIRING.md 72bce4aa9b3c953d677eb45659061996d3e8bc52d2fc0f1c64d11c6d1b908147,
    plus platformio.ini, README.md and lib/ESP32Lib/NOTICE.md -- all six identical
    to the local copies. The tree at ref=main lists 167 entries under
    living-frame/firmware/vga-proof."
  wiring_table_sha256: "72bce4aa9b3c953d677eb45659061996d3e8bc52d2fc0f1c64d11c6d1b908147 (WIRING.md, quoted in full in the reply body below)"
  ntfy: nordtronics-build-ed05a663 @ 2026-10-15T19:24:52Z (id NiCXnFCbU0Ru)
  ntfy_note: "Published, HTTP 200, id NiCXnFCbU0Ru. Two of the receipt's fields are
    honestly 'none' rather than omitted, because this is a build task with a
    compiled artifact but no CI: Workflow -> 'none - cabin-play carries no CI
    workflow; build is local PlatformIO esp32dev'; Artifacts -> 'none - no CI run,
    so no artifact URL'. The receipt carries the branch, the exact SHA, the
    platformio target, the RAM/flash figures and the status instead. The task file
    names no topic; nordtronics-build-ed05a663 is the topic the worker prompt and
    every prior build reply (0046-0052) use, and a build for Stephen's project is
    what it exists for."
  not_touched: "Nothing in nordtronics changed except the two mailbox transitions
    (pickup 0409726, this staging commit). No nordtronics branch was created: the
    deliverable is not a nordtronics tree. Three pre-existing untracked paths in
    the local nordtronics worktree (firmware/wildfire-node-v1/bench-override.ini,
    hardware/solar-gate-v2/, take_screenshots.py) were left exactly as found."
notes: |
  STAGED (iteration 1), 2026-10-15 19:15-19:26 UTC / 13:15-13:26 MDT. Off-peak
  (PEAK: OFF-PEAK 19:15 UTC). PROTOCOL: MATCH protocol_version=1.0.0
  (origin/main:mailbox/README.md). Gate: mailbox changed ->
  main-not-at-origin/main inbox-non-empty. Queue state at pickup: mailbox/active/
  held only .gitkeep and mailbox/inbox/ held only 0138, so 0138 was the lowest
  actionable task. Local main was 2 commits behind origin (0137's staged ->
  archive commits) and was fast-forwarded before anything was touched.

  DELIVERABLE: commit eac708539842a8f82deb75ca51b6ed3fc392e0ef on
  pagosacabin/cabin-play main -- a self-contained PlatformIO project at
  living-frame/firmware/vga-proof/ that builds a 640x480 VGA colour-bar test
  pattern for a classic ESP32.

  LIBRARY CHOICE (the task asks which, and why). bitluni's ESP32Lib, not FabGL.
  Reasons: it is the library the classic-ESP32 VGA community has used for this
  exact job for years (VGA out over I2S) so its pin configurations are the ones
  adapter hardware is built around; it is small (24% of flash here); and its
  3-bit driver keeps a 640x480 frame buffer inside the ESP32's internal SRAM.
  FabGL would need more of the chip for the same static picture and buys nothing
  for a colour-bar proof.

  WHY 3-BIT COLOUR (8 colours) AT 640x480 -- forced by memory, not preference:
  a 640x480 frame buffer is 614 kB at 14-bit and 307 kB at 6-bit, against a
  classic ESP32's 320 kB total. At 3-bit it is 154 kB (2 pixels packed per byte,
  unpacked by the I2S interrupt) and fits. It costs nothing for this proof
  because the classic eight-bar pattern IS eight colours (white, yellow, cyan,
  green, magenta, red, blue, black), all at a single full-scale level. 320x200
  at 6-bit (64 colours) remains available later if colour depth ever matters more
  than resolution; WIRING.md section 4 says how.

  PINS: R=GPIO14, G=GPIO19, B=GPIO27, HSYNC=GPIO32, VSYNC=GPIO33 -- the
  library's own classic-ESP32 set (PinConfig VGAv01 / VGABlackEdition, 3-bit view
  r4/g4/b3 + hSync/vSync). They are named constants at the top of src/main.cpp so
  the firmware and WIRING.md cannot drift apart.

  DECLARED GAP -- THE JP1 PIN MAPPING COULD NOT BE READ, AND I DID NOT GUESS IT.
  The task said the adapter's JP1 mapping should be used "if unclear ... say so in
  the notes field rather than guessing". It is worse than unclear: the schematic
  is not reachable from here at all. Checks run and their results:
  `git ls-tree -r --name-only origin/main | grep -iE 'adapter|db15|vga'` -> only
  this task file; the same over every remote branch -> only this task file;
  `git log --all -S "PCB_0024"` and `-S "JP1"` -> only the 0138 commits; a content
  grep of the whole mailbox tree -> only this task file; a filesystem search of
  the host (`find / -iname '*PCB_0024*'`, `-iname '*VGA_ADAPTER*'`, all
  *.kicad_sch outside repos, plus /home/astroboy/Desktop/Old_ubuntu_laptop) ->
  nothing; `gh search code 'PCB_0024' --owner pagosacabin` -> only this task file;
  a global `gh search code 'PCB_0024'` -> other people's projects (the Serial
  Wombat PCB_0024 VGA breakout is a real commercial part; its design files are not
  in BroadwellConsultingInc/SerialWombatPCBs, the only Serial Wombat PCB repo that
  is public, and I did not assume Stephen's board is that part). So the wiring
  spec identifies every connection by the adapter header's SIGNAL LABEL
  (R / G / B / HS / VS / GND) instead of a JP1 position number, and WIRING.md
  section 2 gives the two one-minute ways to close the gap without the schematic:
  match the silkscreen names, or buzz each JP1 pin to its DB15 pin with a
  multimeter (1=R, 2=G, 3=B, 13=HSYNC, 14=VSYNC, 5-10=GND). It also carries the
  direct-to-DB15 fallback in full, so the rung can be wired today without the
  adapter or the schematic.

  RESISTOR VALUES, DERIVED NOT COPIED: 270 ohm in series on R/G/B, because the TV
  terminates each colour line with 75 ohm and 3.3 * 75/(270+75) = 0.72 V, which is
  the VGA 0.7 V full-scale level. The 470 ohm rung of a two-rung ladder gives
  0.45 V (about 60%, visibly dim), so the ESP32's single output per channel goes
  to the 270 ohm rung and the other rung is left unconnected. Both numbers are in
  WIRING.md as a table with the arithmetic shown.

  CORRECTIONS MADE ALONG THE WAY, declared rather than hidden:
  1. Upstream ESP32Lib does not compile at all against the arduino-esp32 2.x core
     PlatformIO installs today: src/I2S/I2S_ESP32.cpp calls the ESP-IDF 3.x RTC
     API (`rtc_clk_apll_enable(true)` + `rtc_clk_apll_coeff_set(...)`), removed in
     IDF 4.4. The library is therefore VENDORED into lib/ESP32Lib with that call
     replaced by the current one, which upstream already carries commented out on
     the line above. This is a scope addition beyond "write the firmware": 143
     files of third-party source (CC-BY-SA 4.0, attribution and the full list of
     changes in lib/ESP32Lib/NOTICE.md). It was chosen over a lib_deps pin plus a
     build-time patch hook because the patched tree then builds with no patching
     step and no library download at all -- fewer ways for Stephen's own machine
     to fail. The alternative would have been pinning the 2020-era espressif32@3
     platform, which would have made the project unbuildable for anyone with a
     current VSCode/PlatformIO install.
  2. My first patch attempt fixed only ONE of the two call sites (the two blocks
     are indented differently and I matched literally); the build failed again on
     the second, and both sites are now fixed and verified by grep
     (`rtc_clk_apll_coeff_set` and the 1-argument `rtc_clk_apll_enable` both
     return 0 hits in the vendored copy).

  WHAT WAS DONE, MECHANICALLY. Project authored in scratch, built, then the
  library fetched at the pinned upstream commit, patched, vendored; the deliverable
  was copied into the cabin-play worktree, REBUILT THERE from the exact staged
  bytes (SUCCESS), committed, pushed over SSH, then re-read from origin and
  re-hashed. The reply body below quotes the wiring table.

  VERIFICATION PERFORMED vs NOT PERFORMED. Performed: local `pio run` SUCCESS in
  the deliverable tree with the RAM/flash figures above; a second identical build
  from scratch; grep proof that both APLL call sites are patched; origin re-read
  of the commit sha via ls-remote and the API; API tree listing; sha256 of six
  pushed files re-downloaded from raw.githubusercontent.com; ntfy publish
  confirmed by the returned message id. NOT performed, and not claimable: nothing
  was flashed, nothing was wired, and no picture was seen. No classic ESP32 is
  attached to this host -- the two Espressif boards on the USB bus enumerate as
  `303a:1001` ESP32-S3-family native USB-JTAG devices on /dev/ttyACM0-1, and the
  S3 has no proven VGA path in this project; there is also no display here. The
  task itself sets the acceptance bar as Stephen's photo of the Magnavox ("he
  reports it, you don't need it to stage"), so the on-screen half is his to
  report, and README.md section "Status of the proof" says so in the repo.

  ON THE BENCH INVENTORY: 0138's context states the classic ESP32 boards
  (ESP32-WROOM-32 modules, DFRobot FireBeetle ESP32-E) from Stephen's own photos
  relayed by Juno. I did not photograph the bench; my own direct observation is
  the USB enumeration above, which is a cross-check that the boards plugged in
  are NOT the target and confirms the board choice matters.
---


# 0138 — Living-frame rung 0 proof: VGA test pattern on the Magnavox

## context

Living-frame project (Christmas gift): Magnavox 22MD311B/F7 22" LCD TV driven
by ESP32 through its VGA/PC input. Stephen's ruling: proof first — nothing
else starts until the display path is proven.

Bench inventory (confirmed by Stephen 2026-10-15 with photos): classic ESP32
devkits on hand — ESP32-WROOM-32 modules and a DFRobot FireBeetle ESP32-E
(DFR0654). Both are classic ESP32 (not S3): the proven VGA path.

Play projects live in `pagosacabin/cabin-play`, never nordtronics. Push via
SSH from the laptop — the API credential is nordtronics-scoped (403 on
cabin-play), as learned on 0136.

## task

Build the rung-0 proof firmware: a classic ESP32 outputting a VGA test
pattern (color bars + a text label, 640x480) using bitluni's ESP32Lib
(or FabGL — your call, say which and why in the notes).

1. Write the firmware (PlatformIO project preferred; Arduino sketch
   acceptable if you say why). Target board: classic ESP32-WROOM-32 devkit.
2. Specify the EXACT wiring: which GPIO goes to which VGA pin (R, G, B,
   HSYNC, VSYNC, GND) including resistor values for the DAC ladder, per the
   library's wiring diagram. Stephen will wire this by hand — pin numbers
   must be unambiguous.
   NOTE 2026-10-15: Stephen has a VGA adapter for this — his
   PCB_0024_VGA_ADAPTER v1 schematic (270/470 ladders on R/G/B, direct
   HSYNC/VSYNC to a DB15). He has the physical module or can build from the
   schematic. Use the adapter's JP1 pin mapping for your wiring spec instead
   of a hand-wired ladder: the ESP32 plugs into the adapter, the adapter
   drives the VGA cable. If any JP1 pin assignment is unclear from the
   schematic, say so in the notes field rather than guessing.
3. Include flash instructions (one command or the exact IDE steps).
4. Push the project to `pagosacabin/cabin-play` under
   `living-frame/firmware/vga-proof/` via SSH.

## success_criteria

- Firmware builds cleanly for the classic ESP32 target.
- Wiring spec is complete and unambiguous (GPIO -> VGA pin + resistor).
- Stephen can flash, wire, and see a stable, correct-color test pattern on
  the Magnavox. (His photo of the TV is the final acceptance — he reports
  it, you don't need it to stage.)

## constraints

- Classic ESP32 only. No TV modifications, no panel work.
- One deliverable: proof firmware + wiring spec in cabin-play.
- No secrets in the task file or staged reply.

## proof

- Commit SHA on `pagosacabin/cabin-play` main containing
  `living-frame/firmware/vga-proof/`.
- Build log excerpt (successful build for the ESP32 target).
- The wiring table, quoted in the staged reply.

## reply_format

Stage the reply under `mailbox/staged/0138-vga-proof-rung.md` with front
matter (`task_id: 0138`, `status: staged`, `iteration: 1`), the proof
pointers above, and a `notes:` field (library choice + anything unexpected).


---

# Reply — 0138 staged: rung 0 VGA proof firmware

Deliverable: commit **`eac708539842a8f82deb75ca51b6ed3fc392e0ef`** on
**`pagosacabin/cabin-play` main** (149 files, +16591/-0) — a self-contained
PlatformIO project at `living-frame/firmware/vga-proof/`.

## Build evidence (classic ESP32 target)

```
$ cd living-frame/firmware/vga-proof && pio run
Processing esp32dev (platform: espressif32@6.12.0; board: esp32dev; framework: arduino)
PLATFORM: Espressif 32 (6.12.0) > Espressif ESP32 Dev Module
HARDWARE: ESP32 240MHz, 320KB RAM, 4MB Flash
 - framework-arduinoespressif32 @ 3.20017.241212+sha.dcc1105b
 - toolchain-xtensa-esp32 @ 8.4.0+2021r2-patch5
RAM:   [=         ]   7.1% (used 23128 bytes from 327680 bytes)
Flash: [==        ]  24.0% (used 315009 bytes from 1310720 bytes)
Successfully created esp32 image.
========================= [SUCCESS] Took 6.28 seconds =========================
```

(Reproduced cleanly twice, including once from the exact tree that was committed.
The 12 warnings in the log are all the vendored library's own pre-existing
`-Wpointer-arith` notes; `src/main.cpp` compiles warning-free. Full 166-line log
committed as `build-esp32dev.log.txt`.)

## Wiring spec, quoted

Library choice: **bitluni's ESP32Lib** (`VGA3BitI`, `MODE640x480` at 25.175 MHz).
FabGL was the alternative; ESP32Lib wins on being the library classic-ESP32 VGA
hardware is actually built around, and on fitting the frame buffer.

| ESP32-WROOM-32 pin | Signal | Adapter header label | VGA DB15 pin (direct route) | Series resistor |
|---|---|---|---|---|
| **GPIO 14** | RED   | `R`            | 1  | 270 Ω (on the adapter) |
| **GPIO 19** | GREEN | `G`            | 2  | 270 Ω (on the adapter) |
| **GPIO 27** | BLUE  | `B`            | 3  | 270 Ω (on the adapter) |
| **GPIO 32** | HSYNC | `HS` / `HSYNC` | 13 | none — direct |
| **GPIO 33** | VSYNC | `VS` / `VSYNC` | 14 | none — direct |
| **GND**     | GND   | `G` / `GND`    | 5, 6, 7, 8, 10 | none |

Resistor values, derived rather than copied: the TV terminates each colour line
into 75 Ω, so `3.3 V × 75/(270+75) = 0.72 V` — the VGA 0.7 V full-scale level.
The other rung of a 270/470 ladder gives `0.45 V` (~60 %, visibly dim), so the
ESP32's single output per channel goes on the **270 Ω** rung.

**The one thing I could not do as specified:** use the adapter's JP1 *position*
mapping. `PCB_0024_VGA_ADAPTER v1` is not reachable from here — not on
nordtronics main or any branch, not in cabin-play, not on this host, not in
`pagosacabin/ESP32_projects`, not in GitHub code search (where `PCB_0024` returns
only this task file, plus other people's projects). Rather than guess, the table
names each connection by the adapter header's signal label and WIRING.md §2 gives
two one-minute ways to close the gap: match the silkscreen names, or buzz each
JP1 pin to its DB15 pin with a multimeter. §3 carries the full direct-to-DB15
fallback, so the rung can be wired today with or without the adapter.

`WIRING.md` also has the flash commands (and the VS Code steps), the expected
serial output, what a correct picture looks like (eight full-scale bars, the
two-line label, four corner marks that expose TV overscan), and a troubleshooting
table.

## Not done here, on purpose

* **Nothing was flashed and no picture was seen.** No classic ESP32 is attached to
  this host (the two boards on the USB bus are ESP32-S3-family, `303a:1001`), and
  there is no VGA display here. Your photo of the Magnavox is the acceptance
  evidence for this rung, as your ruling already has it.
* No CI run: `pagosacabin/cabin-play` carries no workflows at all (enumerated in
  the proof block), so the evidence is the committed local build log, not a run
  URL.

## One scope addition, declared

Upstream ESP32Lib does not compile against the arduino-esp32 2.x core PlatformIO
installs today (it calls the ESP-IDF 3.x RTC clock API, removed in IDF 4.4), so
the library is **vendored** at
`living-frame/firmware/vga-proof/lib/ESP32Lib/` with that single call fixed —
143 files, CC-BY-SA 4.0, attribution and the exact change list in its
`NOTICE.md`. It buys a project that builds with no patch step and no library
download. The alternative (pinning the 2020-era platform) would have made the
project unbuildable on any current VS Code + PlatformIO install.
