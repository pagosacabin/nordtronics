# Wildfire Node v1 — Power Budget v1 (datasheet-derived)

**Task:** 0092 · **Status:** v1, paper analysis only — no firmware, hardware, app or backend change.
**Question it answers:** does the 12-minute packet cadence close, on paper, before anything is bought?

This is arithmetic on published datasheets. Every current × duration number below either cites a
datasheet section or is flagged **[assumption]**. Nothing here is a measurement: the node has not
been instrumented, and where a number is a bench impression it is labelled as such and marked
lower-confidence. The point of the exercise is to have a defensible budget that a bench run can
then confirm or falsify — not to produce a comfortable number.

---

## 1. What is being budgeted

Per the Rev C schematic (`hardware/wildfire-node-v1/`, task 0079 branch), the node is:

> LiPo cell → F1 resettable fuse → **Adafruit MiniBoost 5 V** boost (U5, TPS61023, EN pin gated)
> → `5V2` rail → PMS5003 (U2). The **Heltec HTIT-WB32LAF V4.2** (U1, ESP32-S3R2 + SX1262 US915)
> is the hub. AS3935 (U3) and BME688 (U4) sit behind a switched `Vext` rail (Q7 + R21/R22 gate).
> A temp-gate block (A1) sits between the solar panel input (`SOLAR_IN`) and `SOLAR_OUT`.

Two structural facts drive the whole budget:

1. **The particulate sensor is the load.** The PMS5003 needs its fan and laser running for a
   warm-up period before any reading is trustworthy. That duty cycle, repeated every packet, is
   larger than every other line in the node combined.
2. **Everything loud is already switched.** The boost is EN-gated, `Vext` is switched, and the
   MCU deep-sleeps between wakes. The remaining floor is a handful of microamps.

Battery: **3.7 V 3000 mAh li-ion, single cell or 2P (6000 mAh)** (Stephen, 2026-10-02 — the cells
are confirmed; paralleling two is allowed if the budget needs it).
Bench solar panel: **13 W / 5 V**.

---

## 2. Sources

Every citation below is a manufacturer document. Vendor marketing pages were not used as sources.

