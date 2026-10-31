---
task_id: "0166"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
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
proof:
  - branch: none
    note: "read-only research task -- the spec says clone/fetch the public repo and
      change nothing. No branch was created, nothing was pushed except this mailbox
      transition on main, and no file in nordtronics was modified."
  - run: none
    note: "no CI run covers this deliverable and none can: no repository file changed,
      so no workflow could fire. Enumerated: .github/workflows/ holds android-build.yml,
      android-companion-v0.yml, backend.yml, detection-sim.yml, platformio.yml,
      website-check.yml -- each is branch-allow-listed and/or path-filtered, and none
      of their paths changed here. The only runs this run's commits can produce are
      the marketing-site check on the mailbox commit itself."
  - files: []
    note: "no repository file changed by design (read-only task). The evidence below
      is quoted document text, not an artifact in the tree."
  - sources:
      - repo: "github.com/makerbase-mks/MKS-SERVO42C, branch MKS-SERVO42C-V1.1,
          HEAD 08d6322be2732bbdc30a526573dce1bded8e18be (2022-10-31).
          Path: 'User Manual/MKS SERVO42C User Manual V1.1.2.pdf' (33 pp)."
      - repo: "same project's official wiki, git clone
          https://github.com/makerbase-mks/MKS-SERVO42C.wiki.git,
          HEAD ecd764b351dfc046f7c13f91b45cbefa5e1f38eb (2022-05-30).
          Paths quoted by file+line: Serial-communication-description.md,
          Serial-communication-description-V1.0.md, Serial-communication-format-V1.0.md,
          Run-the-motor-with-serial-command.md, Error-list.md."
      - method: "PDF text layer extracted with `pdftotext -layout`; every checksum frame
          quoted below was recomputed from its bytes (8-bit additive sum, low byte) and
          validates. No value in this reply is inferred from a third-party driver."
