---
task_id: "0089"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
proof: []
notes: ""
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
