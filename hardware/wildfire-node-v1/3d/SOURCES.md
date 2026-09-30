# wildfire-node v1 — 3D STEP models (task 0081)

STEP geometry for the node v1 BOM, for the FreeCAD assembly (task 0080) and
later KiCad 3D views. 11 parts covered, 2 gaps, 1.6 MB total.

**Read the "model type" column before using a file.** Seven of the eleven are
**package-generic** — real, correctly-dimensioned package geometry, but not a
part-specific model (no pin-1 marking, no part silhouette). Four are
**part-specific**, taken from the manufacturer's own published CAD data.

Nothing here was modelled by hand. Every file is a download from the source in
the table, and every source URL was re-fetched and hash-compared by hand at
review time.

## Files

Sizes are on-disk bytes; `bbox` is the **solid** bounding box measured in
FreeCAD with `Part.read()` (not a raw point-cloud bbox — see note 2).

### Part-specific (manufacturer-published CAD)

| File | Part / package | Source | URL | Size | sha256 (first 16) | bbox mm (X×Y×Z) |
|---|---|---|---|---|---|---|
| `jst-ph-2pin-smd.step` | JST **S2B-PH-K-S**, PH 2.00 mm 2-circuit, side-entry SMD | JST Mfg. official 3D data | `https://www.jst-mfg.com/product/detail_e.php?series=199` → STEP for `S2B-PH-K-S` | 205,022 | `0556dff6583ed65b` | 5.9 × 8.2 × 7.6 |
| `jst-gh-2pin-1.25mm.step` | JST **SM02B-GHS-TB**, GH 1.25 mm 2-circuit, side-entry SMD | JST Mfg. official 3D data | `https://www.jst-mfg.com/product/detail_e.php?series=105` → STEP for `SM02B-GHS-TB` | 243,791 | `3e587c11de40124f` | 5.75 × 4.35 × 4.85 |
| `ntc-10k-disc.step` | TDK/EPCOS **B57164K** (3D outline `K164`), 10 kΩ leaded NTC disc | TDK official 3D outline | `https://product.tdk.com/system/files/dam/doc/product/sensor/temperature/ntc_elements/3doutline_step/k164.stp` | 403,005 | `4dfdf1fbb1289fe6` | 37.1 × 5.35 × 6.15 *(includes ~30 mm leads)* |
| `miniboost-4654.step` | Adafruit **MiniBoost 5 V @ 1 A, PID 4654** (TPS61023) board | Adafruit official CAD repo | `https://raw.githubusercontent.com/adafruit/Adafruit_CAD_Parts/main/4654%20MiniBoost%205V/4654%20MiniBoost%205V.step` | 179,502 | `e923ce8054ca5459` | 11.43 × 17.78 × 5.07 *(11 solids: PCB 1.57 + header + parts)* |

