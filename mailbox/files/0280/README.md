# 0280 -- live-photo colour A/B evidence

| file | what it is |
|---|---|
| `ab-01-baseline-WB0.50-sat-1.png` | shipped 0261 colour: WB strength 0.50, saturation -1 |
| `ab-02-WB0.25-sat-1.png` | WB 0.25 |
| `ab-03-WB0.00-sat-1.png` | WB 0 (no grey-world correction) |
| `ab-04-WB0.50-sat0.png` | saturation 0 (driver neutral) |
| `ab-05-WB0.50-sat+1.png` | saturation +1 |
| `montage.jpg` | the five candidates side by side, labelled |
| `gains-and-stats.txt` | gains per shot, mean channel levels, R/B, pixel-difference noise floor |
| `stats.json` | the same numbers, machine-readable |
| `console-*.txt` | the camera console per variant, including the `shoot` line with the gains and the `raster BEGIN/END` SHA |
| `flash-*.log` | PlatformIO upload tail per variant |
| `raster-*.bin.gz` | the raw 307,200-byte panel raster each PNG was rendered from (gzip) |
| `bench-variant.diff` | the bench-only source edit (WB/SAT compile knobs + `j` raster dump) |
| `restore-0261-check.txt` | the stock 0261 build's boot + a local capture after it was flashed back |

Method: the camera board was flashed with a bench-only 0261 variant, one knob at a
time from the shipped combination, and for each variant the exact raster was dumped
over serial and hash-verified against the firmware's own SHA-256 (307,200 B, equal
every time). PNGs are that raster rendered through the measured 0250 DAC ladder and
sRGB -- they show the panel's intended signal, not a photograph of the panel.

Bench left as found: the stock 0261 build (hermes/0261-mirror-live-photo @ b5c0f525)
is back on the camera board and `restore-0261-check.txt` shows it booting, mirror fix
applied, strength 0.50.
