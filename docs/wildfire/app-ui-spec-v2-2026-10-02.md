# Nordtronics Wildfire Companion — UI Spec v2 (2026-10-02)

Three-screen redesign: **Property overview → Node detail → Alerts**. Incorporates the
2026-10-02 design review (mock-data consistency, alert states, battery card, palette,
location wording, Android frame).

## Global

- **Palette — dawn-in-the-pines, taken from the live nordtronics.io CSS:**
  - Background ink-pine `#101B15`; elevated cards `#1C2E28` / `#171F1A`
  - Bone text `#EDE6D6`; secondary text `#9AA3AD`
  - Solar amber `#F2A33C` — brand and active states ONLY
  - Sage `#9DBE9C` — healthy / all-clear / cleared states
  - Muted sky `#86B8CC` — data accents and info
  - Alarm orange `#E76F51` — warnings and alerts (never amber; the two must stay distinct)
- Frame: modern **Android** phone, status bar 7:31, 5G indicator.
- Thin top banner on every screen: **"PROTOTYPE – MOCK DATA"**.
- Bottom nav: **Nodes** (active) · **Alerts** (badge) · **System**.
- Footer on every screen:
  `Readings from local mock backend · Privacy-first · No cameras · Locations stay on your network`
  (Zone/trail names are the user's own property labels; nothing leaves the local network.)
- Staleness is always visible: "Last packet received 4 min ago" / "Updated 4 min ago · Live" with pulse.

## Mock state — identical on all three screens

- **Node 01** — Healthy. PM2.5 12.4 µg/m³ · 70 °F (21.3 °C) · 38 % RH · battery 87 %, solar charging. Updated 4 min ago.
- **Node 02** — **Watch**. PM2.5 47.9 µg/m³ (elevated on ONE node — **not** an alert; consensus
  requires ≥2 nodes rising before escalation) · 82 °F · 22 % RH · battery 3.71 V (below 3.8 V
  notice threshold). Updated 4 min ago.
- Property banner: **"Watch — 1 of 2 nodes outside limits. Rest of network at baseline."** + "2 of 2 nodes reporting".

## Screen 1 — Property overview ("Property line")

- Header: mountain mark + "Property line", green dot "Last packet received 4 min ago".
- Watch banner (alarm-orange border): "1 of 2 nodes outside limits. Rest of network at baseline."
  + "2 of 2 nodes reporting" in sage, small rising sparkline at right.
- Metric cards 2×2: PM2.5 median 30 µg/m³ · Air temperature 76 °F · Humidity 30 % RH ·
  Reporting 2/2 nodes. Thin sparklines, muted-sky line color.
- "Property schematic" with "Approximate layout" label: dashed property boundary, scale bar
  0/50/100 ft, Node 01 (sage dot) and Node 02 (alarm-orange dot + Watch halo), connecting line.
- "Field nodes" list:
  - Node 01 — Healthy · Updated 4 min ago — 12.4 µg/m³ · 70 °F · 38 % RH · 87 % (sage battery)
  - Node 02 — **Watch · awaiting confirmation** · Updated 4 min ago — 47.9 µg/m³ · 82 °F · 22 % RH · 3.71 V (alarm-orange accents)
- Alerts badge: **2**.

## Screen 2 — Node 01 detail

- Header: "Nordtronics / Wildfire companion", back arrow, overflow menu.
- Live banner: "Updated 4 min ago" + "Live" with sage pulse line graphic.
- Title "Node 01 — Outdoor Air & Environmental Monitor", "Healthy" sage chip,
  "Sierra Ridge Trail · Zone 7 · Node ID: NT-01-7A3F".
- Metric cards 2×2 with 12-hour sparklines labeled -12h / -6h / Now:
  - PM2.5 12.4 µg/m³ + sage "Clean-air" chip (sage sparkline, flat-ish)
  - Temperature 70 °F (21.3 °C) (alarm-orange line only because warm, no chip)
  - Humidity 38 % RH (muted-sky sparkline)
  - Battery: **87 % · Solar charging · +6 % today** with battery icon (voltage lives in hardware section, not here)
- "Link & hardware" — Connected (sage dot): LoRa Link RSSI −68 dBm with margin bar 32 dB ·
  Gateway hops 1 · Hardware v2.3.1 · HW Rev 4 · Serial I-01-7A3F-22 · **Battery 4.05 V**.
- "Trends": 24-hour PM2.5 chart, 7:00→7:00, threshold bands labeled exactly
  **"Clean < 12"** (sage), **"Elevated 12 – 35"** (amber/orange), **"> 35"** (alarm orange).
  Legend: "PM2.5 · Now: 12.4 µg/m³". "24-hour trend" dropdown.
- "Recent history": SMOKE · node-01 · Sep 23 — **"Cleared"** sage badge, single **"View trend"**
  button. No Acknowledge on cleared events — that was the v1 contradiction.
- Alerts badge: **2**.

## Screen 3 — Alerts

- Title "Alerts", subtitle: "Consensus-based detection cuts down false alarms."
- Hero card: sage checkmark, "No active alerts",
  "The network needs a matching rise across nearby nodes before escalating a smoke event.
  Node 02 is elevated on its own — shown as Watch until a second node confirms."
- Filter chips: **All (4) · Watch (1) · Smoke (1) · Battery (1) · Heat (1)**.
- Recent activity (newest first):
  1. **WATCH · node-02** — "PM2.5 47.9 µg/m³ on one node, awaiting confirmation." Severity: Watch (alarm orange). Buttons: View nodes · View trend.
  2. **BATTERY · node-02** — "Battery below 3.8 V." Severity: Notice (muted sky). Unacknowledged. Buttons: View node · Acknowledge.
  3. **HEAT · node-02** — "Temperature above 27 °C." Severity: Warning. State: **Acknowledged** (sage check). Buttons: View trend · View node.
  4. **SMOKE · node-01** — "PM2.5 rising fast." Sep 23 · 13:58. State: **Cleared** (sage). Single button: View trend.
- Alerts badge: **2** (= the two unacknowledged items: Watch + Battery notice).

## Alert state machine (for the builder)

- `Watch` (single-node elevation) → never auto-escalates; clears when the node returns to baseline.
- `Active` → `Acknowledged` → `Cleared`. Acknowledge is offered only on Active/Notice items.
- `Notice` (battery etc.) → `Acknowledged` or `Snoozed 24h`.
- Escalation to `Active` smoke alert requires a matching rise on ≥2 nearby nodes within the window.
- Badge count = unacknowledged Watch + Active + Notice items. Never counts Cleared/Acknowledged/Snoozed.

## What changed from v1 (review log)

1. One consistent mock state across all three screens (badge 2 everywhere; median math checks out).
2. Node 01's stale SMOKE warning now reads "Cleared" with no Acknowledge CTA.
3. Battery card: "% · Solar charging · +gained today"; voltage moved to Link & hardware.
4. Footer: "Locations stay on your network" replaces the contradictory "No location".
5. Chart band typo fixed ("Clean < 12" etc.).
6. Android frame / 5G instead of iPhone / 3G.
7. Consistent unit spacing ("70 °F", "38 % RH").
8. Palette pinned to the live site's dawn-in-the-pines hex values; alarm orange #E76F51 kept distinct from brand amber #F2A33C.
