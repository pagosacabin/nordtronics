---
task_id: "0075"
protocol_version: 1.0.0
status: verified
iteration: 2
expect-reply-within: 6h
proof:
  branch: hermes/0075-solar-gate-v2-io-labels
  sha: 397f17d23018a74b592e838a77c0ca31208926dc
  run: none — no workflow matches hardware/** (evidence in "CI" below)
  main_transition_run: https://github.com/pagosacabin/nordtronics/actions/runs/36414628981
  files:
    - hardware/solar-gate-v2/solar-gate-v2.kicad_sch
    - hardware/solar-gate-v2/solar-gate-v2-schematic.svg
    - hardware/solar-gate-v2/solar-gate-v2-schematic.png
    - hardware/solar-gate-v2/solar-gate-v2-schematic.pdf
    - hardware/solar-gate-v2/solar-gate-v2-erc.txt
    - hardware/solar-gate-v2/README.md
  renders:
    svg: hardware/solar-gate-v2/solar-gate-v2-schematic.svg
    png: hardware/solar-gate-v2/solar-gate-v2-schematic.png (7016x4961, 600 dpi A4 landscape)
    pdf: hardware/solar-gate-v2/solar-gate-v2-schematic.pdf
  netlist: 14 nets / 29 components — membership of the original 27 unchanged
  erc: 0 errors / 24 warnings, all [lib_symbol_mismatch]
  model: deepseek-flash
---

# 0075 — I/O connectors and device labels for solar-gate-v2

**Status: staged.** Deliverable is on `hermes/0075-solar-gate-v2-io-labels` @
`397f17d23018a74b592e838a77c0ca31208926dc` (verified `git ls-remote --heads origin
hermes/0075-solar-gate-v2-io-labels` == local `HEAD`).

## What changed

1. **J1 "SOLAR IN"** — `Connector_Generic:Conn_01x02`, pin 1 → `P`, pin 2 → `GND`.
   Footprint family chosen: **JST GH 1.25 mm**,
   `Connector_JST:JST_GH_SM02B-GHS-TB_1x02-1MP_P1.25mm_Horizontal` (a stocked,
   latching 2-pin; v1 already standardises on JST GH).
2. **J2 "TO HELTEC"** — same symbol/footprint, pin 1 → `SOLAR_OUT`, pin 2 → `GND`.
3. **NTC note** — "NTC on leads — thermally couple to the battery cell".
4. **Six block annotations** — cold comparator, hot comparator, fault-OR (DA/DB),
   Vref (U2/TL431), gate drive (Q3), pass switch (Q1A/Q1B/Q2A/Q2B back-to-back).
5. **Renders re-exported** from the committed revision: SVG, PNG (7016x4961, 600 dpi
   A4 landscape, same recipe as 0074), PDF.
6. **README** updated: revision history, J1/J2 rows in the component table, `P`/`GND`/
   `SOLAR_OUT` net rows, ERC section (22 → 24 warnings), verification tables (wired
   counts, crossing check, per-text clearances), library conventions, deferred items.

Both connectors are wired with **real wires**, not labels: J1 pin 1 taps the existing
`P` riser at (254.00, 40.64) and J1 pin 2 drops to an extended bottom `GND` rail via a
riser at x = 273.05; J2 pin 1 taps the existing `SOLAR_OUT` riser at (260.35, 96.52) and
J2 pin 2 drops via a riser at x = 266.70. The connectors are staggered 6.35 mm in x so
J1's `GND` riser passes through the 3.81 mm lane between J2's pin ends and J2's body —
that is what makes the revision crossing-free.

## Proof — grep on the committed `.kicad_sch`

```
$ grep -n 'Conn_01x02' hardware/solar-gate-v2/solar-gate-v2.kicad_sch
1775:		(symbol "Connector_Generic:Conn_01x02"      # lib_symbols cache
1791:			(property "Value" "Conn_01x02"
1844:			(symbol "Conn_01x02_1_1"
5961:		(lib_id "Connector_Generic:Conn_01x02")     # J2 instance
6040:		(lib_id "Connector_Generic:Conn_01x02")     # J1 instance
```

J1/J2 pin-to-net, from the wires that terminate on their pins (`278.13 = J1 pin 1`,
`271.78 = J2 pin 1`, pin 2 x-coords 273.05 / 266.70):

```
6192:			(xy 254 40.64) (xy 278.13 40.64)          # J1.1 -> P  (T-tap at 254,40.64)
6172:			(xy 278.13 43.18) (xy 273.05 43.18)       # J1.2 -> GND riser
6182:			(xy 273.05 43.18) (xy 273.05 147.32)      #   riser down to the GND rail
6120:			(xy 226.06 147.32) (xy 266.7 147.32)      # GND rail extension east
6130:			(xy 266.7 147.32) (xy 273.05 147.32)      #   meets the J1.2 riser
6208:			(xy 260.35 96.52) (xy 271.78 96.52)       # J2.1 -> SOLAR_OUT (T-tap at 260.35,96.52)
6146:			(xy 271.78 99.06) (xy 266.7 99.06)        # J2.2 -> GND
6156:			(xy 266.7 99.06) (xy 266.7 147.32)        #   riser down to the GND rail
```

Junctions added where the taps/risers meet existing wires: `(254, 40.64)`,
`(260.35, 96.52)`, `(266.7, 147.32)`, plus `(226.06, 147.32)` which the rail extension
turned from a 2-endpoint corner into a 3-way. Counts on the committed file:
`grep -c '(wire'` = **125** (was 115), `grep -c '(junction'` = **43** (was 39),
`grep -c '(text '` = **9**.

The authoritative connectivity is the netlist, not the geometry:

```
Components (29):
  J1: SOLAR IN (Connector_JST:JST_GH_SM02B-GHS-TB_1x02-1MP_P1.25mm_Horizontal)
  J2: TO HELTEC (Connector_JST:JST_GH_SM02B-GHS-TB_1x02-1MP_P1.25mm_Horizontal)
  ... (the original 27, unchanged: C1 C2 D1 D2 DA DB NTC Q1A Q1B Q2A Q2B Q3
       R1 R2 Rb Rb1 Rb2 Rbias Rfa Rfb Rg Rpua Rpub Rpuc Rt U1 U2)

Nets (14):
  /C: DA/2, DB/2, Rb/1, Rpuc/2
  /G: Q1A/1, Q1B/1, Q2A/1, Q2B/1, Q3/3, Rg/2
  /GND: C1/2, C2/2, J1/2, J2/2, NTC/2, Q3/1, R2/2, Rb2/2, U1/4, U2/3
  /H: D1/1, D2/1, Rg/1
  /M: Q1A/3, Q1B/3, Q2A/3, Q2B/3
  /N: NTC/1, Rfb/2, Rt/2, U1/2, U1/5
  /OA: DA/1, Rfa/1, Rpua/2, U1/1
  /OB: DB/1, Rfb/1, Rpub/2, U1/7
  /P: C1/1, D1/2, J1/1, Q1A/2, Q1B/2, Rbias/1, U1/8
  /PA: R1/2, R2/1, Rfa/2, U1/3
  /Q3B: Q3/2, Rb/2
  /SOLAR_OUT: D2/2, J2/1, Q2A/2, Q2B/2
  /VREF: C2/1, R1/1, Rb1/1, Rbias/2, Rpua/1, Rpub/1, Rpuc/1, Rt/1, U2/1, U2/2
  /Vb: Rb1/2, Rb2/1, U1/6
```

Unchanged membership, proven by diffing this against the 0074 netlist generated the same
way from a copy of `origin/hermes/0074-solar-gate-v2-wires`: 14 nets both sides, identical
net-name sets, **zero pins removed or altered**, and the only additions are `J1/1` on `P`,
`J1/2` and `J2/2` on `GND`, `J2/1` on `SOLAR_OUT`. (Pin names in the README table vs
numbers here: `Q1a/S` = `Q1A/2`, `Q3/C` = `Q3/3`, `DA/A` = `DA/2`, `U2/K` = `U2/1`,
`U2/REF` = `U2/2`, `U1/A-IN−` = `U1/2`, etc. — same pins, two label conventions.)

## Proof — ERC

```
** ERC messages: 24  Errors 0  Warnings 24
```

`kicad-cli sch erc --output hardware/solar-gate-v2/solar-gate-v2-erc.txt
--severity-all --format report hardware/solar-gate-v2/solar-gate-v2.kicad_sch`
(Flatpak KiCad 10, via `~/.local/bin/kicad-cli-10`). **All 24 warnings are the single
rule `[lib_symbol_mismatch]`**, on `R`/`C`/`D_Schottky`/`Thermistor_NTC`/`PWR_FLAG` and
now `Conn_01x02`: the MCP loader embeds host RPM KiCad 9.0.7 library copies while ERC
compares the Flatpak KiCad 10 copies. 22 of those warnings are the ones 0074 reported;
the two new ones are J1 and J2 — the connector symbol inherits the existing skew, and no
new class of warning appeared. No pin, net or connection is affected, and the project's
own symbol (TL431) and the comparator/transistor libraries raise none. Fixing it is a
server-env change (`KICAD_SYMBOL_DIR`), deliberately out of this task's scope.

## Proof — renders

`hardware/solar-gate-v2/solar-gate-v2-schematic.{svg,png,pdf}` on the branch, all
re-exported from the committed `.kicad_sch` (md5 `660c275358a8541b2f06e4270654bcc1`, i.e.
the same bytes that were measured and netlisted). The PNG is 7016x4961 (600 dpi A4
landscape) — the 0074 recipe.

## How the drawing was verified

Not by eye. The committed `kicad-cli sch export svg` output was parsed and every string's
ink box measured from its glyph stroke paths (same method that produced the 0074 numbers):

| Check | 0074 | 0075 |
|-------|------|------|
| Wire-wire crossings (strict interior-interior) | 0 | **0** |
| Collinear wire overlaps | 0 | **0** |
| Text-vs-text / text-vs-wire / text-vs-symbol-body overlaps | 0/0/0 | **0/0/0** |
| Text or symbol crossing the frame's inner border | 0 | 0 |

Smallest clearance from any of the 9 new strings to any other ink is **0.74 mm** (the two
long right-justified notes to the frame line at x = 285); smallest to another string is
3.94 mm; smallest to a wire is 5.54 mm. Full per-string table is in the README. The em
dashes render as real bars, not missing-glyph boxes (measured glyph geometry; a delegated
vision pass independently re-read the strings off the raster but its transcription was
partly unreliable, so the geometric measurement is what I am standing behind).

## Where the deliverable lives

`hardware/solar-gate-v2/` is **not on `main`** — it exists only on this branch and its
0074 parent (checked: `git ls-tree -r --name-only main hardware/` lists
`battery-pcb`, `busbar`, `hello-kicad`, `photos`, `solar-gate-v1`, and no
`solar-gate-v2`, which is also why the directory shows as untracked in a main worktree).
That is inherited state, not something this task changed: the 0074 revision Juno verified
and archived is still branch-only too. **The branch tip is therefore the deliverable**
and `files:` above names paths *on the branch*.

## CI

`gh run list -R pagosacabin/nordtronics --branch hermes/0075-solar-gate-v2-io-labels`
returns **no runs**, and that is a structural fact rather than a missing result: the repo
has exactly three workflows and none of them can fire for this branch. `website-check.yml`
triggers on `push` to `branches: [main, hermes/0068-site-email-refresh,
hermes/0070-site-rewrite]` only; `android-build.yml` on `push` to
`[android-toolchain-setup]` only; `platformio.yml` on `paths: firmware/tank-monitor/**,
firmware/node-v1/**` (and its own workflow file). No workflow anywhere in the repo names
`hardware/**` — verified by `grep -ln hardware .github/workflows/*.yml`, which matches
nothing — so a hardware-only branch cannot produce a run, and there is **no ntfy receipt**
either (that topic is for compiled companion builds, not a schematic). 0074 landed on the
same basis. The green run cited in `main_transition_run` is the pickup commit's
`Website Check`, verified `conclusion: success`, `headSha ==
bc74f0bf9ec8208ffd0e6a66e23bdb0918507273` == current `main`.

## Deviations and corrections (declared, not hidden)

1. **Three existing wire entries were split; no existing wire changed shape.** Adding a
   T-tap requires a vertex at the tap, so the `P` riser (254.00, 30.48 → 90.17) and the
   `SOLAR_OUT` riser (260.35, 60.96 → 111.76) each became two collinear segments meeting
   at the tap, and the bottom `GND` rail gained an appended segment rather than being
   edited. A segment-level diff of the two revisions shows exactly 2 removed / 12 added
   segments, the removed pair being the two risers, and both replaced by halves that span
   the identical coordinates. No component value, reference or net name was touched.
2. **Symbol choice differs from the suggestion in the spec.** The task offered
   `Connector:Conn_02x2_OddEven` "or equivalent"; a `Conn_02x2` is a 4-pin dual row,
   whereas both connectors here are 2-pin, so `Connector_Generic:Conn_01x02` (stock
   library, two pins, pins pointing left off the sheet edge) was used with the JST GH
   1.25 mm 2-pin footprint.
3. **`(wire`/`(junction`/`(text` counts are token-boundary greps.** KiCad 10 writes
   `(wire\n`, so a `grep '(wire '` with a trailing space returns 0 on a fully wired sheet;
   the numbers above use `grep -c '(wire'`.
4. **Nine free-form texts, not four.** The success criteria say "the four text
   labels/notes above"; the file carries **9** — the two connector notes (task item 1),
   the NTC note (item 2) and the six block annotations (item 3), all of them required by
   the task body. Nothing extra was invented beyond the spec's own list.

## Not done (out of scope, stated so it is not read as an omission)

No PCB layout, no footprint assignment for the passives/diodes/NTC (J1/J2 are the
exception), no `fp-lib-table`, no bench validation. The sheet is still schematic-only.