| # | Source | Used for |
|---|---|---|
| S1 | **Plantower PMS5003 series data manual, V2.3**, 2016-06-01 — "Technical Index" table and "Circuit Attentions" §4. Copy: <https://www.aqmd.gov/docs/default-source/aq-spec/resources-page/plantower-pms5003-manual_v2-3.pdf> (Plantower publishes no stable direct PDF link; this is the unmodified manual as hosted by South Coast AQMD). | PMS5003 supply 5.0 V typ, **Active Current ≤100 mA**, Standby ≤200 µA, Total Response Time ≤10 s, and the 30 s stability requirement |
| S2 | **Bosch BME688 datasheet, BST-BME688-DS000**, Table 2 "Gas sensor parameter specification". <https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme688-ds000.pdf> | Heater supply current during heater operation at 320 °C: **12 mA typ / 13 mA max** |
| S3 | (same family, for comparison) **BME680 datasheet BST-BME680-DS001**, Table 2: 9 / **12** / 13 mA at the same condition | confirms the BME688 figure is the family value, not a transcription |
| S4 | **ScioSense AS3935 Franklin Lightning Sensor datasheet v2**, Electrical Characteristics: `I_LSMRON` **70 µA** (listening, VREG on), `I_LSMROFF` 60–80 µA (VREG off), `I_PWDROFF` 1–2 µA. Copy: <https://cdn.sparkfun.com/assets/learn_tutorials/9/2/1/AS3935_Datasheet_EN_v2.pdf> | AS3935 continuous listening current |
| S5 | **Semtech SX1261/2 datasheet DS_SX1261-2 V1.1**, `IDDTX` table (SX1262, 915 MHz): **+22 dBm → 118 mA** (90 mA @ +14 dBm). Mirror: <https://www.mouser.com/datasheet/2/761/DS_SX1261-2_V1.1-1307803.pdf>. Independently corroborated in a module datasheet built on the same silicon: eRIC-SX1262-HCI, "TX supply current … 118 mA @ +22 dBm", <https://www.mouser.com/datasheet/2/743/LPRS_11172023_eRIC_SX1262_HCI_DS_v1-3367738.pdf> | LoRa TX supply current |
| S6 | **Texas Instruments TPS61023 datasheet** — Electrical Characteristics: `I_Q` into VOUT **20 µA typ / 30 µA max** (enabled, no load, no switching), `I_Q` into VIN 0.9/3.0 µA, `I_SD` shutdown 0.1 µA. <https://www.ti.com/lit/ds/symlink/tps61023.pdf> | boost quiescent when left enabled |
| S7 | **Adafruit MiniBoost 5 V @ 1 A (product 4654)** — TPS61023-based, regulated output 5.2 V, "true disconnect" EN. <https://www.adafruit.com/product/4654> | the actual module on the Rev C board (Vout 5.2 V, EN-gated) |
| S8 | **Espressif ESP32-S3 series datasheet**, Table 21 "Current consumption depending on work modes": Light-sleep 240 µA; Deep-sleep (RTC memory + RTC peripherals powered) typ **7 µA** — with the datasheet's own footnote that Table 21 applies to parts *with no SiP flash or SiP PSRAM*. <https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf> | MCU sleep floor (see §3: the footnote matters here) |
| S9 | **PVGIS 5.2** (radiation DB PVGIS-NSRDB, meteo ERA5, 2005–2015), 37.2694 N / 107.0097 W, tilt 37°, azimuth 0° (south), free-standing, **0 % system loss**: December `E_d` = **3.83 kWh/kWp/day**, in-plane `H(i)_d` = **3.88 kWh/m²/day**, monthly SD 18.19. Reproduce: <https://re.jrc.ec.europa.eu/api/v5_2/PVcalc?lat=37.2694&lon=-107.0097&peakpower=1&loss=0&angle=37&aspect=0&outputformat=json&mountingplace=free> | December in-plane insolation at the site |
| S10 | Rev C schematic + `hardware/wildfire-node-v1/README.md` (task 0079 branch) | what exists on the board and which rails are switched |
| S11 | `docs/wildfire/detection-logic-spec-v0.1-review-notes.md`, §A.5 latency budget table | the cadence the budget must support |

---

## 3. Assumptions register

Anything not in a datasheet is here, stated as an assumption, with what it costs if it is wrong.