`miniboost-4654.step` measured 11.43 × 17.78 mm matches Adafruit's published
board size of 17.8 × 11.3 mm; Z 5.07 mm vs the published 5.6 mm (the published
figure includes the header's full pin height).

### Package-generic (official package libraries — not part-specific)

| File | Part | Package the model represents | Source | Size | sha256 (first 16) | bbox mm | package standard |
|---|---|---|---|---|---|---|---|
| `as3935-bqft-qfn16.step` | AS3935-BQFT | QFN-16-1EP 4×4 mm P0.65 mm | KiCad official `kicad-packages3D` (GitLab) | 192,117 | `3c723730b2db4946` | 4.0 × 4.0 × 0.77 | 16LD MLPQ 4×4×0.9 |
| `bme688-lga8.step` | BME688 | Bosch LGA-8 3×3 mm P0.8 mm | KiCad official `kicad-packages3D` | 74,172 | `4f4d9f09cdb4546c` | 3.0 × 3.0 × 0.78 | LGA-8 3.0×3.0×0.93 |
| `lm393-soic8.step` | LM393 | SOIC-8 3.9×4.9 mm P1.27 mm | KiCad official `kicad-packages3D` | 125,320 | `07f46cff378d30e6` | 6.0 × 4.9 × 1.75 | SOIC-8, 1.75 max height |
| `ao3401-sot23.step` | AO3401 | SOT-23 | KiCad official `kicad-packages3D` | 60,296 | `dd5d1711204e1d8d` | 2.5 × 3.0 × 1.2 | JEDEC TO-236 (SOT-23) |
| `bat54-sot23.step` | BAT54 | SOT-23 | KiCad official `kicad-packages3D` | 60,296 | `dd5d1711204e1d8d` | 2.5 × 3.0 × 1.2 | JEDEC TO-236 (SOT-23) |
| `tl431-sot23.step` | TL431 | SOT-23 | KiCad official `kicad-packages3D` | 60,296 | `dd5d1711204e1d8d` | 2.5 × 3.0 × 1.2 | JEDEC TO-236 (SOT-23) |
| `2n3904-to92.step` | 2N3904 | TO-92, inline leads | KiCad official `kicad-packages3D` | 27,693 | `f0b61c32ecbf0067` | 4.83 × 3.75 × 9.8 | TO-92 body 4.83 dia; 9.8 mm is body + straight leads |

The three SOT-23 files are **byte-identical** (same sha256): one JEDEC package
model, three part names. They are kept as separate files because the spec asks
for one file per part; treat them as one geometry.

KiCad package-model URLs follow
`https://gitlab.com/kicad/libraries/kicad-packages3D/-/raw/master/<Library>.3dshapes/<Model>.step`
with `<Library>` ∈ `Package_DFN_QFN`, `Package_LGA`, `Package_SO`,
`Package_TO_SOT_SMD`, `Package_TO_SOT_THT`.

## Gaps

### 1. PMS5003 (Plantower particulate sensor) — no CAD published

- **Plantower (manufacturer):** publishes no CAD at all. `plantower.com`
  returns 403 to a plain request; behind its cookie gate the PMS5003 product
  page (`/en/products_33/74.html`, 200) and the download centre
  (`/en/download_39/`, 200) contain **zero** `.step`/`.stp`/3D-model links —
  one company PDF only, plus the Chinese mirrors.
- **Distributors:** Mouser returns an Akamai "Access to this page has been
  denied" shell; DigiKey 403 (all locale variants); Arrow times out; Farnell /
  Newark / element14 403. No distributor stocks the bare module anyway
  (DigiKey and Mouser carry it only inside kits).
- **KiCad official:** no Plantower/PMS footprint or model exists
  (`Sensor.3dshapes/` contains only `Aosong_DHT11_5.5x12.0_P2.54mm.step`).
- STEP models for this part exist only on community file-share sites, which
  the task excludes. **Deliberately not downloaded.**

### 2. Littelfuse RXEF075 (radial PPTC, F1) — model is login-walled

- **Littelfuse:** the RXEF075-2 product page's only model asset is a
  `Partner ECAD Models` entry (SamacSys/Component Search Engine), which
  requires registration. No `.step`/`.igs` file is served from the page.
- **Distributors:** Mouser Akamai-blocked, DigiKey 403 (`.com`/`.ca`),
  Farnell/element14 403, Arrow unreachable (connection timeout).
- **TE Connectivity** (task's alternate manufacturer): `product-RXEF075-2.html`
  and `product-RF2632-000.html` both 404; TE states CAD is available by e-mail
  only; `teconnectivity.partcommunity.com` 403.
- **KiCad official:** `Fuse.3dshapes/` has chip fuses, BelFuse radial parts and
  holders — no radial PolySwitch disc. Not substituted: the Bel 0ZRE0075FF
  model is a different body size (L11.5 × W4.8 mm) from the RXEF075
  (≈10.2 × 3 mm), so using it would be a mis-sized stand-in.

### 3. Part-specific models exist but are all behind login/CAPTCHA

For **AS3935-BQFT, BME688, LM393, TL431, 2N3904, BAT54, AO3401** the
part-specific model exists but only on account-gated services, which the task
excludes ("note it as a gap rather than signing up for anything"). The
package-generic rows above cover the geometry in the meantime.

| Route | What happens |
|---|---|
| SnapEDA / SnapMagic, Ultra Librarian | login; TI's CAD export is Ultra Librarian behind a reCAPTCHA (`#SubmitLink` stays disabled); DigiKey model pages read "You must be logged-in to download the model"; Mouser serves the SnapMagic plugin |
| onsemi.com | 403 (Akamai bot wall) for 2N3904 / BAT54 pages |
| diodes.com | its published `SOT23.stp` is Cloudflare-403 anyway, and is package-level |
| aosmd.com (AO3401) | documents datasheet/marking/reliability/tape — **no STEP offered at all** |
| bosch-sensortec.com (BME688) | product page + downloads hub carry datasheet/flyer/app-note PDFs only; no 3D data |
| ScioSense (AS3935, ex-ams) | publishes `AS3935.zip` (10.95 MB, downloaded and fully extracted: datasheets, gerbers, BOM, firmware — **no CAD**); its whole WordPress media library (749 items) holds only PNG renders; ams-OSRAM no longer hosts the product page |

Constraint respected: no community file-share binaries (GrabCAD,
3DContentCentral, Thingiverse, Printables) and no account-gated downloads were
used for any committed file.

## Sources verified but deliberately not committed

| Part | URL | Size | Why not committed |
|---|---|---|---|
| SOT-23 (manufacturer-authored, Nexperia) | `https://assets.nexperia.com/documents/design-support/SOT23.step` | 220,257 | Duplicate of the SOT-23 geometry already covered; offered here as the manufacturer-authored alternative to KiCad's package model (verified: opens, bbox 2.9 × 0.95 × 2.4 mm, 5 solids, sha256 `37bd133a222045d2…`) |
| Adafruit 5046 BME688 breakout **board** | `https://raw.githubusercontent.com/adafruit/Adafruit_CAD_Parts/main/5046%20BME688%20Sensor/5046%20BME688%20Sensor.step` | 849,321 | The breakout PCB, not the bare IC; node v1 uses the bare IC. Verified: opens, bbox 28.4 × 20.3 × 3.2 mm |
| JST connectors (KiCad's package-level versions) | see KiCad URL pattern above | 186,394 / 228,309 | Superseded by JST's own official STEP files for the exact parts |

## Notes

1. **Naming.** `<part>-<package>.step` as specified. Package-generic files are
   named after the part they stand in for; the table above is the authority on
   which are part-specific.
2. **Do not trust a naive point-cloud bbox on `miniboost-4654.step`.** The file
   carries construction geometry far outside the part (a `CARTESIAN_POINT` at
   z ≈ 5×10⁵ mm), so a raw min/max over all points reports a nonsense size. The
   real solid bbox is 11.43 × 17.78 × 5.07 mm and FreeCAD's STEP importer loads
   it as 11 clean solids with no stray geometry — verified by importing the
   committed file with `import_step`, not just with `Part.read()`.
3. **Package heights are body-only.** The KiCad LGA-8 (0.78 mm) and QFN-16
   (0.77 mm) models stop at the moulded body, so they read slightly under the
   datasheet maximums (0.93 mm / 0.9 mm). That is the library convention, not a
   defect; it only matters if a clearance stack-up is being computed.
4. **AS3935 exposed pad.** The QFN-16 model's exposed pad is 2.5 mm against the
   AS3935-BQFT's 2.70 mm nominal — 0.2 mm small. Fine for assembly
   visualisation, worth knowing for paste-stencil work.
5. **`2n3904-to92.step` and the SOT-23 trio are package-level placeholder-grade.**
   If the Rev C BOM's real packages turn out to be SOT-23 for 2N3904 (rather
   than TO-92), `ao3401-sot23.step` is the same geometry and can be reused.
6. **Extensions.** All files use `.step` for uniformity; `ntc-10k-disc.step` was
   downloaded as `.stp` (same format, renamed only).

## Method

Every file was downloaded over HTTP from the URL in the table (no hand-authored
geometry), then verified with FreeCAD 1.1.1:

- opens as a valid solid (`Part.read()` → `isValid() == True`, ≥1 solid);
- measured solid bbox compared against the part/package dimensions — nothing
  off by anywhere near the 2× flag threshold;
- for the four part-specific files the source URL was re-fetched independently
  at review time and the bytes hash-compared against the committed file
  (JST ×2 and TDK matched exactly; the Adafruit file re-fetched at 179,502
  bytes, byte-identical).
