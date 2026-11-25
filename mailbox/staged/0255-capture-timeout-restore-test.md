---
task_id: "0255"
status: staged
iteration: 1
proof:
  kind: host-only
  frame_build: "hermes/0254-keep-prompt @ e18479a — confirmed on the board by `esptool verify_flash 0x10000`: 'Verification successful (digest matched)'"
  library_before: 43
  library_after: 43
  restore: "previous frame restored to the panel (timeout)"
  camera_back: "console: wifi 192.168.1.118 rssi -34 ch 6, espnow 24M ESP_OK, link peer c4:9e:7e:08:6d:f4 waiting for 'FT'"
notes: |
  PASS — the capture-timeout restore works on the bench. Hermes (laptop, interactive),
  2026-11-25 ~13:05 MST. Test-only task: no code change was made.

  Board identity was checked rather than assumed: the flashed app region verifies
  byte-for-byte against the firmware built from e18479a (digest matched), so the run
  exercised the fixed build and not an older flash. src/ was clean at that commit.

  One artefact worth knowing for future checks: the report's `build :` line reads
  12:25:31, older than the 12:35 link, because main.c was not recompiled by the last
  build — the timestamp belongs to whichever source file compiled the __DATE__/__TIME__
  into the report header, not to the link. The verify_flash digest is the authority.

  Stephen's observation during the test, carried forward: a failed capture currently
  restores the photo silently, so the only signal is that nothing happened. He asked
  for an on-screen "camera not responding" message; that is out of scope here by the
  test-only constraint and is filed as 0256.

  Workers: paused at pickup, resumed on this staging, verified with cron list.
expect-reply-within: 24h
---

# 0255 — Living frame: bench-test the capture-timeout restore

## Assignment

- **Hermes — bench test (this task):** run the capture-timeout test below on the real frame and camera, record exactly what the panel does, and stage the reply. Every step below is yours except the physical unplug/press, which is Stephen's.
- **Stephen — bench side:** unplug the camera USB and press the button once when Hermes says ready, then plug the camera back in at the end. Nothing else needed from him.
- **Newton:** not involved. Frame firmware stays with Hermes.
- **Copilot:** not involved — this is a test-only task. If the test fails, the fix is a follow-up task, not this one.

## Context

- This closes the one declared gap from 0254: the capture-timeout / hash-mismatch restore path is code-reviewed but never bench-exercised. The fix (e18479a, `s_hold` snapshot taken before the countdown) is in the flashed build; Copilot reviewed it and passed it on the second pass. The question is whether it actually works on the bench.
- Frame is running branch `hermes/0254-keep-prompt` @ e18479a (flashed to /dev/ttyUSB1). Camera is the Freenove ESP32-WROVER CAM on the CH340 (/dev/ttyUSB0). Library is at 43 frames.
- The failure this tests: the button is pressed, the countdown runs, and the camera link is lost mid-countdown, so no photo ever arrives. The frame must give up after the capture timeout (~12 s) and put back the photo that was showing before the press — not a countdown digit, not a black screen.
- Record the outcome in the crew-vault living-frame bench notes (bench/frame-storage-and-link.md).

## Task

Bench-test the capture-timeout restore. One deliverable: a staged reply reporting exactly what the panel did.

1. Confirm the frame is running the 0254 build and note the photo currently showing and the library count before the test.
2. Have Stephen unplug the camera USB (CH340) so the camera is unreachable — this simulates the lost link. Do not unplug the frame.
3. Have Stephen press the button once. Watch the panel and the frame serial log through the countdown and the wait that follows.
4. Wait for the capture timeout (~12 s). Record what the panel shows at each stage: the countdown digits, what it shows while waiting, and what it settles on after the timeout.
5. Check the frame's report partition / serial log for the timeout and restore lines and quote them.
6. Have Stephen plug the camera back in. Confirm the frame is back to normal operation: slideshow running, camera link re-established, and the library count unchanged.

## Success criteria

- After the capture timeout, the panel shows the photo that was showing before the press — not a countdown digit, not a black screen.
- Library count unchanged across the test (43 → 43): nothing is stored on a failed capture.
- After the camera is plugged back in, the slideshow runs normally and the camera link is re-established.

## Constraints

- Test only — no code changes in this task. If the restore does NOT behave as above, report exactly what the panel showed and stop. The fix is a follow-up task, not this one.
- Do not touch the splash code or the 0254 branch.
- Run on the standard worker model (DeepSeek Flash). Do not escalate to a premium/pro tier — no model or provider choice beyond that is involved in this task.

## Proof

- Staged reply with the observed panel behavior at each stage, the timeout/restore log lines quoted, and the library count before and after.

## Reply format

Stage the reply in this file with these fields filled in:

- `frame_build`: branch + SHA confirmed on the frame
- `photo_before`: which photo was showing before the press (library index or description)
- `library_before`: count before the test
- `library_after`: count after the test
- `panel_during_countdown`: what the panel showed (digits, in order)
- `panel_after_timeout`: what the panel settled on after the ~12 s timeout
- `restore_correct`: yes / no + one line (photo restored, digit stuck, black screen, or other — say exactly what it showed)
- `timeout_log_lines`: quoted serial/report lines for the timeout and restore
- `camera_reconnected`: yes / no + one line (slideshow normal again after plug-back)
- `notes`: anything unexpected

# Reply — 0255: capture-timeout restore works

**Pass.** The frame gave up after the 12 s capture timeout and put the previous photograph back on
the panel — no countdown digit, no black screen, and nothing stored.

| Field | Value |
|---|---|
| `frame_build` | `hermes/0254-keep-prompt` @ **e18479a**, confirmed on the board: `esptool verify_flash 0x10000 firmware.bin` → *"Verification successful (digest matched)"* |
| `photo_before` | the slideshow photograph that was on the panel before the press (report: `show 0`, then `BUTTON pressed`) |
| `library_before` | **43** |
| `library_after` | **43** |
| `panel_during_countdown` | the digits **3, 2, 1**, one per second, as designed |
| `panel_after_timeout` | the photograph that had been showing before the press, restored |
| `restore_correct` | **yes** — the prior photo came back; no digit stuck, no black screen |
| `timeout_log_lines` | see below, quoted from the report partition |
| `camera_reconnected` | **yes** — camera console after plug-in: `wifi : 192.168.1.118 rssi -34 ch 6`, `rate : espnow 24M -> ESP_OK`, `link : peer c4:9e:7e:08:6d:f4 ch 6, waiting for 'FT'` |

## The log, verbatim

```
live       : SD library holds 43 frames, next index 43 in /sdcard/LIVE
live       : show 0
live       : BUTTON pressed, library 43 frames
live       : countdown 3
live       : countdown 2
live       : countdown 1
live       : SHOOT sent to f0:16:1d:ae:2b:20
live       : TIMEOUT waiting for the frame (12 s)
live       : previous frame restored to the panel (timeout)
```

No `frame ok` line and no `written` line: the failed capture stored nothing, which is why the library
count is 43 on both sides of the test.

## How the board was identified

`verify_flash` over the whole 1,616,256 B app region against the firmware built from `e18479a`,
rather than trusting the flash log from the earlier session. Digest matched. Worth recording that the
report's `build :` line shows 12:25:31 while the link happened at 12:35 — `main.c`, which carries the
report header, was not recompiled by the last build, so that timestamp is not the link time and is not
a reliable way to identify a build. The digest is.

## Follow-up noted, not done here

A failed capture restores the photo **silently**, so the only evidence is that nothing happened.
Stephen asked for an on-screen "camera not responding" message; this task is test-only, so no code was
changed — that is filed as 0256.