| ID | Assumption | Value | Rationale / risk if wrong |
|---|---|---|---|
| A1 | PMS5003 **active** current | **100 mA** @ 5.0 V | Datasheet *maximum* (S1), used deliberately as the design number. Bench reports on this class of sensor range 50–100 mA; the bench note for this project says ~30 s to trustworthy but does not record a current. Using the datasheet max is the conservative choice and it is the single largest line in the budget — if the real average is 80 mA the daily total falls by ~20 %. |
| A2 | PMS5003 **sample window** after warm-up | **10 s** @ 100 mA | S1 gives "Total Response Time ≤10 s"; once past the 30 s stability point (S1 §4) the reading can be taken inside that window. Held as a second block of 100 mA fan/laser time rather than assuming the reading is free. If the sample is taken inside the 30 s warm-up instead, the PMS line drops from 212 to 159 mAh/day (§6 sensitivity). |
| A3 | Boost efficiency, 3.7 V → 5.0 V @ 100 mA | **85 %** | TPS61023 (S6) is a high-efficiency boost; at 0.5 W out from 3.7 V in the datasheet curves sit in the mid-to-high 80s. 85 % is a deliberately round, mildly conservative figure. 80 % would add 13 mAh/day; 90 % would remove 12 mAh/day. |
| A4 | Regulated `5V2` rail voltage for the arithmetic | **5.0 V** | The PMS5003 spec is 5.0 V typ (4.5–5.5 V) (S1); the Adafruit module actually regulates to 5.2 V (S7). Using 5.2 V adds 4 % to the PMS line (219.3 → 227.8 mAh/day). Kept at 5.0 V in the base case so the head line matches the sensor datasheet. |
| A5 | BME688 gas heater duration per reading | **150 ms** at 320 °C | **[unverified]** — the datasheet gives the heater *current* (S2), not a recommended plateau for our use; 150 ms is a short forced-mode gas sub-measurement. This line is 0.05 mAh/day either way; even a 4 s plateau would only reach ~1.4 mAh/day. It is included precisely so a missing sensor cannot hide in the total. |
| A6 | AS3935 interrupt service cost | MCU wake, **0.5 s @ 30 mA** @ 3.3 V per event | Event count, not per-event cost, is what moves this line (see A7). |
| A7 | Lightning **event rate** | quiet day **20/day**; storm day **500/day** | The AS3935 is notorious for disturbers (light pulses, other discharges), so zero is not a design value — the task's warning is correct. Both cases are costed; the storm case adds 1.9 mAh/day for readout-only, 5.6 mAh/day if every event also transmits. Masking disturbers in the AS3935 (`MASK_DIST`) is the firmware lever that collapses this line. |
| A8 | ESP32-S3 active window per wake | **4 s @ 25 mA** @ 3.3 V | Covers sensor readout, packet assembly and the radio hand-off. ESP32-S3 active current at low clock is tens of mA; 25 mA is a mid-range figure for a mostly-idle active window. |
| A9 | LoRa airtime | SF9, BW125 kHz, CR 4/5, preamble 8, **24 B payload**, CRC on, explicit header → **ToA 205.8 ms** (computed with the Semtech formula, not quoted) | US915 400 ms/channel dwell limit is respected. SF12 with the same payload is 1482.8 ms — 7.2× the airtime, so the spreading factor choice matters at the margins but not at the total (§6). Payload = PM2.5 + T + RH + Vbat + sequence, which matches the packet contents in the detection spec §A.1. |
| A10 | TX window overhead | **+50 ms** on top of ToA (window: 255.8 ms total) | Radio ramp + SPI + PA settle. Small either way. |
| A11 | TX current | **118 mA** (S5, +22 dBm) | Per the task, a conservative figure is also costed: 140 mA raises this line from 0.90 to 1.07 mAh/day. Both totals are quoted in §6 so nobody later "finds" phantom margin. |
| A12 | Board-level deep sleep | **30 µA** @ 3.7 V, 24 h | **S8's own footnote disqualifies the 7 µA figure for this part**: the node's module is the ESP32-S3**R2** with 2 MB of in-package PSRAM, and Table 21 is explicitly for parts with no SiP flash/PSRAM. On top of the die, the Heltec board carries an LDO and a USB-UART bridge. 30 µA is a **[assumption]** for a well-managed board; sibling Heltec V3-class boards have been reported at hundreds of µA to ~1.4 mA when the UART bridge is not held in reset, which would add 0.7–1.1 mAh/day per 30 µA and up to ~30 mAh/day in the bad case. **This is the line most likely to be wrong and the cheapest to measure.** |
| A13 | Battery-sense divider | 2 × 100 kΩ across VBAT → **18.5 µA** | **[assumption]** — Rev C does not enumerate a divider; the Heltec module has an on-board battery ADC divider. If the divider is 10× higher impedance the line vanishes; if it is 10× lower it becomes 4.4 mAh/day. Worth confirming against the module schematic. |
| A14 | Solar temp-gate (A1) quiescent | **20 µA**, 24 h | **[assumption]** — the solar-gate-v2 block's quiescent current is not in this repo. It must monitor temperature to decide when to gate, so it is not zero. To be confirmed against the solar-gate-v2 board. |
| A15 | MiniBoost left enabled between wakes | **no** (gated off) | The Rev C has R2 (4.7 kΩ) as an EN pull-down, and the TPS61023 is a "true disconnect" part (S7). Leaving EN high 24/7 with no load costs 20 µA → +0.48 mAh/day (§6). Note for the firmware/interface review: a pull-down only *disables* the boost — the MCU must actively drive EN high to bring `5V2` up at all. |
| A16 | Charge-path efficiency, panel → cell | **75 %** | **[assumption]** — Rev C shows no charge controller; the panel is 5 V and the cell is 3.7 V, so either a linear charger or a buck is in the path. A 5 V-to-3.7 V linear charger is worth ~70–75 %; a buck would be better. This is the weakest link in the solar number and is stated as such. |
| A17 | Snow / soiling derating, Pagosa December | **×0.75** (−25 %) | December in the San Juans means mornings with snow or ice on an unheated, low-mounted panel, plus low sun angles that make any soiling worse. A 25 % monthly-average haircut is a standard conservative allowance for a small unmounted panel at this latitude and 2 163 m elevation; it is not a measurement. |
| A18 | Usable capacity derating | **×0.90** of nameplate | Discharging a 3.7 V nominal cell to a 3.0–3.2 V cut-off rather than to 2.5 V; also protects cycle life. A "3000 mAh" cell is a 25 °C, low-rate figure. |
| A19 | Cold derating | **×0.85** (−15 %) | Li-ion gives back less capacity and more internal resistance below freezing; Pagosa winter nights sit at or under 0 °C for hours. Applied on top of A18 — both deratings are multiplicative and both are labelled, not buried: 3000 → 2295 mAh usable. |

