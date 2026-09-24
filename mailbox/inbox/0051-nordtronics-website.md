---
task_id: "0051"
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0051 — Nordtronics static website (GitHub Pages)

## Context

Nordtronics has no public face yet. Build a static marketing/product website
for it, in the same dark theme as the companion app, covering everything we
have so far: the wildfire early-warning sensor network (flagship), the
off-grid solar / battery / embedded services, and a pilot-program call to
action. The site deploys to GitHub Pages via Actions, all version controlled.

Base the work on origin/main @
`ba7a6c1741a8d4bb5090574a75d1ca3ce6dd376e`. Create branch
`hermes/0051-nordtronics-website` from that tip.

## Task

Build a static website in a new `website/` directory at the repo root:

1. **Pages/sections** (single-page with anchored sections is fine):
   - Hero: Nordtronics — off-grid solar, custom batteries, embedded IoT.
     Pagosa Springs, Colorado.
   - Wildfire early-warning network: the product story. How it works as an
     inline SVG diagram: sensor nodes → LoRa 915 MHz → ESP32-S3 base station
     → customer Wi-Fi / MQTT over TLS → cloud → HTTPS API + push → Android
     app.
   - Sensor stack: PMS5003 (primary smoke channel), BME680 (fire-weather
     temp/humidity/pressure + secondary VOC), AS3935 (lightning).
   - Power: 18650 cell + 5 V/2 W solar, hardware-autonomous cold-charge gate
     (cuts charging at ~0 °C, re-enables at ~2.3 °C).
   - Detection: ~100 m effective radius per node; rate-of-change triggers
     with multi-node statistical consensus against false positives.
   - Privacy: only environmental telemetry leaves the property (PM2.5,
     temperature, humidity, battery voltage) — no cameras, microphones, or
     location trail. Local-only mode available (no away-from-home alerts).
   - Status honesty: phase-1 prototype in active development; label it
     "prototype" wherever the hardware/app is shown.
   - Services: off-grid solar installs, custom LiFePO4 battery builds
     (quality cells + JK BMS), ESP32/LoRa/MQTT/Home Assistant consulting.
   - Pilot program: insurer-supported pilots for the wildfire network —
     "contact us for pilot pricing". No invented prices.
   - About: short plainspoken bio of Stephen Nordlund, founder.
   - Contact: placeholders only (see constraints).
2. **Theme**: match the companion app's dark Nordtronics palette exactly
   (from `android/companion-v0/.../res/values/colors.xml` on branch
   `hermes/0049-companion-v01-ui` — the `android/` tree is not on main):
   bg `#121718`, rail `#171D1E`, surface `#242D2D`/`#2C3535`, ink `#F2F4EF`,
   muted `#AAB3B0`, line `#394342`, accent `#FF9E36`/`#FFB35E`,
   ok `#65C99B`, warn `#FF9B6E`, blue `#74BBD1`, brand-dark `#222629`.
   Use the repo logo assets in `branding/nordtronics-logo/` (full lockup +
   mark-only PNGs); favicon from the mark-only PNG.
3. **Tech**: plain HTML + CSS, vanilla JS only if needed. No build step, no
   npm, no frameworks — it must deploy to Pages as committed. Mobile-first
   responsive. Meta/OG tags for sharing.
4. **Deploy**: add `.github/workflows/pages.yml` (or extend the existing
   Pages setup if one exists on main) so pushing the branch builds and
   deploys `website/` to GitHub Pages. Do NOT add a CNAME file —
   `nordtronics.io` is not purchased yet; that is a follow-up task.
5. **App screenshots**: you may copy the real 0049 emulator screenshots
   (`android/companion-v0/screenshots/*.png` on branch
   `hermes/0049-companion-v01-ui`) into the site to show the companion app.
   Do not fabricate screenshots.

## Success criteria

- `website/index.html` (+ assets) renders the full site with all sections
  above; dark theme matches the palette hex-for-hex.
- SVG architecture diagram is legible at mobile width.
- No broken links/images; valid HTML (no console errors when served).
- GitHub Actions Pages workflow is green at the branch tip SHA and the site
  is reachable at the `*.github.io` Pages URL.
- Desktop (1280px) and mobile (390px) screenshots of the rendered site are
  committed under `website/screenshots/`.

## Constraints

- Every factual claim on the site must come from the data in this task or be
  labeled prototype/TBD. No invented specs, prices, testimonials, customers,
  or availability dates.
- Do NOT claim the Android app is on the Play Store. Label it prototype.
- Do NOT publish real contact details: no phone numbers, no personal email
  addresses, no home address. Use clearly-marked placeholders
  (e.g. `hello@example.com` with an HTML comment `<!-- STEPHEN: replace -->`).
- Do NOT publish server IPs, internal infrastructure, or anything about the
  VPS.
- Keep the copy plainspoken and human, no corporate polish.
- One deliverable: the website + its deploy workflow. No firmware, app,
  hardware, or DNS work.

## Proof

- Origin branch: `hermes/0051-nordtronics-website` with full SHA.
- Green GitHub Actions run at that exact SHA (Pages build + deploy).
- Live GitHub Pages URL of the deployed site.
- Committed `website/screenshots/desktop.png` and `website/screenshots/mobile.png`.
- ntfy build-green receipt on topic `nordtronics-build-ed05a663`.

## Reply format

Front-matter `status: staged` with the `proof:` block (branch + SHA, run
URL, Pages URL, ntfy receipt, files list) and a `notes:` field describing
the site structure, how the theme match was verified, and anything marked
TBD/placeholder for Stephen.
