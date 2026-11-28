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

# 0280 -- implementation run (Stephen picked WB 0.00, 2026-11-27 20:53 MST)

| file | what it is |
|---|---|
| `0280-implement-branch.diff` | the shipped change: `WB_STRENGTH 0.5f` -> `0.0f` + comment, on `hermes/0280-warm-colour` (off 0261 `b5c0f52`, tip `cdb7f62`) |
| `firmware-sha256.txt` | sha256 of the deliverable `firmware.bin` (`c7d3cf35...`) |
| `verify-flash-deliverable.log` | `esptool verify_flash 0x10000 firmware.bin` -> `verify OK (digest matched)` on the board |
| `fresh-shoot-console.txt` | fresh shoot on the **deliverable** build: gains, mirror lines, shot line |
| `post-restore-console.txt` | the same after the control flash was reverted: digest verified, then boot + shoot (strength 0.00) |
| `measurement-build.diff` | deliverable -> the measurement copy that was flashed to dump the raster (adds `j`; no knob, no behaviour change) |
| `fresh-shoot-raster-log.txt`, `fresh-shoot-raster.png`, `fresh-shoot-raster.bin.gz` | the fresh-shoot raster (SHA-matched both sides), rendered through the same 0250 ladder |
| `control-wb050-now-log.txt` | control: the A/B base build (WB 0.50, sat -1) shot at 21:10 MST -- also black, so the darkness is the scene |
| `stats-with-final.json` | `conv.py` stats for the fresh shoot and the A/B base, same metric definitions |

Result: the change is implemented, flashed and verified on the board, and the shoot
line reports `wb r 1.000 g 1.000 b 1.000 (strength 0.00)` with the 0261 mirror fix
intact (`hmirror 1` at power-on -> `0` after fix). The **visual** half of criterion 2
is not demonstrable tonight: at 21:06 MST the fixed mount saw an unlit room -- mean
panel level 0.062/0.062/0.062 versus 2.244/0.981/1.018 in the A/B session, only two
distinct byte values across 307,200 pixels, channels identical -- so there is no
colour in the frame to judge. The WB-0.50 control shot at 21:10 is equally black,
which rules out the change as the cause.