---

## 4. Per-wake energy, one 12-minute cycle

Periodic loads only. Continuous loads are in §5 because they do not scale with cadence.

| Load | I | V | t | Load energy | Path | Battery-referred (3.7 V) |
|---|---|---|---|---|---|---|
| PMS5003 **warm-up** (fan + laser) | 100 mA | 5.0 V | 30 s | 4.167 mWh | ÷ 0.85 boost (A3) | **1.325 mAh** |
| PMS5003 **sample** | 100 mA | 5.0 V | 10 s | 1.389 mWh | ÷ 0.85 boost | **0.442 mAh** |
| BME688 gas heater (TPHG forced mode) | 12 mA | 3.3 V | 150 ms | 0.0017 mWh | — | 0.00045 mAh |
| ESP32-S3 active window | 25 mA | 3.3 V | 4 s | 0.0917 mWh | — | 0.0248 mAh |
| SX1262 LoRa TX (SF9, 24 B) | 118 mA | 3.3 V | 255.8 ms (incl. A10) | 0.0277 mWh | — | 0.0075 mAh |
| **Subtotal** | | | | | | **≈ 1.80 mAh / wake** |

The three biggest contributors to a wake are, in order: **PMS5003 warm-up (74 %),
PMS5003 sample (25 %), ESP32-S3 active window (1.4 %)**. Everything else is a round-off in
comparison. That ordering is the whole finding of this document.

---

## 5. Daily total at 12-minute cadence

120 wakes/day.

| Line | Source | mAh/day |
|---|---|---|
| PMS5003 (40 s × 100 mA @ 5 V, boost-corrected) | S1, A1–A4 | **211.98** |
| ESP32-S3 active windows (120 × 4 s @ 25 mA) | A8 | 2.97 |
| AS3935 listening, 24 h continuous (70 µA) | S4 | 1.68 |
| LoRa TX (120 × 255.8 ms @ 118 mA) | S5, A9–A11 | 0.90 |
| ESP32-S3 + board deep sleep, 24 h (30 µA) | S8, A12 | 0.72 |
| Solar temp-gate quiescent, 24 h (20 µA) | A14 | 0.48 |
| Battery-sense divider, 24 h (18.5 µA) | A13 | 0.44 |
| BME688 heater (120 × 150 ms @ 12 mA) | S2, A5 | 0.05 |
| AS3935 interrupt service (20 events/day) | A6, A7 | 0.07 |
| SX1262 sleep (0.6 µA) | — | 0.01 |
| MiniBoost quiescent | A15: gated off | 0.00 |
| **Total** | | **≈ 219.3 mAh/day** |

```
per_wake_mah: 1.77 (PMS5003 warm-up 1.33, PMS5003 sample 0.44, ESP32-S3 active window 0.02)
per_day_mah 12-min: 219.3
```

**Cross-check against Juno's independent pass (~220 mAh/day):** my total is 219.3, i.e. **within
0.3 %** of hers. No reconciliation is owed — but the *composition* differs from a naive reading of
her note, and that is worth recording: essentially the entire total (97 %) is the particulate
sensor, and the MCU/radio/always-on composite is only ~7 mAh/day. If the number is wrong, it is
wrong in the PMS line (see A1/A2/A3), not in the MCU arithmetic. Juno's derived autonomy
(≈11–12 days single, ≈23 days 2P) is within 10 % of mine (§7); the difference is the derating
stack, which I have made explicit rather than implicit.

### 5.1 Reconciling the superseded draft's 92 mAh/day

