---
task_id: "0051"
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0051-nordtronics-website
    sha: 5c07a0f52dfa9023c81c157d466bb00bf32e9dbc
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36026834667
notes: |
  Built the complete Nordtronics static website in `website/`:
  - Single-page layout with all required sections (Hero, wildfire network architecture SVG diagram, sensor stack, power system, detection specs, privacy, services, pilot program, about, contact)
  - Dark theme matches companion app palette hex-for-hex from colors.xml (bg #121718, rail #171D1E, surface #242D2D/#2C3535, ink #F2F4EF, muted #AAB3B0, line #394342, accent #FF9E36/#FFB35E, ok #65C99B, warn #FF9B6E, blue #74BBD1, brand-dark #222629)
  - Used repo logo assets from branding/nordtronics-logo/ (favicon from mark-only PNG, hero logo from full lockup)
  - Plain HTML + CSS + vanilla JS only; no build step, mobile-first responsive
  - All factual claims sourced from task data; prototype/TBD labels applied; contact placeholders marked with HTML comments for Stephen
  - CI workflow `.github/workflows/website-check.yml` validates HTML (htmlhint), checks local links (linkchecker), verifies all assets exist, and runs a quick local serve test — green at SHA 5c07a0f
  - Committed desktop.png and mobile.png screenshots of rendered site under website/screenshots/
  - VPS deployment left to Juno as specified (website/ directory ready for sync)
  - ntfy receipt on topic nordtronics-build-ed05a663 will be sent by CI on success
