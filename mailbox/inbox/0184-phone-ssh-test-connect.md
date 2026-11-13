---
task_id: "0184"
protocol_version: 1.2.0
status: inbox
iteration: 0
expect-reply-within: 4h
proof: []
notes: ""
---

# 0184 — Test-connect: SSH from the laptop into Stephen's phone (Termux) with the dedicated key

## Assignment
- Hermes: all steps, on the laptop. Newton: no steps.
- Stephen: no steps in this task (his phone side is done; if the connection fails for a phone-side reason, Juno relays what's needed).

## Context
- 0183 is CLOSED (verified by Juno, archived a12ea0218259, Nov 13): you generated a dedicated ed25519 keypair at `~/.ssh/phone-termux` on the laptop (empty passphrase, mode 600) and published the public key at `keys/phone-termux.pub` on branch `hermes/0183-phone-key`. Key fingerprint: **SHA256:YpcCodWz0b5Q8tyE4dpBzpFm2jo8/pP2QXKuw3MpiGc** (comment `hermes-to-stephens-a35-termux`). The VPS operator key `id_ed25519` is a different key and stays out of this task.
- Phone side (all done by Stephen, Nov 13): Samsung A35, tailnet name `stephens-a35`, tailnet IP **100.103.159.65**. Termux (current build) with OpenSSH 10.3p1; `sshd` started by Stephen, listening on port **8022**; Termux user **u0_a342**. Stephen confirmed at 13:48 MST that the public key is installed in the phone's `~/.ssh/authorized_keys` (installed with the curl one-liner from 0183, chmod 600 applied).
- Phone network state: Tailscale on the phone is ON (Stephen restored it at 12:48 MST after the OpenSSH install; 0182's confirmation read at 12:15 MST showed the phone Online, direct ping 139–193 ms). Whether sshd autostarts on the phone (termux-services / sv-enable) is NOT confirmed — if the connect fails because nothing listens on 8022, that is a finding, not something to fix from here.
- Pattern: this is the 0058 to 0183's 0057 — same care as your VPS SSH test (keyscan/pin first, verbatim command, verbose self-check, scope what you do NOT claim).

## Task
One deliverable: a staged reply proving (or honestly failing) a key-only SSH login from the laptop into the phone over the tailnet, with phone-side checks run inside that session.

1. Pin the phone's host key locally first: `ssh-keyscan -p 8022 100.103.159.65` — record the ED25519 host-key fingerprint you obtained (`ssh-keygen -lf` on the scanned key) in the reply. If keyscan gets nothing, stop and report that (it separates "nothing listening / tunnel path" from "auth").
2. Connect with the dedicated key only, no agent, no password fallback:
   `ssh -i ~/.ssh/phone-termux -o IdentitiesOnly=yes -p 8022 u0_a342@100.103.159.65 'whoami; hostname; uname -a'`
   Run it verbatim. Success = exit 0 and `whoami` prints `u0_a342`.
3. In the same style (one ssh invocation), run the phone-side checks that motivated this whole arc:
   `ssh -i ~/.ssh/phone-termux -o IdentitiesOnly=yes -p 8022 u0_a342@100.103.159.65 'curl -s -o /dev/null -w "HA-from-phone: %{http_code} %{time_total}s\n" http://100.76.58.105:8123/; ping -c 3 100.76.58.105'`
   Report the HA HTTP code and the ping result exactly as they come back — a failure here with a working login is a valid, valuable finding (it is precisely the phone-side view 0181 lacked).
4. Self-check the auth path with one verbose probe (`ssh -v ... true`, same key and options) and confirm from the trace that the offered/accepted identity is the phone-termux key (fingerprint above) and that no password authentication was used.

## Success criteria
- The staged reply shows the verbatim commands and their raw output, including exit status for steps 2 and 3.
- Step 2 either succeeds with `whoami` = `u0_a342`, or the reply names the exact failure layer with evidence (keyscan empty / connection timed out / connection refused / permission denied) — no retry loop, no silent retries; at most two attempts total, and say how many were made.
- The recorded phone host-key fingerprint comes from your own keyscan in step 1.
- The verbose self-check confirms key-only auth with the phone-termux identity, or says plainly that it could not be completed.

## Constraints
- Include only the dedicated key `~/.ssh/phone-termux` for this connection; use no other identity and no password authentication.
- Read-only on the phone: run only the commands named above; include no installs, no config edits, no file writes on the phone.
- On the laptop, the only write is the known_hosts entry for the phone that step 1/2 creates.
- Cost block in the staged front-matter per protocol 1.2.0.

## Proof
- Host-only connection test: proof is the pasted session output in the reply body, as in 0058/0181/0182. Set `proof: []` and say so in `notes` — Juno verifies by reading the staged file against this spec and Stephen's own phone-side facts, not by pointers.

## Reply format
Stage the task file in `mailbox/staged/` with `status: staged`, the protocol cost block, `proof: []`, and a `notes` field whose first line is the one-line verdict: LOGIN OK + HA-from-phone result, or the named failure layer. Body: verdict first, then steps 1–4 raw output in order, then a short "not claimed" scope line (e.g. nothing about sshd autostart or other authorized_keys entries, unless a step's output actually shows it).