The earlier draft's 92 mAh/day is exactly reproduced by:

```
80 mA × (30 s / 3600) × 120 wakes × (5.0 V / 3.7 V) × 0.85 = 91.9 mAh/day
```

which means that line contained **three** errors, not one:

1. **The boost ratio was applied, then the efficiency was applied in the wrong direction.**
   A converter efficiency *divides* the battery-side current; multiplying by 0.85 makes the
   conversion loss *negative*. Correct treatment alone moves 108 → 127 mAh/day.
2. **The 30 s sample was omitted entirely** — the reading is taken *after* the stability point,
   not inside it. That is +10 s of 100 mA per wake.
3. **80 mA was used instead of the datasheet's ≤100 mA** (S1). The bench "50–80 mA" impression is
   real but it is an impression; the design number has to be the datasheet maximum.

Corrected: **100 mA × 40 s × 120 × (5/3.7) ÷ 0.85 = 211.98 mAh/day** — **2.3× the draft**, which is
why the draft's downstream autonomy figure was optimistic by the same factor. The task's "~40 %"
figure for the blanket factor alone is the same error in kind: 100 mA × 40 s × 120 = **133 mAh/day
referred to the 5 V rail**, and treating that as cell current — which is exactly what a blanket
percentage allowance on the 5 V number does — understates the correct 212 mAh/day by **37 %**.
The specific 92 mAh/day in the draft carries the two additional errors above on top of it.

---

## 6. Sensitivity

Base case is the §5 total. One variable at a time.

| Variation | Daily total | Δ |
|---|---|---|
| **Base (12-min cadence)** | **219.3 mAh/day** | — |
| Boost efficiency 85 % → 80 % | 232.6 | +13.3 |
| Boost efficiency 85 % → 90 % (not claimed) | 207.5 | −11.8 |
| `5V2` at the module's real 5.2 V (A4) | 227.8 | +8.5 |
| TX current 118 → 140 mA (conservative, A11) | 219.5 | +0.2 |
| Sample window folded into the 30 s warm-up (A2) | 166.3 | −53.0 |
| PMS active current 100 → 80 mA (A1) | 176.9 | −42.4 |
| Boost EN left high 24/7 (A15) | 219.8 | +0.5 |
| AS3935 storm day, 500 events, readout only | 221.1 | +1.8 |
| AS3935 storm day, 500 events, readout + TX each | 224.8 | +5.5 |
| AS3935 disturbers masked in firmware (`MASK_DIST`) | 219.3 | −0.07 |
| **Conservative stack** (80 % eff, 5.2 V, 140 mA, 8 s MCU window, 40 µA sleep) | **245.0 mAh/day** | +25.7 |
| Deep sleep at 1.4 mA (A12 failure mode, UART bridge live) | 252.2 | +32.9 |

Reading: the budget is **robust to every assumption except the PMS duty cycle and the deep-sleep
floor**. Both are measurable in an afternoon on the bench, and both should be measured before the
panel and cells are bought.

### 6.1 Cadence sensitivity (the input the detection spec needs)

| Cadence | Wakes/day | PMS line | MCU + TX | Continuous | **Total** | Zero-sun autonomy, single cell |
|---|---|---|---|---|---|---|
| 6 min | 240 | 424.0 | 7.7 | 3.41 | **435.2 mAh/day** | 5.3 days |
| **12 min** | **120** | **212.0** | **3.9** | **3.41** | **219.3 mAh/day** | **10.5 days** |
| 30 min | 48 | 84.8 | 1.6 | 3.41 | **89.8 mAh/day** | 25.6 days |
| 60 min | 24 | 42.4 | 0.8 | 3.41 | **46.6 mAh/day** | 49.2 days |

