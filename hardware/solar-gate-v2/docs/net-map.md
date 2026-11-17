# Net map — solar gate Mark 1 Rev 1

Generated from `solar-gate-v2.kicad_sch` (`kicad-cli sch export netlist --format kicadxml`)
and checked net-by-net against the previous capture
(`origin/hermes/0075-solar-gate-v2-io-labels` @ `397f17d`): same 14 nets, same member sets,
after the documented pin remap (FQP27P06 G/D/S = 1/2/3 vs AO3401 G/S/D = 1/2/3, and the
TL431LP REF/A/K numbering).

## Nets

| Net | Pins | Members |
|---|---|---|
| `/P` | 7 | C1.1, D1.2, J1.1, Q1.3, Q2.3, R3.1, U1.8 |
| `/M` | 4 | Q1.2, Q2.2, Q3.2, Q4.2 |
| `/SOLAR_OUT` | 4 | D2.2, J2.1, Q3.3, Q4.3 |
| `/G` | 6 | Q1.1, Q2.1, Q3.1, Q4.1, Q5.3, R13.2 |
| `/H` | 3 | D1.1, D2.1, R13.1 |
| `/C` | 4 | D3.2, D4.2, R11.2, R12.1 |
| `/Q5B` | 2 | Q5.2, R12.2 |
| `/VREF` | 10 | C2.1, R1.1, R10.1, R11.1, R3.2, R4.1, R5.1, R9.1, U2.1, U2.3 |
| `/N` | 5 | R4.2, R8.2, TH1.1, U1.2, U1.5 |
| `/PA` | 4 | R1.2, R2.1, R7.2, U1.3 |
| `/Vb` | 3 | R5.2, R6.1, U1.6 |
| `/OA` | 4 | D3.1, R7.1, R9.2, U1.1 |
| `/OB` | 4 | D4.1, R10.2, R8.1, U1.7 |
| `/GND` | 10 | C1.2, C2.2, J1.2, J2.2, Q5.1, R2.2, R6.2, TH1.2, U1.4, U2.2 |

`M` is the common-drain node between the two back-to-back pairs: `P` → Q1/Q2 sources →
`M` → Q3/Q4 sources → `SOLAR_OUT`. With the FETs off, neither body diode can conduct, so
`P` cannot beep to `SOLAR_OUT` in either direction (checked on the bench 2026-11-16).

## Bench name → designator

References were renumbered to the KiCad convention (designator must end in digits, or the
annotation check and BOM flag them as unannotated). Nothing electrical changed; the net name
`Q3B` followed its transistor and is now `Q5B`.

| As-built / 0075 name | Designator | Part |
|---|---|---|
| Rbias | R3 | 1.6 k |
| Rt | R4 | 34.0 k |
| Rb1 | R5 | 200 k |
| Rb2 | R6 | 23.2 k |
| Rfa | R7 | 1.5 M |
| Rfb | R8 | 1.5 M |
| Rpua | R9 | 100 k |
| Rpub | R10 | 100 k |
| Rpuc | R11 | 100 k |
| Rb | R12 | 10 k |
| Rg | R13 | 100 k |
| Q1A / Q1B | Q1 / Q2 | FQP27P06 (pair 1, sources at `P`) |
| Q2A / Q2B | Q3 / Q4 | FQP27P06 (pair 2, sources at `SOLAR_OUT`) |
| Q3 | Q5 | 2N3904 |
| DA / DB | D3 / D4 | BAT85 (OR-ing the two comparator outputs) |
| NTC | TH1 | 10 k B3950, off-board on pads (net `N`) |
| D1 / D2 | D1 / D2 | BAT85 (gate clamp to `P` / `SOLAR_OUT`) |

`J1` = `SOLAR IN` (`P`, `GND`), `J2` = `TO HELTEC` (`SOLAR_OUT`, `GND`), `U1` = LM393,
`U2` = TL431, `C1`/`C2` = 100 nF.
