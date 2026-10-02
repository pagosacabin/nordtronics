---
task_id: "0093"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 24h
proof:
  branch: "hermes/0093-website-specs-refresh"
  sha: "c5e3379c659bdbd89b907375a681b39739441cba"
  run: "https://github.com/pagosacabin/nordtronics/actions/runs/37046628759 (Website Check, success, headSha == branch tip)"
  merged_to_main: "d4c3ed9b35ae85092b85e419a57f2d5061212e94 — merge commit, run https://github.com/pagosacabin/nordtronics/actions/runs/37046707985 (Website Check, success)"
  files:
    - website/index.html
    - website/screenshots/desktop.png
    - website/screenshots/mobile.png
    - .github/workflows/website-check.yml
  applied_to_vps:
    - "deploy@89.117.21.105: /var/www/nordtronics (synced from ~/nordtronics-src @ d4c3ed9)"
    - "backup of the previous web root: /home/deploy/backups/nordtronics-www-20261002-202129.tar.gz"
  live_probe: "https://nordtronics.io/ last-modified Fri, 02 Oct 2026 18:21:30 GMT; served index.html sha256 df7ea72bd265f60549c593194f3da1e9412e387d0cdf195f5b26c3bdfa062668 == repo sha256"
notes: "Content-only change; no palette, layout or privacy-section edits. Contact placeholder was ALREADY hello@nordtronics.io in the tree (0068 landed it) — nothing to swap, verified by decoding the live Cloudflare-obfuscated addresses, not by grep. Scope extensions declared: (1) .github/workflows/website-check.yml gained this branch in its push allow-list — without it a push to a fresh hermes/NNNN-* branch starts no run at all (rule 13); (2) website/screenshots/desktop.png and mobile.png were regenerated — the committed copies dated 2026-09-26 (pre-0070/pre-reskin) are served at 200 from /screenshots/ and are full-page renders of the site, so they carried the stale BME680 / 2 W text that criterion 1 forbids. Correction declared: the task's battery autonomy figures (≈11–12 d / ≈23 d) are used verbatim on the site, but Hermes's own 0092 (verified, archived) landed on 10.5 d / 20.9 d for the same cells — the 9% gap is the explicit 0.90 usable x 0.85 cold deratings vs one folded factor. Site carries 'design target' language; if Stephen prefers the derated figures, that is a one-line follow-up. Deploy note: two unreferenced PNGs are still served from Cloudflare's edge cache (max-age=14400) at their bare URLs — origin bytes verified correct via a cache-busting query."
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

---

# Reply — 0093: wildfire specs refreshed and live

