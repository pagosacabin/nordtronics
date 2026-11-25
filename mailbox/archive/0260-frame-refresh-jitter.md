---
task_id: "0260"
protocol_version: 1.4.0
status: verified
iteration: 1
proof:
  branch: "hermes/0260-refresh-jitter"
  repo: "pagosacabin/cabin-play"
  sha: "ac8c40ae6bd1560515fefeabed4092a9f791a62c"
  base: "hermes/0259-slideshow-fixes @ 1a21a3e -- 0259 was built and verified before this branch, so 0260 sits on it rather than on 0258, as the card allows; no rebase was needed"
  run: none
  run_note: |
    cabin-play carries no workflows at all: `ls .github/workflows/` is "No such file or
    directory", so no run exists or is expected for this branch.  The deliverable was
    built, flashed and verified on the bench instead: `pio run -e n16r8-photo` SUCCESS,
    upload "Hash of data verified", and `esptool --port /dev/ttyUSB1 verify_flash 0x10000
    .pio/build/n16r8-photo/firmware.bin` -> "verify OK (digest matched)" against
    sha256 2f218650f57b7af8be7048fcdbdf2c3fc8489a0d95be5ad8edb166a5c3209e36.
  files:
    - living-frame/firmware/n16r8-live/src/main.c
    - living-frame/firmware/n16r8-live/src/live.c
    - living-frame/firmware/n16r8-live/bench/read_report.py
    - living-frame/firmware/n16r8-live/bench/swap_model.py
  root_cause: "not the swap's content: main.c's bounce ISR reads s_fb_live once per 16-line burst (30 fills/frame, measured), and live.c published a finished frame with a bare `s_fb_live = buf` from the loop -- the store lands wherever the raster is, so one refresh is part old photo, part new. That is the splash->photo handover Stephen filmed and the same artifact on every return to a photo."
  fix: "the frame is handed over in s_fb_pending and only the ISR takes it, at pos_px == 0, before the first burst of a frame is filled; a handoff that sees no boundary in 100 ms is cancelled under the ISR's own lock rather than published directly."
  swap_timing_proof: "11 real swaps on the bench, every one applied at pos 0 with mixed lines 0; ISR-measured request->applied latency 1.0-16.35 ms, never more than one frame (16.8 ms). Log quoted in the reply."
  boot_transition_clean: "PENDING STEPHEN -- not claimed. The splash->photo handover ran 6 times on the bench and every swap is instrumented clean, but nobody has looked at the panel."
  button_transitions_clean: "PENDING STEPHEN -- not exercised at all: this run pressed no button (Stephen is away). The keep prompt, camera notice and delete confirm all publish through the same fixed path, but that is code, not a bench observation."
  regressions_checked: "exercised without a press: slideshow cycling, shuffle order, splash release, report log -- all pass. NOT exercised (need a press): keep / discard / timeout-keep / camera notice / silent delete / delete-confirm cancel. Portrait text and splash: untouched files, nothing written to them."
  copilot_review: "six passes on branch hermes/0260-refresh-jitter: FIX (3 findings), FIX (2), FIX (1), FIX (2 minor), FIX (1 major + 1 minor), then SHIP with one non-blocking NIT. Every finding fixed at the source; the one NIT (show_notice clears the photo-on-panel flag even if the notice failed to publish, which can only disable a delete, never enable one for a hidden photo) is left."
  ntfy: "not published -- the card names no topic and this build is not a companion APK; nothing is published as an artifact."
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-25T22:51:15+00:00"
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
  DONE, BENCH-INSTRUMENTED, IN REVIEW SHIP -- visual confirm owed by Stephen.

  ROOT CAUSE, from the code and then from the log (not assumed from the video):
  main.c's bounce ISR copies from s_fb_live one 16-line burst at a time -- the app
  itself measures 30 fills per frame -- and live.c published with a bare pointer
  store from the loop.  That store lands at whatever line the raster is on, so the
  frame being scanned out is half the old buffer and half the new one.  The video's
  "part has switched, the rest is catching up" is that single torn refresh; the same
  path is used for the splash->photo handover, the keep prompt, the notices and the
  delete confirm, which is why it repeated on button presses.

  FIX: s_fb_pending + the ISR takes it at pos_px == 0.  A frame is only ever
  replaced before its first burst is filled, so every line of a frame comes from one
  buffer.  After boot nothing outside the ISR assigns s_fb_live (grep: two remaining
  assignments, both in app_main before panel_start).

  EVIDENCE, three independent pieces:
   1. 11 real swaps in one boot, every one "applied at pos 0 of the next frame ...
      mixed lines 0", requests landing with the raster at lines 0..464 of 480, ISR
      latency 1.0-16.35 ms (one frame is 16.8 ms).  The log also quotes what the old
      bare store would have mixed at that instant: 16 to 480 lines.
   2. bench/swap_model.py over one frame at 1/8-burst resolution: 262 of 263 request
      instants tear the old way (worst 240 of 480 lines); the new protocol is
      mixed == 0 at every one.
   3. the image on the board is proven by digest, not by its banner: verify_flash
      digest matched firmware.bin sha256 2f218650...209e36.  The __DATE__ banner still
      reads 16:14:08 because main.c did not change and PlatformIO reused its object --
      the skill's known trap, not a stale flash.

  SCOPE CHANGE, declared: the report writer moved to 0243's record format (erase once
  in app_main BEFORE panel_start, then 4 KiB records with no erase).  The old code
  erased 64 KiB on every write_report() -- including calls made while the panel was
  streaming -- which stutters the cache the bounce ISR reads PSRAM through, and this
  task's whole subject is what the panel does during a transition.  The swap log is
  flushed once every 12 swaps, and the flush is placed BEFORE the request so a
  flash-write stall can only touch the outgoing frame.  bench/read_report.py reads
  the new format from /dev/ttyUSB1 (the FRAME; /dev/ttyUSB0 is the camera's CH340).

  REVIEW, six passes; every finding real and fixed at the source rather than
  papered over: (1) the 100 ms fallback published from the task and could tear if
  scanout resumed; (2) a timed-out handoff left the pending slot naming a buffer the
  caller was about to rewrite -- now one portMUX section covers request, ISR take and
  cancel; (3) publish() was void, so a delete answer could be taken for a confirm
  that never appeared, and the slideshow could spend an index on an unseen photo;
  (4) the shuffle skipped a photo whose handoff failed and a dropped keep prompt left
  a countdown digit on the glass; (5) s_cur was standing in for "a photo is visible",
  so with a failed restore a hold could open the delete confirm for a hidden photo --
  s_photo_on_panel now tracks what is actually on the glass.  The final NIT is left
  deliberately (see proof.copilot_review).

  HONEST GAPS: no button was pressed, so the press-path criteria in the card are
  unverified by eye and unexercised by this run -- the code is shared, the observation
  is not there.  The hold bar still animates in the on-screen buffer (a 0254
  trade-off): the swap is frame-boundary safe, the bar can still tear a few rows.
  Nothing is claimed as visually confirmed.

  FIELD NOTE for the next task on this rig: opening /dev/ttyUSB1 with esptool to read
  the report resets the board, which erases the report at the next boot -- each read
  captures the run that just ended, so read once, not twice.

  Also recorded in the crew-vault bench notes (bench/frame-storage-and-link.md,
  commit af1cc4d, local-only repo).
expect-reply-within: 24h
---

# 0260 — Living frame: eliminate refresh jitter/tearing on image swaps

## Assignment

- **Hermes — firmware + bench (this task):** find and fix the torn-frame jitter below, verify it, and push the branch. Stephen is away from the bench for this filing — implement the fix without button presses; the visual confirmation is his step when he's back (see Task 4).
- **Copilot — peer review:** review the branch before staging (GitHub-bound work). Reviewer only — Juno verifies independently on top.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Stephen — bench side:** visual confirm on return — one boot (watch the splash→first-photo handover) plus a few button presses (prompt/notice→photo transitions).

## Context

- Stephen's video, Nov 25 ~15:18. The splash holds steady for its 6 s, then the handover to the first photo shows a torn frame — part of the screen has already switched while the rest is still catching up — and then the photo settles. The same artifact appears on button presses, when the frame goes back to a photo after a prompt or a notice (keep prompt, discard notice, camera notice, delete confirm). **Video of the symptom: `mailbox/attachments/0260-refresh-jitter.mp4` in this repo** (trimmed to the splash→photo handover, ~4.6 s; the full clip is 8.6 s). The text description here stands on its own if the video won't play.
- Likely cause (Juno's hypothesis, to be confirmed from the code, not assumed): the buffer swap happens mid-scan while the VGA DAC is reading the raster out, so one refresh is half old image, half new. Fix direction: swap only during vertical blank (or equivalent — finish the new frame before flipping, or blank during the transition — Hermes's choice, as long as the visible result is a clean swap).
- This is a timing/sync issue, not the content issue 0254 fixed with the s_hold snapshot (that was about restoring the right photo; this is about when the swap lands).
- Branch from `hermes/0258-portrait-text` (latest frame branch on origin; 0259 is filed but not yet built — if 0259 lands first, rebase onto it and say so in the reply). **Do not touch splash code or the splash asset.**
- Record the outcome in the crew-vault living-frame bench notes (bench/frame-storage-and-link.md).

## Task

Eliminate the visible jitter/tearing when the frame swaps images. One deliverable: the frame firmware on branch `hermes/0260-refresh-jitter` in cabin-play, with clean swaps on the bench.

1. Establish the actual cause from the code: where and when the active buffer is swapped relative to VSYNC/scanout. State it in the reply — do not assume Juno's mid-scan hypothesis is right.
2. Make swaps land cleanly: only during vertical blank, or by an equivalent method (complete the new frame before flipping; blank the output during the transition). The visible result must be no torn frame.
3. Instrument the swap timing if the firmware can (log swap vs VSYNC) and include that log as proof in the reply.
4. Verify visually with Stephen on his return: one boot — the splash→first-photo handover shows no tear; and button-press transitions back to a photo (keep prompt→photo, notice→photo, delete→next photo) show no tear. Stephen is away from the bench while this is filed, so implement first and confirm when he's back.

## Success criteria

- No visible tearing/jitter on the splash→first-photo transition (Stephen's visual confirm).
- No visible tearing/jitter on button-press transitions back to a photo (Stephen's visual confirm).
- Swap timing proven at/near vertical blank (code + log if instrumented).
- No regression: the 0254 keep/discard/timeout-keep flows, the 0256 camera notice, the 0257 silent delete, and the 0258 portrait text all still work as before.

## Constraints

- Splash/logo untouched — do not regenerate or modify the splash asset or splash code.
- One button only (GPIO14). No new hardware. No behavior changes beyond the swap timing.
- Stephen is away from the bench for this filing: implement without button presses; the visual confirm is his step on return. Do not block the implementation on it, but do not claim the fix verified until he has seen it.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Branch `hermes/0260-refresh-jitter` pushed to origin in cabin-play, tip SHA quoted in the staged reply.
- The cause as found in the code, quoted in the staged reply.
- Swap-timing evidence (log vs VSYNC if instrumented) and Stephen's visual confirm, quoted in the staged reply.
- Copilot peer-review result quoted in the staged reply.

## Reply format

Stage the reply in this file with these fields filled in:

- `branch`: hermes/0260-refresh-jitter
- `tip_sha`: commit SHA on origin
- `root_cause`: what the code showed (mid-scan swap / other) + one line
- `fix`: what changed (swap at blank / other) + one line
- `swap_timing_proof`: log/code evidence the swap lands at blank + one line
- `boot_transition_clean`: yes / no + one line (Stephen's visual confirm)
- `button_transitions_clean`: yes / no + one line (Stephen's visual confirm)
- `regressions_checked`: keep / discard / timeout-keep / camera notice / silent delete / portrait text / splash — each pass / fail + one line
- `copilot_review`: result quoted
- `notes`: anything unexpected

# Reply — 0260: a clean swap at the frame boundary

**The tear is fixed at the source and proven with an instrumented bench run. Copilot SHIP on the sixth pass. Stephen's visual confirm is still owed — nothing here claims to have seen the panel.**

| Field | Value |
|---|---|
| `branch` | `hermes/0260-refresh-jitter` |
| `tip_sha` | **ac8c40a** (`ac8c40ae6bd1560515fefeabed4092a9f791a62c`) |
| `root_cause` | **Mid-scan pointer swap, exactly as suspected — confirmed from the code, then measured.** `main.c`'s bounce ISR copies from `s_fb_live` one 16-line burst at a time (the app itself measures 30 fills per frame); `live.c` published a finished frame with a bare `s_fb_live = buf` from the loop, which lands at whatever line the raster happens to be scanning. One refresh is part old image, part new. |
| `fix` | The finished frame is left in `s_fb_pending`; **only the ISR takes it, at `pos_px == 0`** — the first burst of a frame, before any of that frame's lines are scanned out. A handoff that sees no boundary within 100 ms is cancelled inside the same `portMUX` section the ISR takes it in, never published directly. |
| `swap_timing_proof` | **11 real swaps on the bench, every one applied at pos 0, `mixed lines 0`**, requests landing at raster lines 0–464 of 480, ISR-measured latency **1.0–16.35 ms** against a 16.8 ms frame. Model check: 262/263 request instants tear the old way, 0/263 the new one. Log quoted below. |
| `boot_transition_clean` | **pending Stephen.** The splash→photo handover ran 6 times in this session and every swap is instrumented clean, but nobody has looked at the panel. Not claimed. |
| `button_transitions_clean` | **pending Stephen — and not exercised at all.** No button was pressed this run (Stephen is away). Keep prompt, camera notice, delete confirm and restore all publish through the same fixed path, but that is code, not an observation. |
| `regressions_checked` | slideshow cycling **pass**, shuffle order **pass**, splash release **pass**, report log **pass** — all exercised without a press. keep / discard / timeout-keep / camera notice / silent delete / delete-cancel: **not exercised** (they need a press). portrait text, splash asset: **untouched files**. |
| `copilot_review` | six passes — FIX, FIX, FIX, FIX, FIX, then **SHIP**; findings listed below, each fixed at the source |

## 1. What the code showed

`main.c` streams `no_fb` through 16-line bounce buffers, and the ISR refills one burst per
callback:

```c
static bool IRAM_ATTR bounce_fill(...)
{
    s_fill_count++;
    memcpy(bounce_buf, (const uint8_t *)s_fb_live + pos_px, (size_t)len_bytes);
    return false;
}
```

`live.c` published from the task side:

```c
s_fb_live = dst;      /* atomic publish */
s_w ^= 1;
```

A 32-bit pointer store is atomic, which is why this looked safe in 0244 — but "atomic" only
means the ISR never sees a half-pointer. It still sees the **new** buffer from the next burst
onwards. If the store lands with the raster at line 224, lines 0–223 of that refresh come
from the old photograph and 224–479 from the new one: the torn handover in the clip, and the
same thing on every return to a photo (keep prompt → photo, notice → photo, delete → next
photo). It is the *when* of the swap; 0254's `s_hold` snapshot fixed the *what*.

## 2. The fix

- `main.c`: `s_fb_pending` plus the ISR taking it **only when `pos_px == 0`**, before the
  first burst of a frame is filled, inside a `portMUX` critical section shared with the task.
- `live.c`: one `publish()` helper used by every publication site (photo, countdown, keep
  prompt, notice, delete confirm, restore, kept photo). It records where the raster was,
  hands the buffer over, and waits for the ISR to take it.
- A handoff that sees no frame boundary in 100 ms is **cancelled**, not published directly —
  a direct store *is* the tear. Callers that would act on what the panel shows (delete,
  keep, slideshow index) now honour that: a photo whose handoff failed keeps its place in the
  shuffle, a confirm that never appeared cannot be answered, and the delete gesture requires
  a photograph actually on the glass (`s_photo_on_panel`) rather than merely a valid `s_cur`.

## 3. Bench evidence

Frame 8 built from `ac8c40a`; `esptool verify_flash 0x10000 firmware.bin` → `verify OK
(digest matched)` (sha256 `2f218650…209e36`), so the image on the board is the branch's.
(The `build` banner still reads 16:14:08 — `main.c` did not change in the last two commits
and PlatformIO reused its object; the digest is the proof, not the banner.)

Eleven swaps in one boot, quoted verbatim:

```
swap : photo requested at px 276480 (raster at line 432/480, vsync 525) -> applied at pos 0 of the next frame in 1041 us (vsync 525), swap #1, mixed lines 0 (a bare pointer store would have mixed 48)
swap : photo requested at px 10240 (raster at line 16/480, vsync 1121) -> applied at pos 0 of the next frame in 14723 us (vsync 1121), swap #2, mixed lines 0 (a bare pointer store would have mixed 464)
swap : photo requested at px 0 (raster at line 0/480, vsync 1716) -> applied at pos 0 of the next frame in 16346 us (vsync 1717), swap #3, mixed lines 0 (a bare pointer store would have mixed 480)
... 8 more, same shape: lines 384, 64, 368, 432, 352, 64, 0, 368; latencies 1041..16349 us; mixed lines 0 in all 11
```

Read that as: the request arrives mid-frame (line 16 to 464 — the old code would have split
the refresh across 16 to 480 lines), the swap waits for the next `pos_px == 0`, and the
latency is always less than one frame (16.8 ms) with the longest waits at the earliest lines.
`bench/swap_model.py` computes the same shape analytically: 262 of 263 sampled request
instants tear the old way (worst 240 of 480 lines), and the new protocol is mixed == 0 at
every sampled instant.

## 4. What changed around it, and why

The `report` partition writer moved to 0243's record format — erased once in `app_main`
**before** `panel_start`, then 4 KiB records appended with no erase. The old code erased
64 KiB on every `write_report()`, including calls made while the panel was streaming; an
erase that size stalls the cache the bounce ISR reads PSRAM through, and this task is
entirely about what the panel does during a transition. The swap log is persisted once every
12 swaps and the flush happens *before* the request, so a flash-write stall can only touch
the frame that is leaving. `bench/read_report.py` reads the new format (port `/dev/ttyUSB1`
— the frame; `/dev/ttyUSB0` is the camera's CH340).

## 5. Review history

| pass | verdict | findings |
|---|---|---|
| 1 | FIX | the 100 ms fallback published from the task (could tear if scanout resumed); the boot erase ran after `panel_start`; hold bar NIT |
| 2 | FIX | a timed-out handoff left `s_fb_pending` naming a buffer the caller was about to rewrite; erase retried during scanout |
| 3 | FIX | `publish()` was `void`, so a delete answer could be taken for a confirm that never appeared |
| 4 | FIX | the shuffle spent an index on a photo nobody saw; a dropped keep prompt left a countdown digit on the glass |
| 5 | FIX | `s_cur` stood in for "a photo is visible" — with a failed restore, a hold could open delete for a hidden photo |
| 6 | **SHIP** | one non-blocking NIT left deliberately: `show_notice()` clears `s_photo_on_panel` even if the notice failed to publish — it can only disable a delete briefly, never enable one for a hidden photo |

## 6. Owed

- **Stephen's visual confirm**, both halves: one boot for the splash→photo handover, and a few
  presses for the returns to a photo. The code path is shared and instrumented; the eyes are his.
- The hold bar (`draw_hold_bar`) still animates in the on-screen buffer — a 0254 trade-off kept
  because the card forbids behaviour changes beyond the swap timing. The swap is clean; the bar
  can still tear a few rows.

## Closure — verified by Juno, 2026-11-25

- Branch tip verified against origin: hermes/0260-refresh-jitter @ ac8c40ae6bd1560515fefeabed4092a9f791a62c. Matches the staged reply. Base hermes/0259-slideshow-fixes @ 1a21a3e (as the card allowed, no rebase needed). No CI run — honestly declared: cabin-play carries no workflows.
- Bench proof (read in full from the staged reply): root cause a mid-scan pointer swap — live.c published with a bare `s_fb_live = buf` while the bounce ISR reads the buffer in 16-line bursts (30 fills/frame), so one refresh was part old image, part new. Fix: finished frame waits in s_fb_pending; only the ISR takes it, at pos_px == 0, under a shared portMUX; a handoff with no boundary in 100 ms is cancelled, never published directly. 11 real swaps, every one applied at pos 0, mixed lines 0, latency 1.0-16.35 ms (frame 16.8 ms); model check 262/263 request instants tear the old way, 0/263 the new one. Flash digest matched the branch (verify_flash OK, sha256 2f218650...209e36).
- Copilot: six passes, FIX x5 then SHIP; every finding fixed at the source; one non-blocking NIT left deliberately (show_notice clears s_photo_on_panel even if the notice failed to publish — can only disable a delete, never enable one for a hidden photo).
- Stephen's visual confirm (Nov 25 ~16:54, in chat): rebooted the frame, watched the splash->photo handover, pushed the buttons, deleted files — "everything looks great. Everything syncs really well." That satisfies the two criteria the staged reply left PENDING (boot_transition_clean, button_transitions_clean) and exercises the press-path regressions, including delete.
- Declared gaps carried forward: the hold bar still animates in the on-screen buffer (0254 trade-off) — the swap is frame-boundary safe, the bar can still tear a few rows. Report-partition writer moved to 0243's record format (erase once in app_main before panel_start, then 4 KiB records) — declared scope change, motivated by the old 64 KiB erase stalling the cache the bounce ISR reads PSRAM through. Field note: opening /dev/ttyUSB1 with esptool to read the report resets the board and erases the report at the next boot — read once, not twice.
