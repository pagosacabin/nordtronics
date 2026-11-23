---
task_id: "0247"
title: Live link recovery — WB per shot, camera off the AP, bounded waits, slideshow holes
status: in_progress
iteration: 1
expect-reply-within: 24h
proof: []
notes: |
  Filed by Hermes 2026-11-23 from Stephen's external static review of the 0244/0245
  firmware (review passed on to Stephen in chat; not a Juno filing). Claimed
  immediately in the same interactive session, workers paused for the duration.
  Deviates from the usual provenance rule: commits are hermes-cronrunner, so this
  file looks self-filed because it is.
---

## Assignment

- **Hermes:** the code change, the bench verification, the staged reply.
- **Copilot:** not requested for this card; the review that generated it already
  came from outside. If the change grows, ask for a pass.
- **Juno:** verifies the staged reply.

## Context

The 0244/0245 firmware works and is archived, but an external static review found
that its strength (strict verification) is paired with a weakness: almost no
retry, and several real defects around state. This card fixes the correctness
items. The structural report-partition rework is deliberately left to its own card.

## Task

1. **White balance per shot, with a NaN guard.** `wb_update()` smooths 60/40
   against the previous shot, a leftover from streaming; one press per minute can
   be biased by the previous scene. Worse, a channel mean of 0 gives inf/NaN, the
   `g < 0.5f` clamp is false for NaN, and the filter then carries NaN until
   reboot. Compute gains fresh per shot and bail out (identity gains) if any
   channel mean is below 1 LSB.
2. **Camera off the AP.** ESP-NOW needs no association. `WiFi.mode(WIFI_STA)` +
   `WiFi.disconnect()` + an explicit channel, no `WiFi.begin()`, no `secrets.h`,
   no 30 s boot wait. Log the result of `esp_wifi_config_espnow_rate` so the
   24 Mbps claim is measured rather than asserted.
3. **Bounded waits and a failure redraw.** The post-SHOOT deadline drops from
   30 s to 12 s, and a timeout or rejection re-publishes the previous frame so
   the countdown digit does not stay on the panel.
4. **Slideshow holes.** `live_load()` is tried `s_lib_count` times while indices
   run to `s_lib_max`; with holes that can burn many 10 s cycles. Try up to
   `s_lib_max`.
5. **Session id per frame**, not per boot, so it identifies a frame.
6. **Docs and dead code.** README transfer figure (1.7-2.1 s, not 5.5 s),
   "post-countdown" wording for the 2.9 s number, remove the unused
   `link_recv`/`link_init` in `main.c` (stale 4-byte header offset), fix the
   `CMakeLists.txt` comment, and note that the 800x600/1024x768 envs cannot run
   this 640x480 image.

**Declared not done here, with reasons in the reply:** the chunk NACK/resend
protocol (the `s_seen` bitmap as a one-packet reply after `FE`), which is the
right fix for a lost chunk but is a protocol change with its own bench matrix;
and the report-partition rework (erase before `panel_start`, append-only records,
an `app_logf` mutex, and the USB-Serial-JTAG console check), which is structural.

## Success criteria

- A shot taken in different light immediately after another uses its own gains
  (log both gain triples, they differ).
- A fully black frame produces identity gains, no NaN, and a normal store.
- The camera boots and shoots with no WiFi association; the log shows the rate
  call's return code.
- A rejected/timed-out frame leaves the previous picture on the panel, not the
  countdown digit.
- Five consecutive frames: fail 0, SHA match, library grows by one each.
- Docs and dead code match the shipped behaviour.

## Proof

- Branch `hermes/0247-live-recovery` on origin at the reported SHA.
- Camera log: no `WiFi.begin`, the rate call's return, the gain triples.
- Frame report: the five frames, the deadline behaviour, the redraw after a
  deliberately induced failure.

## Reply format

Standard staged reply: status, per-item change with numbers, proof mapping,
deviations.