notes: |-
  ANSWERED FROM THE DOC, NOT FROM THE BENCH. (a) 0x33 is the received-pulse counter
  (int32) -- confirmed three ways in the official doc, and the "working mode" table that
  disagreed is not this firmware. (b) NO documented per-command pulse/distance limit
  exists anywhere in the manual or wiki: the 0xFD distance field is uint32_t and the only
  documented limit is speed, `Vrpm no great than 2000RPM`. So the 2224-pulse ceiling is
  NOT explained by the doc and stays an open bench question -- but the doc does resolve
  two things around it: 0xFD's ack in the manual is a 3-state status where 01 means
  "run starting" (not "done"), and 0x36 wraps every revolution, which reconciles the two
  3200-pulse angle records in the bench thread that looked contradictory. (c) Factory
  default baud 38400 confirmed; NO command reads back the settings block -- 0165's verdict
  is correct per the doc.

  TEN DISCREPANCIES vs PROTOCOL.md, listed in the reply body. The material ones: 0x30 is
  documented (carry int32 + value uint16 = 6 payload bytes) and the bench reply
  `e0 FF FF FF FF FE DD B7` IS the documented shape -- PROTOCOL.md's "unexplained" item is
  a misreading, and its 0x30/u16 table entry is wrong; 0x3D (release locked-rotor
  protection) is missing from the command table; and the two official docs contradict each
  other on 0xFD's reply semantics (manual: 0/1/2 fail/starting/complete; wiki: 01=ok),
  with PROTOCOL.md silently following the wiki.

  WHAT THE REPO DOES NOT CONTAIN, checked because the task expected docs there:
  `01_Makerbase SERVO42C Related documents/` holds exactly one zip whose non-image
  contents are Arduino + STM32 demo sketches, a shell STL and UartAssist.exe -- no
  protocol document; `MKS SERVO42C CONTROL TOOL/` is two Windows .exe/.zip pairs with no
  source; `Firmware/firmware.txt` is a 30-byte placeholder ("Please wait, we are
  updating."), so there is no firmware to read either. The manual PDF is the only
  protocol document in the tree; 0x36 is absent from it and exists only on the wiki.

  DECLARED LIMIT OF THIS RUN: two illustrations the prose points at
  (repo image/image3/position control V1.1.png and vel V1.1.png) were not OCR'd -- no
  OCR binary on this host -- so a numeric limit could in principle hide in an image. The
  surrounding prose carries none, and no text file in either the repo tree or the wiki
  mentions a pulse/distance limit (searched for limit/great than/max/2000/2224).
---

# 0166 — Pull the UART protocol doc from the official MKS-SERVO42C repo

## Context
Stephen shared the official repo: https://github.com/makerbase-mks/MKS-SERVO42C (default branch MKS-SERVO42C-V1.1). It holds `01_Makerbase SERVO42C Related documents/`, `User Manual/`, firmware, and an open-source `MKS SERVO42C CONTROL TOOL`. Juno could not list the documents directory from her side (API failures); you have web access — pull it yourself. This is the authoritative source that should settle the remaining open questions from the bench thread.

## Task
1. Find the serial/UART protocol documentation in the repo (likely under `01_Makerbase SERVO42C Related documents/` or `User Manual/`).
2. Answer definitively from that doc:
   a. What does command **0x33** actually return — working mode, or a move/pulse counter? (Your bench work treated it as a counter tracking ±800; a web table claimed working mode.)
   b. Is there a documented per-command pulse/distance/speed limit that explains the **2224-pulse ceiling** seen on 3200-pulse commands?
   c. Confirm the factory default UART baud (your research said 38400) and whether ANY command reads back the settings block (your 0165 probe said no).
3. Note anything else in the doc that contradicts your PROTOCOL.md — list each discrepancy.

## Success criteria
- a/b/c answered with doc quotes or section references, not guesses.
- Discrepancy list vs PROTOCOL.md, or "none found."

## Constraints
- Default model tier. Read-only: clone/fetch the public repo, change nothing.

## Proof
- Staged reply quoting the relevant doc sections (with file paths in the repo).

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

---

# Reply — ANSWERED from the official doc (2026-10-31 05:19 MDT), read-only

**status:** staged. **No branch, no SHA, no CI run** — this task changed no repository
file, per its own "read-only: clone/fetch the public repo, change nothing" constraint.
The proof is the quoted document text below, with file + section for each claim.

## Answers, in one screen

| # | Question | Answer |
|---|---|---|
| a | What is **0x33**? | **A pulse counter.** "Read the number of pulses received", payload `pulses(int32_t)` (manual p.12 §5.1 item 2; wiki `Serial-communication-description.md` L19–29). It is **not** working mode — working mode is a *write-only* command of its own (`0x82`, p.14 §5.2 item 3). Bench reading (counter tracking ±800) is correct; the web table was describing a different firmware family. |
| b | Documented per-command pulse/distance/speed limit explaining **2224**? | **No distance limit exists in the doc at all.** The 0xFD distance field is `pulses: uint32_t` (p.29 §5.5 item 5) — 3200 is ~0.00007 % of its range. The only documented limit is **speed**: "the Vrpm no great than 2000RPM" (p.26 §5.5 item 2, p.29 §5.5 item 5). The 2224 shortfall is therefore **not** explained by the doc. Two doc facts do bear on it — see below. |
| c | Factory default baud, and any settings read-back? | **38400 confirmed as the factory default** ("(Default: 38400)", menu item 11, p.9; `0x8A 04` = 38400, p.19 §5.2 item 11). **No command reads back any setting** — the complete command set has no such command (0165's verdict is correct per the doc). Note: the bench unit answers at **9600**, i.e. its `UartBaud` was changed from factory default (0162). |

## (a) 0x33 is the received-pulse counter — three independent statements in the doc

1. **Manual, p.12 §5.1 item 2** (file `User Manual/MKS SERVO42C User Manual V1.1.2.pdf`):

   > `2. Command2 : e0 33 CRC`
   > `Read the number of pulses received.`
   > Uplink packet: `e0 | pulses(int32_t) | tCHK`
   > `Example: Send e0 33 13 / return e0 00 00 01 00 e1  (256 pulses)`

   (checksum recomputed: 0xE0+0x00+0x00+0x01+0x00 = 0xE1 = final byte ✓)

2. **Manual, p.4 §1.3 item 3** — the OLED field that shows the same quantity:
   > `3. 0clk - the pulses have been received.`

3. **Wiki, `Serial-communication-description.md` L19–29** (V1.1 page):
   > `2. command 2 : e0 33 tCHK` — `Read the number of pulses received`
   > `Return e0, number pulses(int32_t),rCHK.` — `return e0 00 00 01 00 e1 (256 pulses)`

**Work mode is a separate, write-only command:** p.14 §5.2 item 3 —
> `3. Set the work mode （Same as the "Mode" option on screen）`
> `e0 82 mode (0~2)` — `mode = 0 CR_OPEN`, `mode = 1 CR_vFOC`, `mode = 2 CR_UART`
> `return e0 01 e1 (successful)`

There is no read counterpart for `0x82`, so "ask the driver which mode it is in" is not a
thing this firmware can do — which is exactly why the 0x33-as-mode table is impossible on a
42C. It matches the SERVO42D/57D (ZDT) family already identified in PROTOCOL.md §5 item 2.

## (b) No documented pulse/distance limit — and the two doc facts that do bear on 2224

**The distance field.** p.29 §5.5 item 5:
> `5. Run the motor by serial command`
> `e0 | FD | VAL (uint8_t) | pulses (uint32_t) | tCHK`
> `VAL: is the direction and speed.` / `pulses: is the number of pulses.`
> `Uplink: e0 | status(0~2) | rCHK`
> `status = 0 run fail.` / `status = 1 run starting….` / `status = 2 run complete.`
> `Note: the Vrpm no great than 2000RPM.`

**The only numeric limit in the whole command set is speed.** Searching the extracted
manual text and the entire wiki clone for `limit|great than|no more than|max|2000` returns
exactly two hits, both the same RPM note (p.26 for `0xF6`, p.29 for `0xFD`). Nothing caps
distance, and nothing caps pulses per command. So a 3200-pulse command is inside every
documented bound and the 2224 result cannot be dismissed as "the driver enforcing a spec
limit" — **the doc has no such limit to enforce.**

**Two things the doc does establish about that test, both of which change how it reads:**

1. **`0xFD`'s reply is a lifecycle status, not a success flag** (manual p.29, quoted above):
   `01` means *"run starting"*, `02` means *"run complete"*. The bench treated `01` as
   "ok" and read `0x33`/`0x36` immediately after — which is reading during the move, not
   after it. **The documented completion check is to poll until `02`** (or sample `0x33`
   until it stops changing), and only then compare counts. If the driver is still
   accelerating through the ACC ramp when the counter is sampled, a short count is expected
   with no firmware fault involved. This is a bench-method gap the doc names explicitly.
   (The wiki contradicts this — see discrepancy 3.)
2. **`0x36` wraps every revolution**, so the two 3200-pulse angle records in the bench
   thread are the *same* move, not two different ones. p.12 §5.1 / wiki L37:
   > `The motor rotates one circle, the corresponding angle value range is 0~65535.`

   The arithmetic closes: `2224 × 20.48 = 45547.5 cnt = 250.2°`, and `65536 − 45547.5 =
   19988.5`, which is the `+19960 cnt` recorded in the second probe (`19960 / 20.48 =
   974.6 pulses`, i.e. the same 250.2° arc seen across the wrap, within the bench's own
   ±1-pulse scale error). So `0163`'s `-45548 cnt` row and its `+19960 cnt` INFO lines are
   consistent, and **2224 is a real, repeatable shortfall** (69.5 % of the commanded
   3200) rather than a measurement artefact — it is just not a *documented* one.

Still open, and it needs the bench, not the doc: poll status to `02`, sample `0x33` until
stationary, then read `0x3E` (blocked/unblocked) and, if blocked, `0x3D` to release. Worth
noting that `Protect` ships **disabled** by default (menu item 9, p.8, "(Default: Disable)"),
so a stall would not have latched a protection state on a factory-default unit.

## (c) Default baud 38400 confirmed; no settings read-back confirmed

**Factory default is 38400** — three statements agree:

- Menu item 11, p.9: `11. UartBaud ：Set the baud rate of serial.` … `(Default: 38400)`
- §5.2 item 11, p.19: `e0 8A baud(01~06)` — `01 9600 / 02 19200 / 03 25000 / 04 38400 / 05 57600 / 06 115200`
- Part6, p.30: `6.1 Config the SERVO42C — 1. Menu → Mode → CR_UART. 2. Menu → UartBaud → 38400. 3. Menu → UartAddr → 0xe0.`
  and the V1.0 wiki format page: "The serial communication baud rate of the driver board
  38400 is set on the UartBaud option".

**No command reads back the settings block.** The manual's complete command set is:

| Section | Codes | Returns |
|---|---|---|
| 5.1 read parameters (p.12–13) | `0x30 0x33 0x39 0x3A 0x3D 0x3E` | live physical values only (encoder, pulses, angle error, En pin, protection release, shaft status) |
| 5.2 system parameters (p.14–19) | `0x80 0x81 0x82 0x83 0x84 0x85 0x86 0x87 0x88 0x89 0x8A 0x8B 0x3F` | `status` only (01 success / 00 fail) |
| 5.3 zeroMode (p.20–22) | `0x90 0x91 0x92 0x93 0x94` | `status` only |
| 5.4 PID/ACC/torque (p.23–24) | `0xA1 0xA2 0xA3 0xA4 0xA5` | `status` only |
| 5.5 control (p.25–29) | `0xF3 0xF6 0xF7 0xFF 0xFD` | `status` / position status |

Every read returns a measured quantity; every parameter setting returns one status byte and
nothing else. **0165's "no settings readout" verdict is correct per the doc**, and the doc
offers no exception anywhere. Note the one apparent counter-example is not one: `0x3D` sits
in the *read-parameter* section but is a **write** (release protection).

## Discrepancy list vs PROTOCOL.md (`firmware/servo42c-uart-test/PROTOCOL.md`, 0163 lineage)

1. **`0x30` is documented, and the bench's reply was valid — PROTOCOL.md's "unexplained"
   item and its table entry are both wrong.** PROTOCOL.md §3 lists the `0x30` reply as
   `u16, 0…0xFFFF`, and §5 item 7 concludes the captured `e0 FF FF FF FF FE DD B7` "only
   validates as addr + **6** payload bytes … not the documented `u16`". The manual documents
   6 payload bytes: p.12 §5.1 item 1 shows `carry int32_t` **and** `value uint16_t`, with
   `Send e0 30 10 → return e0 00 00 00 00 40 00 20`, and Part6 p.31 shows a live
   `return "E0 FF FF FF FF E6 7E 40"`. So the captured frame is exactly the documented
   shape — carry `-1`, value `0xFEDD` — and its checksum validates (sum 0x6B7 → B7 ✓).
   Correct the table to carry+value and delete the "unexplained" note.
2. **`0x3D` is missing entirely.** p.13 §5.1 item 6: `e0 3D` — "Release the motor shaft
   locked-rotor protection state", `status =1 release success` / `0`, example
   `e0 3d 1d → e0 01 e1`. PROTOCOL.md's "six codes are the *only* read commands" survives
   scrutiny (0x3D writes), but the command table is incomplete without it, and it is the
   documented recovery path for a latched stall.
3. **`0xFD`'s acknowledgement: the two official docs contradict each other.** Manual p.29:
   `status 0 = run fail, 1 = run starting…, 2 = run complete`. Wiki L804–810:
   `01 - successful; 00 - Failure`. PROTOCOL.md adopted the wiki's reading without saying
   so. This is the one discrepancy with behavioural consequence: on the manual's reading,
   `01` is *not* permission to start measuring.
4. **`0x36` exists only in the wiki, not in the manual.** The manual's read section
   (p.12–13) documents `0x30 0x33 0x39 0x3A 0x3D 0x3E` — no `0x36` — and its item numbering
   skips "command5". The command is real (wiki L31–43: `e0 36 tCHK`, `angle(int32_t)`,
   `0~65535` per revolution, `return e0 00 00 40 00 20 (angle 90°)`; and a live
   `E0 FF FF F0 07 D5` in `Run-the-motor-with-serial-command.md`), but anyone checking the
   PDF alone will not find it. PROTOCOL.md should cite the wiki, not "a Makerbase source",
   for this row.
5. **Whole command families are absent from PROTOCOL.md**: `0x80` calibrate, `0x81`
   MotType, `0x82` work mode, `0x83` Ma, `0x85` En active level, `0x86` Dir, `0x87`
   AutoSDD, `0x88` Protect, `0x89` Mplyer, `0x8A` baud, `0x8B` address, `0x90–0x94` zeroMode,
   `0xA1–0xA5` Kp/Ki/Kd/ACC/MaxT, plus `0x3D`. Defensible (PROTOCOL.md scopes itself to what
   the bench needed) but §3 reads as *the* command table; either complete it or label it a
   subset.
6. **Speed ceiling: manual 2000 RPM vs README 1000 RPM.** Manual p.26 and p.29: "the Vrpm
   no great than 2000RPM". Repo `README.md` comparison table: "Maximum speed | 1000RPM".
   PROTOCOL.md states no top speed at all. Any speed-gear calculation should clamp at the
   manual's 2000 RPM and note the README contradiction rather than silently picking one.
7. **Wiki `0x30` example is malformed** (`Serial-communication-description.md` L13–17):
   `Send e0 30 10 / return e0 40 00 20` — a 3-byte reply that contradicts the 6-byte
   description three lines above it and the manual. Do not use it as a framing reference.
8. **Wiki `0xFD` label bug** (L790–798): frame is `e0 fd xx xx xx xx xx` and then "The last
   two bytes(xx xx xx xx) define the number of pulses" — four bytes called two. The manual's
   `pulses uint32_t` is the reliable wording. PROTOCOL.md's V1.0-int16 / V1.1-int32 split is
   otherwise correct (V1.0 wiki: `e0 fd 01 0c 80`).
9. **The V1.0 self-contradiction PROTOCOL.md flags is confirmed, and wider than stated.**
   The V1.0 page's `0x36` entry says the reply ends in `tCHK` and its example
   `e0 00 00 40 00 20` *does* carry a check byte — while the same page's `0x39`
   (`e0 00 B7`) and `0xFD` (`e0 01`) examples do not, and the V1.0 format page has no
   checksum rule at all. Probing both framings, as the sketch does, remains correct.
10. **Cosmetic:** manual Part4 labels the uplink's trailing byte `rCHK` in the diagram, but
    every worked example sends the same additive-sum byte as the downlink `tCHK`
    (recomputed for 7 frames here, all validate). PROTOCOL.md's rule is right; the diagram
    label is wrong.

**"None found" is not the answer to item 3 of the task** — the list above is what the doc
contradicts. Items 1, 2 and 3 are worth acting on before the next bench run.

## What the doc does not settle

The **2224 shortfall**. Nothing in the manual or the wiki limits distance, so the cause is
either the bench's sampling method (most likely, given the `01`-means-starting status) or an
undocumented firmware behaviour. Settling it needs the board, not the PDF: poll `0xFD` to
`02`, sample `0x33` until stationary, read `0x3E` after the move, and repeat at a low speed
gear so the ACC ramp cannot be the variable.

## Repo contents, checked because the task expected docs there

| Path | Contents | Protocol doc? |
|---|---|---|
| `01_Makerbase SERVO42C Related documents/` | one zip: Arduino + STM32 demo sketches, shell STL, `UartAssist.exe` | **no** |
| `User Manual/` | `MKS SERVO42C User Manual V1.1.2.pdf` (33 pp) | **yes — the only one** |
| `MKS SERVO42C CONTROL TOOL/` | two Windows `.exe`/`.zip` pairs, no source | no |
| `Hardware/` | PCB/schematic PDFs | no |
| `Firmware/firmware.txt` | 30 bytes: "Please wait, we are updating." | no (no firmware published) |

`User Manual/User Manual` is a 1-byte stub, not a document.

**Declared limit of this run:** two illustrations the prose points at
(`image/image3/position control V1.1.png`, `image/image3/vel V1.1.png`) were not OCR'd — no
OCR binary on this host — so a numeric limit could in principle hide in an image. The
surrounding prose carries none and no text file in the repo tree or wiki mentions one.
