---
task_id: "0039"
status: staged
iteration: 1
proof:
  - branch: hermes/0039-solar-gate-route
    sha: ff28c10054fe4d100729a4a80588bee3e6e3c288
notes: |
  Status: done. Full EDA rung exercised through the KiCad MCP server
  (mixelpixx/KiCAD-MCP-Server 2.7.0) + Freerouting 2.4.1: project -> schematic
  -> ERC -> board -> placement -> autoroute -> DRC. 17 parts, 8 nets,
  2-layer 40x30 mm. ERC 0 errors / 0 warnings. DRC 2 errors / 0 warnings --
  both are Freerouting fanout stubs at 0.15 mm on net VA, below the declared
  0.20 mm minimum; reported, not hand-fixed. Autoroute: fanout escaped 39/39
  SMD pins, auto-routing stage final score 999.99 (0 unrouted, 0 violations).
  One part substitution/package correction (TLV3401 has no SC-70-5; used
  SOT-23-5), one required net completion (D1 REF was unlisted), one added
  connector (J4 telemetry out), and one spec inconsistency flagged
  (TLV431 VREF is 1.24 V, not 2.5 V, so the REF2V5/VTH targets do not hold).
  No CI run exists for this branch: android-build.yml is pinned to
  branch android-toolchain-setup and platformio.yml is path-filtered to
  firmware/**, so nothing matches hardware/**. Proof is branch + tip SHA only,
  deliberately with no run pointer. (gh CLI is not authenticated in this
  environment, so workflow triggers were verified by reading
  .github/workflows/*.yml rather than the Actions API.)
---

# 0039 — Solar gate v1: schematic + first autoroute shakedown

## Reply

**Status: done**

- **Branch:** `hermes/0039-solar-gate-route` @ `ff28c10054fe4d100729a4a80588bee3e6e3c288`
  (verified on origin: `git rev-parse origin/hermes/0039-solar-gate-route` returns the same SHA)
- **Project:** `hardware/solar-gate-v1/` — 16 files, incl. `README.md`, schematic, board,
  project symbol library, `sym-lib-table`, `fp-lib-table`, netlist, BOM, both SVGs,
  the DSN/SES router artifacts, `solar-gate-v1-erc.txt` and
  `solar-gate-v1_drc_violations.json`.
- **Test board only.** Nothing here is production-ready, nothing has run on a bench,
  no Gerbers/fab outputs were produced, and **Stephen still has to review both the
  schematic and the layout before anything real is built from it.**

---

### 1. Autoroute prerequisites

Nothing needed installing — both were already present:

```
$ java -version
openjdk version "25.0.1" 2025-10-21        # Freerouting needs 21+

$ ls -la ~/.kicad-mcp/freerouting.jar
-rw-r--r-- 1 astroboy astroboy 64076787 Sep 22 14:42 freerouting.jar
```

Via the server's own `check_freerouting`:

```json
{"java": {"found": true, "path": "/usr/sbin/java",
          "version": "openjdk version \"25.0.1\" 2025-10-21", "java_21_ok": true},
 "docker": {"available": false},
 "freerouting": {"jar_found": true, "jar_path": "/home/astroboy/.kicad-mcp/freerouting.jar"},
 "execution_mode": "direct", "ready": true}
```

Ran in `direct` mode. `freerouting.jar` reports `Freerouting v2.4.1 (build-date: 2026-09-03)`.

### 2. Pinout verification against the datasheets

| Part | Datasheet says | Verdict |
|---|---|---|
| **Q1 AO3401**, SOT-23 | 1 = GATE, 2 = SOURCE, 3 = DRAIN (AOS AO3401 Rev 6.1, Feb 2024; independently agreed by the JIANGSU Changjiang and ElecSuper AO3401 datasheets, and by KiCad's own `Transistor_FET:AO3401A` symbol, which extends `TP0610T` with 1=G, 2=S, 3=D) | spec correct, **no change** |
| **U1 TLV3401**, comparator | TI SLCS135B, SOT-23 (DBV) 5-pin: **1 = OUT, 2 = GND, 3 = IN+, 4 = IN−, 5 = VCC** | pin numbers as expected, **but package corrected** — see below |
| **D1 TLV431**, SOT-23-3 | TI SLVS139Z Table 4-1, DBZ (SOT-23-3): **1 = REF, 2 = CATHODE, 3 = ANODE** | no change needed, but see the trap below |

**Correction 1 — U1 is not available in SC-70-5.** The spec called for an SC-70-5
TLV3401. TI's Device Information table lists the TLV3401 only in **SOT-23 (5)**,
SOIC-8 and PDIP-8 — there is no SC-70-5 option. I kept the part number (TLV3401)
and used the **SOT-23-5** footprint. The 5 pins are the same five signals.

**Correction 2 — TLV431 vs TL431 pinout trap.** The TLV431 in DBZ is
1=REF / 2=CATHODE / 3=ANODE, which is the *opposite* of the ordinary TL431 in the
same DBZ package (1=CATHODE / 2=REF / 3=ANODE). Worth knowing before anyone
substitutes a TL431 into this footprint.

**Correction 3 — the net list omitted D1's REF pin.** `REF2V5` was specified as
`Rs / D1-cathode / R1-top / R2-top`, but a 3-terminal shunt reference does not
regulate at all with REF floating (ERC flagged it as an unconnected input). REF
must tie to CATHODE, so `D1/1 (REF)` is on `REF2V5` as well. This is a required
completion of the netlist, not a design change.

### 3. Schematic

Built entirely through MCP tools: `create_project` → `create_symbol` for the two
parts KiCad has no symbol for → `register_symbol_library` → `batch_add_components`
(19 symbols) → `add_schematic_net_label` snapped to pins (39 labels, 0 failures) →
`run_erc`. No KiCad file was hand-edited.

`generate_netlist` returns exactly the specified connectivity:

```
Nets (8):
  /CHG+:   J2/1, Q1/3
  /GATE:   Q1/1, R4/1, Rf/2, Rg/2, U1/1
  /GND:    C1/2, C2/2, D1/3, J1/2, J2/2, J4/2, R3/2, R5/2, TH1/2, U1/2
  /Panel+: J1/1, Q1/2, Rg/1, Rs/1, U1/5
  /REF2V5: C1/1, D1/1, D1/2, R1/1, R2/1, Rs/2
  /TELEM:  J4/1, R4/2, R5/1
  /VA:     C2/1, R1/2, Rf/1, TH1/1, U1/3
  /VTH:    R2/2, R3/1, U1/4
```

**Extra part, declared:** `J4` — a 2-pin connector for `TELEM`. The spec's
connector list was "Panel+ in, GND, CHG+ out", but `TELEM` is a net that has to
reach the MCU somehow, so it leaves via a 2-pin JST GH alongside the others.
Connectors used: J1 panel in, J2 charger out, TH1 off-board NTC, J4 telemetry.
No other parts added, no values changed.

TH1: the spec said "SH1.25 or pin header". I used a **JST GH 1.25 mm 2-pin**
footprint — JST SH is a 1.0 mm-pitch series, so GH is the 1.25 mm-pitch family
the spec was describing.

### 4. ERC — final output, verbatim

```
ERC result: 0 violation(s)
  Errors: 0  Warnings: 0  Info: 0
```

(Saved to `hardware/solar-gate-v1/solar-gate-v1-erc.txt`. `validate_schematic`
also passes: `valid: true`, `errorCount: 0`, and `kicad-cli` loads it with
`Successfully saved schematic file using the latest format`.)

Getting there took real fixes, not a lucky first pass: the initial run reported 21
violations (3 errors / 18 warnings). The errors were the two unconnected pins
above; the 18 warnings were all `Symbol '<x>' doesn't match copy in library` — see
the environment note at the end.

### 5. Board

- **2-layer, 40 × 30 mm**, 1 oz copper, all parts on the front. Netclass defaults:
  clearance 0.2 mm, track 0.25 mm, min track 0.2 mm, via 0.6/0.3 mm.
- **Passives 0805** (not 0603): this is a bench test vehicle whose whole point is
  being hand-modified, and 0805 is much easier to rework. Reported as required.
- Placement: power path along the top edge J1 → Q1 → J2 (Q1 sits between the two
  connectors); reference/divider passives in the middle bands; U1 next to VA/VTH;
  **TH1 at the left board edge** and J4 at the bottom edge, both for cable access.

**Widths on the 1 A path (state-required):** netclass `POWER_1A` = **0.80 mm** on
`Panel+`, `CHG+` and `GND`; everything else 0.20 mm. `GND` is in the high-current
class because it carries the 1 A return. By IPC-2221 (external layer, 1 oz, 10 °C
rise) 1 A needs **0.300 mm**; 0.80 mm is ≈2.03 A of capability and drops ~25 mV
over a 40 mm run. Verbatim from the routed board:

```
width 0.80 mm  net CHG+    x2
width 0.80 mm  net GND     x39
width 0.80 mm  net Panel+  x11
width 0.20 mm  net GATE    x13
width 0.20 mm  net REF2V5  x9
width 0.20 mm  net TELEM   x3
width 0.20 mm  net VA      x23
width 0.20 mm  net VTH     x11
width 0.15 mm  net VA      x2     <- the two DRC errors
```

### 6. Autoroute — router stats, verbatim

MCP tool result:

```json
{"message": "Autoroute completed in 9.1s", "mode": "direct", "elapsed_seconds": 9.1,
 "board_stats": {"tracks": 117, "vias": 11},
 "netClasses": {"applied": ["POWER_1A", "SIGNAL_0R25"], "assignments": 3}}
```

Freerouting's own log (`~/.local/state/freerouting/logs/freerouting.log`):

```
Fanout stage completed: started with 39 total SMD pins, completed in 1.48 seconds,
  escaped pins: 39/39 (100.0%)
Auto-routing stage started ... for 11 unrouted items.
Pass #1: 2 unrouted items; Pass #2: 1; Pass #3: 1; Pass #4: 1; Pass #5: 0
Auto-routing stage completed: started with 31 unrouted nets, completed in 5.69 seconds,
  final score: 999.99 (0 unrouted and 0 violations)
Optimization stage completed: ... final score: 999.99 (0 unrouted and 0 violations)
```

- **Routed: 100%** — 0 unrouted, 0 violations, final score 999.99.
- **Vias: 11** (117 track segments on the board).
- **Unrouted nets: 0.** Independently checked from the file, not from tool prose:
  all 8 of 8 nets carry routed copper, and the DRC report contains no
  `unconnected_items` entries.

### 7. DRC — full output, verbatim

```
Found 2 DRC violations
  {"total":2, "by_severity":{"error":2,"warning":0,"info":0},
   "by_type":{"track_width":2}}

1. [error] Track width (board setup constraints min width 0.2000 mm; actual 0.1500 mm) @ (15.9012, 11.801)
2. [error] Track width (board setup constraints min width 0.2000 mm; actual 0.1500 mm) @ (15.9012, 11.6008)
```

Both are Freerouting **fanout stubs on net `VA`** escaping R2's pads at 0.15 mm —
below the 0.20 mm minimum the board declares. That is the router's output; I did
**not** hand-fix or widen them, and I did not relax the design rule to make them
disappear. They are left visible for review.

### 8. Two earlier passes (what was fixed on the way, and why)

- **Pass 1** was discarded for genuine layout defects of mine: TH1 and J4 sat so
  close to the edges that their copper overhung the outline (3 × `copper_edge_clearance`
  errors) and J4's courtyard overlapped R5. Fixed by moving both inboard and
  re-spacing the telemetry divider.
- **Pass 2** exposed a real toolchain trap: `create_netclass` **persisted the class
  definition but assigned no nets to it** (`netclass_assignments` stayed `null`), so
  the whole 1 A path was handed to the router in the 0.2 mm default class —
  0.74 A of capability, not enough for 1 A. Fixed by calling `assign_net_to_class`
  per net and *verifying in the exported DSN* that the router really received it:

  ```
  (class POWER_1A CHG+ GND Panel+
    (use_via "Via[0-1]_800:400_um") (rule (width 800) (clearance 200)))
  (class kicad_default GATE REF2V5 TELEM VA VTH
    (use_via "Via[0-1]_600:300_um") (rule (width 200) (clearance 200)))
  ```

  A side effect worth noting: the `SIGNAL_0R25` class also never took, so the five
  signal nets fell back to the board `Default` class and routed at 0.20 mm rather
  than 0.25 mm. Harmless for signals, reported for completeness.

### 9. Environment findings (why ERC/DRC needed configuration, not just design work)

The machine runs **two different KiCads**: a host RPM 9.0.7 (which supplies
`pcbnew` to the MCP server's SWIG backend) and a Flatpak 10.0.6 whose `kicad-cli`
actually executes ERC and DRC. The Flatpak sandbox has its **own `/usr`**, so it
cannot see the host's `/usr/share/kicad` — which surfaced twice:

1. **Every symbol warned `doesn't match copy in library`.** The MCP symbol loader
   was embedding host KiCad 9.0.7 symbols (`version 20241209`) while the ERC engine
   compared them against its own KiCad 10 library (`version 20251024`; 35,898 diff
   lines between the two `Device.kicad_sym` files). Fixed by pointing the loader at
   the Flatpak's KiCad 10 symbol runtime via `KICAD_SYMBOL_DIR`; 18 warnings → 0.
2. **Every footprint reported `not enabled in the current configuration`.** Pointing
   the project `fp-lib-table` at `/usr/share/kicad/footprints` looks correct to the
   host toolchain and is invisible to the sandboxed one. Fixed by aiming the project
   URIs at the **Flatpak KiCad 10 footprint runtime**
   (`/var/lib/flatpak/runtime/.../footprints/<lib>.pretty`) — a host path *both*
   engines can read. 17 warnings → 0.

I verified each of those causally (hiding the file and re-running ERC reproduced the
warnings; restoring it cleared them), rather than assuming. Both fixes are recorded
in the project's `README.md` and `fp-lib-table` comments.

Two smaller toolchain defects found and worked around, worth reporting upstream:
`register_footprint_library` with `scope: "global"` writes to a **cwd-relative**
`kicad/<ver>/fp-lib-table` (it landed in `~/.hermes/hermes-agent/kicad/9.0/`, now
removed) instead of the user's real config; and a tool call issued immediately after
`initialize` fails with "Python process for KiCAD scripting is not running" until the
spawned Python backend is up — the project-local registration path worked fine.

### 10. Open items for Stephen's review

1. **The reference voltage does not add up, and it matters.** The spec calls D1 a
   "TLV431 shunt reference **2.5 V**" and names the node `REF2V5`, with `VTH` =
   1.25 V from the R2/R3 halving. But the TLV431's `VREF` is **1.24 V**, so with REF
   tied to CATHODE this node sits at ~1.24 V and `VTH` at ~0.62 V — roughly half the
   intended comparison threshold, which would move the gate's switch point a long way.
   Either the part should be a 2.5 V reference (TL431 family) or R2/R3 must be
   re-scaled. I drew the nets exactly as specified and did not silently "fix" the
   values, but this needs a decision before the board means anything.
2. Two unresolved `track_width` DRC errors (section 7), left as router output.
3. `J4` is an added connector (section 3), not in the spec's connector list.
4. `Rf` (270k, GATE → VA) is drawn per spec; with `Rg` (100k, Panel+ → GATE) it sets
   a hysteresis band that has not been calculated against the intended threshold.

Nothing was ordered, no fab outputs exist, and no power-board/high-current layout
work was done beyond this vehicle — those stay with Stephen.
