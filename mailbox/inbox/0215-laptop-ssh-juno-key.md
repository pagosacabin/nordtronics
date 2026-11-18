# 0215 — Laptop: SSH server running + Juno's public key added

## Assignment
- Hermes: all steps, on the laptop.
- Stephen: permission only if Hermes asks for it (e.g. sudo) — Stephen directed this task and said to ask rather than sit blocked.
- Juno: tests the login from her side once your reply is staged. That test is her verification — do not try to test it for her.

## Context
- Juno is on the tailnet (device `muse`, 100.95.20.50) since 2026-11-15 and SSHes to Stephen's phone with a dedicated key. She has no login on the laptop (tailnet 100.78.85.106); her only channel to this machine is the mailbox.
- Stephen directed (2026-11-18): give Juno a laptop login, handled by Hermes as a task. This also underpins the crew-vault pilot (0214), where Juno's access is read + log-append under the crew conventions.
- Juno generated a dedicated keypair on her VM (`~/.ssh/juno_laptop`). The PUBLIC key — public by design, not a secret, safe in this file — is:
  `ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAILaIs1hkFwF//PXSjyYKFzS3J5mt1g6e96gzNOvoigYe juno@muse-vm`
  Fingerprint: `SHA256:u8xXqeDkyx6ho/9Uof9+lwlmEjLOZszQoy8MuXGv/Ac`

## Task
1. Ensure an SSH server is installed and running on the laptop (Fedora: openssh-server, `sshd`). If it is already installed and active, say so. If it needs installing or enabling (`sudo systemctl enable --now sshd`), do it. If any step needs Stephen's permission or password and you cannot complete it, STOP and stage a reply stating exactly what you need from him — Stephen said to ask, not to sit blocked (and per the 2026-11-18 rule, a block always gets a staged reply, never notes only).
2. Add Juno's public key (the exact line above) as its own line in `~/.ssh/authorized_keys` for the astroboy user. If the key is already present, do not duplicate it. Keep permissions correct: `~/.ssh` 700, `authorized_keys` 600.
3. Confirm sshd is listening (e.g. `ss -tlnp` or `systemctl status sshd` excerpt) and note the laptop's tailnet and LAN addresses as the machine itself reports them.
4. Stage the reply: sshd state (already running, or what you installed/enabled), whether the key was added or already present, the fingerprint of the key you added (must match `SHA256:u8xXqeDkyx6ho/9Uof9+lwlmEjLOZszQoy8MuXGv/Ac`), and the listening confirmation.

## Success criteria
- sshd is active and enabled at boot, OR a staged reply names exactly what Stephen needs to do to unblock that.
- Juno's key line is present exactly once in astroboy's `authorized_keys`, fingerprint matching.
- A staged reply documents all of it.

## Constraints
- Add ONLY Juno's key line. Do not remove or alter any other `authorized_keys` entries.
- Do not change sshd configuration beyond enabling the service — no port changes, no password-auth changes. If something in the config would block a key login, report it in the reply instead of improvising a fix.
- No secrets are involved in this task; the public key may be quoted in the reply.
- Cost: standard tier.

## Proof
- `systemctl status sshd` or `ss -tlnp` excerpt showing the listener, and the line number of Juno's key in `authorized_keys` (grep -n on the fingerprint comment `juno@muse-vm`).

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, sshd state, key status, fingerprint, notes.
