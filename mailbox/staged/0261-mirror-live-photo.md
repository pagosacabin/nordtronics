---
task_id: "0261"
protocol_version: 1.4.0
status: staged
iteration: 1
proof:
  branch: "hermes/0261-mirror-live-photo"
  repo: "pagosacabin/cabin-play"
  sha: "b5c0f5257c7f1c8a07278e0f5aa03ebbec4e1e5e"
  base: "ac8c40ae6bd1560515fefeabed4092a9f791a62c (hermes/0260-refresh-jitter, as the card names)"
  run: none
  run_note: |
    cabin-play carries no workflows (`ls .github/workflows/` -> "No such file or directory"),
    so no run exists or is expected.  The deliverable was built, flashed and verified on the
    bench: `pio run -e freenove-cam -t upload` -> "Hash of data verified", firmware.bin
    sha256 991caa87b91b85b0cf12ef03cb26c6c62b359ee7c96dcff67340d1aa7b63c2f9, and
    `esptool --port /dev/ttyUSB0 verify_flash 0x10000 firmware.bin` -> "Verification
    successful (digest matched)".  The flash used the committed tree (worktree clean at
    b5c0f52 apart from untracked .pio dirs).
  files:
    - living-frame/firmware/freenove-cam/src/main.cpp
    - living-frame/firmware/freenove-cam/README.md
  mirror_mechanism: >-
    The GC0308 was delivering a mirrored image, and it is the driver's own doing: its init
    table (gc0308_settings.h) writes {0x14, 0x11} -- page 0, reg 0x14, bit0 = h-mirror -- so
    the part starts mirrored.  Measured at boot, not assumed:
    "sensor : power-on state hmirror 1, vflip 0 (reg 0x14)".  The panel path was ruled out
    first and independently: the frame publishes the received raster straight, and
    convert_photo64.py's portrait->raster map (bx = cy, by = 479 - cx) has determinant +1, a
    rotation, which is the same handedness as live.c's view_index() -- no mirror downstream.
  fix: >-
    set_hmirror(s, 0) in camera_start() -- one I2C write at init, zero per-pixel cost.
    The build_raster() x-flip fallback was not needed.  Note the card's phrasing assumed the
    fix would turn mirroring ON (set_hmirror to enable the mirror); the measurement says the
    bit was already ON by driver default, so the fix is to clear it.  Implemented as the
    card's preferred path, direction corrected from the evidence.
  mechanism_proof: >-
    One flashed build with a temporary runtime toggle for reg 0x14 bit0, and the built raster
    dumped as a 64x24 ASCII luminance map after each capture: the hmirror=1 and hmirror=0
    dumps are exact horizontal mirrors of each other (bright object lower-left vs lower-right,
    gradient on opposite sides), so the bit is the mechanism and one flip is the whole fix.
    The toggle was removed from the shipped build; read-only hooks 'e' (capture without
    sending) and 'd' (ASCII dump) remain.
  text_shot: >-
    NOT TAKEN -- PENDING STEPHEN.  A text shot needs a hand: a button press on the frame (or
    a text object held in front of the camera) and eyes on the panel.  This run made no claim
    of a visual confirm.  What is proven instead is the chain: the delivered raster is now
    the un-mirrored one (dumps above) and nothing downstream of the camera mirrors.
  timing: >-
    End-to-end shoot->display NOT re-measured (needs a press; 2682 ms on 2026-11-25 morning's
    baseline).  Camera-side send of the fixed build: "sent : 1280 pkts, 3662 ms, ok 1282 fail
    0".  The fix adds one 1-byte I2C register write at init and no per-pixel work.
  regressions_checked: >-
    changes confined to the camera firmware: frame firmware, ESP-NOW chunk protocol and
    session-id gating, slideshow, keep/delete prompts, camera notice, swap behaviour, splash
    and convert_photo64.py all untouched (diff is main.cpp + the camera README only).  The
    frame board was not reflashed; it was reset once to read its report and returned to its
    live loop.
  copilot_review: >-
    two passes on the branch.  Pass 1: FIX -- 1 MINOR (camera_start() ignored set_hmirror()'s
    return, and the driver updates status.hmirror before its register writes, so a failed
    write would have reported hmirror 0 while the part stayed mirrored) and 1 NIT ('d' before
    the first capture could print uninitialised PSRAM).  Both fixed at the source in b5c0f52.
    Pass 2: "1. MINOR -- Resolved ... 2. NIT -- Resolved ... Verdict: SHIP".
  vault: "crew-vault/bench/living-frame.md -- new section 'Live camera orientation', committed 197c4c2"
  ntfy: "not published -- the card names no topic and this is firmware, not a companion APK artifact"
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-26T00:32:10+00:00"
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: mixed
  rates_used: "unknown -- the harness does not expose per-call usage in this session"
  cost_usd: unknown
