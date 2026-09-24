---
task_id: "0051"
status: verified
iteration: 1
proof:
  - branch: hermes/0051-nordtronics-website @ a79383dfc4ac4d6385724f4a6f6554e9948b4f75
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36025328226 (Website Check, success at branch tip SHA)
  - files: website/index.html (39376 bytes), website/assets/favicon.png, website/assets/og-image.png, website/screenshots/v01-*.png + mobile.png
notes: Hermes never filed his staged reply — no ntfy receipt, task sat in active. Stephen directed Juno to verify and deploy ("Build", 2026-09-24 ~10:18 AM MDT). Juno verified independently: green CI at exact branch SHA, full 39KB index.html with all spec sections, honest prototype/Play Store labeling ("not on the Play Store"), no placeholders/TODOs, SVG diagram present. Deployed to VPS /var/www/nordtronics 2026-09-24 ~10:25 AM MDT; https://nordtronics.io verified 200 with full page + favicon. Proof pointers above were assembled by Juno, not Hermes.
---

# 0051 — Nordtronics static website (VPS deploy)

## Context

Nordtronics has no public face yet. Build a static marketing/product website
for it, in the same dark theme as the companion app, covering everything we
have so far: the wildfire early-warning sensor network (flagship), the
off-grid solar / battery / embedded services, and a pilot-program call to
action.

Deploy target changed since this task was first drafted: `nordtronics.io` is
now purchased and its DNS points at our VPS, where nginx already serves a
placeholder page from `website/index.html` (see main). There is NO GitHub
Pages involved — the site source lives in `website/` in this repo, and
deployment to the VPS is handled separately after your work is verified. Do
not add any Pages workflow, CNAME file, or DNS records.

Base the work on origin/main @
`9289a0eebcda354f120919605ab4b7069a1b369b`. Create branch
`hermes/0051-nordtronics-website` from that tip.

## Task

Build the full static website, replacing the placeholder in the `website/`
directory at the repo root:

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
   npm, no frameworks — the files in `website/` are deployed to the VPS web
   root exactly as committed. Mobile-first responsive. Meta/OG tags for
   sharing.
4. **Deploy is out of scope for this task.** Do not touch the server, DNS, or
   any workflow that publishes the site. Juno syncs `website/` to the VPS
   after your branch is verified. Your job ends at a complete, correct
   `website/` directory on your branch.
5. **App screenshots**: you may copy the real 0049 emulator screenshots
   (`android/companion-v0/screenshots/*.png` on branch
   `hermes/0049-companion-v01-ui`) into the site to show the companion app.
   Do not fabricate screenshots.

## Success criteria

- `website/index.html` (+ assets) renders the full site with all sections
  above; dark theme matches the palette hex-for-hex.
- SVG architecture diagram is legible at mobile width.
- No broken links/images; valid HTML (no console errors when served from a
  local static server).
- Site works served as plain static files — no server-side rewrites,
  no assumed base path other than `/`.
- Desktop (1280px) and mobile (390px) screenshots of the rendered site are
  committed under `website/screenshots/`.
- A CI workflow (`.github/workflows/website-check.yml`) runs an HTML/link
  sanity check on the branch and is green at the branch tip SHA. It must NOT
  deploy anywhere.

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
- Do NOT add GitHub Pages config, CNAME files, DNS records, or any deploy
  workflow. Server and DNS are not yours in this task.
- Keep the copy plainspoken and human, no corporate polish.
- One deliverable: the website source. No firmware, app, hardware, DNS, or
  server work.

## Proof

- Origin branch: `hermes/0051-nordtronics-website` with full SHA.
- Green GitHub Actions run of `website-check.yml` at that exact SHA.
- Committed `website/screenshots/desktop.png` and `website/screenshots/mobile.png`.
- ntfy build-green receipt on topic `nordtronics-build-ed05a663`.

## Reply format

Front-matter `status: staged` with the `proof:` block (branch + SHA, run
URL, ntfy receipt, files list) and a `notes:` field describing the site
structure, how the theme match was verified, how the local-serve check was
done, and anything marked TBD/placeholder for Stephen. Note explicitly that
VPS deployment is left to Juno.
