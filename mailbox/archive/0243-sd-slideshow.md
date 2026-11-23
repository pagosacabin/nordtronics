---
task_id: "0243"
protocol_version: 1.3.0
status: verified
iteration: 2
proof:
  branch: "hermes/0243-sd-slideshow"
  repo: "pagosacabin/cabin-play"
  sha: "7451fd169ad79e0bf4d0e5b64685c8d6158f5bec"
  built_from: "fad55dc73dedf50cca6b6711bd3802296f46e9ae -- the tip only deletes an empty bench/.keep; src/main.c is byte-identical between the two (`git diff --quiet fad55dc..7451fd1 -- .../src/main.c`), and the binary on the board was built from that source (main.c mtime 22:01:34, firmware.bin 22:01:39, `pio run` re-run is a no-op)"
  run: none
  run_note: |
    cabin-play carries no workflows at all: `ls .github` is "No such file or
    directory" and `git ls-tree -r --name-only origin/hermes/0243-sd-slideshow
    .github/workflows` is empty. No run exists or is expected for this branch --
    enumerated, not assumed.
  files:
    - living-frame/firmware/n16r8-slideshow/src/main.c
    - living-frame/firmware/n16r8-slideshow/platformio.ini
    - living-frame/firmware/n16r8-slideshow/partitions.csv
    - living-frame/firmware/n16r8-slideshow/sdkconfig.defaults
    - living-frame/firmware/n16r8-slideshow/CMakeLists.txt
    - living-frame/firmware/n16r8-slideshow/src/CMakeLists.txt
    - living-frame/firmware/n16r8-slideshow/src/photo64_data.h
    - living-frame/firmware/n16r8-slideshow/src/photo64_meta.h
    - living-frame/firmware/n16r8-slideshow/tools/convert_photo64.py
    - living-frame/firmware/n16r8-slideshow/tools/fetch_frames.py
    - living-frame/firmware/n16r8-slideshow/tools/build_frames.py
    - living-frame/firmware/n16r8-slideshow/tools/sources.json
    - living-frame/firmware/n16r8-slideshow/bench/read_report.py
    - living-frame/firmware/n16r8-slideshow/bench/stage.sh
    - living-frame/firmware/n16r8-slideshow/bench/show.sh
    - living-frame/firmware/n16r8-slideshow/logs/42-0243-stage-run.txt
    - living-frame/firmware/n16r8-slideshow/logs/43-0243-slideshow-run.txt
    - living-frame/firmware/n16r8-slideshow/logs/frames.sha256
    - living-frame/firmware/n16r8-slideshow/README.md
  board_end_state: "left running env:n16r8-slideshow (the slideshow), card mounted, FRAME01..FRAME10 in /sdcard/FRAMES; the run whose log is quoted played a full loop plus the wrap to FRAME01"
  ntfy: "not published -- this task names no topic and produces no compiled deliverable for distribution"
  peer_review: "Copilot (Herdr pane t17), 3 passes: pass 1 (general) no findings/SHIP; pass 2 (focused) 2 MAJORs -- report erase during scanout, gate trusted cid.serial alone; both fixed in fad55dc and the whole bench re-run; pass 3 on the fix: no findings, Verdict: SHIP"
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-23T04:15:38+00:00"
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
  note: "This harness exposes no per-call usage to the cron session; token and cost fields are unknown rather than estimated."
notes: |
  OUTCOME: the slideshow runs. Ten 800x600 rasters sit in /sdcard/FRAMES, the
  firmware plays FRAME01..FRAME10 in filename order and wraps to FRAME01, and
  every frame's SHA-256 on the panel side equals the host-side digest in
  logs/frames.sha256. The board was left running the slideshow.

  PREMISE CORRECTION (declared): 0243 says the base branch's project tree
  `living-frame/firmware/n16r8-sdprobe/` holds "the proven SD stack and the photo
  display build in one tree". It does not -- that tree is the 0229/0241 SD probe
  and contains no display code at all (no esp_lcd include, no framebuffer). The
  display build is `n16r8-photo`. So the deliverable is a NEW project,
  `n16r8-slideshow/`, built from 0228's 800x600 gamma display code plus the
  0239/0241 SD stack copied in; the 0241 tree is untouched and remains the base.

  WHAT WAS BUILT -- two envs from one source:
    n16r8-slideshow-stage  copies the ten rasters from a `frames` data partition
      (new, 0x400000, 5 MiB) to /sdcard/FRAMES/FRAME01..10.BIN, then reads each
      file back off the card and compares SHA-256 with the flash copy.
    n16r8-slideshow        the slideshow: 0239 pre-sacrifice + one stock-driver
      mount at 400 kHz, enumerate /sdcard/FRAMES sorted by name, load the first
      frame, start the panel, then prefetch the next frame into a SECOND PSRAM
      framebuffer while the current one streams, swap, repeat; ~10 s dwell.
      Card absent / unmountable / no loadable frame -> embedded hummingbird
      (the accepted build's raster, 204578cc...bd306), logged as FALLBACK.
    Display and SD parameters are unchanged from the builds that proved them
    (0228 800x600 mode, 60.3 Hz; SD SCK12/MOSI11/MISO13/CS10 mode 0, 400 kHz,
    pre-sacrifice, no format).

  HOW THE FRAMES REACHED THE CARD: the card is in the board's holder, so there
  is no host-side write. The ten rasters are concatenated into a 4,800,000 B
  blob, written to the `frames` flash partition with esptool (compressed, hash
  verified), and the stage build copies them to FatFS. This is why the partition
  table grew a `frames` entry (it also needed board_upload.flash_size=16MB
  before the partition tool would accept a 9 MB table).

  EVIDENCE (logs/42, logs/43; both read back from the report partition, which is
  the only console on this bench -- the FT232R delivers nothing, 0225):
    Stage run: GATE_TESTS psn_0241=1 name_SD32G=1 capacity_28_34GB=1 is_kit=0
      is_recalbox=0 -> AUTHORIZED; NEWCARD.BIN 131072 B present before and after
      (untouched); 10/10 files written (~10.55 s each) and read back (~11.19 s
      each) with digests equal to logs/frames.sha256; STAGE_SUMMARY 10/10 in
      218.3 s.
    Slideshow run: card SD32G psn 0xc6bd0f92; 10 files enumerated, all 480000 B;
      FRAME01 read in 11212 ms; seq 2..10 at 12.138/24.193/36.336/48.394/60.533/
      72.592/84.730/96.784/108.918 s, seq 11 = wrap to FRAME01 at 120.977 s;
      steps 12.05-12.14 s; every logged digest matches the host list.
    Pre-state before any flash: the accepted 0227 build, 800x600 gamma,
    framebuffer sha256 204578cc...bd306 MATCH, 60.35 Hz measured.

  DEVIATIONS AND SELF-CAUGHT DEFECTS:
   1. Copilot reviewed AFTER the bench run, not before (task asks before). Pass 1
      found nothing; pass 2 found two MAJORs, both fixed, and the entire bench was
      re-run on the fixed build (stage + slideshow); pass 3 on the fix: SHIP.
   2. Step is ~12 s, not ~10 s. The dwell IS 10 s; a 480,000 B read at the proven
      400 kHz costs ~12 s, and the read is what gates the step (the prefetch
      overlaps the dwell, it cannot beat it). The clock is a constraint of the
      task, so 12 s is the honest floor at 800x600; at the 640x480 accepted raster
      the same code would sit at 10 s.
   3. My own report read (which resets the board) landed mid-write during staging
      and left FRAME01.BIN 0 bytes; the slideshow skipped it, which is how the
      defect was found. Fixed twice over: frame_load retries a short first read up
      to 3 times and logs RETRY, and staging is never interrupted by a report
      read again (read only when the run it describes has finished).
   4. SHA known-answer test bug, self-caught: the KAT hashed FB_BYTES/4096 whole
      4 KiB blocks (479,232 B, not 480,000) and failed on hardware. The KAT now
      uses 1 MiB of zeros, a whole number of blocks.
   5. Frame 10 was first converted from a public-domain PAINTING ("Common foxes
      in the snow", W. Kuhnert); replaced with a real photograph (Commons Quality
      image "Arabian Red Fox 3") and re-staged.
   6. The report layout was redesigned after review: erased once at boot with the
      panel stopped, then one already-erased 4 KiB record per persist (no erase
      during scanout, which stalls the bounce ISR). A later boot appends after
      the previous run's records -- logs/43 begins with "continuing the existing
      log at sector 2".
   7. Card gate hardened after review: serial AND CID name SD32G AND capacity
      28-34 GB must agree, plus explicit refusal of the kit card and the
      Recalbox SR512. Residual, stated plainly: a relabelled card duplicating all
      three would still pass; the write is confined to /sdcard/FRAMES and nothing
      formats or repartitions. The Recalbox card was never in the holder.
   8. The ten images are bench stand-ins (Commons, PD/CC BY/CC BY-SA, sources in
      tools/sources.json and listed below); Stephen curates the real ones later.

  NOT VERIFIED, and said so rather than implied: the fallback path is stated from
  the code and its log line, not demonstrated live -- the task explicitly allows
  that and forbids risking the staged card to prove it. Likewise the visual
  judgement (does it look good on Stephen's TV) is his; what is verified here is
  that the bytes on the panel are the converted files, by digest.
---

# 0243 — SD-card slideshow: REPLY

**The slideshow is running: ten card images cycle on the frame at ~12 s each and
wrap back to the first, every frame's digest on the board matching the converted
file on the host.** Board left on the slideshow build (no restore this time, as
asked). Branch `hermes/0243-sd-slideshow` @ `7451fd1`, no CI in this repo
(enumerated in `proof.run_note`).

## The ten images (bench stand-ins, all freely licensed)

| # | source (Wikimedia Commons) | licence | author |
|---|---|---|---|
| 01 | Elephant bull in green savanna | CC BY-SA 2.0 | Bernard DUPONT |
| 02 | Glencoe Lochan reflections 3 | CC BY-SA 3.0 | Daniel Kraft |
| 03 | Panorama della Val d'Orcia da Via del Casello, Pienza | CC BY 4.0 | Travel Designer |
| 04 | Beech Forest (AU), Beauchamp Falls 2019 | CC BY-SA 4.0 | Dietmar Rabich |
| 05 | Humpback whale breaching, Ballena Marine NP | CC BY-SA 4.0 | Giles Laurent |
| 06 | Oia, Santorini | CC BY-SA 2.0 | bongo vongo |
| 07 | Superior NF, Fall Lake fire area (aerial) | Public domain | Superior National Forest |
| 08 | Peacock displaying covert feathers | CC BY-SA 4.0 | Chinmayee Mishra |
| 09 | Dead camel thorn trees, Deadvlei | CC BY-SA 4.0 | Giles Laurent |
| 10 | Arabian Red Fox 3 | CC BY-SA 4.0 | Commons Quality image |

Each was converted with the accepted build's converter at the accepted build's
settings (`tools/convert_photo64.py --mode 800x600 --space gamma`) to a 480,000 B
raster, named so that filename order is play order.

## Card state

```
/sdcard/FRAMES/FRAME01.BIN … FRAME10.BIN   480000 B each   (10 files)
/sdcard/NEWCARD.BIN                        131072 B        (0241 evidence, untouched)
```
Card identity: `SD32G`, CID PSN `0xc6bd0f92`, 30436 MB — 0241's SanDisk, checked
before anything was written. The Recalbox card was never in the holder.

## The load log (from `logs/43-0243-slideshow-run.txt`)

```
FRAME seq=1  FRAME01.BIN  480000 B read in 11212 ms  sha 93d22833a3956ac2
FRAME seq=2  FRAME02.BIN  shown at  12.138 s  step 12.138 s  sha fd4610b11aee3c42
FRAME seq=3  FRAME03.BIN  shown at  24.193 s  step 12.055 s  sha 0b14db9555fce686
FRAME seq=4  FRAME04.BIN  shown at  36.336 s  step 12.141 s  sha 844b4348f9bbff5c
FRAME seq=5  FRAME05.BIN  shown at  48.394 s  step 12.058 s  sha 4d3fb365714b3ea6
FRAME seq=6  FRAME06.BIN  shown at  60.533 s  step 12.139 s  sha 88b066e9a16772d7
FRAME seq=7  FRAME07.BIN  shown at  72.592 s  step 12.058 s  sha f139036854e2e400
FRAME seq=8  FRAME08.BIN  shown at  84.730 s  step 12.137 s  sha 2c9b7a0dc7894132
FRAME seq=9  FRAME09.BIN  shown at  96.784 s  step 12.053 s  sha 88cb7fc21526b8cf
FRAME seq=10 FRAME10.BIN  shown at 108.918 s  step 12.134 s  sha f368f81543b197b2
FRAME seq=11 FRAME01.BIN  shown at 120.977 s  step 12.056 s  sha 93d22833a3956ac2   <- wrap
```

Order is filename order, one full loop plus the wrap, and each digest equals the
host-side digest in `logs/frames.sha256`. The step is 12 s rather than 10 s
because a 480,000 B read at the proven 400 kHz takes ~12 s and the read gates the
step; the dwell itself is 10 s. Raising the clock is what would close that gap,
and the task freezes the clock.

## Fallback

If the card is absent, will not mount, or holds no loadable frame, the app copies
the embedded hummingbird (the accepted build's exact raster,
`204578cc…bd306`) and logs `FALLBACK: <reason> -- showing the embedded
hummingbird` plus a digest check against the converter's. A frame file that is
the wrong size or cannot be read is skipped with a `SKIP`/`RETRY` line and the
playback moves on. This is stated from the code and its log lines; it was not
demonstrated by pulling the card, which the task rules out.

## Review

Copilot (Herdr pane t17), three passes. Pass 1: no findings, SHIP. Pass 2,
focused at my request: two MAJORs — a flash erase during scanout can stall the
bounce ISR (report layout redesigned: erase once at boot, then one 4 KiB record
per persist with no erase), and the stage gate trusted `cid.serial` alone (now
serial + CID name + capacity, with the kit and Recalbox cards refused by name).
Both fixed at `fad55dc`, the whole bench re-run on the fixed build, pass 3 on the
fix: no findings, **Verdict: SHIP**. The review came after the first bench run
rather than before it — declared in `notes`.

## How to reproduce

```bash
cd living-frame/firmware/n16r8-slideshow
/usr/bin/python3 tools/fetch_frames.py --out frames-src
/usr/bin/python3 tools/build_frames.py  --src frames-src --out frames
cp frames/frames-blob.bin bench/ && bench/stage.sh   # ~4 min, 10/10 verified
bench/show.sh                                        # slideshow, running
bench/read_report.py                                 # the log above
```


---
## Closure (Juno, 2026-11-22 22:2x MST)

Verified independently: branch hermes/0243-sd-slideshow @ 7451fd169ad79e0bf4d0e5b64685c8d6158f5bec
matches origin; committed log 43 spot-checked on the branch — frame sequence seq 1-9 and
the wrap (seq 11, FRAME01 at 120.977 s, step 12.056 s) match the reply's table exactly.
Step is ~12 s, not 10 s: dwell is 10 s, the 480,000 B read at 400 kHz costs ~12 s and
gates the step (task froze the clock; closing the gap is a later rung). Copilot 3 passes,
final SHIP; the review came after the first bench run (declared), 2 MAJORs fixed and the
whole bench re-run. Premise correction accepted: the 0241 tree held no display code, so
the deliverable is a new n16r8-slideshow project built from 0228's display code + the
0239/0241 SD stack; the 0241 tree is untouched. Fallback stated from code, not
demonstrated live, per the task. Board left running the slideshow.
