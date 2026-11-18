# 0212 — Propane board: re-read Stephen's wifi.txt as it stands now, compare, retry the join

## Assignment
- Hermes: all steps — re-read the staged file, compare, reflash if it changed, capture, stage the reply. No Newton, no Copilot — no repo code change expected.
- Juno: verifies the staged reply independently and closes 0209 if the join lands.

## Context
- 0209 reflashed the propane bench board (ESP32-C3 Super Mini, MAC 9c:cc:01:d0:78:80) with secrets built from Stephen's staged `/home/astroboy/.hermes/wifi.txt`. The board has never joined the 2.4 GHz AP: deauth `reason='Auth Expired'` on every attempt (0209: 90 attempts; 0211's capture: 99 attempts / 100 deauths in 600 s).
- 0209 found a one-character case defect in the staged password (index 0) and a case-corrected flash was also refused. 0211 reported the flashed password byte-identical (sha256) to this host's working PSK for the sibling 5 GHz BSSID.
- Stephen now says (2026-11-18 ~12:51 MST) he may have flubbed a character's case when he typed the password into wifi.txt — same length, one letter's case flipped, possibly not the character already tried. He also recalls the October base-station fight (0098 era), where the disconnect evidence could not distinguish a wrong stored PSK from the WPA3/SAE theory and the agreed cheap discriminator was re-entering the known-good password and retrying. Treat the file's CURRENT contents as new input — Stephen may have re-typed it since 0209 used it.
- Stephen rebooted the Starlink router at ~12:20 MST today, so any router-side refusal state from this morning's attempts should be cleared. No capture has run since the reboot.

## Task
1. Read `/home/astroboy/.hermes/wifi.txt` fresh, exactly as written — case-sensitive, no normalization.
2. Byte-compare the password it contains against (a) the password 0209 actually flashed (your 0209 secrets.yaml), and (b) this host's stored working PSK for the NordNickell SSID. Report MATCH or DIFFER for each comparison. If it differs, report the character index of the first difference and whether the difference is case-only. Report sha256 + length only — never the value or any substring of it.
3. If the file differs from what 0209 flashed: rebuild the board's secrets from the file's current contents (reuse 0208's API key — device identity unchanged), reflash, then run one serial capture up to 10 minutes watching for the join.
4. If the file is identical to what was flashed: do not reflash, but still run one serial capture up to 10 minutes — the router was rebooted at ~12:20 and no capture has run since, so a join may now succeed on its own.
5. If the board joins in either case: record the IP, confirm the ESPHome API accepts a client connection — that completes 0209's step 3 — and say so in the reply.
6. Stage a reply either way. If blocked, stage a short reply as well as notes in the active file (the 2026-11-18 Part C rule).

## Success criteria
- A staged reply stating: file-vs-flashed MATCH/DIFFER, file-vs-host-PSK MATCH/DIFFER, first-difference index and case-only yes/no if any, whether a reflash happened, and the capture outcome (joined with IP + API proof, or attempt/deauth counts with the reason).

## Constraints
- The password itself never appears in any task file, staged reply, commit, or quoted log. Hashes, lengths, and character indices only.
- Do not change the ESPHome config, the 2.50 V threshold in NVS, or the API key.
- One reflash at most, one capture window at most (10 minutes). No repeated cycles.
- Leave 0209 in `mailbox/active/` either way; Juno closes it after verification.

## Proof
- Staged reply with the comparison results, plus the capture log under `mailbox/files/0212/` (SSID/password redacted) if a capture ran.

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, device IP or "none", the comparison results, one-paragraph finding.
