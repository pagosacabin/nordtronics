---
task_id: "0216"
protocol_version: 1.3.0
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T21:46:08+00:00"
lease_until: "2026-11-18T23:16:08+00:00"
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
notes: |
  STEP 1 DONE 2026-11-18 14:50-15:00 MST — SEPARATOR TEST RAN; THE BOARD IS THE FAULT.
  Interactive run from Stephen's session, using the 0217 hotspot relay (hotspot ON,
  2.4 GHz). No new access grant, no router change, no change to threshold, API key or
  sensor content.
    - The second AP is a plain WPA2 one: `Nightrain`, security `WPA2`, key-mgmt
      `wpa-psk`, pmf `0`, BSSID 3E:7A:51:90:BA:F4 ch 6. Its passphrase was proven from
      an independent client BEFORE the board test — this laptop joined it and took
      10.255.222.69 — so a board refusal cannot be blamed on a bad relay. (The relayed
      value was never staged to a file on this host; 0217 is its whole provenance.)
    - Board flashed with those credentials (same config otherwise, esp-idf,
      config_hash=0xbf0df018), one flash (`Hash of data verified.`), one 600 s capture
      14:50:30-15:00:30 MST: **0 joins, 0 `Got IP`**, 191 `reason='Auth Expired'`,
      16 `Probe Request Unsuccessful`, 10 `Unspecified` (all-zero bssid, first attempt).
      Sensors healthy throughout: `MQ-6=1.475 V  Figaro=1.354 V  thr=2.50 V  alarm=off`.
    - **Branch entered: (b) the board is faulty.** Suspect (a) — the Starlink refusing
      this client — is dead, because an AP that admits other clients with a verified
      password refuses this board identically. **MAC 9c:cc:01:d0:78:80 CONDENNED.**
      Presence 9c:cc:01:d0:64:6c and the shed JK-BMS are healthy on the same AP, so
      this is a per-board defect, not a family or firmware trait.
    - Board restored afterwards to the delivered house-credential image
      (`thr=2.50 V  alarm=off`); the hotspot build, its secrets file and its build tree
      were shredded. Nothing about the hotspot credential remains on disk.
  STEP 3 OUTSTANDING — Stephen's hands: move the rig onto a spare C3 Super Mini, rebuild,
    flash, provision it through the skeleton's portal, adopt; 0209 step 3 then closes
    against the new board, and the condemned board is physically marked.
  STEP 4 DONE: crew vault `bench/wifi-lessons.md` @ 63a48d8 (separator-test section,
  condensed rules 7-8); skeleton addendum `firmware/skeleton/BUILD-AND-PROVISION.md` on
  branch `hermes/0213-esphome-skeleton` @ **7924694** (tip moved from c0e5abd by that
  documentation-only commit — no code change on the branch).
  0217 deleted from `inbox/` in this same commit, per its own instruction.
  NOT staged: the task is blocked on hardware, `iteration` unchanged, no `proof` block.
  The worker's park note below is the previous run's record and is left as written.

  PARKED — hot-spot test owed (Stephen's hands), no proof block, nothing staged.
  Run 2026-11-18 14:46-15:20 MST (21:46-22:20 UTC), off-peak (PEAK: OFF-PEAK
  21:45 UTC), PROTOCOL MATCH 1.3.0. 0216 arrived with no front-matter; the
  protocol fields were added at pickup (rule 16). Claimed with the transition
  helper at 77067dc (iteration 1). Lowest-task-number check first: 0142 and 0187
  are `status: superseded`; 0159, 0209, 0213 and 0214 are decision-blocked and
  were left completely untouched (no notes edit, no iteration change); 0176 in
  inbox/ is `status: superseded` ("Do not pick up"). 0216 was the only
  actionable task. Cost fields are `unknown` because this harness exposes no
  per-call usage to the cron session (not estimated).

  WHY PARKED AND NOT STAGED. Step 1 (the second-AP separator test) needs a
  2.4 GHz phone hotspot that is NOT UP and a credential that was never staged:
  no hotspot SSID/password file exists anywhere on this host (`~/.hermes/wifi.txt`
  is untouched since 12:50 MST and is the old house-network file; a sweep for
  files newer than 13:00 MST found none), and a 45 s read-only capture at
  15:05 MST saw exactly one BSSID in band — the house AP the board is already
  refused by. Nothing else in 0216 can be decided without that test, so no
  branch of the fault tree can be entered and the success criteria (fault
  isolated to router or board) are unattainable this run. Per the mailbox rule
  a task whose criterion failed is parked, never staged.

  WHAT WAS DONE (real, verified):
   1. Read-only board state capture, 45 s, /dev/ttyACM0 opened O_RDONLY|
      O_NONBLOCK with no line changes and no tcsetattr: 4,070 bytes / 44 lines,
      and **zero `ESP-ROM` banners**, so this capture did not reset the board
      (contrast the vault's rule 4 — a capture that shows a boot banner did).
      The board is on the delivered image and retrying: 7x
      `Disconnected ssid='<house SSID>' bssid=BE:D9:D7:28:BA:46
      reason='Auth Expired'`, 0 `Got IP`, sensors healthy and drifting as
      expected (MQ-6 1.390 -> 1.406 V, Figaro 1.345 -> 1.367 V), thr=2.50 V,
      alarm=off. No threshold, API key or sensor-config change; nothing flashed.
   2. Crew vault updated and committed (0216 step 4, first bullet):
      /home/astroboy/crew-vault @ 29082d9 — `bench/wifi-lessons.md` now carries
      the 2026-11-18 in-session result as ruled-out rows (build method,
      framework, signal/association, MAC collision), the firmware-hash equality
      with 0209's deliverable image (...ba74307d) that rules the credential out
      for good, the two remaining suspects (AP refusing 9c:cc:01:d0:78:80, or a
      faulty radio/efuse), the separator test recorded as OWED NOT RUN, the
      corrected presence-build history (local esphome 2026.9.1 + the panels'
      secrets file, never the HA add-on), and three new earned rules (a capture
      with no boot banner did not reset the board; establish how a working build
      was built before citing it; a proven password can still not be the cause).
      INDEX.md and `log/2026-11-18.md` were updated to match. Local-only repo
      (no remote), so the pointer is the commit SHA.
   3. Skeleton documentation (0216 step 4, second bullet): addendum file
      `firmware/skeleton/BUILD-AND-PROVISION.md` committed on branch
      `hermes/0213-esphome-skeleton` (an ADDENDUM beside README.md, not a
      rewrite, so 0213's reviewed three-layer text is untouched). It states
      plainly: build locally with esphome (2026.9.1, esp-idf), never the HA
      add-on; credentials by captive portal from the skeleton onward, never
      staged files again. It also separates what is verified from what is
      designed: local build + flash is hardware-verified, portal provisioning is
      marked "design, to be proven by the first board provisioned through it".
      Pushed 668f244 -> c0e5abd (branch tip; `git ls-remote` confirms).
      NOTE FOR 0213's VERIFIER: 0213's notes cite tip 668f244. The code files are
      unchanged at 668f244; the tip moved only by this one addendum. Zero CI runs
      on the branch — earned, not asserted: none of the six workflows matches
      `firmware/skeleton/**` or a `hermes/NNNN-*` branch (`gh run list --branch
      hermes/0213-esphome-skeleton` empty; commits/c0e5abd/check-runs
      total_count = 0). No ntfy receipt: no compiled artifact was published —
      the skeleton's compile proof is still 0213's, and this run's change is
      documentation only.
      The key-generation snippet in the addendum was executed in scratch before
      being written down (44-char base64, 32 decoded bytes, mode 600).

  WHAT STEP 1 NEEDS FROM STEPHEN, EXACTLY (one action, then this task runs to a
  decision in one tick): turn on the phone's **2.4 GHz** hotspot, WPA2, any
  throwaway password, and stage SSID + password in a file (the established
  convention is `~/.hermes/wifi.txt`, or any new file — say which). The next run
  then flashes the board with those credentials and the same config otherwise,
  captures <=10 minutes, and takes branch 2 (router: client count / blocked
  entry for 9c:cc:01:d0:78:80, then the WPA2-only test) or branch 3 (board:
  spare C3 Super Mini, old MAC recorded as condemned in the vault). If the
  hotspot cannot be raised 2.4 GHz, branch 2's router checks alone are still
  the cheapest next step and need no staging.

  NO SECRETS: no password value appears here, in the vault's diff or in the
  skeleton addendum; PSKs are cited as sha256 prefixes only, per 0216's
  constraint.
---
## Run status — 2026-11-18 ~15:10 MST: PARKED, hot-spot test owed

**No proof block: the fault is not yet isolated, so there is nothing to prove.**
The board is back on the delivered image and retrying; nothing was flashed this
run and no threshold, key or sensor config moved.

**Which branch ran: none.** Step 1 needs the 2.4 GHz phone hotspot, and it is not
up: no hotspot credential is staged on this host and a 45 s read-only capture at
15:05 MST saw only the house AP in band. So the separator test that decides
"router or board" could not be executed. Device IP: **none**.

**Evidence this run.** 4,070 B / 44 lines from `/dev/ttyACM0`, opened
non-blocking with no line changes; **0 `ESP-ROM` banners**, so the capture did not
reset the board. Seven refusals in the window, all
`reason='Auth Expired'` against `BE:D9:D7:28:BA:46`; sensors reporting
(MQ-6 1.390 → 1.406 V, Figaro 1.345 → 1.367 V), `thr=2.50 V`, `alarm=off`.

**Delivered on the permanent half (step 4).**
- Crew vault `~/crew-vault` @ `29082d9` — `bench/wifi-lessons.md` carries the
  full evidence chain, the ruled-out table extended with build method, framework,
  signal and MAC collision, the credential ruled out for good (firmware hash
  matches 0209's deliverable, `…ba74307d`), the two remaining suspects, the
  separator test marked owed, the corrected presence-build history (local
  `esphome` + the panels' secrets file, never the add-on), and three new rules.
- `firmware/skeleton/BUILD-AND-PROVISION.md` on
  `hermes/0213-esphome-skeleton` @ `c0e5abd` (addendum beside README.md, so
  README.md is unchanged): local `esphome` build, portal provisioning,
  never a staged credential file again — with the verified/designed halves
  labelled separately.

**One action owed (Stephen).** Raise the phone's **2.4 GHz** hotspot (WPA2, any
throwaway password) and stage SSID + password in a file. The next tick then runs
the test and either opens the router branch (client count / blocked entry for
`9c:cc:01:d0:78:80`, then the WPA2-only check) or condemns the board and moves the
rig to a spare C3. If a 2.4 GHz hotspot is not available, the router checks alone
are the cheapest next step and need no staging — say so and this task takes
branch 2 without the separator.

# 0216 — Propane board: end the WiFi fight, and make it the last one (AMENDED 2026-11-18 ~14:40 MST)

## Assignment
- Hermes: all steps. Drive Newton/Copilot via Herdr only if needed.
- Stephen: hands only where a step says so (phone hotspot, router setting). Hermes asks via the staged reply or the live session; Stephen decides.
- Juno: verifies the staged reply and closes 0209 on a successful join + adoption.

## Amendment note — read first
This replaces the original 0216 filed minutes earlier. Its add-on steps are VOID. In Stephen's interactive session (2026-11-18 ~14:36) Hermes established from his own work that **the presence sensor was never built through the HA add-on**: it was a local esphome 2026.9.1 build using the panels' secrets file (`~/Documents/esphome/JC4827W543/secrets.yaml`). The add-on account in Hermes's earlier message was his own error, which he owned in-session. Do not chase add-on access for this task.

## Context — what is now settled by direct test
- Hermes rebuilt and reflashed the propane board in-session exactly the presence way: local esphome 2026.9.1, esp-idf, same config content, the panels' secrets file (PSK byte-identical to the laptop's, sha256 d2462d2f…), only the board's own api_encryption_key merged in. Result: **unchanged** — `Auth Expired` on every attempt. Serial after: MQ-6=1.444 V, Figaro=1.385 V, thr=2.50 V, alarm off; board left on the delivered firmware.
- The compile's firmware hash matches 0209's deliverable image hash (…ba74307d), independently confirming the credential in the build is exactly the one already ruled on. **The password is ruled out for good.**
- Also ruled out by direct test: build method, framework (arduino build also refused), signal (full bars; failure is at auth, not association), MAC collision (this board 9c:cc:01:d0:78:80 vs presence 9c:cc:01:d0:64:6c).
- Two suspects remain: **(a)** the Starlink AP is refusing this specific client (blocked entry for the MAC, or a client limit), or **(b)** this Super Mini's radio/efuse is faulty.
- Stephen's direction stands: fix this, and fix it so it is never an issue again. The permanent half is already in motion: the 0213 skeleton with captive-portal provisioning (no credentials in any build).

## Task
1. **Second-AP test (the separator).** Stephen enables his phone hotspot — **2.4 GHz band** (the C3 cannot see 5 GHz), WPA2, any password — and stages the SSID + password in a file for you, as you asked in-session. Flash the board with those credentials (same config otherwise), capture serial up to 10 minutes.
   - **Joins the hotspot** → the board is fine; the Starlink is refusing it on NordNickell. Go to step 2.
   - **Refused there too** → the board is faulty. Go to step 3.
2. **Router branch.** Stage a reply (or report in-session) asking Stephen for the two router-side checks, in order: (i) the Starlink app's client count and any blocked entry for 9c:cc:01:d0:78:80; (ii) if nothing shows, the WPA2-only test on the 2.4 GHz band. His hands, his call. On a join to NordNickell afterwards: record IP, prove the API, complete HA adoption (0209 step 3).
3. **Board branch.** Move the propane rig to a spare ESP32-C3 Super Mini from Stephen's stock: rebuild with the standard (NordNickell) credentials, flash, capture. On join: IP + API proof + HA adoption (0209 step 3 closes against the new board). Record the old board's MAC as condemned in the vault page below, so it is never trusted again; Stephen marks the physical board.
4. **Make it permanent (every outcome path):**
   - Update the crew vault page `bench/wifi-lessons.md` (you are librarian) with the final diagnosis, the evidence chain, and the resolution — dated, sourced to this task. Include the corrected presence-build history: local esphome + the panels' secrets file, never the add-on.
   - Write the verified ESPHome build/provision path into the 0213 skeleton's README on its branch (or a short addendum file beside it; say which). The path must state plainly: local esphome build; credentials by captive portal from the skeleton onward — never staged files again.

## Success criteria
- The fault is isolated to the router or the board by the second-AP test, with capture evidence — and, on the router branch, either the board joins NordNickell after Stephen's router step or the reply names exactly what remains his to decide; on the board branch, the rig is running on the spare, joined, and adopted.
- Vault page updated; skeleton documentation carries the verified path and the corrected history.

## Constraints
- Secrets: no password values (house or hotspot) in any reply, file, or log excerpt — hashes/lengths only. The hotspot password is Stephen's throwaway; treat it the same anyway.
- Do not change the alarm threshold (2.50 V), the API key, or the sensor config content.
- One flash + one 10-minute capture per branch step. No retry loops.
- No new access grants for Hermes in this task.
- If blocked, stage a short reply as well as notes in the active file (Part C rule).
- Cost: standard tier.

## Proof
- Serial capture excerpts for each flash (join + IP, or refusal counts), the threshold line, HA adoption evidence where reached, vault commit pointer, skeleton-doc pointer.

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, device IP or "none", which branch ran and its evidence, the permanent-path pointers.
