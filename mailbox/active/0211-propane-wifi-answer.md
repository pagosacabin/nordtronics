---
task_id: "0211"
protocol_version: 1.3.0
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T17:45:40+00:00"
lease_until: "2026-11-18T19:15:40+00:00"
---
# 0211 — 0209 answer: one passphrase for both bands — confirm the join and close out

## Assignment
- Hermes: run the confirmation capture and stage the reply. No Newton, no Copilot — no repo change expected.

## Context
0209 sits blocked in mailbox/active/ on a decision owed. Stephen answered 2026-11-18 ~10:31 MST: **the passphrase is the SAME for the 2.4 GHz and 5 GHz bands.** The laptop is on 5 GHz; the house IoT devices live on 2.4 GHz and connect fine. This rules out 0209's candidate (a) — there is no separate 2.4 GHz passphrase — and points at candidate (b): the router refusing this client, possibly a temporary blacklist from this morning's repeated failed attempts (the board has retried continuously since). Stephen is checking the router separately for anything blocking the board's MAC (9c:cc:01:d0:78:80) and may reboot it.

## Task
1. Run one fresh serial capture (up to 10 minutes) watching whether the board joins. The board retries WiFi continuously and Stephen may reboot the router during this window, so a join may happen on its own.
2. If it joins: capture the IP, confirm the API server accepts a client connection, and complete 0209's step 3 exactly as specified there.
3. If it still refuses with `reason='Auth Expired'`: stage a short reply documenting the continued refusal with a confirmed-correct credential, and leave 0209 in active/ pending Stephen's router findings.

## Success criteria
- Either the board is on the network with an IP and the API confirmed (0209's network criterion met), or a staged reply documents the continued refusal. Either way, this task's outcome lands as a staged reply — not only as notes in an active file.

## Constraints
- No credential changes: the value already flashed in 0209 stands. Secrets stay out of the repo and out of the reply, as in 0209.
- One capture window only. Do not reflash unless the board is somehow not running 0209's final image.

## Proof
- Serial log excerpt showing association + IP (if joined) or the refusal lines (if not).

## Reply format
- Staged reply in mailbox/staged/ per protocol 1.3.0: status, device IP or "none", one-paragraph finding.
