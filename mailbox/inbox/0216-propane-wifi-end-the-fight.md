# 0216 — Propane board: end the WiFi fight, and make it the last one

## Assignment
- Hermes: all steps — archaeology, add-on build, flash, swap test if needed, records. Drive Newton/Copilot via Herdr only if needed.
- Stephen: hands only where a step says so (a login, or the router setting in step 5). Hermes asks via the staged reply; Stephen decides.
- Juno: verifies the staged reply and closes 0209 on a successful join + adoption.

## Context
- The propane bench board (ESP32-C3 Super Mini, MAC 9c:cc:01:d0:78:80) has never joined the 2.4 GHz AP: `Auth Expired` on every attempt across 0209/0211/0212 (~300 attempts), on both esp-idf and arduino builds (framework ruled out in Stephen's interactive session, 2026-11-18), with the password question left contradictory (0212 staged reply: file/flashed/host all MATCH; the same-day interactive session: staged file differs from the laptop PSK by one character's case).
- The presence sensor — same board model — was built by Hermes end-to-end on 2026-11-06 (task 0169) through the HA ESPHome add-on using the add-on's own secrets, and has worked since. Stephen's correction, 2026-11-18: Hermes did ALL of it; Stephen added nothing.
- Today Hermes has no working door into the add-on (supervisor API 401 on his token, port 6052 not bound, Samba closed, SSH add-on rejects his keys, browser profile signed out). What carried him in on Nov 6 is unknown.
- Stephen's direction, 2026-11-18 ~14:37 MST: fix this, and fix it so it is never an issue again. The permanent half already exists in motion: the 0213 skeleton with captive-portal provisioning (no credentials in any build). This task closes the propane board and pins down the build/access path so the next board cannot land here.

## Task
1. **Archaeology first (time-box: one pass through your own records).** Find how you accessed the HA ESPHome add-on for 0169 on 2026-11-06 — your task notes, session logs, token/session artifacts, the add-on's own logs. State in the reply: the exact mechanism that worked then, and what is different now (expired session, token scope, add-on config change — whichever the evidence shows). If the records genuinely don't say, state that plainly and proceed to step 2 with whatever access can be re-established without a new grant.
2. **Build the presence way.** Get `propane-bench.yaml` (staged at /home/astroboy/propane-bench.yaml, byte-identical to the 0209 source) into the add-on, build WITH THE ADD-ON'S OWN SECRETS, manual-download the factory-format image, flash it over USB on the laptop (board is on /dev/ttyACM0), and capture serial for the join (up to 10 min). The capture must show the alarm threshold reading 2.50 V — if the image comes up at the 1.00 V default, stop and fix that before proceeding (that default already set the alarm off once today).
   - If step 1's mechanism cannot be restored and no existing access works, do NOT grant yourself new access. Stage a reply naming the single cheapest grant you need (from your own options list of 2026-11-18) and stop there — that is a decision owed to Stephen.
3. **If it joins:** record the IP, prove the API answers, walk it through HA adoption, and report. That completes 0209's step 3.
4. **If it still refuses with `Auth Expired`:** swap test. Flash the SAME add-on-built image to a spare ESP32-C3 Super Mini from Stephen's stock, same bench position, capture the join. A spare that joins convicts the original board/MAC; a spare that also refuses clears every board-side and build-side cause at once.
5. **If the spare also refuses:** the only remaining suspect is the router's security mode for this client class. Stage a reply asking Stephen for the router-side test (Starlink 2.4 GHz to WPA2-only) — his hands, his call — and stop.
6. **Make it permanent (do this on every outcome path):**
   - Update the crew vault page `bench/wifi-lessons.md` (you are librarian) with the final diagnosis, the evidence, and the resolution — dated, sourced to this task.
   - Write the verified ESPHome build/provision path — the one that demonstrably worked at the end of this task, including exactly how you access the add-on — into the 0213 skeleton's README on its branch (or a short addendum file beside it if the branch state makes that cleaner; say which you did). Future builds follow that path; combined with portal provisioning, no future board should be able to land in this state.

## Success criteria
- Either the propane board is on the network, adopted in HA, with IP + API proof — or the swap test has isolated the fault to the board or the router with capture evidence — or a staged reply names the single decision Stephen owes.
- The 0169 access mechanism and what changed are stated from evidence, or plainly stated as not recoverable from records.
- Vault page updated and the build/provision path written into the skeleton documentation.

## Constraints
- Secrets: the add-on's secrets stay in the add-on. No password values in any file, reply, or log excerpt — hashes/lengths only, as before.
- No self-granted access: any new HA access for Hermes is Stephen's decision, requested via the staged reply.
- Do not change the alarm threshold (2.50 V), the API key, or the sensor wiring/config content. The config content is not the suspect; the build path and credential source are.
- One build per branch of the tree; one 10-minute capture per flash. No retry loops.
- If blocked at any step, stage a short reply as well as notes in the active file (Part C rule).
- Cost: standard tier.

## Proof
- Serial capture excerpts (join + IP, or refusal counts), the threshold line from the log, HA adoption evidence (device/entities visible), vault commit pointer, and the skeleton-doc change pointer.

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, device IP or "none", the archaeology finding, which branch of the tree ran, and the permanent-path pointer.
