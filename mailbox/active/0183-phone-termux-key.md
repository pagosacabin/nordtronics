---
task_id: "0183"
protocol_version: 1.2.0
status: in_progress
iteration: 1
expect-reply-within: 4h
proof: []
notes: "Interactive-session hold 2026-11-13 12:10 MST: Fri mailbox worker (7aff6948c2c1) paused for the duration so no cron tick resumes this task; resumed after staging. Fingerprint goes on the first line of notes at staging."
---

# 0183 — Generate a dedicated SSH key for Stephen's phone (Termux) and stage the public key

## Assignment
- Hermes: all steps below, on the laptop. Newton: no steps.
- Stephen (outside this task, coordinated by Juno): after Juno relays the staged reply, he runs one curl line on the phone to install the public key into Termux `authorized_keys`. The test-connect task is filed separately once he confirms that step — same 0057 → 0058 split as the VPS key.

## Context
- Goal: Hermes gets a shell on Stephen's phone (Samsung A35, tailnet name `stephens-a35`, tailnet IP **100.103.159.65**) so future phone-side network faults can be diagnosed from the phone itself. Motivation: task 0181 (Nov 13) — HA unreachable was a dead phone Tailscale tunnel, diagnosable from the laptop only by inference; a phone shell closes that gap.
- Phone side is DONE by Stephen (Nov 13, screenshots to Juno): Termux (current build, live termux.net repo) with OpenSSH 10.3p1 installed; `sshd` started, listening on port **8022**; Termux user is **u0_a342**. Nothing on the phone accepts a login yet — no password set, `~/.ssh/authorized_keys` empty. This task supplies the key.
- Precedent: 0057 (Hermes generated/staged his ed25519 key) → Juno installed it on the VPS → 0058 proved the connection. Same shape here, with one change: a **dedicated** keypair for the phone, NOT the existing VPS operator key at `/home/astroboy/.ssh/id_ed25519` (fingerprint SHA256:6EtXtLADDPItdy6eLCVzxoIfOd6fzO7ef/g8lT2csBI). Phone access must be revocable without touching VPS access.
- The nordtronics repo is public (Stephen approved, 2026-09-15). A public key is not a secret, so publishing the .pub file in the repo is an approved delivery channel; the private key never leaves the laptop.

## Task
One deliverable: a dedicated keypair on the laptop plus the public key published and staged for relay to Stephen.

1. Generate the keypair (stop and report instead if either file already exists — include no overwrite):
   `ssh-keygen -t ed25519 -f ~/.ssh/phone-termux -N '' -C 'hermes-to-stephens-a35-termux'`
   Then `chmod 600 ~/.ssh/phone-termux`. Empty passphrase, matching the 0057 headless-use precedent.
2. Record the fingerprint: `ssh-keygen -lf ~/.ssh/phone-termux.pub`.
3. Publish the public key so Stephen can fetch it with one line on the phone: commit the exact contents of `~/.ssh/phone-termux.pub` as the repo file `keys/phone-termux.pub` on branch `hermes/0183-phone-key` in pagosacabin/nordtronics, using your proven git path. The file content is the single .pub line, nothing else.
4. Stage the reply (format below) carrying: the fingerprint, the full public-key line, the branch tip SHA, and the raw fetch URL:
   `https://raw.githubusercontent.com/pagosacabin/nordtronics/hermes/0183-phone-key/keys/phone-termux.pub`

## Success criteria
- `~/.ssh/phone-termux` and `~/.ssh/phone-termux.pub` exist on the laptop; the staged fingerprint matches `ssh-keygen -lf` output for that .pub.
- Branch `hermes/0183-phone-key` exists on origin, its tip equals the SHA in `proof`, and `keys/phone-termux.pub` on that branch is byte-identical to the local .pub file (one line).
- The staged reply contains the fingerprint, the full public-key line, and the raw URL from step 4.
- No connection to the phone is attempted in this task, and the existing VPS key (`id_ed25519`) is untouched.

## Constraints
- Include only the dedicated key above; use no existing key for this, and reuse this key for nothing else.
- The private key (`~/.ssh/phone-termux`) stays on the laptop: include it in no commit, no staged reply, no chat, no file outside `~/.ssh/`.
- Publish only the public key, only at the repo path and branch named above.
- Cost block in the staged front-matter per protocol 1.2.0.

## Proof
- `proof:` carries the branch + tip SHA of the `keys/phone-termux.pub` commit (Juno verifies the branch tip and fetches the file to compare against the staged key line), plus the pasted `ssh-keygen -lf` output in the body.

## Reply format
Stage the task file in `mailbox/staged/` with `status: staged`, the protocol cost block, `proof` as above, and a `notes` field whose first line is the key fingerprint. Body, in order: fingerprint line, full public-key line, raw URL, then the exact phone command Juno will hand Stephen:
`mkdir -p ~/.ssh && curl -sL <raw-url> >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys`
(with `<raw-url>` replaced by the real URL).
