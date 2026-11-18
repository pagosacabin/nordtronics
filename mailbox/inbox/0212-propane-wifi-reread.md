# 0212 — Propane board: verify the password three ways, then retry the join post-reboot

## Assignment
- Hermes: all steps — compare the three password values, reflash only if the board holds the wrong one, capture, stage the reply. No Newton, no Copilot — no repo code change expected.
- Juno: verifies the staged reply independently and closes 0209 if the join lands.

## Context
- 0209 reflashed the propane bench board (ESP32-C3 Super Mini, MAC 9c:cc:01:d0:78:80) with secrets built from Stephen's staged `/home/astroboy/.hermes/wifi.txt`. The board has never joined the 2.4 GHz AP: deauth `reason='Auth Expired'` on every attempt (0209: 90 attempts; 0211's capture: 99 attempts / 100 deauths in 600 s).
- 0209 found a one-character case defect in the staged password at index 0 and flashed a case-corrected version; that flash was also refused. 0211 reported the flashed password byte-identical (sha256) to this host's working PSK for the sibling 5 GHz BSSID.
- Stephen confirmed (2026-11-18 ~12:53 MST) the flub he suspected is exactly that index-0 character: **he typed the first character as a capital; it should be lowercase.** So the staged FILE likely still holds the wrong (capital) value, and the case-corrected value 0209 flashed is the one that should be right. He also recalls the October base-station fight (0098 era), where the disconnect evidence could not distinguish a wrong stored PSK from the WPA3/SAE theory.
- Stephen rebooted the Starlink router at ~12:20 MST today, so any router-side refusal state from this morning's attempts should be cleared. No capture has run since the reboot.

## Task
1. Read `/home/astroboy/.hermes/wifi.txt` fresh, exactly as written — case-sensitive, no normalization.
2. Compare three ways, byte-for-byte: (a) file vs the password 0209 actually flashed (your 0209 secrets.yaml), (b) file vs this host's stored working PSK for the NordNickell SSID, (c) flashed vs host PSK. Report MATCH or DIFFER for each. For any DIFFER, report the character index of the first difference and whether it is case-only. Report sha256 + length only — never the value or any substring of it.
3. The host's working PSK is the reference truth (first character lowercase, per Stephen). **Do NOT rebuild secrets from the staged file blindly — the file is the suspect document.** Cases:
   - If the flashed value already matches the host PSK: do NOT reflash. The board already holds the correct password; the file's capital first letter is the flub Stephen identified, and flashing from the file would put the wrong value back. Go to step 4.
   - If the flashed value does NOT match the host PSK: rebuild the secrets so the board holds the host-PSK value (reuse 0208's API key — device identity unchanged), reflash, then go to step 4.
4. Run one serial capture up to 10 minutes watching for the join (post-reboot; none has run yet).
5. If the board joins: record the IP, confirm the ESPHome API accepts a client connection — that completes 0209's step 3 — and say so in the reply.
6. Stage a reply either way. If blocked, stage a short reply as well as notes in the active file (the 2026-11-18 Part C rule).

## Success criteria
- A staged reply stating: the three MATCH/DIFFER results, first-difference index and case-only yes/no for any DIFFER, whether a reflash happened and why, and the capture outcome (joined with IP + API proof, or attempt/deauth counts with the reason).

## Constraints
- The password itself never appears in any task file, staged reply, commit, or quoted log. Hashes, lengths, and character indices only.
- Do not change the ESPHome config, the 2.50 V threshold in NVS, or the API key.
- One reflash at most, one capture window at most (10 minutes). No repeated cycles.
- Leave 0209 in `mailbox/active/` either way; Juno closes it after verification.

## Proof
- Staged reply with the comparison results, plus the capture log under `mailbox/files/0212/` (SSID/password redacted).

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, device IP or "none", the comparison results, one-paragraph finding.
