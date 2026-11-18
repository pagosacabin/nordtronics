---
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T21:46:08+00:00"
lease_until: "2026-11-18T23:16:08+00:00"
---
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
