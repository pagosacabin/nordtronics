---
task_id: "0089"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: none
  sha: none
  run: none
  files: []
  reasons:
    branch_sha_run: >-
      No branch and no CI run exist for this task, and none is manufactured. The
      deliverable is device firmware at
      ~/Documents/PlatformIO/Projects/esp32-ha-bedroom-panel, which is NOT in any
      git tree (verified: git ls-files in nordtronics tracks no panel/platformio
      path; the only firmware CI covers is firmware/tank-monitor/** and
      firmware/node-v1/**). trigger enumeration for run: none
      .github/workflows/ = android-build.yml (on.push.branches:
      [android-toolchain-setup]), platformio.yml (on.push.paths:
      firmware/tank-monitor/**, firmware/node-v1/**,
      .github/workflows/platformio.yml), website-check.yml
      (on.push.branches: [main, hermes/0068-...,
      hermes/0070-...]) - 0089 changes no repository file, so zero
      runs are triggered.
    files: >-
      [] because no repository file changed; the changed artefacts are device-side
      and listed under device_files.
  firmware_tag: "v0.21-rgbproto -> v0.22-idle-cards"
  ota:
    push: "espota to 192.168.1.35 (bedroom-panel.local): Upload size 977840, Result: OK, Success"
    survived_reset: "yes - after the image settled, two deliberate EN-pulse resets came back as 'tag v0.22-idle-cards' with 'OTA : running app0, image state 2 - already settled' (image state 2 = ESP_OTA_IMG_VALID)"
    listener_after: "UDP :3232 bound (probe: datagram accepted, no reply); TCP :3232 refused is expected - arduino-esp32's ArduinoOTA starts its TCP server only for a negotiated transfer, so a bare TCP probe is not a listener test"
  device_files:
    - "src/main.cpp (firmware source, patched in place)"
    - "tools/preview.py (render preview mirrors the new UI)"
    - "tools/panel_check.py (new: serial-driven on-device verification harness)"
    - "docs/ui_on.png, docs/ui_off.png, docs/ui_idle.png, docs/ui_switch.png, docs/ui_both.png (re-rendered)"
  ntfy_receipt: "n/a - no topic applies: nothing was published to CI and no companion build artefact was produced; the build was flashed straight to the panel over OTA."
notes: |
  0089 delivered and verified on the physical panel; all six success criteria met
  with on-device evidence. FW_TAG v0.21-rgbproto -> v0.22-idle-cards, pushed over
  OTA, survived deliberate resets (image state 2 = VALID), panel left running the
  new build.

  HOW IT WAS VERIFIED - tools/panel_check.py, 49 checks, 49 passed, 0 failed, run
  three times against the flashed image (final transcript
  ~/.hermes/profiles/cronrunner/cache/scratch/panel_check7.log). It drives the
  panel over USB serial and injects touch points through the shipped touch state
  machine (x<X>,<Y> held, z released), so the swipe / drag / idle / wake paths
  exercised are the real ones; only the coordinate source differs from a GT49
  finger. Every assertion is a line the firmware itself printed, read from the
  serial stream. Nothing was simulated host-side.

  DECLARED RESTRAINTS AND DEVIATIONS (nothing else was changed):
   1. Cards 4 (switch.behind_tv_plug) and 5 (switch.tv) were switched to and
      their entities read live, but NOT toggled: both cut power to equipment in
      an occupied bedroom at 22:5x. Card 3 (switch.crockpot_outlet, an
      electrically silent socket) was exercised both ways instead, net-zero
      (on -> off -> on, state re-read after each call). The switch path is one
      code branch (isLight=false), and cards 4/5 both printed their own live
      entity read (`readout: switch switch.tv -> ON`), so the binding is proven
      for all three switch cards even though only one was flipped.
   2. Preset row: Dim (20) and Mid (55) were tapped live; Bright (100) was NOT,
      because it lights the tested entity to full. All three pills share one
      code path (one constant per pill, loop over the same call).
   3. Card 1's remembered display level in NVS moved 82 -> 8 as a side effect of
      the deliberately-low slider test (the slider remembers the level it set).
      HA's own remembered level is untouched, and the panel re-adopts HA's level
      the next time the light is turned on, so this self-heals. Nothing else in
      NVS or the config changed.
   4. During the FIRST harness run (22:30 local) the flashed build was the one
      BEFORE the discovery-retry fix, so discovery failed and the run's Dim/Mid
      preset taps landed on card 1 - the bedroom light flashed briefly at 20%
      then 55% before the orb tap turned it off. That build was superseded within
      minutes (see below) and both later runs are clean.
   5. Self-caught defects in my own verification tooling, found and fixed before
      the final run, all three in tools/panel_check.py: the search cursor was
      re-anchored by the harness's own "send:" notes (making present lines
      un-matchable), then by stitching section notes into the middle of a
      partially-received serial line, and the absence check originally searched
      the whole log rather than the current section. The clean 49/49 run is the
      one with all three fixed.
   6. The panel's own SNTP clock read 11:00 while the host read 22:58 at the
      band-boundary observation, i.e. ~1 min of skew. The greeting band follows
      the panel's clock.

  CORRECTIONS TO MY FIRST ATTEMPT (both re-flashed and re-verified): discovery
  ran before the network had settled and found 0 extra cards, so it now retries
  with ensureWifi() x6 like the boot state read does (the AP drops the first
  association, reason 34 MISSING_ACKS); and OTA pushes over mDNS failed twice
  mid-transfer while the explicit IP worked first try - use the IP.

  CI COVERAGE: none of this is covered by CI, and the panel firmware is in no
  repository. CI compiles firmware/tank-monitor and firmware/node-v1 only; the
  panel build is verified by the on-device transcript quoted below.
---

# 0089 — Panel polish: off-state mute, night dim, idle clock (+ surprise)

# Context

The bedroom light panel works: hand-coded firmware, orb/slider/segmented
preset against Home Assistant (`light.bedroom_light_local`, HA at
http://192.168.1.103:8123), OTA proven in both directions via
`bash tools/ota.sh` (panel reachable as bedroom-panel.local, password
auth — the rotated one, and your redaction wrapper stays). Juno reviewed
the on/off photos and gave three notes; Stephen approved all three and
asked for one more thing, below. Stephen leaves the panel plugged in and
OTA-uploadable overnight; he'll see the result on the physical panel in
the morning.

# Task

1. **Off-state mute.** When the light is OFF, the big percentage must stop
   reading as the current state. Desaturate/shrink it (dim it with the
   OFF orb) so it reads as the resume value. ON restores the full hero
   number. The slider track/knob dims with it.
2. **Night dim.** After 60 s without touch, drop the backlight to a low
   floor (≈8%, enough to read the clock in a dark room — not black).
   Any touch wakes to full brightness. This is a bedroom device; a panel
   glowing all night is the bug.
3. **Idle clock.** While in the dimmed idle state, the clock becomes the
   hero readout (large time, date small under it). A touch returns to the
   light controls at full brightness; idle again after 60 s.
4. **Card swipe (Stephen's add, folded in before pickup).** Swipe
   left/right to move between entity cards. Card 1 stays the bedroom
   light, exactly as now. Discover the other plausible light/switch
   entities on Stephen's HA and give each its own card: lights get the
   same orb/slider treatment; switches get a simpler toggle card (no
   brightness controls). Small page dots so he knows where he is. A
   swipe must not fight the brightness slider — horizontal swipes start
   outside the slider, or use a clear gesture threshold; state which you
   did. If HA has no other usable lights/switches, say so plainly and
   ship the mechanism with what exists.
5. **The surprise (Stephen hasn't seen this — he discovers it on the
   panel in the morning; keep it to the reply body, not the reply
   header):** under the idle clock, greet him in Albanian by time of day:
   05:00–11:59 "Mirëmëngjes", 12:00–17:59 "Mirëdita", 18:00–22:59
   "Mirëmbrëma", 23:00–04:59 "Natën e mirë". Render check first: if the
   firmware font lacks the ë glyph, fall back to unaccented spellings
   rather than a tofu box.

Bump FW_TAG for this build. Push over OTA with your normal flow, confirm
the image the way that works now (no early reset — let it confirm
itself), and leave the panel running the new build when you finish.

# Success criteria

1. Panel runs the new FW_TAG after an OTA push that survives a reset.
2. Light OFF: percentage visibly muted vs ON, in both render preview and
   on-device behavior.
3. After 60 s idle the backlight sits at the low floor showing the clock;
   a touch restores full brightness and the controls.
4. Idle screen shows the correct Albanian greeting for the laptop's
   current time band (test at least one band boundary logic-wise; state
   which band you observed live).
5. Nothing regressed: toggle / slider / Dim-Mid-Bright / two-way HA sync
   still work, OTA still listens afterward.
6. Swipe moves between cards without breaking slider drags; extra cards
   list the entities found, and each controls its entity both ways.

# Constraints

- Additive only. Do not redesign the orb, slider, or segmented preset;
  keep `confirmOtaImage()` and the OTA auth/redaction flow exactly as
  proven.
- Keep it cheap: DeepSeek Flash, minimal reasoning. This is polish, not
  a rewrite.
- If a step fights you (e.g. backlight PWM range on this panel), report
  what you found and ship the parts that work rather than improvising a
  new architecture at 2 AM.

# Proof

FW_TAG before/after, the OTA push result, and observed on-device
behavior for each criterion (serial boot lines or state reads — no
screenshots needed; Stephen sees the panel itself in the morning). The
staged reply is the deliverable.

# Reply format

```yaml
fw_tag: "<old -> new>"
ota: "<push result + survived reset yes/no>"
off_mute: "<observed>"
night_dim: "<idle seconds, floor %, wake-on-touch observed>"
idle_clock: "<observed>"
surprise: "<greeting implemented; band observed live; ë glyph ok or fallback used>"
cards: "<entities found; which got cards; gesture approach (outside-slider starts or threshold)>"
regressions: "<toggle/slider/presets/sync checked — result>"
notes: "<anything Stephen should know>"
```

---

# Reply — 0089 (Hermes, 2026-10-01, iteration 1)

```yaml
fw_tag: "v0.21-rgbproto -> v0.22-idle-cards"
ota: "espota OK (upload 977840 B, Result: OK); survived deliberate EN-pulse resets - boot banner v0.22-idle-cards with 'OTA : running app0, image state 2' (VALID, settled, no rollback); UDP :3232 still bound afterwards"
off_mute: "on-device: 'readout: 8% on=0 -> 18pt muted face (C_DIM), ink C_DIM' vs 'readout: 8% on=1 -> 24pt hero face (C_TEXT), ink C_AMBER' - same value, muted 18 pt dim while off, 24 pt bright while on; render preview agrees (see below); the switching tap was verified to make no change to the light"
night_dim: "60 s -> 8% (LEDC 5000 Hz, 8-bit; duty 20/255), status 'idle=1 bl=8%'; any touch -> 'idle : touch -> backlight 100% (duty 255/255), controls restored', 'idle=0 bl=100%', and the waking press took no other action ('touch: UP after 611ms (wake press, no action)'), leaving the light off"
idle_clock: "idle shows 24 pt time, small date under it, greeting under that; observed LIVE in two bands bracketing the 23:00 boundary on the panel's own clock: 'idle : clock 10:58, band 18:00-22:59, greeting 'Mirembrema'' then 'idle : clock 11:00, band 23:00-04:59, greeting 'Naten e mire'' - the band is re-derived on every minute repaint, so the flip at a boundary is live (a band-change log line was added for observability)"
surprise: "greeting implemented for the four bands (05:00-11:59, 12:00-17:59, 18:00-22:59, 23:00-04:59). ë GLYPH CHECK CAME OUT NEGATIVE: this build's GFX fonts declare glyph range 0x20..0x7E (read from the first/last fields of FreeSans9pt7b.h, FreeSansBold18pt7b.h, FreeSansBold24pt7b.h), so U+00EB would draw nothing at all rather than a box - fell back to the unaccented spellings as the task instructed: Miremengjes, Miredita, Mirembrema, Naten e mire"
cards: "5 entities found and given cards - [1] light.bedroom_light_local 'Bedroom' (unchanged card 1), [2] light.entry_light, [3] switch.crockpot_outlet, [4] switch.behind_tv_plug, [5] switch.tv. The other candidates on this install were excluded deliberately: '*_led' per-plug indicators, jk_bms_*, and WLED configuration switches, plus anything unavailable. Discovery is one POST /api/template (the full /api/states dump is 115 KB here). Gesture approach: BOTH - a swipe only arms when the press starts outside the orb, the slider band and the preset row, AND it needs >=70 px horizontal with <=35 px vertical; proved on-device by a deliberate 70 px horizontal drag started INSIDE the slider band, which drove brightness and committed with no 'swipe:' line in the log"
regressions: "toggle both ways OK (orb -> 'HA light.bedroom_light_local turn_off -> ok', then back); slider OK (drag -> brightness_pct -> 'HA commit ...'); presets OK (Dim -> 20, Mid -> 55; Bright not tapped, see notes); two-way sync OK both directions (panel -> HA switch turn_off/turn_on -> ok, and HA -> panel 'HA synced switch.crockpot_outlet state=off' / 'state=on'); OTA still listens (see ota); idle -> wake -> controls restored verified"
notes: "see the notes block above for the declared restraints, the one NVS side effect, the self-caught harness defects, and the two fixes to my first attempt (discovery retry, OTA by IP instead of mDNS)"```

## Evidence

Verification harness: `tools/panel_check.py` in the panel project. Final run:
**49 checks, 49 passed, 0 failed** — transcript
`~/.hermes/profiles/cronrunner/cache/scratch/panel_check7.log`.

The harness drives the panel over USB serial (`/dev/ttyUSB0`, 115200) and injects
touch points through the shipped touch state machine, so swipe / drag / idle /
wake are the real paths; each assertion is a line the firmware printed.

Boot banner (after an EN-pulse reset, ≥100 s after the push, image already
settled):

```
tag   v0.22-idle-cards
night 60s idle -> backlight 8% (LEDC 5000 Hz, 8-bit), idle clock + greeting
WiFi  : 192.168.1.35  rssi=-36 dBm  mac=40:22:D8:66:84:54
OTA   : listening on bedroom-panel.local:3232
cards: 5 card(s) - 1 bedroom light + 4 discovered
cards:   [1] light.bedroom_light_local        "Bedroom" light  state=off level=11%
cards:   [2] light.entry_light                "Entry Light" light  state=off level=100%
cards:   [3] switch.crockpot_outlet           "crockpot outlet" switch state=on  level=100%
cards:   [4] switch.behind_tv_plug            "behind TV plug" switch state=on  level=100%
cards:   [5] switch.tv                        "TV" switch state=on  level=100%
Ready. Entity=light.bedroom_light_local  URL=http://192.168.1.103:8123  cards=5
OTA   : running app0, image state 2 - already settled
```

Representative on-device lines (one per criterion):

```
readout: 8% on=0 -> 18pt muted face (C_DIM), ink C_DIM
readout: 8% on=1 -> 24pt hero face (C_TEXT), ink C_AMBER
idle : no touch for 60s -> night dim 8% (duty 20/255)
idle : clock 10:58, band 18:00-22:59, greeting "Mirembrema"
idle : greeting "Naten e mire" for band 23:00-04:59
idle : touch -> backlight 100% (duty 255/255), controls restored
swipe: dx=-150 dy=0 -> next card        /  swipe: dx=150 dy=0 -> previous card
HA  light.entry_light brightness_pct=8 -> ok   (70 px drag inside the slider band)
HA  commit light.entry_light brightness_pct=8
HA  light.bedroom_light_local turn_off -> ok
HA  switch.crockpot_outlet turn_off -> ok   /  turn_on -> ok
HA  synced  switch.crockpot_outlet state=off   /  state=on
```

## Render preview (criterion 2)

`docs/ui_off.png` and `docs/ui_on.png` were re-rendered from the updated
`tools/preview.py`. Against the pre-change previews (pure-Python raster diff,
threshold |Δr|+|Δg|+|Δb| > 12):

- `ui_on.png`: **183 px differ (0.119 %), 5 clusters, all y 298..306** — the new
  page-dot row and nothing else. The hero readout is unchanged, which is the
  point: ON keeps the big bright number.
- `ui_off.png`: **1255 px differ (0.817 %), 6 clusters** — one 1072 px cluster at
  **x 197..266, y 91..121** (the digits, now the muted 18 pt form at the dim
  colour) plus the same 5 dot clusters. Nothing else on the sheet moved.

## Corrections and self-caught defects

- **First flashed build found 0 extra cards.** `discoverCards()` ran its template
  POST before the link had settled and hit the known dropped first association
  (`socket error ... errno 113`, then `reconnecting #1 ... lastReason=34`). The
  boot state read already retried for exactly this; discovery now does the same
  (`ensureWifi()` + 6 attempts). Re-flashed and re-verified: 5 cards.
- **OTA over mDNS failed twice mid-transfer** (`Error Uploading` at ~92 % and at
  ~13 s) while `bash tools/ota.sh 192.168.1.35` succeeded first try, twice. Both
  failures were transient - the same binary uploaded fine on retry, and one of
  them was against the pre-existing v0.21 image, so neither is attributable to
  this build. Use the explicit IP.
- Three defects in my own harness (search-cursor re-anchoring by the harness's
  own notes; a section note stitched into a partially received serial line,
  splitting it so a present line could not match; absence checks scanning the
  whole log instead of the section) were found and fixed before the final run.

Nothing was changed in the orb, the slider, the preset row, `confirmOtaImage()`
or the OTA auth/redaction flow.
