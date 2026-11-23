---
task_id: "0247"
title: Live link recovery — WB per shot, bounded waits, failure redraw, slideshow holes
status: staged
iteration: 1
proof:
  repo: pagosacabin/cabin-play
  branch: hermes/0247-live-recovery
  sha: 113c06798e99ac3c7a020df677bceab88c6c325c
  base: 154354b (0245 tip)
  files:
    - living-frame/firmware/freenove-cam/src/main.cpp (WB per shot + NaN guard, chunk retry, rate call logged)
    - living-frame/firmware/n16r8-live/src/live.c (12 s deadline, failure redraw, slideshow bound, session per frame)
    - living-frame/firmware/n16r8-live/src/main.c (dead 0230 link code removed)
    - living-frame/firmware/n16r8-live/{README.md,src/CMakeLists.txt} (figures and comments corrected)
  bench: "5 consecutive frames 1561-1766 ms fail 0, every SHA acked, library 23->26; post-cleanup frame 1706 ms, fs->display 1761 ms, library 36"
notes: |
  Filed by Hermes from Stephen's external static review of the 0244/0245 firmware;
  claimed and worked in the same interactive session with the workers paused, and
  resumed on staging.  No Copilot pass was requested for this card.
---

## Status: staged

Every correctness item from the review is fixed and bench-verified, except the
two that were declared out of scope and are still open.

## Per item

**White balance per shot, with a NaN guard.** Gains are computed fresh for every
shot instead of smoothing 60/40 against the previous frame (a streaming leftover:
one press per minute was smoothing across scenes). A near-zero channel mean now
falls back to identity, and the clamp is written NaN-safe (`!(g > 0.5f)`) because
the old `g < 0.5f` is false for NaN — the filter used to carry NaN until reboot.
Bench gains differ per shot as expected: `r 1.107 g 0.844 b 1.193`,
`r 1.108 g 0.843 b 1.193`. The black-frame path is code-verified only; producing a
truly black frame needs the lens covered and was not done in this run.

**Per-chunk retry.** `send_pkt()` retries up to three times when the send callback
reports failure. The frame-level SHA still decides acceptance.

**Bounded wait and failure redraw.** Post-SHOOT deadline 30 s → 12 s; a timeout or
hash mismatch re-publishes the buffer holding the previous frame so the countdown
digit does not stay on the panel. `shoot()`'s return is now checked.

**Slideshow holes.** `live_load()` is tried up to `s_lib_max` times, not
`s_lib_count`.

**Session id per frame**, not per boot.

**Camera off the AP — tried and reverted, with numbers.** Unassociated STA, same
channel, `WIFI_PS_NONE`, TX forced to 19.5 dBm: fail 0 but **6544-6959 ms** per
frame. Associated: **1561-1766 ms**. The AP's beacons and the driver's associated
rate table are worth more than the credentials cost, so the association stays and
the reason is recorded in the source.

**Docs and dead code.** README transfer figure corrected to ~1.6 s associated /
~6.5 s unassociated; the 0230-era `link_recv`/`link_init` block deleted from
`main.c` (it carried a stale 4-byte chunk header); `CMakeLists.txt` comment fixed.

## Proof

Camera log, five consecutive frames:

```
sent    : 1280 pkts, 1766 ms, ok 1282 fail 0, sha e9c9f55f...
peer ack: sha e9c9f55f...
sent    : 1280 pkts, 1567 ms, ok 2564 fail 0, sha 3f6abf1a...
...
sent    : 1280 pkts, 1562 ms, ok 6410 fail 0, sha ad459a0f...
```

Frame report:

```
live : frame ok, sent sha 3f6abf1a..., stored sha 3f6abf1a..., library 24
timing : fs->display 1607 ms
...
live : SD library holds 36 frames, next index 36
live : frame ok, sent sha fcf350ed..., stored sha fcf350ed..., library 36
```

Library grew by exactly one per frame; every stored hash equals the sent hash.

## Declared not done

1. **Chunk NACK/resend** (the `s_seen` bitmap returned to the camera after `FE`).
   This is the right fix for a lost chunk, but it is a protocol change with its
   own bench matrix; the per-chunk retry covers the common case. Its own card.
2. **Report-partition rework** — erase before `panel_start`, append-only 4 KiB
   records, an `app_logf` mutex, the 6,000-byte cap that silently stops logging,
   and the USB-Serial-JTAG console check. Structural, own card, as filed.
3. **Core affinity experiment** (panel task on core 1) and the `build_raster`
   lookup-table optimisation — both worth trying, neither is a defect.