```yaml
branch: "hermes/0093-website-specs-refresh"
sha: "c5e3379c659bdbd89b907375a681b39739441cba"
run: "https://github.com/pagosacabin/nordtronics/actions/runs/37046628759 — Website Check, success, headSha c5e3379... == branch tip"
changes: |
  1. Environmental sensor BME680 -> BME688, everywhere it appeared:
     - sensor-stack diagram: three node labels BME680 -> BME688 (nodes 1-3)
     - sensor card heading BME680 -> BME688, and its temperature spec
       "±1.0 °C" -> "±0.5 °C" (the 688's accuracy; same driver)
     LIVE: https://nordtronics.io/#wildfire  (section "Multi-Modal Detection")
  2. Battery card "18650 Li-ion Cell / Li-ion (NMC or LiFePO4 option) /
     3000–3500 mAh typical / 3.7 V / 3.2 V / 7–14 days no sun"
     -> "Li-ion Battery Pack / Li-ion, 3.7 V nominal / 3000 mAh single cell,
     or 6000 mAh (2P) / Runtime (design target): ≈11–12 days single cell,
     ≈23 days 2P, no sun". The NMC/LiFePO4 option line is dropped (not true
     of this pack).
     LIVE: https://nordtronics.io/#power
  3. Solar card "5 V / 2 W Solar Panel ... Power: 2 W (peak, full sun)"
     -> "13 W / 5 V Solar Panel ... Power: 13 W (peak, full sun)".
     The stale "~350 mA max into cell" figure is replaced by "Set by the
     charge controller" — no charge-current number is asserted anywhere,
     because the only sourced panel fact is 13 W / 5 V.
     LIVE: https://nordtronics.io/#power
  4. Diagram power badge "18650 + 5V Solar" -> "Li-ion + 13 W Solar".
     Cold-charge-gate block left exactly as it was (still accurate).
  5. Detection logic, now specific and explicitly unvalidated: the card reads
     PM2.5 ≥ 25 µg/m³ AND ≥ 15 µg/m³ above the node's own rolling baseline on
     two consecutive packets; consensus in the BASE STATION; a Watch needs
     ≥ 2 neighboring nodes confirmed inside a 20-minute window; a single
     elevated node never auto-escalates; "design targets — field validation
     is pending". The section intro now says consensus is base-station-side.
     LIVE: https://nordtronics.io/#detection
  6. AS3935 "Interface: I²C / SPI" -> "SPI (INT on GPIO6)".
     LIVE: https://nordtronics.io/#wildfire  (sensor-card grid)
contact: "Nothing to swap — the tree already carried hello@nordtronics.io in
  both contact-path mailto links (0068 landed it), so the site was never
  serving a placeholder in this revision. Verified the way it can actually be
  verified: Cloudflare rewrites addresses to /cdn-cgi/l/email-protection and a
  grep of the live page for any address returns ZERO hits even when a
  placeholder is live. Decoding the live page's data-cfemail values yields
  exactly 'hello@nordtronics.io?subject=Off-grid power inquiry' and
  'hello@nordtronics.io?subject=Custom electronics inquiry'. No example.com
  string exists anywhere in the source or the served bytes."
notes: |
  Deploy: merged to main (d4c3ed9) and synced website/ to /var/www/nordtronics
  on deploy@89.117.21.105 from the host checkout ~/nordtronics-src, the same
  path the dawn reskin went out through. Previous web root backed up first to
  /home/deploy/backups/nordtronics-www-20261002-202129.tar.gz. The deployed
  index.html hashes identically to the repo copy
  (df7ea72b…062668); https://nordtronics.io/ reports last-modified
  2026-10-02 18:21:30 GMT and no longer contains BME680, "2 W", "3000–3500",
  "7–14 days" or "LiFePO4 option".

  Two scope extensions, declared with their criterion as the reason:
  (a) website-check.yml's push allow-list gained hermes/0093-website-specs-refresh.
      The workflow triggers only on listed branches, so a push to a fresh
      hermes/NNNN-* branch starts no run at all — the one-line diff is what
      makes criterion 3's CI evidence attainable.
  (b) website/screenshots/desktop.png and mobile.png were regenerated from the
      updated page. They are committed, served (HTTP 200 at /screenshots/) and
      are full-page renders of the site; the copies in the tree dated
      2026-09-26, i.e. before the 0070 copy rewrite and the dawn reskin, so
      they contained the BME680 / 2 W text criterion 1 forbids. They are not
      referenced by index.html, so nothing the page displays depended on them.
      Caveat, not a defect: Cloudflare's edge still serves the pre-deploy bytes
      at those two bare URLs (cf-cache-status: HIT, cache-control max-age=14400,
      age ~291 after the sync); the origin is correct — a cache-busting query
      returns bytes whose sha256 equals the repo copy exactly
      (desktop.png 8a374587…4c1a). The edge self-heals inside 4 hours; no
      purge token exists on this host.

  One correction declared rather than silently smoothed: the task's battery
  autonomy figures (≈11–12 days single cell, ≈23 days 2P) are what the site now
  says, verbatim, and the task named them authoritative. Hermes's own 0092 —
  verified and archived today — landed on 10.5 days / 20.9 days for the same
  3000 mAh / 6000 mAh cells. The 9% gap is not a contradiction: 0092 makes the
  0.90 usable x 0.85 cold deratings explicit instead of folding them into one
  factor. The site labels both as design targets, and the detection thresholds
  are likewise labelled design targets with field validation pending, so
  nothing on the page reads as measured. If Stephen prefers 0092's derated
  numbers, it is a one-line follow-up.

  Not claimed: no pixel-level before/after render diff was performed for this
  change — the content edits are text-only and were verified by reading the
  deployed bytes and the rendered page text, not by image diffing.

  ntfy receipt: not applicable, and stated rather than omitted — this task
  compiles nothing and the reply format it specifies (branch/sha/run/changes/
  contact/notes) has no receipt field. No artifact was published.
```