notes: |
  DONE ON THE FIRMWARE, FLASHED AND MECHANISM-PROVEN; the text shot and Stephen's visual
  confirm are PENDING.  Staged per the 0260 precedent (visual confirm owed by Stephen, who is
  away from the bench) -- nothing is claimed as visually confirmed.

  1. MECHANISM, measured.  Boot log from the flashed build:
       sensor  : power-on state hmirror 1, vflip 0 (reg 0x14)
       sensor  : after fix    hmirror 0, vflip 0 (reg 0x14), write ok
     The driver's init table writes {0x14, 0x11}, and esp32-camera maps reg 0x14 bit0 to
     h-mirror (set_hmirror does set_reg_bits(0x14, bit0)).  So the sensor was mirrored at
     power-on; that is the backwards text.  The panel path was excluded separately: the live
     raster is published straight, convert_photo64.py's map is a rotation (det +1), and
     live.c's view_index() matches the converter's rot90 preview exactly.

  2. FIX, one line plus its return check: set_hmirror(s, 0) in camera_start().  Correcting the
     card's assumption is deliberate: the card said "preferred fix: the sensor's own
     horizontal-mirror setting (set_hmirror)" as if enabling it, but the bit was already ON,
     so the value that fixes the image is 0.  The fallback (flip x in build_raster) was not
     used -- the sensor path is free and fixes it for every consumer.

  3. PROOF OF THE FLIP, without eyes.  A measuring build carried a temporary runtime toggle of
     reg 0x14 bit0 and an ASCII dump of the built raster.  hmirror=1 vs hmirror=0 dumps are
     exact horizontal mirrors (same scene, bright region mirrored left<->right).  The toggle
     is not in the shipped build; 'e' (capture, don't send) and 'd' (dump) remain as read-only
     bench hooks -- 'e' exists so the frame board need not be disturbed to inspect the camera.

  4. BOARD STATE.  /dev/ttyUSB0 (CH340, camera) carries the branch build: verify_flash digest
     matched.  The frame board was reset once (esptool read of its report partition) and is
     back in its live loop; no frame firmware was written.  Declared bench side effect: the
     frame was rebooted, so any pending on-panel prompt it held was cleared.

  5. HONEST GAPS, all needing a hand: the text-object shot (the criterion), the normal-scene
     shot, Stephen's visual confirm, and the end-to-end shoot->display timing.  The camera
     side is ready: one press of the frame button will produce a shot with the corrected
     orientation.

  6. DEVIATION DECLARED: two read-only bench hooks were added beyond the one-line fix ('e',
     'd'), and the shipped build was flashed to the camera board.  No other file changed.
---

# 0261 — Living frame: mirror live camera shots so text reads correctly

## Assignment

- **Hermes — firmware + bench (this task):** implement the mirror fix below, verify it on the panel with a real text shot, and push the branch. Button presses during verification are Stephen's.
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Camera firmware stays with Hermes.
- **Stephen — bench side:** button presses during verification; visual confirm that text reads correctly on the panel.

## Context

- Stephen held a hat with words on it up to the camera and shot it: the words display backwards on the frame. The live camera image is mirrored left-right.
- Juno read the code before filing. The mirror is in what the GC0308 sensor delivers: `freenove-cam/src/main.cpp` has no flip anywhere (`build_raster()` copies pixels straight through, `camera_start()` sets brightness/saturation only), and the frame side just publishes the received buffer. The 90-degree counter-clockwise rotation in `convert_photo64.py` is a rotation, not a mirror — rotation cannot produce backwards text.
- `convert_photo64.py` is NOT in the live-camera path. It is a host-side tool for converting still photos into .bin framebuffer files for the SD slideshow and embedded assets. Do NOT flip anything in `convert_photo64.py`: the pre-populated stills are normal photos and must stay un-mirrored.
- The gift plan (Stephen, Nov 25): the current SD library (45 frames) is being wiped. The card will be pre-populated with the Black Bart's photos Juno scraped from Facebook/Instagram plus whatever Christy sends, converted through `convert_photo64.py`, and that frame + card ships as the gift. This task does NOT touch the library — no deleting, no rewriting, no batch-flipping stored frames. The mirror fix is for new camera shots taken with the frame (people holding up hats/signs), so they read correctly going forward.
- OUT OF SCOPE: the boot splash / Nordtronics logo — do not regenerate, rotate, flip, or otherwise modify the splash asset. The 0259/0260 slideshow and swap work stays as-is.

## Task

Mirror live camera shots left-right so text reads correctly on the panel. One deliverable: the camera firmware on branch `hermes/0261-mirror-live-photo` in cabin-play, with a text shot reading correctly on the bench.

1. Establish the actual mirror mechanism from the code/datasheet before changing anything: confirm the GC0308 delivers a mirrored image (sensor register / mounting) and state it in the reply — do not assume.
2. Preferred fix: the sensor's own horizontal-mirror setting (`set_hmirror`) in `camera_start()` — one line, zero per-pixel cost, fixes every shot at the source. If the GC0308 driver ignores it on this board, or it proves not to be the mechanism, fall back to flipping x in `build_raster()` (read the source pixel from `FB_W-1-rx`). Say which one landed and why.
3. Verify on the panel with a real text object (hat, sign, or book with words): shoot it and confirm the words read correctly. Also shoot a normal scene and confirm it still looks right (not double-mirrored).
4. Confirm no regressions: shoot timing stays ~2.9 s shoot-to-display, the ESP-NOW chunk protocol and session-id gating are unchanged, and the 0259/0260 slideshow, keep/delete prompts, camera notice, and swap behavior are untouched.
5. Record the outcome in the crew-vault living-frame bench notes (name the vault page in the reply).

## Success criteria

- A shot of printed text displays with the words reading correctly on the panel (Stephen's visual confirm).
- A normal scene still looks correct, not mirrored the wrong way (Stephen's visual confirm).
- Shoot-to-display timing unchanged (~2.9 s); no chunk-protocol or slideshow regressions.
- No change to `convert_photo64.py`, no change to the splash, no change to the stored SD library.
- Branch `hermes/0261-mirror-live-photo` on origin in cabin-play; Copilot SHIP.

## Constraints

- Camera firmware only (`living-frame/firmware/freenove-cam/`), unless the frame side is proven necessary — say so in the reply if it is.
- Branch from `hermes/0260-refresh-jitter` @ ac8c40ae6bd1560515fefeabed4092a9f791a62c (latest verified frame branch on origin).
- No new hardware. No secrets in the commit (repo is public). Push cabin-play via SSH from the laptop (the gh.py credential is 403 on cabin-play writes).
- Cost: DeepSeek pinned tier as usual; no model choice without a stated bound.

## Proof

- Branch name + commit SHA on origin for pagosacabin/cabin-play.
- Bench log lines: the shoot, the shot count, and the send log for the verification shot.
- Before/after panel photos of the same text object (or a short video) showing the words backwards before the fix and reading correctly after — Stephen's eyes are the gate, his visual confirm closes it.
- Copilot review verdict.

## Reply format

- `branch`: name + SHA on origin
- `mirror_mechanism`: one line, what the sensor was doing and how it was confirmed
- `fix`: set_hmirror / build_raster flip + one line on why that one
- `text_shot`: what object, before/after result, Stephen's confirm (or pending)
- `timing`: shoot-to-display ms for the verification shot
- `regressions`: slideshow/prompts/notice/swap — pass or fail each, one line each
- `vault`: page updated

## Status: firmware fixed, flashed, mechanism proven — text shot and visual confirm PENDING STEPHEN

`mirror_mechanism` — the GC0308 was delivering a mirrored image, and it is the driver's own
init table doing it: `gc0308_settings.h` writes `{0x14, 0x11}`, and esp32-camera maps reg 0x14
bit0 to h-mirror. Measured at boot, not assumed:

```
sensor  : power-on state hmirror 1, vflip 0 (reg 0x14)
sensor  : after fix    hmirror 0, vflip 0 (reg 0x14), write ok
```

The panel path was ruled out separately: the frame publishes the received raster straight,
`convert_photo64.py`'s portrait→raster map (`bx = cy, by = 479 - cx`) is a rotation
(determinant +1), and `live.c`'s `view_index()` is exactly the `np.rot90(..., k=-1)` the
converter uses for its as-seen preview. Nothing downstream of the camera mirrors.

`fix` — `set_hmirror(s, 0)` in `camera_start()`: one I2C write at init, zero per-pixel cost.
The `build_raster()` x-flip fallback was not needed. **The card's direction was inverted by the
evidence**: it reads as though `set_hmirror` would turn a mirror on, but the bit was already
ON by driver default, so the value that fixes the image is `0`. I implemented the card's
preferred mechanism with the measured value and state the correction here rather than
mirroring an assumption.

How the flip was proven without eyes: a measuring build carried a temporary runtime toggle of
reg 0x14 bit0 plus an ASCII dump of the built raster. At hmirror=1 the bright region sits
lower-left with the gradient upper-left; at hmirror=0 the same scene is exactly reversed
left↔right — an exact horizontal mirror, so the bit is the mechanism and one flip is the whole
fix. The toggle is not in the shipped build.

`text_shot` — **NOT TAKEN, pending Stephen.** It needs a hand: a button press on the frame (or
a text object held in front of the camera) and eyes on the panel. Nothing in this reply claims
a visual confirm. A normal scene is likewise unshot.

`timing` — end-to-end shoot→display not re-measured (needs a press; this morning's baseline on
this rig was 2682 ms). Camera-side send of the fixed build: `sent : 1280 pkts, 3662 ms, ok 1282
fail 0`. The change adds one 1-byte register write at init and no per-pixel work.

`regressions` — frame firmware: untouched. ESP-NOW chunk protocol + session gating: untouched.
Slideshow: untouched. Keep/delete prompts: untouched. Camera notice: untouched. Swap behaviour:
untouched. Splash: untouched. `convert_photo64.py`: untouched. Stored SD library: untouched
(nothing stored, nothing deleted by this run). The diff is `main.cpp` and the camera README.

`vault` — `crew-vault/bench/living-frame.md`, new section "Live camera orientation: the sensor
was mirrored, not the panel — 0261" (commit `197c4c2`).

`copilot` — two passes on the branch. Pass 1: **FIX** — 1 MINOR (`camera_start()` ignored
`set_hmirror()`'s return, and the driver sets `status.hmirror` before its register writes, so a
failed write would have reported hmirror 0 while the part stayed mirrored) and 1 NIT (`d` before
the first capture could print uninitialised PSRAM). Both fixed at the source in `b5c0f52`. Pass
2: "1. MINOR — Resolved … 2. NIT — Resolved … **Verdict: SHIP**".

`board state` — /dev/ttyUSB0 (CH340, camera) carries the branch build:
`verify_flash 0x10000` → "Verification successful (digest matched)" against sha256
`991caa87…b63c2f9`. The frame board (FTDI) was reset once to read its report partition and is
back in its live loop; no frame firmware was written. Declared side effect: that reset cleared
any on-panel prompt the frame was holding.

### What Stephen's one press will close

Press the frame button with any printed text in front of the camera: the shot should now read
the right way round, and a normal scene should look normal (not mirrored the other way). If it
still reads backwards, the mirror is physical (lens/sensor mounting) rather than the register,
and the next step is the `build_raster()` x-flip instead — say so and I will switch it.
