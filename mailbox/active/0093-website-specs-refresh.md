---
task_id: "0093"
protocol_version: 1.0.0
status: inbox
expect-reply-within: 24h
proof:
  branch: ""
  sha: ""
  run: ""
---

# 0093 — Website: refresh wildfire tech specs + swap contact to hello@nordtronics.io

# Context

nordtronics.io's wildfire specs section is stale relative to decisions made 2026-10-02.
The site is the single-page `website/` dir in pagosacabin/nordtronics (dawn reskin already
live, `website-check.yml` CI). This task updates the specs content and the contact address,
then redeploys.

Authoritative values (all decided 2026-10-02 — do not invent alternatives):

- **Environmental sensor is BME688, not BME680.** The 680 is end-of-life; the 688 is the
  drop-in (±0.5 °C; same driver). Site currently says BME680 ±1.0 °C — replace everywhere,
  including the sensor-stack cards and any comparison tables.
- **Battery:** 3.7 V 3000 mAh li-ion, single cell or 2P (6000 mAh). Autonomy design
  estimates (Juno-corrected, pending Hermes's 0092 validation): ≈11–12 days single cell,
  ≈23 days dual cell, no sun. Site currently says "18650, 3000–3500 mAh, 7–14 days" —
  replace with the real config. Drop "NMC or LiFePO4 option" unless it is still true.
- **Solar:** 13 W / 5 V panel. Site currently says "5 V / 2 W" — replace. Keep the
  cold-charge gate block as-is (still accurate).
- **Detection logic:** the site is vague ("rate-of-change triggers with multi-node
  statistical consensus"). It can now be specific — consensus lives in the **base station**
  (Stephen's call): a node rise = PM2.5 ≥ 25 µg/m³ AND ≥ 15 µg/m³ above its rolling baseline
  on two consecutive packets; a network Watch needs ≥2 neighbor nodes confirmed inside a
  20-minute window; a single elevated node never auto-escalates. Present these as
  **design targets, field validation pending** — the sim harness (0091) hasn't run yet.
  Keep the existing "working prototype, specs will change" framing.
- **AS3935 interface:** site says "I²C / SPI" — the bench wiring is SPI (INT on GPIO6).
  Say SPI.
- **Contact:** swap every `hello@example.com` placeholder (and any other example address)
  to `hello@nordtronics.io`. Receiving already works via Cloudflare Email Routing.

# Task

1. Update the specs content per the values above. Keep the dawn palette, layout, and the
   privacy section ("No cameras · No microphones · No location trail") untouched.
2. Swap the contact placeholder to hello@nordtronics.io everywhere it appears.
3. Run `website-check.yml` (or the repo's normal site check) and redeploy the way the
   dawn reskin went out.

# Success criteria

1. No mention of BME680, 2 W panel, or hello@example.com remains on the live site.
2. Battery/solar/detection numbers match the values above, with "design target / field
   validation pending" caveats where noted.
3. Site CI green and the live site shows the updates.

# Constraints

- Content update only. No redesign, no palette or layout changes.
- Do not present the detection thresholds as validated — "design target" language is mandatory.
- Keep it cheap: DeepSeek Flash, minimal reasoning.

# Proof

Branch + SHA on origin, site-check run URL + result, and the live URLs (or section anchors)
showing each change. The staged reply is the deliverable.

# Reply format

```yaml
branch: "<branch name>"
sha: "<origin SHA>"
run: "<site-check run URL + result>"
changes: "<per-item: old text -> new text, with live URL>"
contact: "<hello@nordtronics.io live where the placeholder was>"
notes: "<anything Stephen should know>"
```