Latency cost, from the v0.1 rules as written (roll-up: first packet at +T, second-packet
confirmation at +2T, then the 20-minute inter-node window, +1 minute delivery — the arithmetic in
Review Note #1 of the detection spec, which found the published ≤25 min claim is really ~45 min):

| Cadence | Honest worst case (2T + 20 min + 1 min) | v0.1 published claim |
|---|---|---|
| 6 min | 33 min | ≤ 25 min (nominal / best-aligned only) |
| **12 min** | **45 min** | ≤ 25 min — **does not hold**, see Note #1 |
| 30 min | 81 min | — |
| 60 min | 141 min | — |

So the trade is explicit: **6 minutes buys ~12 minutes of worst-case latency for 2× the energy;
**30 minutes buys 2.4× the autonomy and pays 36 minutes of latency.**
because the *continuous* floor (3.4 mAh/day) is negligible — below ~30 minutes the budget is
almost entirely proportional to the wake count, so autonomy scales nearly as 1/cadence.

---

## 7. Battery sizing — zero solar input

Derating stack (both factors labelled, neither buried): **× 0.90 usable** (A18) **× 0.85 cold**
(A19) = 0.765 of nameplate.

| Configuration | Nameplate | Usable (0.90 × 0.85) | Days at 219.3 mAh/day |
|---|---|---|---|
| Single cell | 3 000 mAh | **2 295 mAh** | **10.5 days** |
| 2P | 6 000 mAh | **4 590 mAh** | **20.9 days** |

Reference points: Juno's pass gave ≈11–12 / ≈23 days; the 0093 website text currently says ≈11–12 /
≈23 days "pending Hermes's 0092 validation". My figures are **10.5 / 20.9** — 9 % and 9 % below,
entirely explained by making the two deratings explicit and multiplicative. At 30-minute cadence
the same stack gives **25.6 days single / 51.1 days 2P**.

Cold is doing real work here: without A19 the single-cell figure is 12.3 days, and a −25 % cold
derating (not unreasonable for a cell sitting at −10 °C overnight) would take it to 9.2 days.

---

## 8. Solar sizing — Pagosa Springs, December (worst month)

**Peak-sun-hours figure used: 3.88 h/day**, the December *in-plane* irradiation at the site from
PVGIS 5.2 (S9) — 37.2694 N, 107.0097 W, elevation 2 163 m, 37° tilt, due south, 0 % system loss.
`E_d` = 3.83 kWh/kWp/day and `H(i)_d` = 3.88 kWh/m²/day, monthly SD 18.19; the two agree, which is
the sanity check that the array geometry and the module model are consistent. This is in-plane, so
it converts directly to effective peak sun hours and needs no transposition factor.

**Snow/dirt derating: × 0.75** (A17), defended in one line: an unheated, low-mounted panel in the
San Juans spends December mornings under snow or ice, and low sun angles make any soiling worse —
a 25 % monthly-average haircut is the honest allowance, not a measured number.

```
Daily load energy        = 219.3 mAh × 3.7 V          = 0.812 Wh/day
Panel-referred energy    = 0.812 / 0.75 (charge path) = 1.082 Wh/day      [A16]
Effective peak sun hours = 3.88 × 0.75                = 2.91 h/day
Required panel power     = 1.082 / 2.91               = 0.372 W
```

**Verdict: 0.37 W required against the 13 W / 5 V bench panel — ~35× headroom.**

- **December headroom: 35×.** The shortfall is zero; the panel is oversized for a single node by
  more than an order of magnitude. At 30-minute cadence the requirement falls to 0.15 W.
- The honest caveat is not the panel's wattage but its **nameplate-to-real ratio** and the
  **charge path**, which Rev C does not show. A "13 W / 5 V" folding panel is a 2.6 A nameplate;
  real December output from a small outdoor panel at 5 V is typically a fraction of nameplate.
  Even at **1/10 of nameplate** (1.3 W) the design is energy-neutral at 12-minute cadence with
  ~3.5× to spare. The solar side is not the binding constraint and should not be sized further
  until the charge controller is chosen.
- Structural note: because the load is ~0.81 Wh/day and the panel is 13 W nameplate, **the system
  is battery-limited, not solar-limited.** Autonomy in a snowed-in week is governed by §7, not §8.

---

## 9. Verdict

**Yes — the 12-minute cadence closes, on paper, with margin.**

| Claim | Figure |
|---|---|
| Per-wake energy (12-min cycle) | **≈ 1.80 mAh**, of which 1.77 is the PMS5003 |
| Daily energy at 12-min | **219.3 mAh/day** |
| Per-day totals, 6 / 12 / 30 / 60 min | **435.2 / 219.3 / 89.8 / 46.6 mAh/day** |
| Autonomy, no sun — single 3 000 mAh | **10.5 days** |
| Autonomy, no sun — 2P 6 000 mAh | **20.9 days** |
| December solar required vs 13 W bench panel | **0.372 W vs 13 W → 35× headroom** |
| Honest detection latency at 12 min | **~45 min worst case**, not the published ≤25 min |

Margin statement: 10.5 days of zero-sun autonomy on a single cell, 20.9 days on 2P, and a
December solar requirement an order of magnitude smaller than the panel that is already on the
bench. The conservative stack (245 mAh/day, §6) still leaves **9.4 days / 18.7 days** — the
verdict does not depend on the optimistic end of any assumption. The one thing it *does* depend on
is A12 (the deep-sleep floor): at the 1.4 mA failure mode reported on sibling Heltec boards, the
the single-cell figure falls to ~9.1 days and December solar headroom to ~30×
margin is thinner than the headline.

**Cadence recommendation:** keep 12 minutes. It is not energy-limited at this load — it is
latency-led, and 12 minutes sits at a defensible point: 30 minutes would buy 2.6× the autonomy
(25.6 days single cell) at the cost of a ~81-minute honest worst case. That trade belongs in the
detection spec's next revision, which now has the numbers to make it.

**Latency honesty (cross-reference):** Review Note #1 of the v0.1 spec is correct — the published
≤25 min figure does not hold at any cadence on this list, because the 20-minute inter-node window
is additive to two packet intervals. The cadence table in §6.1 supplies the corrected column.

---

## 10. v2 opportunities (recorded, not required now)

1. **Deep-sleep the MCU through the PMS5003 warm-up** (the task's suggestion, costed honestly).
   The MCU-active line is only **2.97 mAh/day** at 12-minute cadence; halving it saves ~1.5 mAh/day,
   i.e. **0.7 %** of the total. Worth doing on principle (it is free to implement), but it is not a
   battery-life lever and should not be sold as one. The *real* version of this idea is to let the
   boost and the fan run while the ESP32-S3 sleeps in RTC-timer mode, which does keep the window
   short — the energy is in the fan, and the fan has to run for 30 s regardless.
2. **The only large lever is the fan duty cycle.** The PMS5003's SET-pin sleep mode draws ≤200 µA
   (S1) but the same manual requires ≥30 s of run time after each wake before data is stable, so a
   per-wake 30 s is structurally unavoidable at this cadence. Two real options, both for v2:
   sample *less often than you transmit* (transmit the last reading every 12 min, sample every
   30 min → 61 % of the PMS energy, at the cost of stale data in the packet), or accept a longer
   cadence (§6.1).
3. **Confirm/repair the deep-sleep floor (A12).** Holding the USB-UART bridge in reset and
   confirming the RTC/PSRAM retention mode is the single highest-value bench check in this budget:
   it is worth up to ~33 mAh/day (15 % of the total) and costs nothing to test.
4. **Mask disturbers in the AS3935** (`MASK_DIST`). The interrupt-service line is only 0.07 mAh/day
   at 20 events/day, but it is the line that a storm day or a noisy site moves, and the AS3935 is
   famous for false events — masking them is free insurance for the MCU-wakeup budget.
5. **Confirm A13 (battery divider) and A14 (temp-gate quiescent)** against the module and
   solar-gate-v2 schematics — together 0.92 mAh/day, and both are currently assumptions.

---

## 11. What this document does not prove

- No current was measured. Every "current" here is a datasheet number; every duration that is not
  a datasheet number is an assumption in §3.
- The warm-up behaviour is taken from the sensor manual's stability requirement (S1 §4, ≥30 s) and
  the bench note that agrees with it; the actual reading-to-reading stability of *this* sensor over
  temperature is untested.
- Boost efficiency (A3) and charge-path efficiency (A16) are stated round numbers, not curves read
  off the datasheets at our exact operating point.
- The December insolation figure is a 2005–2015 satellite/model average at a single grid point; the
  monthly SD is 18.19 kWh/m² (≈15 % of the mean), so an individual December can be materially worse
  than the average. The 35× headroom absorbs that; a marginal design would not.
- Vext switching, the Q7 gate stage and R21/R22 are unpopulated values (TBD in Rev C) — the switched
  rail's own leakage is not modelled.
